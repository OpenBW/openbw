#ifndef BWGAME_REPLAY_SAVER_H
#define BWGAME_REPLAY_SAVER_H

#include "bwgame.h"
#include "replay.h"

namespace bwgame {

namespace data_loading {

template<bool default_little_endian = true, bool bounds_checking = true>
struct data_writer {
	uint8_t* ptr = nullptr;
	uint8_t* begin = nullptr;
	const uint8_t* end = nullptr;
	data_writer() = default;
	data_writer(uint8_t* ptr, const uint8_t* end) : ptr(ptr), begin(ptr), end(end) {}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "don't know how to write this type");
		size_t n = size();
		skip(sizeof(T));
		data_loading::set_value_at<little_endian>(data() + n, v);
	}
	void skip(size_t n) {
		if (bounds_checking && left() < n) error("data_writer: attempt to write past end");
		ptr += n;
	}
	void put_bytes(const uint8_t* src, size_t n) {
		size_t pos = size();
		skip(n);
		memcpy(data() + pos, src, n);
	}
	size_t size() const {
		return ptr - begin;
	}
	const uint8_t* data() const {
		return begin;
	}
	uint8_t* data() {
		return begin;
	}
	size_t left() const {
		return end - ptr;
	}
};

template<typename dst_T, bool default_little_endian = true>
struct vector_writer {
	dst_T& dst;
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "don't know how to write this type");
		size_t n = dst.size();
		skip(sizeof(T));
		data_loading::set_value_at<little_endian>(data() + n, v);
	}
	void skip(size_t n) {
		if (left() < n) error("data_writer: attempt to write past end");
		dst.resize(dst.size() + n);
	}
	void put_bytes(const uint8_t* src, size_t n) {
		size_t pos = dst.size();
		skip(n);
		memcpy(data() + pos, src, n);
	}
	size_t size() const {
		return dst.size();
	}
	const uint8_t* data() const {
		return dst.data();
	}
	uint8_t* data() {
		return dst.data();
	}
	size_t left() const {
		return dst.capacity() - dst.size();
	}
};

template<typename buffers_T, bool default_little_endian = true>
struct buffers_writer {
	buffers_T& buffers;
	buffers_writer(buffers_T& buffers) : buffers(buffers) {}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "data_writer: don't know how to write this type");
		put_bytes((const uint8_t*)&v, sizeof(v));
	}
	void put_bytes(const uint8_t* src, size_t n) {
		if (buffers.empty()) buffers.emplace_back();
		auto& buf = buffers.back();
		size_t pos = buf.size();
		size_t left = buf.capacity() - pos;
		if (left >= n) {
			buf.resize(pos + n);
			memcpy(buf.data() + pos, src, n);
		} else {
			buf.resize(pos + left);
			memcpy(buf.data() + pos, src, left);
			buffers.emplace_back();
			put_bytes(src + left, n - left);
		}
	}
};

template<typename buffers_T, bool default_little_endian = true>
auto get_buffers_writer(buffers_T& buffers) {
	return buffers_writer<buffers_T, default_little_endian>(buffers);
}

template<typename base_writer_T, bool default_little_endian = true>
struct replay_file_writer {
	crc32_t crc32;
	base_writer_T& w;
	replay_file_writer(base_writer_T& w) : w(w) {
	}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "data_writer: don't know how to write this type");
		put_bytes((const uint8_t*)&v, sizeof(v));
	}
	void put_bytes(const uint8_t* data, size_t size) {
		w.template put<uint32_t>(crc32(data, size));
		size_t segments = (size + 8191) / 8192;
		w.template put<uint32_t>(segments);
		
		size_t output_pos = 0;
		for (size_t i = 0; i != segments; ++i) {
			size_t segment_output_size = size - output_pos;
			if (segment_output_size > 8192) segment_output_size = 8192;
			w.template put<uint32_t>(segment_output_size);
			w.put_bytes(data + output_pos, segment_output_size);
			output_pos += segment_output_size;
		}
		
		if (output_pos != size) error("replay_file_writer: wrote %d bytes, expected %d", output_pos, size);
	}
};

template<typename base_reader_T>
auto make_replay_file_writer(base_reader_T& reader) {
	return replay_file_writer<base_reader_T>(reader);
}

