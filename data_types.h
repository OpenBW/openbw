
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

struct unit_type_t {
	enum flags_t : uint32_t {
		flag_building = 1,

		flag_flyer = 4,

		flag_worker = 8,
		flag_turret = 0x10,
		flag_flying_building = 0x20,
		flag_hero = 0x40,
		flag_regens_hp = 0x80,

		flag_two_units_in_one_egg = 0x400,
		flag_detector = 0x800,

		flag_resource = 0x2000,

		flag_creep = 0x20000,

		flag_has_energy = 0x200000,
		flag_initially_cloaked = 0x400000,
		flag_can_move = 0x8000000,
		flag_can_turn = 0x10000000,
		flag_invincible = 0x20000000,
	};

	int id;

	const flingy_type_t* flingy;
	unit_type_t* turret_unit_type;
	unit_type_t* subunit2;
	int infestation;
	const image_type_t* construction_animation;
	int unit_direction;
	int has_shield;
	int shield_points;
	fp8 hitpoints;
	int elevation_level;
	int unknown1;
	int sublabel;
	const order_type_t* computer_ai_idle;
	const order_type_t* human_ai_idle;
	const order_type_t* return_to_idle;
	const order_type_t* attack_unit;
	const order_type_t* attack_move;
	const weapon_type_t* ground_weapon;
	int max_ground_hits;
	const weapon_type_t* air_weapon;
	int max_air_hits;
	int ai_internal;
	flags_t flags;
	int target_acquisition_range;
	int sight_range;
	upgrade_type_t* armor_upgrade;
	int unit_size;
	int armor;
	int right_click_action;
	int ready_sound;
	int first_what_sound;
	int last_what_sound;
	int first_pissed_sound;
	int last_pissed_sound;
	int first_yes_sound;
	int last_yes_sound;
	xy tile_size;
	int addon_horizontal;
	int addon_vertical;
	rect dimensions;
	int portrait;
	int mineral_cost;
	int gas_cost;
	int build_time;
	int unknown2;
	int group_flags;
	int supply_provided;
	int supply_required;
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
	int id;

	int label;
	const flingy_type_t* flingy;
	int unused;
	int target_flags;
	int min_range;
	int max_range;
	upgrade_type_t* damage_upgrade;
	int weapon_type;
	int weapon_behavior;
	int remove_after;
	int explosion_type;
	int inner_splash_radius;
	int medium_splash_radius;
	int outer_splash_radius;
	int damage_amount;
	int damage_bonus;
	int cooldown;
	int damage_factor;
	fp8 attack_angle;
	int launch_spin;
	int forward_offset;
	int upward_offset;
	int target_error_message;
	int icon;
};

using weapon_types_t = type_container<weapon_type_t>;

struct upgrade_type_t {
	int id;

	int mineral_cost_base;
	int mineral_cost_factor;
	int gas_cost_base;
	int gas_cost_factor;
	int research_time_base;
	int research_time_factor;
	int unknown;
	int icon;
	int label;
	int race;
	int max_level;
	bool is_broodwar;
};

using upgrade_types_t = type_container<upgrade_type_t>;

struct tech_type_t {
	int id;

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
	int id;

	sprite_type_t* sprite;
	fp8 top_speed;
	fp8 acceleration;
	fp8 halt_distance;
	fp8 turn_rate;
	int unused;
	int movement_type;
};

using flingy_types_t = type_container<flingy_type_t>;

struct sprite_type_t {
	int id;

	image_type_t* image;
	int health_bar_size;
	int unk0;
	bool visible;
	int selection_circle;
	int selection_circle_vpos;
};

using sprite_types_t = type_container<sprite_type_t>;

struct image_type_t {
	int id;

	int grp_filename_index;
	bool has_directional_frames;
	bool is_clickable;
	bool has_iscript_animations;
	bool hidden_until_unit_completed;
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
	int id;

	int use_weapon_targeting;
	int background;
	int unused3;
	bool valid_for_turret;
	int unused5;
	bool can_be_interrupted;
	int unk7;
	bool can_be_queued;
	int unk9;
	bool can_be_obstructed;
	int unk11;
	int unused12;
	int weapon;    // weapon and tech_type could be pointers, but then order_types would 
	int tech_type; // need to live in game_state. Since I'd like to keep it in global_state,
	int seq;       // they will remain as ints
	int highlight;
	int dep_index;
	int obscured;
};

using order_types_t = type_container<order_type_t>;

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
};
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
};

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
		int left;
		int top;
		int right;
		int bottom;
	};
	int width;
	int height;
	a_vector<frame_t> frames;
};

