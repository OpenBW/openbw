
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

#include "bwdata.h"
using namespace BW;

#include "bwdat.h"

#include "bwenums.h"

namespace bwgame {
;

#include "util.h"

#include "game_types.h"

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
static_assert(sizeof(vf4_entry) == 32, "cv5_entry: wrong size");
struct vx4_entry {
	std::array<uint16_t, 16> images;
};
static_assert(sizeof(vx4_entry) == 32, "cv5_entry: wrong size");
struct vr4_entry {
	std::array<uint8_t, 64> bitmap;
};
static_assert(sizeof(vr4_entry) == 64, "cv5_entry: wrong size");

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
	operator bool() const {
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
	iscript_t iscript;
};

struct cache_state {

};

struct game_state {
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

	a_vector<tile_id> gfx_creep_tiles;
	a_vector<tile_id> gfx_tiles;
	a_vector<cv5_entry> cv5;
	a_vector<vf4_entry> vf4;
	a_vector<uint16_t> mega_tile_flags;

	units_dat_t units_dat;
	weapons_dat_t weapons_dat;
	upgrades_dat_t upgrades_dat;
	techdata_dat_t techdata_dat;
	flingy_dat_t flingy_dat;
	sprites_dat_t sprites_dat;
	images_dat_t images_dat;

	std::array<std::array<bool, 228>, 12> unit_type_allowed;
	std::array<std::array<int, 61>, 12> max_upgrade_levels;
	std::array<std::array<bool, 44>, 12> tech_available;

	std::array<xy, 12> start_locations;
};

struct state_base {

	const global_state*global;
	game_state*game;

	int update_tiles_countdown;

	std::array<int8_t, 12> selection_circle_color;

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

	std::array<std::array<int8_t, 61>, 12> upgrade_levels;
	std::array<std::array<bool, 44>, 12> tech_researched;

	// rearrange order 228 12 ?
	std::array<std::array<int32_t, 12>, 228> completed_unit_count;

	uint32_t local_mask;

	std::array<int, 12> shared_vision;

	a_vector<tile_t> tiles;
	a_vector<uint16_t> tiles_mega_tile_index;

	std::array<int, 0x100> random_counts;
	int total_random_counts;
	uint32_t lcg_rand_state;

	int last_error;
};

struct state : state_base {

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

	state() = default;
	state(state&) = delete;
	state(state&&) = default;
	state&operator=(state&) = delete;
	state&operator=(state&&) = default;
};

struct state_functions {

	state&st;
	const global_state&global_st = *st.global;
	const game_state&game_st = *st.game;

	const units_dat_t&units_dat = game_st.units_dat;
	const weapons_dat_t&weapons_dat = game_st.weapons_dat;
	const upgrades_dat_t&upgrades_dat = game_st.upgrades_dat;
	const techdata_dat_t&techdata_dat = game_st.techdata_dat;
	const flingy_dat_t&flingy_dat = game_st.flingy_dat;
	const sprites_dat_t&sprites_dat = game_st.sprites_dat;
	const images_dat_t&images_dat = game_st.images_dat;

	state_functions(state&st) : st(st) {}

	bool in_game_loop = false;
	bool update_tiles = false;

	bool Completed(unit_t*u) {
		return !!(u->status_flags&StatusFlags::Completed);
	};
	bool InBuilding(unit_t*u) {
		return !!(u->status_flags&StatusFlags::InBuilding);
	};
	bool Unmovable(unit_t*u) {
		return !!(u->status_flags&StatusFlags::Unmovable);
	};
	bool Burrowed(unit_t*u) {
		return !!(u->status_flags&StatusFlags::Burrowed);
	};
	bool IsABuilding(unit_t*u) {
		return !!(u->status_flags&StatusFlags::IsABuilding);
	};
	bool IsAUnit(unit_t*u) {
		return !!(u->status_flags&StatusFlags::IsAUnit);
	};
	bool GroundedBuilding(unit_t*u) {
		return !!(u->status_flags&StatusFlags::GroundedBuilding);
	};
	bool IsHallucination(unit_t*u) {
		return !!(u->status_flags&StatusFlags::IsHallucination);
	};
	bool InAir(unit_t*u) {
		return !!(u->status_flags&StatusFlags::InAir);
	};

	bool SA_Subunit(unit_t*u) {
		return !!(units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Subunit);
	};
	bool SA_Worker(unit_t*u) {
		return !!(units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Worker);
	};

	bool SPR_Hidden(sprite_t*sprite) {
		return !!(sprite->flags&SpriteFlags::Hidden);
	};

	unit_t* get_unit(unit_id id) {
		size_t idx = id.index();
		if (!idx) return nullptr;
		size_t actual_index = idx - 1;
		if (actual_index >= st.units.size()) xcept("attempt to dereference invalid unit id %d (actual index %d)", idx, actual_index);
		unit_t*u = &st.units[actual_index];
		if (u->unit_id_generation != id.generation()) return nullptr;
		return u;
	};


	void setAllImageGroupFlagsPal11(sprite_t*sprite) {
		for (image_t*img : sprite->images) {
			if (img->palette_type == 0xb) img->flags |= 1;
		}
	};

