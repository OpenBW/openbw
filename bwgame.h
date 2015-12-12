
struct unit_id {
	uint16_t raw_value = 0;
	unit_id() = default;
	explicit unit_id(uint16_t raw_value) : raw_value(raw_value) {}
	explicit unit_id(size_t index, int generation) : raw_value((uint16_t)(index | generation << 11)) {}
	size_t index() {
		return raw_value & 0x7ff;
	}
	int generation() {
		return raw_value >> 11;
	}
};

#include "bwenums.h"

namespace bwgame {
;

#include "util.h"

#include "data_types.h"
#include "game_types.h"

#include "data_loading.h"

// Broodwar linked lists insert new elements between the first and second entry.
template<typename cont_T, typename T>
void bw_insert_list(cont_T&cont, T&v) {
	if (cont.empty()) cont.push_front(v);
	else cont.insert(++cont.begin(), v);
}

struct sight_values_t {
	struct maskdat_node_t;
	typedef a_vector<maskdat_node_t> maskdat_t;
	struct maskdat_node_t {
		// 
		//  I would like to change this structure a bit, move vision_propagation to a temporary inside reveal_sight_at,
		//  and change prev_count to a bool since it can only have two values, or remove it entirely.
		//
		// TODO: remove vision_propagation, since this struct is supposed to be static (stored in game_state)
		//
		maskdat_node_t*prev; // the tile from us directly towards the origin (diagonal is allowed and preferred)
		// the other tile with equal diagonal distance to the origin as prev, if it exists.
		// otherwise, it is prev
		maskdat_node_t*prev2;
		size_t map_index_offset;
		// temporary variable used when spreading vision to make sure we don't go through obstacles
		mutable uint32_t vision_propagation;
		int8_t x;
		int8_t y;
		// prev_count will be 1 if prev and prev2 are equal, otherwise it is 2
		int8_t prev_count;
	};
	static_assert(sizeof(maskdat_node_t) == 20, "maskdat_node_t: wrong size");

	int max_width, max_height;
	int min_width, min_height;
	int min_mask_size;
	int ext_masked_count;
	maskdat_t maskdat;

};

struct cv5_entry {
	uint16_t field_0;
	uint16_t flags;
	uint16_t left;
	uint16_t top;
	uint16_t right;
	uint16_t bottom;
	uint16_t field_C;
	uint16_t field_E;
	uint16_t field_10;
	uint16_t field_12;
	std::array<uint16_t, 16> megaTileRef;
};
static_assert(sizeof(cv5_entry) == 52, "cv5_entry: wrong size");
struct vf4_entry {
	std::array<uint16_t, 16> flags;
};
static_assert(sizeof(vf4_entry) == 32, "vf4_entry: wrong size");
struct vx4_entry {
	std::array<uint16_t, 16> images;
};
static_assert(sizeof(vx4_entry) == 32, "vx4_entry: wrong size");
struct vr4_entry {
	std::array<uint8_t, 64> bitmap;
};
static_assert(sizeof(vr4_entry) == 64, "vr4_entry: wrong size");

struct tile_id {
	uint16_t raw_value = 0;
	tile_id() = default;
	explicit tile_id(uint16_t raw_value) : raw_value(raw_value) {}
	explicit tile_id(size_t group_index, size_t subtile_index) : raw_value((uint16_t)(group_index << 4 | subtile_index)) {}
	bool has_creep() {
		return ((raw_value >> 4) & 0x8000) != 0;
	}
	size_t group_index() {
		return (raw_value >> 4) & 0x7ff;
	}
	size_t subtile_index() {
		return raw_value & 0xf;
	}
	explicit operator bool() const {
		return raw_value != 0;
	}
};

struct tile_t {
	enum {
		flag_walkable = 1,
		flag_unk0 = 2,
		flag_unwalkable = 4,
		flag_unk1 = 8,
		flag_unk2 = 0x10,
		flag_unk3 = 0x20,
		flag_has_creep = 0x40,
		flag_unbuildable = 0x80,
		flag_very_high = 0x100,
		flag_middle = 0x200,
		flag_high = 0x400,
		flag_occupied = 0x800,
		flag_creep_receding = 0x1000,
		partially_walkable = 0x2000,
		flag_temporary_creep = 0x4000,
		flag_unk4 = 0x8000
	};
	union {
		struct {
			uint8_t visible;
			uint8_t explored;
			uint16_t flags;
		};
		uint32_t raw;
	};
	operator uint32_t() const {
		return raw;
	}
	bool operator==(uint32_t val) const {
		return raw == val;
	}
};

struct global_state {

	global_state() = default;
	global_state(global_state&) = delete;
	global_state(global_state&&) = default;
	global_state&operator=(global_state&) = delete;
	global_state&operator=(global_state&&) = default;

	flingy_types_t flingy_types;
	sprite_types_t sprite_types;
	image_types_t image_types;
	order_types_t order_types;
	iscript_t iscript;

	a_vector<grp_t> grps;
	a_vector<grp_t*> image_grp;
	a_vector<a_vector<a_vector<xy>>> lo_offsets;
	a_vector<std::array<a_vector<a_vector<xy>>*, 6>> image_lo_offsets;

	std::array<xy, 256> direction_table;
};

struct game_state {

	game_state() = default;
	game_state(game_state&) = delete;
	game_state(game_state&&) = default;
	game_state&operator=(game_state&) = delete;
	game_state&operator=(game_state&&) = default;

	size_t map_tile_width;
	size_t map_tile_height;
	size_t map_width;
	size_t map_height;

	a_string map_file_name;

	a_vector<a_string> map_strings;
	a_string get_string(size_t index) {
		if (index == 0) return "<null string>";
		--index;
		if (index >= map_strings.size()) return "<invalid string index>";
		return map_strings[index];
	}

	a_string scenario_name;
	a_string scenario_description;

	std::array<int, 228> unit_air_strength;
	std::array<int, 228> unit_ground_strength;

	struct force_t {
		a_string name;
		uint8_t flags;
	};
	std::array<force_t, 4> forces;

	std::array<sight_values_t, 12> sight_values;

	size_t tileset_index;

	//a_vector<tile_id> gfx_creep_tiles;
	a_vector<tile_id> gfx_tiles;
	a_vector<cv5_entry> cv5;
	a_vector<vf4_entry> vf4;
	a_vector<uint16_t> mega_tile_flags;

	unit_types_t unit_types;
	weapon_types_t weapon_types;
	upgrade_types_t upgrade_types;
	tech_types_t tech_types;

	std::array<std::array<bool, 228>, 12> unit_type_allowed;
	std::array<std::array<int, 61>, 12> max_upgrade_levels;
	std::array<std::array<bool, 44>, 12> tech_available;

	std::array<xy, 12> start_locations;

	bool is_replay;
	int local_player;

	int max_unit_width;
	int max_unit_height;
};

struct state_base {

	const global_state*global;
	game_state*game;

	int update_tiles_countdown;

	std::array<int, 12> selection_circle_color;

	int order_timer_counter;
	int secondary_order_timer_counter;

	struct player_t {
		enum {
			controller_inactive,
			controller_computer_game,
			controller_occupied,
			controller_rescue_passive,
			controller_unused_rescue_active,
			controller_computer,
			controller_open,
			controller_neutral,
			controller_closed,
			controller_unused_observer,
			controller_user_left,
			controller_computer_defeated
		};
		int controller;
		int race;
		int force;
	};
	std::array<player_t, 12> players;

	std::array<std::array<int, 61>, 12> upgrade_levels;
	std::array<std::array<bool, 44>, 12> tech_researched;

	std::array<std::array<int, 228>, 12> unit_counts;
	std::array<std::array<int, 228>, 12> completed_unit_counts;

	std::array<int, 12> factory_counts;
	std::array<int, 12> building_counts;
	std::array<int, 12> non_building_counts;

	std::array<int, 12> completed_factory_counts;
	std::array<int, 12> completed_building_counts;
	std::array<int, 12> completed_non_building_counts;

	std::array<int, 12> total_buildings_ever_completed;
	std::array<int, 12> total_non_buildings_ever_completed;

	std::array<int, 12> unit_score;
	std::array<int, 12> building_score;

	std::array<std::array<int, 12>, 3> supply_used;
	std::array<std::array<int, 12>, 3> supply_available;

	uint32_t local_mask;

	std::array<int, 12> shared_vision;

	a_vector<tile_id> gfx_creep_tiles;
	a_vector<tile_t> tiles;
	a_vector<uint16_t> tiles_mega_tile_index;

	std::array<int, 0x100> random_counts;
	int total_random_counts;
	uint32_t lcg_rand_state;

	int last_net_error;

	rect viewport;

	int unit_finder_size;
	a_vector<std::pair<unit_t*, int>> unit_finder_x;
	a_vector<std::pair<unit_t*, int>> unit_finder_y;
};

struct state : state_base {

	state() = default;
	state(state&) = delete;
	state(state&&) = default;
	state&operator=(state&) = delete;
	state&operator=(state&&) = default;

	intrusive_list<unit_t, default_link_f> visible_units;
	intrusive_list<unit_t, default_link_f> hidden_units;
	intrusive_list<unit_t, default_link_f> scanner_sweep_units;
	intrusive_list<unit_t, default_link_f> sight_related_units;
	intrusive_list<unit_t, default_link_f> free_units;

	a_vector<unit_t> units = a_vector<unit_t>(1700);

	std::array<intrusive_list<unit_t, intrusive_list_member_link<unit_t, &unit_t::player_units_link>>, 12> player_units;

	a_vector<intrusive_list<sprite_t, default_link_f>> sprites_on_tile_line;
	intrusive_list<sprite_t, default_link_f> free_sprites;
	a_vector<sprite_t> sprites = a_vector<sprite_t>(2500);

	intrusive_list<image_t, default_link_f> free_images;
	a_vector<image_t> images = a_vector<image_t>(5000);

	intrusive_list<order_t, default_link_f> free_orders;
	a_vector<order_t> orders = a_vector<order_t>(2000);
	int allocated_order_count;
};

struct state_functions {

	state&st;
	const global_state&global_st = *st.global;
	const game_state&game_st = *st.game;

	explicit state_functions(state&st) : st(st) {}

	bool in_game_loop = false;
	bool update_tiles = false;
	unit_t*iscript_unit = nullptr;
	unit_t*iscript_order_unit = nullptr;

	const order_type_t*get_order_type(int id) {
		if ((size_t)id >= 189) xcept("invalid order id %d", id);
		return &global_st.order_types.vec[id];
	}

	struct iscript_unit_setter {
		unit_t*&iscript_unit;
		unit_t*prev_iscript_unit;
		iscript_unit_setter(state_functions*sf, unit_t*new_iscript_unit) : iscript_unit(sf->iscript_unit) {
			prev_iscript_unit = iscript_unit;
			iscript_unit = new_iscript_unit;
		}
		~iscript_unit_setter() {
			iscript_unit = prev_iscript_unit;
		}
	};

	void u_set_status_flag(unit_t*u, unit_t::status_flags_t flag, bool value) {
		if (value) u->status_flags |= flag;
		else u->status_flags &= ~flag;
	};

	bool ut_flag(unit_t*u, unit_type_t::flags_t flag) {
		return !!(u->unit_type->flags & flag);
	};
	bool u_status_flag(unit_t*u, unit_t::status_flags_t flag) {
		return !!(u->status_flags & flag);
	};
	bool st_flag(sprite_t*s, sprite_type_t::flags_t flag) {
		return !!(s->sprite_type->flags & flag);
	};

	bool u_completed(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_completed);
	};
	bool u_in_building(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_in_building);
	};
	bool u_immovable(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_immovable);
	};
// 	bool u_unpowered(unit_t*u) {
// 		return u_status_flag(u, unit_t::status_flag_unpowered);
// 	};
	bool u_disabled(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_disabled);
	};
	bool u_burrowed(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_burrowed);
	};
// 	bool u_not_building(unit_t*u) {
// 		return u_status_flag(u, unit_t::status_flag_not_building);
// 	};
// 	bool u_can_attack(unit_t*u) {
// 		return u_status_flag(u, unit_t::status_flag_can_attack);
// 	};
	bool u_non_building(unit_t *u) {
		return u_status_flag(u, unit_t::status_flag_non_building);
	}
	bool u_building(unit_t *u) {
		return u_status_flag(u, unit_t::status_flag_building);
	}
	bool u_grounded_building(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_grounded_building);
	};
	bool u_hallucination(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_hallucination);
	};
	bool u_flying(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_flying);
	};
	bool u_speed_upgrade(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_speed_upgrade);
	};
	bool u_cooldown_upgrade(unit_t*u) {
		return u_status_flag(u, unit_t::status_flag_cooldown_upgrade);
	};
	bool u_gathering(unit_t* u) {
		return u_status_flag(u, unit_t::status_flag_gathering);
	}
	bool u_requires_detector(unit_t* u) {
		return u_status_flag(u, unit_t::status_flag_requires_detector);
	}
	bool u_cloaked(unit_t* u) {
		return u_status_flag(u, unit_t::status_flag_cloaked);
	}

	bool ut_is_turret(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_is_turret);
	};
	bool ut_worker(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_worker);
	};
	bool ut_hero(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_hero);
	};
	bool ut_building(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_building);
	};
	bool ut_flyer(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_flyer);
	};
