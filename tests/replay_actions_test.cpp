#include "replay_actions.h"
#include "replay_saver.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

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

static void put_le32(a_vector<uint8_t>& v, int32_t x) {
	uint32_t u = (uint32_t)x;
	v.push_back((uint8_t)u);
	v.push_back((uint8_t)(u >> 8));
	v.push_back((uint8_t)(u >> 16));
	v.push_back((uint8_t)(u >> 24));
}

static void put_u16(a_vector<uint8_t>& v, uint16_t x) {
	v.push_back((uint8_t)x);
	v.push_back((uint8_t)(x >> 8));
}

static int test_names() {
	CHECK(std::strcmp(replay_action_name(54), "Stim Pack") == 0);
	CHECK(std::strcmp(replay_action_name(92), "Chat") == 0);
	CHECK(std::strcmp(replay_action_name(87), "Leave Game") == 0);
	CHECK(std::strcmp(replay_action_name(9), "Select") == 0);
	CHECK(std::strcmp(replay_action_name(123), "Unknown") == 0);
	CHECK(replay_action_is_selection(9));
	CHECK(replay_action_is_selection(10));
	CHECK(!replay_action_is_selection(54));
	CHECK(replay_actions_per_minute(0, 1000) == 0);
	CHECK(replay_actions_per_minute(42, 0) == 0);
	double apm = replay_actions_per_minute(100, 1000);
	CHECK(std::fabs(apm - (100 * (60000.0 / 42.0) / 1000)) < 1e-9);
	return 0;
}

static int test_scan_commands() {
	a_vector<uint8_t> buf;
	put_le32(buf, 10);
	buf.push_back(2);
	buf.push_back(0);
	buf.push_back(54);

	put_le32(buf, 42);
	buf.push_back(7);
	buf.push_back(0);
	buf.push_back(9);
	buf.push_back(2);
	put_u16(buf, 1);
	put_u16(buf, 2);

	put_le32(buf, 100);
	a_vector<uint8_t> chat_blob;
	chat_blob.push_back(1);
	chat_blob.push_back(92);
	a_string msg = "gg wp";
	for (size_t i = 0; i != 81; ++i) {
		chat_blob.push_back(i < msg.size() ? (uint8_t)msg[i] : 0);
	}
	buf.push_back((uint8_t)chat_blob.size());
	buf.insert(buf.end(), chat_blob.begin(), chat_blob.end());

	put_le32(buf, 200);
	buf.push_back(3);
	buf.push_back(1);
	buf.push_back(87);
	buf.push_back(1);

	replay_action_scan_config cfg;
	cfg.collect_events = true;
	cfg.duration_frames = 1000;
	auto stats = scan_replay_actions(buf.data(), buf.data() + buf.size(), cfg);
	CHECK(stats.total_actions == 4);
	CHECK(stats.last_stream_frame == 200);
	CHECK(stats.duration_frames == 1000);
	CHECK(stats.players[0].action_count == 2);
	CHECK(stats.players[0].command_count == 1);
	CHECK(stats.players[0].by_action_id[54] == 1);
	CHECK(stats.players[0].by_action_id[9] == 1);
	CHECK(stats.players[0].first_action_frame == 10);
	CHECK(stats.players[0].last_action_frame == 42);
	CHECK(stats.players[1].action_count == 2);
	CHECK(stats.players[1].command_count == 2);
	CHECK(stats.players[1].by_action_id[92] == 1);
	CHECK(stats.players[1].by_action_id[87] == 1);
	CHECK(stats.chat.size() == 1);
	CHECK(stats.chat[0].owner == 1);
	CHECK(stats.chat[0].text == "gg wp");
	CHECK(stats.leaves.size() == 1);
	CHECK(stats.leaves[0].owner == 1);
	CHECK(stats.leaves[0].reason == 1);
	CHECK(stats.events.size() == 4);
	CHECK(stats.players[0].average_apm > 0);
	CHECK(stats.players[0].peak_apm > 0);
	CHECK(std::fabs(stats.players[0].average_apm - replay_actions_per_minute(2, 1000)) < 1e-9);
	return 0;
}

static int test_player_id_map() {
	a_vector<uint8_t> buf;
	put_le32(buf, 1);
	buf.push_back(2);
	buf.push_back(5);
	buf.push_back(54);
	replay_action_scan_config cfg;
	cfg.use_player_id_map = true;
	cfg.player_id_map[0] = 5;
	auto stats = scan_replay_actions(buf, cfg);
	CHECK(stats.players[0].action_count == 1);
	CHECK(stats.players[5].action_count == 0);
	return 0;
}