	int visible_hp_plus_shields(unit_t*u) {
		int r = 0;
		if (units_dat.ShieldEnable[u->unit_type]) r += u->shield_points >> 8;
		r += (u->hp + 0xff) >> 8;
		return r;
	};
	int max_visible_hp(unit_t*u) {
		int hp = units_dat.HitPoints[u->unit_type] >> 8;
		if (hp == 0) hp = (u->hp + 0xff) >> 8;
		if (hp == 0) hp = 1;
		return hp;
	};
	int max_visible_hp_plus_shields(unit_t*u) {
		int shields = 0;
		if (units_dat.ShieldEnable[u->unit_type]) shields += units_dat.ShieldAmount[u->unit_type];
		return max_visible_hp(u) + shields;
	};

	int getUnitStrength(unit_t*u, bool ground) {
		if (u->unit_type == UnitTypes::Zerg_Larva || u->unit_type == UnitTypes::Zerg_Egg || u->unit_type == UnitTypes::Zerg_Cocoon || u->unit_type == UnitTypes::Zerg_Lurker_Egg) return 0;
		int vis_hp_shields = visible_hp_plus_shields(u);
		int max_vis_hp_shields = max_visible_hp_plus_shields(u);
		if (u->status_flags&StatusFlags::IsHallucination) {
			if (vis_hp_shields < max_vis_hp_shields) return 0;
		}

		int r = ground ? game_st.unit_ground_strength[u->unit_type] : game_st.unit_air_strength[u->unit_type];
		if (u->unit_type == UnitTypes::Terran_Bunker) {
			xcept("fixme getUnitStrength bunker; see getUnitStrength_AirOrGround");
		}
		if (units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Spellcaster) {
			if (~u->status_flags&StatusFlags::IsHallucination) r += (u->energy >> 8) / 2;
		}
		return r * vis_hp_shields / max_vis_hp_shields;
	};

	void setUnitHP(unit_t*u, int hitpoints) {
		u->hp = hitpoints;
		if (u->hp > units_dat.HitPoints[u->unit_type]) u->hp = units_dat.HitPoints[u->unit_type];
		if (u->sprite->flags & SpriteFlags::Selected && u->sprite->visibility_flags&st.local_mask) {
			setAllImageGroupFlagsPal11(u->sprite);
		}
		if (u->status_flags & StatusFlags::Completed) {
			// damage overlay stuff

			u->air_strength = getUnitStrength(u, false);
			u->ground_strength = getUnitStrength(u, true);
		}
	};

	bool is_frozen_or_in_air(unit_t*u) {
		if (u->status_flags&StatusFlags::InAir) return true;
		if (u->lockdown_timer) return true;
		if (u->stasis_timer) return true;
		if (u->maelstrom_timer) return true;
		return false;
	};

	void set_buttonset(unit_t*u, int type) {
		if (type != UnitTypes::None && ~units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Building) {
			if (is_frozen_or_in_air(u)) return;
		}
		u->current_button_set = type;
	};

	image_t* find_overlay(sprite_t*sprite, int first, int last) {
		for (image_t*i : sprite->images) {
			if (i->image_id >= first && i->image_id <= last) return i;
		}
		return nullptr;
	};

	void playImageIscript(image_t*img, int id) {
		// todo
		xcept("playImageIscript");
	};

