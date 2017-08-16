#ifndef BWGAME_REPLAY_H
#define BWGAME_REPLAY_H

#include "actions.h"
#include "data_loading.h"
#include "korean.h"
#include "bwgame.h"

namespace bwgame {

namespace data_loading {

struct crc32_t {
	std::array<uint32_t, 256> table;
	crc32_t() {
		for (uint32_t i = 0; i != 256; ++i) {
			uint32_t v = i;
			for (size_t b = 0; b != 8; ++b) {
				v = (v >> 1) ^ (v & 1 ? 0xedb88320 : 0);
			}
			table[i] = v;
		}
	}
	uint32_t operator()(const uint8_t* data, size_t data_size) {
		uint32_t r = 0xffffffff;
		const uint8_t* end = data + data_size;
		for (; data != end; ++data) {
			r = (r >> 8) ^ table[(r ^ *data) & 0xff];
		}
		return r;
	}
};

template<typename base_reader_T, bool default_little_endian = true>
struct replay_file_reader {
	crc32_t crc32;
	base_reader_T& r;
	replay_file_reader(base_reader_T& r) : r(r) {
	}

	void get_bytes(uint8_t* output, size_t output_size) {
		uint32_t crc32_sum = r.template get<uint32_t>();
		size_t segments = r.template get<uint32_t>();
		
		a_vector<uint8_t> compressed_data;

		size_t output_pos = 0;
		for (size_t i = 0; i != segments; ++i) {
			size_t segment_input_size = r.template get<uint32_t>();
			
			size_t segment_output_size = output_size - output_pos;
			if (segment_output_size > 8192) segment_output_size = 8192;
			
			if (segment_input_size > segment_output_size) error("replay_file_reader: output buffer too small");
			if (segment_input_size == segment_output_size) {
				r.get_bytes(output + output_pos, segment_input_size);
			} else {
				compressed_data.resize(segment_input_size);
				r.get_bytes(compressed_data.data(), segment_input_size);
				decompress(compressed_data.data(), segment_input_size, output + output_pos, segment_output_size);
			}
			output_pos += segment_output_size;
		}
		
		if (output_pos != output_size) error("replay_file_reader: read %d bytes, expected %d", output_pos, output_size);
		
		uint32_t calculcated_crc32_sum = crc32(output, output_size);
		if (calculcated_crc32_sum != crc32_sum) error("replay_file_reader: crc32 mismatch: got %08x, expected %08x", calculcated_crc32_sum, crc32_sum);
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

struct replay_state {
	a_vector<uint8_t> actions_data_buffer;
	int end_frame = 0;
	a_string map_name;
	std::array<a_string, 12> player_name;
	int game_type = 0;
};

struct replay_functions: action_functions {
	replay_state& replay_st;
	explicit replay_functions(state& st, action_state& action_st, replay_state& replay_st) : action_functions(st, action_st), replay_st(replay_st) {}
	
	void load_replay_file(a_string filename, bool initial_processing = true, std::vector<uint8_t>* get_map_data = nullptr) {
		auto file_r = data_loading::file_reader<>(std::move(filename));
		load_replay(data_loading::make_replay_file_reader(file_r), initial_processing, get_map_data);
	}
	void load_replay_data(const uint8_t* data, size_t data_size, bool initial_processing = true, std::vector<uint8_t>* get_map_data = nullptr) {
		auto r = data_loading::data_reader_le(data, data + data_size);
		load_replay(data_loading::make_replay_file_reader(r), initial_processing, get_map_data);
	}
	template<typename reader_T>
	void load_replay(reader_T&& r, bool initial_processing = true, std::vector<uint8_t>* get_map_data = nullptr) {
		
		uint32_t identifier = r.template get<uint32_t>();
		if (identifier != 0x53526572) error("load_replay: invalid identifier %#x", identifier);

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
		auto game_type = gir.get<uint16_t>(); // game type ?
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
		auto victory_condition = gir.get<uint8_t>(); // victory condition
		int resource_type = gir.get<uint8_t>(); // resource type
		gir.get<uint8_t>(); // use standard unit stats
		gir.get<uint8_t>(); // fog of war enabled
		auto create_initial_units = gir.get<uint8_t>();
		gir.get<uint8_t>(); // use fixed positions ?
		gir.get<uint8_t>(); // restriction flags ?
		gir.get<uint8_t>(); // allies enabled
		gir.get<uint8_t>(); // teams enabled
		gir.get<uint8_t>(); // cheats enabled
		auto tournament_mode = gir.get<uint8_t>(); // tournament mode ?
		gir.get<uint32_t>(); // victory condition value?
		auto starting_minerals = gir.get<uint32_t>(); // starting minerals
		gir.get<uint32_t>(); // starting gas
		gir.get<uint8_t>(); // ?
		
		(void)player_name;
		(void)game_name;
		
		auto arr_str = [&](auto& str) {
			a_string r;
			for (auto& v : str) {
				if (!v) break;
				if ((unsigned char)v >= 21) r += v;
			}
			return r;
		};
		replay_st.map_name = arr_str(map_name);
		a_string kn;
		if (korean::korean_locale_to_utf8(replay_st.map_name, kn)) replay_st.map_name = kn;
		
		std::array<int, 12> slot_player_id;
		std::array<int, 12> slot_controller;
		std::array<int, 12> slot_race;
		std::array<int, 12> slot_force;
		
		for (size_t i = 0; i != 12; ++i) {
			gir.get<uint32_t>(); // slot ?
			slot_player_id[i] = gir.get<uint32_t>(); // player id
			slot_controller[i] = gir.get<uint8_t>(); // controller
			slot_race[i] = gir.get<uint8_t>(); // race
			slot_force[i] = gir.get<uint8_t>(); // force
			auto name = gir.get<std::array<char, 25>>(); // player name
			replay_st.player_name[i] = arr_str(name);
			
			action_st.player_id[i] = slot_player_id[i];
		}
		
		auto player_color = gir.get<std::array<uint32_t, 8>>(); // player colors
		auto create_melee_units_for_player = gir.get<std::array<uint8_t, 8>>();
		
		replay_st.end_frame = frame_count;
		replay_st.game_type = game_type;
		
		replay_st.actions_data_buffer.resize(r.template get<uint32_t>());
		r.get_bytes(replay_st.actions_data_buffer.data(), replay_st.actions_data_buffer.size());
		
		a_vector<uint8_t> map_buffer;
		map_buffer.resize(r.template get<uint32_t>());
		r.get_bytes(map_buffer.data(), map_buffer.size());

		if (get_map_data) *get_map_data = map_buffer;
		
		game_load_functions game_load_funcs(st);
		game_load_funcs.load_map_data(map_buffer.data(), map_buffer.size(), [&]() {
			game_load_funcs.setup_info.victory_condition = victory_condition;
			game_load_funcs.setup_info.starting_units = create_initial_units;
			game_load_funcs.setup_info.tournament_mode = tournament_mode;
			game_load_funcs.setup_info.resource_type = resource_type;
			game_load_funcs.setup_info.starting_minerals = starting_minerals;
			for (size_t i = 0; i != 12; ++i) {
				st.players[i].controller = slot_controller[i];
				st.players[i].race = (race_t)slot_race[i];
				st.players[i].force = slot_force[i];
				if (victory_condition == 0 && tournament_mode == 0) {
					if (i >= 8) game_load_funcs.setup_info.create_melee_units_for_player[i] = false;
					else game_load_funcs.setup_info.create_melee_units_for_player[i] = create_melee_units_for_player[i] != 0;
				}
			}
			st.lcg_rand_state = random_seed;
		}, initial_processing);
		
		std::array<int, 8> source_colors;
		for (size_t i = 0; i != 8; ++i) {
			source_colors[i] = st.players[i].color;
		}
		for (size_t i = 0; i != 8; ++i) {
			st.players[i].color = source_colors.at(player_color[i]);
		}
	}
	
	void next_frame() {
		if (st.current_frame == replay_st.end_frame) error("replay: attempt to play past end");
		execute_actions(replay_st.actions_data_buffer.data(), replay_st.actions_data_buffer.data() + replay_st.actions_data_buffer.size());
		state_functions::next_frame();
	}
	
	bool is_done() {
		return st.current_frame == replay_st.end_frame;
	}
	
};

struct replay_player: game_player {
	action_state action_st;
	replay_state replay_st;
	optional<replay_functions> opt_funcs;
	replay_player() = default;
	replay_player(const game_player& n) {
		set_st(n.st());
	}
	
	void load_replay_file(a_string filename, bool initial_processing = true) {
		auto file_r = data_loading::file_reader<>(std::move(filename));
		load_replay(data_loading::make_replay_file_reader(file_r), initial_processing);
	}
	void load_replay_data(uint8_t* data, size_t data_size, bool initial_processing = true) {
		load_replay(data_loading::data_reader_le(data, data + data_size), initial_processing);
	}
	void lazy_init() {
		if (!opt_funcs) opt_funcs.emplace(st(), action_st, replay_st);
	}

	template<typename reader_T>
	void load_replay(reader_T&& r, bool initial_processing = true) {
		lazy_init();
		opt_funcs->load_replay(r, initial_processing);
	}
	
	void next_frame() {
		lazy_init();
		opt_funcs->next_frame();
	}
	
	bool is_done() {
		lazy_init();
		return opt_funcs->is_done();
	}
	
};

}

#endif
