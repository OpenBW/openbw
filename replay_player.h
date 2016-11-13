#ifndef BWGAME_REPLAY_PLAYER_H
#define BWGAME_REPLAY_PLAYER_H

#include "actions.h"
#include "data_loading.h"
#include "bwgame.h"

namespace bwgame {

namespace data_loading {

struct crc32 {
	std::array<uint32_t, 256> table;
	crc32() {
		for (size_t i = 0; i != 256; ++i) {
			uint32_t v = i;
			for (size_t b = 0; b != 8; ++b) {
				v = (v >> 1) ^ (v & 1 ? 0xedb88320 : 0);
			}
			table[i] = v;
		}
	}
	uint32_t operator()(uint8_t* data, size_t data_size) {
		uint32_t r = 0xffffffff;
		uint8_t* end = data + data_size;
		for (; data != end; ++data) {
			r = (r >> 8) ^ table[(r ^ *data) & 0xff];
		}
		return r;
	}
};

template<typename base_reader_T, bool default_little_endian = true>
struct replay_file_reader {
	crc32 crc32;
	base_reader_T& r;
	replay_file_reader(base_reader_T& r) : r(r) {
	}

	void get_bytes(uint8_t* output, size_t output_size) {
		uint32_t crc32_sum = r.get<uint32_t>();
		size_t segments = r.get<uint32_t>();
		
		a_vector<uint8_t> compressed_data;
		
		size_t output_pos = 0;
		for (size_t i = 0; i != segments; ++i) {
			size_t segment_input_size = r.get<uint32_t>();
			
			size_t segment_output_size = output_size - output_pos;
			if (segment_output_size > 8192) segment_output_size = 8192;
			
			if (segment_input_size > segment_output_size) xcept("replay_file_reader: output buffer too small");
			if (segment_input_size == segment_output_size) {
				r.get_bytes(output + output_pos, segment_input_size);
			} else {
				compressed_data.resize(segment_input_size);
				r.get_bytes(compressed_data.data(), segment_input_size);
				decompress(compressed_data.data(), segment_input_size, output + output_pos, segment_output_size);
			}
			output_pos += segment_output_size;
		}
		
		if (output_pos != output_size) xcept("replay_file_reader: read %d bytes, expected %d", output_pos, output_size);
		
		uint32_t calculcated_crc32_sum = crc32(output, output_size);
		if (calculcated_crc32_sum != crc32_sum) xcept("replay_file_reader: crc32 mismatch: got %08x, expected %08x", calculcated_crc32_sum, crc32_sum);
	}