// 	bool ut_can_attack(unit_t*u) {
// 		return ut_flag(u, unit_type_t::flag_can_attack);
// 	};
	bool ut_non_building(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_non_building);
	};
	bool ut_invincible(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_invincible);
	};
	bool ut_two_units_in_one_egg(unit_t*u) {
		return ut_flag(u, unit_type_t::flag_two_units_in_one_egg);
	};

	bool st_hidden(sprite_t*sprite) {
		return st_flag(sprite, sprite_type_t::flag_hidden);
	};

	const unit_type_t*get_unit_type(int id) {
		if ((size_t)id >= 228) xcept("invalid unit id %d", id);
		return &game_st.unit_types.vec[id];
	}
	const image_type_t*get_image_type(int id) {
		if ((size_t)id >= 999) xcept("invalid image id %d", id);
		return &global_st.image_types.vec[id];
	}

	unit_t*get_unit(unit_id id) {
		size_t idx = id.index();
		if (!idx) return nullptr;
		size_t actual_index = idx - 1;
		if (actual_index >= st.units.size()) xcept("attempt to dereference invalid unit id %d (actual index %d)", idx, actual_index);
		unit_t*u = &st.units[actual_index];
		if (u->unit_id_generation != id.generation()) return nullptr;
		return u;
	};

	bool unit_type_spreads_creep(const unit_type_t*ut, bool include_non_evolving) {
		if (ut->id == UnitTypes::Zerg_Hatchery && include_non_evolving) return true;
		if (ut->id == UnitTypes::Zerg_Lair) return true;
		if (ut->id == UnitTypes::Zerg_Hive) return true;
		if (ut->id == UnitTypes::Zerg_Creep_Colony && include_non_evolving) return true;
		if (ut->id == UnitTypes::Zerg_Spore_Colony) return true;
		if (ut->id == UnitTypes::Zerg_Sunken_Colony) return true;
		return false;
	}

	void setAllImageGroupFlagsPal11(sprite_t*sprite) {
		for (image_t*img : sprite->images) {
			if (img->palette_type == 0xb) img->flags |= 1;
		}
	};

	int visible_hp_plus_shields(unit_t*u) {
		int r = 0;
		if (u->unit_type->has_shield) r += u->shield_points >> 8;
		r += (u->hp + 0xff) >> 8;
		return r;
	};
	int max_visible_hp(unit_t*u) {
		int hp = u->unit_type->hitpoints >> 8;
		if (hp == 0) hp = (u->hp + 0xff) >> 8;
		if (hp == 0) hp = 1;
		return hp;
	};
	int max_visible_hp_plus_shields(unit_t*u) {
		int shields = 0;
		if (u->unit_type->has_shield) shields += u->unit_type->shield_points;
		return max_visible_hp(u) + shields;
	};

	int get_unit_strength(unit_t*u, bool ground) {
		if (u->unit_type->id == UnitTypes::Zerg_Larva || u->unit_type->id == UnitTypes::Zerg_Egg || u->unit_type->id == UnitTypes::Zerg_Cocoon || u->unit_type->id == UnitTypes::Zerg_Lurker_Egg) return 0;
		int vis_hp_shields = visible_hp_plus_shields(u);
		int max_vis_hp_shields = max_visible_hp_plus_shields(u);
		if (u->status_flags&StatusFlags::IsHallucination) {
			if (vis_hp_shields < max_vis_hp_shields) return 0;
		}

		int r = ground ? game_st.unit_ground_strength[u->unit_type->id] : game_st.unit_air_strength[u->unit_type->id];
		if (u->unit_type->id == UnitTypes::Terran_Bunker) {
			xcept("fixme getUnitStrength bunker; see getUnitStrength_AirOrGround");
		}
		if (u->unit_type->flags & UnitPrototypeFlags::Spellcaster) {
			if (~u->status_flags&StatusFlags::IsHallucination) r += (u->energy >> 8) / 2;
		}
		return r * vis_hp_shields / max_vis_hp_shields;
	};

	void set_unit_hp(unit_t*u, int hitpoints) {
		u->hp = hitpoints;
		if (u->hp > u->unit_type->hitpoints) u->hp = u->unit_type->hitpoints;
		if (u->sprite->flags & SpriteFlags::Selected && u->sprite->visibility_flags&st.local_mask) {
			setAllImageGroupFlagsPal11(u->sprite);
		}
		if (u->status_flags & StatusFlags::Completed) {
			// damage overlay stuff

			u->air_strength = get_unit_strength(u, false);
			u->ground_strength = get_unit_strength(u, true);
		}
	};

	bool is_frozen_or_flying(unit_t*u) {
		if (u_flying(u)) return true;
		if (u->lockdown_timer) return true;
		if (u->stasis_timer) return true;
		if (u->maelstrom_timer) return true;
		return false;
	};

	void set_current_button_set(unit_t*u, int type) {
		if (type != UnitTypes::None && !ut_building(u)) {
			if (is_frozen_or_flying(u)) return;
		}
		u->current_button_set = type;
	};

	image_t*find_image(sprite_t*sprite, int first_id, int last_id) {
		for (image_t*i : sprite->images) {
			if (i->image_type->id >= first_id && i->image_type->id <= last_id) return i;
		}
		return nullptr;
	};

	void freeze_effect_end(unit_t*u, int first, int last) {
		bool still_frozen = is_frozen_or_flying(u);
		if (u->subunit && !still_frozen) {
			u->status_flags &= ~StatusFlags::DoodadStatesThing;
			xcept("freeze_effect_end: orderComputer_cl");
			// orderComputer_cl(u->subunit, units_dat.ReturntoIdle[u->subunit->unit_type]);
		}
		image_t*image = find_image(u->sprite, first, last);
		if (!image && u->subunit) image = find_image(u->subunit->sprite, first, last);
		if (image) iscript_run_anim(image, iscript_anims::Death);
		if (u->unit_type->flags & UnitPrototypeFlags::Worker && !still_frozen) {
			// sub_468DB0
			unit_t*target = u->worker.harvest_target;
			if (target && target->unit_type->flags & UnitPrototypeFlags::FlyingBuilding) {
				if (u->worker.is_carrying_something) {
					if (target->building.resource.gather_queue_count) {
						//if (u->order_id )
						xcept("weird logic, fix me when this throws");
					}
				}
			}
		}
		u->order_queue_timer = 15;
	};

	void remove_stasis(unit_t*u) {
		u->stasis_timer = 0;
		set_current_button_set(u, u->unit_type->id);
		if (~u->unit_type->flags & UnitPrototypeFlags::Invincible) {
			u->status_flags &= ~StatusFlags::Invincible;
		}
		freeze_effect_end(u, idenums::IMAGEID_Stasis_Field_Small, idenums::IMAGEID_Stasis_Field_Large);
	};

	void updateUnitStatusTimers(unit_t*u) {
		if (u->stasis_timer) {
			--u->stasis_timer;
			if (!u->stasis_timer) {
				remove_stasis(u);
			}
		}
		if (u->stim_timer) {
			--u->stim_timer;
			if (!u->stim_timer) {
				xcept("remove stim");
			}
		}
		if (u->ensnare_timer) {
			--u->ensnare_timer;
			if (!u->ensnare_timer) {
				xcept("remove ensnare");
			}
		}
		if (u->defense_matrix_timer) {
			--u->defense_matrix_timer;
			if (!u->defense_matrix_timer) {
				xcept("remove defense matrix");
			}
		}
		if (u->irradiate_timer) {
			--u->irradiate_timer;
			xcept("irradiate damage");
			if (!u->irradiate_timer) {
				xcept("remove irradiate");
			}
		}
		if (u->lockdown_timer) {
			--u->lockdown_timer;
			if (!u->lockdown_timer) {
				xcept("remove lockdown");
			}
		}
		if (u->maelstrom_timer) {
			--u->maelstrom_timer;
			if (!u->maelstrom_timer) {
				xcept("remove maelstrom");
			}
		}
		if (u->plague_timer) {
			xcept("plague stuff");
		}
		if (u->storm_timer) --u->storm_timer;
		int prev_acidSporeCount = u->acid_spore_count;
		for (auto&v : u->acid_spore_time) {
			if (!v) continue;
			--v;
			if (!v) --u->acid_spore_count;
		}
		if (u->acid_spore_count) {
			xcept("acid spore stuff");
		} else if (prev_acidSporeCount) {
			xcept("RemoveOverlays(u, IMAGEID_Acid_Spores_1_Overlay_Small, IMAGEID_Acid_Spores_6_9_Overlay_Large);");
		}

	};

	bool create_selection_circle(sprite_t*sprite, int color, int imageid) {
		return false;
	};

	void removeSelectionCircle(sprite_t*sprite) {

	};

	void update_selection_sprite(sprite_t*sprite, int color) {
		if (!sprite->selection_timer) return;
		--sprite->selection_timer;
		if (~sprite->visibility_flags&st.local_mask) sprite->selection_timer = 0;
		if (sprite->selection_timer & 8 || (sprite->selection_timer == 0 && sprite->flags&SpriteFlags::Selected)) {
			if (~sprite->flags&SpriteFlags::DrawSelection) {
				if (create_selection_circle(sprite, color, idenums::IMAGEID_Selection_Circle_22pixels)) {
					sprite->flags |= SpriteFlags::DrawSelection;
				}
			}
		} else removeSelectionCircle(sprite);
	};

	void updateEnergyTimer(unit_t*u) {
		if (~u->unit_type->flags & UnitPrototypeFlags::Spellcaster) return;
		xcept("updateEnergyTimer");
	};

	bool unit_hp_below_33_percent(unit_t*u) {
		int max_hp = max_visible_hp(u);
		int hp = (u->hp + 0xff) >> 8;
		return hp * 100 / max_hp <= 33;
	};

	void update_unit_timers(unit_t*u) {
		if (u->main_order_timer) --u->main_order_timer;
		if (u->ground_weapon_cooldown) --u->ground_weapon_cooldown;
		if (u->air_weapon_cooldown) --u->air_weapon_cooldown;
		if (u->spell_cooldown) --u->spell_cooldown;
		if (u->unit_type->has_shield) {
			int max_shields = u->unit_type->shield_points << 8;
			if (u->shield_points != max_shields) {
				u->shield_points += 7;
				if (u->shield_points > max_shields) u->shield_points = max_shields;
				if (u->sprite->flags & SpriteFlags::Selected) {
					setAllImageGroupFlagsPal11(u->sprite);
				}
			}
		}
		if (u->unit_type->id == UnitTypes::Zerg_Zergling || u->unit_type->id == UnitTypes::Hero_Devouring_One) {
			if (u->ground_weapon_cooldown == 0) u->order_queue_timer = 0;
		}
		u->is_being_healed = false;
		if (u->status_flags & StatusFlags::Completed || ~u->sprite->flags & SpriteFlags::Hidden) {
			++u->cycle_counter;
			if (u->cycle_counter >= 8) {
				u->cycle_counter = 0;
				updateUnitStatusTimers(u);
			}
		}
		if (u->status_flags & StatusFlags::Completed) {
			if (u->unit_type->flags & UnitPrototypeFlags::RegeneratesHP) {
				if (u->hp > 0 && u->unit_type->hitpoints) {
					set_unit_hp(u, u->hp + 4);
				}
			}
			updateEnergyTimer(u);
			if (u->recent_order_timer) --u->recent_order_timer;
			bool remove_timer_hit_zero = false;
			if (u->remove_timer) {
				--u->remove_timer;
				if (!u->remove_timer) {
					xcept("orders_SelfDestructing...");
					return;
				}
			}
			int gf = u->unit_type->staredit_group_flags;
			if (gf&GroupFlags::Terran && ~gf&(GroupFlags::Zerg | GroupFlags::Protoss)) {
				if (u->status_flags&StatusFlags::GroundedBuilding || u->unit_type->flags & UnitPrototypeFlags::FlyingBuilding) {
					if (unit_hp_below_33_percent(u)) {
						xcept("killTargetUnitCheck(...)");
					}
				}
			}
		}
	};

	void update_unit_orders(unit_t*u) {
		if (~u->unit_type->flags & UnitPrototypeFlags::Subunit && ~u->sprite->flags&SpriteFlags::Hidden) {
			update_selection_sprite(u->sprite, st.selection_circle_color[u->owner]);
		}

		update_unit_timers(u);

		xcept("...");

	};

	int unit_movepos_state(unit_t*u) {
		if (u->sprite->position != u->move_target.pos) return 0;
		return u_immovable(u) ? 2 : 1;
	};

	bool unit_order_dead(unit_t*u) {
		return u->order_type->id == Orders::Die && u->order_state == 1;
	};

	bool Unit_ExecPathingState(unit_t*u) {

		bool refresh_vision = update_tiles;

		auto UMInitialize = [&](unit_t*u) {
			u->pathing_flags &= ~(1 | 2);
			if (u->sprite->elevation_level) u->pathing_flags |= 1;
			u->contour_bounds = { {0,0},{0,0} };
			int next_state = UM_Lump;
			if (!ut_is_turret(u) && u_in_building(u)) {
				next_state = UM_InitSeq;
			} else if (!u->sprite || unit_order_dead(u)) {
				next_state = UM_Lump;
			} else if (u_in_building(u)) {
				next_state = UM_Bunker;
			} else if (st_hidden(u->sprite)) {
				if (u->movement_flags & MovementFlags::Accelerating || unit_movepos_state(u) == 0) {
					// SetMoveTarget_xy(u)
					// ...
					xcept("todo hidden sprite pathing stuff");
				}
				next_state = UM_Hidden;
			} else if (u_burrowed(u)) {
				next_state = UM_Lump;
			}
			//else if (u_not_building(u)) next_state = u->pathing_flags & 1 ? UM_AtRest : UM_Flyer;
			//else if (u_can_attack(u)) next_state = ut_is_turret(u) ? UM_BldgTurret : UM_Turret;
			else if (u->pathing_flags & 1 && (u->movement_flags & MovementFlags::Accelerating || unit_movepos_state(u) == 0)) next_state = UM_LumpWannabe;
			u->movement_state = next_state;
		};

		switch (u->movement_state) {
		case UM_Init:
			UMInitialize(u);
			break;
		default:
			xcept("fixme: movement state %d\n", u->movement_state);
		}

		return refresh_vision;
	};

	bool is_transforming_zerg_building(unit_t*u) {
		if (u_completed(u)) return false;
		unit_type_t*t = u->build_queue[u->build_queue_slot];
		if (!t) return false;
		int tt = t->id;
		return tt == UnitTypes::Zerg_Hive || tt == UnitTypes::Zerg_Lair || tt == UnitTypes::Zerg_Greater_Spire || tt == UnitTypes::Zerg_Spore_Colony || tt == UnitTypes::Zerg_Sunken_Colony;
	};


	int unit_sight_range2(unit_t*u, bool ignore_blindness) {
		if (u_grounded_building(u) && !u_completed(u) && !is_transforming_zerg_building(u)) return 4;
		if (!ignore_blindness && u->is_blind) return 2;
		if (u->unit_type->id == UnitTypes::Terran_Ghost && st.upgrade_levels[u->owner][UpgradeTypes::Ocular_Implants]) return 11;
		if (u->unit_type->id == UnitTypes::Zerg_Overlord && st.upgrade_levels[u->owner][UpgradeTypes::Antennae]) return 11;
		if (u->unit_type->id == UnitTypes::Protoss_Observer && st.upgrade_levels[u->owner][UpgradeTypes::Sensor_Array]) return 11;
		if (u->unit_type->id == UnitTypes::Protoss_Scout && st.upgrade_levels[u->owner][UpgradeTypes::Apial_Sensors]) return 11;
		return u->unit_type->sight_range;
	};
	int unit_sight_range(unit_t*u) {
		return unit_sight_range2(u, false);
	};
	int unit_sight_range_ignore_blindness(unit_t*u) {
		return unit_sight_range2(u, true);
	};

	int unit_max_energy(unit_t*u) {
		if (ut_hero(u)) return 250 << 8;
		auto energy_upgrade = [&]() {
			switch (u->unit_type->id) {
			case UnitTypes::Terran_Ghost: return UpgradeTypes::Moebius_Reactor;
			case UnitTypes::Terran_Wraith: return UpgradeTypes::Apollo_Reactor;
			case UnitTypes::Terran_Science_Vessel: return UpgradeTypes::Titan_Reactor;
			case UnitTypes::Terran_Battlecruiser: return UpgradeTypes::Colossus_Reactor;
			case UnitTypes::Terran_Medic: return UpgradeTypes::Caduceus_Reactor;
			case UnitTypes::Zerg_Queen: return UpgradeTypes::Gamete_Meiosis;
			case UnitTypes::Zerg_Defiler: return UpgradeTypes::Metasynaptic_Node;
			case UnitTypes::Protoss_Corsair: return UpgradeTypes::Argus_Jewel;
			case UnitTypes::Protoss_Dark_Archon: return UpgradeTypes::Argus_Talisman;
			case UnitTypes::Protoss_High_Templar: return UpgradeTypes::Khaydarin_Amulet;
			case UnitTypes::Protoss_Arbiter: return UpgradeTypes::Khaydarin_Core;
			};
			return UpgradeTypes::None;
		};
		int upg = energy_upgrade();
		if (upg == UpgradeTypes::None) return 200 << 8;
		if (st.upgrade_levels[u->owner][upg]) return 250 << 8;
		return 200 << 8;
	}


	bool visible_to_everyone(unit_t*u) {
		if (ut_worker(u)) {
			if (u->worker.powerup && u->worker.powerup->unit_type->id == UnitTypes::Powerup_Flag) return true;
			else return false;
		}
		if (!u->unit_type->space_provided) return false;
		if (u->unit_type->id == UnitTypes::Zerg_Overlord && !st.upgrade_levels[u->owner][UpgradeTypes::Ventral_Sacs]) return false;
		if (u_hallucination(u)) return false;
		for (auto idx : u->loaded_units) {
			unit_t*lu = get_unit(idx);
			if (!lu || !lu->sprite) continue;
			if (unit_order_dead(lu)) continue;
			if (!ut_worker(lu)) continue;
			if (lu->worker.powerup && lu->worker.powerup->unit_type->id == UnitTypes::Powerup_Flag) return true;
		}
		return false;
	};

	size_t tile_index(xy pos) {
		size_t ux = (size_t)pos.x / 32;
		size_t uy = (size_t)pos.y / 32;
		if (ux >= game_st.map_tile_width || uy >= game_st.map_tile_height) xcept("attempt to get tile index for invalid position %d %d", pos.x, pos.y);
		return uy * game_st.map_tile_width + ux;
	};

	int get_ground_height_at(xy pos) {
		size_t index = tile_index(pos);
		log("index %d, st.gfx_creep_tiles.size() is %d\n", index, st.gfx_creep_tiles.size());
		tile_id creep_tile = st.gfx_creep_tiles.at(index);
		tile_id tile_id = creep_tile ? creep_tile : game_st.gfx_tiles.at(index);
		size_t megatile_index = game_st.cv5.at(tile_id.group_index()).megaTileRef[tile_id.subtile_index()];
		size_t ux = pos.x;
		size_t uy = pos.y;
		int flags = game_st.vf4.at(megatile_index).flags[uy / 8 % 4 * 4 + ux / 8 % 4];
		if (flags&MiniTileFlags::High) return 2;
		if (flags&MiniTileFlags::Middle) return 1;
		return 0;
	};

	void reveal_sight_at(xy pos, int range, int reveal_to, bool in_air) {
		log("reveal sight at %d %d\n", pos.x, pos.y);
		log("map is %dx%d\n", game_st.map_width, game_st.map_height);
		int visibility_mask = ~reveal_to;
		int height_mask = 0;
		if (!in_air) {
			int height = get_ground_height_at(pos);
			if (height == 2) height_mask = tile_t::flag_very_high;
			else if (height == 1) height_mask = tile_t::flag_very_high | tile_t::flag_high;
			else height_mask = tile_t::flag_very_high | tile_t::flag_high | tile_t::flag_middle;
		}
		tile_t reveal_tile_mask;
		reveal_tile_mask.visible = visibility_mask;
		reveal_tile_mask.explored = visibility_mask;
		reveal_tile_mask.flags = 0;
		tile_t required_tile_mask = reveal_tile_mask;
		required_tile_mask.flags = height_mask;
		auto&sight_vals = game_st.sight_values.at(range);
		size_t tile_x = (size_t)pos.x / 32;
		size_t tile_y = (size_t)pos.y / 32;
		tile_t*base_tile = &st.tiles[tile_x + tile_y*game_st.map_tile_width];
		if (!in_air) {
			auto*cur = sight_vals.maskdat.data();
			auto*end = cur + sight_vals.min_mask_size;
			for (; cur != end; ++cur) {
				if (tile_x + cur->x >= game_st.map_tile_width) continue;
				if (tile_y + cur->y >= game_st.map_tile_height) continue;
				auto&tile = base_tile[cur->map_index_offset];
				tile.raw &= reveal_tile_mask.raw;
				cur->vision_propagation = tile.raw;
			}
			end += sight_vals.ext_masked_count;
			for (; cur != end; ++cur) {
				cur->vision_propagation = 0xff;
				if (tile_x + cur->x >= game_st.map_tile_width) continue;
				if (tile_y + cur->y >= game_st.map_tile_height) continue;
				bool okay = false;
				okay |= !(cur->prev->vision_propagation&required_tile_mask.raw);
				if (cur->prev_count == 2) okay |= !(cur->prev2->vision_propagation&required_tile_mask.raw);
				if (!okay) continue;
				auto&tile = base_tile[cur->map_index_offset];
				tile.raw &= reveal_tile_mask.raw;
				cur->vision_propagation = tile.raw;
			}
		} else {
			// This seems bugged; even for air units, if you only traverse ext_masked_count nodes,
			// then you will still miss out on the min_mask_size (9) last ones
			auto*cur = sight_vals.maskdat.data();
			auto*end = cur + sight_vals.ext_masked_count;
			for (; cur != end; ++cur) {
				if (tile_x + cur->x >= game_st.map_tile_width) continue;
				if (tile_y + cur->y >= game_st.map_tile_height) continue;
				base_tile[cur->map_index_offset].raw &= reveal_tile_mask.raw;
			}
		}
	};

	void refresh_unit_vision(unit_t*u) {
		if (u->owner >= 8 && !u->parasite_flags) return;
		if (u->unit_type->id == UnitTypes::Terran_Nuclear_Missile) return;
		int visible_to = 0;
		if (visible_to_everyone(u) || (u->unit_type->id == UnitTypes::Powerup_Flag && u->order_type->id == Orders::UnusedPowerup)) visible_to = 0xff;
		else {
			visible_to = st.shared_vision[u->owner] | u->parasite_flags;
			if (u->parasite_flags) {
				visible_to |= u->parasite_flags;
				for (size_t i = 0; i < 12; ++i) {
					if (~u->parasite_flags&(1 << i)) continue;
					visible_to |= st.shared_vision[i];
				}
			}
			log("reveal sight!\n");
			reveal_sight_at(u->sprite->position, unit_sight_range(u), visible_to, u_flying(u));
			log("done\n");
		}
	};

	void update_unit_pathing(unit_t*u) {

		bool refresh_vision = Unit_ExecPathingState(u);
		if (refresh_vision) refresh_unit_vision(u);
		if (u->status_flags&StatusFlags::Completed) {
			if (u->subunit && ~u->unit_type->flags & UnitPrototypeFlags::Subunit) {
				xcept("update_unit_pathing: subunit stuff");
			}
		}
	};

	void UpdateUnitSpriteInfo(unit_t*u) {

	};

	void update_units() {

		// place box/target order cursor/whatever

		--st.order_timer_counter;
		if (!st.order_timer_counter) {
			st.order_timer_counter = 150;
			int v = 0;
			for (unit_t*u : st.visible_units) {
				u->order_queue_timer = v;
				++v;
				if (v == 8) v = 0;
			}
		}
		--st.secondary_order_timer_counter;
		if (!st.secondary_order_timer_counter) {
			st.secondary_order_timer_counter = 300;
			int v = 0;
			for (unit_t*u : st.visible_units) {
				u->secondary_order_timer = v;
				++v;
				if (v == 30) v = 0;
			}
		}

		// some_units_loaded_and_disruption_web begin
		for (unit_t*u : st.visible_units) {
			if (~u->status_flags&StatusFlags::InAir || u->status_flags&StatusFlags::UNKNOWN1) {
				u->status_flags &= ~StatusFlags::CanNotAttack;
				if (~u->status_flags&StatusFlags::IsHallucination && (u->unit_type->id != UnitTypes::Zerg_Overlord || st.upgrade_levels[u->owner][UpgradeTypes::Ventral_Sacs]) && u->unit_type->space_provided) {
					xcept("sub_4EB2F0 loaded unit stuff");
				} else if (u->subunit) {
					u->subunit->status_flags &= ~StatusFlags::CanNotAttack;
				}
			}
		}
		if (st.completed_unit_counts[11][UnitTypes::Spell_Disruption_Web]) {
			xcept("disruption web stuff");
		}
		// some_units_loaded_and_disruption_web end


		for (unit_t*u : st.sight_related_units) {
			xcept("fixme first_sight_related_unit stuff in UpdateUnits");
		}

		for (unit_t*u : st.visible_units) {
			iscript_order_unit = u;
			iscript_unit = u;
			update_unit_pathing(u);
		}

		if (update_tiles) {
			for (unit_t*u : st.scanner_sweep_units) {
				refresh_unit_vision(u);
			}
		}

		for (unit_t*u : st.visible_units) {
			UpdateUnitSpriteInfo(u);
			xcept("...");
		}

		for (unit_t*u : st.visible_units) {
			iscript_order_unit = u;
			iscript_unit = u;
			update_unit_orders(u);
		}

		xcept("...");

		iscript_order_unit = nullptr;
		iscript_unit = nullptr;
	};

	void game_loop() {

		in_game_loop = true;

		if (st.update_tiles_countdown == 0) st.update_tiles_countdown = 100;
		--st.update_tiles_countdown;
		update_tiles = st.update_tiles_countdown == 0;

		update_units();

		in_game_loop = false;

	}

	int lcg_rand(int source) {
		++st.random_counts[source];
		++st.total_random_counts;
		st.lcg_rand_state *= 22695477;
		++st.lcg_rand_state;
		return (st.lcg_rand_state >> 16) & 0x7fff;
	}

	void net_error_string(int str_index) {
		if (str_index) log(" error %d: (insert string here)\n", str_index);
		st.last_net_error = str_index;
	}

	void local_unit_status_error(unit_t* u, int err) {
		log("if local player, display unit status error %d\n", err);
	}

	bool is_in_bounds(const unit_type_t*unit_type, xy pos) {
		if (pos.x - unit_type->dimensions.from.x < 0) return false;
		if (pos.y - unit_type->dimensions.from.y < 0) return false;
		if ((size_t)(pos.x + unit_type->dimensions.to.x) >= game_st.map_width) return false;
		if ((size_t)(pos.y + unit_type->dimensions.to.y) >= game_st.map_height) return false;
// 		size_t ux = pos.x;
// 		size_t uy = pos.y;
// 		if (ux - unit_type->dimensions.from.x >= game_st.map_width) return false;
// 		if (uy - unit_type->dimensions.from.y >= game_st.map_height) return false;
// 		if (ux - unit_type->dimensions.to.x >= game_st.map_width) return false;
// 		if (uy - unit_type->dimensions.to.y >= game_st.map_height) return false;
		return true;
	};
	size_t get_sprite_tile_line_index(int y) {
		int r = y / 32;
		if (r < 0) return 0;
		if ((size_t)r >= game_st.map_tile_height) return game_st.map_tile_height - 1;
		return (size_t)r;
	}
	void add_sprite_to_tile_line(sprite_t*sprite) {
		size_t index = get_sprite_tile_line_index(sprite->position.y);
		bw_insert_list(st.sprites_on_tile_line[index], *sprite);
	}
	void remove_sprite_from_tile_line(sprite_t*sprite) {
		size_t index = get_sprite_tile_line_index(sprite->position.y);
		st.sprites_on_tile_line[index].remove(*sprite);
	}

	void set_sprite_visibility(sprite_t*sprite, int visibility_flags) {
		visibility_flags &= st.local_mask;
		if ((sprite->visibility_flags&visibility_flags) == visibility_flags) return;
		sprite->visibility_flags = visibility_flags;
		for (image_t*i : sprite->images) i->flags |= image_t::flag_redraw;
	}

	void set_image_offset(image_t*image, xy offset) {
		if (image->offset == offset) return;
		image->offset = offset;
		image->flags |= image_t::flag_redraw;
	}

	void set_image_palette_type(image_t*image, int palette_type) {
		image->palette_type = palette_type;
		if (palette_type == 17) {
			// coloring_data might be a union, since this is written
			// using two single-byte writes
			image->coloring_data = 48 | (2 << 8);
		}
		image->flags |= image_t::flag_redraw;
	}

	void set_image_palette_type(image_t*image, image_t*copy_from) {
		if (copy_from->palette_type < 2 || copy_from->palette_type > 7) return;
		set_image_palette_type(image, copy_from->palette_type);
		// seems like it's actually just two values, since this is also
		// written using two single-byte writes
		image->coloring_data = copy_from->coloring_data;
	}

	void hide_image(image_t*image) {
		if (image->flags&image_t::flag_hidden) return;
		image->flags |= image_t::flag_hidden;
	}

	void update_image_offset(image_t*image) {
		xcept("update_image_offset");
	}

	void update_image_frame_index(image_t*image) {
		int frame_index = image->frame_set + image->direction;
		if (image->frame_index != frame_index) {
			image->frame_index = frame_index;
			image->flags |= image_t::flag_redraw;
		}
	}

	void set_image_direction(image_t*image, int direction) {
		if (image->flags & image_t::flag_has_directional_frames) {
			bool facing;
			if (direction <= 16) {
				facing = false;
			} else {
				direction = 32 - direction;
				facing = true;
			}
			if (!!(image->flags & 2) != facing || image->direction != direction) {
				image->direction = direction;
				if (facing) image->flags |= 2;
				else image->flags &= 2;
				set_image_palette_type(image, image->palette_type);
				update_image_frame_index(image);
				if (image->flags & 0x80) update_image_offset(image);
			}
		}
	}

	void update_image_position(image_t*image) {

		xy map_pos = image->sprite->position + image->offset;
		auto&frame = image->grp->frames[image->frame_index];
		if (image->flags&image_t::flag_horizontally_flipped) {
			map_pos.x += image->grp->width / 2 - (frame.right + frame.left);
		} else {
			map_pos.x += frame.left - image->grp->width / 2;
		}
		if (image->flags & image_t::flag_y_frozen) map_pos.y = image->map_position.y;
		else map_pos.y += frame.top - image->grp->height / 2;
		rect grp_bounds = { { 0, 0 },{ frame.right, frame.bottom } };
		xy screen_pos = map_pos - st.viewport.from;
		if (screen_pos.x < 0) {
			grp_bounds.from.x -= screen_pos.x;
			grp_bounds.to.x += screen_pos.x;
		}
		if (screen_pos.y < 0) {
			grp_bounds.from.y -= screen_pos.y;
			grp_bounds.to.y += screen_pos.y;
		}
		if (grp_bounds.to.x > st.viewport.to.x - map_pos.x) grp_bounds.to.x = st.viewport.to.x - map_pos.x;
		if (grp_bounds.to.y > st.viewport.to.y - map_pos.y) grp_bounds.to.y = st.viewport.to.y - map_pos.y;

		image->map_position = map_pos;
		image->screen_position = screen_pos;
		image->grp_bounds = grp_bounds;

	}

	xy get_image_lo_offset(image_t*image, int lo_index, int offset_index) {
		int frame = image->frame_index;
		auto&lo_offsets = global_st.image_lo_offsets.at(image->image_type->id);
		if ((size_t)lo_index >= lo_offsets.size()) xcept("invalid lo index %d\n", lo_index);
		auto&frame_offsets = *lo_offsets[lo_index];
		if ((size_t)frame >= frame_offsets.size()) xcept("image %d lo_index %d does not offsets for frame %d", image->image_type->id, lo_index, frame);
		if ((size_t)offset_index >= frame_offsets[frame].size()) xcept("image %d lo_index %d frame %d does not contain an offset at index %d", image->image_type->id, lo_index, frame, offset_index);
		return frame_offsets[frame][offset_index];
	}

	int get_modified_unit_speed(unit_t*u, int base_speed) {
		int speed = base_speed;
		int mod = 0;
		if (u->stim_timer) ++mod;
		if (u_speed_upgrade(u)) ++mod;
		if (u->ensnare_timer) ++mod;
		if (mod < 0) speed /= 2;
		if (mod > 0) {
			if (u->unit_type->id == UnitTypes::Protoss_Scout || u->unit_type->id == UnitTypes::Hero_Mojo || u->unit_type->id == UnitTypes::Hero_Artanis) speed = (6 << 8) + ((1 << 8) - ((1 << 8) / 3)); // 1707, 6 + 2/3 rounded up (nearest)
			else {
				speed += speed / 2;
				int min_speed = (3 << 8) + (1 << 8) / 3; // 3 + 1/3 rounded down (nearest)
				if (speed < min_speed) speed = min_speed;
			}
		}
		return speed;
	}

	void iscript_set_script(image_t*image, int script_id) {
		auto i = global_st.iscript.scripts.find(script_id);
		if (i == global_st.iscript.scripts.end()) {
			xcept("script %d does not exist", script_id);
		}
		image->iscript_state.current_script = &i->second;
	}

	bool iscript_execute(image_t*image, iscript_state_t&state, bool no_side_effects = false, int*distance_moved = nullptr) {

		if (state.wait) {
			--state.wait;
			return true;
		}

		auto play_frame = [&](int frame_index) {
			if (image->frame_set == frame_index) return;
			image->frame_set = frame_index;
			update_image_frame_index(image);
		};

		auto add_image = [&](int image_id, xy offset, int order) {
			log("add_image %d\n", image_id);
			const image_type_t*image_type = get_image_type(image_id);
			image_t*script_image = image;
			image_t*image = std::get<1>(create_image(image_type, script_image->sprite, offset, 0, order, script_image));
			// Possible behavior change:
			// Broodwar only returns here if the image creation failed, we are returning
			// if it failed or no longer exists (deleted by iscript).
			// Despite that, I doubt there is actual behavior change here, since this stuff
			// would just set fields on an image that no longer exists, and thus wouldn't
			// actually be used for anything.
			if (!image) return (image_t*)nullptr;
			
			if (image->palette_type == 0 && iscript_unit && u_hallucination(iscript_unit)) {
				if (game_st.is_replay || iscript_unit->owner == game_st.local_player) {
					set_image_palette_type(image, image_t::palette_type_hallucination);
					image->coloring_data = 0;
				}
			}
			if (image->flags & image_t::flag_has_directional_frames) {
				int dir = script_image->flags & image_t::flag_horizontally_flipped ? 32 - script_image->direction : script_image->direction;
				set_image_direction(image, dir);
			}
			update_image_frame_index(image);
			if (iscript_unit && (u_grounded_building(iscript_unit) || u_completed(iscript_unit))) {
				if (!image_type->draw_if_cloaked) {
					hide_image(image);
				} else if (image->palette_type==0) {
					set_image_palette_type(image, script_image);
				}
			}
			return image;
		};

		const int*program_data = global_st.iscript.program_data.data();
		const int*p = program_data + state.program_counter;
		while (true) {
			using namespace iscript_opcodes;
			size_t pc = p - program_data;
			int opc = *p++ - 0x808091;
			log("iscript: %04x: opc %d\n", pc, opc);
			int a, b, c;
			switch (opc) {
			case opc_playfram:
				a = *p++;
				if (no_side_effects) break;
				play_frame(a);
				break;
			case opc_playframtile:
				a = *p++;
				if (no_side_effects) break;
				if ((size_t)a + game_st.tileset_index < image->grp->frames.size()) play_frame(a + game_st.tileset_index);
				break;
			case opc_sethorpos:
				a = *p++;
				if (no_side_effects) break;
				if (image->offset.x != a) {
					image->offset.x = a;
					image->flags |= image_t::flag_redraw;
				}
				break;
			case opc_setvertpos:
				a = *p++;
				if (no_side_effects) break;
				if (!iscript_unit || !(iscript_unit->status_flags&(StatusFlags::Completed | StatusFlags::GroundedBuilding))) {
					if (image->offset.y != a) {
						image->offset.y = a;
						image->flags |= image_t::flag_redraw;
					}
				}
				break;
			case opc_setpos:
				a = *p++;
				b = *p++;
				if (no_side_effects) break;
				set_image_offset(image, xy(a, b));
				break;
			case opc_wait:
				state.wait = *p++ - 1;
				state.program_counter = p - program_data;
				return true;
			case opc_waitrand:
				a = *p++;
				b = *p++;
				if (no_side_effects) break;
				state.wait = a + (lcg_rand(3) % (b - a + 1)) - 1;
				return true;
			case opc_goto:
				p = program_data + *p;
				break;
			case opc_imgol:
			case opc_imgul:
				a = *p++;
				b = *p++;
				c = *p++;
				if (no_side_effects) break;
				add_image(a, image->offset + xy(b, c), opc == opc_imgol ? image_order_above : image_order_below);
				break;
			case opc_imgolorig:
			case opc_switchul:
				a = *p++;
				if (no_side_effects) break;
				if (image_t*new_image = add_image(a, xy(), opc == opc_imgolorig ? image_order_above : image_order_below)) {
					if (~new_image->flags & 0x80) {
						new_image->flags |= 0x80;
						update_image_offset(image);
					}
				}
				break;
// 			case opc_imgoluselo:
// 				a = *p++;
// 				b = *p++;
// 				if (no_side_effects) break;
// 
// 				break;

			case opc_followmaingraphic:
				if (no_side_effects) break;
				if (image_t*main_image = image->sprite->main_image) {
					if (main_image->frame_index == image->frame_index && ((main_image->flags & 2) == (image->flags & 2))) {
						image->frame_set = main_image->frame_set;
						image->direction = main_image->direction;
						if (main_image->flags & 2) image->flags |= 2;
						else image->flags &= ~2;
					}
				}
				break;

			case opc_move:
				a = *p++;
				if (distance_moved) {
					*distance_moved = get_modified_unit_speed(iscript_unit, a << 8);
				}
				if (no_side_effects) break;
				xcept("opc_move");
				break;
			default:
				xcept("iscript: unhandled opcode %d", opc);
			}
		}
		
	}

	bool iscript_run_anim(image_t*image, int new_anim) {
		using namespace iscript_anims;
		int old_anim = image->iscript_state.animation;
		if (new_anim == Death && old_anim == Death) return true;
		if (~image->flags & image_t::flag_has_iscript_animations && new_anim != Init && new_anim != Death) return true;
		if ((new_anim == Walking || new_anim == IsWorking) && new_anim == old_anim) return true;
		if (new_anim == GndAttkRpt && old_anim != GndAttkRpt) {
			if (old_anim != GndAttkInit) new_anim = GndAttkInit;
		}
		if (new_anim == AirAttkRpt && old_anim != AirAttkRpt) {
			if (old_anim != AirAttkInit) new_anim = AirAttkInit;
		}
		auto*script = image->iscript_state.current_script;
		if (!script) xcept("attempt to start animation without a script");
		auto&anims_pc = script->animation_pc;
		if ((size_t)new_anim >= anims_pc.size()) xcept("script %d does not have animation %d", script->id, new_anim);
		image->iscript_state.animation = new_anim;
		image->iscript_state.program_counter = anims_pc[new_anim];
		image->iscript_state.return_address = 0;
		image->iscript_state.wait = 0;
		return iscript_execute(image, image->iscript_state);
	}

	bool iscript_execute_sprite(sprite_t*sprite) {
		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t*image = *i++;
			iscript_execute(image, image->iscript_state);
		}
		if (!sprite->images.empty()) return true;

		remove_sprite_from_tile_line(sprite);
		bw_insert_list(st.free_sprites, *sprite);
		
		return false;
	}

	void initialize_image(image_t*image, const image_type_t*image_type, sprite_t*sprite, xy offset) {
		image->image_type = image_type;
		image->grp = global_st.image_grp[image_type->id];
		int flags = 0;
		if (image_type->has_directional_frames) flags |= image_t::flag_has_directional_frames;
		if (image_type->is_clickable) flags |= 0x20;
		image->flags = flags;
		image->frame_set = 0;
		image->direction = 0;
		image->frame_index = 0;
		image->sprite = sprite;
		image->offset = offset;
		image->grp_bounds = { {0,0},{0,0} };
		image->coloring_data = 0;
		image->iscript_state.current_script = nullptr;
		image->iscript_state.program_counter = 0;
		image->iscript_state.return_address = 0;
		image->iscript_state.animation = 0;
		image->iscript_state.wait = 0;
		int palette_type = image_type->palette_type;
		if (palette_type == 14) image->coloring_data = sprite->owner;
		if (palette_type == 9) {
			//images_dat.remapping[image_id];
			// some color shift stuff based on the tileset
			image->coloring_data = 0; // fixme
		}
	};

	enum {
		image_order_top,
		image_order_bottom,
		image_order_above,
		image_order_below
	};
	std::pair<bool, image_t*> create_image(int image_id, sprite_t*sprite, xy offset, int direction, int order, image_t*relimg = nullptr) {
		if ((size_t)image_id >= 999) xcept("attempt to create image with invalid id %d", image_id);
		return create_image(get_image_type(image_id), sprite, offset, direction, order, relimg);
	}
	std::pair<bool,image_t*> create_image(const image_type_t*image_type, sprite_t*sprite, xy offset, int direction, int order, image_t*relimg = nullptr) {
		if (!image_type)  xcept("attempt to create image of null type");
		log("create image %d\n", image_type->id);

		if (st.free_images.empty()) return { false, nullptr };
		image_t*image = &st.free_images.front();
		st.free_images.pop_front();

		if (sprite->images.empty()) {
			sprite->main_image = image;
			sprite->images.push_front(*image);
		} else {
			if (order == image_order_top) sprite->images.push_front(*image);
			else if (order == image_order_bottom) sprite->images.push_back(*image);
			else {
				if (!relimg) relimg = sprite->main_image;
				if (order == image_order_above) sprite->images.insert(sprite->images.iterator_to(*relimg), *image);
				else sprite->images.insert(++sprite->images.iterator_to(*relimg), *image);
			}
		}
		initialize_image(image, image_type, sprite, offset);
		int palette_type = image->image_type->palette_type;
		set_image_palette_type(image, palette_type);
		if (image->image_type->has_iscript_animations) image->flags |= image_t::flag_has_iscript_animations;
		else image->flags &= image_t::flag_has_iscript_animations;
		iscript_set_script(image, image->image_type->iscript_id);
		// Broodwar does not have this check. The problem is that if init ends, then
		// the image is strictly speaking no longer valid. Broodwar doesn't care
		// and just keeps using the pointer, which is fine as long as the image
		// is not re-used, and no such code path seems to exist.
		// Nonetheless, I don't find it acceptable to return a pointer to an object that 
		// no longer exists.
		if (!iscript_run_anim(image, iscript_anims::Init)) xcept("iscript init deleted the image, remove me when this throws");
		//if (!iscript_run_anim(image, iscript_anims::Init)) return { true, nullptr };
		update_image_position(image);
		set_image_direction(image, direction);
		return { true, image };
	}

	sprite_t*create_sprite(const sprite_type_t*sprite_type, xy pos, int owner) {
		if (!sprite_type)  xcept("attempt to create sprite of null type");
		log("create sprite %d\n", sprite_type->id);

		if (st.free_sprites.empty()) return nullptr;
		sprite_t*sprite = &st.free_sprites.front();
		st.free_sprites.pop_front();

		auto initialize_sprite = [&]() {
			if ((size_t)pos.x >= game_st.map_width || (size_t)pos.y >= game_st.map_height) return false;
			sprite->owner = owner;
			sprite->sprite_type = sprite_type;
			sprite->flags = 0;
			sprite->position = pos;
			sprite->visibility_flags = ~0;
			sprite->elevation_level = 4;
			sprite->selection_timer = 0;
			if (!sprite_type->flags) {
				sprite->flags |= SpriteFlags::Hidden;
				set_sprite_visibility(sprite, 0);
			}
			sprite->images.clear();
			if (!create_image(sprite_type->image, sprite, { 0,0 }, 0, image_order_above).first) return false;
			sprite->width = std::min(sprite->main_image->grp->width, 0xff);
			sprite->height = std::min(sprite->main_image->grp->width, 0xff);
			return true;
		};

		if (!initialize_sprite()) {
			bw_insert_list(st.free_sprites, *sprite);
			return nullptr;
		}
		add_sprite_to_tile_line(sprite);

		return sprite;
	}

	bool initialize_flingy(flingy_t*f, const flingy_type_t*flingy_type, xy pos, int owner, int direction) {
		f->flingy_type = flingy_type;
		f->movement_flags = 0;
		f->current_speed2 = 0;
		f->flingy_top_speed = flingy_type->top_speed;
		f->flingy_acceleration = flingy_type->acceleration;
		f->flingy_turn_rate = flingy_type->turn_rate;
		f->flingy_movement_type = flingy_type->movement_type;

		f->position = pos;
		f->halt = { pos.x << 8, pos.y << 8 };

		// f->move_target.pos will be uninitialized here if this is a new unit/flingy
		if (f->move_target.pos != pos) {
			f->move_target.pos = pos;
			f->move_target.unit = nullptr;
			f->next_movement_waypoint = pos;
			f->movement_flags = 1;
		}
		if (f->next_target_waypoint != pos) {
			f->next_target_waypoint = pos;
		}
		f->current_direction1 = direction;
		f->velocity_direction1 = direction;

		f->sprite = create_sprite(flingy_type->sprite, pos, owner);
		if (!f->sprite) return false;
		int dir = f->current_direction1;
		for (image_t*i : f->sprite->images) {
			set_image_direction(i, dir);
		}

		return true;
	}

	void update_unit_speed_upgrades(unit_t*u) {
		auto speed_upgrade = [&]() {
			switch (u->unit_type->id) {
			case UnitTypes::Terran_Vulture:
			case UnitTypes::Hero_Jim_Raynor_Vulture:
				return UpgradeTypes::Ion_Thrusters;
			case UnitTypes::Zerg_Overlord:
				return UpgradeTypes::Pneumatized_Carapace;
			case UnitTypes::Zerg_Zergling:
				return UpgradeTypes::Metabolic_Boost;
			case UnitTypes::Zerg_Hydralisk:
				return UpgradeTypes::Muscular_Augments;
			case UnitTypes::Protoss_Zealot:
				return UpgradeTypes::Leg_Enhancements;
			case UnitTypes::Protoss_Scout:
				return UpgradeTypes::Gravitic_Thrusters;
			case UnitTypes::Protoss_Shuttle:
				return UpgradeTypes::Gravitic_Drive;
			case UnitTypes::Protoss_Observer:
				return UpgradeTypes::Gravitic_Boosters;
			case UnitTypes::Zerg_Ultralisk:
				return UpgradeTypes::Anabolic_Synthesis;
			};
			return UpgradeTypes::None;
		};
		bool cooldown = false;
		if (u->unit_type->id == UnitTypes::Hero_Devouring_One) cooldown = true;
		if (u->unit_type->id == UnitTypes::Zerg_Zergling && st.upgrade_levels[u->owner][UpgradeTypes::Adrenal_Glands]) cooldown = true;
		bool speed = false;
		int speed_upg = speed_upgrade();
		if (speed_upg != UpgradeTypes::None && st.upgrade_levels[u->owner][speed_upg]) speed = true;
		if (u->unit_type->id == UnitTypes::Hero_Hunter_Killer) speed = true;
		if (u->unit_type->id == UnitTypes::Hero_Yggdrasill) speed = true;
		if (u->unit_type->id == UnitTypes::Hero_Fenix_Zealot) speed = true;
		if (u->unit_type->id == UnitTypes::Hero_Mojo) speed = true;
		if (u->unit_type->id == UnitTypes::Hero_Artanis) speed = true;
		if (u->unit_type->id == UnitTypes::Zerg_Lurker) speed = true;
		if (cooldown!=u_cooldown_upgrade(u) || speed!=u_speed_upgrade(u)) {
			if (cooldown) u->status_flags |= unit_t::status_flag_cooldown_upgrade;
			if (speed) u->status_flags |= unit_t::status_flag_speed_upgrade;
			update_unit_speed(u);
		}
	}

	void update_unit_speed(unit_t*u) {
		
		int movement_type = u->unit_type->flingy->movement_type;
		if (movement_type != 0 && movement_type != 1) {
			if (u->flingy_movement_type == 2) {
				image_t*image = u->sprite->main_image;
				if (!image) xcept("null image");
				auto*script = image->iscript_state.current_script;
				auto&anims_pc = script->animation_pc;
				int anim = iscript_anims::Walking;
				// BW just returns if the animation doesn't exist,
				// so this could be changed to a return statement if it throws
				if ((size_t)anim >= anims_pc.size()) xcept("script %d does not have animation %d", script->id, anim);
				iscript_unit_setter ius(this, u);
				iscript_state_t st;
				st.current_script = script;
				st.animation = anim;
				st.program_counter = anims_pc[anim];
				st.return_address = 0;
				st.wait = 0;
				int total_distance_moved = 0;
				for (int i = 0; i < 32; ++i) {
					int distance_moved = 0;
					iscript_execute(image, st, true, &distance_moved);
					int mod = 0;
					if (u->stim_timer) ++mod;
					if (u_speed_upgrade(u)) ++mod;
					if (u->ensnare_timer) --mod;
					if (mod < 0) distance_moved -= distance_moved / 4;
					if (mod > 0) distance_moved *= 2;
					total_distance_moved += distance_moved;
				}
				int avg_distance_moved = total_distance_moved / 32;
				u->flingy_top_speed = avg_distance_moved;
				log("avg_distance_moved is %d\n", avg_distance_moved);
			}
		} else {
			xcept("fixme update_unit_speed");
		}

	}

	void increment_unit_counts(unit_t*u, int count) {
		if (u_hallucination(u)) return;
		if (ut_is_turret(u)) return;

		st.unit_counts[u->owner][u->unit_type->id] += count;
		int supply_required = u->unit_type->supply_required;
		if (u->unit_type->staredit_group_flags & GroupFlags::Zerg) {
			const unit_type_t*ut = u->unit_type;
			if (ut->id==UnitTypes::Zerg_Egg || ut->id==UnitTypes::Zerg_Cocoon || ut->id==UnitTypes::Zerg_Lurker_Egg) {
				ut = u->build_queue[u->build_queue_slot];
				supply_required = ut->supply_required;
				if (ut_two_units_in_one_egg(u)) supply_required *= 2;
			} else {
				if (ut_flyer(u) && !u_completed(u)) supply_required *= 2;
			}
			st.supply_used[0][u->owner] += supply_required * count;
		} else if (u->unit_type->staredit_group_flags & GroupFlags::Terran) {
			st.supply_used[1][u->owner] += supply_required * count;
		} else if (u->unit_type->staredit_group_flags & GroupFlags::Protoss) {
			st.supply_used[2][u->owner] += supply_required * count;
		}
		if (u->unit_type->staredit_group_flags & GroupFlags::Factory) st.factory_counts[u->owner] += count;
		if (u->unit_type->staredit_group_flags & GroupFlags::Men) {
			st.non_building_counts[u->owner] += count;
		} else if (u->unit_type->staredit_group_flags & GroupFlags::Building) st.building_counts[u->owner] += count;
		else if (u->unit_type->id == UnitTypes::Zerg_Egg || u->unit_type->id == UnitTypes::Zerg_Cocoon || u->unit_type->id == UnitTypes::Zerg_Lurker_Egg) {
			st.non_building_counts[u->owner] += count;
		}
		if (st.unit_counts[u->owner][u->unit_type->id] < 0) st.unit_counts[u->owner][u->unit_type->id] = 0;
	}

	size_t unit_finder_lower_bound_index(const a_vector<std::pair<unit_t*, int>>& unit_finder, int value) {
		auto i = std::lower_bound(unit_finder.begin(), unit_finder.begin() + st.unit_finder_size, value, [](const auto& a, int value) {
			return a.second < value;
		});
		return i - unit_finder.begin();
	}
	size_t unit_finder_upper_bound_index(const a_vector<std::pair<unit_t*, int>>& unit_finder, int value) {
		auto i = std::upper_bound(unit_finder.begin(), unit_finder.begin() + st.unit_finder_size, value, [](int value, const auto& b) {
			return b.second < value;
		});
		return i - unit_finder.begin();
	}

	void insert_unit_finder_x(size_t index, unit_t* u, int value) {
		for (size_t i = st.unit_finder_size; i-- > index;) {
			unit_t* n = st.unit_finder_x[i].first;
			if (n->unit_finder_left_index == i) ++n->unit_finder_left_index;
			else if (n->unit_finder_right_index == i) ++n->unit_finder_right_index;
		}
		std::copy_backward(st.unit_finder_x.begin() + index, st.unit_finder_x.begin() + st.unit_finder_size, st.unit_finder_x.begin() + st.unit_finder_size + 1);
		st.unit_finder_x[index] = std::make_pair(u, value);
	}
	void insert_unit_finder_y(size_t index, unit_t* u, int value) {
		for (size_t i = st.unit_finder_size; i-- > index;) {
			unit_t* n = st.unit_finder_y[i].first;
			if (n->unit_finder_top_index == i) ++n->unit_finder_top_index;
			else if (n->unit_finder_bottom_index == i) ++n->unit_finder_bottom_index;
		}
		std::copy_backward(st.unit_finder_y.begin() + index, st.unit_finder_y.begin() + st.unit_finder_size, st.unit_finder_y.begin() + st.unit_finder_size + 1);
		st.unit_finder_y[index] = std::make_pair(u, value);
	}

	void update_unit_finder(unit_t*u) {
		if (ut_is_turret(u)) return;
		
		xy top_left = u->position - u->unit_type->dimensions.from;
		xy bottom_right = u->position + u->unit_type->dimensions.to;

		size_t left_index = unit_finder_lower_bound_index(st.unit_finder_x, top_left.x);
		size_t top_index = unit_finder_lower_bound_index(st.unit_finder_y, top_left.y);
		u->unit_finder_left_index = left_index;
		u->unit_finder_top_index = top_index;
		insert_unit_finder_x(left_index, u, top_left.x);
		insert_unit_finder_y(top_index, u, top_left.y);
		++st.unit_finder_size;

		size_t right_index = unit_finder_lower_bound_index(st.unit_finder_x, bottom_right.x);
		size_t bottom_index = unit_finder_lower_bound_index(st.unit_finder_y, bottom_right.y);
		u->unit_finder_right_index = right_index;
		u->unit_finder_bottom_index = bottom_index;
		insert_unit_finder_x(right_index, u, bottom_right.x);
		insert_unit_finder_y(bottom_index, u, bottom_right.y);
		++st.unit_finder_size;

		log("indexes is %d %d %d %d\n", left_index, top_index, right_index, bottom_index);
		
	}

	bool initialize_unit_type(unit_t*u, const unit_type_t*unit_type, xy pos, int owner) {

		iscript_unit_setter ius(this, u);
		if (!initialize_flingy(u, unit_type->flingy, pos, owner, 0)) return false;

		u->owner = owner;
		u->order_type = get_order_type(Orders::Fatal);
		u->order_state = 0;
		u->order_signal = 0;
		u->main_order_timer = 0;
		u->ground_weapon_cooldown = 0;
		u->air_weapon_cooldown = 0;
		u->spell_cooldown = 0;
		u->order_target.unit = nullptr;
		u->order_target.pos = { 0,0 };
		u->unit_type = unit_type;
		u->resource_type = 0;
		u->secondary_order_timer = 0;
		
		if (!iscript_execute_sprite(u->sprite)) {
			xcept("initialize_unit_type: iscript removed the sprite (if this throws, then Broodwar would crash)");
			u->sprite = nullptr;
		}
		u->last_attacking_player = 8;
		u->shield_points = u->unit_type->shield_points << 8;
		if (u->unit_type->id == UnitTypes::Protoss_Shield_Battery) u->energy = 100 << 8;
		else u->energy = unit_max_energy(u) / 4;
		// u->ai_action_flag = 0;
		u->sprite->elevation_level = unit_type->elevation_level;
		u_set_status_flag(u, unit_t::status_flag_grounded_building, ut_building(u));
		u_set_status_flag(u, unit_t::status_flag_flying, ut_flyer(u));
		u_set_status_flag(u, unit_t::status_flag_non_building, ut_non_building(u));
		u_set_status_flag(u, (unit_t::status_flags_t)0x20000, ut_flag(u, (unit_type_t::flags_t)0x8000000));
		u_set_status_flag(u, (unit_t::status_flags_t)0x100000, !ut_flyer(u));
		if (u->unit_type->elevation_level < 12) u->pathing_flags |= 1;
		else u->pathing_flags &= ~1;
		if (ut_building(u)) {
			u->building.addon = nullptr;
			u->building.addon_build_type = nullptr;
			u->building.upgrade_research_time = 0;
			u->building.tech_type = nullptr;
			u->building.upgrade_type = nullptr;
			u->building.larva_timer = 0;
			u->building.landing_timer = 0;
			u->building.creep_timer = 0;
			u->building.upgrade_level = 0;
		}
		u->path = nullptr;
		u->movement_state = 0;
		u->recent_order_timer = 0;
		u_set_status_flag(u, unit_t::status_flag_invincible, ut_invincible(u));

		// u->building_overlay_state = 0; xcept("fixme building overlay state needs damage graphic?");

		if (u->unit_type->build_time == 0) {
			u->remaining_build_time = 1;
			u->hp_construction_rate = 1;
		} else {
			u->remaining_build_time = u->unit_type->build_time;
			u->hp_construction_rate = (u->unit_type->hitpoints - u->unit_type->hitpoints / 10 + u->unit_type->build_time - 1) / u->unit_type->build_time;
			if (u->hp_construction_rate == 0) u->hp_construction_rate = 1;
		}
		if (u->unit_type->has_shield && u_grounded_building(u)) {
			int max_shields = u->unit_type->shield_points << 8;
			u->shield_points = max_shields / 10;
			if (u->unit_type->build_time == 0) {
				u->shield_construction_rate = 1;
			} else {
				u->shield_construction_rate = (max_shields - u->shield_points) / u->unit_type->build_time;
				if (u->shield_construction_rate == 0) u->shield_construction_rate = 1;
			}
		}
		update_unit_speed_upgrades(u);
		update_unit_speed(u);

		return true;
	}

	void destroy_unit(unit_t*u) {

		xcept("destroy unit %p\n", u);
		
	}

	unit_t*create_unit(const unit_type_t*unit_type, xy pos, int owner) {
		if (!unit_type) xcept("attempt to create unit of null type");

		if (in_game_loop) {
			lcg_rand(14);
		}
		auto get_new = [&](const unit_type_t*unit_type) {
			if (st.free_units.empty()) {
				net_error_string(61); // Cannot create more units
				return (unit_t*)nullptr;
			}
			if (!is_in_bounds(unit_type, pos)) {
				net_error_string(0);
				return (unit_t*)nullptr;
			}
			unit_t*u = st.free_units.front();
			auto initialize_unit = [&]() {

				u->order_queue.clear();

				u->auto_target_unit = nullptr;
				u->connected_unit = nullptr;

				u->order_queue_count = 0;
				u->order_queue_timer = 0;
				u->unknown_0x086 = 0;
				u->attack_notify_timer = 0;
				u->displayed_unit_id = 0;
				u->last_event_timer = 0;
				u->last_event_color = 0;
				//u->unused_0x08C = 0;
				u->rank_increase = 0;
				u->kill_count = 0;

				u->remove_timer = 0;
				u->defense_matrix_damage = 0;
				u->defense_matrix_timer = 0;
				u->stim_timer = 0;
				u->ensnare_timer = 0;
				u->lockdown_timer = 0;
				u->irradiate_timer = 0;
				u->stasis_timer = 0;
				u->plague_timer = 0;
				u->storm_timer = 0;
				u->irradiated_by = nullptr;
				u->irradiate_owner = 0;
				u->parasite_flags = 0;
				u->cycle_counter = 0;
				u->is_blind = 0;
				u->maelstrom_timer = 0;
				u->unused_0x125 = 0;
				u->acid_spore_count = 0;
				for (auto&v : u->acid_spore_time) v = 0;
				u->status_flags = 0;
				u->user_action_flags = 0;
				u->pathing_flags = 0;
				u->previous_hp = 1;
				u->ai = nullptr;

				if (!initialize_unit_type(u, unit_type, pos, owner)) return false;

				u->build_queue.fill(nullptr);
				u->unit_id_generation = (u->unit_id_generation + 1) % (1 << 5);
				auto produces_units = [&]() {
					if (u->unit_type->id == UnitTypes::Terran_Command_Center) return true;
					if (u->unit_type->id == UnitTypes::Terran_Barracks) return true;
					if (u->unit_type->id == UnitTypes::Terran_Factory) return true;
					if (u->unit_type->id == UnitTypes::Terran_Starport) return true;
					if (u->unit_type->id == UnitTypes::Zerg_Infested_Command_Center) return true;
					if (u->unit_type->id == UnitTypes::Zerg_Hatchery) return true;
					if (u->unit_type->id == UnitTypes::Zerg_Lair) return true;
					if (u->unit_type->id == UnitTypes::Zerg_Hive) return true;
					if (u->unit_type->id == UnitTypes::Protoss_Nexus) return true;
					if (u->unit_type->id == UnitTypes::Protoss_Gateway) return true;
					return false;
				};
				if (!is_frozen_or_flying(u) || u_completed(u)) {
					if (produces_units()) u->current_button_set = UnitTypes::Factories;
					else u->current_button_set = UnitTypes::Buildings;
				}
				if (in_game_loop) u->wireframe_randomizer = lcg_rand(15);
				else u->wireframe_randomizer = 0;
				if (ut_is_turret(u)) u->hp = 1;
				else u->hp = u->unit_type->hitpoints / 10;
				if (u_grounded_building(u)) u->order_type = get_order_type(Orders::Nothing);
				else u->order_type = u->unit_type->human_ai_idle;
				// secondary_order_id is uninitialized
				if (u->secondary_order_id != Orders::Nothing) {
					u->secondary_order_id = Orders::Nothing;
					u->secondary_order_unk_a = 0;
					u->secondary_order_unk_b = 0;
					u->current_build_unit = nullptr;
					u->secondary_order_state = 0;
				}
				u->unit_finder_left_index = -1;
				u->unit_finder_top_index = -1;
				u->unit_finder_right_index = -1;
				u->unit_finder_bottom_index = -1;
				st.player_units[owner].push_front(*u);
				increment_unit_counts(u, 1);

				if (u_grounded_building(u)) {
					update_unit_finder(u);
				} else {
					if (unit_type->id==UnitTypes::Terran_Vulture || unit_type->id==UnitTypes::Hero_Jim_Raynor_Vulture) {
						u->vulture.spider_mine_count = 0;
					}
					u->sprite->flags |= sprite_t::flag_hidden;
					set_sprite_visibility(u->sprite, 0);
				}
				u->visibility_flags = ~0;
				if (ut_is_turret(u)) {
					u->sprite->flags |= 0x10;
				} else {
					if (~u->sprite->flags & sprite_t::flag_hidden) {
						refresh_unit_vision(u);
					}
				}

				return true;
			};
			if (!initialize_unit()) {
				net_error_string(62); // Unable to create unit
				return (unit_t*)nullptr;
			}
			st.free_units.pop_front();
			return u;
		};
		unit_t*u = get_new(unit_type);
		if (u_grounded_building(u)) bw_insert_list(st.visible_units, *u);
		else bw_insert_list(st.hidden_units, *u);

		if (unit_type->id < UnitTypes::Terran_Command_Center && unit_type->turret_unit_type) {
			unit_t*su = get_new(unit_type->turret_unit_type);
			if (!su) {
				destroy_unit(u);
				return nullptr;
			}
			u->subunit = su;
			su->subunit = u;
			set_image_offset(u->sprite->main_image, get_image_lo_offset(u->sprite->main_image, 0, 2));
		} else {
			u->subunit = nullptr;
		}

		return u;
	}

	unit_t*create_unit(int unit_type_id, xy pos, int owner) {
		if ((size_t)unit_type_id >= 228) xcept("attempt to create unit with invalid id %d", unit_type_id);
		return create_unit(get_unit_type(unit_type_id), pos, owner);
	}

	void replace_sprite_image(sprite_t*sprite, int image_id, int direction) {
		bool was_selected = !!(sprite->flags & sprite_t::flag_selected);
		// if (was_selected) remove_selection_circle_and_hp_bar();
		// else remove_selection_circle();

		// other stuff...
	}

	void apply_unit_effects(unit_t*u) {
		if (u->defense_matrix_timer) {
			xcept("apply_defensive_matrix");
		}
		if (u->lockdown_timer) {
			u->lockdown_timer = 0;
			xcept("lockdown_hit");
		}
		if (u->maelstrom_timer) {
			u->maelstrom_timer = 0;
			xcept("set_maelstrom_timer");
		}
		if (u->irradiate_timer) {
			xcept("apply_irradiate");
		}
		if (u->ensnare_timer) {
			u->ensnare_timer = 0;

		}
	}

	void set_construction_graphic(unit_t*u, bool animated) {

		bool requires_detector_or_cloaked = u_requires_detector(u) || u_cloaked(u);
		int coloring_data = 0;
		if (requires_detector_or_cloaked) {
			coloring_data = u->sprite->main_image->coloring_data;
		}
		iscript_unit_setter ius(this, u);
		int construction_animation = u->unit_type->construction_animation;
		if (!animated || construction_animation == 0) construction_animation = u->sprite->sprite_type->image->id;
		replace_sprite_image(u->sprite, construction_animation, u->current_direction1);

		if (requires_detector_or_cloaked) {
			// some stuff...
		}

		apply_unit_effects(u);

	}

	void set_unit_direction(unit_t* u, int direction) {
		
		u->velocity_direction1 = direction;
		u->current_direction1 = direction;
		u->current_direction2 = direction;
		u->current_speed = global_st.direction_table[direction] * u->current_speed1 / 256;
		if (u->next_target_waypoint != u->sprite->position) {
			u->next_target_waypoint = u->sprite->position;
		}
		for (image_t*img : u->sprite->images) {
			set_image_direction(img, direction);
		}

	}

	void finish_building_unit(unit_t*u) {
		if (u->remaining_build_time) {
			u->hp = u->unit_type->hitpoints;
			u->shield_points = u->unit_type->shield_points << 8;
			u->remaining_build_time = 0;
		}
		set_current_button_set(u, u->unit_type->id);
		if (u_grounded_building(u)) {
			u->parasite_flags = 0;
			u->is_blind = false;
			set_construction_graphic(u, false);
		} else {
			if (u_non_building(u)) {
				int dir = u->unit_type->unit_direction;
				if (dir == 32) dir = lcg_rand(36) % 32;
				set_unit_direction(u, dir * 8);
			}
			if (u->unit_type->id >= UnitTypes::Special_Floor_Missile_Trap && u->unit_type->id <= UnitTypes::Special_Right_Wall_Flame_Trap) {
				xcept("some trap doodad stuff");
			}
		}

	}

	bool place_initial_unit(unit_t* u) {
		if (u->sprite->flags & SpriteFlags::Hidden) {
			// implement me
		}
		return true;
	}

	void add_completed_unit(int count, unit_t* u, bool increment_score) {
		if (u_hallucination(u)) return;
		if (ut_is_turret(u)) return;

		st.completed_unit_counts[u->owner][u->unit_type->id] += count;
		if (u->unit_type->staredit_group_flags & GroupFlags::Zerg) {
			st.supply_available[0][u->owner] += u->unit_type->supply_provided * count;
		} else if (u->unit_type->staredit_group_flags & GroupFlags::Terran) {
			st.supply_available[1][u->owner] += u->unit_type->supply_provided * count;
		} else if (u->unit_type->staredit_group_flags & GroupFlags::Protoss) {
			st.supply_available[2][u->owner] += u->unit_type->supply_provided * count;
		}

		if (u->unit_type->staredit_group_flags & GroupFlags::Factory) {
			st.completed_factory_counts[u->owner] += count;
		}
		if (u->unit_type->staredit_group_flags & GroupFlags::Men) {
			st.completed_building_counts[u->owner] += count;
		} else if (u->unit_type->staredit_group_flags & GroupFlags::Building) {
			st.completed_building_counts[u->owner] += count;
		}
		if (increment_score) {
			if (u->owner != 11) {
				if (u->unit_type->staredit_group_flags & GroupFlags::Men) {
					bool morphed = false;
					if (u->unit_type->id == UnitTypes::Zerg_Guardian) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Devourer) morphed = true;
					if (u->unit_type->id == UnitTypes::Protoss_Dark_Archon) morphed = true;
					if (u->unit_type->id == UnitTypes::Protoss_Archon) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Lurker) morphed = true;
					if (!morphed) st.total_non_buildings_ever_completed[u->owner] += count;
					st.unit_score[u->owner] += u->unit_type->build_score * count;
				} else if (u->unit_type->staredit_group_flags & GroupFlags::Building) {
					bool morphed = false;
					if (u->unit_type->id == UnitTypes::Zerg_Lair) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Hive) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Greater_Spire) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Spore_Colony) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Sunken_Colony) morphed = true;
					if (!morphed) st.total_buildings_ever_completed[u->owner] += count;
					st.building_score[u->owner] += u->unit_type->build_score * count;
				}
			}
		}

		if (st.completed_unit_counts[u->owner][u->unit_type->id] < 0) st.completed_unit_counts[u->owner][u->unit_type->id] = 0;

	}

	void remove_queued_order(unit_t* u, order_t* o) {
		if (o->order_type->highlight != -1) --u->order_queue_count;
		if (u->order_queue_count == -1) u->order_queue_count = 0;
		u->order_queue.remove(*o);
		bw_insert_list(st.free_orders, *o);
		--st.allocated_order_count;
	}

	void queue_order(unit_t* u, const order_type_t* order_type, order_t* insert_after, order_target target) {
		auto get_new = [&](const order_type_t* order_type, order_target target) {
			if (st.free_orders.empty()) return (order_t*)nullptr;
			order_t* o = st.free_orders.front();
			st.free_orders.pop_front();
			++st.allocated_order_count;
			o->order_type = order_type;
			o->target = target;
			return o;
		};
		order_t* o = get_new(order_type, target);
		if (!o) {
			local_unit_status_error(u, 872);
			return;
		}
		if (o->order_type->highlight != -1) ++u->order_queue_count;
		if (insert_after) u->order_queue.insert(++u->order_queue.iterator_to(*insert_after), *o);
		else bw_insert_list(u->order_queue, *o);
	}

	void set_queued_order(unit_t* u, bool interrupt, const order_type_t* order_type, order_target target) {
		if (u->order_type->id == Orders::Die) return;
		while (!u->order_queue.empty()) {
			order_t* o = u->order_queue.back();
			if (!o) break;
			if ((!interrupt || !o->order_type->can_be_interrupted) && o->order_type != order_type) break;
			remove_queued_order(u, o);
		}
		if (order_type->id == Orders::Cloak) {
			xcept("cloak fixme");
		} else {
			queue_order(u, order_type, nullptr, target);
		}
	}

	void iscript_run_to_idle(unit_t* u) {
		u->status_flags &= ~unit_t::status_flag_iscript_nobrk;
		u->sprite->flags &= ~sprite_t::flag_iscript_nobrk;
		iscript_unit_setter ius(this, u);
		int anim = -1;
		switch (u->sprite->main_image->iscript_state.animation) {
		case iscript_anims::AirAttkInit:
		case iscript_anims::AirAttkRpt:
			anim = iscript_anims::AirAttkToIdle;
			break;
		case iscript_anims::AlmostBuilt:
			if (u->sprite->sprite_type->id != idenums::SPRITEID_SCV) break;
			if (u->sprite->sprite_type->id != idenums::SPRITEID_Drone) break;
			if (u->sprite->sprite_type->id != idenums::SPRITEID_Probe) break;
			anim = iscript_anims::GndAttkToIdle;
			break;
		case iscript_anims::GndAttkInit:
		case iscript_anims::GndAttkRpt:
			anim = iscript_anims::GndAttkToIdle;
			break;
		case iscript_anims::SpecialState1:
			if (u->sprite->sprite_type->id == idenums::SPRITEID_Medic) anim = iscript_anims::WalkingToIdle;
			break;
		case iscript_anims::CastSpell:
			anim = iscript_anims::WalkingToIdle;
			break;
		}
		if (anim != -1) {
			for (image_t* img : u->sprite->images) {
				iscript_run_anim(img, anim);
			}
		}
		u->movement_flags &= ~8;
	}

	void execute_next_order(unit_t* u) {
		if (u->order_queue.empty()) return;
		if (u->ai) xcept("ai stuff");
		if ((u_in_building(u) || u_burrowed(u)) && u->order_queue.front().order_type->id != Orders::Die) return;
		const order_type_t* order_type = u->order_queue.front().order_type;
		order_target target = u->order_queue.front().target;
		remove_queued_order(u, u->order_queue.front());

		u->user_action_flags &= ~1;
		u->status_flags &= ~(unit_t::status_flag_disabled | unit_t::status_flag_order_not_interruptible | unit_t::status_flag_holding_position);
		if (!order_type->can_be_interrupted) u->status_flags |= unit_t::status_flag_order_not_interruptible;
		u->order_queue_timer = 0;
		u->recent_order_timer = 0;

		u->order_type = order_type;
		u->order_state = 0;

		if (target.unit) {
			u->order_target.unit = target.unit;
			u->order_target.pos = target.unit->sprite->position;
			u->order_unit_type = nullptr;
		} else {
			u->order_target.unit = nullptr;
			u->order_target.pos = target.position;
			u->order_unit_type = target.unit_type;
		}
		if (!u->ai) u->auto_target_unit = nullptr;
		iscript_run_to_idle(u);
		if (!ut_is_turret(u) && u->subunit && ut_is_turret(u->subunit)) {
			const order_type_t* turret_order_type = order_type;
			if (order_type == u->unit_type->return_to_idle) turret_order_type = u->subunit->unit_type->return_to_idle;
			else if (order_type == u->unit_type->attack_unit) turret_order_type = u->subunit->unit_type->attack_unit;
			else if (order_type == u->unit_type->attack_move) turret_order_type = u->subunit->unit_type->attack_move;
			else if (!order_type->valid_for_turret) turret_order_type = nullptr;
			if (turret_order_type) {
				set_unit_order(u->subunit, turret_order_type, target);
			}
		}

	}

	void set_unit_order(unit_t* u, const order_type_t* order_type, order_target target = order_target()) {
		u->user_action_flags |= 1;
		set_queued_order(u, true, order_type, target);
		execute_next_order(u);
	}

	template<typename F>
	unit_t* find_unit(rect area, const F& predicate) {
		bool is_max_width = false;
		if (area.to.x - area.from.x + 1 < game_st.max_unit_width) {
			area.to.x = area.from.x + 1 + game_st.max_unit_width;
			is_max_width = true;
		}
		bool is_max_height = false;
		if (area.to.y - area.from.y + 1 < game_st.max_unit_height) {
			area.to.y = area.from.y + 1 + game_st.max_unit_height;
			is_max_height = true;
		}

		size_t left_index = unit_finder_lower_bound_index(st.unit_finder_x, area.from.x);
		size_t top_index = unit_finder_lower_bound_index(st.unit_finder_y, area.from.y);
		size_t right_index = unit_finder_lower_bound_index(st.unit_finder_x, area.to.x);
		size_t bottom_index = unit_finder_lower_bound_index(st.unit_finder_y, area.to.y);

		if (right_index <= left_index) return nullptr;
		if (bottom_index <= top_index) return nullptr;

		for (size_t i = left_index; i < right_index; ++i) {
			unit_t* u = st.unit_finder_x[i].first;
			if (!is_max_width) u->unit_finder_mark = 1;
			else if (u->sprite->position.x - u->unit_type->dimensions.from.x <= area.to.x) u->unit_finder_mark = 1;
			else u->unit_finder_mark = 0;
		}
		for (size_t i = top_index; i < bottom_index; ++i) {
			unit_t* u = st.unit_finder_y[i].first;
			if (u->unit_finder_mark == 1) {
				if (!is_max_height) u->unit_finder_mark = 3;
				else if (u->sprite->position.y - u->unit_type->dimensions.from.y <= area.to.y) u->unit_finder_mark = 3;
			}
		}
		for (size_t i = left_index; i < right_index; ++i) {
			unit_t* u = st.unit_finder_x[i].first;
			if (u->unit_finder_mark == 3) {
				auto ret = predicate(u);
				if (ret) {
					for (; i < right_index; ++i) {
						st.unit_finder_x[i].first->unit_finder_mark = 0;
					}
					return u;
				}
			}
			u->unit_finder_mark = 0;
		}
		return nullptr;
	}

	rect unit_sprite_bounding_box(unit_t* u) {
		return { u->sprite->position - u->unit_type->dimensions.from, u->sprite->position + u->unit_type->dimensions.to };
	}

	bool units_intersecting(unit_t* a, unit_t* b) {
		if (st.unit_finder_x[a->unit_finder_right_index].second < st.unit_finder_x[b->unit_finder_left_index].second) return false;
		if (st.unit_finder_y[a->unit_finder_bottom_index].second < st.unit_finder_y[b->unit_finder_top_index].second) return false;
		if (st.unit_finder_x[a->unit_finder_left_index].second > st.unit_finder_x[b->unit_finder_right_index].second) return false;
		if (st.unit_finder_y[a->unit_finder_top_index].second > st.unit_finder_y[b->unit_finder_bottom_index].second) return false;
		return true;
	}

	void check_unit_collision(unit_t* u) {
		find_unit(unit_sprite_bounding_box(u), [&](unit_t* nu) {
			if (u_grounded_building(nu)) u->status_flags |= unit_t::status_flag_collision;
			else if (!u_flying(nu) && (!u_gathering(nu) || u_grounded_building(u))) {
				if (units_intersecting(u, nu)) {
					nu->status_flags |= unit_t::status_flag_collision;
				}
			}
			return false;
		});
	}

	void show_unit(unit_t* u) {
		if (~u->sprite->flags & sprite_t::flag_hidden) return;
		u->sprite->flags &= ~sprite_t::flag_hidden;
		if (u->subunit && !ut_is_turret(u)) u->subunit->sprite->flags &= ~sprite_t::flag_hidden;
		refresh_unit_vision(u);
		UpdateUnitSpriteInfo(u);
		update_unit_finder(u);
		if (u_grounded_building(u)) {
			xcept("update tiles (mask in flag_occupied)");
		}
		check_unit_collision(u);
		if (u_flying(u)) {
			xcept("set repulse angle");
		}
		log("toggle_unit_path(u)\n");
		
		u->movement_state = 0;
		if (u->sprite->elevation_level < 12) u->pathing_flags |= 1;
		else u->pathing_flags &= ~1;
		if (u->subunit && !ut_is_turret(u)) {
			log("toggle_unit_path(u->subunit)\n");
			u->subunit->movement_state = 0;
			if (u->subunit->sprite->elevation_level < 12) u->subunit->pathing_flags |= 1;
			else u->subunit->pathing_flags &= ~1;
		}
		st.hidden_units.remove(*u);
		bw_insert_list(st.visible_units, *u);
	}

	void update_unit_strength_and_apply_default_orders(unit_t* u) {

		if (ut_flyer(u)) {
			increment_unit_counts(u, -1);
			u->status_flags |= unit_t::status_flag_completed;
			increment_unit_counts(u, 1);
		} else {
			u->status_flags |= unit_t::status_flag_completed;
		}
		add_completed_unit(1, u, true);
		if (u->unit_type->id == UnitTypes::Spell_Scanner_Sweep || u->unit_type->id == UnitTypes::Special_Map_Revealer) {
			xcept("fixme scanner/map revealer");
		} else {
			if (u->sprite->flags & sprite_t::flag_hidden) {
				if (u->unit_type->id != UnitTypes::Protoss_Interceptor && u->unit_type->id != UnitTypes::Protoss_Scarab) {
					show_unit(u);
				}
			}
		}
		bool is_trap = false;
		if (u->unit_type->id == UnitTypes::Special_Floor_Missile_Trap) is_trap = true;
		if (u->unit_type->id == UnitTypes::Special_Floor_Gun_Trap) is_trap = true;
		if (u->unit_type->id == UnitTypes::Special_Wall_Missile_Trap) is_trap = true;
		if (u->unit_type->id == UnitTypes::Special_Wall_Flame_Trap) is_trap = true;
		if (u->unit_type->id == UnitTypes::Special_Right_Wall_Missile_Trap) is_trap = true;
		if (u->unit_type->id == UnitTypes::Special_Right_Wall_Flame_Trap) is_trap = true;
		if (is_trap) {
			u->status_flags |= unit_t::status_flag_cloaked | unit_t::status_flag_requires_detector;
			u->visibility_flags = 0x80000000;
			u->secondary_order_timer = 0;
		}
		if (st.players[u->owner].controller == state::player_t::controller_rescue_passive) {
			xcept("fixme rescue passive");
		} else {
			if (st.players[u->owner].controller == state::player_t::controller_neutral) set_unit_order(u, get_order_type(Orders::Neutral));
			else if (st.players[u->owner].controller == state::player_t::controller_computer_game) set_unit_order(u, u->unit_type->computer_ai_idle);
			else set_unit_order(u, u->unit_type->human_ai_idle);
		}
		if (ut_worker(u)) {
			xcept("fixme worker");
		}
		u->air_strength = get_unit_strength(u, false);
		u->ground_strength = get_unit_strength(u, true);

	}

	unit_t*create_initial_unit(const unit_type_t*unit_type, xy pos, int owner) {
		unit_t*u = create_unit(unit_type, pos, owner);
		if (!u) {
			display_last_net_error_for_player(owner);
			return nullptr;
		}
		if (unit_type_spreads_creep(unit_type, true) || unit_type->flags&unit_type_t::flag_creep) {
			xcept("apply creep");
		}
		finish_building_unit(u);
		if (!place_initial_unit(u)) {
			xcept("place_initial_unit failed");
		}

		update_unit_strength_and_apply_default_orders(u);

		return u;
	}

	void display_last_net_error_for_player(int player) {
		log("fixme: display last error (%d)\n", st.last_net_error);
	}

};