static int test_unknown_and_truncated() {
	a_vector<uint8_t> unknown;
	put_le32(unknown, 0);
	unknown.push_back(2);
	unknown.push_back(0);
	unknown.push_back(99);
	bool threw = false;
	try {
		scan_replay_actions(unknown);
	} catch (const exception&) {
		threw = true;
	}
	CHECK(threw);

	a_vector<uint8_t> trunc;
	put_le32(trunc, 0);
	trunc.push_back(5);
	trunc.push_back(0);
	trunc.push_back(9);
	trunc.push_back(2);
	threw = false;
	try {
		scan_replay_actions(trunc);
	} catch (const exception&) {
		threw = true;
	}
	CHECK(threw);

	a_vector<uint8_t> bad_select;
	put_le32(bad_select, 0);
	bad_select.push_back(3);
	bad_select.push_back(0);
	bad_select.push_back(9);
	bad_select.push_back(13);
	threw = false;
	try {
		scan_replay_actions(bad_select);
	} catch (const exception&) {
		threw = true;
	}
	CHECK(threw);
	return 0;
}

static int test_ext_cheat_and_empty() {
	a_vector<uint8_t> empty;
	auto z = scan_replay_actions(empty);
	CHECK(z.total_actions == 0);
	CHECK(z.duration_frames == 0);

	a_vector<uint8_t> buf;
	put_le32(buf, 8);
	buf.push_back(1 + 1 + 1 + 1 + 1 + 4);
	buf.push_back(0);
	buf.push_back(210);
	buf.push_back(1);
	buf.push_back(2);
	buf.push_back(0);
	buf.push_back(0x10);
	buf.push_back(0x27);
	buf.push_back(0);
	buf.push_back(0);
	auto stats = scan_replay_actions(buf);
	CHECK(stats.total_actions == 1);
	CHECK(stats.players[0].by_action_id[210] == 1);
	return 0;
}

static int test_visit() {
	a_vector<uint8_t> buf;
	put_le32(buf, 3);
	buf.push_back(2);
	buf.push_back(2);
	buf.push_back(54);
	int seen = 0;
	int bad = 0;
	visit_replay_actions(buf.data(), buf.data() + buf.size(), [&](int frame, int player_id, int action_id, const uint8_t*, size_t n) {
		++seen;
		if (frame != 3 || player_id != 2 || action_id != 54 || n != 0) bad = 1;
	});
	CHECK(seen == 1);
	CHECK(bad == 0);
	return 0;
}

static int test_roundtrip_file() {
	replay_saver_state saver_st;
	uint8_t map_bytes[] = {9, 8, 7, 6};
	saver_st.map_data = map_bytes;
	saver_st.map_data_size = sizeof(map_bytes);
	saver_st.player_name = "Host";
	saver_st.map_tile_width = 64;
	saver_st.map_tile_height = 64;
	saver_st.active_player_count = 2;
	saver_st.slot_count = 8;
	saver_st.game_type = 2;
	saver_st.game_name = "Game";
	saver_st.map_name = "Map";
	saver_st.players[0].controller = player_t::controller_occupied;
	saver_st.players[0].race = race_t::terran;
	saver_st.player_names[0] = "Alice";
	saver_st.players[1].controller = player_t::controller_occupied;
	saver_st.players[1].race = race_t::zerg;
	saver_st.player_names[1] = "Bob";

	uint8_t stim[] = {54};
	replay_saver_functions(saver_st).add_action(5, 0, stim, sizeof(stim));
	uint8_t chat[82]{};
	chat[0] = 92;
	const char* text = "hello";
	for (size_t i = 0; text[i]; ++i) chat[1 + i] = (uint8_t)text[i];
	replay_saver_functions(saver_st).add_action(20, 1, chat, sizeof(chat));
	uint8_t leave[] = {87, 2};
	replay_saver_functions(saver_st).add_action(40, 1, leave, sizeof(leave));

	a_vector<uint8_t> file_bytes;
	grow_writer gw(file_bytes);
	replay_saver_functions(saver_st).save_replay(4800, gw);

	auto stats = scan_replay_data(file_bytes.data(), file_bytes.size());
	CHECK(stats.duration_frames == 4800);
	CHECK(stats.players[0].action_count == 1);
	CHECK(stats.players[0].by_action_id[54] == 1);
	CHECK(stats.players[1].action_count == 2);
	CHECK(stats.chat.size() == 1);
	CHECK(stats.chat[0].owner == 1);
	CHECK(stats.chat[0].text == "hello");
	CHECK(stats.leaves.size() == 1);
	CHECK(stats.leaves[0].reason == 2);
	CHECK(std::fabs(stats.players[0].average_apm - replay_actions_per_minute(1, 4800)) < 1e-9);

	const char* path = "openbw_replay_actions_test.rep";
	{
		data_loading::file_writer<> fw(path);
		fw.put_bytes(file_bytes.data(), file_bytes.size());
	}
	auto from_file = scan_replay_file(path);
	CHECK(from_file.players[0].action_count == 1);
	CHECK(from_file.chat.size() == 1);
	std::remove(path);
	return 0;
}

int main() {
	if (test_names()) return 1;
	if (test_scan_commands()) return 1;
	if (test_player_id_map()) return 1;
	if (test_unknown_and_truncated()) return 1;
	if (test_ext_cheat_and_empty()) return 1;
	if (test_visit()) return 1;
	if (test_roundtrip_file()) return 1;
	std::printf("replay_actions_test: ok\n");
	return 0;
}