template<bool default_little_endian = true>
struct file_writer {
	a_string filename;
	FILE* f = nullptr;
	file_writer() = default;
	explicit file_writer(a_string filename) {
		open(std::move(filename));
	}
	~file_writer() {
		if (f) fclose(f);
	}
	file_writer(const file_writer&) = delete;
	file_writer(file_writer&& n) {
		f = n.f;
		n.f = nullptr;
	}
	file_writer& operator=(const file_writer&) = delete;
	file_writer& operator=(file_writer&& n) {
		std::swap(f, n.f);
		return *this;
	}
	void open(a_string filename) {
		if (f) fclose(f);
		f = fopen(filename.c_str(), "wb");
		if (!f) error("file_writer: failed to open %s for writing", filename.c_str());
		this->filename = std::move(filename);
	}
	void put_bytes(const uint8_t* src, size_t n) {
		if (!fwrite(src, n, 1, f)) {
			error("file_writer: %s: write error", filename);
		}
	}
	template<typename T, bool little_endian = default_little_endian>
	void put(T v) {
		static_assert(std::is_integral<T>::value, "data_writer: don't know how to write this type");
		put_bytes((const uint8_t*)&v, sizeof(v));
	}
	void seek(size_t offset) {
		if ((size_t)(long)offset != offset || fseek(f, (long)offset, SEEK_SET)) error("file_writer: %s: failed to seek to offset %d", filename, offset);
	}
	bool eof() {
		return feof(f);
	}
	size_t left() const {
		return size() - tell();
	}
	size_t tell() const {
		return (size_t)ftell(f);
	}
	size_t size() {
		auto prev_pos = ftell(f);
		fseek(f, 0, SEEK_END);
		auto r = ftell(f);
		fseek(f, prev_pos, SEEK_SET);
		return r;
	}
};

}

struct replay_saver_state {
	a_deque<static_vector<uint8_t, 0x10000>> history;
	int current_history_frame = -1;
	size_t current_actions_size = 0;
	size_t current_actions_size_index = 0;
	size_t current_actions_size_offset = 0;
	
	const uint8_t* map_data = nullptr;
	size_t map_data_size = 0;
	
	uint32_t random_seed = 42;
	a_string player_name;
	
	size_t map_tile_width = 0;
	size_t map_tile_height = 0;
	int active_player_count = 0;
	int slot_count = 0;
	int game_speed = 3;
	int game_type = 0;
	int tileset = 0;
	
	a_string game_name;
	a_string map_name;
	game_load_functions::setup_info_t setup_info;
	std::array<player_t, 12> players;
	std::array<a_string, 12> player_names;
	
};

struct replay_saver_functions {
	replay_saver_state& replay_saver_st;
	explicit replay_saver_functions(replay_saver_state& replay_saver_st) : replay_saver_st(replay_saver_st) {}
	
	bool add_action(int current_frame, int owner, const uint8_t* data, size_t data_size) {
		auto w = data_loading::get_buffers_writer(replay_saver_st.history);
		if (current_frame != replay_saver_st.current_history_frame || replay_saver_st.current_actions_size + data_size >= 0x100) {
			replay_saver_st.current_history_frame = current_frame;
			replay_saver_st.current_actions_size = 1 + data_size;
			w.template put<uint32_t>(current_frame);
			if (data_size >= 0x100) error("replay_saver_functions::add_action: data_size (%d) > 0x100", data_size);
			w.template put<uint8_t>(1 + data_size);
			replay_saver_st.current_actions_size_index = replay_saver_st.history.size() - 1;
			replay_saver_st.current_actions_size_offset = replay_saver_st.history.back().size() - 1;
		} else {
			replay_saver_st.current_actions_size += 1 + data_size;
			replay_saver_st.history.at(replay_saver_st.current_actions_size_index).at(replay_saver_st.current_actions_size_offset) = replay_saver_st.current_actions_size;
		}
		w.template put<uint8_t>(owner);
		w.put_bytes(data, data_size);
	}
	