void advance(state&st) {

	state_functions funcs(st);

	funcs.game_loop();

}

struct game_load_functions : state_functions {

	explicit game_load_functions(state&st) : state_functions(st) {}

	game_state&game_st = *st.game;

	unit_type_t*get_unit_type(int id) {
		if ((size_t)id >= 228) xcept("invalid unit id %d", id);
		return &game_st.unit_types.vec[id];
	}
	weapon_type_t*get_weapon_type(int id) {
		if ((size_t)id >= 130) xcept("invalid weapon id %d", id);
		return &game_st.weapon_types.vec[id];
	}
	upgrade_type_t*get_upgrade_type(int id) {
		if ((size_t)id >= 61) xcept("invalid upgrade id %d", id);
		return &game_st.upgrade_types.vec[id];
	}
	tech_type_t*get_tech_type(int id) {
		if ((size_t)id >= 44) xcept("invalid tech id %d", id);
		return &game_st.tech_types.vec[id];
	}
	const flingy_type_t*get_flingy_type(int id) {
		if ((size_t)id >= 209) xcept("invalid flingy id %d", id);
		return &global_st.flingy_types.vec[id];
	}

	void reset() {

		game_st.unit_types = data_loading::load_units_dat("arr\\units.dat");
		game_st.weapon_types = data_loading::load_weapons_dat("arr\\weapons.dat");
		game_st.upgrade_types = data_loading::load_upgrades_dat("arr\\upgrades.dat");
		game_st.tech_types = data_loading::load_techdata_dat("arr\\techdata.dat");

		auto fixup_unit_type = [&](unit_type_t*&ptr) {
			size_t index = (size_t)ptr;
			if (index == 228) ptr = nullptr;
			else ptr = get_unit_type(index);
		};
		auto fixup_weapon_type = [&](weapon_type_t*&ptr) {
			size_t index = (size_t)ptr;
			if (index == 130) ptr = nullptr;
			else ptr = get_weapon_type(index);
		};
		auto fixup_upgrade_type = [&](upgrade_type_t*&ptr) {
			size_t index = (size_t)ptr;
			if (index == 61) ptr = nullptr;
			else ptr = get_upgrade_type(index);
		};
		auto fixup_flingy_type = [&](const flingy_type_t*&ptr) {
			size_t index = (size_t)ptr;
			if (index == 61) ptr = nullptr;
			else ptr = get_flingy_type(index);
		};
		auto fixup_order_type = [&](const order_type_t*&ptr) {
			size_t index = (size_t)ptr;
			ptr = get_order_type(index);
		};

		for (auto&v : game_st.unit_types.vec) {
			fixup_flingy_type(v.flingy);
			fixup_unit_type(v.turret_unit_type);
			fixup_unit_type(v.subunit2);
			fixup_weapon_type(v.ground_weapon);
			fixup_weapon_type(v.air_weapon);
			fixup_upgrade_type(v.armor_upgrade);
			fixup_order_type(v.computer_ai_idle);
			fixup_order_type(v.human_ai_idle);
			fixup_order_type(v.return_to_idle);
			fixup_order_type(v.attack_unit);
			fixup_order_type(v.attack_move);
		}
		for (auto&v : game_st.weapon_types.vec) {
			fixup_flingy_type(v.flingy);
			fixup_upgrade_type(v.damage_upgrade);
		}

		for (auto&v : game_st.unit_type_allowed) v.fill(true);
		for (auto&v : game_st.tech_available) v.fill(true);
		st.tech_researched.fill({});
		for (auto&v : game_st.max_upgrade_levels) {
			for (size_t i = 0; i < game_st.max_upgrade_levels.size(); ++i) {
				v[i] = get_upgrade_type(i)->max_repeats;
			}
		}
		st.upgrade_levels.fill({});
		// upgrade progress?
		// UPRP stuff?

		for (auto&v : st.unit_counts) v.fill(0);
		for (auto&v : st.completed_unit_counts) v.fill(0);

		st.factory_counts.fill(0);
		st.building_counts.fill(0);
		st.non_building_counts.fill(0);

		st.completed_factory_counts.fill(0);
		st.completed_building_counts.fill(0);
		st.completed_non_building_counts.fill(0);

		st.total_buildings_ever_completed.fill(0);
		st.total_non_buildings_ever_completed.fill(0);

		st.unit_score.fill(0);
		st.building_score.fill(0);

		for (auto&v : st.supply_used) v.fill(0);
		for (auto&v : st.supply_available) v.fill(0);

		auto set_acquisition_ranges = [&]() {
			for (int i = 0; i < 228; ++i) {
				unit_type_t*unit_type = get_unit_type(i);
				const unit_type_t*shooting_type = unit_type->turret_unit_type ? unit_type->turret_unit_type : unit_type;
				const weapon_type_t*ground_weapon = shooting_type->ground_weapon;
				const weapon_type_t*air_weapon = shooting_type->air_weapon;
				int acq_range = shooting_type->target_acquisition_range;
				if (ground_weapon) acq_range = std::max(acq_range, ground_weapon->max_range);
				if (air_weapon) acq_range = std::max(acq_range, air_weapon->max_range);
				unit_type->target_acquisition_range = acq_range;
			}
		};
		set_acquisition_ranges();

		calculate_unit_strengths();

		generate_sight_values();

		load_tile_stuff();

		st.tiles.clear();
		st.tiles.resize(game_st.map_tile_width*game_st.map_tile_height);
		st.tiles_mega_tile_index.clear();
		st.tiles_mega_tile_index.resize(st.tiles.size());

		st.gfx_creep_tiles.clear();
		st.gfx_creep_tiles.resize(game_st.map_tile_width * game_st.map_tile_height);

		st.order_timer_counter = 10;
		st.secondary_order_timer_counter = 150;

		st.visible_units.clear();
		st.hidden_units.clear();
		st.scanner_sweep_units.clear();
		st.sight_related_units.clear();
		for (auto&v : st.player_units) v.clear();

		auto clear_and_make_free = [&](auto& list, auto& free_list) {
			free_list.clear();
			memset(list.data(), 0, &*list.end() - list.data());
			for (auto& v: list) {
				bw_insert_list(free_list, v);
			}
		};

		clear_and_make_free(st.units, st.free_units);
		clear_and_make_free(st.sprites, st.free_sprites);
		st.sprites_on_tile_line.clear();
		st.sprites_on_tile_line.resize(game_st.map_tile_height);
		clear_and_make_free(st.images, st.free_images);
		clear_and_make_free(st.orders, st.free_orders);
		st.allocated_order_count = 0;

		st.last_net_error = 0;

		game_st.is_replay = false;
		game_st.local_player = 0;

		st.unit_finder_size = 0;
		st.unit_finder_x.clear();
		st.unit_finder_x.resize(st.units.size() * 2);
		st.unit_finder_y.clear();
		st.unit_finder_y.resize(st.units.size() * 2);

		int max_unit_width = 0;
		int max_unit_height = 0;
		for (auto&v : game_st.unit_types.vec) {
			int width = v.dimensions.from.x + 1 + v.dimensions.to.x;
			int height = v.dimensions.from.y + 1 + v.dimensions.to.y;
			if (width > max_unit_width) max_unit_width = width;
			if (height > max_unit_height) max_unit_height = height;
		}
		game_st.max_unit_width = max_unit_width;
		game_st.max_unit_height = max_unit_height;

		st.random_counts.fill(0);
		st.total_random_counts = 0;
		st.lcg_rand_state = 42;

	}

