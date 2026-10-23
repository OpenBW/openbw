#ifndef BWGAME_REPLAY_INFO_H
#define BWGAME_REPLAY_INFO_H

#include "replay.h"

namespace bwgame {

struct replay_player_info {
	int slot = 0;
	int player_id = 0;
	int controller = 0;
	int race = 0;
	int force = 0;
	int color = 0;
	a_string name;
	bool create_melee_units = false;
};

struct replay_info {
	bool is_broodwar = false;
	int end_frame = 0;
	int campaign_id = 0;
	uint32_t random_seed = 0;
	a_string host_player_name;
	uint32_t game_flags = 0;
	int map_tile_width = 0;
	int map_tile_height = 0;
	int active_player_count = 0;
	int slot_count = 0;
	int game_speed = 0;
	int game_state = 0;
	int game_type = 0;
	int game_sub_type = 0;
	int tileset = 0;
	bool replay_autosaved = false;
	int computer_player_count = 0;
	a_string game_name;
	a_string map_name;
	int victory_condition = 0;
	int resource_type = 0;
	bool standard_unit_stats = true;
	int fog_of_war = 0;
	int starting_units = 0;
	bool allies_enabled = false;
	bool teams_enabled = false;
	bool cheats_enabled = false;
	int tournament_mode = 0;
	int starting_minerals = 0;
	int starting_gas = 0;
	std::array<replay_player_info, 12> players;
	a_vector<uint8_t> actions_data;
	a_vector<uint8_t> map_data;
	size_t actions_data_size = 0;
	size_t map_data_size = 0;
};

struct replay_info_options {
	bool load_actions = true;
	bool load_map = false;
};

inline a_string replay_fixed_string(const char* data, size_t max_size) {
	a_string r;
	for (size_t i = 0; i != max_size; ++i) {
		unsigned char v = (unsigned char)data[i];
		if (!v) break;
		if (v >= 21) r += (char)v;
	}
	return r;
}

template<size_t N>
inline a_string replay_fixed_string(const std::array<char, N>& str) {
	return replay_fixed_string(str.data(), N);
}

inline void replay_maybe_korean_utf8(a_string& s) {
	a_string kn;
	if (korean::korean_locale_to_utf8(s, kn)) s = kn;
}

// Logical game time at fastest speed (42ms per frame).
inline int replay_duration_ms(int end_frame) {
	if (end_frame < 0) return 0;
	return end_frame * 42;
}

inline const char* replay_game_type_name(int game_type) {
	switch (game_type) {
	case 2: return "Melee";
	case 3: return "Free For All";
	case 4: return "One on One";
	case 5: return "Capture The Flag";
	case 6: return "Greed";
	case 7: return "Slaughter";
	case 8: return "Sudden Death";
	case 9: return "Ladder";
	case 10: return "Use Map Settings";
	case 11: return "Team Melee";
	case 12: return "Team Free For All";
	case 13: return "Team Capture The Flag";
	case 15: return "Top vs Bottom";
	case 16: return "Iron Man Ladder";
	default: return "Unknown";
	}
}

inline const char* replay_tileset_name(int tileset) {
	switch (tileset) {
	case 0: return "Badlands";
	case 1: return "Space Platform";
	case 2: return "Installation";
	case 3: return "Ashworld";
	case 4: return "Jungle";
	case 5: return "Desert";
	case 6: return "Arctic";
	case 7: return "Twilight";
	default: return "Unknown";
	}
}

inline const char* replay_game_speed_name(int game_speed) {
	switch (game_speed) {
	case 0: return "Slowest";
	case 1: return "Slower";
	case 2: return "Slow";
	case 3: return "Normal";
	case 4: return "Fast";
	case 5: return "Faster";
	case 6: return "Fastest";
	default: return "Unknown";
	}
}

inline const char* replay_controller_name(int controller) {
	switch (controller) {
	case player_t::controller_inactive: return "Inactive";
	case player_t::controller_computer_game: return "Computer";
	case player_t::controller_occupied: return "Occupied";
	case player_t::controller_rescue_passive: return "Rescue Passive";
	case player_t::controller_unused_rescue_active: return "Rescue Active";
	case player_t::controller_computer: return "Computer (Open)";
	case player_t::controller_open: return "Open";
	case player_t::controller_neutral: return "Neutral";
	case player_t::controller_closed: return "Closed";
	case player_t::controller_unused_observer: return "Observer";
	case player_t::controller_user_left: return "Left";
	case player_t::controller_computer_defeated: return "Computer Defeated";
	default: return "Unknown";
	}
}

inline const char* replay_race_name(int race) {
	switch (race) {
	case (int)race_t::zerg: return "Zerg";
	case (int)race_t::terran: return "Terran";
	case (int)race_t::protoss: return "Protoss";
	case (int)race_t::none: return "None";
	case 6: return "Random";
	default: return "Unknown";
	}
}

inline bool replay_player_is_human(int controller) {
	return controller == player_t::controller_occupied || controller == player_t::controller_user_left;
}

inline bool replay_player_is_computer(int controller) {
	return controller == player_t::controller_computer_game || controller == player_t::controller_computer || controller == player_t::controller_computer_defeated;
}

inline replay_info parse_replay_game_info(const uint8_t* data, size_t size) {
	if (size < 633) error("parse_replay_game_info: expected 633 bytes, got %d", (int)size);
	data_loading::data_reader_le gir(data, data + 633);
	replay_info info;
	info.is_broodwar = gir.get<uint8_t>() != 0;
	info.end_frame = (int)gir.get<uint32_t>();
	info.campaign_id = gir.get<uint16_t>();
	gir.get<uint8_t>(); // command byte
	info.random_seed = gir.get<uint32_t>();
	gir.get<std::array<uint8_t, 8>>();
	gir.get<uint32_t>();
	auto host_name = gir.get<std::array<char, 24>>();
	info.game_flags = gir.get<uint32_t>();
	info.map_tile_width = gir.get<uint16_t>();
	info.map_tile_height = gir.get<uint16_t>();
	info.active_player_count = gir.get<uint8_t>();
	info.slot_count = gir.get<uint8_t>();
	info.game_speed = gir.get<uint8_t>();
	info.game_state = gir.get<uint8_t>();
	info.game_type = gir.get<uint16_t>();
	info.game_sub_type = gir.get<uint16_t>();
	gir.get<uint32_t>();
	info.tileset = gir.get<uint16_t>();
	info.replay_autosaved = gir.get<uint8_t>() != 0;
	info.computer_player_count = gir.get<uint8_t>();
	auto game_name = gir.get<std::array<char, 25>>();
	auto map_name = gir.get<std::array<char, 32>>();
	gir.get<uint16_t>(); // game type (repeat)
	gir.get<uint16_t>();
	gir.get<uint16_t>();
	gir.get<uint16_t>();
	info.victory_condition = gir.get<uint8_t>();
	info.resource_type = gir.get<uint8_t>();
	info.standard_unit_stats = gir.get<uint8_t>() != 0;
	info.fog_of_war = gir.get<uint8_t>();
	info.starting_units = gir.get<uint8_t>();
	gir.get<uint8_t>(); // fixed positions
	gir.get<uint8_t>(); // restriction flags
	info.allies_enabled = gir.get<uint8_t>() != 0;
	info.teams_enabled = gir.get<uint8_t>() != 0;
	info.cheats_enabled = gir.get<uint8_t>() != 0;
	info.tournament_mode = gir.get<uint8_t>();
	gir.get<uint32_t>(); // victory condition value
	info.starting_minerals = (int)gir.get<uint32_t>();
	info.starting_gas = (int)gir.get<uint32_t>();
	gir.get<uint8_t>();

	info.host_player_name = replay_fixed_string(host_name);
	info.game_name = replay_fixed_string(game_name);
	info.map_name = replay_fixed_string(map_name);
	replay_maybe_korean_utf8(info.host_player_name);
	replay_maybe_korean_utf8(info.game_name);
	replay_maybe_korean_utf8(info.map_name);

	for (size_t i = 0; i != 12; ++i) {
		auto& p = info.players[i];
		p.slot = (int)gir.get<uint32_t>();
		p.player_id = (int)gir.get<uint32_t>();
		p.controller = gir.get<uint8_t>();
		p.race = gir.get<uint8_t>();
		p.force = gir.get<uint8_t>();
		auto name = gir.get<std::array<char, 25>>();
		p.name = replay_fixed_string(name);
		replay_maybe_korean_utf8(p.name);
	}
	auto player_color = gir.get<std::array<uint32_t, 8>>();
	auto create_melee = gir.get<std::array<uint8_t, 8>>();
	for (size_t i = 0; i != 8; ++i) {
		info.players[i].color = (int)player_color[i];
		info.players[i].create_melee_units = create_melee[i] != 0;
	}
	if (gir.left() != 0) error("parse_replay_game_info: %d trailing bytes", (int)gir.left());
	return info;
}

template<typename reader_T>
replay_info read_replay_info(reader_T&& r, replay_info_options options = {}) {
	uint32_t identifier = r.template get<uint32_t>();
	if (identifier != 0x53526572) error("read_replay_info: invalid identifier %#x", identifier);

	std::array<uint8_t, 633> game_info_buffer;
	r.get_bytes(game_info_buffer.data(), game_info_buffer.size());
	replay_info info = parse_replay_game_info(game_info_buffer.data(), game_info_buffer.size());

	uint32_t actions_size = r.template get<uint32_t>();
	info.actions_data_size = actions_size;
	if (options.load_actions) {
		info.actions_data.resize(actions_size);
		if (actions_size) r.get_bytes(info.actions_data.data(), actions_size);
	} else if (options.load_map) {
		a_vector<uint8_t> discard(actions_size);
		if (actions_size) r.get_bytes(discard.data(), actions_size);
	} else {
		return info;
	}

	uint32_t map_size = r.template get<uint32_t>();
	info.map_data_size = map_size;
	if (options.load_map) {
		info.map_data.resize(map_size);
		if (map_size) r.get_bytes(info.map_data.data(), map_size);
	}
	return info;
}

inline replay_info read_replay_info_data(const uint8_t* data, size_t data_size, replay_info_options options = {}) {
	auto r = data_loading::data_reader_le(data, data + data_size);
	return read_replay_info(data_loading::make_replay_file_reader(r), options);
}

inline replay_info read_replay_info_file(a_string filename, replay_info_options options = {}) {
	auto file_r = data_loading::file_reader<>(std::move(filename));
	return read_replay_info(data_loading::make_replay_file_reader(file_r), options);
}

}

#endif
