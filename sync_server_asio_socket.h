#ifndef BWGAME_SYNC_SERVER_ASIO_SOCKET_H
#define BWGAME_SYNC_SERVER_ASIO_SOCKET_H

#include "util.h"

#define ASIO_STANDALONE
#include "deps/asio/asio.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <iostream>

namespace {
static inline void silence_asio_warnings() {
	(void)asio::error::system_category;
	(void)asio::error::netdb_category;
	(void)asio::error::addrinfo_category;
	(void)asio::error::misc_category;
}
}

namespace bwgame {

template<typename socket_T>
struct sync_server_asio_socket {

	asio::io_service io_service;
	asio::io_service::work work{io_service};
	asio::steady_timer timer{io_service};

	template<typename T, typename release_F>
	struct async_handle_t {
		T* obj;
		release_F release_f;
		async_handle_t(T* obj, release_F&& release_f) : obj(obj), release_f(release_f) {
			++obj->async_count;
		}
		~async_handle_t() {
			if (!--obj->async_count) release_f(obj);
		}
		async_handle_t(const async_handle_t& n) : obj(n.obj), release_f(n.release_f) {
			++obj->async_count;
		}
		operator T*() const {
			return obj;
		}
		T* get() const {
			return obj;
		}
	};
	template<typename T, typename release_F>
	async_handle_t<T, release_F> async_handle(T* obj, release_F&&f) {
		return async_handle_t<T, release_F>(obj, std::forward<release_F>(f));
	}

	const size_t recv_size = 0x1000;

	struct send_buffer_t {
		std::array<uint8_t, 0x2000> buffer;
		int refcount = 0;
		size_t pos = 0;
	};

	using send_buffers_t = a_list<send_buffer_t>;
	send_buffers_t send_buffers;

	struct message_buffer_handle {
		sync_server_asio_socket* server = nullptr;
		typename send_buffers_t::iterator buffer;
		size_t offset = 0;
		size_t size = 0;
		message_buffer_handle() = default;
		message_buffer_handle(sync_server_asio_socket& server, typename send_buffers_t::iterator buffer) : server(&server), buffer(buffer) {
			++buffer->refcount;
		}
		message_buffer_handle(const message_buffer_handle& n) {
			server = n.server;
			buffer = n.buffer;
			offset = n.offset;
			size = n.size;
			if (server) ++buffer->refcount;
		}
		message_buffer_handle& operator=(const message_buffer_handle& n) {
			server = n.server;
			buffer = n.buffer;
			offset = n.offset;
			size = n.size;
			if (server) ++buffer->refcount;
			return *this;
		}
		~message_buffer_handle() {
			if (server && --buffer->refcount == 0) {
				server->send_buffers.splice(server->send_buffers.begin(), server->send_buffers, buffer);
			}
		}
	};

	struct client_t {
		client_t(socket_T socket) : socket(std::move(socket)) {}
		typename a_list<client_t>::iterator my_it;
		socket_T socket;
		int async_count = 0;
		a_vector<uint8_t> recv_buffer;
		size_t recv_message_size = 0;
		a_deque<message_buffer_handle> send_queue;
		bool is_dead = false;
		std::function<void()> on_kill;
		std::function<void(const void*, size_t)> on_message;
		bool allow_send = false;
	};

	a_list<client_t> clients;

	typename send_buffers_t::iterator get_send_buffer_with_space(size_t n) {
		for (auto i = send_buffers.begin(); i != send_buffers.end(); ++i) {
			if (i->buffer.size() - i->pos >= n) return i;
		}
		send_buffers.emplace_back();
		return std::prev(send_buffers.end());
	}

	struct message_t {
		sync_server_asio_socket& server;
		static_vector<message_buffer_handle, 2> buffers;
		size_t total_size = 0;
		template<typename T>
		void put(T v) {
			std::array<uint8_t, sizeof(T)> buf;
			data_loading::set_value_at<true>(buf.data(), v);
			put(buf.data(), buf.size());
		}
		void put(const void* data, size_t size) {
			auto* buf = &*buffers.back().buffer;
			size_t left = buf->buffer.size() - buf->pos;
			if (left >= size) {
				memcpy(buf->buffer.data() + buf->pos, data, size);
				buf->pos += size;
				buffers.back().size += size;
				total_size += size;
			} else {
				memcpy(buf->buffer.data() + buf->pos, data, left);
				buf->pos += left;
				buffers.back().size += left;
				total_size += left;
				if (buffers.size() == buffers.max_size()) error("message_t: too much data :(");
				buffers.emplace_back(server, server.get_send_buffer_with_space(size - left));
				buffers.back().offset = buffers.back().buffer->pos;
				put((const void*)((const char*)data + left), size - left);
			}
		}
	};

	message_t new_message() {
		message_t r{*this};
		auto buffer = get_send_buffer_with_space(0x10);
		r.buffers.emplace_back(*this, buffer);
		r.buffers.back().offset = buffer->pos;
		r.template put<uint16_t>(0);
		return r;
	}

	void write_handler(client_t* c, const asio::error_code& ec, size_t bytes_transferred) {
		if (ec) {
			if (c->on_kill) c->on_kill();
		} else {
			auto& v = c->send_queue.front();
			if (bytes_transferred > v.size) error("write_handler: bytes_transferred > v.size");
			v.offset += bytes_transferred;
			v.size -= bytes_transferred;
			if (v.size == 0) c->send_queue.pop_front();
			if (!c->send_queue.empty()) send_send_queue(c);
		}
	}

