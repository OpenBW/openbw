#ifndef BWGAME_GAME_TYPES_H
#define BWGAME_GAME_TYPES_H

#include "util.h"
#include "data_types.h"
#include "containers.h"

namespace bwgame {

struct sprite_t;
struct flingy_t;
struct unit_t;

template<typename T>
struct unit_id_t {
	T raw_value = 0;
	unit_id_t() = default;
	explicit unit_id_t(T raw_value) : raw_value(raw_value) {}
	explicit unit_id_t(size_t index, unsigned int generation) : raw_value((T)(index | generation << 11)) {}
	bool operator==(const unit_id_t& n) const {
		return raw_value == n.raw_value;
	}
	size_t index() const {
		return raw_value & 0x7ff;
	}
	unsigned int generation() const {
		return raw_value >> 11;
	}
};

using unit_id = unit_id_t<uint16_t>;
using unit_id_32 = unit_id_t<uint32_t>;

struct default_link_f {
	template<typename T>
	auto* operator()(T* ptr) {
		return (std::pair<T*, T*>*)&ptr->link;
	}
};

template<typename cont_T, typename T>
static void bw_insert_list(cont_T& cont, T& v) {
	if (cont.empty()) cont.push_front(v);
	else cont.insert(std::next(cont.begin()), v);
}

template<typename T, size_t max_size, size_t allocation_granularity>
struct object_container {
	a_deque<std::array<T, allocation_granularity>> list;
	intrusive_list<T, default_link_f> free_list;
	size_t size = 0;
	
	T* get(size_t index, bool add_new_to_free = true) {
		if (index) index = max_size - index;
		while (size <= index) grow(add_new_to_free);
		return &list[index / allocation_granularity][index % allocation_granularity];
	}
	
	T* try_get(size_t index) {
		if (index) index = max_size - index;
		if (size <= index) return nullptr;
		return &list[index / allocation_granularity][index % allocation_granularity];
	}
	
	T* at(size_t index) {
		if (index) index = max_size - index;
		if (size <= index) error("object_container::get const: invalid index %u", index);
		return &list[index / allocation_granularity][index % allocation_granularity];
	}
	
	const T* at(size_t index) const {
		if (index) index = max_size - index;
		if (size <= index) error("object_container::get const: invalid index %u", index);
		return &list[index / allocation_granularity][index % allocation_granularity];
	}
	
	void grow(bool add_new_to_free) {
		if (size == max_size) error("object_container: attempt to grow beyond max_size");
		list.emplace_back();
		size_t n = std::min(allocation_granularity, max_size - size);
		for (size_t i = 0; i != n; ++i) {
			T* obj = &list.back()[i];
			obj->index = size == 0 ? 0 : max_size - size;
			if (add_new_to_free) free_list.push_back(*obj);
			++size;
		}
	}
	
	T* top() {
		if (free_list.empty()) {
			if (size == max_size) return nullptr;
			grow(true);
		} else if (size != max_size && std::next(free_list.begin()) == free_list.end()) {
			grow(true);
		}
		auto* r = &free_list.front();
		if (r != get(r->index)) error("index mismatch for %d\n", r->index);
		return r;
	}
	void pop() {
		free_list.pop_front();
	}
	void push(T* obj) {
		bw_insert_list(free_list, *obj);
	}
};

enum struct race_t {
	zerg,
	terran,
	protoss,
	none
};


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
	int controller = controller_inactive;
	race_t race{};
	int force = 0;
	int color = 0;
	
	bool initially_active = false;
	int victory_state = 0;
};

struct sight_values_t {
	struct maskdat_node_t {
		size_t prev;
		size_t prev2;
		int relative_tile_index;
		int x;
		int y;
	};
	int max_width, max_height;
	int min_width, min_height;
	int min_mask_size;
	int ext_masked_count;
	a_vector<maskdat_node_t> maskdat;
};

struct trigger {
	struct condition {
		int location;
		int group;
		int count_n;
		int unit_id;
		int num_n;
		int type;
		int extra_n;
		int flags;
		int unk;
	};
	struct action {
		int location;
		int string_index;
		int sound_index;
		int time_n;
		int group_n;
		int group2_n;
		int extra_n;
		int type;
		int num_n;
		int flags;
		int unk;
	};
	std::array<condition, 16> conditions;
	std::array<action, 64> actions;
	int execution_flags;
	std::array<bool, 28> enabled;
};

struct running_trigger {
	struct action {
		int flags = 0;
	};
	std::array<action, 64> actions;
	const trigger* t = nullptr;
	int flags = 0;
	size_t current_action_index = 0;
};

