#include "replay_info.h"
#include "replay_saver.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace bwgame;

#define CHECK(cond) do { \
	if (!(cond)) { \
		std::fprintf(stderr, "CHECK failed: %s (%s:%d)\n", #cond, __FILE__, __LINE__); \
		return 1; \
	} \
} while (0)

struct grow_writer {
	a_vector<uint8_t>& dst;
	explicit grow_writer(a_vector<uint8_t>& dst) : dst(dst) {}
	void put_bytes(const uint8_t* src, size_t n) {
		dst.insert(dst.end(), src, src + n);
	}
	template<typename T>
	void put(T v) {
		put_bytes(reinterpret_cast<const uint8_t*>(&v), sizeof(v));
	}
};

static void put_string_field(data_loading::data_writer<>& w, const char* str, size_t max_size) {
	size_t n = std::strlen(str);
	if (n > max_size) n = max_size;
	for (size_t i = 0; i != n; ++i) w.put<uint8_t>((uint8_t)str[i]);
	for (size_t i = n; i != max_size; ++i) w.put<uint8_t>(0);
}

static std::array<uint8_t, 633> make_game_info() {
	std::array<uint8_t, 633> buf{};
	data_loading::data_writer<> w(buf.data(), buf.data() + buf.size());
	w.put<uint8_t>(1);
	w.put<uint32_t>(12345);
	w.put<uint16_t>(0);
	w.put<uint8_t>(0);
	w.put<uint32_t>(0x12345678);
	for (int i = 0; i != 8; ++i) w.put<uint8_t>(0);
	w.put<uint32_t>(0);
	put_string_field(w, "HostName", 24);
	w.put<uint32_t>(0x11);
	w.put<uint16_t>(128);
	w.put<uint16_t>(96);
	w.put<uint8_t>(2);
	w.put<uint8_t>(8);
	w.put<uint8_t>(6);
	w.put<uint8_t>(0);
	w.put<uint16_t>(15);
	w.put<uint16_t>(2);
	w.put<uint32_t>(0);
	w.put<uint16_t>(4);
	w.put<uint8_t>(1);
	w.put<uint8_t>(0);
	put_string_field(w, "TvB Game", 25);
	put_string_field(w, "Fighting Spirit", 32);
	w.put<uint16_t>(15);
	w.put<uint16_t>(2);
	w.put<uint16_t>(0);
	w.put<uint16_t>(0);
	w.put<uint8_t>(0);
	w.put<uint8_t>(1);
	w.put<uint8_t>(1);
	w.put<uint8_t>(2);
	w.put<uint8_t>(1);
	w.put<uint8_t>(0);
	w.put<uint8_t>(0);
	w.put<uint8_t>(1);
	w.put<uint8_t>(1);
	w.put<uint8_t>(0);
	w.put<uint8_t>(0);
	w.put<uint32_t>(0);
	w.put<uint32_t>(400);
	w.put<uint32_t>(50);
	w.put<uint8_t>(0);
	for (uint32_t i = 0; i != 12; ++i) {
		w.put<uint32_t>(i);
		w.put<uint32_t>(i);
		if (i == 0) {
			w.put<uint8_t>(player_t::controller_occupied);
			w.put<uint8_t>((uint8_t)race_t::terran);
			w.put<uint8_t>(1);
			put_string_field(w, "Alice", 25);
		} else if (i == 1) {
			w.put<uint8_t>(player_t::controller_computer_game);
			w.put<uint8_t>((uint8_t)race_t::zerg);
			w.put<uint8_t>(2);
			put_string_field(w, "Bob", 25);
		} else {
			w.put<uint8_t>(player_t::controller_closed);
			w.put<uint8_t>((uint8_t)race_t::none);
			w.put<uint8_t>(0);
			put_string_field(w, "", 25);
		}
	}
	for (uint32_t i = 0; i != 8; ++i) w.put<uint32_t>(i + 1);
	for (uint8_t i = 0; i != 8; ++i) w.put<uint8_t>(i < 2 ? 1 : 0);
	if (w.size() != 633) error("make_game_info: wrote %d bytes, expected 633", (int)w.size());
	return buf;
}

