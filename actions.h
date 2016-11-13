#ifndef BWGAME_ACTIONS_H
#define BWGAME_ACTIONS_H

#include "bwgame.h"

namespace bwgame {

struct action_state {
	std::array<int, 12> player_id{};
	
	const uint8_t* actions_data_begin = nullptr;
	const uint8_t* actions_data_end = nullptr;
	const uint8_t* actions_data = nullptr;
	int next_action_frame = 0;
	
	std::array<static_vector<unit_t*, 12>, 12> selection{};
};

struct action_functions: state_functions {
	action_state& action_st;
	explicit action_functions(state& st, action_state& action_st) : state_functions(st), action_st(action_st) {}
	
	bool unit_can_be_multi_selected(const unit_t* u) const {
		if (ut_building(u)) return false;
		if (ut_flag(u, (unit_type_t::flags_t)0x800)) return false;
		if (is_disabled(u)) return false;
		int tid = u->unit_type->id;
		if (tid >= UnitTypes::Special_Floor_Missile_Trap && tid <= UnitTypes::Special_Right_Wall_Flame_Trap) return false;
		if (tid == UnitTypes::Terran_Vulture_Spider_Mine) return false;
		if (tid == UnitTypes::Zerg_Egg) return false;
		if (tid == UnitTypes::Critter_Rhynadon) return false;
		if (tid == UnitTypes::Critter_Bengalaas) return false;
		if (tid == UnitTypes::Critter_Scantid) return false;
		if (tid == UnitTypes::Critter_Kakaru) return false;
		if (tid == UnitTypes::Critter_Ragnasaur) return false;
		if (tid == UnitTypes::Critter_Ursadon) return false;
		if (tid == UnitTypes::Spell_Dark_Swarm) return false;
		if (tid == UnitTypes::Spell_Disruption_Web) return false;
		return true;
	}
	
	unit_t* selected_unit(unit_t* u) const {
		return u && u->sprite && !unit_dead(u) ? u : nullptr;
	}
	
	auto selected_units(int owner) const {
		return make_filter_range(make_transform_range(action_st.selection[owner], [this](unit_t* u) {
			return selected_unit(u);
		}), [](unit_t* u) {
			return u != nullptr;
		});
	}
	
	unit_t* get_single_selected_unit(int owner) const {
		auto&& sel = selected_units(owner);
		if (sel.empty()) return nullptr;
		auto i = sel.begin();
		if (std::next(i) != sel.end()) return nullptr;
		return *i;
	}
	
	template<typename units_T>
	bool action_select(int owner, units_T&& units) {
		auto& selection = action_st.selection[owner];
		selection.clear();
		for (unit_t* u : units) {
			if (u && u->unit_type->id != UnitTypes::Terran_Nuclear_Missile) {
				if (std::find(selection.begin(), selection.end(), u) == selection.end()) {
					if (!us_hidden(u) && (selection.empty() || unit_can_be_multi_selected(u))) {
						if (selection.size() >= 12) xcept("attempt to select more than 12 units");
						selection.push_back(u);
					}
				}
			}
		}
		return true;
	}
	
	bool action_select(int owner, unit_t* u) {
		return action_select(owner, std::array<unit_t*, 1>{u});
	}
	
	bool action_train(int owner, const unit_type_t* unit_type) {
		unit_t* u = get_single_selected_unit(owner);
		if (!u) return false;
		xcept("train %d\n", unit_type->id);
		return false;
	}
	
	template<typename reader_T>
	bool read_action_select(int owner, reader_T&& r) {
		size_t n = r.template get<uint8_t>();
		if (n > 12) xcept("invalid selection of %d units", n);
		static_vector<unit_t*, 12> units;
		for (size_t i = 0; i != n; ++i) {
			auto uid = unit_id(r.template get<uint16_t>());
			units.push_back(get_unit(uid));
		}
		return action_select(owner, units);
	}
	
	template<typename reader_T>
	bool read_action_train(int owner, reader_T&& r) {
		auto* unit_type = get_unit_type(r.template get<uint16_t>());
		return action_train(owner, unit_type);
	}
	
	template<typename reader_T>
	bool read_action(reader_T&& r) {
		int player_id = r.template get<uint8_t>();
		int action_id = r.template get<uint8_t>();
		auto i = std::find(action_st.player_id.begin(), action_st.player_id.end(), player_id);
		if (i == action_st.player_id.end()) xcept("execute_action: player id %d not found", player_id);
		int owner = (int)(i - action_st.player_id.begin());
		switch (action_id) {
		case 9:
			return read_action_select(owner, r);
		case 31:
			return read_action_train(owner, r);
		default:
			xcept("execute_action: unknown action %d", action_id);
		}
		return false;
	}
	
	bool read_action(uint8_t* data, size_t data_size) {
		data_loading::data_reader_le r(data, data + data_size);
		return read_action(r);
	}
	
	void execute_actions() {
		if (st.current_frame != action_st.next_action_frame) return;
		if (action_st.actions_data == action_st.actions_data_end) return;
		while (r.left()) {
			data_loading::data_reader_le r(action_st.actions_data, action_st.actions_data_end);
			int frame = r.get<int32_t>();
			if (frame != st.current_frame) {
				action_st.next_action_frame = frame;
				return;
			}
			size_t actions_size = r.get<uint8_t>();
			const uint8_t* end = r.ptr + actions_size;
			data_loading::data_reader_le r2(r.ptr, end);
			while (r2.ptr != end) {
				read_action(r2);
			}
			action_st.actions_data = end;
		}
	}
	
};

struct actions_player {
private:
	action_state action_st;
	optional<action_functions> opt_funcs;
public:
	a_vector<uint8_t> actions_data_buffer;
	actions_player() = default;
	explicit actions_player(state& st) : opt_funcs(in_place, st, action_st) {}
	
	void set_actions_data_buffer() {
		action_st.actions_data_begin = actions_data_buffer.data();
		action_st.actions_data_end = actions_data_buffer.data() + actions_data_buffer.size();
		action_st.actions_data = action_st.actions_data_begin;
	}
	
	void set_st(state& st) {
		opt_funcs.emplace(st, action_st);
	}
	
	void next_frame() {
		if (!opt_funcs) xcept("actions_player: not initialized");
		execute_actions();
		funcs().next_frame();
	}
	
	void execute_actions() {
		funcs().execute_actions();
	}
	
	action_functions& funcs() {
		return *opt_funcs;
	}
	state& st() {
		return funcs().st;
	}
	
};

}

#endif