struct location {
	rect area;
	int elevation_flags;
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
	bool has_creep() const {
		return ((raw_value >> 4) & 0x8000) != 0;
	}
	size_t group_index() const {
		return (raw_value >> 4) & 0x7ff;
	}
	size_t subtile_index() const {
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
		flag_provides_cover = 0x10,
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
	uint8_t visible;
	uint8_t explored;
	uint16_t flags;
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

struct creep_life_t {
	int recede_timer = 0;
	int check_dead_unit_timer = 0;
	
	struct entry {
		std::pair<entry*, entry*> hash_link;
		std::pair<entry*, entry*> list_link;
		xy_t<size_t> tile_pos;
		size_t n_neighboring_creep_tiles = 0;
	};
	struct entry_hash_table {
		std::array<intrusive_list<entry, void, &entry::hash_link>, 0x200> buckets;
		entry* find(xy_t<size_t> tile_pos) {
			size_t index = (tile_pos.y * 7 + tile_pos.x) % buckets.size();
			for (auto& v : buckets[index]) {
				if (v.tile_pos == tile_pos) return &v;
			}
			return nullptr;
		}
		void remove(entry* v) {
			size_t index = (v->tile_pos.y * 7 + v->tile_pos.x) % buckets.size();
			buckets[index].remove(*v);
		}
		void insert(entry* v) {
			size_t index = (v->tile_pos.y * 7 + v->tile_pos.x) % buckets.size();
			buckets[index].push_front(*v);
		}
	};
	std::array<intrusive_list<entry, void, &entry::list_link>, 9> lists;
	std::array<size_t, 9> lists_size{};
	intrusive_list<entry, void, &entry::list_link> free_list;
	size_t free_list_size = 0;
	entry_hash_table table;
	
	a_vector<entry> entry_container = a_vector<entry>(1024);
	
	creep_life_t() {
		for (auto& v : entry_container) {
			free_list.push_back(v);
			++free_list_size;
		}
	}
	creep_life_t(creep_life_t&&) = default;
	creep_life_t(const creep_life_t& n) {
		*this = n;
	}
	creep_life_t& operator=(creep_life_t&&) = default;
	creep_life_t& operator=(const creep_life_t& n) {
		recede_timer = n.recede_timer;
		check_dead_unit_timer = n.check_dead_unit_timer;
		for (size_t i = 0; i != entry_container.size(); ++i) {
			entry_container[i].tile_pos = n.entry_container[i].tile_pos;
			entry_container[i].n_neighboring_creep_tiles = n.entry_container[i].n_neighboring_creep_tiles;
		}
		auto assemble = [&](auto& dst, auto& src) {
			dst.clear();
			for (auto& v : src) {
				dst.push_back(entry_container[&v - n.entry_container.data()]);
			}
		};
		for (size_t i = 0; i != lists.size(); ++i) {
			assemble(lists[i], n.lists[i]);
			lists_size[i] = n.lists_size[i];
		}
		assemble(free_list, n.free_list);
		free_list_size = n.free_list_size;
		for (size_t i = 0; i != table.buckets.size(); ++i) {
			assemble(table.buckets[i], n.table.buckets[i]);
		}
		return *this;
	}
};

struct target_t {
	xy pos;
	unit_t* unit = nullptr;
};

struct link_base {
	std::pair<link_base*, link_base*> link;
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
		flag_clickable = 0x20,
		flag_hidden = 0x40,
		flag_uses_special_offset = 0x80
	};
	
	size_t index;
	const image_type_t* image_type;
	int modifier;
	size_t frame_index_offset;
	int flags;
	xy offset;
	iscript_state_t iscript_state;
	size_t frame_index_base;
	size_t frame_index;
	const grp_t* grp;
	int modifier_data1;
	int modifier_data2;
	sprite_t* sprite;
	int frozen_y_value;

};

struct sprite_t: link_base {

	enum flags_t : uint_fast32_t {
		flag_selected = 0x8,
		flag_turret = 0x10,
		flag_hidden = 0x20,
		flag_burrowed = 0x40,
		flag_iscript_nobrk = 0x80,
	};

	size_t index;
	const sprite_type_t* sprite_type;
	int owner;
	int selection_index;
	int visibility_flags;
	int elevation_level;
	int flags;
	int selection_timer;
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

