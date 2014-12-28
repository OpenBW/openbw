
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

template<typename utype>
struct xy_typed {
	utype x, y;
	typedef xy_typed<utype> xy;
	xy() : x(0), y(0) {}
	xy(utype x, utype y) : x(x), y(y) {}
	xy(Position pos) : x(pos.x), y(pos.y) {}
	bool operator<(const xy&n) const {
		if (y == n.y) return x < n.x;
		return y < n.y;
	}
	bool operator>(const xy&n) const {
		if (y == n.y) return x > n.x;
		return y > n.y;
	}
	bool operator<=(const xy&n) const {
		if (y == n.y) return x <= n.x;
		return y <= n.y;
	}
	bool operator>=(const xy&n) const {
		if (y == n.y) return x >= n.x;
		return y >= n.y;
	}
	bool operator==(const xy&n) const {
		return x == n.x && y == n.y;
	}
	bool operator!=(const xy&n) const {
		return x != n.x || y != n.y;
	}
	xy operator-(const xy&n) const {
		xy r(*this);
		return r -= n;
	}
	xy&operator-=(const xy&n) {
		x -= n.x;
		y -= n.y;
		return *this;
	}
	xy operator+(const xy&n) const {
		xy r(*this);
		return r += n;
	}
	xy&operator+=(const xy&n) {
		x += n.x;
		y += n.y;
		return *this;
	}
	double length() const {
		return sqrt(x*x + y*y);
	}
	xy operator -() const {
		return xy(-x, -y);
	}
	double angle() const {
		return atan2(y, x);
	}
	xy operator/(const xy&n) const {
		xy r(*this);
		return r /= n;
	}
	xy&operator/=(const xy&n) {
		x /= n.x;
		y /= n.y;
		return *this;
	}
	xy operator/(utype d) const {
		xy r(*this);
		return r /= d;
	}
	xy&operator/=(utype d) {
		x /= d;
		y /= d;
		return *this;
	}
	xy operator*(const xy&n) const {
		xy r(*this);
		return r *= n;
	}
	xy&operator*=(const xy&n) {
		x *= n.x;
		y *= n.y;
		return *this;
	}
	xy operator*(utype d) const {
		xy r(*this);
		return r *= d;
	}
	xy&operator*=(utype d) {
		x *= d;
		y *= d;
		return *this;
	}
};

typedef xy_typed<int> xy;


struct global_state {

	std::array<int, 228> unit_air_strength;
	std::array<int, 228> unit_ground_strength;
};

struct sight_values_t {
	struct maskdat_node_t;
	typedef a_vector<maskdat_node_t> maskdat_t;
	struct maskdat_node_t {
		// 
		//  I would like to change this structure a bit, move vision_propagation to a temporary inside reveal_sight_at,
		//  and change prev_count to a bool since it can only have two values, or remove it entirely.
		//
		// TODO: remove vision_propagation, since this struct is supposed to be static (stored in map_state)
		//
		maskdat_node_t*prev; // the tile from us directly towards the origin (diagonal is allowed and preferred)
		// if two neighbors have equal (diagonal) distance to the origin,
		// then this is the other tile directly towards the origin, otherwise it is prev
		maskdat_node_t*prev2;
		int map_index_offset;
		// temporary variable used when spreading vision to make sure we don't go through obstacles
		uint32_t vision_propagation;
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

struct map_state {
	size_t map_tile_width;
	size_t map_tile_height;

	a_string map_file_name;

	a_vector<a_string> map_strings;
	struct player_t {
		int controller;
		int race;
		int force;
	};
	a_string get_string(size_t index) {
		if (index == 0) return "<null string>";
		--index;
		if (index >= map_strings.size()) return "<invalid string index>";
		return map_strings[index];
	}

	std::array<player_t, 12> players;

	a_string scenario_name;
	a_string scenario_description;

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

	units_dat units_dat;
	weapons_dat weapons_dat;
	upgrades_dat upgrades_dat;
	techdata_dat techdata_dat;

	std::array<std::array<bool, 228>, 12> unit_type_allowed;
	std::array<std::array<int, 61>, 12> max_upgrade_levels;
	std::array<std::array<bool, 44>, 12> tech_available;

	std::array<xy, 12> start_locations;
};

struct game_state {
	global_state*g;
	map_state*map;
};

struct state_base {

	game_state*game;

	int update_tiles_countdown;
	CUnit*first_unit;
	CUnit*last_unit;
	CUnit*first_scanner_sweep_unit;
	CUnit*last_scanner_sweep_unit;

	CUnit*first_sight_related_unit;
	CUnit*last_sight_related_unit;

	std::array<int8_t, 12> selection_circle_color;

	int order_timer_counter;
	int secondary_order_timer_counter;

	std::array<std::array<int8_t, 61>, 12> upgrade_levels;
	std::array<std::array<bool, 44>, 12> tech_researched;

	// rearrange order 228 12 ?
	std::array<std::array<int32_t, 12>, 228> completed_unit_count;
	std::array<CUnit*, 12> first_player_unit;

	uint32_t local_mask;

	std::array<int, 12> shared_vision;

	a_vector<tile_t> tiles;
	a_vector<uint16_t> tiles_mega_tile_index;
};

struct state : state_base {

	a_vector<CUnit> units = a_vector<CUnit>(1700);

