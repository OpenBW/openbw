#ifndef BWGAME_DATA_TYPES_H
#define BWGAME_DATA_TYPES_H

#include "bwenums.h"
#include "util.h"
#include "containers.h"

namespace bwgame {

struct unit_type_t;
struct weapon_type_t;
struct upgrade_type_t;
struct tech_type_t;
struct flingy_type_t;
struct sprite_type_t;
struct image_type_t;
struct order_type_t;


template<typename T>
struct type_container {
	a_vector<T> vec;
	type_container() = default;
	type_container(const type_container&) = delete;
	type_container(type_container&&) = default;
	type_container& operator=(const type_container&) = delete;
	type_container& operator=(type_container&&) = default;
};

template<typename T>
struct id_type_for;
template<> struct id_type_for<flingy_type_t> { using type = FlingyTypes; };
template<> struct id_type_for<unit_type_t> { using type = UnitTypes; };
template<> struct id_type_for<weapon_type_t> { using type = WeaponTypes; };
template<> struct id_type_for<upgrade_type_t> { using type = UpgradeTypes; };
template<> struct id_type_for<tech_type_t> { using type = TechTypes; };
template<> struct id_type_for<sprite_type_t> { using type = SpriteTypes; };
template<> struct id_type_for<image_type_t> { using type = ImageTypes; };
template<> struct id_type_for<order_type_t> { using type = Orders; };
template<typename T>
using id_type_for_t = typename id_type_for<typename std::remove_const<T>::type>::type;

template<typename T>
struct type_id {
private:
	using id_T = id_type_for_t<T>;
	id_T id;
	T* pointer;
public:
	type_id() = default;
	type_id(T* pointer) : id(pointer ? pointer->id : id_T::None),  pointer(pointer) {}
	type_id(id_T id) : id(id), pointer(nullptr) {}
	T* operator->() {
		return pointer;
	}
	const T* operator->() const {
		return pointer;
	}
	T& operator*() {
		return *pointer;
	}
	const T& operator*() const {
		return *pointer;
	}
	explicit operator bool() const {
		return pointer != nullptr;
	}
	operator T*() {
		return pointer;
	}
	operator const T*() const {
		return pointer;
	}
	operator id_T() const {
		return id;
	}
};

struct unit_type_t {
	enum flags_t : uint32_t {
		flag_building = 1,
		flag_addon = 2,
		flag_flyer = 4,

		flag_worker = 8,
		flag_turret = 0x10,
		flag_flying_building = 0x20,
		flag_hero = 0x40,
		flag_regens_hp = 0x80,
		flag_100 = 0x100,
		
		flag_two_units_in_one_egg = 0x400,
		flag_powerup = 0x800,
		flag_resource_depot = 0x1000,
		flag_resource = 0x2000,
		flag_robotic = 0x4000,
		flag_detector = 0x8000,
		flag_organic = 0x10000,
		flag_requires_creep = 0x20000,

		flag_requires_psionic_matrix = 0x80000,
		flag_can_burrow = 0x100000,
		flag_has_energy = 0x200000,
		flag_initially_cloaked = 0x400000,
		flag_sprite_size_medium = 0x2000000,
		flag_sprite_size_large = 0x4000000,
		flag_can_move = 0x8000000,
		flag_can_turn = 0x10000000,
		flag_invincible = 0x20000000,
		flag_mechanical = 0x40000000,
		flag_80000000 = 0x80000000,
	};

	UnitTypes id;

