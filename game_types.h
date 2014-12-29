

struct image_t;
struct sprite_t;
struct order_t;
struct path_t;
struct unit_t;

struct target_t {
	xy pos;
	unit_t* unit;
};

struct rect_t {
	int left;
	int top;
	int right;
	int bottom;
};

struct link_base {
	std::pair<link_base*, link_base*> link;
};

struct default_link_f {
	template<typename T>
	auto*operator()(T*ptr) {
		return (std::pair<T*, T*>*)&ptr->link;
	}
};

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

//
// These are based on BWAPIs CUnit. I've kept the order the same and names similar for convenience.
// This also means the order is the same as in Broodwar, but the types (and thus sizes, offsets) are not.
//
//
// These are all memset to 0 at game start, because Broodwar does so, and I believe it's necessary in order to
// replicate some minor "bugs".
// For intrusive_list, only clear() or the destructor can be called after a memset.
//
// The reason for the pointer cast operators is that intrusive_list works with references, but I'd like
// to get pointers when iterating.
//

struct iscript_state_t {
	const iscript_t::script*current_script;
	size_t program_counter;
	size_t return_address;
	int animation;
	int wait;
};

struct image_t: link_base {

	operator image_t*() {
		return this;
	}
	enum {
		flag_iscript_running = 0x10,
		flag_hidden = 0x20
	};

	int image_id;
	int palette_type;
	int direction;
	int flags;
	xy offset;
	iscript_state_t iscript_state;
	int frame_set;
	int frame_index;
	xy map_position;
	xy screen_position;
	rect_t grp_bounds;
	void* grp_file;
	int coloring_data;
	// void(*draw)(...);
	// void(*update(...);
	sprite_t* sprite;

};

struct sprite_t: link_base {

	operator sprite_t*() {
		return this;
	}

	int sprite_id;
	int owner;
	int selection_index;
	int visibility_flags;
	int elevation_level;
	int flags;
	int selection_timer;
	int index;
	xy position;
	image_t* main_image;
	intrusive_list<image_t, default_link_f> images;

};

struct flingy_t: link_base {

	operator flingy_t*() {
		return this;
	}

	int hp;
	sprite_t* sprite;
	target_t move_target;
	xy next_movement_waypoint;
	xy next_target_waypoint;
	int movement_flags;
	int current_direction1;
	int flingy_turn_radius;
	int velocity_direction1;
	int flingy_id;
	int unknown_0x026;
	int flingy_movement_type;
	xy position;
	xy halt;
	int flingy_top_speed;
	int current_speed1;
	int current_speed2;
	xy current_speed;
	int flingy_acceleration;
	int current_direction2;
	int velocity_direction2;
	int owner;
	int order_id;
	int order_state;
	int order_signal;
	int order_unit_type;
	int main_order_timer;
	int ground_weapon_cooldown;
	int air_weapon_cooldown;
	int spell_cooldown;
	target_t order_target;

};

struct order_t : link_base {

};

struct unit_t: flingy_t {

	// We must supply a constructor since there are intrusive_lists inside unions,
	// and intrusive_lists have non-trivial constructors. However, they have no
	// destructors and clear() has absolutely no preconditions (the object does
	// not have to be in a valid state), so we don't need to do anything special.
	unit_t() {}
	operator unit_t*() {
		return this;
	}

	int shield_points;
	int unit_type;

	std::pair<unit_t*, unit_t*> player_units_link;

	unit_t* subunit;
	intrusive_list<order_t, default_link_f> order_queue;
	unit_t* auto_target_unit;
	unit_t* connected_unit;
	unit_t* order_queue_count;
	int order_queue_timer;
	int unknown_0x086;
	int attack_notify_timer;
	int displayed_unit_id;
	int last_event_timer;
	int last_event_color;
	// _unused_0x08c
	int rank_increase;
	int kill_count;
	int last_attacking_player;
	int secondary_order_timer;
	// int ai_action_flag;
	int user_action_flags;
	int current_button_set;
	bool is_cloaked;
	int movement_state;
	std::array<int, 5> build_queue;
	int energy;
	int build_queue_slot;
	int unit_id_generation;
	int secondary_order_id;
	int building_overlay_state;
	int hp_gain_during_repair;
	int unknown_0x0aa;
	int remaining_build_time;
	int previous_hp;
	std::array<unit_id, 8> loaded_units;

	union {
		struct {
			int spider_mine_count;
		} vulture;
		struct {
			unit_t* parent;
			std::pair<unit_t*, unit_t*> fighter_link;
			bool in_hangar;
		} fighter;
		struct fighter_link {
			auto* operator()(unit_t*ptr) {
				return &ptr->fighter.fighter_link;
			}
		};
		struct {
			intrusive_list<unit_t, fighter_link> inside_units;
			intrusive_list<unit_t, fighter_link> outside_units;
			int inside_count;
			int outside_count;
		} carrier;
		struct {
			int unknown_00;
			int unknown_04;
			int flag_spawn_frame;
		} beacon;

		struct {
			unit_t* powerup;
			xy target_resource;
			unit_t* target_resource_unit;
			int repair_resource_loss_timer;
			bool is_carrying_something;
			int resource_carry_count;
			unit_t* harvest_target;
			std::pair<unit_t*, unit_t*> gather_link;
		} worker;
		struct worker_gather_link {
			auto* operator()(unit_t*ptr) {
				return &ptr->worker.gather_link;
			}
		};

		struct {
			unit_t* addon;
			int addon_build_type;
			int upgrade_research_time;
			int tech_type;
			int upgrade_type;
			int larva_timer;
			int landing_timer;
			int creep_timer;
			int upgrade_level;
			union {
				struct {
					int resource_count;
					int resource_iscript;
					int gather_queue_count;
					intrusive_list<unit_t, worker_gather_link> gather_queue;
					int resource_group;
					bool resource_belongs_to_ai;
				} resource;
				struct {
					unit_t* exit;
				} nydus;
				struct {
					sprite_t* nuke_dot;
				} ghost;
				struct {
					sprite_t* pylon_aura;
				} pylon;
				struct {
					unit_t* nuke;
					bool ready;
				} silo;
				
				struct {
					xy origin;
				} powerup;
			};
		} building;
	};

	int status_flags;
	int resource_type;
	int wireframe_randomizer;
	int secondary_order_state;
	int recent_order_timer;
	int visibility_flags;
	int unknown_0x0e8;
	int unknown_0x0ea;
	unit_t*current_build_unit;
	std::pair<unit_t*, unit_t*> burrowed_unit_link;

	union {
		struct {
			xy position;
			unit_t* unit;
		} rally;
		struct {
			std::pair<unit_t*, unit_t*> psi_link;
		} pylon;
	};

	path_t* path;
	int pathing_collision_interval;
	int pathing_flags;
	int unused_0x106;
	bool is_being_healed;
	rect_t contour_bounds;

	int remove_timer;
	int defense_matrix_damage;
	int defense_matrix_timer;
	int stim_timer;
	int ensnare_timer;
	int lockdown_timer;
	int irradiate_timer;
	int stasis_timer;
	int plague_timer;
	int storm_timer;
	unit_t* irradiated_by;
	int irradiate_owner;
	int parasite_flags;
	int cycle_counter;
	bool is_blind;
	int maelstrom_timer;
	int unused_0x125;
	int acid_spore_count;
	std::array<int, 9> acid_spore_time;

	int bullet_behavior_3_by_3_attack_sequence;
	void* ai;
	int air_strength;
	int ground_strength;
	rect_t finder;
	int repulse_unknown;
	int repulse_angle;
	xy drift_pos;

};