	int get_unit_strength(const unit_type_t*unit_type, const weapon_type_t*weapon_type) {
		switch (unit_type->id) {
		case UnitTypes::Terran_Vulture_Spider_Mine:
		case UnitTypes::Protoss_Interceptor:
		case UnitTypes::Protoss_Scarab:
			return 0;
		}
		int hp = unit_type->hitpoints >> 8;
		if (unit_type->has_shield) hp += unit_type->shield_points;
		if (hp == 0) return 0;
		int fact = weapon_type->damage_factor;
		int cd = weapon_type->cooldown;
		int dmg = weapon_type->damage_amount;
		int range = weapon_type->max_range;
		unsigned int a = (range / (unsigned)cd) * fact * dmg;
		unsigned int b = (hp * ((int64_t)(fact*dmg << 11) / cd)) >> 8;
		int score = (int)(sqrt(a + b)*7.58);
		switch (unit_type->id) {
		case UnitTypes::Terran_SCV:
		case UnitTypes::Zerg_Drone:
		case UnitTypes::Protoss_Probe:
			return score / 4;
		case UnitTypes::Terran_Firebat:
		case UnitTypes::Zerg_Mutalisk:
		case UnitTypes::Protoss_Zealot:
			return score * 2;
		case UnitTypes::Zerg_Scourge:
		case UnitTypes::Zerg_Infested_Terran:
			return score / 16;
		case UnitTypes::Protoss_Reaver:
			return score / 10;
		default:
			return score;
		}
	};

