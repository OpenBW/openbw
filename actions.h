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
		if (unit_is_disabled(u)) return false;
		auto tid = u->unit_type->id;
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
	
	unit_t* get_first_selected_unit(int owner) const {
		for (unit_t* u : selected_units(owner)) {
			return u;
		}
		return nullptr;
	}
	
	unit_t* get_single_selected_unit(int owner) const {
		auto sel = selected_units(owner);
		if (sel.empty()) return nullptr;
		auto i = sel.begin();
		if (std::next(i) != sel.end()) return nullptr;
		return *i;
	}
	
	const order_type_t* get_default_gather_order(const unit_t* u, const unit_t* target) const {
		if (ut_flag(target, (unit_type_t::flags_t)0x800)) return get_order_type(Orders::Move);
		if (unit_is_mineral_field(target)) {
			if ((u->carrying_flags & ~3) == 0) return get_order_type(Orders::MoveToMinerals);
			else return get_order_type(Orders::Move);
		} else if (unit_is_refinery(target) && u_completed(target) && target->owner == u->owner) {
			if ((u->carrying_flags & ~3) == 0) return get_order_type(Orders::MoveToGas);
			else return get_order_type(Orders::Move);
		} else if (unit_is_active_resource_depot(target) && target->owner == u->owner) {
			if (u->carrying_flags & 2) return get_order_type(Orders::ReturnMinerals);
			else if (u->carrying_flags & 1) return get_order_type(Orders::ReturnGas);
			else return nullptr;
		} else return nullptr;
	}
	
	const order_type_t* get_default_order(size_t action, const unit_t* u, xy pos, const unit_t* target, const unit_type_t* target_unit_type) const {
		const order_type_t* order;
		switch (action) {
		case 0:
			return nullptr;
		case 1:
			if (u_grounded_building(u)) return nullptr;
			else if (target) {
				if (unit_is_special_beacon(target)) return nullptr;
				else if (unit_target_is_enemy(u, target)) return get_order_type(Orders::Attack1);
				else if (unit_provides_space(target) && unit_can_load_target(target, u)) return get_order_type(Orders::EnterTransport);
				else if (u_burrowed(u)) return get_order_type(Orders::Move);
				else return get_order_type(Orders::Follow);
			} else {
				if (target_unit_type) return get_order_type(Orders::RightClickAction);
				else return get_order_type(Orders::Move);
			}
		case 2:
			if (u_grounded_building(u)) {
				if (unit_is_factory(u)) {
					if (target) return get_order_type(Orders::RallyPointUnit);
					else return get_order_type(Orders::RallyPointTile);
				} else return nullptr;
			} else if (target) {
				if (unit_provides_space(u) && unit_can_load_target(u, target)) return get_order_type(Orders::PickupTransport);
				else if (unit_provides_space(target) && unit_can_load_target(target, u)) return get_order_type(Orders::EnterTransport);
				else if (u_burrowed(target)) return get_order_type(Orders::Move);
				else if (unit_is_queen(u) && !u_invincible(target) && unit_can_be_infested(target)) return get_order_type(Orders::CastInfestation);
				else if (unit_is(u, UnitTypes::Terran_Medic)) return get_order_type(Orders::HealMove);
				else return get_order_type(Orders::Follow);
			} else {
				if (unit_is_queen(u) && target_unit_type) return get_order_type(Orders::RightClickAction);
				else return get_order_type(Orders::Move);
			}
		case 5:
			if (!target) return get_default_order(1, u, pos, target, target_unit_type);
			order = get_default_gather_order(u, target);
			if (order) return order;
			if (u_grounded_building(target) && !u_completed(target)) {
				if (target->owner == u->owner && unit_race(target) == race::terran) {
					if (!ut_addon(target)) {
						if (!target->connected_unit || target->connected_unit->order_target.unit != target) {
							return get_order_type(Orders::ConstructingBuilding);
						}
					}
				}
			} else if (!unit_target_is_enemy(u, target)) {
				bool can_enter = unit_provides_space(target) && unit_can_load_target(target, u);
				if (!ut_building(target) && can_enter) {
					return get_order_type(Orders::EnterTransport);
				} else if (unit_race(target) == race::terran && ut_mech(target) && u_completed(target) && u->hp < u->unit_type->hitpoints) {
					return get_order_type(Orders::Repair);
				} else if (can_enter) {
					return get_order_type(Orders::EnterTransport);
				}
			}
			return get_default_order(1, u, pos, target, target_unit_type);
		case 6:
			return nullptr;
		default:
			xcept("get_default_order: unknown action %u", action);
		}
		return nullptr;
	}
	
	struct group_move_t {
		xy original_target_pos;
		xy target_pos;
		xy move_offset;
		bool has_move_offset;
		bool target_is_in_unit_bounds;
	};
	
	void calc_group_move(group_move_t& g, int owner, xy target_pos, bool can_be_obstructed) {
		g.original_target_pos = target_pos;
		g.has_move_offset = false;
		g.target_is_in_unit_bounds = false;
		
		rect area{{std::numeric_limits<int>::max(), std::numeric_limits<int>::max()}, {0, 0}};
		bool any_collision_enabled_units = false;
		size_t n_units = 0;
		xy sum_pos;
		unsigned units_area = 0;
		for (unit_t* u : selected_units(owner)) {
			++n_units;
			if (u->pathing_flags & 1) any_collision_enabled_units = true;
			xy pos = u->sprite->position;
			if (pos.x < area.from.x) area.from.x = pos.x;
			if (pos.x > area.to.x) area.to.x = pos.x;
			if (pos.y < area.from.y) area.from.y = pos.y;
			if (pos.y > area.to.y) area.to.y = pos.y;
			sum_pos += pos;
			int w = u->unit_type->dimensions.from.x + u->unit_type->dimensions.to.x + 1;
			int h = u->unit_type->dimensions.from.y + u->unit_type->dimensions.to.y + 1;
			units_area += w * h;
		}
		if (n_units == 0) return;
		
		if (any_collision_enabled_units && can_be_obstructed) {
			if (n_units == 1) {
				unit_t* u = get_first_selected_unit(owner);
				if (!unit_type_can_fit_at(u->unit_type, target_pos)) {
					xcept("waa can't fit");
				}
			} else {
				int units_area_square_width = isqrt(units_area);
				if (!rectangle_can_fit_at(target_pos, units_area_square_width, units_area_square_width)) {
					xcept("waa can't multi fit");
				}
			}
			target_pos = restrict_pos_to_map_bounds(target_pos);
		}
		if (n_units >= 2) {
			if (is_in_inner_bounds(g.original_target_pos, area)) {
				g.target_is_in_unit_bounds = true;
				g.move_offset = {};
			} else if (any_collision_enabled_units) {
				if (area.to.x - area.from.x <= 192 && area.to.y - area.from.y <= 192) {
					g.move_offset = g.original_target_pos - sum_pos / n_units;
					g.has_move_offset = true;
				}
			} else {
				if (area.to.x - area.from.x <= 256 && area.to.y - area.from.y <= 256) {
					g.move_offset = g.original_target_pos - sum_pos / n_units;
					g.has_move_offset = true;
				}
			}
		}
		
		g.target_pos = target_pos;
	}
	
	xy get_group_move_pos(const unit_t* u, xy pos, const group_move_t& g) const {
		xy target_pos = u->pathing_flags & 1 ? g.target_pos : g.original_target_pos;
		if (g.target_is_in_unit_bounds) {
			pos = u->sprite->position;
			if (pos.x <= target_pos.x - 32 || pos.x >= target_pos.x + 32) {
				pos.x = target_pos.x;
			}
			if (pos.y <= target_pos.y - 32 || pos.y >= target_pos.y + 32) {
				pos.y = target_pos.y;
			}
		} else {
			if (g.has_move_offset) {
				pos.x = u->sprite->position.x + g.move_offset.x;
				pos.y = u->sprite->position.y + g.move_offset.y;
			}
		}
		
		pos = restrict_move_target_to_valid_bounds(u, pos);
		if (~u->pathing_flags & 1) return pos;
		auto* source_region = get_region_at(target_pos);
		if (unit_type_can_fit_at(u->unit_type, pos)) {
			auto* pos_region = get_region_at(pos);
			if (pos_region == source_region) return pos;
			for (auto* n : source_region->walkable_neighbors) {
				if (n == pos_region) return pos;
			}
		}
		int distance_to_target = xy_length(target_pos - pos);
		if (distance_to_target == 0) distance_to_target = 1;
		int step_distance = std::min(distance_to_target / 4, 16);
		if (step_distance < 2) step_distance = distance_to_target;
		for (int distance = step_distance; distance <= distance_to_target; distance += step_distance) {
			xy npos = pos + (target_pos - pos) * distance / distance_to_target;
			auto* npos_region = get_region_at(npos);
			if (npos_region == source_region) return npos;
			for (auto* n : source_region->walkable_neighbors) {
				if (n == npos_region) return npos;
			}
		}
		return target_pos;
	}
	
	template<typename units_T>
	bool action_select(int owner, units_T&& units) {
		auto& selection = action_st.selection[owner];
		selection.clear();
		bool retval = false;
		for (unit_t* u : units) {
			if (u && u->unit_type->id != UnitTypes::Terran_Nuclear_Missile) {
				if (std::find(selection.begin(), selection.end(), u) == selection.end()) {
					if (!us_hidden(u) && (selection.empty() || unit_can_be_multi_selected(u))) {
						if (selection.size() >= 12) xcept("attempt to select more than 12 units");
						selection.push_back(u);
						retval = true;
					}
				}
			}
		}
		return retval;
	}
	
	bool action_select(int owner, unit_t* u) {
		return action_select(owner, std::array<unit_t*, 1>{u});
	}
	
	template<typename units_T>
	bool action_shift_select(int owner, units_T&& units) {
		auto& selection = action_st.selection[owner];
		bool retval = false;
		for (unit_t* u : units) {
			if (u) {
				if (std::find(selection.begin(), selection.end(), u) == selection.end()) {
					if (!us_hidden(u) && (selection.empty() || unit_can_be_multi_selected(u))) {
						if (selection.size() >= 12) xcept("attempt to select more than 12 units");
						selection.push_back(u);
						retval = true;
					}
				}
			}
		}
		return retval;
	}
	
	bool action_shift_select(int owner, unit_t* u) {
		return action_shift_select(owner, std::array<unit_t*, 1>{u});
	}
	
	template<typename units_T>
	bool action_deselect(int owner, units_T&& units) {
		auto& selection = action_st.selection[owner];
		bool retval = false;
		for (unit_t* u : units) {
			if (u) {
				auto i = std::find(selection.begin(), selection.end(), u);
				if (i != selection.end()) {
					selection.erase(i);
					retval = true;
				}
			}
		}
		return retval;
	}
	
	bool action_deselect(int owner, unit_t* u) {
		return action_deselect(owner, std::array<unit_t*, 1>{u});
	}
	
	bool action_train(int owner, const unit_type_t* unit_type) {
		unit_t* u = get_single_selected_unit(owner);
		if (!u) return false;
		if (u->owner != owner) return false;
		if (!unit_can_build(u, unit_type)) return false;
		if (unit_type->id > UnitTypes::Spell_Disruption_Web) return false;
		if (build_queue_push(u, unit_type)) {
			set_secondary_order(u, get_order_type(Orders::Train));
		}
		return true;
	}
	
	bool action_build(int owner, const order_type_t* order_type, const unit_type_t* unit_type, xy_t<size_t> tile_pos) {
		unit_t* u = get_single_selected_unit(owner);
		if (!u) return false;
		if (u->owner != owner) return false;
		if (!unit_build_order_valid(u, order_type, unit_type)) return false;
		if (ut_addon(unit_type)) {
			xcept("action_build: fixme addon");
		} else {
			// todo: check tiles
			log("action_build: fixme\n");
			xy pos(32 * tile_pos.x + unit_type->placement_size.x / 2, 32 * tile_pos.y + unit_type->placement_size.y / 2);
			place_building(u, order_type, unit_type, pos);
		}
		return true;
	}
	
	bool action_default_order(int owner, xy pos, unit_t* target, const unit_type_t* target_unit_type, bool queue) {
		if (target) target_unit_type = nullptr;
		else if (target_unit_type) {
			if (player_position_is_visible(owner, pos)) {
				target = find_unit({pos - xy(96, 96), pos + xy(97, 97)}, [&](unit_t* n) {
					if (n->unit_type != target_unit_type) return false;
					if (n->sprite->position.x + n->unit_type->placement_size.x / 2 - (unsigned)pos.x >= (unsigned)n->unit_type->placement_size.x) return false;
					if (n->sprite->position.y + n->unit_type->placement_size.y / 2 - (unsigned)pos.y >= (unsigned)n->unit_type->placement_size.y) return false;
					return true;
				});
				target_unit_type = nullptr;
			}
		}
		group_move_t g;
		bool has_calculated_group_move = false;
		bool retval = false;
		for (unit_t* u : selected_units(owner)) {
			size_t action = 0;
			if (u->unit_type->id == UnitTypes::Zerg_Lurker && u_burrowed(u)) {
				action = 3;
			} else {
				action = u->unit_type->right_click_action;
				if (action == 0 && u_grounded_building(u) && unit_is_factory(u)) {
					action = 2;
				}
			}
			if (u_hallucination(u)) {
				if (action == 4 || action == 5) {
					action = 1;
				}
			}
			const order_type_t* order = get_default_order(action, u, pos, target, target_unit_type);
			if (order) {
				if (order->id == Orders::RallyPointUnit) {
					if (unit_is_factory(u)) {
						retval = true;
						if (!target) target = u;
						u->building.rally.unit = target;
						u->building.rally.pos = target->sprite->position;
					}
				} else if (target != u) {
					if (order->id == Orders::RallyPointTile) {
						if (unit_is_factory(u)) {
							retval = true;
							u->building.rally.unit = nullptr;
							u->building.rally.pos = pos;
						}
					} else {
						if (order->id == Orders::Attack1) {
							if (u->subunit) {
								issue_order(u->subunit, queue, u->subunit->unit_type->attack_unit, target);
							}
							order = u->unit_type->attack_unit;
						}
						if (unit_can_receive_order(u, order)) {
							retval = true;
							if (target) {
								order_target_t order_target;
								order_target.unit = target;
								issue_order(u, queue, order, order_target);
							} else {
								order_target_t order_target;
								order_target.position = pos;
								order_target.unit_type = target_unit_type;
								if (!target_unit_type) {
									if (!has_calculated_group_move) {
										has_calculated_group_move = true;
										calc_group_move(g, owner, pos, true);
									}
									order_target.position = get_group_move_pos(u, pos, g);
								}
								issue_order(u, queue, order, order_target);
							}
							u->user_action_flags |= 2;
						}
					}
				}
				
			}
		}
		return retval;
	}
	
	bool action_order(int owner, const order_type_t* order, xy pos, unit_t* target, const unit_type_t* target_unit_type, bool queue) {
		switch (order->id) {
		case Orders::Die:
		case Orders::InfestedCommandCenter:
		case Orders::VultureMine:
		case Orders::DroneStartBuild:
		case Orders::InfestingCommandCenter:
		case Orders::PlaceBuilding:
		case Orders::PlaceProtossBuilding:
		case Orders::CreateProtossBuilding:
		case Orders::ConstructingBuilding:
		case Orders::PlaceAddon:
		case Orders::BuildNydusExit:
		case Orders::BuildingLand:
		case Orders::BuildingLiftOff:
		case Orders::DroneLiftOff:
		case Orders::Harvest2:
		case Orders::MoveToGas:
		case Orders::WaitForGas:
		case Orders::HarvestGas:
		case Orders::MoveToMinerals:
		case Orders::WaitForMinerals:
		case Orders::MiningMinerals:
		case Orders::GatheringInterrupted:
		case Orders::GatherWaitInterrupted:
		case Orders::CTFCOP2:
			return false;
		default:
			break;
		}
		bool can_be_obstructed = order->can_be_obstructed;
		group_move_t g;
		bool has_calculated_group_move = false;
		bool retval = false;
		for (unit_t* u : selected_units(owner)) {
			if (order->tech_type == TechTypes::None) {
				if (!unit_can_receive_order(u, order)) continue;
			} else {
				xcept("action_order: fixme tech type");
			}
			if (u == target) {
				if (!unit_order_can_target_self(u, order)) continue;
			}
			if (u->order_type->id == Orders::NukeLaunch) continue;
			if (u->unit_type->right_click_action == 0) {
				if (u_grounded_building(u) && !unit_is_factory(u)) {
					auto oid = u->order_type->id;
					if (oid != Orders::RallyPointUnit && oid != Orders::RallyPointTile && oid != Orders::RechargeShieldsBattery && oid != Orders::PickupBunker) continue;
				}
			}
			if (unit_is(u, UnitTypes::Terran_Medic)) {
				if (order->id == Orders::AttackMove || order->id == Orders::Attack1) {
					order = get_order_type(Orders::HealMove);
				}
			}
			const order_type_t* subunit_order = get_order_type(Orders::Nothing);
			if (order->id == Orders::Attack1) {
				if (target) order = u->unit_type->attack_unit;
				else order = u->unit_type->attack_move;
				if (order->id == Orders::Nothing) continue;
				if (u->subunit) {
					if (target) subunit_order = u->subunit->unit_type->attack_unit;
					else subunit_order = u->subunit->unit_type->attack_move;
				}
			}
			if (order->id == Orders::RallyPointUnit) {
				if (unit_is_factory(u)) {
					retval = true;
					if (!target) target = u;
					u->building.rally.unit = target;
					u->building.rally.pos = target->sprite->position;
				}
				u->user_action_flags |= 2;
			} else if (order->id == Orders::RallyPointTile) {
				if (unit_is_factory(u)) {
					retval = true;
					u->building.rally.unit = nullptr;
					u->building.rally.pos = pos;
				}
				u->user_action_flags |= 2;
			} else {
				retval = true;
				if (target) {
					order_target_t order_target;
					order_target.unit = target;
					issue_order(u, queue, order, order_target);
					if (subunit_order->id != Orders::Nothing) issue_order(u->subunit, queue, subunit_order, order_target);
				} else {
					order_target_t order_target;
					order_target.position = pos;
					order_target.unit_type = target_unit_type;
					if (!target_unit_type) {
						if (!has_calculated_group_move) {
							has_calculated_group_move = true;
							calc_group_move(g, owner, pos, can_be_obstructed);
						}
						order_target.position = get_group_move_pos(u, pos, g);
					}
					issue_order(u, queue, order, order_target);
					if (subunit_order->id != Orders::Nothing) issue_order(u->subunit, queue, subunit_order, order_target);
				}
				u->user_action_flags |= 2;
			}
		}
		return retval;
	}
	
	bool action_stop(int owner, bool queue) {
		bool retval = false;
		for (unit_t* u : selected_units(owner)) {
			if (unit_can_receive_order(u, get_order_type(Orders::Stop))) {
				retval = true;
				issue_order(u, queue, get_order_type(Orders::Stop), {});
			}
		}
		return retval;
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
	bool read_action_shift_select(int owner, reader_T&& r) {
		size_t n = r.template get<uint8_t>();
		if (n > 12) xcept("invalid selection of %d units", n);
		static_vector<unit_t*, 12> units;
		for (size_t i = 0; i != n; ++i) {
			auto uid = unit_id(r.template get<uint16_t>());
			units.push_back(get_unit(uid));
		}
		return action_shift_select(owner, units);
	}
	
	template<typename reader_T>
	bool read_action_deselect(int owner, reader_T&& r) {
		size_t n = r.template get<uint8_t>();
		if (n > 12) xcept("invalid deselection of %d units", n);
		static_vector<unit_t*, 12> units;
		for (size_t i = 0; i != n; ++i) {
			auto uid = unit_id(r.template get<uint16_t>());
			units.push_back(get_unit(uid));
		}
		return action_deselect(owner, units);
	}
	
	template<typename reader_T>
	bool read_action_train(int owner, reader_T&& r) {
		auto* unit_type = get_unit_type((UnitTypes)r.template get<uint16_t>());
		return action_train(owner, unit_type);
	}
	
	template<typename reader_T>
	bool read_action_default_order(int owner, reader_T&& r) {
		int x = r.template get<int16_t>();
		int y = r.template get<int16_t>();
		unit_t* target = get_unit(unit_id(r.template get<uint16_t>()));
		UnitTypes target_unit_type_id = (UnitTypes)r.template get<uint16_t>();
		const unit_type_t* target_unit_type = target_unit_type_id == UnitTypes::None ? nullptr : get_unit_type(target_unit_type_id);
		bool queue = r.template get<uint8_t>() != 0;
		return action_default_order(owner, {x, y}, target, target_unit_type, queue);
	}
	
	template<typename reader_T>
	bool read_action_player_leave(int owner, reader_T&& r) {
		(void)owner;
		int v = r.template get<int8_t>(); // ?
		(void)v;
		return true;
	}
	
	template<typename reader_T>
	bool read_action_build(int owner, reader_T&& r) {
		auto* order_type = get_order_type((Orders)r.template get<uint8_t>());
		size_t tile_x = r.template get<uint16_t>();
		size_t tile_y = r.template get<uint16_t>();
		auto* unit_type = get_unit_type((UnitTypes)r.template get<uint16_t>());
		return action_build(owner, order_type, unit_type, {tile_x, tile_y});
	}
	
	template<typename reader_T>
	bool read_action_stop(int owner, reader_T&& r) {
		bool queue = r.template get<uint8_t>() != 0;
		return action_stop(owner, queue);
	}
	
	template<typename reader_T>
	bool read_action_order(int owner, reader_T&& r) {
		int x = r.template get<int16_t>();
		int y = r.template get<int16_t>();
		unit_t* target = get_unit(unit_id(r.template get<uint16_t>()));
		UnitTypes target_unit_type_id = (UnitTypes)r.template get<uint16_t>();
		const unit_type_t* target_unit_type = target_unit_type_id == UnitTypes::None ? nullptr : get_unit_type(target_unit_type_id);
		Orders order_id = (Orders)r.template get<uint8_t>();
		const order_type_t* order_type = get_order_type(order_id);
		bool queue = r.template get<uint8_t>() != 0;
		return action_order(owner, order_type, {x, y}, target, target_unit_type, queue);
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
		case 10:
			return read_action_shift_select(owner, r);
		case 11:
			return read_action_deselect(owner, r);
		case 12:
			return read_action_build(owner, r);
		case 20:
			return read_action_default_order(owner, r);
		case 21:
			return read_action_order(owner, r);
		case 26:
			return read_action_stop(owner, r);
		case 31:
			return read_action_train(owner, r);
		case 87:
			return read_action_player_leave(owner, r);
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
		while (action_st.actions_data != action_st.actions_data_end) {
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
