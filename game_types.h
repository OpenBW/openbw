#ifndef BWGAME_GAME_TYPES_H
#define BWGAME_GAME_TYPES_H

#include "util.h"
#include "data_types.h"
#include "containers.h"

namespace bwgame {

struct sprite_t;
struct flingy_t;
struct unit_t;

struct unit_id {
	uint16_t raw_value = 0;
	unit_id() = default;
	explicit unit_id(uint16_t raw_value) : raw_value(raw_value) {}
	explicit unit_id(size_t index, int generation) : raw_value((uint16_t)(index | generation << 11)) {}
	size_t index() const {
		return raw_value & 0x7ff;
	}
	int generation() const {
		return raw_value >> 11;
	}
};

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
		maskdat_node_t* prev; // the tile from us directly towards the origin (diagonal is allowed and preferred)
		// the other tile with equal diagonal distance to the origin as prev, if it exists.
		// otherwise, it is prev
		maskdat_node_t* prev2;
		size_t map_index_offset;
		// temporary variable used when spreading vision to make sure we don't go through obstacles
		mutable uint32_t vision_propagation;
		int8_t x;
		int8_t y;
		// prev_count will be 1 if prev and prev2 are equal, otherwise it is 2
		int8_t prev_count;
	};

	int max_width, max_height;
	int min_width, min_height;
	int min_mask_size;
	int ext_masked_count;
	maskdat_t maskdat;

};

struct cv5_entry {
	uint16_t flags;
	std::array<uint16_t, 16> mega_tile_index;
};
struct vf4_entry {
	enum flags_t : uint16_t {
		flag_walkable = 1,
		flag_middle = 2,
		flag_high = 4,
		flag_very_high = 8
	};

	std::array<uint16_t, 16> flags;
};

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
		flag_partially_walkable = 0x2000,
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

struct regions_t {

	struct region {
		uint16_t flags = 0x1FFD;
		size_t index = ~(size_t)0;
		xy_t<size_t> tile_center;
		rect_t<xy_t<size_t>> tile_area;
		xy_t<fp8> center;
		rect area;
		size_t tile_count = 0;
		size_t group_index = 0;
		a_vector<region*> walkable_neighbors;
		a_vector<region*> non_walkable_neighbors;
		int priority = 0;

		mutable int pathfinder_flag = 0; // fixme: remove
		mutable void* pathfinder_node = nullptr; // fixme: remove

		bool walkable() const {
			return flags != 0x1ffd;
		}
	};

	struct split_region {
		uint16_t mask;
		region* a;
		region* b;
	};

	struct contour {
		std::array<int, 3> v;
		size_t dir;
		uint8_t flags;
	};

	// tile_region_index values -
	//  [0, 5000) index into regions
	//  [5000, 0x2000) unmapped (0x1ffd unwalkable, otherwise walkable)
	//  [0x2000, ...] index + 0x2000 into split_regions
	a_vector<size_t> tile_region_index = a_vector<size_t>(256 * 256);

	rect_t<xy_t<size_t>> tile_bounding_box;

	a_vector<region> regions;

	a_vector<split_region> split_regions;

	std::array<a_vector<contour>, 4> contours;

};



struct target_t {
	xy pos;
	unit_t* unit = nullptr;
};

struct link_base {
	std::pair<link_base*, link_base*> link;
};

struct default_link_f {
	template<typename T>
	auto* operator()(T* ptr) {
		return (std::pair<T*, T*>*)&ptr->link;
	}
};


struct iscript_state_t {
	const iscript_t::script* current_script;
	size_t program_counter;
	size_t return_address;
	int animation;
	int wait;
};

struct image_t: link_base {

	enum flags_t : uint_fast32_t {
		flag_redraw = 1,
		flag_horizontally_flipped = 2,
		flag_y_frozen = 4,
		flag_has_directional_frames = 8,
		flag_has_iscript_animations = 0x10,
		flag_hidden = 0x40,
		flag_uses_special_offset = 0x80
	};
	enum {
		modifier_cloaked = 2,
		modifier_hallucination = 16
	};