static int test_parse_game_info() {
	auto buf = make_game_info();
	auto info = parse_replay_game_info(buf.data(), buf.size());
	CHECK(info.is_broodwar);
	CHECK(info.end_frame == 12345);
	CHECK(info.random_seed == 0x12345678);
	CHECK(info.host_player_name == "HostName");
	CHECK(info.game_flags == 0x11);
	CHECK(info.map_tile_width == 128);
	CHECK(info.map_tile_height == 96);
	CHECK(info.active_player_count == 2);
	CHECK(info.slot_count == 8);
	CHECK(info.game_speed == 6);
	CHECK(info.game_type == 15);
	CHECK(info.game_sub_type == 2);
	CHECK(info.tileset == 4);
	CHECK(info.replay_autosaved);
	CHECK(info.game_name == "TvB Game");
	CHECK(info.map_name == "Fighting Spirit");
	CHECK(info.starting_minerals == 400);
	CHECK(info.starting_gas == 50);
	CHECK(info.allies_enabled);
	CHECK(info.teams_enabled);
	CHECK(!info.cheats_enabled);
	CHECK(info.players[0].name == "Alice");
	CHECK(info.players[0].controller == player_t::controller_occupied);
	CHECK(info.players[0].race == (int)race_t::terran);
	CHECK(info.players[0].force == 1);
	CHECK(info.players[0].color == 1);
	CHECK(info.players[0].create_melee_units);
	CHECK(info.players[1].name == "Bob");
	CHECK(info.players[1].controller == player_t::controller_computer_game);
	CHECK(info.players[1].race == (int)race_t::zerg);
	CHECK(info.players[1].force == 2);
	CHECK(replay_player_is_human(info.players[0].controller));
	CHECK(replay_player_is_computer(info.players[1].controller));
	CHECK(!replay_player_is_human(info.players[2].controller));
	return 0;
}

static int test_names_and_duration() {
	CHECK(std::strcmp(replay_game_type_name(2), "Melee") == 0);
	CHECK(std::strcmp(replay_game_type_name(10), "Use Map Settings") == 0);
	CHECK(std::strcmp(replay_game_type_name(15), "Top vs Bottom") == 0);
	CHECK(std::strcmp(replay_game_type_name(99), "Unknown") == 0);
	CHECK(std::strcmp(replay_tileset_name(4), "Jungle") == 0);
	CHECK(std::strcmp(replay_game_speed_name(6), "Fastest") == 0);
	CHECK(std::strcmp(replay_race_name((int)race_t::protoss), "Protoss") == 0);
	CHECK(std::strcmp(replay_race_name(6), "Random") == 0);
	CHECK(std::strcmp(replay_controller_name(player_t::controller_occupied), "Occupied") == 0);
	CHECK(replay_duration_ms(1000) == 42000);
	CHECK(replay_duration_ms(-1) == 0);
	CHECK(replay_duration_ms(0) == 0);
	return 0;
}

static int test_short_game_info_errors() {
	uint8_t tiny[16]{};
	bool threw = false;
	try {
		parse_replay_game_info(tiny, sizeof(tiny));
	} catch (const exception&) {
		threw = true;
	}
	CHECK(threw);
	return 0;
}

static int test_invalid_identifier() {
	uint8_t junk[64]{};
	bool threw = false;
	try {
		read_replay_info_data(junk, sizeof(junk));
	} catch (const exception&) {
		threw = true;
	}
	CHECK(threw);
	return 0;
}