	// RemoveOverlaysBetween
	void freeze_effect_end(unit_t*u, int first, int last) {
		bool still_frozen = is_frozen_or_in_air(u);
		if (!still_frozen) {
			u->status_flags &= ~StatusFlags::DoodadStatesThing;
			xcept("freeze_effect_end: orderComputer_cl");
			// orderComputer_cl(u->subunit, units_dat.ReturntoIdle[u->subunit->unit_type]);
		}
		image_t*overlay = find_overlay(u->sprite, first, last);
		if (!overlay && u->subunit) overlay = find_overlay(u->subunit->sprite, first, last);
		if (overlay) playImageIscript(overlay, idenums::ISCRIPT_Anim_Death);
		if (units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Worker && !still_frozen) {
			// sub_468DB0
			unit_t*target = u->worker.harvest_target;
			if (target && units_dat.SpecialAbilityFlags[target->unit_type] & UnitPrototypeFlags::FlyingBuilding) {
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
		set_buttonset(u, u->unit_type);
		if (~units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Invincible) {
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
		if (~units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Spellcaster) return;
		xcept("updateEnergyTimer");
	};

	bool unit_hp_below_33_percent(unit_t*u) {
		int max_hp = max_visible_hp(u);
		int hp = (u->hp + 0xff) >> 8;
		return hp * 100 / max_hp <= 33;
	};

	void updateUnitTimers(unit_t*u) {
		if (u->main_order_timer) --u->main_order_timer;
		if (u->ground_weapon_cooldown) --u->ground_weapon_cooldown;
		if (u->air_weapon_cooldown) --u->air_weapon_cooldown;
		if (u->spell_cooldown) --u->spell_cooldown;
		if (units_dat.ShieldEnable[u->unit_type]) {
			int max_shields = units_dat.ShieldAmount[u->unit_type] << 8;
			if (u->shield_points != max_shields) {
				u->shield_points += 7;
				if (u->shield_points > max_shields) u->shield_points = max_shields;
				if (u->sprite->flags & SpriteFlags::Selected) {
					setAllImageGroupFlagsPal11(u->sprite);
				}
			}
		}
		if (u->unit_type == UnitTypes::Zerg_Zergling || u->unit_type == UnitTypes::Hero_Devouring_One) {
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
			if (units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::RegeneratesHP) {
				if (u->hp > 0 && units_dat.HitPoints[u->unit_type]) {
					setUnitHP(u, u->hp + 4);
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
			int gf = units_dat.StarEditGroupFlags[u->unit_type];
			if (gf&GroupFlags::Terran && ~gf&(GroupFlags::Zerg | GroupFlags::Protoss)) {
				if (u->status_flags&StatusFlags::GroundedBuilding || units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::FlyingBuilding) {
					if (unit_hp_below_33_percent(u)) {
						xcept("killTargetUnitCheck(...)");
					}
				}
			}
		}
	};

	void UpdateUnitOrderData(unit_t*u) {
		if (~units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Subunit && ~u->sprite->flags&SpriteFlags::Hidden) {
			update_selection_sprite(u->sprite, st.selection_circle_color[u->owner]);
		}

		updateUnitTimers(u);

	};

	int unit_movepos_state(unit_t*u) {
		if (u->sprite->position != u->move_target.pos) return 0;
		return Unmovable(u) ? 2 : 1;
	};

	bool unit_order_dead(unit_t*u) {
		return u->order_id == Orders::Die && u->order_state == 1;
	};

	bool Unit_ExecPathingState(unit_t*u) {

		bool refresh_vision = update_tiles;

		auto UMInitialize = [&](unit_t*u) {
			u->pathing_flags &= ~(1 | 2);
			if (u->sprite->elevation_level) u->pathing_flags |= 1;
			u->contour_bounds = { 0,0,0,0 };
			int next_state = UM_Lump;
			if (!SA_Subunit(u) && InBuilding(u)) {
				next_state = UM_InitSeq;
			} else if (!u->sprite || unit_order_dead(u)) {
				next_state = UM_Lump;
			} else if (InBuilding(u)) {
				next_state = UM_Bunker;
			} else if (SPR_Hidden(u->sprite)) {
				if (u->movement_flags & MovementFlags::Accelerating || unit_movepos_state(u) == 0) {
					// SetMoveTarget_xy(u)
					// ...
					xcept("todo hidden sprite pathing stuff");
				}
				next_state = UM_Hidden;
			} else if (Burrowed(u)) {
				next_state = UM_Lump;
			}
			// Doesn't this seems backwards? It sure does.
			else if (IsABuilding(u)) next_state = u->pathing_flags & 1 ? UM_AtRest : UM_Flyer;
			else if (IsAUnit(u)) next_state = SA_Subunit(u) ? UM_BldgTurret : UM_Turret;
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
		if (Completed(u)) return false;
		int tt = u->build_queue[u->build_queue_slot];
		return tt == UnitTypes::Zerg_Hive || tt == UnitTypes::Zerg_Lair || tt == UnitTypes::Zerg_Greater_Spire || tt == UnitTypes::Zerg_Spore_Colony || tt == UnitTypes::Zerg_Sunken_Colony;
	};


	int unit_sight_range2(unit_t*u, bool ignore_blindness) {
		if (GroundedBuilding(u) && !Completed(u) && !is_transforming_zerg_building(u)) return 4;
		if (!ignore_blindness && u->is_blind) return 2;
		if (u->unit_type == UnitTypes::Terran_Ghost && st.upgrade_levels[u->owner][UpgradeTypes::Ocular_Implants]) return 11;
		if (u->unit_type == UnitTypes::Zerg_Overlord && st.upgrade_levels[u->owner][UpgradeTypes::Antennae]) return 11;
		if (u->unit_type == UnitTypes::Protoss_Observer && st.upgrade_levels[u->owner][UpgradeTypes::Sensor_Array]) return 11;
		if (u->unit_type == UnitTypes::Protoss_Scout && st.upgrade_levels[u->owner][UpgradeTypes::Apial_Sensors]) return 11;
		return units_dat.SightRange[u->unit_type];
	};
	int unit_sight_range(unit_t*u) {
		return unit_sight_range2(u, false);
	};
	int unit_sight_range_ignore_blindness(unit_t*u) {
		return unit_sight_range2(u, true);
	};

	bool visible_to_everyone(unit_t*u) {
		if (SA_Worker(u)) {
			if (u->worker.powerup && u->worker.powerup->unit_type == UnitTypes::Powerup_Flag) return true;
			else return false;
		}
		if (!units_dat.SpaceProvided[u->unit_type]) return false;
		if (u->unit_type == UnitTypes::Zerg_Overlord && !st.upgrade_levels[u->owner][UpgradeTypes::Ventral_Sacs]) return false;
		if (IsHallucination(u)) return false;
		for (auto idx : u->loaded_units) {
			unit_t*lu = get_unit(idx);
			if (!lu || !lu->sprite) continue;
			if (unit_order_dead(lu)) continue;
			if (!SA_Worker(lu)) continue;
			if (lu->worker.powerup && lu->worker.powerup->unit_type == UnitTypes::Powerup_Flag) return true;
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
		tile_id creep_tile = game_st.gfx_creep_tiles.at(index);
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
		int visibility_mask = ~reveal_to;
		int height_mask = 0;
		if (!in_air) {
			int height = get_ground_height_at(pos);
			if (height == 2) height_mask = tile_t::flag_very_high;
			else if (height == 1) height_mask = tile_t::flag_high | tile_t::flag_high;
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
				for (int i2 = 0; i2 < cur->prev_count; ++i2) {
					if (!((i2 == 0 ? cur->prev : cur->prev2)->vision_propagation & required_tile_mask.raw)) {
						okay = true;
						break;
					}
				}
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

	void refreshUnitVision(unit_t*u) {
		if (u->owner >= 8 && !u->parasite_flags) return;
		if (u->unit_type == UnitTypes::Terran_Nuclear_Missile) return;
		int visible_to = 0;
		if (visible_to_everyone(u) || (u->unit_type == UnitTypes::Powerup_Flag && u->order_id == Orders::UnusedPowerup)) visible_to = 0xff;
		else {
			visible_to = st.shared_vision[u->owner] | u->parasite_flags;
			if (u->parasite_flags) {
				visible_to |= u->parasite_flags;
				for (size_t i = 0; i < 12; ++i) {
					if (~u->parasite_flags&(1 << i)) continue;
					visible_to |= st.shared_vision[i];
				}
			}
			reveal_sight_at(u->sprite->position, unit_sight_range(u), visible_to, InAir(u));
		}
	};

	void update_unit_pathing(unit_t*u) {

		bool refresh_vision = Unit_ExecPathingState(u);
		if (refresh_vision) refreshUnitVision(u);
		if (u->status_flags&StatusFlags::Completed) {
			if (u->subunit && ~units_dat.SpecialAbilityFlags[u->unit_type] & UnitPrototypeFlags::Subunit) {
				xcept("update_unit_pathing: subunit stuff");
			}
		}
	};

	void UpdateUnitSpriteInfo(unit_t*u) {

	};

	void UpdateUnits() {

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
				if (~u->status_flags&StatusFlags::IsHallucination && (u->unit_type != UnitTypes::Zerg_Overlord || st.upgrade_levels[u->owner][UpgradeTypes::Ventral_Sacs]) && units_dat.SpaceProvided[u->unit_type]) {
					xcept("sub_4EB2F0 loaded unit stuff");
				} else if (u->subunit) {
					u->subunit->status_flags &= ~StatusFlags::CanNotAttack;
				}
			}
		}
		if (st.completed_unit_count[UnitTypes::Spell_Disruption_Web][11]) {
			xcept("disruption web stuff");
		}
		// some_units_loaded_and_disruption_web end

		//for (unit_t*u = st.first_sight_related_unit; u; u = u->next) {
		//for (unit_t*u = st.first_sight_related_unit; u;) {
		for (unit_t*u : st.sight_related_units) {
			xcept("fixme first_sight_related_unit stuff in UpdateUnits");
		}

		for (unit_t*u : st.visible_units) {
			update_unit_pathing(u);
		}

		if (update_tiles) {
			//for (unit_t*u = st.first_scanner_sweep_unit; u; u = u->next) {
			for (unit_t*u : st.scanner_sweep_units) {
				xcept("moo");
				refreshUnitVision(u);
			}
		}

		for (unit_t*u : st.visible_units) {
			UpdateUnitSpriteInfo(u);
		}

		for (unit_t*u : st.visible_units) {
			UpdateUnitOrderData(u);
		}
	};

	void game_loop() {

		in_game_loop = true;

		if (st.update_tiles_countdown == 0) st.update_tiles_countdown = 100;
		--st.update_tiles_countdown;
		update_tiles = st.update_tiles_countdown == 0;

		UpdateUnits();

		in_game_loop = false;

	}

	int lcg_rand(int source) {
		++st.random_counts[source];
		++st.total_random_counts;
		st.lcg_rand_state *= 22695477;
		++st.lcg_rand_state;
		return (st.lcg_rand_state >> 16) & 0x7fff;
	}

	void error_string(int str_index) {
		if (str_index) log(" error %d: (insert string here)\n", str_index);
		st.last_error = str_index;
	}

	bool is_in_bounds(int unit_type, xy pos) {
		size_t ux = pos.x;
		size_t uy = pos.y;
		if (ux - units_dat.UnitSizeLeft[unit_type] >= game_st.map_width) return false;
		if (uy - units_dat.UnitSizeUp[unit_type] >= game_st.map_height) return false;
		if (ux - units_dat.UnitSizeRight[unit_type] >= game_st.map_width) return false;
		if (uy - units_dat.UnitSizeDown[unit_type] >= game_st.map_height) return false;
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

	void set_sprite_visibility(sprite_t*sprite, int visibility_flags) {
		visibility_flags &= st.local_mask;
		if ((sprite->visibility_flags&visibility_flags) == visibility_flags) return;
		sprite->visibility_flags = visibility_flags;
		for (image_t*i : sprite->images) i->flags |= 1;
	}

	void iscript_set_script(image_t*image, int script_id) {
		auto i = global_st.iscript.scripts.find(script_id);
		if (i == global_st.iscript.scripts.end()) {
			xcept("script %d does not exist", script_id);
		}
		image->iscript_state.current_script = &i->second;
	}

	void iscript_execute(image_t*image, iscript_state_t&state, bool no_side_effects, int*distance_moved) {
		xcept("iscript_execute");
	}

	void iscript_run_anim(image_t*image, int new_anim) {
		using namespace iscript_anims;
		int old_anim = image->iscript_state.animation;
		if (new_anim == Death && old_anim == Death) return;
		if (~image->flags & image_t::flag_iscript_running && new_anim != Init && new_anim != Death) return;
		if ((new_anim == Walking || new_anim == IsWorking) && new_anim == old_anim) return;
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
		iscript_execute(image, image->iscript_state, false, nullptr);
	}

	image_t* create_image(int image_id, sprite_t*sprite, xy offset, int direction) {
		if (st.free_images.empty()) return nullptr;
		image_t*image = &st.free_images.front();
		st.free_images.pop_front();
		if (sprite->images.empty()) sprite->main_image = image;
		bw_insert_list(sprite->images, *image);

		auto initialize_image = [&]() {
			image->image_id = image_id;
			image->grp_file = nullptr; // fixme? might only be needed for rendering
			int flags = 0;
			if (images_dat.isTurnable[image_id] & 1) flags |= 8;
			if (images_dat.isClickable[image_id] & 1) flags |= image_t::flag_hidden;
			image->flags = flags;
			image->frame_set = 0;
			image->direction = 0;
			image->frame_index = 0;
			image->sprite = sprite;
			image->offset = offset;
			image->grp_bounds = { 0,0,0,0 };
			image->coloring_data = 0;
			image->iscript_state.current_script = nullptr;
			image->iscript_state.program_counter = 0;
			image->iscript_state.return_address = 0;
			image->iscript_state.animation = 0;
			image->iscript_state.wait = 0;
			int drawfunc = images_dat.drawFunction[image_id];
			if (drawfunc == 14) image->coloring_data = sprite->owner;
			if (drawfunc == 9) {
				image->coloring_data = 0; // fixme, see InitializeImageData
			}
		};
		auto start_image = [&]() {
			int drawfunc = images_dat.drawFunction[image_id];
			image->palette_type = drawfunc;
			// fixme ?
			// image->update = update_functions[drawfunc];
			// if (image->flags&2) image->draw = draw_functions2[drawfunc];
			// else image->draw = draw_functions[drawfunc];
			if (drawfunc == 17) {
				// coloring_data might be a union, since this is written
				// using two single-byte writes
				image->coloring_data = 48 | (2 << 8);
			}
			image->flags |= 1;
			image->flags &= 16;
			if (images_dat.useFullIscript[image_id] & 1) image->flags |= 16;
			iscript_set_script(image, images_dat.iscriptEntry[image_id]);
			iscript_run_anim(image, iscript_anims::Init);
			xcept("updateGraphicData");
		};

		initialize_image();
		start_image();
		xcept("create image waa\n");

		return nullptr;
	}

	sprite_t* create_sprite(int sprite_id, xy pos, int owner) {

		if (st.free_sprites.empty()) return nullptr;
		sprite_t*sprite = &st.free_sprites.front();
		st.free_sprites.pop_front();

		auto init_sprite = [&]() {
			if ((size_t)pos.x >= game_st.map_width || (size_t)pos.y >= game_st.map_height) return false;
			sprite->owner = owner;
			sprite->sprite_id = sprite_id;
			sprite->flags = 0;
			sprite->position = pos;
			sprite->visibility_flags = ~0;
			sprite->selection_timer = 0;
			if (!sprites_dat.isVisible[sprite_id]) {
				sprite->flags |= SpriteFlags::Hidden;
				set_sprite_visibility(sprite, 0);
			}
			if (!create_image(sprites_dat.image[sprite_id], sprite, { 0,0 }, 0)) return false;
			return true;
		};

		if (!init_sprite()) {
			bw_insert_list(st.free_sprites, *sprite);
			return nullptr;
		}
		add_sprite_to_tile_line(sprite);

		return sprite;
	}

	bool initialize_flingy(flingy_t*f, int flingy_id, xy pos, int owner, int direction) {
		f->flingy_id = flingy_id;
		f->movement_flags = 0;
		f->current_speed2 = 0;
		f->flingy_top_speed = flingy_dat.topSpeed[flingy_id];
		f->flingy_acceleration = flingy_dat.acceleration[flingy_id];
		f->flingy_turn_radius = flingy_dat.turnSpeed[flingy_id];
		f->flingy_movement_type = flingy_dat.moveControl[flingy_id];

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

		int sprite_id = flingy_dat.sprite[flingy_id];
		f->sprite = create_sprite(sprite_id, pos, owner);

		if (f->sprite) {
			int dir = f->current_direction1;
			for (image_t*i : f->sprite->images) {
				xcept("set_image_direction(i,dir)");
			}
		}

		return false;
	}

	bool initialize_unit_type(unit_t*u, int type, xy pos, int owner) {

		int flingy_id = units_dat.Graphics[type];
		if (!initialize_flingy(u, flingy_id, pos, owner, 0)) return false;

		xcept("initialize unit type please");

		return true;
	}

	unit_t* new_unit(int type, xy pos, int owner) {

		if (st.free_units.empty()) {
			error_string(61); // Cannot create more units
			return nullptr;
		}
		if (!is_in_bounds(type, pos)) {
			error_string(0);
			return nullptr;
		}
		unit_t*u = st.free_units.front();
		auto construct_unit = [&]() {

			u->worker.harvest_target = nullptr;
			u->worker.gather_link = { nullptr, nullptr };

			u->pylon.psi_link = { nullptr, nullptr };

			u->burrowed_unit_link = { nullptr, nullptr };

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
			u->previous_hp = 0;
			u->ai = nullptr;

			if (!initialize_unit_type(u, type, pos, owner)) return false;
			xcept("unit type initialized");

			return true;
		};
		if (!construct_unit()) {
			error_string(62); // Unable to create unit
			return nullptr;
		}
		st.free_units.pop_front();

		return nullptr;

	}

	unit_t* create_unit(int type, xy pos, int owner) {

		if (in_game_loop) {
			lcg_rand(14);
		}
		unit_t*u = new_unit(type, pos, owner);


		return u;
	}

	unit_t* create_initial_unit(int type, xy pos, int owner) {
		unit_t*u = create_unit(type, pos, owner);
		xcept("create_initial_unit %p", u);
	}

};

void advance(state&st) {

	state_functions funcs(st);

	funcs.game_loop();

}

template<typename T>
struct is_std_array : std::false_type {};
template<typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type{};

template<bool default_little_endian = true>
struct data_reader {
	uint8_t*ptr = nullptr;
	uint8_t*end = nullptr;
	data_reader() = default;
	data_reader(uint8_t*ptr, uint8_t*end) : ptr(ptr), end(end) {}
	template<typename T, bool little_endian, typename std::enable_if<is_std_array<T>::value>::type* = nullptr>
	T value_at(uint8_t*ptr) {
		T r;
		for (auto&v : r) {
			v = value_at<std::remove_reference<decltype(v)>::type, little_endian>(ptr);
			ptr += sizeof(v);
		}
		return r;
	}
	template<typename T, bool little_endian, typename std::enable_if<!is_std_array<T>::value>::type* = nullptr>
	T value_at(uint8_t*ptr) {
		static_assert(std::is_integral<T>::value, "can only read integers and arrays of integers");
		T r = 0;
		for (size_t i = 0; i < sizeof(T); ++i) {
			r |= ptr[i] << ((little_endian ? i : sizeof(T) - 1 - i) * 8);
		}
		return r;
	}
	template<typename T, bool little_endian = default_little_endian>
	T get() {
		if (ptr + sizeof(T) - end > 0) xcept("data_reader: attempt to read past end");
		ptr += sizeof(T);
		return value_at<T, little_endian>(ptr - sizeof(T));
	}
	uint8_t*get_n(size_t n) {
		uint8_t*r = ptr;
		if (ptr + n - end > 0) xcept("data_reader: attempt to read past end");
		ptr += n;
		return r;
	}
	template<typename T, bool little_endian = default_little_endian>
	a_vector<T> get_vec(size_t n) {
		uint8_t*data = get_n(n*sizeof(T));
		a_vector<T> r(n);
		for (size_t i = 0; i < n; ++i, data += sizeof(T)) {
			r[i] = value_at<T, little_endian>(data);
		}
		return r;
	}
	void skip(size_t n) {
		if (ptr + n - end > 0 || ptr + n < ptr) xcept("data_reader: attempt to seek past end");
		ptr += n;
	}
	size_t left() {
		return end - ptr;
	}
};

using data_reader_le = data_reader<true>;
using data_reader_be = data_reader<false>;

struct game_load_functions : state_functions {

	game_load_functions(state&st) : state_functions(st) {}

	game_state&game_st = *st.game;

	units_dat_t&units_dat = game_st.units_dat;
	weapons_dat_t&weapons_dat = game_st.weapons_dat;
	upgrades_dat_t&upgrades_dat = game_st.upgrades_dat;
	techdata_dat_t&techdata_dat = game_st.techdata_dat;
	flingy_dat_t&flingy_dat = game_st.flingy_dat;
	sprites_dat_t&sprites_dat = game_st.sprites_dat;
	images_dat_t&images_dat = game_st.images_dat;

	void reset() {

		units_dat = load_units_dat("arr\\units.dat");
		weapons_dat = load_weapons_dat("arr\\weapons.dat");
		upgrades_dat = load_upgrades_dat("arr\\upgrades.dat");
		techdata_dat = load_techdata_dat("arr\\techdata.dat");
		flingy_dat = load_flingy_dat("arr\\flingy.dat");
		sprites_dat = load_sprites_dat("arr\\sprites.dat");
		images_dat = load_images_dat("arr\\images.dat");

		for (auto&v : game_st.unit_type_allowed) v.fill(true);
		for (auto&v : game_st.tech_available) v.fill(true);
		st.tech_researched.fill({});
		for (auto&v : game_st.max_upgrade_levels) {
			for (size_t i = 0; i < game_st.max_upgrade_levels.size(); ++i) {
				v[i] = upgrades_dat.MaxRepeats[i];
			}
		}
		st.upgrade_levels.fill({});
		// upgrade progress?
		// UPRP stuff?

		auto set_acquisition_ranges = [&]() {
			for (int i = 0; i < 228; ++i) {
				int shooting_type = i;
				if (units_dat.Subunit1[i]) shooting_type = units_dat.Subunit1[i];
				int ground_weapon = units_dat.GroundWeapon[shooting_type];
				int air_weapon = units_dat.AirWeapon[shooting_type];
				int acq_range = units_dat.TargetAcquisitionRange[shooting_type];
				if (ground_weapon < 130) acq_range = std::max(acq_range, (int)weapons_dat.MaximumRange[ground_weapon]);
				if (air_weapon < 130) acq_range = std::max(acq_range, (int)weapons_dat.MaximumRange[air_weapon]);
				units_dat.TargetAcquisitionRange[i] = acq_range;
			}
		};
		set_acquisition_ranges();

		calculate_unit_strengths();

		generate_sight_values();

		load_tile_stuff();

		st.tiles.resize(game_st.map_tile_width*game_st.map_tile_height);
		st.tiles_mega_tile_index.resize(st.tiles.size());

		st.order_timer_counter = 10;
		st.secondary_order_timer_counter = 150;

		st.visible_units.clear();
		st.hidden_units.clear();
		st.scanner_sweep_units.clear();
		st.sight_related_units.clear();
		for (auto&v : st.player_units) v.clear();

		st.free_units.clear();
		for (unit_t*u : st.units) {
			memset(u, 0, sizeof(*u));
			bw_insert_list(st.free_units, *u);
		}

		st.free_sprites.clear();
		for (sprite_t*sprite : st.sprites) {
			bw_insert_list(st.free_sprites, *sprite);
		}
		st.sprites_on_tile_line.resize(game_st.map_tile_height);

		st.free_images.clear();
		for (image_t*image : st.images) {
			bw_insert_list(st.free_images, *image);
		}

		st.last_error = 0;
	}

	int get_unit_strength(int unit_type, int weapon_type) {
		switch (unit_type) {
		case UnitTypes::Terran_Vulture_Spider_Mine:
		case UnitTypes::Protoss_Interceptor:
		case UnitTypes::Protoss_Scarab:
			return 0;
		}
		int hp = units_dat.HitPoints[unit_type] >> 8;
		if (units_dat.ShieldEnable[unit_type]) hp += units_dat.ShieldAmount[unit_type];
		if (hp == 0) return 0;
		int fact = weapons_dat.DamageFactor[weapon_type];
		int cd = weapons_dat.WeaponCooldown[weapon_type];
		int dmg = weapons_dat.DamageAmount[weapon_type];
		int range = weapons_dat.MaximumRange[weapon_type];
		unsigned int a = (range / (unsigned)cd) * fact * dmg;
		unsigned int b = (hp * ((int64_t)(fact*dmg << 11) / cd)) >> 8;
		int score = (int)(sqrt(a + b)*7.58);
		switch (unit_type) {
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

			int shooting_type = idx;
			int air_strength = 0;
			int ground_strength = 0;
			if (shooting_type != UnitTypes::Zerg_Larva && shooting_type != UnitTypes::Zerg_Egg && shooting_type != UnitTypes::Zerg_Cocoon && shooting_type != UnitTypes::Zerg_Lurker_Egg) {
				if (shooting_type == UnitTypes::Protoss_Carrier || shooting_type == UnitTypes::Hero_Gantrithor) shooting_type = UnitTypes::Protoss_Interceptor;
				else if (shooting_type == UnitTypes::Protoss_Reaver || shooting_type == UnitTypes::Hero_Warbringer) shooting_type = UnitTypes::Protoss_Scarab;
				else if (units_dat.Subunit1[shooting_type] != UnitTypes::None) shooting_type = units_dat.Subunit1[shooting_type];

				int air_weapon = units_dat.AirWeapon[shooting_type];
				if (air_weapon == WeaponTypes::None) air_strength = 1;
				else air_strength = get_unit_strength(idx, air_weapon);

				int ground_weapon = units_dat.GroundWeapon[shooting_type];
				if (ground_weapon == WeaponTypes::None) ground_strength = 1;
				else ground_strength = get_unit_strength(idx, ground_weapon);
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

		a_unordered_map<tag_t, std::function<void(data_reader_le)>, tag_t> tag_funcs;

		auto tagstr = [&](tag_t tag) {
			std::array<char, 5> name;
			name[0] = tag.data[0];
			name[1] = tag.data[1];
			name[2] = tag.data[2];
			name[3] = tag.data[3];
			name[4] = 0;
			return name;
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
					if (std::get<1>(v)) xcept("map is missing required chunk '%s'", tagstr(tag).data());
				} else {
					if (!tag_funcs[tag]) xcept("tag '%s' is missing a function", tagstr(tag).data());
					log("loading tag '%s'...\n", tagstr(tag).data());
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
			auto shield = r.get_vec<uint16_t>(228);
			auto armor = r.get_vec<uint8_t>(228);
			auto build_time = r.get_vec<uint16_t>(228);
			auto mineral_cost = r.get_vec<uint16_t>(228);
			auto gas_cost = r.get_vec<uint16_t>(228);
			auto string_index = r.get_vec<uint16_t>(228);
			auto weapon_damage = r.get_vec<uint16_t>(broodwar ? 130 : 100);
			auto weapon_bonus_damage = r.get_vec<uint16_t>(broodwar ? 130 : 100);
			for (int i = 0; i < 228; ++i) {
				if (uses_default_settings[i]) continue;
				game_st.units_dat.HitPoints[i] = hp[i];
				game_st.units_dat.ShieldAmount[i] = shield[i];
				game_st.units_dat.Armor[i] = armor[i];
				game_st.units_dat.BuildTime[i] = build_time[i];
				game_st.units_dat.MineralCost[i] = mineral_cost[i];
				game_st.units_dat.VespeneCost[i] = gas_cost[i];
				game_st.units_dat.UnitMapString[i] = string_index[i];
				int shooting_type = i;
				if (game_st.units_dat.Subunit1[i]) shooting_type = game_st.units_dat.Subunit1[i];
				int ground_weapon = game_st.units_dat.GroundWeapon[shooting_type];
				int air_weapon = game_st.units_dat.AirWeapon[shooting_type];
				if (ground_weapon != WeaponTypes::None) {
					game_st.weapons_dat.DamageAmount[ground_weapon] = weapon_damage[ground_weapon];
					game_st.weapons_dat.DamageBonus[ground_weapon] = weapon_bonus_damage[ground_weapon];
				}
				if (air_weapon != WeaponTypes::None) {
					game_st.weapons_dat.DamageAmount[air_weapon] = weapon_damage[air_weapon];
					game_st.weapons_dat.DamageBonus[air_weapon] = weapon_bonus_damage[air_weapon];
				}
			}
		};

		auto upgrades = [&](data_reader_le r, bool broodwar) {
			auto uses_default_settings = r.get_vec<uint8_t>(broodwar ? 62 : 46);
			auto mineral_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto mineral_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto gas_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto gas_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto build_time = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto build_time_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			for (int i = 0; i < (broodwar ? 61 : 46); ++i) {
				if (uses_default_settings[i]) continue;
				game_st.upgrades_dat.MineralCostBase[i] = mineral_cost[i];
				game_st.upgrades_dat.MineralCostFactor[i] = mineral_cost_factor[i];
				game_st.upgrades_dat.VespeneCostBase[i] = gas_cost[i];
				game_st.upgrades_dat.BespeneCostFactor[i] = gas_cost_factor[i];
				game_st.upgrades_dat.ResearchTimeBase[i] = build_time[i];
				game_st.upgrades_dat.ResearchTimeFactor[i] = build_time_factor[i];
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
				game_st.techdata_dat.mineralCost[i] = mineral_cost[i];
				game_st.techdata_dat.gasCost[i] = gas_cost[i];
				game_st.techdata_dat.researchTime[i] = build_time[i];
				game_st.techdata_dat.energyCost[i] = energy_cost[i];
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
				int unit_type = r.get<uint16_t>();
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

				if ((size_t)unit_type >= 228) xcept("invalid UNIT type %d",unit_type);
				if ((size_t)owner >= 12) xcept("invalid UNIT owner");

				log("create unit of type %d\n", unit_type);

				if (unit_type == UnitTypes::Special_Start_Location) {
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
					if (unit_type == UnitTypes::Resource_Mineral_Field) return true;
					if (unit_type == UnitTypes::Resource_Mineral_Field_Type_2) return true;
					if (unit_type == UnitTypes::Resource_Mineral_Field_Type_3) return true;
					if (unit_type == UnitTypes::Resource_Vespene_Geyser) return true;
					if (unit_type == UnitTypes::Critter_Rhynadon) return true;
					if (unit_type == UnitTypes::Critter_Bengalaas) return true;
					if (unit_type == UnitTypes::Critter_Scantid) return true;
					if (unit_type == UnitTypes::Critter_Kakaru) return true;
					if (unit_type == UnitTypes::Critter_Ragnasaur) return true;
					if (unit_type == UnitTypes::Critter_Ursadon) return true;
					return false;
				};
				if (!should_create_units_for_this_player()) continue;
				if (bStartingUnits && !is_neutral_unit()) continue;
				if (!bVictoryCondition && !bStartingUnits && !bTournamentModeEnabled) {
					// what is player_force?
					std::array<int, 12> player_force{};
					if (player_force[owner] && ~game_st.units_dat.StarEditGroupFlags[unit_type] & GroupFlags::Neutral) continue;
				}

				unit_t*u = create_initial_unit(unit_type, { x,y }, owner);

				xcept("created unit %p\n", u);

			}
		};

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

	}
};

void global_init(global_state&st) {

	auto load_iscript_bin = [&]() {

		using namespace iscript_opcodes;
		std::array<const char*, 69> ins_data;

		ins_data[opc_playfram] = "2";
		ins_data[opc_playframtile] = "2";
		ins_data[opc_sethorpos] = "1";
		ins_data[opc_setvertpos] = "1";
		ins_data[opc_setpos] = "11";
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

			std::function<size_t(int)> decode_at = [&](size_t initial_address) {
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
				a_vector<std::tuple<size_t, size_t>> branches;
				bool done = false;
				while (!done) {
					size_t pc = program_data.size();
					size_t cur_address = r.ptr - base_r.ptr;
					if (cur_address != initial_address) {
						auto in = decode_map.emplace(cur_address, pc);
						if (!in.second) {
							//log("0x%04x (0x%x): already decoded, inserting jmp\n", cur_address, pc);
							program_data.push_back(opc_goto + 1);
							program_data.push_back(in.first->second);
							break;
						}
					}
					size_t opcode = r.get<uint8_t>();
					if (opcode >= ins_data.size()) xcept("iscript load: at 0x%04x: invalid instruction %d", cur_address, opcode);
					//log("0x%04x (0x%x): opcode %d\n", cur_address, pc, opcode);
					program_data.push_back(opcode + 1);
					const char*c = ins_data[opcode];
					while (*c) {
						if (*c == '1') program_data.push_back(r.get<uint8_t>());
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
				for (auto&v : branches) {
					//log("doing branch to 0x%04x (fixup %x)\n", std::get<0>(v), std::get<1>(v));
					program_data[std::get<1>(v)] = decode_at(std::get<0>(v));
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

	load_iscript_bin();

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
}

}