	const image_type_t* image_type;
	int modifier;
	size_t frame_index_offset;
	int flags;
	xy offset;
	iscript_state_t iscript_state;
	size_t frame_index_base;
	size_t frame_index;
	grp_t* grp;
	int modifier_data1;
	int modifier_data2;
	sprite_t* sprite;

};

struct sprite_t: link_base {

	enum flags_t : uint_fast32_t {
		flag_selected = 0x8,
		flag_hidden = 0x20,
		flag_burrowed = 0x40,
		flag_iscript_nobrk = 0x80,
	};

	const sprite_type_t* sprite_type;
	int owner;
	int selection_index;
	int visibility_flags;
	int elevation_level;
	int flags;
	int selection_timer;
	size_t index;
	size_t width;
	size_t height;
	xy position;
	image_t* main_image;
	intrusive_list<image_t, default_link_f> images;

};

struct thingy_t: link_base {
	fp8 hp;
	sprite_t* sprite;
};

struct flingy_t: thingy_t {

	target_t move_target;
	xy next_movement_waypoint;
	xy next_target_waypoint;
	int movement_flags;
	direction_t heading;
	fp8 flingy_turn_rate;
	direction_t next_velocity_direction;
	const flingy_type_t* flingy_type;
	int flingy_movement_type;
	xy position;
	xy_fp8 exact_position;
	fp8 flingy_top_speed;
	fp8 current_speed;
	fp8 next_speed;
	xy_fp8 velocity;
	fp8 flingy_acceleration;
	direction_t current_velocity_direction;
	direction_t desired_velocity_direction;
	int order_signal;

};

struct bullet_t: flingy_t {
	enum {
		state_init,
		state_move,
		state_follow,
		state_bounce,
		state_damage_over_time,
		state_dying,
		state_hit_near_target
	};
	int bullet_state;
	target_t bullet_target;
	const weapon_type_t* weapon_type;
	int remaining_time;
	int hit_flags;
	int remaining_bounces;
	unit_t* source_unit;
	unit_t* prev_bounce_unit;
	size_t hit_near_target_position_index;
};

struct order_target_t {
	xy position;
	unit_t* unit = nullptr;
	const unit_type_t* unit_type = nullptr;
	order_target_t() = default;
	explicit order_target_t(unit_t* unit) : unit(unit) {}
};

struct order_t: link_base {

	const order_type_t* order_type;
	order_target_t target;
};

struct path_t: link_base {

	int delay = 0;
	int creation_frame = 0;
	int state_flags = 0;

	a_deque<const regions_t::region*> long_path;
	size_t full_long_path_size;
	a_deque<xy> short_path;

	size_t current_long_path_index = 0;
	size_t current_short_path_index = 0;

	xy source;
	xy destination;
	xy next;

	unit_id last_collision_unit;
	fp8 last_collision_speed;
	direction_t slide_free_direction;

};

struct unit_t: flingy_t {

	unit_t() {}

	enum status_flags_t : uint_fast32_t {
		status_flag_completed = 1,
		status_flag_grounded_building = 2,
		status_flag_flying = 4,
		status_flag_8 = 8,
		status_flag_burrowed = 0x10,
		status_flag_hidden = 0x20,

		status_flag_requires_detector = 0x100,
		status_flag_cloaked = 0x200,
		status_flag_disabled = 0x400,

		status_flag_order_not_interruptible = 0x1000,
		status_flag_iscript_nobrk = 0x2000,

		status_flag_cannot_attack = 0x8000,
		status_flag_can_turn = 0x10000,
		status_flag_can_move = 0x20000,
		status_flag_collision = 0x40000,
		status_flag_immovable = 0x80000,
		status_flag_ground_unit = 0x100000,
		status_flag_no_collide = 0x200000,
		status_flag_400000 = 0x400000,
		status_flag_gathering = 0x800000,

		status_flag_invincible = 0x4000000,
		status_flag_holding_position = 0x8000000,

		status_flag_speed_upgrade = 0x10000000,
		status_flag_cooldown_upgrade = 0x20000000,
		status_flag_hallucination = 0x40000000,
		status_flag_lifetime_expired = 0x80000000,
	};