static int test_roundtrip_file() {
	replay_saver_state saver_st;
	uint8_t map_bytes[] = {1, 2, 3, 4, 5, 6, 7, 8};
	saver_st.map_data = map_bytes;
	saver_st.map_data_size = sizeof(map_bytes);
	saver_st.random_seed = 99;
	saver_st.player_name = "HostName";
	saver_st.map_tile_width = 64;
	saver_st.map_tile_height = 64;
	saver_st.active_player_count = 2;
	saver_st.slot_count = 8;
	saver_st.game_speed = 6;
	saver_st.game_type = 2;
	saver_st.tileset = 0;
	saver_st.game_name = "Melee Game";
	saver_st.map_name = "Python";
	saver_st.setup_info.starting_minerals = 50;
	saver_st.players[0].controller = player_t::controller_occupied;
	saver_st.players[0].race = race_t::protoss;
	saver_st.players[0].force = 1;
	saver_st.players[0].color = 4;
	saver_st.player_names[0] = "Alice";
	saver_st.players[1].controller = player_t::controller_occupied;
	saver_st.players[1].race = race_t::zerg;
	saver_st.players[1].force = 2;
	saver_st.players[1].color = 7;
	saver_st.player_names[1] = "Bob";

	uint8_t stim[] = {54};
	replay_saver_functions(saver_st).add_action(10, 0, stim, sizeof(stim));

	a_vector<uint8_t> file_bytes;
	grow_writer gw(file_bytes);
	replay_saver_functions(saver_st).save_replay(2400, gw);

	replay_info_options opt;
	opt.load_actions = true;
	opt.load_map = true;
	auto info = read_replay_info_data(file_bytes.data(), file_bytes.size(), opt);
	CHECK(info.is_broodwar);
	CHECK(info.end_frame == 2400);
	CHECK(info.random_seed == 99);
	CHECK(info.host_player_name == "HostName");
	CHECK(info.map_name == "Python");
	CHECK(info.game_name == "Melee Game");
	CHECK(info.game_type == 2);
	CHECK(std::strcmp(replay_game_type_name(info.game_type), "Melee") == 0);
	CHECK(info.tileset == 0);
	CHECK(info.map_tile_width == 64);
	CHECK(info.map_tile_height == 64);
	CHECK(info.starting_minerals == 50);
	CHECK(info.players[0].name == "Alice");
	CHECK(info.players[0].race == (int)race_t::protoss);
	CHECK(info.players[0].color == 4);
	CHECK(info.players[1].name == "Bob");
	CHECK(info.players[1].race == (int)race_t::zerg);
	CHECK(info.actions_data_size == info.actions_data.size());
	CHECK(!info.actions_data.empty());
	CHECK(info.map_data_size == sizeof(map_bytes));
	CHECK(info.map_data.size() == sizeof(map_bytes));
	CHECK(std::memcmp(info.map_data.data(), map_bytes, sizeof(map_bytes)) == 0);
	CHECK(replay_duration_ms(info.end_frame) == 2400 * 42);

	replay_info_options header_only;
	header_only.load_actions = false;
	header_only.load_map = false;
	auto header = read_replay_info_data(file_bytes.data(), file_bytes.size(), header_only);
	CHECK(header.map_name == "Python");
	CHECK(header.players[0].name == "Alice");
	CHECK(header.actions_data.empty());
	CHECK(header.map_data.empty());
	CHECK(header.actions_data_size > 0);
	CHECK(header.map_data_size == 0);

	const char* path = "openbw_replay_info_test.rep";
	{
		data_loading::file_writer<> fw(path);
		fw.put_bytes(file_bytes.data(), file_bytes.size());
	}
	auto from_file = read_replay_info_file(path, opt);
	CHECK(from_file.map_name == "Python");
	CHECK(from_file.players[1].name == "Bob");
	CHECK(from_file.map_data.size() == sizeof(map_bytes));
	std::remove(path);
	return 0;
}

int main() {
	if (test_parse_game_info()) return 1;
	if (test_names_and_duration()) return 1;
	if (test_short_game_info_errors()) return 1;
	if (test_invalid_identifier()) return 1;
	if (test_roundtrip_file()) return 1;
	std::printf("replay_info_test: ok\n");
	return 0;
}