	type_id<const flingy_type_t> flingy;
	type_id<unit_type_t> turret_unit_type;
	type_id<unit_type_t> subunit2;
	type_id<unit_type_t> infestation_unit;
	type_id<const image_type_t> construction_animation;
	int unit_direction;
	bool has_shield;
	int shield_points;
	fp8 hitpoints;
	int elevation_level;
	int unknown1;
	int sublabel;
	type_id<const order_type_t> computer_ai_idle;
	type_id<const order_type_t> human_ai_idle;
	type_id<const order_type_t> return_to_idle;
	type_id<const order_type_t> attack_unit;
	type_id<const order_type_t> attack_move;
	type_id<const weapon_type_t> ground_weapon;
	int max_ground_hits;
	type_id<const weapon_type_t> air_weapon;
	int max_air_hits;
	int ai_internal;
	flags_t flags;
	int target_acquisition_range;
	int sight_range;
	type_id<upgrade_type_t> armor_upgrade;
	int unit_size;
	int armor;
	size_t right_click_action;
	int ready_sound;
	int first_what_sound;
	int last_what_sound;
	int first_pissed_sound;
	int last_pissed_sound;
	int first_yes_sound;
	int last_yes_sound;
	xy placement_size;
	xy addon_position;
	rect dimensions;
	int portrait;
	int mineral_cost;
	int gas_cost;
	int build_time;
	int unknown2;
	int group_flags;
	fp1 supply_provided;
	fp1 supply_required;
	size_t space_required;
	size_t space_provided;
	int build_score;
	int destroy_score;
	size_t unit_map_string_index;
	bool is_broodwar;
	int staredit_availability_flags;
};

using unit_types_t = type_container<unit_type_t>;

struct weapon_type_t {
	enum {
		bullet_type_fly,
		bullet_type_follow_target,
		bullet_type_appear_at_target_unit,
		bullet_type_persist_at_target_pos,
		bullet_type_appear_at_target_pos,
		bullet_type_appear_at_source_unit,
		bullet_type_self_destruct,
		bullet_type_bounce,
		bullet_type_attack_target_pos,
		bullet_type_extend_to_max_range
	};
	enum {
		hit_type_none,
		hit_type_normal,
		hit_type_radial_splash,
		hit_type_enemy_splash,
		hit_type_lockdown,
		hit_type_nuclear_missile,
		hit_type_parasite,
		hit_type_broodlings,
		hit_type_emp_shockwave,
		hit_type_irradiate,
		hit_type_ensnare,
		hit_type_plague,
		hit_type_stasis_field,
		hit_type_dark_swarm,
		hit_type_consume,
		hit_type_yamato_gun,
		hit_type_restoration,
		hit_type_disruption_web,
		hit_type_corrosive_acid,
		hit_type_mind_control,
		hit_type_feedback,
		hit_type_optical_flare,
		hit_type_maelstrom,
		hit_type_23,
		hit_type_air_splash
	};
	enum {
		damage_type_none,
		damage_type_explosive,
		damage_type_concussive,
		damage_type_normal,
		damage_type_ignore_armor
	};

	WeaponTypes id;

	int label;
	type_id<const flingy_type_t> flingy;
	int unused;
	int target_flags;
	int min_range;
	int max_range;
	type_id<upgrade_type_t> damage_upgrade;
	int damage_type;
	int bullet_type;
	int lifetime;
	int hit_type;
	int inner_splash_radius;
	int medium_splash_radius;
	int outer_splash_radius;
	int damage_amount;
	int damage_bonus;
	int cooldown;
	int bullet_count;
	fp8 attack_angle;
	direction_t bullet_heading_offset;
	int forward_offset;
	int upward_offset;
	int target_error_message;
	int icon;
};

using weapon_types_t = type_container<weapon_type_t>;

struct upgrade_type_t {
	UpgradeTypes id;

	int mineral_cost_base;
	int mineral_cost_factor;
	int gas_cost_base;
	int gas_cost_factor;
	int time_cost_base;
	int time_cost_factor;
	int unknown;
	int icon;
	int label;
	int race;
	int max_level;
	bool is_broodwar;
};

using upgrade_types_t = type_container<upgrade_type_t>;

struct tech_type_t {
	TechTypes id;

	int mineral_cost;
	int gas_cost;
	int research_time;
	int energy_cost;
	int unknown;
	int icon;
	int label;
	int race;
	int flags;
};

using tech_types_t = type_container<tech_type_t>;

struct flingy_type_t {
	FlingyTypes id;

	type_id<sprite_type_t> sprite;
	fp8 top_speed;
	fp8 acceleration;
	fp8 halt_distance;
	fp8 turn_rate;
	int unused;
	int movement_type;
};

using flingy_types_t = type_container<flingy_type_t>;

struct sprite_type_t {
	SpriteTypes id;

	type_id<image_type_t> image;
	int health_bar_size;
	int unk0;
	bool visible;
	int selection_circle;
	int selection_circle_vpos;
};

using sprite_types_t = type_container<sprite_type_t>;

struct image_type_t {
	ImageTypes id;