	size_t index;
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
	size_t index;
	int bullet_state;
	unit_t* bullet_target;
	xy bullet_target_pos;
	const weapon_type_t* weapon_type;
	int remaining_time;
	int hit_flags;
	int remaining_bounces;
	int owner;
	unit_t* bullet_owner_unit;
	unit_t* prev_bounce_unit;
	size_t hit_near_target_position_index;
};

struct order_target_t {
	xy position;
	unit_t* unit = nullptr;
	const unit_type_t* unit_type = nullptr;
	order_target_t() = default;
	order_target_t(unit_t* unit) : unit(unit) {}
	order_target_t(xy position, unit_t* unit) : position(position), unit(unit) {}
	order_target_t(xy position) : position(position) {}
};

struct order_t: link_base {
	size_t index;
	const order_type_t* order_type;
	order_target_t target;
};

struct path_t: link_base {

	int delay = 0;
	int creation_frame = 0;
	int state_flags = 0;

	a_circular_vector<const regions_t::region*> long_path;
	size_t full_long_path_size;
	a_circular_vector<xy> short_path;

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
		status_flag_in_bunker = 0x20,
		status_flag_loaded = 0x40,
		
		status_flag_requires_detector = 0x100,
		status_flag_cloaked = 0x200,
		status_flag_disabled = 0x400,
		status_flag_passively_cloaked = 0x800,
		status_flag_order_not_interruptible = 0x1000,
		status_flag_iscript_nobrk = 0x2000,
		status_flag_4000 = 0x4000,
		status_flag_cannot_attack = 0x8000,
		status_flag_can_turn = 0x10000,
		status_flag_can_move = 0x20000,
		status_flag_collision = 0x40000,
		status_flag_immovable = 0x80000,
		status_flag_ground_unit = 0x100000,
		status_flag_no_collide = 0x200000,
		status_flag_400000 = 0x400000,
		status_flag_gathering = 0x800000,
		status_flag_turret_walking = 0x1000000,
		
		status_flag_invincible = 0x4000000,
		status_flag_ready_to_attack = 0x8000000,

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
	int order_process_timer;
	int unknown_0x086;
	int attack_notify_timer;
	const unit_type_t* previous_unit_type;
	int last_event_timer;
	int last_event_color;
	int rank_increase;
	int kill_count;
	int last_attacking_player;
	int secondary_order_timer;
	int user_action_flags;
	int cloak_counter;
	int movement_state;
	static_vector<const unit_type_t*, 5> build_queue;
	fp8 energy;
	unsigned int unit_id_generation;
	const order_type_t* secondary_order_type;
	int damage_overlay_state;
	fp8 hp_construction_rate;
	fp8 shield_construction_rate;
	int remaining_build_time;
	int previous_hp;
	std::array<unit_id, 8> loaded_units;

	struct fighter_link {
		auto* operator()(unit_t* ptr) {
			return &ptr->fighter.fighter_link;
		}
		auto* operator()(const unit_t* ptr) {
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
			bool is_outside;
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
			thingy_t* nuke_dot;
		} ghost;
	};

	struct {
		unit_t* powerup;
		xy target_resource_position;
		unit_t* target_resource_unit;
		int repair_timer;
		bool is_gathering;
		int resources_carried;
		unit_t* gather_target;
		std::pair<unit_t*, unit_t*> gather_link;
	} worker;
	struct worker_gather_link {
		auto* operator()(unit_t* ptr) {
			return &ptr->worker.gather_link;
		}
		auto* operator()(const unit_t* ptr) {
			return &ptr->worker.gather_link;
		}
	};

	struct building_t {
		building_t() {}
		
		unit_t* addon;
		const unit_type_t* addon_build_type;
		int upgrade_research_time;
		const tech_type_t* researching_type;
		const upgrade_type_t* upgrading_type;
		int larva_timer;
		bool is_landing;
		int creep_timer;
		int upgrading_level;
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
				sprite_t* psi_field_sprite;
				std::pair<unit_t*, unit_t*> psionic_matrix_link;
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
	int move_target_timer;
	uint32_t detected_flags;
	unit_t* current_build_unit;
	std::pair<unit_t*, unit_t*> cloaked_unit_link;

	path_t* path;
	int pathing_collision_counter;
	int pathing_flags;
	int unused_0x106;
	bool is_being_healed;
	rect terrain_no_collision_bounds;

	int remove_timer;
	fp8 defensive_matrix_hp;
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
	int blinded_by;
	int maelstrom_timer;
	int acid_spore_count;
	std::array<int, 9> acid_spore_time;

	int next_hit_near_target_position_index;
	int air_strength;
	int ground_strength;
	int repulse_flags;
	direction_t repulse_direction;
	size_t repulse_index;

	rect unit_finder_bounding_box;
	std::array<bool, 4> unit_finder_visited;
	size_t unit_finder_index_from;
	size_t unit_finder_index_to;
};

}

#endif
