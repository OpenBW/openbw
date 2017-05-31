#ifndef BWGAME_SYNC_H
#define BWGAME_SYNC_H

#include "bwgame.h"
#include "actions.h"
#include "replay.h"
#include <chrono>
#include <random>
#include <thread>
#include <functional>
#include <type_traits>
#include <typeinfo>

namespace bwgame {

struct sync_state {
	struct scheduled_action {
		uint8_t frame;
		size_t data_begin;
		size_t data_end;
	};
	
	int latency = 2;
	bool is_first_bwapi_compatible_frame = true;
	
	int game_starting_countdown = 0;
	uint32_t start_game_seed = 0;
	bool game_started = false;
	
	struct uid_t {
		std::array<uint32_t, 8> vals{};
		static uid_t generate() {
			uid_t r;
			std::array<uint32_t, 8> arr;
			arr[0] = 42;
			arr[1] = (uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
			arr[2] = (uint32_t)std::hash<std::thread::id>()(std::this_thread::get_id());
			arr[3] = (uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();;
			arr[4] = (uint32_t)std::chrono::steady_clock::now().time_since_epoch().count();;
			arr[5] = (uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();;
			arr[6] = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();;
			arr[7] = 1;
			std::seed_seq seq(arr.begin(), arr.end());
			seq.generate(r.vals.begin(), r.vals.end());
			data_loading::crc32_t crc32;
			const uint8_t* c = (const uint8_t*)r.vals.data();
			size_t n = 32;
			for (auto& v : r.vals) {
				v ^= crc32(c, n);
				c += 2;
				n -= 2;
			}
			return r;
		}
		bool operator<(const uid_t& n) const {
			return vals > n.vals;
		}
		bool operator==(const uid_t& n) const {
			return vals == n.vals;
		}
		bool operator!=(const uid_t& n) const {
			return vals != n.vals;
		}
		a_string str() {
			a_string r;
			for (auto& v : vals) r += format(r.empty() ? "%08x" : "-%08x", v);
			return r;
		}
	};
	
	struct client_t {
		uid_t uid;
		bool has_uid = false;
		int local_id = 0;
		int player_slot = -1;
		const void* h = nullptr;
		a_vector<uint8_t> buffer;
		size_t buffer_begin = 0;
		size_t buffer_end = 0;
		a_deque<scheduled_action> scheduled_actions;
		uint8_t frame = 0;
		a_string name;
		bool game_started = false;
		bool has_greeted = false;
		std::chrono::steady_clock::time_point last_synced;
	};
	
	a_list<client_t> clients = {{uid_t::generate(), true}};
	int next_client_id = 1;
	client_t* local_client = &clients.front();
	
	int sync_frame = 0;
	
	bool has_initialized = false;
	std::array<race_t, 12> initial_slot_races;
	std::array<int, 12> initial_slot_controllers;
	
	bool game_type_melee = false;
};

struct sync_server_noop {
	struct message_t {
		template<typename T>
		void put(T v) {}
		void put(const void* data, size_t size) {}
	};
	message_t new_message() {return {};}
	void send_message(const message_t& d, const void* h) {}
	void allow_send(const void* h, bool allow) {}
	void kill_client(const void* h) {}
	template<typename F>
	void set_on_kill(const void* h, F&& f) {}
	template<typename F>
	void set_on_message(const void* h, F&& f) {}
	std::chrono::steady_clock::time_point timeout_time;
	std::function<void()> timeout_function;
	template<typename duration_T, typename callback_F>
	void set_timeout(duration_T&& duration, callback_F&& callback) {
		timeout_time = std::chrono::steady_clock::now() + duration;
		timeout_function = std::forward<callback_F>(callback);
	}
	template<typename on_new_client_F>
	void poll(on_new_client_F&& on_new_client) {
		if (timeout_function && std::chrono::steady_clock::now() >= timeout_time) {
			auto f = std::move(timeout_function);
			timeout_function = nullptr;
			f();
		}
	}
	template<typename on_new_client_F>
	void run_one(on_new_client_F&& on_new_client) {
		if (timeout_function) {
			while (std::chrono::steady_clock::now() < timeout_time) {
				std::this_thread::sleep_until(timeout_time);
			}
			auto f = std::move(timeout_function);
			timeout_function = nullptr;
			f();
		} else error("sync_server_noop::run_one: can't wait without a timeout");
	}
	template<typename on_new_client_F, typename pred_F>
	void run_until(on_new_client_F&& on_new_client, pred_F&& pred) {
		while (!pred()) {
			run_one(on_new_client);
		}
	}
};

namespace sync_messages {
	enum {
		id_client_uid,
		id_client_frame,
		id_occupy_slot,
		id_start_game,
		id_game_info,
		id_set_race,
		id_game_started,
		id_leave_game
	};
}

struct sync_functions: action_functions {
	sync_state& sync_st;
	explicit sync_functions(state& st, action_state& action_st, sync_state& sync_st) : action_functions(st, action_st), sync_st(sync_st) {}
	
	template<typename action_F>
	void execute_scheduled_actions(action_F&& action_f) {
		for (auto i = sync_st.clients.begin(); i != sync_st.clients.end();) {
			sync_state::client_t* c = &*i;
			++i;
			while (!c->scheduled_actions.empty() && (uint8_t)sync_st.sync_frame == c->scheduled_actions.front().frame) {
				auto act = c->scheduled_actions.front();
				c->scheduled_actions.pop_front();
				c->buffer_begin = act.data_end;
				const uint8_t* data = c->buffer.data();
				if (data + act.data_end > data + c->buffer.size()) error("data beyond end");
				data_loading::data_reader_le r(data + act.data_begin, data + act.data_end);
				if (!action_f(c, r)) break;
			}
		}
	}
	
	void next_frame() = delete;

	template<typename server_T>
	void next_frame(server_T& server) {
		sync(server);
		action_functions::next_frame();
	}

	template<typename server_T>
	void bwapi_compatible_next_frame(server_T& server) {
		if (sync_st.is_first_bwapi_compatible_frame) sync_st.is_first_bwapi_compatible_frame = false;
		else action_functions::next_frame();
		sync(server);
	}

	template<typename reader_T>
	bool schedule_action(sync_state::client_t* client, reader_T&& r) {
		size_t n = r.left();
		auto& buffer = client->buffer;
		auto& buffer_begin = client->buffer_begin;
		auto& buffer_end = client->buffer_end;
		size_t pos = buffer_end;
		size_t new_end = pos + n;
		auto grow_buffer = [&]() {
			const size_t max_size = 1024u * 4 * sync_st.latency;
			size_t new_size = buffer.size() + buffer.size() / 2;
			if (new_size > max_size) new_size = max_size;
			size_t required_size = n;
			for (auto& v : client->scheduled_actions) {
				required_size += v.data_end - v.data_begin;
			}
			if (new_size < required_size) new_size = required_size;
			if (new_size >= max_size) error("action buffer is full for client (%d-%s)", client->local_id, client->uid.str());
			if (new_size >= max_size) return false;
			a_vector<uint8_t> new_buffer(new_size);
			buffer_begin = 0;
			size_t end = 0;
			const uint8_t* src = buffer.data();
			uint8_t* dst = new_buffer.data();
			for (auto& v : client->scheduled_actions) {
				size_t vn = v.data_end - v.data_begin;
				std::memcpy(dst + end, src + v.data_begin, vn);
				v.data_begin = end;
				v.data_end = end + vn;
				end += vn;
			}
			buffer = std::move(new_buffer);
			buffer_end = end;
			return true;
		};
		if (buffer_end < buffer_begin) {
			if (new_end >= buffer_begin) {
				if (!grow_buffer()) return false;
				pos = buffer_end;
				new_end = pos + n;
			}
		} else if (new_end > buffer.size()) {
			if (n < buffer_begin) {
				pos = 0;
				new_end = n;
			} else {
				if (buffer.size() < new_end) {
					if (!grow_buffer()) return false;
					pos = buffer_end;
					new_end = pos + n;
				}
			}
		}
		if (new_end > buffer.size()) error("new_end > buffer.size()");
		if (new_end != pos + n) error("new_end != pos + n");
		for (auto& v : client->scheduled_actions) {
			if (v.data_begin < new_end && v.data_end > pos) error("overlapping waa");
		}
		buffer_end = new_end;
		r.get_bytes(buffer.data() + pos, n);
		a_string str;
		for (size_t i = 0; i != n; ++i) str += format("%02x", (buffer.data() + pos)[i]);
		client->scheduled_actions.push_back({(uint8_t)(client->frame + sync_st.latency), pos, buffer_end});
		return true;
	}
	
	bool schedule_action(sync_state::client_t* client, const uint8_t* data, size_t data_size) {
		data_loading::data_reader_le r(data, data + data_size);
		return schedule_action(client, r);
	}
	
	template<size_t max_size, bool default_little_endian = true>
	struct writer {
		std::array<uint8_t, max_size> arr;
		size_t pos = 0;
		template<typename T, bool little_endian = default_little_endian>
		void put(T v) {
			static_assert(std::is_integral<T>::value, "don't know how to write this type");
			size_t n = pos;
			skip(sizeof(T));
			data_loading::set_value_at<little_endian>(data() + n, v);
		}
		void skip(size_t n) {
			pos += n;
			if (pos > arr.size()) error("sync_functions::writer: attempt to write past end");
		}
		void put_bytes(const uint8_t* src, size_t n) {
			skip(n);
			memcpy(data() + pos - n, src, n);
		}
		size_t size() const {
			return pos;
		}
		const uint8_t* data() const {
			return arr.data();
		}
		uint8_t* data() {
			return arr.data();
		}
	};
	
	template<bool default_little_endian = true>
	struct dynamic_writer {
		std::vector<uint8_t> vec;
		size_t pos = 0;
		dynamic_writer() = default;
		dynamic_writer(size_t initial_size) : vec(initial_size) {}
		template<typename T, bool little_endian = default_little_endian>
		void put(T v) {
			static_assert(std::is_integral<T>::value, "don't know how to write this type");
			size_t n = pos;
			skip(sizeof(T));
			data_loading::set_value_at<little_endian>(data() + n, v);
		}
		void skip(size_t n) {
			pos += n;
			if (pos >= vec.size()) {
				if (vec.size() < 2048) vec.resize(std::max(pos, vec.size() + vec.size()));
				else vec.resize(std::max(pos, std::max(vec.size() + vec.size() / 2, (size_t)32)));
			}
		}
		void put_bytes(const uint8_t* src, size_t n) {
			skip(n);
			memcpy(data() + pos - n, src, n);
		}
		size_t size() const {
			return pos;
		}
		const uint8_t* data() const {
			return vec.data();
		}
		uint8_t* data() {
			return vec.data();
		}
	};
	
	template<typename server_T>
	struct syncer_t {
		sync_functions& funcs;
		server_T& server;
		state& st;
		sync_state& sync_st;
		syncer_t(sync_functions& funcs, server_T& server) : funcs(funcs), server(server), st(funcs.st), sync_st(funcs.sync_st) {}
		
		syncer_t(const syncer_t&) = delete;
		syncer_t& operator=(const syncer_t&) = delete;
		
		~syncer_t() {
			send_leave_game();
			final_sync();
		}
		
		const uint32_t greeting_value = 0x39e25069;
		
		void send(const uint8_t* data, size_t size, const void* h = nullptr) {
			if (size == 0) error("attempt to send no data");
			auto d = server.new_message();
			d.put(data, size);
			server.send_message(d, h);
			if (!h || h == sync_st.local_client) recv(sync_st.local_client, data, size);
		}
		template<typename data_T>
		void send(data_T&& data, const void* h = nullptr) {
			send(data.data(), data.size(), h);
		}
		template<typename reader_T>
		void recv(sync_state::client_t* client, reader_T&& r) {
			auto t = r.tell();
			int id = r.template get<uint8_t>();
			switch (id) {
			case sync_messages::id_client_frame:
				client->frame = r.template get<uint8_t>();
				break;
			case sync_messages::id_client_uid: {
				sync_state::uid_t uid;
				for (auto& v : uid.vals) v = r.template get<uint32_t>();
				if (get_client(uid)) {
					this->kill_client(client);
				} else {
					size_t clients_with_uid = 0;
					for (auto* c : ptr(sync_st.clients)) {
						if (c->has_uid) ++clients_with_uid;
					}
					if (clients_with_uid >= 2) {
						this->kill_client(client);
					} else {
						client->uid = uid;
						client->has_uid = true;
						
						for (int i = 0; i != 12; ++i) {
							st.players[i].controller = sync_st.initial_slot_controllers[i];
							st.players[i].race = sync_st.initial_slot_races[i];
						}
						
						for (auto* c : ptr(sync_st.clients)) {
							c->player_slot = -1;
							clear_scheduled_actions(c);
							c->frame = 0;
						}
						sync_st.sync_frame = 0;
						if (client->h) {
							server.allow_send(client->h, true);
						}
						
						sync_st.clients.sort([&](auto& a, auto& b) {
							return a.uid < b.uid;
						});
					}
				}
				break;
			}
			default:
				if (!client->has_uid) kill_client(client);
				else {
					r.seek(t);
					funcs.schedule_action(client, r);
				}
			}
		}
		
		void recv(sync_state::client_t* client, const uint8_t* data, size_t data_size) {
			data_loading::data_reader_le r(data, data + data_size);
			return recv(client, r);
		}
		
		void send_greeting(const void* h) {
			auto d = server.new_message();
			d.template put<uint32_t>(greeting_value);
			d.template put<uint8_t>(sync_st.sync_frame);
			server.send_message(d, h);
		}
		
		sync_state::client_t* get_client(const sync_state::uid_t& uid) {
			for (auto& c : sync_st.clients) {
				if (c.uid == uid) return &c;
			}
			return nullptr;
		}
		
		auto get_player_left_action(bool player_left) {
			writer<2> w;
			w.put<uint8_t>(87);
			w.put<uint8_t>(player_left ? 0 : 6);
			return w;
		}
		
		void kill_client(sync_state::client_t* client, bool player_left = false) {
			if (client->player_slot != -1) {
				if (sync_st.game_started) {
					auto w = get_player_left_action(player_left);
					data_loading::data_reader_le r(w.data(), w.data() + w.size());
					funcs.read_action(client->player_slot, r);
				} else {
					st.players[client->player_slot].controller = player_t::controller_open;
				}
				client->player_slot = -1;
			}
			if (client == sync_st.local_client) error("attempt to kill local client");
			if (client->h) server.kill_client(client->h);
			for (auto i = sync_st.clients.begin(); i != sync_st.clients.end(); ++i) {
				if (&*i == client) {
					sync_st.clients.erase(i);
					break;
				}
			}
		}
		sync_state::client_t* new_client(const void* h) {
			sync_st.clients.emplace_back();
			sync_st.clients.back().local_id = sync_st.next_client_id++;
			sync_st.clients.back().h = h;
			sync_st.clients.back().last_synced = std::chrono::steady_clock::now();
			return &sync_st.clients.back();
		}
		void send_uid(const void* h) {
			writer<1 + 32> w;
			w.put<uint8_t>(sync_messages::id_client_uid);
			for (auto& v : sync_st.local_client->uid.vals) w.put<uint32_t>(v);
			send(w, h);
		}
		
		void send_start_game() {
			writer<5> w;
			w.put<uint8_t>(sync_messages::id_start_game);
			uint32_t seed = 0;
			for (uint32_t v : sync_state::uid_t::generate().vals) {
				seed ^= v;
			}
			w.put<uint32_t>(seed);
			send(w);
		}
		void send_switch_to_slot(int n) {
			writer<2> w;
			w.put<uint8_t>(sync_messages::id_occupy_slot);
			w.put<uint8_t>(n);
			send(w);
		}
		void send_set_race(race_t race) {
			writer<2> w;
			w.put<uint8_t>(sync_messages::id_set_race);
			w.put<uint8_t>((int)race);
			send(w);
		}
		
		void clear_scheduled_actions(sync_state::client_t* client) {
			client->buffer.clear();
			client->buffer_begin = 0;
			client->buffer_end = 0;
			client->scheduled_actions.clear();
		}
		
		void send_game_info(const void* h) {
			dynamic_writer<> w(0x100);
			w.put<uint8_t>(sync_messages::id_game_info);
			for (sync_state::client_t* c : ptr(sync_st.clients)) {
				if (c->uid == sync_state::uid_t{}) continue;
				w.put<uint8_t>(1);
				for (auto& v : c->uid.vals) w.put<uint32_t>(v);
				w.put<int8_t>(c->player_slot);
				if (c->player_slot == -1) w.put<int8_t>(-1);
				else w.put<int8_t>((int)st.players.at(c->player_slot).race);
				size_t n = std::min(c->name.size(), (size_t)0x20);
				w.put<uint8_t>(n);
				for (size_t i = 0; i != n; ++i) w.put<uint8_t>((uint8_t)*(c->name.data() + i));
			}
			w.put<uint8_t>(0);
			send(w, h);
		}

		void on_new_client(const void* h) {
			if (sync_st.game_started || sync_st.game_starting_countdown) {
				server.kill_client(h);
				return;
			}
			auto* c = new_client(h);
			send_greeting(h);
			send_uid(h);
			auto frame = sync_st.sync_frame;
			sync_st.sync_frame = 0;
			sync_st.sync_frame = frame;
			server.allow_send(h, false);
			server.set_on_message(h, std::bind(&syncer_t::on_message, this, c, std::placeholders::_1, std::placeholders::_2));
			server.set_on_kill(h, std::bind(&syncer_t::kill_client, this, c, false));
		}
		void on_message(sync_state::client_t* client, const void* data, size_t size) {
			data_loading::data_reader_le r((const uint8_t*)data, (const uint8_t*)data + size);
			if (!client->has_greeted) {
				auto v = r.get<uint32_t>();
				if (v != greeting_value) {
					kill_client(client);
				} else client->has_greeted = true;
				return;
			}
			recv(client, (const uint8_t*)data, size);
		}
		void send_client_frame() {
			writer<2> w;
			w.put<uint8_t>(sync_messages::id_client_frame);
			w.put<uint8_t>(sync_st.sync_frame);
			send(w);
		}
		void timeout_func() {
			auto now = std::chrono::steady_clock::now();
			for (auto i = sync_st.clients.begin(); i != sync_st.clients.end();) {
				auto* c = &*i;
				++i;
				if (now - c->last_synced >= std::chrono::seconds(10)) {
					if ((int8_t)(sync_st.sync_frame - c->frame) >= (int8_t)sync_st.latency) {
						kill_client(c);
					}
				}
			}
			server.set_timeout(std::chrono::seconds(1), std::bind(&syncer_t::timeout_func, this));
		}
		
		void send_game_started() {
			writer<1> w;
			w.put<uint8_t>(sync_messages::id_game_started);
			send(w);
		}
		void send_leave_game() {
			if (!sync_st.local_client->game_started) {
				writer<1> w;
				w.put<uint8_t>(sync_messages::id_leave_game);
				send(w);
			} else {
				send(get_player_left_action(true));
			}
		}
		
		void start_game(uint32_t seed) {
			
			a_string seed_str;
			for (auto& v : sync_st.clients) seed_str += v.uid.str();
			uint32_t rand_state = seed ^ data_loading::crc32_t()((const uint8_t*)seed_str.data(), seed_str.size());
			st.lcg_rand_state = rand_state;
			
			for (int i = 0; i != 12; ++i) {
				auto& v = st.players[i];
				if (v.controller == player_t::controller_computer) {
					v.controller = player_t::controller_occupied;
				}
				if (!funcs.player_slot_active(i)) {
					v.controller = player_t::controller_inactive;
				} else {
					if ((int)v.race > 2) v.race = (bwgame::race_t)funcs.lcg_rand(144, 0, 2);
				}
			}
			
			auto slot_available = [&](size_t index) {
				auto c = sync_st.initial_slot_controllers[index];
				if (c == player_t::controller_open) return true;
				if (c == player_t::controller_computer) return true;
				if (c == player_t::controller_rescue_passive) return true;
				if (c == player_t::controller_unused_rescue_active) return true;
				if (c == player_t::controller_neutral) return true;
				return false;
			};
			
			auto randomize_slots = [&](auto pred) {
				bwgame::static_vector<size_t, 12> available_slots;
				for (auto& v : st.players) {
					size_t index = (size_t)(&v - st.players.data());
					if (slot_available(index) && pred(index)) {
						size_t index = (size_t)(&v - st.players.data());
						if (index < 8) available_slots.push_back(index);
					}
				}
				for (size_t i = available_slots.size(); i > 1;) {
					--i;
					size_t old_index = available_slots[i];
					size_t new_index = available_slots[funcs.lcg_rand(33, 0, i)];
					if (old_index == new_index) continue;
					std::swap(st.players[old_index], st.players[new_index]);
					for (auto* c : ptr(sync_st.clients)) {
						if ((int)old_index == c->player_slot) c->player_slot = new_index;
						else if ((int)new_index == c->player_slot) c->player_slot = old_index;
					}
				}
			};
			
			if (sync_st.game_type_melee) {
				randomize_slots([](size_t){return true;});
			} else {
				for (int force = 1; force <= 4; ++force) {
					randomize_slots([&](size_t index){return st.players.at(index).force == force;});
				}
			}
			sync_st.game_started = true;
			
			sync_st.clients.sort([&](auto& a, auto& b) {
				if ((unsigned)a.player_slot != (unsigned)b.player_slot) return (unsigned)a.player_slot < (unsigned)b.player_slot;
				return a.uid < b.uid;
			});
			send_game_started();
			
		}
		
		void process_messages() {
			
			if (sync_st.game_starting_countdown) {
				--sync_st.game_starting_countdown;
				if (sync_st.game_starting_countdown == 0) {
					start_game(sync_st.start_game_seed);
				}
			}
			
			if (sync_st.game_started) {
				funcs.execute_scheduled_actions([this](sync_state::client_t* client, auto& r) {
					if (client->game_started) {
						if (client->player_slot != -1) {
							funcs.read_action(client->player_slot, r);
							if (st.players.at(client->player_slot).controller != player_t::controller_occupied) {
								if (client != sync_st.local_client) this->kill_client(client);
								else this->clear_scheduled_actions(client);
								return false;
							}
						}
					} else {
						int id = r.template get<uint8_t>();
						if (id == sync_messages::id_game_started) {
							client->game_started = true;
						}
					}
					return true;
				});
			} else {
				funcs.execute_scheduled_actions([this](sync_state::client_t* client, auto& r) {
					int id = r.template get<uint8_t>();
					switch (id) {
					case sync_messages::id_game_info:
						while (r.template get<uint8_t>() != 0) {
							sync_state::uid_t uid;
							for (auto& v : uid.vals) v = r.template get<uint32_t>();
							sync_state::client_t* c = this->get_client(uid);
							if (!c) {
								c = this->new_client(nullptr);
								c->uid = uid;
							}
							c->player_slot = r.template get<int8_t>();
							if (c->player_slot < 0 || c->player_slot >= 12) c->player_slot = -1;
							int race = r.template get<int8_t>();
							if (c->player_slot != -1) {
								for (auto* c2 : ptr(sync_st.clients)) {
									if (c != c2 && c2->player_slot == c->player_slot) c2->player_slot = -1;
								}
								st.players[c->player_slot].controller = player_t::controller_occupied;
								st.players[c->player_slot].race = (bwgame::race_t)race;
							}
							size_t n = r.template get<uint8_t>();
							c->name.resize(std::min(n, (size_t)0x20));
							for (size_t i = 0; i != n; ++i) {
								if (i < c->name.size()) c->name[i] = (char)r.template get<uint8_t>();
							}
						}
						break;
					case sync_messages::id_occupy_slot: {
						int n = r.template get<uint8_t>();
						for (int i = 0; i != 12; ++i) {
							if (i == n && st.players[i].controller == player_t::controller_open) {
								race_t race = sync_st.initial_slot_races[i];
								if (client->player_slot != -1) {
									race = st.players[client->player_slot].race;
									st.players[client->player_slot].controller = player_t::controller_open;
									st.players[client->player_slot].race = sync_st.initial_slot_races[client->player_slot];
								}
								client->player_slot = i;
								st.players[i].controller = player_t::controller_occupied;
								if (sync_st.initial_slot_races[i] == (bwgame::race_t)5) {
									st.players[i].race = race;
								}
								break;
							}
						}
						break;
					}
					case sync_messages::id_set_race: {
						race_t race = (race_t)r.template get<uint8_t>();
						if (client->player_slot != -1) {
							if (sync_st.initial_slot_races[client->player_slot] == (bwgame::race_t)5) {
								st.players[client->player_slot].race = race;
							}
						}
						break;
					}
					case sync_messages::id_start_game:
						if (!sync_st.game_starting_countdown) {
							sync_st.game_starting_countdown = 10;
							sync_st.start_game_seed = r.template get<uint32_t>();
						}
						break;
					case sync_messages::id_leave_game:
						if (client != sync_st.local_client) this->kill_client(client);
						else this->clear_scheduled_actions(client);
						return false;
					default: error("unknown pre game message id %d", id);
					}
					return true;
				});
			}
		}
		
		void sync_next_frame() {
			if (!sync_st.has_initialized) {
				sync_st.has_initialized = true;
				for (int i = 0; i != 12; ++i) {
					sync_st.initial_slot_races[i] = st.players[i].race;
					sync_st.initial_slot_controllers[i] = st.players[i].controller;
				}
			}
			++sync_st.sync_frame;
			send_client_frame();
		}
		
		bool all_clients_in_sync() {
			for (auto* c : ptr(sync_st.clients)) {
				if ((int8_t)(sync_st.sync_frame - c->frame) >= (int8_t)sync_st.latency) {
					return false;
				}
			}
			return true;
		}

		void sync() {
			sync_next_frame();
			
			server.poll(std::bind(&syncer_t::on_new_client, this, std::placeholders::_1));
			
			auto pred = [this]() {
				return all_clients_in_sync();
			};
			
			if (!sync_st.game_started && !sync_st.game_starting_countdown && pred()) {
				auto any_scheduled_actions = [&]() {
					for (auto& c : sync_st.clients) {
						if (!c.scheduled_actions.empty()) return true;
					}
					return false;
				};
				bool timed_out = false;
				server.set_timeout(std::chrono::milliseconds(50), [&]{
					timed_out = true;
				});
				while (!any_scheduled_actions() && !timed_out) {
					server.run_one(std::bind(&syncer_t::on_new_client, this, std::placeholders::_1));
				}
				if (!pred()) {
					server.set_timeout(std::chrono::seconds(1), std::bind(&syncer_t::timeout_func, this));
					server.run_until(std::bind(&syncer_t::on_new_client, this, std::placeholders::_1), pred);
				}
			} else {
				server.set_timeout(std::chrono::seconds(1), std::bind(&syncer_t::timeout_func, this));
				server.run_until(std::bind(&syncer_t::on_new_client, this, std::placeholders::_1), pred);
			}
			
			auto now = std::chrono::steady_clock::now();
			for (auto& c : sync_st.clients) {
				c.last_synced = now;
			}
			
			process_messages();
		}
		
		void final_sync() {
			bool timed_out = false;
			server.set_timeout(std::chrono::milliseconds(250), [&]{
				timed_out = true;
			});
			while (!sync_st.local_client->scheduled_actions.empty() && !timed_out) {
				sync_next_frame();
				server.poll(std::bind(&syncer_t::on_new_client, this, std::placeholders::_1));
				while (!all_clients_in_sync() && !timed_out) {
					server.run_one(std::bind(&syncer_t::on_new_client, this, std::placeholders::_1));
				}
				if (!timed_out) process_messages();
			}
		}
	};
	
	struct syncer_container_t {
		static const size_t size = 0x40;
		static const size_t alignment = alignof(std::max_align_t);
		const std::type_info* type = nullptr;
		std::aligned_storage<size, alignment>::type obj;
		void (syncer_container_t::* destroy_f)();
		
		syncer_container_t() = default;
		syncer_container_t(const syncer_container_t&) = delete;
		syncer_container_t& operator=(const syncer_container_t&) = delete;
		~syncer_container_t() {
			destroy();
		}
		
		void destroy() {
			if (type) (this->*destroy_f)();
		}
		
		template<typename T, typename... args_T>
		void construct(args_T&&... args) {
			static_assert(sizeof(T) <= size || alignof(T) <= alignment, "syncer_container_t size or alignment too small");
			new ((T*)&obj) T(std::forward<args_T>(args)...);
			type = &typeid(T);
			destroy_f = &syncer_container_t::destroy<T>;
		}
		template<typename T>
		void destroy() {
			as<T>().~T();
			type = nullptr;
		}
		template<typename T>
		T& as() {
			static_assert(sizeof(T) <= size || alignof(T) <= alignment, "syncer_container_t size or alignment too small");
			return (T&)obj;
		}
		
		template<typename T, typename... args_T>
		T& try_emplace(args_T&&... args) {
			if (type) {
				if (type == &typeid(T) || *type == typeid(T)) return as<T>();
				(this->*destroy_f)();
			}
			construct<T>(std::forward<args_T>(args)...);
			return as<T>();
		}
	};
	
	syncer_container_t syncer_container;
	
	template<typename server_T>
	syncer_t<server_T>& get_syncer(server_T& server) {
		return syncer_container.try_emplace<syncer_t<server_T>>(*this, server);
	}
	
	template<typename server_T>
	void sync(server_T& server) {
		get_syncer<server_T>(server).sync();
	}
	
	template<typename server_T>
	void start_game(server_T& server) {
		if (sync_st.game_started) return;
		get_syncer<server_T>(server).send_start_game();
	}
	
	template<typename server_T>
	void switch_to_slot(server_T& server, int n) {
		if (sync_st.game_started) return;
		get_syncer<server_T>(server).send_switch_to_slot(n);
	}
	
	template<typename server_T>
	void set_local_client_name(server_T& server, a_string name) {
		if (sync_st.game_started) return;
		sync_st.local_client->name = std::move(name);
	}
	
	template<typename server_T>
	void set_local_client_race(server_T& server, race_t race) {
		if (sync_st.game_started) return;
		get_syncer<server_T>(server).send_set_race(race);
	}
	
	template<typename server_T>
	void input_action(server_T& server, const uint8_t* data, size_t size) {
		get_syncer<server_T>(server).send(data, size);
	}
	
	void leave_game() {
		syncer_container.destroy();
	}

};


}

#endif