	void calculate_unit_strengths() {

		for (int idx = 0; idx < 228; ++idx) {

			const unit_type_t*unit_type = get_unit_type(idx);
			const unit_type_t*shooting_type = unit_type;
			int air_strength = 0;
			int ground_strength = 0;
			if (shooting_type->id != UnitTypes::Zerg_Larva && shooting_type->id != UnitTypes::Zerg_Egg && shooting_type->id != UnitTypes::Zerg_Cocoon && shooting_type->id != UnitTypes::Zerg_Lurker_Egg) {
				if (shooting_type->id == UnitTypes::Protoss_Carrier || shooting_type->id == UnitTypes::Hero_Gantrithor) shooting_type = get_unit_type(UnitTypes::Protoss_Interceptor);
				else if (shooting_type->id == UnitTypes::Protoss_Reaver || shooting_type->id == UnitTypes::Hero_Warbringer) shooting_type = get_unit_type(UnitTypes::Protoss_Scarab);
				else if (shooting_type->turret_unit_type) shooting_type = shooting_type->turret_unit_type;

				const weapon_type_t*air_weapon = shooting_type->air_weapon;
				if (!air_weapon) air_strength = 1;
				else air_strength = get_unit_strength(unit_type, air_weapon);

				const weapon_type_t*ground_weapon = shooting_type->ground_weapon;
				if (!ground_weapon) ground_strength = 1;
				else ground_strength = get_unit_strength(unit_type, ground_weapon);
			}
			if (air_strength == 1 && ground_strength > air_strength) air_strength = 0;
			if (ground_strength == 1 && air_strength > ground_strength) ground_strength = 0;

			//log("strengths for %d is %d %d\n", idx, air_strength, ground_strength);

			game_st.unit_air_strength[idx] = air_strength;
			game_st.unit_ground_strength[idx] = ground_strength;

		}

	}