	template<typename writer_T>
	void save_replay(int current_frame, writer_T& w) {
		std::array<uint8_t, 633> game_info_buffer;
		data_loading::data_writer<> giw(game_info_buffer.data(), game_info_buffer.data() + game_info_buffer.size());
		
		auto put_string = [&](const a_string& str, size_t max_size) {
			size_t i = 0;
			for (; i != str.size() && i != max_size; ++i) giw.put<uint8_t>(str[i]);
			for (; i != max_size; ++i) giw.put<uint8_t>(0);
		};
		
		giw.put<uint8_t>(1); // is broodwar
		giw.put<uint32_t>(current_frame); // frame count
		giw.put<uint16_t>(0); // campaign id
		giw.put<uint8_t>(0); // command byte ?
		giw.put<uint32_t>(replay_saver_st.random_seed);
		for (size_t i = 0; i != 8; ++i) giw.put<uint8_t>(0); // player bytes ?
		giw.put<uint32_t>(0); // ?
		put_string(replay_saver_st.player_name, 24);
		giw.put<uint32_t>(0); // game flags?
		giw.put<uint16_t>(replay_saver_st.map_tile_width); // map width
		giw.put<uint16_t>(replay_saver_st.map_tile_height); // map height
		giw.put<uint8_t>(replay_saver_st.active_player_count); // active player acount
		giw.put<uint8_t>(replay_saver_st.slot_count); // slot count
		giw.put<uint8_t>(replay_saver_st.game_speed); // game speed
		giw.put<uint8_t>(0); // game state ?
		giw.put<uint16_t>(replay_saver_st.game_type); // game type ?
		giw.put<uint16_t>(0); // game sub type ?
		giw.put<uint32_t>(0); // ?
		giw.put<uint16_t>(replay_saver_st.tileset); // tileset
		giw.put<uint8_t>(1); // replay saved
		giw.put<uint8_t>(0); // computer player count?
		put_string(replay_saver_st.game_name, 25);
		put_string(replay_saver_st.map_name, 32);
		giw.put<uint16_t>(replay_saver_st.game_type); // game type ?
		giw.put<uint16_t>(0); // game sub type ?
		giw.put<uint16_t>(0); // sub type display ?
		giw.put<uint16_t>(0); // sub type label ?
		giw.put<uint8_t>(replay_saver_st.setup_info.victory_condition); // victory condition
		giw.put<uint8_t>(replay_saver_st.setup_info.resource_type); // resource type
		giw.put<uint8_t>(1); // use standard unit stats
		giw.put<uint8_t>(2); // fog of war enabled
		giw.put<uint8_t>(replay_saver_st.setup_info.starting_units); // create initial units
		giw.put<uint8_t>(0); // use fixed positions ?
		giw.put<uint8_t>(0); // restriction flags ?
		giw.put<uint8_t>(0); // allies enabled
		giw.put<uint8_t>(0); // teams enabled
		giw.put<uint8_t>(0); // cheats enabled
		giw.put<uint8_t>(replay_saver_st.setup_info.tournament_mode); // tournament mode ?
		giw.put<uint32_t>(0); // victory condition value?
		giw.put<uint32_t>(replay_saver_st.setup_info.starting_minerals); // starting minerals
		giw.put<uint32_t>(0); // starting gas
		giw.put<uint8_t>(0); // ?
		
		for (size_t i = 0; i != 12; ++i) {
			giw.put<uint32_t>(i); // slot ?
			giw.put<uint32_t>(i); // player id
			giw.put<uint8_t>(replay_saver_st.players[i].controller); // controller
			giw.put<uint8_t>((int)replay_saver_st.players[i].race); // race
			giw.put<uint8_t>(replay_saver_st.players[i].force); // force
			put_string(replay_saver_st.player_names[i], 25);
		}
		
		for (size_t i = 0; i != 8; ++i) {
			giw.put<uint32_t>(replay_saver_st.players[i].color);
		}
		for (size_t i = 0; i != 8; ++i) {
			giw.put<uint8_t>(0); // create_melee_units_for_player
		}
		
		auto rw = data_loading::make_replay_file_writer(w);
		
		rw.template put<uint32_t>(0x53526572);
		rw.put_bytes(game_info_buffer.data(), game_info_buffer.size());
		
		size_t history_size = 0;
		for (auto& v : replay_saver_st.history) history_size += v.size();
		rw.template put<uint32_t>(history_size);
		a_vector<uint8_t> tmp_buf;
		tmp_buf.reserve(history_size);
		for (auto& v : replay_saver_st.history) {
			tmp_buf.insert(tmp_buf.end(), v.begin(), v.end());
		}
		rw.put_bytes(tmp_buf.data(), tmp_buf.size());
		
		if (!replay_saver_st.map_data) error("replay_saver_functions::save_replay: replay_saver_state::map_data is null");
		rw.template put<uint32_t>(replay_saver_st.map_data_size);
		rw.put_bytes(replay_saver_st.map_data, replay_saver_st.map_data_size);
		
	}
	
};


}

#endif