	int grp_filename_index;
	bool has_directional_frames;
	bool is_clickable;
	bool has_iscript_animations;
	bool always_visible;
	int modifier;
	int color_shift;
	int iscript_id;
	int shield_filename_index;
	int attack_filename_index;
	int damage_filename_index;
	int special_filename_index;
	int landing_dust_filename_index;
	int lift_off_filename_index;
};

using image_types_t = type_container<image_type_t>;

struct order_type_t {
	Orders id;

	int label;
	bool targets_enemies;
	int background;
	int unused3;
	bool valid_for_turret;
	int unused5;
	bool can_be_interrupted;
	bool unk7;
	bool can_be_queued;
	int unk9;
	bool can_be_obstructed;
	int unk11;
	int unused12;
	WeaponTypes weapon;    // weapon and tech_type could be pointers, but then order_types would 
	TechTypes tech_type;   // need to live in game_state
	int animation;
	int highlight;
	int dep_index;
	Orders target_order;
};

using order_types_t = type_container<order_type_t>;

struct sound_type_t {
	Sounds id;

	int filename_index;
	int priority;
	int flags;
	int race;
	int min_volume;
};

using sound_types_t = type_container<sound_type_t>;

namespace iscript_opcodes {
	enum {
		opc_playfram,
		opc_playframtile,
		opc_sethorpos,
		opc_setvertpos,
		opc_setpos,
		opc_wait,
		opc_waitrand,
		opc_goto,
		opc_imgol,
		opc_imgul,
		opc_imgolorig,
		opc_switchul,
		opc___0c,
		opc_imgoluselo,
		opc_imguluselo,
		opc_sprol,
		opc_highsprol,
		opc_lowsprul,
		opc_uflunstable,
		opc_spruluselo,
		opc_sprul,
		opc_sproluselo,
		opc_end,
		opc_setflipstate,
		opc_playsnd,
		opc_playsndrand,
		opc_playsndbtwn,
		opc_domissiledmg,
		opc_attackmelee,
		opc_followmaingraphic,
		opc_randcondjmp,
		opc_turnccwise,
		opc_turncwise,
		opc_turn1cwise,
		opc_turnrand,
		opc_setspawnframe,
		opc_sigorder,
		opc_attackwith,
		opc_attack,
		opc_castspell,
		opc_useweapon,
		opc_move,
		opc_gotorepeatattk,
		opc_engframe,
		opc_engset,
		opc___2d,
		opc_nobrkcodestart,
		opc_nobrkcodeend,
		opc_ignorerest,
		opc_attkshiftproj,
		opc_tmprmgraphicstart,
		opc_tmprmgraphicend,
		opc_setfldirect,
		opc_call,
		opc_return,
		opc_setflspeed,
		opc_creategasoverlays,
		opc_pwrupcondjmp,
		opc_trgtrangecondjmp,
		opc_trgtarccondjmp,
		opc_curdirectcondjmp,
		opc_imgulnextid,
		opc___3e,
		opc_liftoffcondjmp,
		opc_warpoverlay,
		opc_orderdone,
		opc_grdsprol,
		opc___43,
		opc_dogrddamage
	};
}
namespace iscript_anims {
	enum {
		Init,
		Death,
		GndAttkInit,
		AirAttkInit,
		Unused1,
		GndAttkRpt,
		AirAttkRpt,
		CastSpell,
		GndAttkToIdle,
		AirAttkToIdle,
		Unused2,
		Walking,
		WalkingToIdle,
		SpecialState1,
		SpecialState2,
		AlmostBuilt,
		Built,
		Landing,
		LiftOff,
		IsWorking,
		WorkingToIdle,
		WarpIn,
		Unused3,
		StarEditInit,
		Disable,
		Burrow,
		UnBurrow,
		Enable
	};
}

struct iscript_t {
	struct script {
		int id;
		a_vector<size_t> animation_pc;
	};
	a_unordered_map<int, script> scripts;
	a_vector<int> program_data;
};

struct grp_t {
	struct frame_t {
		xy_t<size_t> offset;
		xy_t<size_t> size;
		a_vector<size_t> line_data_offset;
		a_vector<uint8_t> data_container;
	};
	size_t width;
	size_t height;
	a_vector<frame_t> frames;
};

}

#endif