	void generate_sight_values() {
		for (size_t i = 0; i < game_st.sight_values.size(); ++i) {
			auto&v = game_st.sight_values[i];
			v.max_width = 3 + (int)i * 2;
			v.max_height = 3 + (int)i * 2;
			v.min_width = 3;
			v.min_height = 3;
			v.min_mask_size = 0;
			v.ext_masked_count = 0;
		}

		for (auto&v : game_st.sight_values) {
			struct base_mask_t {
				sight_values_t::maskdat_node_t*maskdat_node;
				bool masked;
			};
			a_vector<base_mask_t> base_mask(v.max_width*v.max_height);
			auto mask = [&](size_t index) {
				if (index >= base_mask.size()) xcept("attempt to mask invalid base mask index %d (size %d) (broken brood war algorithm)", index, base_mask.size());
				base_mask[index].masked = true;
			};
			v.min_mask_size = v.min_width*v.min_height;
			int offx = v.max_width / 2 - v.min_width / 2;
			int offy = v.max_height / 2 - v.min_height / 2;
			for (int y = 0; y < v.min_height; ++y) {
				for (int x = 0; x < v.min_width; ++x) {
					mask((offy + y)*v.max_width + offx + x);
				}
			}
			auto generate_base_mask = [&]() {
				int offset = v.max_height / 2 - v.max_width / 2;
				int half_width = v.max_width / 2;
				int	max_x2 = half_width;
				int max_x1 = half_width * 2;
				int cur_x1 = 0;
				int cur_x2 = half_width;
				int i = 0;
				int max_i = half_width;
				int cursize1 = 0;
				int cursize2 = half_width*half_width;
				int min_cursize2 = half_width * (half_width - 1);
				int min_cursize2_chg = half_width * 2;
				while (true) {
					if (cur_x1 <= max_x1) {
						for (int i = 0; i <= max_x1 - cur_x1; ++i) {
							mask((offset + cur_x2)*v.max_width + cur_x1 + i);
							mask((offset + max_x2)*v.max_width + cur_x1 + i);
						}
					}
					if (cur_x2 <= max_x2) {
						for (int i = 0; i <= max_x2 - cur_x2; ++i) {
							mask((offset + cur_x1)*v.max_width + cur_x2 + i);
							mask((offset + max_x1)*v.max_width + cur_x2 + i);
						}
					}
					cursize2 += 1 - cursize1 - 2;
					cursize1 += 2;
					--cur_x2;
					++max_x2;
					if (cursize2 <= min_cursize2) {
						--max_i;
						++cur_x1;
						--max_x1;
						min_cursize2 -= min_cursize2_chg - 2;
						min_cursize2_chg -= 2;
					}

					++i;
					if (i > max_i) break;
				}
			};
			generate_base_mask();
			int masked_count = 0;
			for (auto&v : base_mask) {
				if (v.masked) ++masked_count;
			}
			log("%d %d - masked_count is %d\n", v.max_width, v.max_height, masked_count);

			v.ext_masked_count = masked_count - v.min_mask_size;
			v.maskdat.clear();
			v.maskdat.resize(masked_count);

			auto*center = &base_mask[v.max_height / 2 * v.max_width + v.max_width / 2];
			center->maskdat_node = &v.maskdat.front();

			auto at = [&](int index) -> base_mask_t& {
				auto*r = &center[index];
				if (r < base_mask.data() || r >= base_mask.data() + base_mask.size()) xcept("attempt to access invalid base mask center-relative index %d (size %d)", index, base_mask.size());
				return *r;
			};

			size_t next_entry_index = 1;

			int cur_x = -1;
			int cur_y = -1;
			int added_count = 1;
			for (int i = 2; added_count < masked_count; i += 2) {
				for (int dir = 0; dir < 4; ++dir) {
					static const std::array<int, 4> direction_x = { 1,0,-1,0 };
					static const std::array<int, 4> direction_y = { 0,1,0,-1 };
					int this_x;
					int this_y;
					auto do_n = [&](int n) {
						for (int i = 0; i < n; ++i) {
							if (at(this_y*v.max_width + this_x).masked) {
								if (this_x || this_y) {
									auto*this_entry = &v.maskdat.at(next_entry_index++);

									int prev_x = this_x;
									int prev_y = this_y;
									if (prev_x > 0) --prev_x;
									else if (prev_x < 0) ++prev_x;
									if (prev_y > 0) --prev_y;
									else if (prev_y < 0) ++prev_y;
									if (std::abs(prev_x) == std::abs(prev_y) || (prev_x == 0 && direction_x[dir]) || (prev_y == 0 && direction_y[dir])) {
										this_entry->prev = this_entry->prev2 = at(prev_y * v.max_width + prev_x).maskdat_node;
										this_entry->prev_count = 1;
									} else {
										this_entry->prev = at(prev_y * v.max_width + prev_x).maskdat_node;
										int prev2_x = prev_x;
										int prev2_y = prev_y;
										if (std::abs(prev2_x) <= std::abs(prev2_y)) {
											if (this_x >= 0) ++prev2_x;
											else --prev2_x;
										} else {
											if (this_y >= 0) ++prev2_y;
											else --prev2_y;
										}
										this_entry->prev2 = at(prev2_y * v.max_width + prev2_x).maskdat_node;
										this_entry->prev_count = 2;
									}
									this_entry->map_index_offset = this_y * game_st.map_tile_width + this_x;
									this_entry->x = this_x;
									this_entry->y = this_y;
									at(this_y * v.max_width + this_x).maskdat_node = this_entry;
									++added_count;
								}
							}
							this_x += direction_x[dir];
							this_y += direction_y[dir];
						}
					};
					const std::array<int, 4> max_i = { v.max_height,v.max_width,v.max_height,v.max_width };
					if (i > max_i[dir]) {
						this_x = cur_x + i * direction_x[dir];
						this_y = cur_y + i * direction_y[dir];
						do_n(1);
					} else {
						this_x = cur_x + direction_x[dir];
						this_y = cur_y + direction_y[dir];
						do_n(std::min(max_i[(dir + 1) % 4] - 1, i));
					}
					cur_x = this_x - direction_x[dir];
					cur_y = this_y - direction_y[dir];
				}
				if (i < v.max_width - 1) --cur_x;
				if (i < v.max_height - 1) --cur_y;
			}

		}

		// 	log("generated sight stuff:\n");
		// 	for (auto&v : game_st.sight_values) {
		// 		log("%d %d %d %d %d %d\n", v.max_width, v.max_height, v.min_width, v.max_height, v.min_mask_size, v.ext_masked_count);
		// 		size_t maskdat_size = v.min_mask_size + v.ext_masked_count;
		// 		log(" maskdat: \n");
		// 		//log("%s", hexdump(v.maskdat.data(), maskdat_size * 20));
		// 		for (auto&v : v.maskdat) {
		// 			log("%d %d %d\n", v.map_index_offset, v.x, v.y);
		// 		}
		// 	}

	}

