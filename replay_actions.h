#ifndef BWGAME_REPLAY_ACTIONS_H
#define BWGAME_REPLAY_ACTIONS_H

#include "replay.h"

namespace bwgame {

inline const char* replay_action_name(int action_id) {
	switch (action_id) {
	case 9: return "Select";
	case 10: return "Shift Select";
	case 11: return "Deselect";
	case 12: return "Build";
	case 13: return "Set Shared Vision";
	case 14: return "Set Alliances";
	case 18: return "Cheat";
	case 19: return "Control Group";
	case 20: return "Default Order";
	case 21: return "Order";
	case 24: return "Cancel Construction";
	case 25: return "Cancel Morph";
	case 26: return "Stop";
	case 27: return "Carrier Stop";
	case 28: return "Reaver Stop";
	case 30: return "Return Cargo";
	case 31: return "Train";
	case 32: return "Cancel Train";
	case 33: return "Cloak";
	case 34: return "Decloak";
	case 35: return "Morph";
	case 37: return "Unsiege";
	case 38: return "Siege";
	case 39: return "Train Fighter";
	case 40: return "Unload All";
	case 41: return "Unload";
	case 42: return "Archon Warp";
	case 43: return "Hold Position";
	case 44: return "Burrow";
	case 45: return "Unburrow";
	case 46: return "Cancel Nuke";
	case 47: return "Liftoff";
	case 48: return "Research";
	case 49: return "Cancel Research";
	case 50: return "Upgrade";
	case 51: return "Cancel Upgrade";
	case 52: return "Cancel Addon";
	case 53: return "Building Morph";
	case 54: return "Stim Pack";
	case 87: return "Leave Game";
	case 88: return "Minimap Ping";
	case 90: return "Dark Archon Meld";
	case 92: return "Chat";
	case 210: return "Ext Cheat";
	default: return "Unknown";
	}
}

inline bool replay_action_is_selection(int action_id) {
	return action_id == 9 || action_id == 10 || action_id == 11;
}

inline double replay_actions_per_minute(int action_count, int frames) {
	if (frames <= 0 || action_count <= 0) return 0;
	return action_count * (60000.0 / 42.0) / frames;
}

constexpr int replay_apm_window_frames = 10000 / 42;

struct replay_action_event {
	int frame = 0;
	int player_id = 0;
	int owner = -1;
	int action_id = 0;
};

struct replay_chat_message {
	int frame = 0;
	int player_id = 0;
	int owner = -1;
	a_string text;
};

struct replay_leave_event {
	int frame = 0;
	int player_id = 0;
	int owner = -1;
	int reason = 0;
};

struct replay_player_action_stats {
	int action_count = 0;
	int command_count = 0;
	int first_action_frame = -1;
	int last_action_frame = -1;
	int peak_apm = 0;
	double average_apm = 0;
	double average_command_apm = 0;
	std::array<int, 256> by_action_id{};
};

struct replay_action_stats {
	int duration_frames = 0;
	int last_stream_frame = 0;
	int total_actions = 0;
	std::array<replay_player_action_stats, 12> players;
	a_vector<replay_chat_message> chat;
	a_vector<replay_leave_event> leaves;
	a_vector<replay_action_event> events;
};

struct replay_action_scan_config {
	bool collect_events = false;
	bool collect_chat = true;
	bool collect_leaves = true;
	int duration_frames = 0;
	std::array<int, 12> player_id_map{};
	bool use_player_id_map = false;
};

template<typename reader_T>
void skip_replay_action_payload(int action_id, reader_T& r) {
	switch (action_id) {
	case 9:
	case 10:
	case 11: {
		size_t n = r.template get<uint8_t>();
		if (n > 12) error("replay_actions: invalid selection of %d units", (int)n);
		r.skip(n * 2);
		return;
	}
	case 12:
		r.skip(1 + 2 + 2 + 2);
		return;
	case 13:
		r.skip(2);
		return;
	case 14:
		r.skip(4);
		return;
	case 18:
		r.skip(4);
		return;
	case 19:
		r.skip(2);
		return;
	case 20:
		r.skip(2 + 2 + 2 + 2 + 1);
		return;
	case 21:
		r.skip(2 + 2 + 2 + 2 + 1 + 1);
		return;
	case 24:
	case 25:
	case 27:
	case 28:
	case 39:
	case 42:
	case 46:
	case 49:
	case 51:
	case 52:
	case 54:
	case 90:
		return;
	case 26:
	case 30:
	case 33:
	case 34:
	case 37:
	case 38:
	case 40:
	case 43:
	case 44:
	case 45:
		r.skip(1);
		return;
	case 31:
	case 32:
	case 35:
	case 41:
	case 53:
		r.skip(2);
		return;
	case 47:
	case 88:
		r.skip(4);
		return;
	case 48:
	case 50:
	case 87:
		r.skip(1);
		return;
	case 92:
		r.skip(81);
		return;
	case 210: {
		int type = r.template get<uint8_t>();
		if (type == 0) {
			int subtype = r.template get<uint8_t>();
			r.skip(2);
			if (subtype == 0 || subtype == 1 || subtype == 2) r.skip(4);
			else error("replay_actions: unknown ext cheat unit subtype %d", subtype);
		} else if (type == 1) {
			int subtype = r.template get<uint8_t>();
			r.skip(1);
			if (subtype == 0 || subtype == 1) r.skip(2);
			else if (subtype == 2 || subtype == 3) r.skip(4);
			else error("replay_actions: unknown ext cheat player subtype %d", subtype);
		} else error("replay_actions: unknown ext cheat type %d", type);
		return;
	}
	default:
		error("replay_actions: unknown action %d", action_id);
	}
}

template<typename F>
void visit_replay_actions(const uint8_t* begin, const uint8_t* end, F&& fn) {
	data_loading::data_reader_le r(begin, end);
	while (r.left()) {
		int frame = r.template get<int32_t>();
		size_t actions_size = r.template get<uint8_t>();
		const uint8_t* blob = r.get_n(actions_size);
		data_loading::data_reader_le r2(blob, blob + actions_size);
		while (r2.left()) {
			int player_id = r2.template get<uint8_t>();
			int action_id = r2.template get<uint8_t>();
			const uint8_t* payload = r2.ptr;
			skip_replay_action_payload(action_id, r2);
			fn(frame, player_id, action_id, payload, (size_t)(r2.ptr - payload));
		}
	}
}

inline int replay_action_owner(int player_id, const replay_action_scan_config& cfg) {
	if (cfg.use_player_id_map) {
		for (int i = 0; i != 12; ++i) {
			if (cfg.player_id_map[i] == player_id) return i;
		}
		return -1;
	}
	if (player_id >= 0 && player_id < 12) return player_id;
	return -1;
}

inline a_string replay_chat_text(const uint8_t* payload, size_t payload_size) {
	a_string str;
	size_t n = payload_size < 81 ? payload_size : 81;
	for (size_t i = 0; i != n; ++i) {
		uint8_t v = payload[i];
		if (v == 0) break;
		if (v >= 32) str += (char)v;
	}
	return str;
}

inline replay_action_stats scan_replay_actions(const uint8_t* begin, const uint8_t* end, replay_action_scan_config cfg = {}) {
	replay_action_stats stats;
	std::array<a_deque<int>, 12> windows;
	visit_replay_actions(begin, end, [&](int frame, int player_id, int action_id, const uint8_t* payload, size_t payload_size) {
		if (frame > stats.last_stream_frame) stats.last_stream_frame = frame;
		++stats.total_actions;
		int owner = replay_action_owner(player_id, cfg);
		if (cfg.collect_events) {
			replay_action_event ev;
			ev.frame = frame;
			ev.player_id = player_id;
			ev.owner = owner;
			ev.action_id = action_id;
			stats.events.push_back(ev);
		}
		if (cfg.collect_chat && action_id == 92) {
			replay_chat_message chat;
			chat.frame = frame;
			chat.player_id = player_id;
			chat.owner = owner;
			chat.text = replay_chat_text(payload, payload_size);
			stats.chat.push_back(std::move(chat));
		}
		if (cfg.collect_leaves && action_id == 87) {
			replay_leave_event leave;
			leave.frame = frame;
			leave.player_id = player_id;
			leave.owner = owner;
			leave.reason = payload_size ? (int)(int8_t)payload[0] : 0;
			stats.leaves.push_back(leave);
		}
		if (owner < 0 || owner >= 12) return;
		auto& p = stats.players[owner];
		++p.action_count;
		if (!replay_action_is_selection(action_id)) ++p.command_count;
		if (p.first_action_frame < 0) p.first_action_frame = frame;
		p.last_action_frame = frame;
		p.by_action_id[(unsigned)action_id] += 1;
		auto& w = windows[owner];
		w.push_back(frame);
		while (!w.empty() && w.front() <= frame - replay_apm_window_frames) w.pop_front();
		int peak = (int)replay_actions_per_minute((int)w.size(), replay_apm_window_frames);
		if (peak > p.peak_apm) p.peak_apm = peak;
	});
	stats.duration_frames = cfg.duration_frames > 0 ? cfg.duration_frames : stats.last_stream_frame;
	for (auto& p : stats.players) {
		p.average_apm = replay_actions_per_minute(p.action_count, stats.duration_frames);
		p.average_command_apm = replay_actions_per_minute(p.command_count, stats.duration_frames);
	}
	return stats;
}

inline replay_action_stats scan_replay_actions(const a_vector<uint8_t>& data, replay_action_scan_config cfg = {}) {
	return scan_replay_actions(data.data(), data.data() + data.size(), cfg);
}

template<typename reader_T>
a_vector<uint8_t> read_replay_actions_buffer(reader_T&& r, int* end_frame = nullptr, std::array<int, 12>* player_ids = nullptr) {
	uint32_t identifier = r.template get<uint32_t>();
	if (identifier != 0x53526572) error("read_replay_actions_buffer: invalid identifier %#x", identifier);
	std::array<uint8_t, 633> game_info;
	r.get_bytes(game_info.data(), game_info.size());
	if (end_frame || player_ids) {
		data_loading::data_reader_le gir(game_info.data(), game_info.data() + game_info.size());
		gir.get<uint8_t>();
		int frames = (int)gir.get<uint32_t>();
		if (end_frame) *end_frame = frames;
		if (player_ids) {
			gir.skip(156);
			for (size_t i = 0; i != 12; ++i) {
				gir.get<uint32_t>();
				(*player_ids)[i] = (int)gir.get<uint32_t>();
				gir.skip(28);
			}
		}
	}
	uint32_t actions_size = r.template get<uint32_t>();
	a_vector<uint8_t> actions(actions_size);
	if (actions_size) r.get_bytes(actions.data(), actions_size);
	return actions;
}

inline a_vector<uint8_t> read_replay_actions_data(const uint8_t* data, size_t data_size, int* end_frame = nullptr, std::array<int, 12>* player_ids = nullptr) {
	auto r = data_loading::data_reader_le(data, data + data_size);
	return read_replay_actions_buffer(data_loading::make_replay_file_reader(r), end_frame, player_ids);
}

inline a_vector<uint8_t> read_replay_actions_file(a_string filename, int* end_frame = nullptr, std::array<int, 12>* player_ids = nullptr) {
	auto file_r = data_loading::file_reader<>(std::move(filename));
	return read_replay_actions_buffer(data_loading::make_replay_file_reader(file_r), end_frame, player_ids);
}

inline replay_action_stats scan_replay_file(a_string filename, replay_action_scan_config cfg = {}) {
	int end_frame = 0;
	std::array<int, 12> player_ids{};
	auto actions = read_replay_actions_file(std::move(filename), &end_frame, &player_ids);
	if (cfg.duration_frames <= 0) cfg.duration_frames = end_frame;
	if (!cfg.use_player_id_map) {
		cfg.player_id_map = player_ids;
		cfg.use_player_id_map = true;
	}
	return scan_replay_actions(actions, cfg);
}

inline replay_action_stats scan_replay_data(const uint8_t* data, size_t data_size, replay_action_scan_config cfg = {}) {
	int end_frame = 0;
	std::array<int, 12> player_ids{};
	auto actions = read_replay_actions_data(data, data_size, &end_frame, &player_ids);
	if (cfg.duration_frames <= 0) cfg.duration_frames = end_frame;
	if (!cfg.use_player_id_map) {
		cfg.player_id_map = player_ids;
		cfg.use_player_id_map = true;
	}
	return scan_replay_actions(actions, cfg);
}

}

#endif
