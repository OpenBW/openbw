#ifndef BWGAME_SYNC_H
#define BWGAME_SYNC_H

#include "actions.h"
#include "bwgame.h"

namespace bwgame {

struct sync_state {
	std::array<a_vector<uint8_t>, 12> buffer;
	std::array<size_t, 12> buffer_begin{};
	std::array<size_t, 12> buffer_end{};
	struct scheduled_action {
		int frame;
		int owner;
		size_t data_begin;
		size_t data_end;
	};

	a_deque<scheduled_action> scheduled_actions;
	
	int latency = 2;
	bool is_first_bwapi_compatible_frame = true;
};

struct sync_functions: action_functions {
	sync_state& sync_st;
	explicit sync_functions(state& st, action_state& action_st, sync_state& sync_st) : action_functions(st, action_st), sync_st(sync_st) {}
	
	void execute_scheduled_actions() {
		while (!sync_st.scheduled_actions.empty() && st.current_frame >= sync_st.scheduled_actions.front().frame) {
			auto act = sync_st.scheduled_actions.front();
			sync_st.scheduled_actions.pop_front();
			sync_st.buffer_begin.at(act.owner) = act.data_end;
			const uint8_t* data = sync_st.buffer.at(act.owner).data();
			if (data + act.data_end > data + sync_st.buffer[act.owner].size()) error("data beyond end");
			data_loading::data_reader_le r(data + act.data_begin, data + act.data_end);
			read_action(act.owner, r);
		}
	}

	void next_frame() {
		execute_scheduled_actions();
		action_functions::next_frame();
	}

	void bwapi_compatible_next_frame() {
		if (sync_st.is_first_bwapi_compatible_frame) sync_st.is_first_bwapi_compatible_frame = false;
		else action_functions::next_frame();
		execute_scheduled_actions();
	}

	template<typename reader_T>
	bool schedule_action(int owner, reader_T&& r) {
		size_t n = r.size();
		auto& buffer = sync_st.buffer.at(owner);
		auto& buffer_begin = sync_st.buffer_begin[owner];
		auto& buffer_end = sync_st.buffer_end[owner];
		size_t pos = buffer_end;
		size_t new_end = pos + n;
		auto grow_buffer = [&]() {
			const size_t max_size = 1024u * 4 * sync_st.latency;
			size_t new_size = buffer.size() + buffer.size() / 2;
			if (new_size > max_size) new_size = max_size;
			size_t required_size = n;
			for (auto& v : sync_st.scheduled_actions) {
				if (v.owner == owner) required_size += v.data_end - v.data_begin;
			}
			if (new_size < required_size) new_size = required_size;
			if (new_size >= max_size) error("action buffer is full for player %d", owner);
			if (new_size >= max_size) return false;
			a_vector<uint8_t> new_buffer(new_size);
			buffer_begin = 0;
			size_t end = 0;
			const uint8_t* src = buffer.data();
			uint8_t* dst = new_buffer.data();
			for (auto& v : sync_st.scheduled_actions) {
				if (v.owner == owner) {
					size_t n = v.data_end - v.data_begin;
					std::memcpy(dst + end, src + v.data_begin, n);
					v.data_begin = end;
					v.data_end = end + n;
					end += n;
				}
			}
			buffer = std::move(new_buffer);
			buffer_end = end;
		};
		if (buffer_end + n > buffer.size()) {
			if (n <= buffer_begin) {
				pos = 0;
				new_end = n;
			} else {
				if (buffer.size() < new_end) {
					grow_buffer();
				}
			}
		} else if (buffer_end < buffer_begin && new_end >= buffer_begin) {
			grow_buffer();
			pos = buffer_end;
			new_end = pos + n;
		}
		if (new_end > buffer.size()) error("new_end > buffer.size()");
		buffer_end = new_end;
		r.get_bytes(buffer.data() + pos, n);
		sync_st.scheduled_actions.push_back({st.current_frame + sync_st.latency, owner, pos, buffer_end});
		return true;
	}

	bool schedule_action(int owner, const uint8_t* data, size_t data_size) {
		data_loading::data_reader_le r(data, data + data_size);
		return schedule_action(owner, r);
	}

};


}

#endif