	void load_tile_stuff() {
		std::array<const char*, 8> tileset_names = {
			"badlands", "platform", "install", "AshWorld", "Jungle", "Desert", "Ice", "Twilight"
		};

		auto set_mega_tile_flags = [&]() {
			game_st.mega_tile_flags.resize(game_st.vf4.size());
			for (size_t i = 0; i < game_st.mega_tile_flags.size(); ++i) {
				int flags = 0;
				auto&mt = game_st.vf4[i];
				int walkable_count = 0;
				int middle_count = 0;
				int high_count = 0;
				int very_high_count = 0;
				for (size_t y = 0; y < 4; ++y) {
					for (size_t x = 0; x < 4; ++x) {
						if (mt.flags[y * 4 + x] & MiniTileFlags::Walkable) ++walkable_count;
						if (mt.flags[y * 4 + x] & MiniTileFlags::Middle) ++middle_count;
						if (mt.flags[y * 4 + x] & MiniTileFlags::High) ++high_count;
						if (mt.flags[y * 4 + x] & MiniTileFlags::BlocksView) ++very_high_count;
					}
				}
				if (walkable_count > 12) flags |= tile_t::flag_walkable;
				else flags |= tile_t::flag_unwalkable;
				if (walkable_count && walkable_count != 0x10) flags |= tile_t::partially_walkable;
				if (high_count < 12 && middle_count + high_count >= 12) flags |= tile_t::flag_middle;
				if (high_count >= 12) flags |= tile_t::flag_high;
				if (very_high_count) flags |= tile_t::flag_very_high;
				game_st.mega_tile_flags[i] = flags;
			}

		};

		load_data_file(game_st.vf4, format("Tileset\\%s.vf4", tileset_names.at(game_st.tileset_index)));
		set_mega_tile_flags();
		load_data_file(game_st.cv5, format("Tileset\\%s.cv5", tileset_names.at(game_st.tileset_index)));
	}

	struct tag_t {
		tag_t() = default;
		tag_t(const char str[4]) : data({ str[0], str[1], str[2], str[3] }) {}
		tag_t(const std::array<char, 4> data) : data(data) {}
		std::array<char, 4> data = {};
		bool operator==(const tag_t&n) const {
			return data == n.data;
		}
		size_t operator()(const tag_t&v) const {
			return std::hash<uint32_t>()((uint32_t)v.data[0] | v.data[1] << 8 | v.data[2] << 16 | v.data[3] << 24);
		}
	};

	void load_map_file(a_string filename) {

		// campaign stuff? see load_map_file

		log("load map file '%s'\n", filename);

		SArchive archive(filename);
		a_vector<uint8_t> data;
		load_data_file(data, "staredit\\scenario.chk");

		using data_loading::data_reader_le;

		a_unordered_map<tag_t, std::function<void(data_reader_le)>, tag_t> tag_funcs;

		auto tagstr = [&](tag_t tag) {
			return a_string(tag.data.data(), 4);
		};

		using tag_list_t = a_vector<std::pair<tag_t, bool>>;
		auto read_chunks = [&](const tag_list_t&tags) {
			data_reader_le r(data.data(), data.data() + data.size());
			a_unordered_map<tag_t, data_reader_le> chunks;
			while (r.left()) {
				tag_t tag = r.get<std::array<char, 4>>();
				uint32_t len = r.get<uint32_t>();
				//log("tag '%.4s' len %d\n", (char*)&tag, len);
				uint8_t*chunk_data = r.ptr;
				r.skip(len);
				chunks[tag] = { chunk_data, r.ptr };
			}
			for (auto&v : tags) {
				tag_t tag = std::get<0>(v);
				auto i = chunks.find(tag);
				if (i == chunks.end()) {
					if (std::get<1>(v)) xcept("map is missing required chunk '%s'", tagstr(tag));
				} else {
					if (!tag_funcs[tag]) xcept("tag '%s' is missing a function", tagstr(tag));
					log("loading tag '%s'...\n", tagstr(tag));
					tag_funcs[tag](i->second);
				}
			}
		};

		int version = 0;
		tag_funcs["VER "] = [&](data_reader_le r) {
			version = r.get<uint16_t>();
			log("VER: version is %d\n", version);
		};
		tag_funcs["DIM "] = [&](data_reader_le r) {
			game_st.map_tile_width = r.get<uint16_t>();
			game_st.map_tile_height = r.get<uint16_t>();
			game_st.map_width = game_st.map_tile_width * 32;
			game_st.map_height = game_st.map_tile_height * 32;
			log("DIM: dimensions are %d %d\n", game_st.map_tile_width, game_st.map_tile_height);
		};
		tag_funcs["ERA "] = [&](data_reader_le r) {
			game_st.tileset_index = r.get<uint16_t>() % 8;
			log("ERA: tileset is %d\n", game_st.tileset_index);
		};
		tag_funcs["OWNR"] = [&](data_reader_le r) {
			for (size_t i = 0; i < 12; ++i) {
				st.players[i].controller = r.get<int8_t>();
				if (st.players[i].controller == state::player_t::controller_open) st.players[i].controller = state::player_t::controller_occupied;
				if (st.players[i].controller == state::player_t::controller_computer) st.players[i].controller = state::player_t::controller_computer_game;
			}
		};
		tag_funcs["SIDE"] = [&](data_reader_le r) {
			for (size_t i = 0; i < 12; ++i) {
				st.players[i].race = r.get<int8_t>();
			}
		};
		tag_funcs["STR "] = [&](data_reader_le r) {
			auto start = r;
			size_t num = r.get<uint16_t>();
			game_st.map_strings.clear();
			game_st.map_strings.resize(num);
			for (size_t i = 0; i < num; ++i) {
				size_t offset = r.get<uint16_t>();
				auto t = start;
				t.skip(offset);
				char*b = (char*)t.ptr;
				while (t.get<char>());
				game_st.map_strings[i] = a_string(b, (char*)t.ptr - b - 1);
				//log("string %d: %s\n", i, game_st.map_strings[i]);
			}
		};
		tag_funcs["SPRP"] = [&](data_reader_le r) {
			game_st.scenario_name = game_st.get_string(r.get<uint16_t>());
			game_st.scenario_description = game_st.get_string(r.get<uint16_t>());
			log("SPRP: scenario name: '%s',  description: '%s'\n", game_st.scenario_name, game_st.scenario_description);
		};
		tag_funcs["FORC"] = [&](data_reader_le r) {
			for (size_t i = 0; i < 12; ++i) st.players[i].force = 0;
			for (size_t i = 0; i < 4; ++i) {
				game_st.forces[i].name = "";
				game_st.forces[i].flags = 0;
			}
			if (r.left()) {
				for (size_t i = 0; i < 8; ++i) {
					st.players[i].force = r.get<uint8_t>();
				}
				for (size_t i = 0; i < 4; ++i) {
					game_st.forces[i].name = game_st.get_string(r.get<uint16_t>());
				}
				for (size_t i = 0; i < 4; ++i) {
					game_st.forces[i].flags = r.get<uint8_t>();
				}
			}
		};
		tag_funcs["VCOD"] = [&](data_reader_le r) {
			// Starcraft does some verification/checksum stuff here
		};


		tag_funcs["MTXM"] = [&](data_reader_le r) {
			auto gfx_tiles_data = r.get_vec<uint16_t>(game_st.map_tile_width * game_st.map_tile_height);
			game_st.gfx_tiles.resize(gfx_tiles_data.size());
			for (size_t i = 0; i < gfx_tiles_data.size(); ++i) {
				game_st.gfx_tiles[i] = tile_id(gfx_tiles_data[i]);
			}
			for (size_t i = 0; i < game_st.gfx_tiles.size(); ++i) {
				tile_id tile_id = game_st.gfx_tiles[i];
				size_t megatile_index = game_st.cv5.at(tile_id.group_index()).megaTileRef[tile_id.subtile_index()];
				st.tiles_mega_tile_index[i] = (uint16_t)megatile_index;
				st.tiles[i].flags |= game_st.mega_tile_flags.at(megatile_index);
				if (tile_id.has_creep()) {
					st.tiles_mega_tile_index[i] |= 0x8000;
					st.tiles[i].flags |= tile_t::flag_has_creep;
				}
			}
		};

		bool bVictoryCondition = false;
		bool bStartingUnits = false;
		bool bTournamentModeEnabled = false;

		tag_funcs["THG2"] = [&](data_reader_le r) {
			while (r.left()) {
				int unit_type = r.get<uint16_t>();
				int x = r.get<uint16_t>();
				int y = r.get<uint16_t>();
				int owner = r.get<uint8_t>();
				r.get<uint8_t>();
				r.get<uint8_t>();
				int flags = r.get<uint8_t>();
				if (flags & 0x10) {
					xcept("create thingy of type %d", unit_type);
				} else {
					if (unit_type == UnitTypes::Special_Upper_Level_Door) owner = 11;
					if (unit_type == UnitTypes::Special_Right_Upper_Level_Door) owner = 11;
					if (unit_type == UnitTypes::Special_Pit_Door) owner = 11;
					if (unit_type == UnitTypes::Special_Right_Pit_Door) owner = 11;
					if ((!bVictoryCondition && !bStartingUnits && !bTournamentModeEnabled) || owner == 11) {
						xcept("create (thingy) unit of type %d", unit_type);
						if (flags & 0x80) xcept("disable thingy unit");
					}
				}
			}
		};
		tag_funcs["MASK"] = [&](data_reader_le r) {
			auto mask = r.get_vec<uint8_t>(game_st.map_tile_width*game_st.map_tile_height);
			for (size_t i = 0; i < mask.size(); ++i) {
				st.tiles[i].visible |= mask[i];
				st.tiles[i].explored |= mask[i];
			}
		};

		auto units = [&](data_reader_le r, bool broodwar) {
			auto uses_default_settings = r.get_vec<uint8_t>(228);
			auto hp = r.get_vec<uint32_t>(228);
			auto shield_points = r.get_vec<uint16_t>(228);
			auto armor = r.get_vec<uint8_t>(228);
			auto build_time = r.get_vec<uint16_t>(228);
			auto mineral_cost = r.get_vec<uint16_t>(228);
			auto gas_cost = r.get_vec<uint16_t>(228);
			auto string_index = r.get_vec<uint16_t>(228);
			auto weapon_damage = r.get_vec<uint16_t>(broodwar ? 130 : 100);
			auto weapon_bonus_damage = r.get_vec<uint16_t>(broodwar ? 130 : 100);
			for (int i = 0; i < 228; ++i) {
				if (uses_default_settings[i]) continue;
				unit_type_t*unit_type = get_unit_type(i);
				unit_type->hitpoints = hp[i];
				unit_type->shield_points = shield_points[i];
				unit_type->armor = armor[i];
				unit_type->build_time = build_time[i];
				unit_type->mineral_cost = mineral_cost[i];
				unit_type->gas_cost = gas_cost[i];
				unit_type->unit_map_string_index = string_index[i];
				const unit_type_t*shooting_type = unit_type->turret_unit_type ? unit_type->turret_unit_type : unit_type;
				weapon_type_t*ground_weapon = shooting_type->ground_weapon;
				weapon_type_t*air_weapon = shooting_type->air_weapon;
				if (ground_weapon) {
					ground_weapon->damage_amount = weapon_damage[ground_weapon->id];
					ground_weapon->damage_bonus =  weapon_bonus_damage[ground_weapon->id];
				}
				if (air_weapon) {
					air_weapon->damage_amount = weapon_damage[air_weapon->id];
					air_weapon->damage_bonus = weapon_bonus_damage[air_weapon->id];
				}
			}
		};

		auto upgrades = [&](data_reader_le r, bool broodwar) {
			auto uses_default_settings = r.get_vec<uint8_t>(broodwar ? 62 : 46);
			auto mineral_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto mineral_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto gas_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto gas_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto research_time = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto research_time_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			for (int i = 0; i < (broodwar ? 61 : 46); ++i) {
				if (uses_default_settings[i]) continue;
				upgrade_type_t*upg = get_upgrade_type(i);
				upg->mineral_cost_base = mineral_cost[i];
				upg->mineral_cost_factor = mineral_cost_factor[i];
				upg->gas_cost_base = gas_cost[i];
				upg->gas_cost_factor = gas_cost_factor[i];
				upg->research_time_base = research_time[i];
				upg->research_time_factor = research_time_factor[i];
			}
		};

		auto techdata = [&](data_reader_le r, bool broodwar) {
			auto uses_default_settings = r.get_vec<uint8_t>(broodwar ? 44 : 24);
			auto mineral_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			auto gas_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			auto build_time = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			auto energy_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			for (int i = 0; i < (broodwar ? 44 : 24); ++i) {
				if (uses_default_settings[i]) continue;
				tech_type_t*tech = get_tech_type(i);
				tech->mineral_cost = mineral_cost[i];
				tech->gas_cost = gas_cost[i];
				tech->research_time = build_time[i];
				tech->energy_cost = energy_cost[i];
			}
		};

		auto upgrade_restrictions = [&](data_reader_le r, bool broodwar) {
			int count = broodwar ? 61 : 46;
			auto player_max_level = r.get_vec<uint8_t>(12 * count);
			auto player_cur_level = r.get_vec<uint8_t>(12 * count);
			auto global_max_level = r.get_vec<uint8_t>(count);
			auto global_cur_level = r.get_vec<uint8_t>(count);
			auto player_uses_global_default = r.get_vec<uint8_t>(12 * count);
			for (int player = 0; player < 12; ++player) {
				for (int upgrade = 0; upgrade < count; ++upgrade) {
					game_st.max_upgrade_levels[player][upgrade] = !!player_uses_global_default[player*count + upgrade] ? global_max_level[upgrade] : player_max_level[player*count + upgrade];
					st.upgrade_levels[player][upgrade] = !!player_uses_global_default[player*count + upgrade] ? global_cur_level[upgrade] : player_cur_level[player*count + upgrade];
				}
			}
		};
		auto tech_restrictions = [&](data_reader_le r, bool broodwar) {
			int count = broodwar ? 44 : 24;
			auto player_available = r.get_vec<uint8_t>(12 * count);
			auto player_researched = r.get_vec<uint8_t>(12 * count);
			auto global_available = r.get_vec<uint8_t>(count);
			auto global_researched = r.get_vec<uint8_t>(count);
			auto player_uses_global_default = r.get_vec<uint8_t>(12 * count);
			for (int player = 0; player < 12; ++player) {
				for (int upgrade = 0; upgrade < count; ++upgrade) {
					game_st.tech_available[player][upgrade] = !!(!!player_uses_global_default[player*count + upgrade] ? global_available[upgrade] : player_available[player*count + upgrade]);
					st.tech_researched[player][upgrade] = !!(!!player_uses_global_default[player*count + upgrade] ? global_researched[upgrade] : player_researched[player*count + upgrade]);
				}
			}
		};

		tag_funcs["UNIS"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			units(r, false);
		};
		tag_funcs["UPGS"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			upgrades(r, false);
		};
		tag_funcs["TECS"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			techdata(r, false);
		};
		tag_funcs["PUNI"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			auto player_available = r.get_vec<std::array<uint8_t, 228>>(12);
			auto global_available = r.get_vec<uint8_t>(228);
			auto player_uses_global_default = r.get_vec<std::array<uint8_t, 228>>(12);
			for (int player = 0; player < 12; ++player) {
				for (int unit = 0; unit < 228; ++unit) {
					game_st.unit_type_allowed[player][unit] = !!(!!player_uses_global_default[player][unit] ? global_available[unit] : player_available[player][unit]);
				}
			}
		};
		tag_funcs["UPGR"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			upgrade_restrictions(r, false);
		};
		tag_funcs["PTEC"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			tech_restrictions(r, false);
		};

		tag_funcs["UNIx"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			units(r, true);
		};
		tag_funcs["UPGx"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			upgrades(r, true);
		};
		tag_funcs["TECx"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			techdata(r, true);
		};
		tag_funcs["PUPx"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			upgrade_restrictions(r, true);
		};
		tag_funcs["PTEx"] = [&](data_reader_le r) {
			if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
			tech_restrictions(r, true);
		};

		tag_funcs["UNIT"] = [&](data_reader_le r) {
			while (r.left()) {

				int id = r.get<uint32_t>();
				int x = r.get<uint16_t>();
				int y = r.get<uint16_t>();
				int unit_type_id = r.get<uint16_t>();
				int link = r.get<uint16_t>();
				int valid_flags = r.get<uint16_t>();
				int valid_properties = r.get<uint16_t>();
				int owner = r.get<uint8_t>();
				int hp_percent = r.get<uint8_t>();
				int shield_percent = r.get<uint8_t>();
				int energy_percent = r.get<uint8_t>();
				int resources = r.get<uint32_t>();
				int units_in_hangar = r.get<uint16_t>();
				int flags = r.get<uint16_t>();
				r.get<uint32_t>();
				int related_unit_id = r.get<uint32_t>();

				if ((size_t)unit_type_id >= 228) xcept("UNIT: invalid unit type %d", unit_type_id);
				if ((size_t)owner >= 12) xcept("UNIT: invalid owner %d", owner);

				const unit_type_t*unit_type = get_unit_type(unit_type_id);

				log("create unit of type %d\n", unit_type->id);

				if (unit_type->id == UnitTypes::Special_Start_Location) {
					game_st.start_locations[owner] = { x, y };
					// 				int local_player = 0;
					// 				if (owner == local_player) {
					// 					int move_screen_to_tile_x = x / 32 >= 10 ? x / 32 - 10 : 0;
					// 					int move_screen_to_tile_y = y / 32 >= 6 ? y / 32 - 6 : 0;
					// 				}
					continue;
				}
				auto should_create_units_for_this_player = [&]() {
					if (owner >= 8) return true;
					int controller = st.players[owner].controller;
					if (controller == state::player_t::controller_computer_game) return true;
					if (controller == state::player_t::controller_occupied) return true;
					if (controller == state::player_t::controller_rescue_passive) return true;
					if (controller == state::player_t::controller_unused_rescue_active) return true;
					return false;
				};
				auto is_neutral_unit = [&]() {
					if (owner == 11) return true;
					if (unit_type->id == UnitTypes::Resource_Mineral_Field) return true;
					if (unit_type->id == UnitTypes::Resource_Mineral_Field_Type_2) return true;
					if (unit_type->id == UnitTypes::Resource_Mineral_Field_Type_3) return true;
					if (unit_type->id == UnitTypes::Resource_Vespene_Geyser) return true;
					if (unit_type->id == UnitTypes::Critter_Rhynadon) return true;
					if (unit_type->id == UnitTypes::Critter_Bengalaas) return true;
					if (unit_type->id == UnitTypes::Critter_Scantid) return true;
					if (unit_type->id == UnitTypes::Critter_Kakaru) return true;
					if (unit_type->id == UnitTypes::Critter_Ragnasaur) return true;
					if (unit_type->id == UnitTypes::Critter_Ursadon) return true;
					return false;
				};
				if (!should_create_units_for_this_player()) continue;
				if (bStartingUnits && !is_neutral_unit()) continue;
				if (!bVictoryCondition && !bStartingUnits && !bTournamentModeEnabled) {
					// what is player_force?
					std::array<int, 12> player_force{};
					if (player_force[owner] && ~unit_type->staredit_group_flags & GroupFlags::Neutral) continue;
				}

				unit_t*u = create_initial_unit(unit_type, { x,y }, owner);

				//xcept("created unit %p\n", u);
				log("created unit %p\n", u);

			}
		};

		tag_funcs["UPRP"] = [&](data_reader_le r) {
			for (int i = 0; i < 64; ++i) {
				int valid_flags = r.get<uint16_t>();
				int valid_properties = r.get<uint16_t>();
				int owner = r.get<uint8_t>();
				int hp_percent = r.get<uint8_t>();
				int shield_percent = r.get<uint8_t>();
				int energy_percent = r.get<uint8_t>();
				int resources = r.get<uint32_t>();
				int units_in_hangar = r.get<uint16_t>();
				int flags = r.get<uint16_t>();
				r.get<uint32_t>();
			}
		};

		tag_funcs["MRGN"] = [&](data_reader_le r) {
			// 64 or 256 entries
			while (r.left()) {
				int left = r.get<int32_t>();
				int top = r.get<int32_t>();
				int right = r.get<int32_t>();
				int bottom = r.get<int32_t>();
				a_string name = game_st.get_string(r.get<uint16_t>());
				int elevation_flags = r.get<uint16_t>();
			}
		};

		tag_funcs["TRIG"] = [&](data_reader_le r) {
			// todo
			while (r.left()) {
				r.skip(2400);
			}
		};

		// This doesn't really belong here, but it can stay until we have proper
		// game setup code
		in_game_loop = true;

		read_chunks({
			{"VER ", true},
			{"DIM ", true},
			{"ERA ", true},
			{"OWNR", true},
			{"SIDE", true},
			{"STR ", true},
			{"SPRP", true},
			{"FORC", true},
			{"VCOD", true}
		});

		reset();

		if (version == 59) {

			// todo: check game mode
			// this is for use map settings
			tag_list_t tags = {
				{"STR ", true},
				{"MTXM", true},
				{"THG2", true},
				{"MASK", true},
				{"UNIS", true},
				{"UPGS", true},
				{"TECS", true},
				{"PUNI", true},
				{"UPGR", true},
				{"PTEC", true},
				{"UNIx", false},
				{"UPGx", false},
				{"TECx", false},
				{"PUPx", false},
				{"PTEx", false},
				{"UNIT", true},
				{"UPRP", true},
				{"MRGN", true},
				{"TRIG", true}
			};
			read_chunks(tags);

		} else xcept("unsupported map version %d", version);

		in_game_loop = false;

	}
};