	state() = default;
	state(state&) = delete;
	state(state&&) = default;
	state&operator=(state&) = delete;
	state&operator=(state&&) = default;
};

void advance(state&st) {

	auto* const map = st.game->map;

	auto&units_dat = map->units_dat;
	auto&weapons_dat = map->weapons_dat;

	auto Completed = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::Completed);
	};
	auto InBuilding = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::InBuilding);
	};
	auto Unmovable = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::Unmovable);
	};
	auto Burrowed = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::Burrowed);
	};
	auto IsABuilding = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::IsABuilding);
	};
	auto IsAUnit = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::IsAUnit);
	};
	auto GroundedBuilding = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::GroundedBuilding);
	};
	auto IsHallucination = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::IsHallucination);
	};
	auto InAir = [&](CUnit*u) {
		return !!(u->statusFlags&StatusFlags::InAir);
	};

	auto SA_Subunit = [&](CUnit*u) {
		return !!(units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Subunit);
	};
	auto SA_Worker = [&](CUnit*u) {
		return !!(units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Worker);
	};

	auto SPR_Hidden = [&](CSprite*sprite) {
		return !!(sprite->flags&SpriteFlags::Hidden);
	};

	auto get_unit = [&](unit_id id) {
		size_t idx = id.index();
		if (!idx) return (CUnit*)nullptr;
		CUnit*u = &st.units[((size_t)idx & 0x7ff) - 1];
		if (u->targetOrderSpecial != id.generation()) return (CUnit*)nullptr;
		return u;
	};

	if (st.update_tiles_countdown == 0) st.update_tiles_countdown = 100;
	--st.update_tiles_countdown;
	const bool update_tiles = st.update_tiles_countdown == 0;

	auto setAllImageGroupFlagsPal11 = [&](CSprite*sprite) {
		for (CImage*img = sprite->underlay; img; img = img->next) {
			if (img->paletteType == 0xb) img->flags |= 1;
		}
	};

	auto visible_hp_plus_shields = [&](CUnit*u) {
		int r = 0;
		if (units_dat.ShieldEnable[u->unitType]) r += u->shieldPoints >> 8;
		r += (u->hitPoints + 0xff) >> 8;
		return r;
	};
	auto max_visible_hp = [&](CUnit*u) {
		int hp = units_dat.HitPoints[u->unitType] >> 8;
		if (hp == 0) hp = (u->hitPoints + 0xff) >> 8;
		if (hp == 0) hp = 1;
		return hp;
	};
	auto max_visible_hp_plus_shields = [&](CUnit*u) {
		int shields = 0;
		if (units_dat.ShieldEnable[u->unitType]) shields += units_dat.ShieldAmount[u->unitType];
		return max_visible_hp(u) + shields;
	};

	auto getUnitStrength = [&](CUnit*u, bool ground) {
		if (u->unitType == UnitTypes::Zerg_Larva || u->unitType == UnitTypes::Zerg_Egg || u->unitType == UnitTypes::Zerg_Cocoon || u->unitType == UnitTypes::Zerg_Lurker_Egg) return 0;
		int vis_hp_shields = visible_hp_plus_shields(u);
		int max_vis_hp_shields = max_visible_hp_plus_shields(u);
		if (u->statusFlags&StatusFlags::IsHallucination) {
			if (vis_hp_shields < max_vis_hp_shields) return 0;
		}

		int r = ground ? st.game->g->unit_ground_strength[u->unitType] : st.game->g->unit_air_strength[u->unitType];
		if (u->unitType == UnitTypes::Terran_Bunker) {
			xcept("fixme getUnitStrength bunker; see getUnitStrength_AirOrGround");
		}
		if (units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Spellcaster) {
			if (~u->statusFlags&StatusFlags::IsHallucination) r += (u->energy >> 8) / 2;
		}
		return r * vis_hp_shields / max_vis_hp_shields;
	};

	auto setUnitHP = [&](CUnit*u, int hitpoints) {
		u->hitPoints = hitpoints;
		if (u->hitPoints > units_dat.HitPoints[u->unitType]) u->hitPoints = units_dat.HitPoints[u->unitType];
		if (u->sprite->flags & SpriteFlags::Selected && u->sprite->visibilityFlags&st.local_mask) {
			setAllImageGroupFlagsPal11(u->sprite);
		}
		if (u->statusFlags & StatusFlags::Completed) {
			// damage overlay stuff

			u->airStrength = getUnitStrength(u, false);
			u->groundStrength = getUnitStrength(u, true);
		}
	};

	auto is_frozen_or_in_air = [&](CUnit*u) {
		if (u->statusFlags&StatusFlags::InAir) return true;
		if (u->status.lockdownTimer) return true;
		if (u->status.stasisTimer) return true;
		if (u->status.maelstromTimer) return true;
		return false;
	};

	auto set_buttonset = [&](CUnit*u, int type) {
		if (type != UnitTypes::None && ~units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Building) {
			if (is_frozen_or_in_air(u)) return;
		}
		u->currentButtonSet = type;
	};

	auto find_overlay = [&](CSprite*sprite, int first, int last) {
		for (CImage*i = sprite->overlay; i; i = i->next) {
			if (i->imageID >= first && i->imageID <= last) return i;
		}
		return (CImage*)nullptr;
	};

	auto playImageIscript = [&](CImage*img, int id) {
		// todo
		xcept("playImageIscript");
	};

	// RemoveOverlaysBetween
	auto freeze_effect_end = [&](CUnit*u, int first, int last) {
		bool still_frozen = is_frozen_or_in_air(u);
		if (!still_frozen) {
			u->statusFlags &= ~StatusFlags::DoodadStatesThing;
			xcept("freeze_effect_end: orderComputer_cl");
			// orderComputer_cl(u->subUnit, units_dat.ReturntoIdle[u->subUnit->unitType]);
		}
		CImage*overlay = find_overlay(u->sprite, first, last);
		if (!overlay && u->subUnit) overlay = find_overlay(u->subUnit->sprite, first, last);
		if (overlay) playImageIscript(overlay, idenums::ISCRIPT_Anim_Death);
		if (units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Worker && !still_frozen) {
			// sub_468DB0
			CUnit*target = u->worker.harvestTarget;
			if (target && units_dat.SpecialAbilityFlags[target->unitType] & UnitPrototypeFlags::FlyingBuilding) {
				if (u->worker.isCarryingSomething) {
					if (target->building.resource.gatherQueueCount) {
						//if (u->orderID )
						xcept("weird logic, fix me when this throws");
					}
				}
			}
		}
		u->orderQueueTimer = 15;
	};
	
	auto remove_stasis = [&](CUnit*u) {
		u->status.stasisTimer = 0;
		set_buttonset(u, u->unitType);
		if (~units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Invincible) {
			u->statusFlags &= ~StatusFlags::Invincible;
		}
		freeze_effect_end(u, idenums::IMAGEID_Stasis_Field_Small, idenums::IMAGEID_Stasis_Field_Large);
	};

	auto updateUnitStatusTimers = [&](CUnit*u) {
		if (u->status.stasisTimer) {
			--u->status.stasisTimer;
			if (!u->status.stasisTimer) {
				remove_stasis(u);
			}
		}
		if (u->status.stimTimer) {
			--u->status.stimTimer;
			if (!u->status.stimTimer) {
				xcept("remove stim");
			}
		}
		if (u->status.ensnareTimer) {
			--u->status.ensnareTimer;
			if (!u->status.ensnareTimer) {
				xcept("remove ensnare");
			}
		}
		if (u->status.defenseMatrixTimer) {
			--u->status.defenseMatrixTimer;
			if (!u->status.defenseMatrixTimer) {
				xcept("remove defense matrix");
			}
		}
		if (u->status.irradiateTimer) {
			--u->status.irradiateTimer;
			xcept("irradiate damage");
			if (!u->status.irradiateTimer) {
				xcept("remove irradiate");
			}
		}
		if (u->status.lockdownTimer) {
			--u->status.lockdownTimer;
			if (!u->status.lockdownTimer) {
				xcept("remove lockdown");
			}
		}
		if (u->status.maelstromTimer) {
			--u->status.maelstromTimer;
			if (!u->status.maelstromTimer) {
				xcept("remove maelstrom");
			}
		}
		if (u->status.plagueTimer) {
			xcept("plague stuff");
		}
		if (u->status.stormTimer) --u->status.stormTimer;
		int prev_acidSporeCount = u->status.acidSporeCount;
		for (auto&v : u->status.acidSporeTime) {
			if (!v) continue;
			--v;
			if (!v) --u->status.acidSporeCount;
		}
		if (u->status.acidSporeCount) {
			xcept("acid spore stuff");
		} else if (prev_acidSporeCount) {
			xcept("RemoveOverlays(u, IMAGEID_Acid_Spores_1_Overlay_Small, IMAGEID_Acid_Spores_6_9_Overlay_Large);");
		}

	};

	auto create_selection_circle = [&](CSprite*sprite, int color, int imageid) {
		return false;
	};

	auto removeSelectionCircle = [&](CSprite*sprite) {

	};

	auto update_selection_sprite = [&](CSprite*sprite, int color) {
		if (!sprite->selectionTimer) return;
		--sprite->selectionTimer;
		if (~sprite->visibilityFlags&st.local_mask) sprite->selectionTimer = 0;
		if (sprite->selectionTimer & 8 || (sprite->selectionTimer == 0 && sprite->flags&SpriteFlags::Selected)) {
			if (~sprite->flags&SpriteFlags::DrawSelection) {
				if (create_selection_circle(sprite, color, idenums::IMAGEID_Selection_Circle_22pixels)) {
					sprite->flags |= SpriteFlags::DrawSelection;
				}
			}
		} else removeSelectionCircle(sprite);
	};

	auto updateEnergyTimer = [&](CUnit*u) {
		if (~units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Spellcaster) return;
		xcept("updateEnergyTimer");
	};

	auto unit_hp_below_33_percent = [&](CUnit*u) {
		int max_hp = max_visible_hp(u);
		int hp = (u->hitPoints + 0xff) >> 8;
		return hp * 100 / max_hp <= 33;
	};

	auto updateUnitTimers = [&](CUnit*u) {
		if (u->mainOrderTimer) --u->mainOrderTimer;
		if (u->groundWeaponCooldown) --u->groundWeaponCooldown;
		if (u->airWeaponCooldown) --u->airWeaponCooldown;
		if (u->spellCooldown) --u->spellCooldown;
		if (units_dat.ShieldEnable[u->unitType]) {
			int max_shields = units_dat.ShieldAmount[u->unitType] << 8;
			if (u->shieldPoints != max_shields) {
				u->shieldPoints += 7;
				if (u->shieldPoints > max_shields) u->shieldPoints = max_shields;
				if (u->sprite->flags & SpriteFlags::Selected) {
					setAllImageGroupFlagsPal11(u->sprite);
				}
			}
		}
		if (u->unitType == UnitTypes::Zerg_Zergling || u->unitType == UnitTypes::Hero_Devouring_One) {
			if (u->groundWeaponCooldown == 0) u->orderQueueTimer = 0;
		}
		u->isBeingHealed = false;
		if (u->statusFlags & StatusFlags::Completed || ~u->sprite->flags & SpriteFlags::Hidden) {
			++u->status.cycleCounter;
			if (u->status.cycleCounter >= 8) {
				u->status.cycleCounter = 0;
				updateUnitStatusTimers(u);
			}
		}
		if (u->statusFlags & StatusFlags::Completed) {
			if (units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::RegeneratesHP) {
				if (u->hitPoints > 0 && units_dat.HitPoints[u->unitType]) {
					setUnitHP(u, u->hitPoints + 4);
				}
			}
			updateEnergyTimer(u);
			if (u->recentOrderTimer) --u->recentOrderTimer;
			bool remove_timer_hit_zero = false;
			if (u->status.removeTimer) {
				--u->status.removeTimer;
				if (!u->status.removeTimer) {
					xcept("orders_SelfDestructing...");
					return;
				}
			}
			int gf = units_dat.StarEditGroupFlags[u->unitType];
			if (gf&GroupFlags::Terran && ~gf&(GroupFlags::Zerg | GroupFlags::Protoss)) {
				if (u->statusFlags&StatusFlags::GroundedBuilding || units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::FlyingBuilding) {
					if (unit_hp_below_33_percent(u)) {
						xcept("killTargetUnitCheck(...)");
					}
				}
			}
		}
	};

	auto UpdateUnitOrderData = [&](CUnit*u) {
		if (~units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Subunit && ~u->sprite->flags&SpriteFlags::Hidden) {
			update_selection_sprite(u->sprite, st.selection_circle_color[u->playerID]);
		}

		updateUnitTimers(u);
		
	};

	auto unit_movepos_state = [&](CUnit*u) {
		if (u->sprite->position.x != u->moveTarget.pt.x || u->sprite->position.y != u->moveTarget.pt.y) return 0;
		return Unmovable(u) ? 2 : 1;
	};

	auto unit_order_dead = [&](CUnit*u) {
		return u->orderID == Orders::Die && u->orderState == 1;
	};

	auto Unit_ExecPathingState = [&](CUnit*u) {

		bool refresh_vision = update_tiles;

		auto UMInitialize = [&](CUnit*u) {
			u->pathingFlags &= ~(1 | 2);
			if (u->sprite->elevationLevel) u->pathingFlags |= 1;
			u->contourBounds = { 0,0,0,0 };
			int next_state = UM_Lump;
			if (!SA_Subunit(u) && InBuilding(u)) {
				next_state = UM_InitSeq;
			} else if (!u->sprite || unit_order_dead(u)) {
				next_state = UM_Lump;
			} else if (InBuilding(u)) {
				next_state = UM_Bunker;
			} else if (SPR_Hidden(u->sprite)) {
				if (u->movementFlags & MovementFlags::Accelerating || unit_movepos_state(u) == 0) {
					// SetMoveTarget_xy(u)
					// ...
					xcept("todo hidden sprite pathing stuff");
				}
				next_state = UM_Hidden;
			} else if (Burrowed(u)) {
				next_state = UM_Lump;
			}
			// Doesn't this seems backwards? It sure does.
			else if (IsABuilding(u)) next_state = u->pathingFlags & 1 ? UM_AtRest : UM_Flyer;
			else if (IsAUnit(u)) next_state = SA_Subunit(u) ? UM_BldgTurret : UM_Turret;
			else if (u->pathingFlags & 1 && (u->movementFlags & MovementFlags::Accelerating || unit_movepos_state(u) == 0)) next_state = UM_LumpWannabe;
			u->movementState = next_state;
		};

		switch (u->movementState) {
		case UM_Init:
			UMInitialize(u);
			break;
		default:
			xcept("fixme: movement state %d\n", u->movementState);
		}

		return refresh_vision;
	};

	auto is_transforming_zerg_building = [&](CUnit*u) {
		if (Completed(u)) return false;
		int tt = u->buildQueue[u->buildQueueSlot];
		return tt == UnitTypes::Zerg_Hive || tt == UnitTypes::Zerg_Lair || tt == UnitTypes::Zerg_Greater_Spire || tt == UnitTypes::Zerg_Spore_Colony || tt == UnitTypes::Zerg_Sunken_Colony;
	};


	auto unit_sight_range2 = [&](CUnit*u, bool ignore_blindness) {
		if (GroundedBuilding(u) && !Completed(u) && !is_transforming_zerg_building(u)) return 4;
		if (!ignore_blindness && u->status.isBlind) return 2;
		if (u->unitType == UnitTypes::Terran_Ghost && st.upgrade_levels[u->playerID][UpgradeTypes::Ocular_Implants]) return 11;
		if (u->unitType == UnitTypes::Zerg_Overlord && st.upgrade_levels[u->playerID][UpgradeTypes::Antennae]) return 11;
		if (u->unitType == UnitTypes::Protoss_Observer && st.upgrade_levels[u->playerID][UpgradeTypes::Sensor_Array]) return 11;
		if (u->unitType == UnitTypes::Protoss_Scout && st.upgrade_levels[u->playerID][UpgradeTypes::Apial_Sensors]) return 11;
		return (int)units_dat.SightRange[u->unitType];
	};
	auto unit_sight_range = [&](CUnit*u) {
		return unit_sight_range2(u, false);
	};
	auto unit_sight_range_ignore_blindness = [&](CUnit*u) {
		return unit_sight_range2(u, true);
	};

	auto visible_to_everyone = [&](CUnit*u) {
		if (SA_Worker(u)) {
			if (u->worker.pPowerup && u->worker.pPowerup->unitType == UnitTypes::Powerup_Flag) return true;
			else return false;
		}
		if (IsHallucination(u) || (u->unitType == UnitTypes::Zerg_Overlord && !st.upgrade_levels[u->playerID][UpgradeTypes::Ventral_Sacs]) || !units_dat.SpaceProvided[u->unitType]) return false;
		for (auto idx : u->loadedUnitIndex) {
			CUnit*lu = get_unit(idx);
			if (!lu || !lu->sprite) continue;
			if (unit_order_dead(lu)) continue;
			if (!SA_Worker(lu)) continue;
			if (lu->worker.pPowerup && lu->worker.pPowerup->unitType == UnitTypes::Powerup_Flag) return true;
		}
		return false;
	};

	auto tile_index = [&](xy pos) {
		size_t ux = (size_t)pos.x / 32;
		size_t uy = (size_t)pos.y / 32;
		if (ux >= map->map_tile_width || uy >= map->map_tile_height) xcept("attempt to get tile index for invalid position %d %d", pos.x, pos.y);
		return uy * map->map_tile_width + ux;
	};

	auto get_ground_height_at = [&](xy pos) {
		size_t index = tile_index(pos);
		tile_id creep_tile = map->gfx_creep_tiles.at(index);
		tile_id tile_id = creep_tile ? creep_tile : map->gfx_tiles.at(index);
		size_t megatile_index = map->cv5.at(tile_id.group_index()).megaTileRef[tile_id.subtile_index()];
		size_t ux = pos.x;
		size_t uy = pos.y;
		int flags = map->vf4.at(megatile_index).flags[uy / 8 % 4 * 4 + ux / 8 % 4];
		if (flags&MiniTileFlags::High) return 2;
		if (flags&MiniTileFlags::Middle) return 1;
		return 0;
	};

	auto reveal_sight_at = [&](xy pos, int range, int reveal_to, bool in_air) {
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
		reveal_tile_mask.explored= visibility_mask;
		reveal_tile_mask.flags = 0;
		tile_t required_tile_mask = reveal_tile_mask;
		required_tile_mask.flags = height_mask;
		auto&sight_vals = map->sight_values.at(range);
		size_t tile_x = (size_t)pos.x / 32;
		size_t tile_y = (size_t)pos.y / 32;
		tile_t*base_tile = &st.tiles[tile_x + tile_y*map->map_tile_width];
		if (!in_air) {
			auto*cur = sight_vals.maskdat.data();
			auto*end = cur + sight_vals.min_mask_size;
			for (; cur != end; ++cur) {
				if (tile_x + cur->x >= map->map_tile_width) continue;
				if (tile_y + cur->y >= map->map_tile_height) continue;
				auto&tile = base_tile[cur->map_index_offset];
				tile.raw &= reveal_tile_mask.raw;
				cur->vision_propagation = tile.raw;
			}
			end += sight_vals.ext_masked_count;
			for (; cur != end; ++cur) {
				cur->vision_propagation = 0xff;
				if (tile_x + cur->x >= map->map_tile_width) continue;
				if (tile_y + cur->y >= map->map_tile_height) continue;
				bool okay = false;
				for (int i2 = 0; i2 < cur->prev_count; ++i2) {
					if (!((i2==0 ? cur->prev : cur->prev2)->vision_propagation & required_tile_mask.raw)) {
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
				if (tile_x + cur->x >= map->map_tile_width) continue;
				if (tile_y + cur->y >= map->map_tile_height) continue;
				base_tile[cur->map_index_offset].raw &= reveal_tile_mask.raw;
			}
		}
	};

	auto refreshUnitVision = [&](CUnit*u) {
		if (u->playerID >= 8 && !u->status.parasiteFlags) return;
		if (u->unitType == UnitTypes::Terran_Nuclear_Missile) return;
		int visible_to = 0;
		if (visible_to_everyone(u) || (u->unitType == UnitTypes::Powerup_Flag && u->orderID == Orders::UnusedPowerup)) visible_to = 0xff;
		else {
			visible_to = st.shared_vision[u->playerID] | u->status.parasiteFlags;
			if (u->status.parasiteFlags) {
				visible_to |= u->status.parasiteFlags;
				for (size_t i = 0; i < 12; ++i) {
					if (~u->status.parasiteFlags&(1 << i)) continue;
					visible_to |= st.shared_vision[i];
				}
			}
			reveal_sight_at(u->sprite->position, unit_sight_range(u), visible_to, InAir(u));
		}
	};

	auto update_unit_pathing = [&](CUnit*u) {

		bool refresh_vision = Unit_ExecPathingState(u);
		if (refresh_vision) refreshUnitVision(u);
		if (u->statusFlags&StatusFlags::Completed) {
			if (u->subUnit && ~units_dat.SpecialAbilityFlags[u->unitType] & UnitPrototypeFlags::Subunit) {
				xcept("update_unit_pathing: subunit stuff");
			}
		}
	};

	auto UpdateUnitSpriteInfo = [&](CUnit*u) {
		
	};

	auto UpdateUnits = [&]() {

		// place box/target order cursor/whatever

		--st.order_timer_counter;
		if (!st.order_timer_counter) {
			st.order_timer_counter = 150;
			int v = 0;
			for (CUnit*u = st.first_unit; u; u = u->next) {
				u->orderQueueTimer = v;
				++v;
				if (v == 8) v = 0;
			}
		}
		--st.secondary_order_timer_counter;
		if (!st.secondary_order_timer_counter) {
			st.secondary_order_timer_counter = 300;
			int v = 0;
			for (CUnit*u = st.first_unit; u; u = u->next) {
				u->secondaryOrderTimer = v;
				++v;
				if (v == 30) v = 0;
			}
		}

		// some_units_loaded_and_disruption_web begin
		for (CUnit*u = st.first_unit; u; u = u->next) {
			if (~u->statusFlags&StatusFlags::InAir || u->statusFlags&StatusFlags::UNKNOWN1) {
				u->statusFlags &= ~StatusFlags::CanNotAttack;
				if (~u->statusFlags&StatusFlags::IsHallucination && (u->unitType != UnitTypes::Zerg_Overlord || st.upgrade_levels[u->playerID][UpgradeTypes::Ventral_Sacs]) && units_dat.SpaceProvided[u->unitType]) {
					xcept("sub_4EB2F0 loaded unit stuff");
				} else if (u->subUnit) {
					u->subUnit->statusFlags &= ~StatusFlags::CanNotAttack;
				}
			}
		}
		if (st.completed_unit_count[UnitTypes::Spell_Disruption_Web][11]) {
			xcept("disruption web stuff");
		}
		// some_units_loaded_and_disruption_web end

		for (CUnit*u = st.first_sight_related_unit; u; u = u->next) {
			xcept("fixme first_sight_related_unit stuff in UpdateUnits");
		}

		for (CUnit*u = st.first_unit; u; u = u->next) {
			update_unit_pathing(u);
		}
		
		if (update_tiles) {
			for (CUnit*u = st.first_scanner_sweep_unit; u; u = u->next) {
				refreshUnitVision(u);
			}
		}

		for (CUnit*u = st.first_unit; u; u = u->next) {
			UpdateUnitSpriteInfo(u);
		}

		for (CUnit*u = st.first_unit; u; u = u->next) {
			UpdateUnitOrderData(u);
		}
	};


	UpdateUnits();

}



void calculate_unit_strengths(global_state&g, units_dat&units_dat, weapons_dat&weapons_dat) {

	auto get_unit_strength = [&](int unit_type, int weapon_type) {
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

	for (size_t idx = 0; idx < 228; ++idx) {

		int type = idx;
		int air_strength = 0;
		int ground_strength = 0;
		if (type != UnitTypes::Zerg_Larva && type != UnitTypes::Zerg_Egg && type != UnitTypes::Zerg_Cocoon && type != UnitTypes::Zerg_Lurker_Egg) {
			if (type==UnitTypes::Protoss_Carrier || type==UnitTypes::Hero_Gantrithor) type = UnitTypes::Protoss_Interceptor;
			else if (type == UnitTypes::Protoss_Reaver || type == UnitTypes::Hero_Warbringer) type = UnitTypes::Protoss_Scarab;
			else if (units_dat.Subunit1[type] != UnitTypes::None) type = units_dat.Subunit1[type];

			int air_weapon = units_dat.AirWeapon[type];
			if (air_weapon == WeaponTypes::None) air_strength = 1;
			else air_strength = get_unit_strength(idx, air_weapon);

			int ground_weapon = units_dat.GroundWeapon[type];
			if (ground_weapon == WeaponTypes::None) ground_strength = 1;
			else ground_strength = get_unit_strength(idx, ground_weapon);
		}
		if (air_strength == 1 && ground_strength > air_strength) air_strength = 0;
		if (ground_strength == 1 && air_strength > ground_strength) ground_strength = 0;

		log("strengths for %d is %d %d\n", idx, air_strength, ground_strength);

		g.unit_air_strength[idx] = air_strength;
		g.unit_ground_strength[idx] = ground_strength;

	}

}

void generate_sight_values(map_state&ms) {
	for (size_t i = 0; i < ms.sight_values.size(); ++i) {
		auto&v = ms.sight_values[i];
		v.max_width = 3 + i * 2;
		v.max_height = 3 + i * 2;
		v.min_width = 3;
		v.min_height = 3;
		v.min_mask_size = 0;
		v.ext_masked_count = 0;
	}

	for (auto&v : ms.sight_values) {
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
								else if (prev_x< 0) ++prev_x;
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
								this_entry->map_index_offset = this_y * ms.map_tile_width + this_x;
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
// 	for (auto&v : ms.sight_values) {
// 		log("%d %d %d %d %d %d\n", v.max_width, v.max_height, v.min_width, v.max_height, v.min_mask_size, v.ext_masked_count);
// 		size_t maskdat_size = v.min_mask_size + v.ext_masked_count;
// 		log(" maskdat: \n");
// 		//log("%s", hexdump(v.maskdat.data(), maskdat_size * 20));
// 		for (auto&v : v.maskdat) {
// 			log("%d %d %d\n", v.map_index_offset, v.x, v.y);
// 		}
// 	}

}

void init_map_state(map_state&ms);

void load_tile_stuff(map_state&map_st) {
	std::array<const char*, 8> tileset_names = {
		"badlands", "platform", "install", "AshWorld", "Jungle", "Desert", "Ice", "Twilight"
	};

	auto set_mega_tile_flags = [&]() {
		map_st.mega_tile_flags.resize(map_st.vf4.size());
		for (size_t i = 0; i < map_st.mega_tile_flags.size(); ++i) {
			int flags = 0;
			auto&mt = map_st.vf4[i];
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
			map_st.mega_tile_flags[i] = flags;
		}

	};

	load_data_file(map_st.vf4, format("Tileset\\%s.vf4", tileset_names.at(map_st.tileset_index)));
	set_mega_tile_flags();
	load_data_file(map_st.cv5, format("Tileset\\%s.cv5", tileset_names.at(map_st.tileset_index)));
}

struct data_reader {
	uint8_t*ptr = nullptr;
	uint8_t*end = nullptr;
	data_reader() = default;
	data_reader(uint8_t*ptr, uint8_t*end) : ptr(ptr), end(end) {}
	template<typename T>
	T get() {
		if (ptr + sizeof(T) - end > 0) xcept("data_reader: attempt to read past end-of-file");
		ptr += sizeof(T);
		return *(T*)(ptr - sizeof(T));
	}
	uint8_t*get_n(size_t n) {
		uint8_t*r = ptr;
		if (ptr + n - end > 0) xcept("data_reader: attempt to read past end-of-file");
		ptr += n;
		return r;
	}
	template<typename T>
	a_vector<T> get_vec(size_t n) {
		uint8_t*data = get_n(n*sizeof(T));
		a_vector<T> r(n);
		memcpy(r.data(), data, n*sizeof(T));
		return r;
	}
	void skip(int n) {
		if (ptr + n - end > 0) xcept("data_reader: attempt to seek past end-of-file");
		ptr += n;
	}
	size_t left() {
		return end - ptr;
	}
};

void load_map_file(state&st, a_string filename) {

	map_state&map_st = *st.game->map;

	// campaign stuff? see load_map_file

	log("load map file '%s'\n", filename);

	SArchive archive(filename);
	a_vector<uint8_t> data;
	load_data_file(data, "staredit\\scenario.chk");

	a_unordered_map<uint32_t, std::function<void(data_reader)>> tag_funcs;
	
	using tag_list_t = a_vector<std::pair<uint32_t, bool>>;
	auto read_chunks = [&](const tag_list_t&tags) {
		data_reader r(data.data(), data.data() + data.size());
		a_unordered_map<uint32_t, data_reader> chunks;
		while (r.left()) {
			uint32_t tag = r.get<uint32_t>();
			uint32_t len = r.get<uint32_t>();
			//log("tag '%.4s' len %d\n", (char*)&tag, len);
			uint8_t*chunk_data = r.ptr;
			r.skip(len);
			chunks[tag] = { chunk_data, r.ptr };
		}
		for (auto&v : tags) {
			uint32_t rev_tag = std::get<0>(v);
			uint32_t tag = rev_tag >> 24 | (rev_tag >> 8) & 0xff00 | (rev_tag << 8) & 0xff0000 | rev_tag << 24;
			auto i = chunks.find(tag);
			if (i == chunks.end()) {
				if (std::get<1>(v)) xcept("map is missing required chunk '%.4s'", (char*)&tag);
			} else {
				if (!tag_funcs[rev_tag]) xcept("tag '%.4s' is missing a function", (char*)&tag);
				tag_funcs[rev_tag](i->second);
			}
		}
	};

	int version = 0;
	tag_funcs['VER '] = [&](data_reader r) {
		version = r.get<uint16_t>();
		log("VER: version is %d\n", version);
	};
	tag_funcs['DIM '] = [&](data_reader r) {
		map_st.map_tile_width = r.get<uint16_t>();
		map_st.map_tile_height = r.get<uint16_t>();
		log("DIM: dimensions are %d %d\n", map_st.map_tile_width, map_st.map_tile_height);
	};
	tag_funcs['ERA '] = [&](data_reader r) {
		map_st.tileset_index = r.get<uint16_t>() % 8;
		log("ERA: tileset is %d\n", map_st.tileset_index);
	};
	tag_funcs['OWNR'] = [&](data_reader r) {
		for (size_t i = 0; i < 12; ++i) {
			map_st.players[i].controller = r.get<int8_t>();
		}
	};
	tag_funcs['SIDE'] = [&](data_reader r) {
		for (size_t i = 0; i < 12; ++i) {
			map_st.players[i].race = r.get<int8_t>();
		}
	};
	tag_funcs['STR '] = [&](data_reader r) {
		auto start = r;
		size_t num = r.get<uint16_t>();
		map_st.map_strings.clear();
		map_st.map_strings.resize(num);
		for (size_t i = 0; i < num; ++i) {
			size_t offset = r.get<uint16_t>();
			auto t = start;
			t.skip(offset);
			char*b = (char*)t.ptr;
			while (t.get<char>());
			map_st.map_strings[i] = a_string(b, (char*)t.ptr - b - 1);
			//log("string %d: %s\n", i, map_st.map_strings[i]);
		}
	};
	tag_funcs['SPRP'] = [&](data_reader r) {
		map_st.scenario_name = map_st.get_string(r.get<uint16_t>());
		map_st.scenario_description = map_st.get_string(r.get<uint16_t>());
		log("SPRP: scenario name: '%s',  description: '%s'\n", map_st.scenario_name, map_st.scenario_description);
	};
	tag_funcs['FORC'] = [&](data_reader r) {
		for (size_t i = 0; i < 12; ++i) map_st.players[i].force = 0;
		for (size_t i = 0; i < 4; ++i) {
			map_st.forces[i].name = "";
			map_st.forces[i].flags = 0;
		}
		if (r.left()) {
			for (size_t i = 0; i < 8; ++i) {
				map_st.players[i].force = r.get<uint8_t>();
			}
			for (size_t i = 0; i < 4; ++i) {
				map_st.forces[i].name = map_st.get_string(r.get<uint16_t>());
			}
			for (size_t i = 0; i < 4; ++i) {
				map_st.forces[i].flags = r.get<uint8_t>();
			}
		}
	};
	tag_funcs['VCOD'] = [&](data_reader r) {
		// Starcraft does some verification/checksum stuff here
	};


	tag_funcs['MTXM'] = [&](data_reader r) {
		map_st.gfx_tiles = r.get_vec<tile_id>(map_st.map_tile_width * map_st.map_tile_height);
		st.tiles.resize(map_st.gfx_tiles.size());
		st.tiles_mega_tile_index.resize(map_st.gfx_tiles.size());
		for (size_t i = 0; i < map_st.gfx_tiles.size(); ++i) {
			tile_id tile_id = map_st.gfx_tiles[i];
			size_t megatile_index = map_st.cv5.at(tile_id.group_index()).megaTileRef[tile_id.subtile_index()];
			st.tiles_mega_tile_index[i] = (uint16_t)megatile_index;
			st.tiles[i].flags |= map_st.mega_tile_flags.at(megatile_index);
			if (tile_id.has_creep()) {
				st.tiles_mega_tile_index[i] |= 0x8000;
				st.tiles[i].flags |= tile_t::flag_has_creep;
			}
		}
	};

	bool bVictoryCondition = false;
	bool bStartingUnits = false;
	bool bTournamentModeEnabled = false;

	tag_funcs['THG2'] = [&](data_reader r) {
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
	tag_funcs['MASK'] = [&](data_reader r) {
		auto mask = r.get_vec<uint8_t>(map_st.map_tile_width*map_st.map_tile_height);
		for (size_t i = 0; i < mask.size(); ++i) {
			st.tiles[i].visible |= mask[i];
			st.tiles[i].explored |= mask[i];
		}
	};

	auto units = [&](data_reader r, bool broodwar) {
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
		for (size_t i = 0; i < 228; ++i) {
			if (uses_default_settings[i]) continue;
			map_st.units_dat.HitPoints[i] = hp[i];
			map_st.units_dat.ShieldAmount[i] = shield[i];
			map_st.units_dat.Armor[i] = armor[i];
			map_st.units_dat.BuildTime[i] = build_time[i];
			map_st.units_dat.MineralCost[i] = mineral_cost[i];
			map_st.units_dat.VespeneCost[i] = gas_cost[i];
			map_st.units_dat.UnitMapString[i] = string_index[i];
			int type = i;
			if (map_st.units_dat.Subunit1[i]) type = map_st.units_dat.Subunit1[i];
			int ground_weapon = map_st.units_dat.GroundWeapon[type];
			int air_weapon = map_st.units_dat.AirWeapon[type];
			if (ground_weapon != WeaponTypes::None) {
				map_st.weapons_dat.DamageAmount[ground_weapon] = weapon_damage[ground_weapon];
				map_st.weapons_dat.DamageBonus[ground_weapon] = weapon_bonus_damage[ground_weapon];
			}
			if (air_weapon != WeaponTypes::None) {
				map_st.weapons_dat.DamageAmount[air_weapon] = weapon_damage[air_weapon];
				map_st.weapons_dat.DamageBonus[air_weapon] = weapon_bonus_damage[air_weapon];
			}
		}
	};

	auto upgrades = [&](data_reader r, bool broodwar) {
		auto uses_default_settings = r.get_vec<uint8_t>(broodwar ? 62 : 46);
		auto mineral_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
		auto mineral_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
		auto gas_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
		auto gas_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
		auto build_time = r.get_vec<uint16_t>(broodwar ? 61 : 46);
		auto build_time_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
		for (int i = 0; i < (broodwar ? 61 : 46); ++i) {
			if (uses_default_settings[i]) continue;
			map_st.upgrades_dat.MineralCostBase[i] = mineral_cost[i];
			map_st.upgrades_dat.MineralCostFactor[i] = mineral_cost_factor[i];
			map_st.upgrades_dat.VespeneCostBase[i] = gas_cost[i];
			map_st.upgrades_dat.BespeneCostFactor[i] = gas_cost_factor[i];
			map_st.upgrades_dat.ResearchTimeBase[i] = build_time[i];
			map_st.upgrades_dat.ResearchTimeFactor[i] = build_time_factor[i];
		}
	};

	auto techdata = [&](data_reader r, bool broodwar) {
		auto uses_default_settings = r.get_vec<uint8_t>(broodwar ? 44 : 24);
		auto mineral_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
		auto gas_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
		auto build_time = r.get_vec<uint16_t>(broodwar ? 44 : 24);
		auto energy_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
		for (int i = 0; i < (broodwar ? 44 : 24); ++i) {
			if (uses_default_settings[i]) continue;
			map_st.techdata_dat.mineralCost[i] = mineral_cost[i];
			map_st.techdata_dat.gasCost[i] = gas_cost[i];
			map_st.techdata_dat.researchTime[i] = build_time[i];
			map_st.techdata_dat.energyCost[i] = energy_cost[i];
		}
	};

	auto upgrade_restrictions = [&](data_reader r, bool broodwar) {
		int count = broodwar ? 61 : 46;
		auto player_max_level = r.get_vec<uint8_t>(12 * count);
		auto player_cur_level = r.get_vec<uint8_t>(12 * count);
		auto global_max_level = r.get_vec<uint8_t>(count);
		auto global_cur_level = r.get_vec<uint8_t>(count);
		auto player_uses_global_default = r.get_vec<uint8_t>(12 * count);
		for (int player = 0; player < 12; ++player) {
			for (int upgrade = 0; upgrade < count; ++upgrade) {
				map_st.max_upgrade_levels[player][upgrade] = !!player_uses_global_default[player*count + upgrade] ? global_max_level[upgrade] : player_max_level[player*count + upgrade];
				st.upgrade_levels[player][upgrade] = !!player_uses_global_default[player*count + upgrade] ? global_cur_level[upgrade] : player_cur_level[player*count + upgrade];
			}
		}
	};
	auto tech_restrictions = [&](data_reader r, bool broodwar) {
		int count = broodwar ? 44 : 24;
		auto player_available = r.get_vec<uint8_t>(12 * count);
		auto player_researched = r.get_vec<uint8_t>(12 * count);
		auto global_available = r.get_vec<uint8_t>(count);
		auto global_researched = r.get_vec<uint8_t>(count);
		auto player_uses_global_default = r.get_vec<uint8_t>(12 * count);
		for (int player = 0; player < 12; ++player) {
			for (int upgrade = 0; upgrade < count; ++upgrade) {
				map_st.tech_available[player][upgrade] = !!(!!player_uses_global_default[player*count + upgrade] ? global_available[upgrade] : player_available[player*count + upgrade]);
				st.tech_researched[player][upgrade] = !!(!!player_uses_global_default[player*count + upgrade] ? global_researched[upgrade] : player_researched[player*count + upgrade]);
			}
		}
	};

	tag_funcs['UNIS'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		units(r, false);
	};
	tag_funcs['UPGS'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		upgrades(r, false);
	};
	tag_funcs['TECS'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		techdata(r, false);
	};
	tag_funcs['PUNI'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		auto player_available = r.get_vec<std::array<uint8_t, 228>>(12);
		auto global_available = r.get_vec<uint8_t>(228);
		auto player_uses_global_default = r.get_vec<std::array<uint8_t, 228>>(12);
		for (int player = 0; player < 12; ++player) {
			for (int unit = 0; unit < 228; ++unit) {
				map_st.unit_type_allowed[player][unit] = !!(!!player_uses_global_default[player][unit] ? global_available[unit] : player_available[player][unit]);
			}
		}
	};
	tag_funcs['UPGR'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		upgrade_restrictions(r, false);
	};
	tag_funcs['PTEC'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		tech_restrictions(r, false);
	};

	tag_funcs['UNIx'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		units(r, true);
	};
	tag_funcs['UPGx'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		upgrades(r, true);
	};
	tag_funcs['TECx'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		techdata(r, true);
	};
	tag_funcs['PUPx'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		upgrade_restrictions(r, true);
	};
	tag_funcs['PTEx'] = [&](data_reader r) {
		if (bVictoryCondition || bStartingUnits || bTournamentModeEnabled) xcept("wrong game mode");
		tech_restrictions(r, true);
	};

	tag_funcs['UNIT'] = [&](data_reader r) {
		while (r.left()) {

			int id = r.get<uint32_t>();
			int x = r.get<uint16_t>();
			int y = r.get<uint16_t>();
			int unit_type = r.get<uint16_t>();
			int link = r.get<uint16_t>();
			int valid_flags = r.get<uint16_t>();
			int valid_properties = r.get<uint16_t>();

		}
	};

	read_chunks({
		{'VER ', true},
		{'DIM ', true},
		{'ERA ', true},
		{'OWNR', true},
		{'SIDE', true},
		{'STR ', true},
		{'SPRP', true},
		{'FORC', true},
		{'VCOD', true}
	});

	generate_sight_values(map_st);

	load_tile_stuff(map_st);


	if (version == 59) {

		// todo: check game mode
		// this is for use map settings
		tag_list_t tags = {
			{'STR ', true},
			{'MTXM', true},
			{'THG2', true},
			{'MASK', true},
			{'UNIS', true},
			{'UPGS', true},
			{'TECS', true},
			{'PUNI', true},
			{'UPGR', true},
			{'PTEC', true},
			{'UNIx', false},
			{'UPGx', false},
			{'TECx', false},
			{'PUPx', false},
			{'PTEx', false},
			{'UNIT', true},
			{'UPRP', true},
			{'MRGN', true},
			{'TRIG', true}
		};
		read_chunks(tags);

	}
	else xcept("unsupported map version %d", version);

}

void load_map(state&st) {

	auto&map_st = *st.game->map;

	load_map_file(st, map_st.map_file_name);

}

void init_global_state(global_state&g) {

	auto units_dat = load_units_dat("arr\\units.dat");
	auto weapons_dat = load_weapons_dat("arr\\weapons.dat");
	auto upgrades_dat = load_upgrades_dat("arr\\upgrades.dat");
	auto techdata_dat = load_techdata_dat("arr\\techdata.dat");

	calculate_unit_strengths(g, units_dat, weapons_dat);

	map_state map_st;
	state st;
	game_state game_st;
	game_st.g = &g;
	game_st.map = &map_st;
	st.game = &game_st;

	map_st.units_dat = std::move(units_dat);
	map_st.weapons_dat = std::move(weapons_dat);
	map_st.upgrades_dat = std::move(upgrades_dat);
	map_st.techdata_dat = std::move(techdata_dat);

	map_st.map_file_name = R"(X:\Starcraft\StarCraft\maps\testone.scm)";

	load_map(st);
}

}