	template<typename T, bool little_endian = default_little_endian>
	T get() {
		return get_impl<T, little_endian>(*this);
	}
};

template<typename base_reader_T>
auto make_replay_file_reader(base_reader_T& reader) {
	return replay_file_reader<base_reader_T>(reader);
}

}

struct replay_player: actions_player {
	int end_frame = 0;
	replay_player() = default;
	explicit replay_player(state& st) : actions_player(st) {}
	explicit replay_player(game_player& player) : actions_player(player.st()) {}
	void load_replay_file(a_string filename, bool initial_processing = true) {
		auto file_r = data_loading::file_reader<>(std::move(filename));
		load_replay(data_loading::make_replay_file_reader(file_r), initial_processing);
	}
	void load_replay_data(uint8_t* data, size_t data_size, bool initial_processing = true) {
		load_replay(data_loading::data_reader_le(data, data + data_size), initial_processing);
	}
	template<typename reader_T>
	void load_replay(reader_T&& r, bool initial_processing = true) {
		
		uint32_t identifier = r.template get<uint32_t>();
		if (identifier != 0x53526572) xcept("replay_player: invalid identifier %#x", identifier);
		
		std::array<uint8_t, 633> game_info_buffer;
		r.get_bytes(game_info_buffer.data(), game_info_buffer.size());
		
		data_loading::data_reader_le gir(game_info_buffer.data(), game_info_buffer.data() + game_info_buffer.size());
		
		gir.get<uint8_t>(); // is broodwar
		auto frame_count = gir.get<uint32_t>();
		gir.get<uint16_t>(); // campaign id
		gir.get<uint8_t>(); // command byte ?
		uint32_t random_seed = gir.get<uint32_t>();
		gir.get<std::array<uint8_t, 8>>(); // player bytes ?
		gir.get<uint32_t>(); // ?
		auto player_name = gir.get<std::array<char, 24>>();
		gir.get<uint32_t>(); // game flags?
		gir.get<uint16_t>(); // map width
		gir.get<uint16_t>(); // map height
		gir.get<uint8_t>(); // active player acount
		gir.get<uint8_t>(); // slot count
		gir.get<uint8_t>(); // game speed
		gir.get<uint8_t>(); // game state ?
		gir.get<uint16_t>(); // game type ?
		gir.get<uint16_t>(); // game sub type ?
		gir.get<uint32_t>(); // ?
		gir.get<uint16_t>(); // tileset
		gir.get<uint8_t>(); // replay autosaved
		gir.get<uint8_t>(); // computer player count?
		auto game_name = gir.get<std::array<char, 25>>();
		auto map_name = gir.get<std::array<char, 32>>();
		gir.get<uint16_t>(); // game type ?
		gir.get<uint16_t>(); // game sub type ?
		gir.get<uint16_t>(); // sub type display ?
		gir.get<uint16_t>(); // sub type label ?
		gir.get<uint8_t>(); // victory condition
		gir.get<uint8_t>(); // resource type
		gir.get<uint8_t>(); // use standard unit stats
		gir.get<uint8_t>(); // fog of war enabled
		gir.get<uint8_t>(); // ums units enabled
		gir.get<uint8_t>(); // use fixed positions ?
		gir.get<uint8_t>(); // restriction flags ?
		gir.get<uint8_t>(); // allies enabled
		gir.get<uint8_t>(); // teams enabled
		gir.get<uint8_t>(); // cheats enabled
		gir.get<uint8_t>(); // tournament mode ?
		gir.get<uint32_t>(); // victory condition value?
		gir.get<uint32_t>(); // starting minerals
		gir.get<uint32_t>(); // starting gas
		gir.get<uint8_t>(); // ?
		
		(void)player_name;
		(void)game_name;
		(void)map_name;
		
		std::array<int, 12> player_id;
		std::array<int, 12> controller;
		std::array<int, 12> race;
		std::array<int, 12> force;
		
		for (size_t i = 0; i != 12; ++i) {
			size_t slot = gir.get<uint32_t>(); // slot ?
			if (slot >= 12) xcept("replay_player: invalid slot %u", slot);
			player_id[slot] = gir.get<uint32_t>(); // player id ?
			controller[slot] = gir.get<uint8_t>(); // controller
			race[slot] = gir.get<uint8_t>(); // race
			force[slot] = gir.get<uint8_t>(); // force
			gir.get<std::array<char, 25>>(); // player name
			
			funcs().action_st.player_id[slot] = player_id[slot];
		}
		
		gir.get<std::array<uint32_t, 8>>(); // player colors
		gir.get<std::array<uint8_t, 8>>(); // player force ?
		
		end_frame = frame_count;
		
		actions_data_buffer.resize(r.template get<uint32_t>());
		r.get_bytes(actions_data_buffer.data(), actions_data_buffer.size());
		set_actions_data_buffer();
		
		a_vector<uint8_t> map_buffer;
		map_buffer.resize(r.template get<uint32_t>());
		r.get_bytes(map_buffer.data(), map_buffer.size());
		
		game_load_functions game_load_funcs(st());
		game_load_funcs.load_map_data(map_buffer.data(), map_buffer.size(), [&]() {
			for (size_t i = 0; i != 12; ++i) {
				st().players[i].controller = controller[i];
				st().players[i].race = race[i];
				st().players[i].force = force[i];
			}
			st().lcg_rand_state = random_seed;
		});
		
		if (initial_processing) {
			funcs().process_frame();
			funcs().process_frame();
		}
	}
	
	void next_frame() {
		if (st().current_frame == end_frame) xcept("replay_player: attempt to play past end");
		actions_player::next_frame();
	}
	
	bool is_done() {
		return st().current_frame == end_frame;
	}
	
};

}

#endif
