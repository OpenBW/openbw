#ifndef BWGAME_SYNC_SERVER_ASIO_TCP_H
#define BWGAME_SYNC_SERVER_ASIO_TCP_H

#include "sync_server_asio_socket.h"

namespace bwgame {

struct sync_server_asio_tcp: sync_server_asio_socket<asio::ip::tcp::socket> {
	
	struct acceptor_t {
		sync_server_asio_socket& server;
		asio::ip::tcp::acceptor acceptor{server.io_service};
		asio::ip::tcp::socket socket{server.io_service};
		acceptor_t(sync_server_asio_socket& server) : server(server) {}
	};

	void accept_handler(const asio::error_code& ec, std::shared_ptr<acceptor_t> acceptor) {
		if (!ec) new_connection_handler(std::move(acceptor->socket));
		auto* a = &*acceptor;
		a->acceptor.async_accept(a->socket, std::bind(&sync_server_asio_tcp::accept_handler, this, std::placeholders::_1, std::move(acceptor)));
	}
	
	void bind(const asio::ip::tcp::endpoint& ep) {
		auto a = std::make_shared<acceptor_t>(*this);
		auto& acceptor = a->acceptor;
		asio::error_code ec;
		acceptor.open(ep.protocol());
		acceptor.set_option(asio::socket_base::reuse_address(true));
		acceptor.bind(ep, ec);
		if (ec) return;
		acceptor.listen(asio::socket_base::max_connections, ec);
		if (ec) return;
		acceptor.async_accept(a->socket, std::bind(&sync_server_asio_tcp::accept_handler, this, std::placeholders::_1, a));
	}
	
	void bind(const a_string& hostname, int port) {
		asio::error_code ec;
		asio::ip::address address = asio::ip::address::from_string(hostname.c_str(), ec);
		if (ec) {
			auto resolver = std::make_shared<asio::ip::tcp::resolver>(io_service);
			asio::ip::tcp::resolver::query query(hostname.c_str(), "");
			using it_t = asio::ip::tcp::resolver::iterator;
			auto* r = &*resolver;
			r->async_resolve(query, [this, port, resolver = std::move(resolver)](const asio::error_code& ec, it_t iterator) {
				for (;iterator != it_t{}; ++iterator) {
					bind({iterator->endpoint().address(), (unsigned short)port});
				}
			});
		} else {
			bind(asio::ip::tcp::endpoint(address, port));
		}
	}
	
	void connect(const asio::ip::tcp::endpoint& ep) {
		auto socket = std::make_shared<asio::ip::tcp::socket>(io_service);
		auto* s = &*socket;
		s->async_connect(ep, [this, socket = std::move(socket)](const asio::error_code& ec) {
			if (!ec) {
				new_connection_handler(std::move(*socket));
			}
		});
	}
	
	void connect(const a_string& hostname, int port) {
		asio::error_code ec;
		asio::ip::address address = asio::ip::address::from_string(hostname.c_str(), ec);
		if (ec) {
			auto resolver = std::make_shared<asio::ip::tcp::resolver>(io_service);
			asio::ip::tcp::resolver::query query(hostname.c_str(), "");
			using it_t = asio::ip::tcp::resolver::iterator;
			auto* r = &*resolver;
			r->async_resolve(query, [this, port, resolver = std::move(resolver)](const asio::error_code& ec, it_t iterator) {
				for (;iterator != it_t{}; ++iterator) {	
					connect({iterator->endpoint().address(), (unsigned short)port});
				}
			});
		} else {
			connect({address, (unsigned short)port});
		}
	}
};

}

#endif