	void send_send_queue(client_t* client) {
		auto& v = client->send_queue.front();
		client->socket.async_write_some(asio::buffer(v.buffer->buffer.data() + v.offset, v.size), std::bind(&sync_server_asio_socket::write_handler, this, async_handle(client, std::bind(&sync_server_asio_socket::async_release, this, std::placeholders::_1)), std::placeholders::_1, std::placeholders::_2));
	}

	void send_to(const message_t& d, client_t* client) {
		if (!client->allow_send) return;
		for (auto& v : d.buffers) {
			client->send_queue.push_back(v);
			if (client->send_queue.size() == 1) {
				send_send_queue(client);
			}
		}
	}

	void allow_send(const void* h, bool allow) {
		((client_t*)h)->allow_send = allow;
	}

	void send_message(const message_t& d, const void* h) {
		data_loading::set_value_at<true>(d.buffers[0].buffer->buffer.data() + d.buffers[0].offset, (uint16_t)(d.total_size - 2));
		if (h) {
			send_to(d, (client_t*)h);
		} else {
			for (auto& v : clients) {
				send_to(d, &v);
			}
		}
	}

	a_vector<client_t*> new_clients;

	void new_connection_handler(socket_T socket) {
		clients.emplace_back(std::move(socket));
		client_t* c = &clients.back();
		c->my_it = std::prev(clients.end());
		++c->async_count;
		new_clients.push_back(c);
	}

	void kill_client(const void* h) {
		client_t* c = (client_t*)h;
		c->is_dead = true;
		c->on_kill = {};
		c->on_message = {};
		if (c->socket.is_open()) c->socket.close();
		if (--c->async_count == 0) async_release(c);
	}

	struct timer_guard_t {
		~timer_guard_t() {
			timer.cancel();
		}
		asio::steady_timer& timer;
	};

	template<typename duration_T, typename callback_F>
	timer_guard_t set_timeout_guarded(duration_T&& duration, callback_F&& callback) {
		timer.expires_from_now(duration);
		timer.async_wait([callback = std::forward<callback_F>(callback)](const asio::error_code& ec) {
			if (!ec) callback();
		});
		return timer_guard_t{timer};
	}

	template<typename duration_T, typename callback_F>
	void set_timeout(duration_T&& duration, callback_F&& callback) {
		timer.expires_from_now(duration);
		timer.async_wait([callback = std::forward<callback_F>(callback)](const asio::error_code& ec) {
			if (!ec) callback();
		});
	}

	void async_release(client_t* c) {
		clients.erase(c->my_it);
	}

	void read_handler(client_t* c, const asio::error_code& ec, size_t bytes_transferred) {
		if (ec) {
			if (c->on_kill) c->on_kill();
		} else {
			c->recv_buffer.resize(c->recv_buffer.size() - recv_size + bytes_transferred);

			while (true) {
				if (c->recv_message_size == 0) {
					if (c->recv_buffer.size() >= 2) {
						data_loading::data_reader_le r((uint8_t*)c->recv_buffer.data(), (uint8_t*)c->recv_buffer.data() + c->recv_buffer.size());
						c->recv_message_size = r.get<uint16_t>();
						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 2);
					} else break;
				} else if (c->recv_buffer.size() >= c->recv_message_size) {
					if (c->on_message) c->on_message(c->recv_buffer.data(), c->recv_message_size);
					c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + c->recv_message_size);
					c->recv_message_size = 0;
				} else break;
			}

			size_t new_size = c->recv_buffer.size() + recv_size;
			c->recv_buffer.resize(new_size);
			c->socket.async_read_some(asio::buffer(c->recv_buffer.data() + c->recv_buffer.size() - recv_size, recv_size), std::bind(&sync_server_asio_socket::read_handler, this, async_handle(c, std::bind(&sync_server_asio_socket::async_release, this, std::placeholders::_1)), std::placeholders::_1, std::placeholders::_2));
		}
	}

	template<typename F>
	void set_on_kill(const void* h, F&& f) {
		client_t* c = (client_t*)h;
		c->on_kill = std::forward<F>(f);
	}

	template<typename F>
	void set_on_message(const void* h, F&& f) {
		client_t* c = (client_t*)h;
		c->on_message = std::forward<F>(f);
		c->recv_buffer.resize(recv_size);
		c->socket.async_read_some(asio::buffer(c->recv_buffer), std::bind(&sync_server_asio_socket::read_handler, this, async_handle(c, std::bind(&sync_server_asio_socket::async_release, this, std::placeholders::_1)), std::placeholders::_1, std::placeholders::_2));
	}

	template<typename on_new_client_F>
	void poll(on_new_client_F&& on_new_client) {
		io_service.poll();
		for (auto* c : new_clients) {
			c->allow_send = true;
			on_new_client(c);
		}
		new_clients.clear();
	}

	template<typename on_new_client_F>
	void run_one(on_new_client_F&& on_new_client) {
		if (!io_service.run_one()) error("asio io_service has no work");
		for (auto* c : new_clients) {
			c->allow_send = true;
			on_new_client(c);
		}
		new_clients.clear();
	}

	template<typename on_new_client_F, typename pred_F>
	void run_until(on_new_client_F&& on_new_client, pred_F&& pred) {
		while (!pred()) {
			run_one(on_new_client);
		}
	}
};

}

#endif