void global_init(global_state&st) {

	auto get_sprite_type = [&](int id) {
		if ((size_t)id >= 517) xcept("invalid sprite id %d", id);
		return &st.sprite_types.vec[id];
	};
	auto get_image_type = [&](int id) {
		if ((size_t)id >= 999) xcept("invalid image id %d", id);
		return &st.image_types.vec[id];
	};

	auto load_iscript_bin = [&]() {

		using namespace iscript_opcodes;
		std::array<const char*, 69> ins_data;

		ins_data[opc_playfram] = "2";
		ins_data[opc_playframtile] = "2";
		ins_data[opc_sethorpos] = "s1";
		ins_data[opc_setvertpos] = "s1";
		ins_data[opc_setpos] = "s1s1";
		ins_data[opc_wait] = "1";
		ins_data[opc_waitrand] = "11";
		ins_data[opc_goto] = "j";
		ins_data[opc_imgol] = "211";
		ins_data[opc_imgul] = "211";
		ins_data[opc_imgolorig] = "2";
		ins_data[opc_switchul] = "2";
		ins_data[opc___0c] = "";
		ins_data[opc_imgoluselo] = "211";
		ins_data[opc_imguluselo] = "211";
		ins_data[opc_sprol] = "211";
		ins_data[opc_highsprol] = "211";
		ins_data[opc_lowsprul] = "211";
		ins_data[opc_uflunstable] = "2";
		ins_data[opc_spruluselo] = "211";
		ins_data[opc_sprul] = "211";
		ins_data[opc_sproluselo] = "21";
		ins_data[opc_end] = "e";
		ins_data[opc_setflipstate] = "1";
		ins_data[opc_playsnd] = "2";
		ins_data[opc_playsndrand] = "v";
		ins_data[opc_playsndbtwn] = "22";
		ins_data[opc_domissiledmg] = "";
		ins_data[opc_attackmelee] = "v";
		ins_data[opc_followmaingraphic] = "";
		ins_data[opc_randcondjmp] = "1b";
		ins_data[opc_turnccwise] = "1";
		ins_data[opc_turncwise] = "1";
		ins_data[opc_turn1cwise] = "";
		ins_data[opc_turnrand] = "1";
		ins_data[opc_setspawnframe] = "1";
		ins_data[opc_sigorder] = "1";
		ins_data[opc_attackwith] = "1";
		ins_data[opc_attack] = "";
		ins_data[opc_castspell] = "";
		ins_data[opc_useweapon] = "1";
		ins_data[opc_move] = "1";
		ins_data[opc_gotorepeatattk] = "";
		ins_data[opc_engframe] = "1";
		ins_data[opc_engset] = "1";
		ins_data[opc___2d] = "";
		ins_data[opc_nobrkcodestart] = "";
		ins_data[opc_nobrkcodeend] = "";
		ins_data[opc_ignorerest] = "";
		ins_data[opc_attkshiftproj] = "1";
		ins_data[opc_tmprmgraphicstart] = "";
		ins_data[opc_tmprmgraphicend] = "";
		ins_data[opc_setfldirect] = "1";
		ins_data[opc_call] = "b";
		ins_data[opc_return] = "";
		ins_data[opc_setflspeed] = "2";
		ins_data[opc_creategasoverlays] = "1";
		ins_data[opc_pwrupcondjmp] = "b";
		ins_data[opc_trgtrangecondjmp] = "2b";
		ins_data[opc_trgtarccondjmp] = "22b";
		ins_data[opc_curdirectcondjmp] = "22b";
		ins_data[opc_imgulnextid] = "11";
		ins_data[opc___3e] = "";
		ins_data[opc_liftoffcondjmp] = "b";
		ins_data[opc_warpoverlay] = "2";
		ins_data[opc_orderdone] = "1";
		ins_data[opc_grdsprol] = "211";
		ins_data[opc___43] = "";
		ins_data[opc_dogrddamage] = "";

		a_unordered_map<int, a_vector<size_t>> animation_pc;
		a_vector<int> program_data;

		program_data.push_back(0); // invalid/null pc

		using data_loading::data_reader_le;

		a_vector<uint8_t> data;
		load_data_file(data, "scripts\\iscript.bin");
		data_reader_le base_r(data.data(), data.data() + data.size());
		auto r = base_r;
		size_t id_list_offset = r.get<uint32_t>();
		r.skip(id_list_offset);
		while (r.left()) {
			int id = r.get<int16_t>();
			if (id == -1) break;
			size_t script_address = r.get<uint16_t>();
			//log("loading script %d at %d\n", id, script_address);
			auto script_r = base_r;
			script_r.skip(script_address);
			auto signature = script_r.get<std::array<char, 4>>();
			//auto script_program_r = script_r;

			a_unordered_map<int, size_t> decode_map;

			auto decode_at = [&](size_t initial_address) {
				a_deque<std::tuple<size_t, size_t>> branches;
				std::function<size_t(int)> decode = [&](size_t initial_address) {
					if (!initial_address) xcept("iscript load: attempt to decode instruction at null address");
					auto in = decode_map.emplace(initial_address, 0);
					if (!in.second) {
						//log("instruction at 0x%04x already exists with index %d\n", initial_address, in.first->second);
						return in.first->second;
					}
					size_t initial_pc = program_data.size();
					//log("decoding at 0x%04x: initial_pc %d\n", initial_address, initial_pc);
					in.first->second = initial_pc;
					auto r = base_r;
					r.skip(initial_address);
					bool done = false;
					while (!done) {
						size_t pc = program_data.size();
						size_t cur_address = r.ptr - base_r.ptr;
						if (cur_address != initial_address) {
							auto in = decode_map.emplace(cur_address, pc);
							if (!in.second) {
								//log("0x%04x (0x%x): already decoded, inserting jump\n", cur_address, pc);
								program_data.push_back(opc_goto + 0x808091);
								program_data.push_back(in.first->second);
								break;
							}
						}
						size_t opcode = r.get<uint8_t>();
						if (opcode >= ins_data.size()) xcept("iscript load: at 0x%04x: invalid instruction %d", cur_address, opcode);
						//log("0x%04x (0x%x): opcode %d\n", cur_address, pc, opcode);
						program_data.push_back(opcode + 0x808091);
						const char*c = ins_data[opcode];
						while (*c) {
							if (*c == 's') {
								++c;
								if (*c=='1') program_data.push_back(r.get<int8_t>());
								else if (*c == '2') program_data.push_back(r.get<int16_t>());
							} else if (*c == '1') program_data.push_back(r.get<uint8_t>());
							else if (*c == '2') program_data.push_back(r.get<uint16_t>());
							else if (*c == 'v') {
								int n = r.get<uint8_t>();
								program_data.push_back(n);
								for (; n; --n) program_data.push_back(r.get<uint16_t>());
							} else if (*c == 'j') {
								size_t jump_address = r.get<uint16_t>();
								auto jump_pc_it = decode_map.find(jump_address);
								if (jump_pc_it == decode_map.end()) {
									program_data.pop_back();
									r = base_r;
									r.skip(jump_address);
								} else {
									program_data.push_back(jump_pc_it->second);
									done = true;
								}
							} else if (*c == 'b') {
								size_t branch_address = r.get<uint16_t>();
								branches.emplace_back(branch_address, program_data.size());
								program_data.push_back(0);
							} else if (*c == 'e') {
								done = true;
							}
							++c;
						}
					}
					return initial_pc;
				};
				size_t initial_pc = decode(initial_address);
				while (!branches.empty()) {
					auto v = branches.front();
					branches.pop_front();
					//log("doing branch to 0x%04x (fixup %x)\n", std::get<0>(v), std::get<1>(v));
					size_t pc = decode(std::get<0>(v));
					if ((int)pc != pc) xcept("iscript load: 0x%x does not fit in an int", pc);
					program_data[std::get<1>(v)] = (int)pc;
				}
				return initial_pc;
			};

			auto&anim_funcs = animation_pc[id];

			size_t highest_animation = script_r.get<uint32_t>();
			size_t animations = (highest_animation + 1 + 1)&-2;
			for (size_t i = 0; i < animations; ++i) {
				size_t anim_address = script_r.get<uint16_t>();
				if (!anim_address) {
					anim_funcs.push_back(0);
					continue;
				}
				auto anim_r = base_r;
				anim_r.skip(anim_address);
				anim_funcs.push_back(decode_at(anim_address));
			}
		}

		st.iscript.program_data = std::move(program_data);
		st.iscript.scripts.clear();
		for (auto&v : animation_pc) {
			auto&s = st.iscript.scripts[v.first];
			s.id = v.first;
			s.animation_pc = std::move(v.second);
		}
	};

	auto load_images = [&]() {

		using data_loading::data_reader_le;

		a_vector<uint8_t> data;
		load_data_file(data, "arr\\images.tbl");
		data_reader_le base_r(data.data(), data.data() + data.size());

		auto r = base_r;
		size_t file_count = r.get<uint16_t>();

		a_vector<grp_t> grps;
		a_vector<a_vector<a_vector<xy>>> lo_offsets;

		auto load_grp = [&](data_reader_le r) {
			grp_t grp;
			size_t frame_count = r.get<uint16_t>();
			grp.width = r.get<uint16_t>();
			grp.height = r.get<uint16_t>();
			grp.frames.resize(frame_count);
			for (size_t i = 0; i < frame_count; ++i) {
				auto&f = grp.frames[i];
				f.left = r.get<int8_t>();
				f.top = r.get<int8_t>();
				f.right = r.get<int8_t>();
				f.bottom = r.get<int8_t>();
				size_t file_offset = r.get<uint32_t>();
			}
			size_t index = grps.size();
			grps.push_back(std::move(grp));
			return index;
		};
		auto load_offsets = [&](data_reader_le r) {
			auto base_r = r;
			lo_offsets.emplace_back();
			auto&offs = lo_offsets.back();

			size_t frame_count = r.get<uint32_t>();
			size_t offset_count = r.get<uint32_t>();
			for (size_t f = 0; f < frame_count; ++f) {
				size_t file_offset = r.get<uint32_t>();
				auto r2 = base_r;
				r2.skip(file_offset);
				offs.emplace_back();
				auto&vec = offs.back();
				vec.resize(offset_count);
				for (size_t i = 0; i < offset_count; ++i) {
					int x = r2.get<int8_t>();
					int y = r2.get<int8_t>();
					vec[i] = { x,y };
				}
			}
			
			return (size_t)0;
		};

		a_unordered_map<size_t, size_t> loaded;
		auto load = [&](int index, std::function<size_t(data_reader_le)> f) {
			if (!index) return (size_t)0;
			auto in = loaded.emplace(index, 0);
			if (!in.second) return in.first->second;
			auto r = base_r;
			r.skip(2 + (index - 1) * 2);
			size_t fn_offset = r.get<uint16_t>();
			r = base_r;
			r.skip(fn_offset);
			a_string fn;
			while (char c = r.get<char>()) fn += c;

			a_vector<uint8_t> data;
			load_data_file(data, format("unit\\%s", fn));
			data_reader_le data_r(data.data(), data.data() + data.size());
			size_t loaded_index = f(data_r);
			in.first->second = loaded_index;
			return loaded_index;
		};

		a_vector<size_t> image_grp_index;
		std::array<a_vector<size_t>, 6> lo_indices;

		grps.emplace_back(); // null/invalid entry

		for (int i = 0; i < 999; ++i) {
			const image_type_t*image_type = get_image_type(i);
			image_grp_index.push_back(load(image_type->grp_filename_index, load_grp));
			lo_indices[0].push_back(load(image_type->attack_filename_index, load_offsets));
			lo_indices[1].push_back(load(image_type->damage_filename_index, load_offsets));
			lo_indices[2].push_back(load(image_type->special_filename_index, load_offsets));
			lo_indices[3].push_back(load(image_type->landing_dust_filename_index, load_offsets));
			lo_indices[4].push_back(load(image_type->lift_off_filename_index, load_offsets));
			lo_indices[5].push_back(load(image_type->shield_filename_index, load_offsets));
		}

		st.grps = std::move(grps);
		st.image_grp.resize(image_grp_index.size());
		for (size_t i = 0; i < image_grp_index.size(); ++i) {
			st.image_grp[i] = &st.grps.at(image_grp_index[i]);
		}
		st.lo_offsets = std::move(lo_offsets);
		st.image_lo_offsets.resize(999);
		for (size_t i = 0; i < lo_indices.size(); ++i) {
			for (int i2 = 0; i2 < 999; ++i2) {
				st.image_lo_offsets.at(i2).at(i) = &st.lo_offsets.at(lo_indices[i].at(i2));
			}
		}

	};

	st.flingy_types = data_loading::load_flingy_dat("arr\\flingy.dat");
	st.sprite_types = data_loading::load_sprites_dat("arr\\sprites.dat");
	st.image_types = data_loading::load_images_dat("arr\\images.dat");
	st.order_types = data_loading::load_orders_dat("arr\\orders.dat");

	auto fixup_sprite_type = [&](sprite_type_t*&ptr) {
		size_t index = (size_t)ptr;
		if (index == 517) ptr = nullptr;
		else ptr = get_sprite_type(index);
	};
	auto fixup_image_type = [&](image_type_t*&ptr) {
		size_t index = (size_t)ptr;
		if (index == 999) ptr = nullptr;
		else ptr = get_image_type(index);
	};

	for (auto&v : st.flingy_types.vec) {
		fixup_sprite_type(v.sprite);
	}
	for (auto&v : st.sprite_types.vec) {
		fixup_image_type(v.image);
	}

	load_iscript_bin();
	load_images();


	// This function returns std::round(std::sin(PI / 128 * i) * 256) for i [0, 63]
	// using only integer arithmetic.
	auto int_sin = [&](int x) {
		int x2 = x*x;
		int x3 = x2*x;
		int x4 = x3*x;
		int x5 = x4*x;

		int64_t a0 = 690668267770;
		int64_t a1 = 29198338;
		int64_t a2 = -71311096;
		int64_t a3 = 55269;
		int64_t a4 = 1482;
		int64_t a5 = 109946188959;

		return (int)(((x * a0 + x2 * a1 + x3 * a2 + x4 * a3 + x5 * a4) + a5 / 2) / a5);
	};

	// The sin lookup table is hardcoded into Broodwar. We generate it here.
	for (int i = 0; i <= 64; ++i) {
		int v = int_sin(i);
		st.direction_table[i].x = v;
		st.direction_table[63 - i].y = -v;
		st.direction_table[65 + (63 - i)].x = v;
		st.direction_table[63 + i].y = v;
		st.direction_table[128 + i].x = -v;
		st.direction_table[128 + (63 - i)].y = v;
		st.direction_table[(193 + (63 - i)) % 256].x = -v;
		st.direction_table[191 + i].y = -v;
	}

}

void init() {

	global_state global_st;
	game_state game_st;
	state st;
	st.global = &global_st;
	st.game = &game_st;

	global_init(global_st);

	game_load_functions game_load_funcs(st);
	game_load_funcs.load_map_file(R"(X:\Starcraft\StarCraft\maps\testone.scm)");

	for (unit_t* u : st.visible_units) {
		log("visible unit %p\n", u);
	}
}

}