	int owner;
	const order_type_t* order_type;
	int order_state;
	const unit_type_t* order_unit_type;
	int main_order_timer;
	int ground_weapon_cooldown;
	int air_weapon_cooldown;
	int spell_cooldown;
	target_t order_target;

	fp8 shield_points;
	const unit_type_t* unit_type;

	std::pair<unit_t*, unit_t*> player_units_link;

	unit_t* subunit;
	intrusive_list<order_t, default_link_f> order_queue;
	unit_t* auto_target_unit;	
	unit_t* connected_unit;
	int order_queue_count;
	int order_queue_timer;
	int unknown_0x086;
	int attack_notify_timer;
	int displayed_unit_id;
	int last_event_timer;
	int last_event_color;
	int rank_increase;
	int kill_count;
	int last_attacking_player;
	int secondary_order_timer;
	int user_action_flags;
	bool is_cloaked;
	int movement_state;
	static_vector<const unit_type_t*, 5> build_queue;
	fp8 energy;
	int unit_id_generation;
	const order_type_t* secondary_order_type;
	int building_overlay_state;
	fp8 hp_construction_rate;
	fp8 shield_construction_rate;
	int remaining_build_time;
	int previous_hp;
	std::array<unit_id, 8> loaded_units;

	struct fighter_link {
		auto* operator()(unit_t* ptr) {
			return &ptr->fighter.fighter_link;
		}
	};
	union {
		struct {
			size_t spider_mine_count;
		} vulture;
		struct {
			unit_t* parent;
			std::pair<unit_t*, unit_t*> fighter_link;
			bool in_hangar;
		} fighter;
		struct {
			intrusive_list<unit_t, fighter_link> inside_units;
			intrusive_list<unit_t, fighter_link> outside_units;
			size_t inside_count;
			size_t outside_count;
		} carrier, reaver;
		struct {
			int unknown_00;
			int unknown_04;
			int flag_spawn_frame;
		} beacon;
		struct {
			sprite_t* nuke_dot;
		} ghost;
	};

	struct {
		unit_t* powerup;
		xy target_resource_position;
		unit_t* target_resource_unit;
		int repair_resource_loss_timer;
		bool is_gathering;
		int resources_carried;
		unit_t* gather_target;
		std::pair<unit_t*, unit_t*> gather_link;
	} worker;
	struct worker_gather_link {
		auto* operator()(unit_t* ptr) {
			return &ptr->worker.gather_link;
		}
	};

	struct building_t {
		building_t() {}
		unit_t* addon;
		unit_type_t* addon_build_type;
		int upgrade_research_time;
		tech_type_t* tech_type;
		upgrade_type_t* upgrade_type;
		int larva_timer;
		int landing_timer;
		int creep_timer;
		int upgrade_level;
		target_t rally;
		union {
			struct {
				int resource_count;
				int resource_iscript;
				bool is_being_gathered;
				intrusive_list<unit_t, worker_gather_link> gather_queue;
			} resource;
			struct {
				unit_t* exit;
			} nydus;
			struct {
				std::pair<unit_t*, unit_t*> psi_link;
			} pylon;
			struct {
				unit_t* nuke;
				bool ready;
			} silo;
			struct {
				xy origin;
			} powerup;
			struct {
				std::array<int, 4> larva_spawn_side_values;
			} hatchery;
		};
	} building;

	int status_flags;
	int carrying_flags;
	int wireframe_randomizer;
	int secondary_order_state;
	int recent_order_timer;
	uint32_t detected_flags;
	int secondary_order_unk_a;
	int secondary_order_unk_b;
	unit_t* current_build_unit;
	std::pair<unit_t*, unit_t*> burrowed_unit_link;

	path_t* path;
	int pathing_collision_counter;
	int pathing_flags;
	int unused_0x106;
	bool is_being_healed;
	rect terrain_no_collision_bounds;

	int remove_timer;
	int defensive_matrix_hp;
	int defensive_matrix_timer;
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
	int air_strength;
	int ground_strength;
	int repulse_flags;
	direction_t repulse_direction;
	size_t repulse_index;

	rect unit_finder_bounding_box;
	bool unit_finder_visited;
	size_t unit_finder_index_from;
	size_t unit_finder_index_to;
};

}

#endif
