

struct image_t;
struct sprite_t;
struct order_t;
struct path_t;
struct unit_t;

struct target_t {
	xy pos;
	unit_t*unit = nullptr;
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
		flag_redraw = 1,
		flag_horizontally_flipped = 2,
		flag_y_frozen = 4,
		flag_has_directional_frames = 8,
		flag_has_iscript_animations = 0x10,
		flag_hidden = 0x40
	};
	enum {
		palette_type_hallucination = 16
	};

	const image_type_t*image_type;
	int palette_type;
	int direction;
	int flags;
	xy offset;
	iscript_state_t iscript_state;
	int frame_set;
	int frame_index;
	xy map_position;
	xy screen_position;
	rect grp_bounds;
	grp_t*grp;
	int coloring_data;
	sprite_t*sprite;

};

struct sprite_t: link_base {

	operator sprite_t*() {
		return this;
	}

	enum flags_t {
		flag_selected = 0x8,
		flag_hidden = 0x20,

		flag_iscript_nobrk = 0x80,
	};

	const sprite_type_t*sprite_type;
	int owner;
	int selection_index;
	int visibility_flags;
	int elevation_level;
	int flags;
	int selection_timer;
	int index;
	int width;
	int height;
	xy position;
	image_t*main_image;
	intrusive_list<image_t, default_link_f> images;

};

struct flingy_t: link_base {

	operator flingy_t*() {
		return this;
	}

	int hp;
	sprite_t*sprite;
	target_t move_target;
	xy next_movement_waypoint;
	xy next_target_waypoint;
	int movement_flags;
	int current_direction1;
	int flingy_turn_rate;
	int velocity_direction1;
	const flingy_type_t*flingy_type;
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
	const order_type_t* order_type;
	int order_state;
	int order_signal;
	const unit_type_t* order_unit_type;
	int main_order_timer;
	int ground_weapon_cooldown;
	int air_weapon_cooldown;
	int spell_cooldown;
	target_t order_target;

};

struct order_target {
	xy position;
	unit_t* unit;
	unit_type_t* unit_type;
};

struct order_t : link_base {

	operator order_t*() {
		return this;
	}

	const order_type_t* order_type;
	order_target target;
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

	enum status_flags_t : uint32_t {
		status_flag_completed = 1,
		status_flag_grounded_building = 2,
		status_flag_flying = 4,
		status_flag_disabled = 8,
		status_flag_burrowed = 0x10,
		status_flag_in_building = 0x20,

		status_flag_requires_detector = 0x100,
		status_flag_cloaked = 0x200,
		status_flag_frozen = 0x400,

		status_flag_order_not_interruptible = 0x1000,
		status_flag_iscript_nobrk = 0x2000,

		status_flag_can_not_attack = 0x8000,
		status_flag_can_move_or_attack = 0x10000,
		status_flag_can_move = 0x20000,
		//status_flag_ignore_tile_collision = 0x40000,
		status_flag_collision = 0x40000,
		status_flag_immovable = 0x80000,
		status_flag_ground_unit = 0x100000,
		status_flag_gathering = 0x800000,

		status_flag_invincible = 0x4000000,
		status_flag_holding_position = 0x8000000,

		status_flag_speed_upgrade = 0x10000000,
		status_flag_cooldown_upgrade = 0x20000000,
		status_flag_hallucination = 0x40000000,
	};

	int shield_points;
	const unit_type_t*unit_type;

	std::pair<unit_t*, unit_t*> player_units_link;

	unit_t*subunit;
	intrusive_list<order_t, default_link_f> order_queue;
	unit_t*auto_target_unit;	
	unit_t*connected_unit;
	int order_queue_count;
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
	std::array<unit_type_t*, 5> build_queue;
	int energy;
	int build_queue_slot;
	int unit_id_generation;
	int secondary_order_id;
	// int building_overlay_state; fixme
	int hp_construction_rate;
	int shield_construction_rate;
	int remaining_build_time;
	int previous_hp;
	std::array<unit_id, 8> loaded_units;

	union {
		struct {
			int spider_mine_count;
		} vulture;
		struct {
			unit_t*parent;
			std::pair<unit_t*, unit_t*> fighter_link;
			bool in_hangar;
		} fighter;
		struct fighter_link {
			auto*operator()(unit_t*ptr) {
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
	};

	struct {
		unit_t*powerup;
		xy target_resource;
		unit_t*target_resource_unit;
		int repair_resource_loss_timer;
		bool is_carrying_something;
		int resource_carry_count;
		unit_t*harvest_target;
		std::pair<unit_t*, unit_t*> gather_link;
	} worker;
	struct worker_gather_link {
		auto*operator()(unit_t*ptr) {
			return &ptr->worker.gather_link;
		}
	};

	struct building_t {
		building_t() {}
		unit_t*addon;
		unit_type_t*addon_build_type;
		int upgrade_research_time;
		tech_type_t*tech_type;
		upgrade_type_t*upgrade_type;
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
				unit_t*exit;
			} nydus;
			struct {
				sprite_t*nuke_dot;
			} ghost;
			struct {
				sprite_t*pylon_aura;
			} pylon;
			struct {
				unit_t*nuke;
				bool ready;
			} silo;
			struct {
				xy origin;
			} powerup;
		};
	} building;

	int status_flags;
	int resource_type;
	int wireframe_randomizer;
	int secondary_order_state;
	int recent_order_timer;
	int visibility_flags;
	int secondary_order_unk_a;
	int secondary_order_unk_b;
	unit_t*current_build_unit;
	std::pair<unit_t*, unit_t*> burrowed_unit_link;

	union {
		struct {
			xy position;
			unit_t*unit;
		} rally;
		struct {
			std::pair<unit_t*, unit_t*> psi_link;
		} pylon;
	};

	path_t*path;
	unsigned int pathing_collision_interval;
	int pathing_flags;
	int unused_0x106;
	bool is_being_healed;
	rect contour_bounds;

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
	unit_t*irradiated_by;
	int irradiate_owner;
	int parasite_flags;
	int cycle_counter;
	bool is_blind;
	int maelstrom_timer;
	int unused_0x125;
	int acid_spore_count;
	std::array<int, 9> acid_spore_time;

	int bullet_behavior_3_by_3_attack_sequence;
	void*ai;
	int air_strength;
	int ground_strength;
	size_t unit_finder_left_index;
	size_t unit_finder_top_index;
	size_t unit_finder_right_index;
	size_t unit_finder_bottom_index;
	int repulse_unknown;
	int repulse_angle;
	xy drift_pos;

	int unit_finder_mark;
};

