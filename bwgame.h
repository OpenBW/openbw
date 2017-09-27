#ifndef BWGAME_BWGAME_H
#define BWGAME_BWGAME_H

#include "util.h"
#include "data_types.h"
#include "game_types.h"
#include "data_loading.h"
#include "bwenums.h"
#include "korean.h"

#include <algorithm>
#include <utility>
#include <cstdlib>
#include <cmath>
#include <functional>

namespace bwgame {

static const std::array<unsigned int, 64> tan_table = {
	7, 13, 19, 26, 32, 38, 45, 51, 58, 65, 71, 78, 85, 92,
	99, 107, 114, 122, 129, 137, 146, 154, 163, 172, 181,
	190, 200, 211, 221, 233, 244, 256, 269, 283, 297, 312,
	329, 346, 364, 384, 405, 428, 452, 479, 509, 542, 578,
	619, 664, 716, 775, 844, 926, 1023, 1141, 1287, 1476,
	1726, 2076, 2600, 3471, 5211, 10429, std::numeric_limits<unsigned int>::max()
};

static const std::array<int, 16 * 2> repulse_adjust_table = {
	-5, -5, -5, 5, 5, -5, 5, 5, 5, -5, 5, 5, -5, -5, -5, 5, -5, 5, 5, -5, -5, -5, 5, 5, -5, 5, 5, -5, 5, 5, -5, -5
};

static const std::array<xy, 4> cardinal_direction_xy = {xy{1, 0}, xy{0, 1}, xy{-1, 0}, xy{0, -1}};

static const xy_fp8 direction_table[256] = {
	{0_fp8,-256_fp8},{6_fp8,-256_fp8},{13_fp8,-256_fp8},{19_fp8,-255_fp8},{25_fp8,-255_fp8},{31_fp8,-254_fp8},{38_fp8,-253_fp8},{44_fp8,-252_fp8},
	{50_fp8,-251_fp8},{56_fp8,-250_fp8},{62_fp8,-248_fp8},{68_fp8,-247_fp8},{74_fp8,-245_fp8},{80_fp8,-243_fp8},{86_fp8,-241_fp8},{92_fp8,-239_fp8},
	{98_fp8,-237_fp8},{104_fp8,-234_fp8},{109_fp8,-231_fp8},{115_fp8,-229_fp8},{121_fp8,-226_fp8},{126_fp8,-223_fp8},{132_fp8,-220_fp8},{137_fp8,-216_fp8},
	{142_fp8,-213_fp8},{147_fp8,-209_fp8},{152_fp8,-206_fp8},{157_fp8,-202_fp8},{162_fp8,-198_fp8},{167_fp8,-194_fp8},{172_fp8,-190_fp8},{177_fp8,-185_fp8},
	{181_fp8,-181_fp8},{185_fp8,-177_fp8},{190_fp8,-172_fp8},{194_fp8,-167_fp8},{198_fp8,-162_fp8},{202_fp8,-157_fp8},{206_fp8,-152_fp8},{209_fp8,-147_fp8},
	{213_fp8,-142_fp8},{216_fp8,-137_fp8},{220_fp8,-132_fp8},{223_fp8,-126_fp8},{226_fp8,-121_fp8},{229_fp8,-115_fp8},{231_fp8,-109_fp8},{234_fp8,-104_fp8},
	{237_fp8,-98_fp8},{239_fp8,-92_fp8},{241_fp8,-86_fp8},{243_fp8,-80_fp8},{245_fp8,-74_fp8},{247_fp8,-68_fp8},{248_fp8,-62_fp8},{250_fp8,-56_fp8},
	{251_fp8,-50_fp8},{252_fp8,-44_fp8},{253_fp8,-38_fp8},{254_fp8,-31_fp8},{255_fp8,-25_fp8},{255_fp8,-19_fp8},{256_fp8,-13_fp8},{256_fp8,-6_fp8},
	{256_fp8,0_fp8},{256_fp8,6_fp8},{256_fp8,13_fp8},{255_fp8,19_fp8},{255_fp8,25_fp8},{254_fp8,31_fp8},{253_fp8,38_fp8},{252_fp8,44_fp8},
	{251_fp8,50_fp8},{250_fp8,56_fp8},{248_fp8,62_fp8},{247_fp8,68_fp8},{245_fp8,74_fp8},{243_fp8,80_fp8},{241_fp8,86_fp8},{239_fp8,92_fp8},
	{237_fp8,98_fp8},{234_fp8,104_fp8},{231_fp8,109_fp8},{229_fp8,115_fp8},{226_fp8,121_fp8},{223_fp8,126_fp8},{220_fp8,132_fp8},{216_fp8,137_fp8},
	{213_fp8,142_fp8},{209_fp8,147_fp8},{206_fp8,152_fp8},{202_fp8,157_fp8},{198_fp8,162_fp8},{194_fp8,167_fp8},{190_fp8,172_fp8},{185_fp8,177_fp8},
	{181_fp8,181_fp8},{177_fp8,185_fp8},{172_fp8,190_fp8},{167_fp8,194_fp8},{162_fp8,198_fp8},{157_fp8,202_fp8},{152_fp8,206_fp8},{147_fp8,209_fp8},
	{142_fp8,213_fp8},{137_fp8,216_fp8},{132_fp8,220_fp8},{126_fp8,223_fp8},{121_fp8,226_fp8},{115_fp8,229_fp8},{109_fp8,231_fp8},{104_fp8,234_fp8},
	{98_fp8,237_fp8},{92_fp8,239_fp8},{86_fp8,241_fp8},{80_fp8,243_fp8},{74_fp8,245_fp8},{68_fp8,247_fp8},{62_fp8,248_fp8},{56_fp8,250_fp8},
	{50_fp8,251_fp8},{44_fp8,252_fp8},{38_fp8,253_fp8},{31_fp8,254_fp8},{25_fp8,255_fp8},{19_fp8,255_fp8},{13_fp8,256_fp8},{6_fp8,256_fp8},
	{0_fp8,256_fp8},{-6_fp8,256_fp8},{-13_fp8,256_fp8},{-19_fp8,255_fp8},{-25_fp8,255_fp8},{-31_fp8,254_fp8},{-38_fp8,253_fp8},{-44_fp8,252_fp8},
	{-50_fp8,251_fp8},{-56_fp8,250_fp8},{-62_fp8,248_fp8},{-68_fp8,247_fp8},{-74_fp8,245_fp8},{-80_fp8,243_fp8},{-86_fp8,241_fp8},{-92_fp8,239_fp8},
	{-98_fp8,237_fp8},{-104_fp8,234_fp8},{-109_fp8,231_fp8},{-115_fp8,229_fp8},{-121_fp8,226_fp8},{-126_fp8,223_fp8},{-132_fp8,220_fp8},{-137_fp8,216_fp8},
	{-142_fp8,213_fp8},{-147_fp8,209_fp8},{-152_fp8,206_fp8},{-157_fp8,202_fp8},{-162_fp8,198_fp8},{-167_fp8,194_fp8},{-172_fp8,190_fp8},{-177_fp8,185_fp8},
	{-181_fp8,181_fp8},{-185_fp8,177_fp8},{-190_fp8,172_fp8},{-194_fp8,167_fp8},{-198_fp8,162_fp8},{-202_fp8,157_fp8},{-206_fp8,152_fp8},{-209_fp8,147_fp8},
	{-213_fp8,142_fp8},{-216_fp8,137_fp8},{-220_fp8,132_fp8},{-223_fp8,126_fp8},{-226_fp8,121_fp8},{-229_fp8,115_fp8},{-231_fp8,109_fp8},{-234_fp8,104_fp8},
	{-237_fp8,98_fp8},{-239_fp8,92_fp8},{-241_fp8,86_fp8},{-243_fp8,80_fp8},{-245_fp8,74_fp8},{-247_fp8,68_fp8},{-248_fp8,62_fp8},{-250_fp8,56_fp8},
	{-251_fp8,50_fp8},{-252_fp8,44_fp8},{-253_fp8,38_fp8},{-254_fp8,31_fp8},{-255_fp8,25_fp8},{-255_fp8,19_fp8},{-256_fp8,13_fp8},{-256_fp8,6_fp8},
	{-256_fp8,0_fp8},{-256_fp8,-6_fp8},{-256_fp8,-13_fp8},{-255_fp8,-19_fp8},{-255_fp8,-25_fp8},{-254_fp8,-31_fp8},{-253_fp8,-38_fp8},{-252_fp8,-44_fp8},
	{-251_fp8,-50_fp8},{-250_fp8,-56_fp8},{-248_fp8,-62_fp8},{-247_fp8,-68_fp8},{-245_fp8,-74_fp8},{-243_fp8,-80_fp8},{-241_fp8,-86_fp8},{-239_fp8,-92_fp8},
	{-237_fp8,-98_fp8},{-234_fp8,-104_fp8},{-231_fp8,-109_fp8},{-229_fp8,-115_fp8},{-226_fp8,-121_fp8},{-223_fp8,-126_fp8},{-220_fp8,-132_fp8},{-216_fp8,-137_fp8},
	{-213_fp8,-142_fp8},{-209_fp8,-147_fp8},{-206_fp8,-152_fp8},{-202_fp8,-157_fp8},{-198_fp8,-162_fp8},{-194_fp8,-167_fp8},{-190_fp8,-172_fp8},{-185_fp8,-177_fp8},
	{-181_fp8,-181_fp8},{-177_fp8,-185_fp8},{-172_fp8,-190_fp8},{-167_fp8,-194_fp8},{-162_fp8,-198_fp8},{-157_fp8,-202_fp8},{-152_fp8,-206_fp8},{-147_fp8,-209_fp8},
	{-142_fp8,-213_fp8},{-137_fp8,-216_fp8},{-132_fp8,-220_fp8},{-126_fp8,-223_fp8},{-121_fp8,-226_fp8},{-115_fp8,-229_fp8},{-109_fp8,-231_fp8},{-104_fp8,-234_fp8},
	{-98_fp8,-237_fp8},{-92_fp8,-239_fp8},{-86_fp8,-241_fp8},{-80_fp8,-243_fp8},{-74_fp8,-245_fp8},{-68_fp8,-247_fp8},{-62_fp8,-248_fp8},{-56_fp8,-250_fp8},
	{-50_fp8,-251_fp8},{-44_fp8,-252_fp8},{-38_fp8,-253_fp8},{-31_fp8,-254_fp8},{-25_fp8,-255_fp8},{-19_fp8,-255_fp8},{-13_fp8,-256_fp8},{-6_fp8,-256_fp8}
};

static const xy hit_near_target_positions[14] = {
	{-1, -1}, {-51, -31}, {-7, -12}, {36, 52}, {49, -25}, {-40, 46}, {4, 19}, {-28, -50},
	{-50, 15}, {54, 13}, {-11, 53}, {-15, 9}, {19, -50}, {17, -8}
};

static const bool psi_field_mask[5][8] = {
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 0, 0 },
	{ 1, 1, 1, 0, 0, 0, 0, 0 }
};

static const std::array<int, 12> all_player_slots = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

template<typename T>
struct autocast {
	T val;
	operator T() {
		return val;
	}
	T operator->() {
		return val;
	}
	autocast(T val) : val(val) {}
	template<typename T2, typename std::enable_if<std::is_same<typename std::decay<T2>::type, unit_t>::value>::type* = nullptr>
	autocast(T2* ptr) : val(ptr->unit_type) {}
	template<typename T2>
	autocast(type_id<T2> ptr) : val(ptr) {}
};

using unit_type_autocast = autocast<const unit_type_t*>;

struct global_state {

	global_state() = default;
	global_state(global_state&) = delete;
	global_state(global_state&&) = default;
	global_state& operator=(global_state&) = delete;
	global_state& operator=(global_state&&) = default;

	flingy_types_t flingy_types;
	sprite_types_t sprite_types;
	image_types_t image_types;
	order_types_t order_types;
	iscript_t iscript;

	a_vector<grp_t> grps;
	a_vector<grp_t*> image_grp;
	a_vector<a_vector<a_vector<xy>>> lo_offsets;
	a_vector<std::array<a_vector<a_vector<xy>>*, 6>> image_lo_offsets;

	a_vector<uint8_t> units_dat;
	a_vector<uint8_t> weapons_dat;
	a_vector<uint8_t> upgrades_dat;
	a_vector<uint8_t> techdata_dat;

	a_vector<uint8_t> melee_trg;

	std::array<a_vector<uint8_t>, 8> tileset_vf4;
	std::array<a_vector<uint8_t>, 8> tileset_cv5;
};

struct game_state {

	game_state() = default;
	game_state(const game_state&) = delete;
	game_state(game_state&&) = default;
	game_state& operator=(const game_state&) = delete;
	game_state& operator=(game_state&&) = default;

	size_t map_tile_width;
	size_t map_tile_height;
	size_t map_walk_width;
	size_t map_walk_height;
	size_t map_width;
	size_t map_height;

	a_vector<a_string> map_strings;

	a_string scenario_name;
	a_string scenario_description;

	type_indexed_array<int, UnitTypes> unit_air_strength;
	type_indexed_array<int, UnitTypes> unit_ground_strength;

	struct force_t {
		a_string name;
		uint8_t flags;
	};
	std::array<force_t, 4> forces;

	std::array<sight_values_t, 12> sight_values;

	size_t tileset_index;

	a_vector<tile_id> gfx_tiles;
	a_vector<cv5_entry> cv5;
	a_vector<vf4_entry> vf4;
	a_vector<uint16_t> mega_tile_flags;

	unit_types_t unit_types;
	weapon_types_t weapon_types;
	upgrade_types_t upgrade_types;
	tech_types_t tech_types;

	std::array<type_indexed_array<bool, UnitTypes>, 12> unit_type_allowed;
	std::array<type_indexed_array<int, UpgradeTypes>, 12> max_upgrade_levels;
	std::array<type_indexed_array<bool, TechTypes>, 12> tech_available;

	std::array<xy, 12> start_locations;

	int max_unit_width;
	int max_unit_height;

	size_t repulse_field_width;
	size_t repulse_field_height;

	regions_t regions;

	a_vector<trigger> triggers;
};

struct state_base_copyable {

	const global_state* global;
	game_state* game;

	int update_tiles_countdown;

	int order_timer_counter;
	int secondary_order_timer_counter;
	int current_frame;

	std::array<player_t, 12> players;

	std::array<std::array<int, 12>, 12> alliances;

	std::array<type_indexed_array<int, UpgradeTypes>, 12> upgrade_levels;
	std::array<type_indexed_array<bool, UpgradeTypes>, 12> upgrade_upgrading;
	std::array<type_indexed_array<bool, TechTypes>, 12> tech_researched;
	std::array<type_indexed_array<bool, TechTypes>, 12> tech_researching;

	std::array<type_indexed_array<int, UnitTypes>, 12> unit_counts;
	std::array<type_indexed_array<int, UnitTypes>, 12> completed_unit_counts;

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

	std::array<std::array<fp1, 3>, 12> supply_used;
	std::array<std::array<fp1, 3>, 12> supply_available;

	std::array<uint32_t, 12> shared_vision;

	a_vector<tile_t> tiles;
	a_vector<uint16_t> tiles_mega_tile_index;

	std::array<int, 0x100> random_counts;
	int total_random_counts;
	uint32_t lcg_rand_state;

	int last_error;

	int trigger_timer;
	std::array<a_vector<running_trigger>, 8> running_triggers;
	std::array<int, 12> trigger_wait_timers;
	std::array<bool, 12> trigger_waiting;

	size_t active_orders_size;
	size_t active_bullets_size;
	size_t active_thingies_size;

	a_vector<uint8_t> repulse_field;

	bool prev_bullet_heading_offset_clockwise;

	std::array<int, 12> current_minerals;
	std::array<int, 12> current_gas;
	std::array<int, 12> total_minerals_gathered;
	std::array<int, 12> total_gas_gathered;

	std::array<static_vector<std::pair<size_t, size_t>, 16>, 32> recent_lurker_hits;
	size_t recent_lurker_hit_current_index;

	creep_life_t creep_life;
	bool update_psionic_matrix;
	int disruption_webbed_units;
	bool cheats_enabled;
	bool cheat_operation_cwal;

	a_vector<location> locations;
};

struct psionic_matrix_link_f {
	auto* operator()(unit_t* ptr) {
		return &ptr->building.pylon.psionic_matrix_link;
	}
	auto* operator()(const unit_t* ptr) {
		return &ptr->building.pylon.psionic_matrix_link;
	}
};

struct state_base_non_copyable {

	state_base_non_copyable() = default;
	state_base_non_copyable(const state_base_non_copyable&) = delete;
	state_base_non_copyable(state_base_non_copyable&&) = default;
	state_base_non_copyable& operator=(const state_base_non_copyable&) = delete;
	state_base_non_copyable& operator=(state_base_non_copyable&&) = default;

	intrusive_list<unit_t, default_link_f> visible_units;
	intrusive_list<unit_t, default_link_f> hidden_units;
	intrusive_list<unit_t, default_link_f> map_revealer_units;
	intrusive_list<unit_t, default_link_f> dead_units;

	std::array<intrusive_list<unit_t, void, &unit_t::player_units_link>, 12> player_units;
	intrusive_list<unit_t, void, &unit_t::cloaked_unit_link> cloaked_units;
	intrusive_list<unit_t, psionic_matrix_link_f> psionic_matrix_units;

	object_container<unit_t, 1700, 17> units_container;

	intrusive_list<bullet_t, default_link_f> active_bullets;
	object_container<bullet_t, 100, 10> bullets_container;

	a_vector<intrusive_list<sprite_t, default_link_f>> sprites_on_tile_line;
	object_container<sprite_t, 2500, 25> sprites_container;

	object_container<image_t, 5000, 50> images_container;

	object_container<order_t, 2000, 20> orders_container;

	intrusive_list<path_t, default_link_f> free_paths;
	a_list<path_t> paths;

	intrusive_list<thingy_t, default_link_f> active_thingies;
	intrusive_list<thingy_t, default_link_f> free_thingies;
	a_list<thingy_t> thingies;

	struct unit_finder_entry {
		unit_t* u;
		int value;
	};
	a_vector<unit_finder_entry> unit_finder_x;
	a_vector<unit_finder_entry> unit_finder_y;

	const unit_t* consider_collision_with_unit_bug;
	const unit_t* prev_bullet_source_unit;
};

struct state : state_base_copyable, state_base_non_copyable {
};

struct state_functions {

	virtual void play_sound(int id, xy position, const unit_t* source_unit = nullptr, bool add_race_index = false) {}
	virtual void on_unit_deselect(unit_t* u) {}

	virtual void on_unit_destroy(unit_t* u) {}
	virtual void on_kill_unit(unit_t* u) {}

	virtual void on_player_eliminated(int owner) {}
	virtual void on_victory_state(int owner, int state) {}

	virtual ~state_functions() {}

	state& st;
	const global_state& global_st = *st.global;
	const game_state& game_st = *st.game;

	explicit state_functions(state& st) : st(st) {}
	state_functions(state_functions&& n) : st(n.st) {}
	state_functions(const state_functions& n) : st(n.st) {}

	bool update_tiles = false;
	flingy_t* iscript_flingy = nullptr;
	bullet_t* iscript_bullet = nullptr;
	unit_t* iscript_unit = nullptr;
	mutable size_t unit_finder_search_index = 0;

	const order_type_t* get_order_type(Orders id) const {
		if ((size_t)id >= 189) error("invalid order id %d", (size_t)id);
		return &global_st.order_types.vec[(size_t)id];
	}

	template<typename T>
	struct thingy_setter {
		T& thingy;
		T prev_thingy;
		thingy_setter(T& thingy, T new_thingy) : thingy(thingy) {
			prev_thingy = std::move(thingy);
			thingy = std::move(new_thingy);
		}
		~thingy_setter() {
			thingy = std::move(prev_thingy);
		}
	};
	template<typename T, typename T2>
	auto make_thingy_setter(T& thingy, T2&& new_thingy) {
		return thingy_setter<T>(thingy, std::forward<T2>(new_thingy));
	}

	void u_set_status_flag(unit_t* u, unit_t::status_flags_t flag) {
		u->status_flags |= flag;
	}
	void u_unset_status_flag(unit_t* u, unit_t::status_flags_t flag) {
		u->status_flags &= ~flag;
	}

	void u_set_status_flag(unit_t* u, unit_t::status_flags_t flag, bool value) {
		if (value) u->status_flags |= flag;
		else u->status_flags &= ~flag;
	}

	void u_set_movement_flag(flingy_t* u, int flag) {
		u->movement_flags |= flag;
	}
	void u_unset_movement_flag(flingy_t* u, int flag) {
		u->movement_flags &= ~flag;
	}

	void i_set_flag(image_t* i, image_t::flags_t flag) {
		i->flags |= flag;
	}
	void i_unset_flag(image_t* i, image_t::flags_t flag) {
		i->flags &= ~flag;
	}
	void i_set_flag(image_t* i, image_t::flags_t flag, bool value) {
		if (value) i_set_flag(i, flag);
		else i_unset_flag(i, flag);
	}

	bool ut_flag(unit_type_autocast ut, unit_type_t::flags_t flag) const {
		return (ut->flags & flag) != 0;
	}
	bool u_status_flag(const unit_t* u, unit_t::status_flags_t flag) const {
		return (u->status_flags & flag) != 0;
	}
	bool us_flag(const thingy_t* u, sprite_t::flags_t flag) const {
		return (u->sprite->flags & flag) != 0;
	}
	bool s_flag(const sprite_t* s, sprite_t::flags_t flag) const {
		return (s->flags & flag) != 0;
	}
	bool u_movement_flag(const flingy_t* u, int flag) const {
		return (u->movement_flags & flag) != 0;
	}
	bool i_flag(const image_t* i, image_t::flags_t flag) const {
		return (i->flags & flag) != 0;
	}

	bool u_completed(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_completed);
	}
	bool u_in_bunker(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_in_bunker);
	}
	bool u_loaded(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_loaded);
	}
	bool u_immovable(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_immovable);
	}
	bool u_disabled(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_disabled);
	}
	bool u_burrowed(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_burrowed);
	}
	bool u_can_turn(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_can_turn);
	}
	bool u_can_move(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_can_move);
	}
	bool u_grounded_building(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_grounded_building);
	}
	bool u_hallucination(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_hallucination);
	}
	bool u_lifetime_expired(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_lifetime_expired);
	}
	bool u_flying(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_flying);
	}
	bool u_speed_upgrade(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_speed_upgrade);
	}
	bool u_cooldown_upgrade(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_cooldown_upgrade);
	}
	bool u_gathering(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_gathering);
	}
	bool u_requires_detector(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_requires_detector);
	}
	bool u_cloaked(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_cloaked);
	}
	bool u_cannot_attack(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_cannot_attack);
	}
	bool u_order_not_interruptible(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_order_not_interruptible);
	}
	bool u_iscript_nobrk(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_iscript_nobrk);
	}
	bool u_collision(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_collision);
	}
	bool u_ground_unit(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_ground_unit);
	}
	bool u_no_collide(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_no_collide);
	}
	bool u_invincible(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_invincible);
	}
	bool u_ready_to_attack(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_ready_to_attack);
	}
	bool u_passively_cloaked(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_passively_cloaked);
	}

	bool ut_turret(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_turret);
	}
	bool ut_worker(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_worker);
	}
	bool ut_hero(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_hero);
	}
	bool ut_building(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_building);
	}
	bool ut_addon(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_addon);
	}
	bool ut_flyer(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_flyer);
	}
	bool ut_can_turn(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_can_turn);
	}
	bool ut_can_move(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_can_move);
	}
	bool ut_invincible(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_invincible);
	}
	bool ut_two_units_in_one_egg(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_two_units_in_one_egg);
	}
	bool ut_regens_hp(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_regens_hp);
	}
	bool ut_flying_building(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_flying_building);
	}
	bool ut_requires_psionic_matrix(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_requires_psionic_matrix);
	}
	bool ut_can_burrow(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_can_burrow);
	}
	bool ut_has_energy(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_has_energy);
	}
	bool ut_resource_depot(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_resource_depot);
	}
	bool ut_resource(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_resource);
	}
	bool ut_initially_cloaked(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_initially_cloaked);
	}
	bool ut_detector(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_detector);
	}
	bool ut_requires_creep(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_requires_creep);
	}
	bool ut_mechanical(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_mechanical);
	}
	bool ut_organic(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_organic);
	}
	bool ut_robotic(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_robotic);
	}
	bool ut_powerup(unit_type_autocast ut) const {
		return ut_flag(ut, unit_type_t::flag_powerup);
	}

	bool us_hidden(const thingy_t* u) const {
		return us_flag(u, sprite_t::flag_hidden);
	}
	bool s_hidden(const sprite_t* u) const {
		return s_flag(u, sprite_t::flag_hidden);
	}

	const unit_type_t* get_unit_type(UnitTypes id) const {
		if ((size_t)id >= 228) error("invalid unit id %d", (size_t)id);
		return &game_st.unit_types.vec[(size_t)id];
	}
	const image_type_t* get_image_type(ImageTypes id) const {
		if ((size_t)id >= 999) error("invalid image id %d", (size_t)id);
		return &global_st.image_types.vec[(size_t)id];
	}
	const weapon_type_t* get_weapon_type(WeaponTypes id) const {
		if ((size_t)id >= 130) error("invalid weapon id %d", (size_t)id);
		return &game_st.weapon_types.vec[(size_t)id];
	}
	const sprite_type_t* get_sprite_type(SpriteTypes id) const {
		if ((size_t)id >= 517) error("invalid sprite id %d", (size_t)id);
		return &global_st.sprite_types.vec[(size_t)id];
	}
	const upgrade_type_t* get_upgrade_type(UpgradeTypes id) const {
		if ((size_t)id >= 61) error("invalid upgrade id %d", (size_t)id);
		return &game_st.upgrade_types.vec[(size_t)id];
	}
	const tech_type_t* get_tech_type(TechTypes id) const {
		if ((size_t)id >= 44) error("invalid tech id %d", (size_t)id);
		return &game_st.tech_types.vec[(size_t)id];
	}
	const flingy_type_t* get_flingy_type(FlingyTypes id) const {
		if ((size_t)id >= 209) error("invalid flingy id %d", (size_t)id);
		return &global_st.flingy_types.vec[(size_t)id];
	}

	void play_sound(int id, const unit_t* source_unit, bool add_race_index = false) {
		play_sound(id, source_unit ? source_unit->sprite->position : xy(), source_unit, add_race_index);
	}

	void play_sound(int id, bool add_race_index = false) {
		play_sound(id, xy(), nullptr, add_race_index);
	}

	unit_t* get_unit(size_t index) const {
		unit_t* u = st.units_container.try_get(index);
		if (!u) return nullptr;
		if (unit_dead(u)) return nullptr;
		return u;
	}

	template<typename T>
	unit_t* get_unit(unit_id_t<T> id) const {
		size_t idx = id.index();
		if (!idx) return nullptr;
		unit_t* u = get_unit(idx - 1);
		if (!u) return nullptr;
		if (u->unit_id_generation % (1u << (int_bits<T>::value - 11)) != id.generation()) return nullptr;
		return u;
	}

	unit_id get_unit_id(const unit_t* u) const {
		if (!u) return unit_id{};
		return unit_id(u->index + 1, u->unit_id_generation % (1u << 5));
	}

	unit_id_32 get_unit_id_32(const unit_t* u) const {
		if (!u) return unit_id_32{};
		return unit_id_32(u->index + 1, u->unit_id_generation % (1u << 21));
	}

	bool is_in_map_bounds(const unit_type_t* unit_type, xy pos) const {
		if (pos.x - unit_type->dimensions.from.x < 0) return false;
		if (pos.y - unit_type->dimensions.from.y < 0) return false;
		if ((size_t)(pos.x + unit_type->dimensions.to.x) >= game_st.map_width) return false;
		if ((size_t)(pos.y + unit_type->dimensions.to.y) >= game_st.map_height) return false;
		return true;
	}
	bool is_in_map_bounds(rect area) const {
		if (area.from.x < 0) return false;
		if ((size_t)area.to.x > game_st.map_width) return false;
		if (area.from.y < 0) return false;
		if ((size_t)area.to.y > game_st.map_height) return false;
		return true;
	}
	bool is_in_inner_map_bounds(rect area) const {
		if (area.from.x < 0) return false;
		if ((size_t)area.to.x >= game_st.map_width) return false;
		if (area.from.y < 0) return false;
		if ((size_t)area.to.y >= game_st.map_height) return false;
		return true;
	}
	bool is_in_map_bounds(xy pos) const {
		return (size_t)pos.x < game_st.map_width && (size_t)pos.y < game_st.map_height;
	}

	bool is_in_bounds(rect area, rect bounds) const {
		if (area.from.x < bounds.from.x) return false;
		if (area.to.x > bounds.to.x) return false;
		if (area.from.y < bounds.from.y) return false;
		if (area.to.y > bounds.to.y) return false;
		return true;
	}

	bool is_in_inner_bounds(rect area, rect bounds) const {
		if (area.from.x < bounds.from.x) return false;
		if (area.to.x >= bounds.to.x) return false;
		if (area.from.y < bounds.from.y) return false;
		if (area.to.y >= bounds.to.y) return false;
		return true;
	}

	bool is_in_bounds(xy pos, rect bounds) const {
		return pos.x >= bounds.from.x && pos.y >= bounds.from.y && pos.x < bounds.to.x && pos.y < bounds.to.y;
	}

	bool is_in_inner_bounds(xy pos, rect bounds) const {
		return pos.x >= bounds.from.x && pos.y >= bounds.from.y && pos.x <= bounds.to.x && pos.y <= bounds.to.y;
	}

	rect translate_rect(rect src, xy translation) const {
		return {src.from + translation, src.to + translation};
	}

	rect square_at(xy pos, int half_width) const {
		return {pos - xy(half_width, half_width), pos + xy(half_width, half_width)};
	}

	rect unit_type_bounding_box(const unit_type_t* unit_type) const {
		return {-unit_type->dimensions.from, unit_type->dimensions.to + xy(1, 1)};
	}
	rect unit_type_bounding_box(const unit_type_t* unit_type, xy origin) const {
		return {origin - unit_type->dimensions.from, origin + unit_type->dimensions.to + xy(1, 1)};
	}
	rect unit_bounding_box(const unit_t* u, xy origin) const {
		return unit_type_bounding_box(u->unit_type, origin);
	}

	rect unit_sprite_bounding_box(const unit_t* u) const {
		return {u->sprite->position - u->unit_type->dimensions.from, u->sprite->position + u->unit_type->dimensions.to + xy(1, 1)};
	}

	rect unit_type_inner_bounding_box(const unit_type_t* unit_type) const {
		return {-unit_type->dimensions.from, unit_type->dimensions.to};
	}
	rect unit_type_inner_bounding_box(const unit_type_t* unit_type, xy origin) const {
		return {origin - unit_type->dimensions.from, origin + unit_type->dimensions.to};
	}
	rect unit_inner_bounding_box(const unit_t* u, xy origin) const {
		return unit_type_inner_bounding_box(u->unit_type, origin);
	}

	rect unit_sprite_inner_bounding_box(const unit_t* u) const {
		return {u->sprite->position - u->unit_type->dimensions.from, u->sprite->position + u->unit_type->dimensions.to};
	}

	rect map_bounds() const {
		return { {0, 0}, {(int)game_st.map_width, (int)game_st.map_height} };
	}

	xy restrict_unit_pos_to_bounds(xy pos, const unit_type_t* ut, rect bounds) const {
		rect bb = unit_type_bounding_box(ut, pos);
		if (bb.from.x < bounds.from.x) pos.x -= bb.from.x - bounds.from.x;
		else if (bb.to.x >= bounds.to.x) pos.x -= bb.to.x - bounds.to.x;
		if (bb.from.y < bounds.from.y) pos.y -= bb.from.y - bounds.from.y;
		else if (bb.to.y >= bounds.to.y) pos.y -= bb.to.y - bounds.to.y;
		return pos;
	}

	xy restrict_pos_to_map_bounds(xy pos) const {
		if (pos.x < 0) pos.x = 0;
		else if (pos.x >= (int)game_st.map_width) pos.x = (int)game_st.map_width - 1;
		if (pos.y < 0) pos.y = 0;
		else if (pos.y >= (int)game_st.map_height) pos.y = (int)game_st.map_height - 1;
		return pos;
	}

	bool is_inner_bb_move_target_in_valid_bounds(rect bb) const {
		return is_in_inner_bounds(bb, map_bounds() + rect { { 0, 0 }, { 0, -32 } });
	}

	xy restrict_move_target_to_valid_bounds(unit_type_autocast ut, xy move_target) const {
		return restrict_unit_pos_to_bounds(move_target, ut, map_bounds() + rect { { 0, 0 }, { 0, -32 } });
	}

	bool is_walkable(xy pos) const {
		size_t index = tile_index(pos);
		auto& tile = st.tiles[index];
		if (tile.flags & tile_t::flag_has_creep) return true;
		if (tile.flags & tile_t::flag_partially_walkable) {
			size_t ux = pos.x;
			size_t uy = pos.y;
			size_t megatile_index = st.tiles_mega_tile_index[index];
			int flags = game_st.vf4.at(megatile_index).flags[uy / 8 % 4 * 4 + ux / 8 % 4];
			return flags & vf4_entry::flag_walkable;
		}
		if (tile.flags & tile_t::flag_walkable) return true;
		return false;
	}

	void tiles_flags_and(size_t offset_x, size_t offset_y, size_t width, size_t height, int flags) {
		if (offset_x >= game_st.map_tile_width) error("attempt to mask tile out of bounds");
		if (offset_y >= game_st.map_tile_height) error("attempt to mask tile out of bounds");
		if (width > game_st.map_tile_width || offset_x + width > game_st.map_tile_width) error("attempt to mask tile out of bounds");
		if (height > game_st.map_tile_height || offset_y + height > game_st.map_tile_height) error("attempt to mask tile out of bounds");
		for (size_t y = offset_y; y != offset_y + height; ++y) {
			for (size_t x = offset_x; x != offset_x + width; ++x) {
				st.tiles[x + y * game_st.map_tile_width].flags &= flags;
			}
		}
	}
	void tiles_flags_or(size_t offset_x, size_t offset_y, size_t width, size_t height, int flags) {
		if (offset_x >= game_st.map_tile_width) error("attempt to mask tile out of bounds");
		if (offset_y >= game_st.map_tile_height) error("attempt to mask tile out of bounds");
		if (width > game_st.map_tile_width || offset_x + width > game_st.map_tile_width) error("attempt to mask tile out of bounds");
		if (height > game_st.map_tile_height || offset_y + height > game_st.map_tile_height) error("attempt to mask tile out of bounds");
		for (size_t y = offset_y; y != offset_y + height; ++y) {
			for (size_t x = offset_x; x != offset_x + width; ++x) {
				st.tiles[x + y * game_st.map_tile_width].flags |= flags;
			}
		}
	}

	bool unit_type_spreads_creep(unit_type_autocast ut, bool unit_is_completed = true) const {
		if (unit_is(ut, UnitTypes::Zerg_Hatchery) && unit_is_completed) return true;
		if (unit_is(ut, UnitTypes::Zerg_Lair)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Hive)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Creep_Colony) && unit_is_completed) return true;
		if (unit_is(ut, UnitTypes::Zerg_Spore_Colony)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Sunken_Colony)) return true;
		return false;
	}

	int visible_hp_plus_shields(const unit_t* u) const {
		int r = 0;
		if (u->unit_type->has_shield) r += u->shield_points.integer_part();
		r += u->hp.ceil().integer_part();
		return r;
	}
	int max_visible_hp(const unit_t* u) const {
		int hp = u->unit_type->hitpoints.integer_part();
		if (hp == 0) hp = u->hp.ceil().integer_part();
		if (hp == 0) hp = 1;
		return hp;
	}
	int max_visible_hp_plus_shields(const unit_t* u) const {
		int shields = 0;
		if (u->unit_type->has_shield) shields += u->unit_type->shield_points;
		return max_visible_hp(u) + shields;
	}

	auto loaded_units(const unit_t* u) const {
		return make_filter_range(make_transform_range(u->loaded_units, [this](auto uid) {
			return this->get_unit(uid);
		}), [](unit_t* u) {
			return u != nullptr;
		});
	}

	size_t unit_space_occupied(const unit_t* u) const {
		size_t r = 0;
		for (const unit_t* nu : loaded_units(u)) {
			r += nu->unit_type->space_required;
		}
		return r;
	}

	int get_unit_strength(unit_t* u, bool ground) const {
		if (unit_is(u, UnitTypes::Zerg_Larva)) return 0;
		if (unit_is_egg(u)) return 0;
		int vis_hp_shields = visible_hp_plus_shields(u);
		int max_vis_hp_shields = max_visible_hp_plus_shields(u);
		if (u_hallucination(u)) {
			if (vis_hp_shields < max_vis_hp_shields) return 0;
		}

		int r = ground ? game_st.unit_ground_strength[u->unit_type->id] : game_st.unit_air_strength[u->unit_type->id];
		if (unit_is(u, UnitTypes::Terran_Bunker)) {
			r = ground ? game_st.unit_ground_strength[UnitTypes::Terran_Marine] : game_st.unit_air_strength[UnitTypes::Terran_Marine];
			r *= (int)unit_space_occupied(u);
		}
		if (ut_has_energy(u) && !u_hallucination(u)) {
			r += u->energy.integer_part() / 2;
		}
		return r * vis_hp_shields / max_vis_hp_shields;
	}

	void update_unit_damage_overlay(unit_t* u) {
		if (u->sprite->main_image->image_type->damage_filename_index == 0) return;
		int states = damage_overlay_states(u);
		fp8 max_hp = u->unit_type->hitpoints;
		fp8 two_thirds_max_hp = max_hp - max_hp / 3;
		fp8 hp_per_state = two_thirds_max_hp / (states + 1);
		if (hp_per_state == 0_fp8) hp_per_state = 1_fp8;
		int damage_state = (u->hp.raw_value - two_thirds_max_hp.raw_value % hp_per_state.raw_value - max_hp.raw_value / 3 - 1) / hp_per_state.raw_value;
		if (damage_state < 0) damage_state = 0;
		for (; u->damage_overlay_state < damage_state; ++u->damage_overlay_state) {
			auto* large_flames = find_image(u->sprite, ImageTypes::IMAGEID_Flames1_Type1_Large, ImageTypes::IMAGEID_Flames8_Type3_Large);
			if (large_flames) {
				size_t index = (size_t)large_flames->image_type->id - (size_t)ImageTypes::IMAGEID_Flames1_Type1_Large;
				destroy_image(large_flames);
				xy offset = get_image_lo_offset(u->sprite->main_image, 1, index);
				auto* image_type = get_image_type(ImageTypes((size_t)ImageTypes::IMAGEID_Flames1_Type1_Small + index));
				create_image(image_type, u->sprite, offset, image_order_top);
			} else {
				destroy_image_from_to(u->sprite, ImageTypes::IMAGEID_Flames1_Type1_Small, ImageTypes::IMAGEID_Flames8_Type3_Small);
			}
		}
		for (; u->damage_overlay_state > damage_state; --u->damage_overlay_state) {
			auto* small_flames = find_image(u->sprite, ImageTypes::IMAGEID_Flames1_Type1_Small, ImageTypes::IMAGEID_Flames8_Type3_Small);
			if (small_flames) {
				size_t index = (size_t)small_flames->image_type->id - (size_t)ImageTypes::IMAGEID_Flames1_Type1_Small;
				destroy_image(small_flames);
				xy offset = get_image_lo_offset(u->sprite->main_image, 1, index);
				auto* image_type = get_image_type(ImageTypes((size_t)ImageTypes::IMAGEID_Flames1_Type1_Large + index));
				create_image(image_type, u->sprite, offset, image_order_top);
			} else {
				size_t index = lcg_rand(8) % (states / 2) + 1;
				size_t n = 0;
				auto& offsets = global_st.image_lo_offsets.at((size_t)u->sprite->main_image->image_type->id).at(1)->at(u->sprite->main_image->frame_index);
				for (size_t i = 0; i != offsets.size();) {
					xy offset = offsets[i];
					if (offset != xy(127, 127)) {
						++n;
						auto image_id = ImageTypes((size_t)ImageTypes::IMAGEID_Flames1_Type1_Large + i);
						auto* image = find_image(u->sprite, image_id, image_id);
						if (!image && n >= index) {
							auto new_image_id = ImageTypes((size_t)ImageTypes::IMAGEID_Flames1_Type1_Small + i);
							if (i_flag(u->sprite->main_image, image_t::flag_horizontally_flipped)) offset.x = -offset.x;
							create_image(get_image_type(new_image_id), u->sprite, offset, image_order_top);
							break;
						}
					}
					++i;
					if (i == offsets.size()) {
						i = 0;
						if (n == 0) error("missing damage overlay offset");
					}
				}

			}
		}
	}

	void set_unit_hp(unit_t* u, fp8 hitpoints) {
		u->hp = std::min(hitpoints, u->unit_type->hitpoints);
		if (u_completed(u)) {
			update_unit_damage_overlay(u);

			u->air_strength = get_unit_strength(u, false);
			u->ground_strength = get_unit_strength(u, true);
		}
	}

	void set_unit_shield_points(unit_t* u, fp8 shield_points) {
		u->shield_points = std::min(shield_points, fp8::integer(u->unit_type->shield_points));
	}

	void set_unit_energy(unit_t* u, fp8 energy) {
		u->energy = std::min(energy, unit_max_energy(u));
	}

	bool unit_is_mineral_field(unit_type_autocast ut) const {
		if (ut->id == UnitTypes::Resource_Mineral_Field) return true;
		if (ut->id == UnitTypes::Resource_Mineral_Field_Type_2) return true;
		if (ut->id == UnitTypes::Resource_Mineral_Field_Type_3) return true;
		return false;
	}

	void set_unit_resources(unit_t* u, int resources) {
		if (!ut_resource(u)) return;
		u->building.resource.resource_count = resources;
		if (unit_is_mineral_field(u)) {
			int anim = iscript_anims::WorkingToIdle;
			if (resources < 250) anim = iscript_anims::SpecialState1;
			else if (resources < 500) anim = iscript_anims::SpecialState2;
			else if (resources < 750) anim = iscript_anims::AlmostBuilt;
			if (u->building.resource.resource_iscript != anim) {
				u->building.resource.resource_iscript = anim;
				sprite_run_anim(u->sprite, anim);
			}
		}
	}

	bool unit_is_disabled(const unit_t* u) const {
		if (u_disabled(u)) return true;
		if (u->lockdown_timer) return true;
		if (u->stasis_timer) return true;
		if (u->maelstrom_timer) return true;
		return false;
	}

	image_t* find_image(sprite_t* sprite, ImageTypes first_id, ImageTypes last_id) const {
		for (image_t* i : ptr(sprite->images)) {
			if (i->image_type->id >= first_id && i->image_type->id <= last_id) return i;
		}
		return nullptr;
	}

	image_t* find_image(unit_t* u, ImageTypes first_id, ImageTypes last_id) const {
		image_t* r = find_image(u->sprite, first_id, last_id);
		if (!r && u->subunit) r = find_image(u->subunit->sprite, first_id, last_id);
		return r;
	}

	void destroy_image_from_to(sprite_t* sprite, ImageTypes first_id, ImageTypes last_id) {
		image_t* image = find_image(sprite, first_id, last_id);
		if (image) destroy_image(image);
	}

	void destroy_image_from_to(unit_t* u, ImageTypes first_id, ImageTypes last_id) {
		destroy_image_from_to(u->sprite, first_id, last_id);
		if (u->subunit) destroy_image_from_to(u->subunit->sprite, first_id, last_id);
	}

	void disable_effect_end(unit_t* u, ImageTypes first, ImageTypes last) {
		bool still_disabled = unit_is_disabled(u);
		if (u->subunit && !still_disabled) {
			u_unset_status_flag(u->subunit, unit_t::status_flag_disabled);
			set_unit_order(u->subunit, u->subunit->unit_type->return_to_idle);
		}
		image_t* image = find_image(u->sprite, first, last);
		if (!image && u->subunit) image = find_image(u->subunit->sprite, first, last);
		if (image) iscript_run_anim(image, iscript_anims::Death);
		if (ut_worker(u) && !still_disabled) {
			unit_t* target = u->worker.gather_target;
			if (target && ut_resource(target)) {
				if (u->worker.is_gathering && target->building.resource.is_being_gathered) {
					if (u->order_type->id == Orders::WaitForMinerals || u->order_type->id == Orders::WaitForGas) {
						if (u->order_state == 2) {
							target->building.resource.gather_queue.remove(*u);
							remove_one_order(u, get_order_type(Orders::GatherWaitInterrupted));
							try_gather_resource(u, target);
						}
					}
				}
			}
		}
		u->order_process_timer = 15;
	}

	void remove_stasis(unit_t* u) {
		u->stasis_timer = 0;
		u_set_status_flag(u, unit_t::status_flag_invincible, ut_invincible(u));
		disable_effect_end(u, ImageTypes::IMAGEID_Stasis_Field_Small, ImageTypes::IMAGEID_Stasis_Field_Large);
	}

	void deal_irradiate_damage(unit_t* source_unit) {
		auto damage = [&](unit_t* target) {
			if (!ut_organic(target)) return;
			if (ut_building(target)) return;
			if (unit_is(target, UnitTypes::Zerg_Larva)) return;
			if (unit_is(target, UnitTypes::Zerg_Egg)) return;
			if (unit_is(target, UnitTypes::Zerg_Lurker_Egg)) return;
			if (u_burrowed(target) && target != source_unit) return;
			if (!u_loaded(target)) {
				if (!unit_target_in_range(source_unit, target, 32)) return;
			}
			auto* w = get_weapon_type(WeaponTypes::Irradiate);
			weapon_deal_damage(w, fp8::integer(w->damage_amount) / w->cooldown, 1, target, 0_dir, source_unit->irradiated_by, source_unit->irradiate_owner);
		};
		if (u_burrowed(source_unit)) {
			damage(source_unit);
		} else if (u_loaded(source_unit)) {
			if (source_unit->connected_unit) damage(source_unit->connected_unit);
		} else {
			for (unit_t* n : find_units_noexpand(square_at(source_unit->sprite->position, 160))) {
				damage(n);
			}
		}
	}

	void remove_irradiate(unit_t* u) {
		u->irradiate_timer = 0;
		u->irradiated_by = 0;
		u->irradiate_owner = 8;
		destroy_image_from_to(u, ImageTypes::IMAGEID_Irradiate_Small, ImageTypes::IMAGEID_Irradiate_Large);
	}

	void remove_ensnare(unit_t* u) {
		u->ensnare_timer = 0;
		destroy_image_from_to(u, ImageTypes::IMAGEID_Ensnare_Overlay_Small, ImageTypes::IMAGEID_Ensnare_Overlay_Large);
		update_unit_speed(u);
	}

	void remove_lockdown(unit_t* u) {
		u->lockdown_timer = 0;
		disable_effect_end(u, ImageTypes::IMAGEID_Lockdown_Field_Small, ImageTypes::IMAGEID_Lockdown_Field_Large);
	}

	void remove_plague(unit_t* u) {
		u->plague_timer = 0;
		destroy_image_from_to(u, ImageTypes::IMAGEID_Plague_Overlay_Small, ImageTypes::IMAGEID_Plague_Overlay_Large);
	}

	void remove_maelstrom(unit_t* u) {
		u->maelstrom_timer = 0;
		disable_effect_end(u, ImageTypes::IMAGEID_Maelstorm_Overlay_Small, ImageTypes::IMAGEID_Maelstorm_Overlay_Large);
	}

	void remove_acid_spores(unit_t* u) {
		u->acid_spore_count = 0;
		u->acid_spore_time = {};
		destroy_image_from_to(u, ImageTypes::IMAGEID_Acid_Spores_1_Overlay_Small, ImageTypes::IMAGEID_Acid_Spores_6_9_Overlay_Large);
	}

	void update_unit_status_timers(unit_t* u) {
		if (u->stasis_timer) {
			--u->stasis_timer;
			if (!u->stasis_timer) {
				remove_stasis(u);
			}
		}
		if (u->stim_timer) {
			--u->stim_timer;
			if (!u->stim_timer) {
				update_unit_speed(u);
			}
		}
		if (u->ensnare_timer) {
			--u->ensnare_timer;
			if (!u->ensnare_timer) {
				remove_ensnare(u);
			}
		}
		if (u->defensive_matrix_timer) {
			--u->defensive_matrix_timer;
			if (!u->defensive_matrix_timer) {
				deal_defensive_matrix_damage(u, u->defensive_matrix_hp);
			}
		}
		if (u->irradiate_timer) {
			--u->irradiate_timer;
			deal_irradiate_damage(u);
			if (!u->irradiate_timer) {
				remove_irradiate(u);
			}
		}
		if (u->lockdown_timer) {
			--u->lockdown_timer;
			if (!u->lockdown_timer) {
				remove_lockdown(u);
			}
		}
		if (u->maelstrom_timer) {
			--u->maelstrom_timer;
			if (!u->maelstrom_timer) {
				remove_maelstrom(u);
			}
		}
		if (u->plague_timer) {
			--u->plague_timer;
			if (!u_invincible(u)) {
				auto damage = fp8::integer(get_weapon_type(WeaponTypes::Plague)->damage_amount) / 76;
				if (u->hp > damage) unit_deal_damage(u, damage, nullptr, ~0, true);
			}
			if (!u->plague_timer) {
				remove_plague(u);
			}
		}
		if (u->storm_timer) --u->storm_timer;
		int prev_acid_spore_count = u->acid_spore_count;
		if (prev_acid_spore_count) {
			for (auto& v : u->acid_spore_time) {
				if (!v) continue;
				--v;
				if (!v) --u->acid_spore_count;
			}
			if (u->acid_spore_count) {
				update_acid_spore_image(u);
			} else {
				remove_acid_spores(u);
			}
		}

	}

	fp8 unit_cloak_energy_cost(const unit_t* u) const {
		switch (u->unit_type->id) {
		case UnitTypes::Terran_Ghost:
		case UnitTypes::Hero_Sarah_Kerrigan:
		case UnitTypes::Hero_Alexei_Stukov:
		case UnitTypes::Hero_Samir_Duran:
		case UnitTypes::Hero_Infested_Duran:
		case UnitTypes::Hero_Infested_Kerrigan:
			return 10_fp8;
		case UnitTypes::Terran_Wraith:
		case UnitTypes::Hero_Tom_Kazansky:
			return 13_fp8;
		default:
			return 0_fp8;
		}
	}

	void set_secondary_order(unit_t* u, const order_type_t* order_type) {
		if (u->secondary_order_type == order_type) return;
		u->secondary_order_type = order_type;
		u->secondary_order_state = 0;
		u->current_build_unit = nullptr;
	}

	void update_unit_energy(unit_t* u) {
		if (!ut_has_energy(u)) return;
		if (u_hallucination(u)) return;
		if (!u_completed(u)) return;
		if ((u_cloaked(u) || u_requires_detector(u)) && !u_passively_cloaked(u)) {
			fp8 cost = unit_cloak_energy_cost(u);
			if (u->energy < cost) {
				if (u->secondary_order_type->id == Orders::Cloak) set_secondary_order(u, get_order_type(Orders::Nothing));
			} else {
				u->energy -= cost;
			}
		} else {
			fp8 max_energy = unit_max_energy(u);
			if (unit_is(u, UnitTypes::Protoss_Dark_Archon) && u->order_type->id == Orders::CompletingArchonSummon && u->order_state == 0) {
				max_energy = fp8::integer(50);
			}
			u->energy = std::min(u->energy + 8_fp8, max_energy);
		}
	}

	int unit_hp_percent(const unit_t* u) const {
		int max_hp = max_visible_hp(u);
		int hp = u->hp.ceil().integer_part();
		return hp * 100 / max_hp;
	}

	void update_unit_values(unit_t* u) {
		if (u->main_order_timer) --u->main_order_timer;
		if (u->ground_weapon_cooldown) --u->ground_weapon_cooldown;
		if (u->air_weapon_cooldown) --u->air_weapon_cooldown;
		if (u->spell_cooldown) --u->spell_cooldown;
		if (u->unit_type->has_shield) {
			fp8 max_shields = fp8::integer(u->unit_type->shield_points);
			if (u->shield_points != max_shields) {
				u->shield_points += 7_fp8;
				if (u->shield_points > max_shields) u->shield_points = max_shields;
			}
		}
		if (unit_is(u, UnitTypes::Zerg_Zergling) || unit_is(u, UnitTypes::Hero_Devouring_One)) {
			if (u->ground_weapon_cooldown == 0) u->order_process_timer = 0;
		}
		if (u->is_being_healed) u->is_being_healed = false;
		if (u_completed(u) || !us_hidden(u)) {
			++u->cycle_counter;
			if (u->cycle_counter >= 8) {
				u->cycle_counter = 0;
				update_unit_status_timers(u);
			}
		}
		if (u_completed(u)) {
			if (ut_regens_hp(u)) {
				if (u->hp > 0_fp8 && u->hp != u->unit_type->hitpoints) {
					set_unit_hp(u, u->hp + 4_fp8);
				}
			}
			update_unit_energy(u);
			if (u->move_target_timer) --u->move_target_timer;
			if (u->remove_timer != 0 && --u->remove_timer == 0) {
				kill_unit(u);
			} else {
				if (unit_race(u) == race_t::terran) {
					if (u_grounded_building(u) || ut_flying_building(u)) {
						if (unit_hp_percent(u) <= 33) {
							unit_deal_damage(u, 20_fp8, nullptr, u->last_attacking_player);
						}
					}
				}
			}
		}
	}

	unit_t* unit_turret(const unit_t* u) const {
		if (!u->subunit) return nullptr;
		if (!ut_turret(u->subunit)) return nullptr;
		return u->subunit;
	}

	const unit_t* unit_main_unit(const unit_t* u) const {
		return ut_turret(u) ? u->subunit : u;
	}

	const unit_t* unit_attacking_unit(const unit_t* u) const {
		return u->subunit && ut_turret(u->subunit) ? u->subunit : u;
	}

	unit_t* unit_main_unit(unit_t* u) const {
		return ut_turret(u) ? u->subunit : u;
	}

	unit_t* unit_attacking_unit(unit_t* u) const {
		return u->subunit && ut_turret(u->subunit) ? u->subunit : u;
	}

	const weapon_type_t* unit_ground_weapon(const unit_t* u) const {
		if (unit_is(u, UnitTypes::Zerg_Lurker) && !u_burrowed(u)) return nullptr;
		return u->unit_type->ground_weapon;
	}

	const weapon_type_t* unit_air_weapon(const unit_t* u) const {
		return u->unit_type->air_weapon;
	}

	const weapon_type_t* unit_or_subunit_ground_weapon(const unit_t* u) const {
		auto* w = unit_ground_weapon(u);
		if (w || !u->subunit) return w;
		return unit_ground_weapon(u->subunit);
	}

	const weapon_type_t* unit_or_subunit_air_weapon(const unit_t* u) const {
		auto* w = unit_air_weapon(u);
		if (w || !u->subunit) return w;
		return unit_air_weapon(u->subunit);
	}

	const weapon_type_t* unit_target_weapon(const unit_t* u, const unit_t* target) const {
		return u_flying(target) ? unit_air_weapon(unit_attacking_unit(u)) : unit_ground_weapon(unit_attacking_unit(u));
	}

	bool unit_is_carrier(unit_type_autocast ut) const {
		return unit_is(ut, UnitTypes::Protoss_Carrier) || unit_is(ut, UnitTypes::Hero_Gantrithor);
	}

	bool unit_is_reaver(unit_type_autocast ut) const {
		return unit_is(ut, UnitTypes::Protoss_Reaver) || unit_is(ut, UnitTypes::Hero_Warbringer);
	}

	bool unit_is_queen(unit_type_autocast ut) const {
		return unit_is(ut, UnitTypes::Zerg_Queen) || unit_is(ut, UnitTypes::Hero_Matriarch);
	}

	bool unit_is_hatchery(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Zerg_Hatchery)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Lair)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Hive)) return true;
		return false;
	}

	bool unit_is_marine(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Marine)) return true;
		if (unit_is(ut, UnitTypes::Hero_Jim_Raynor_Marine)) return true;
		return false;
	}

	bool unit_is_firebat(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Firebat)) return true;
		if (unit_is(ut, UnitTypes::Hero_Gui_Montag)) return true;
		return false;
	}

	bool unit_is_ghost(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Ghost)) return true;
		if (unit_is(ut, UnitTypes::Hero_Sarah_Kerrigan)) return true;
		if (unit_is(ut, UnitTypes::Hero_Alexei_Stukov)) return true;
		if (unit_is(ut, UnitTypes::Hero_Samir_Duran)) return true;
		if (unit_is(ut, UnitTypes::Hero_Infested_Duran)) return true;
		return false;
	}

	bool unit_is_wraith(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Wraith)) return true;
		if (unit_is(ut, UnitTypes::Hero_Tom_Kazansky)) return true;
		return false;
	}

	bool unit_is_map_revealer(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Spell_Scanner_Sweep)) return true;
		if (unit_is(ut, UnitTypes::Special_Map_Revealer)) return true;
		return false;
	}

	bool unit_is_refinery(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Refinery)) return true;
		if (unit_is(ut, UnitTypes::Protoss_Assimilator)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Extractor)) return true;
		return false;
	}

	bool unit_is_non_flag_beacon(unit_type_autocast ut) const {
		return ut->id >= UnitTypes::Special_Zerg_Beacon && ut->id <= UnitTypes::Special_Protoss_Beacon;
	}

	bool unit_is_special_beacon(unit_type_autocast ut) const {
		return ut->id >= UnitTypes::Special_Zerg_Beacon && ut->id <= UnitTypes::Special_Protoss_Flag_Beacon;
	}

	bool unit_is_scout(unit_type_autocast ut) const {
		return unit_is(ut, UnitTypes::Protoss_Scout) || unit_is(ut, UnitTypes::Hero_Mojo) || unit_is(ut, UnitTypes::Hero_Artanis);
	}

	bool unit_is_vulture(unit_type_autocast ut) const {
		return unit_is(ut, UnitTypes::Terran_Vulture) || unit_is(ut, UnitTypes::Hero_Jim_Raynor_Vulture);
	}

	bool unit_is_nydus(unit_type_autocast ut) const {
		return unit_is(ut, UnitTypes::Zerg_Nydus_Canal);
	}

	bool unit_is_defiler(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Zerg_Defiler)) return true;
		if (unit_is(ut, UnitTypes::Hero_Unclean_One)) return true;
		return false;
	}

	bool unit_is_ultralisk(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Zerg_Ultralisk)) return true;
		if (unit_is(ut, UnitTypes::Hero_Torrasque)) return true;
		return false;
	}

	bool unit_is_arbiter(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Protoss_Arbiter)) return true;
		if (unit_is(ut, UnitTypes::Hero_Danimoth)) return true;
		return false;
	}

	bool unit_is_egg(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Zerg_Egg)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Cocoon)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Lurker_Egg)) return true;
		return false;
	}

	bool unit_is_goliath(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Goliath)) return true;
		if (unit_is(ut, UnitTypes::Hero_Alan_Schezar)) return true;
		return false;
	}

	bool unit_is_critter(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Critter_Rhynadon)) return true;
		if (unit_is(ut, UnitTypes::Critter_Bengalaas)) return true;
		if (unit_is(ut, UnitTypes::Critter_Ragnasaur)) return true;
		if (unit_is(ut, UnitTypes::Critter_Scantid)) return true;
		if (unit_is(ut, UnitTypes::Critter_Kakaru)) return true;
		if (unit_is(ut, UnitTypes::Critter_Ursadon)) return true;
		return false;
	}

	bool unit_is_sieged_tank(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Siege_Tank_Siege_Mode)) return true;
		if (unit_is(ut, UnitTypes::Hero_Edmund_Duke_Siege_Mode)) return true;
		return false;
	}

	bool unit_is_unsieged_tank(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Terran_Siege_Tank_Tank_Mode)) return true;
		if (unit_is(ut, UnitTypes::Hero_Edmund_Duke_Tank_Mode)) return true;
		return false;
	}

	bool unit_is_tank(unit_type_autocast ut) const {
		return unit_is_sieged_tank(ut) || unit_is_unsieged_tank(ut);
	}

	bool unit_is_trap(unit_type_autocast ut) const {
		return ut->id >= UnitTypes::Special_Floor_Missile_Trap && ut->id <= UnitTypes::Special_Right_Wall_Flame_Trap;
	}

	bool unit_is_zerg_building(unit_type_autocast ut) const {
		return ut->id >= UnitTypes::Zerg_Infested_Command_Center && ut->id <= UnitTypes::Special_Cerebrate_Daggoth;
	}

	bool unit_is_fighter(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Protoss_Interceptor)) return true;
		if (unit_is(ut, UnitTypes::Protoss_Scarab)) return true;
		return false;
	}

	bool unit_is_undetected(const unit_t* u, int owner) const {
		if (!u_cloaked(u) && !u_requires_detector(u)) return false;
		if (u->detected_flags & (1 << owner)) return false;
		return true;
	}

	bool unit_target_is_undetected(const unit_t* u, const unit_t* target) const {
		return unit_is_undetected(target, u->owner);
	}

	bool unit_target_is_visible(const unit_t* u, const unit_t* target) const {
		return (target->sprite->visibility_flags & (1 << u->owner)) != 0;
	}

	bool unit_position_is_visible(const unit_t* u, xy position) const {
		return (st.tiles[tile_index(position)].visible & (1 << u->owner)) == 0;
	}

	bool unit_position_is_explored(const unit_t* u, xy position) const {
		return (st.tiles[tile_index(position)].explored & (1 << u->owner)) == 0;
	}

	bool unit_can_see_target(const unit_t* u, const unit_t* target) const {
		if (unit_target_is_undetected(u, target)) return false;
		return unit_target_is_visible(u, target);
	}

	bool unit_order_can_target_self(const unit_t* u, const order_type_t* order) const {
		if (u_grounded_building(u)) {
			if (order->id == Orders::RallyPointUnit) return true;
			if (order->id == Orders::RallyPointTile) return true;
		}
		if (unit_is_defiler(u)) {
			if (order->id == Orders::CastDarkSwarm) return true;
		}
		if (unit_provides_space(u)) {
			if (order->id == Orders::MoveUnload) return true;
		}
		return false;
	}

	bool player_position_is_visible(int owner, xy position) const {
		return (tile_visibility(position) & (1 << owner)) != 0;
	}

	bool player_position_is_explored(int owner, xy position) const {
		return (tile_explored(position) & (1 << owner)) != 0;
	}

	const regions_t::region* get_region_at(xy pos) const {
		size_t index = game_st.regions.tile_region_index.at((size_t)pos.y / 32 * 256 + (size_t)pos.x / 32);
		if (index >= 0x2000) {
			size_t mask_index = ((size_t)pos.y / 8 & 3) * 4 + ((size_t)pos.x / 8 & 3);
			auto* split = &game_st.regions.split_regions[index - 0x2000];
			if (split->mask & (1 << mask_index)) return split->b;
			else return split->a;
		} else return &game_st.regions.regions[index];
	}

	const regions_t::region* get_region_at_prefer_walkable(xy pos) const {
		size_t index = game_st.regions.tile_region_index.at((size_t)pos.y / 32 * 256 + (size_t)pos.x / 32);
		if (index >= 0x2000) {
			auto* split = &game_st.regions.split_regions[index - 0x2000];
			return split->a;
		} else return &game_st.regions.regions[index];
	}

	bool is_reachable(xy from, xy to) const {
		return get_region_at(from)->group_index == get_region_at(to)->group_index;
	}

	bool is_reachable(const unit_t* u, xy to) const {
		if (~u->pathing_flags & 1) return true;
		return is_reachable(u->sprite->position, to);
	}

	// inconsistent pathing_flags check; fixme?
	bool is_reachable(const unit_t* from_u, const unit_t* to_u) const {
		return is_reachable(from_u->sprite->position, to_u->sprite->position);
	}

	bool unit_can_be_infested(const unit_t* u) const {
		if (!unit_is(u, UnitTypes::Terran_Command_Center)) return false;
		if (!u_completed(u)) return false;
		return unit_hp_percent(u) < 50;
	}

	bool unit_can_attack_target(const unit_t* u, const unit_t* target) const {
		if (!target) return false;
		if (unit_is_disabled(u)) return false;
		if (u_invincible(target)) return false;
		if (us_hidden(target)) return false;
		if (unit_target_is_undetected(u, target)) return false;
		if (unit_is_carrier(u)) return true;
		if (unit_is_reaver(u)) {
			if (u_flying(target)) return false;
			return is_reachable(u->sprite->position, target->sprite->position);
		}
		if (unit_is_queen(u)) {
			return unit_can_be_infested(target);
		}
		return unit_target_weapon(u, target) != nullptr;
	}

	bool unit_autoattack(unit_t* u) {
		if (!u->auto_target_unit) return false;
		if (unit_target_is_enemy(u, u->auto_target_unit) && unit_can_attack_target(u, u->auto_target_unit)) {
			auto_attack_target(u, u->auto_target_unit);
			u_set_status_flag(u, unit_t::status_flag_8);
			return true;
		} else {
			u->auto_target_unit = nullptr;
			return false;
		}
	}

	bool unit_can_collide_with(const unit_t* u, const unit_t* target) const {
		if (target == u) return false;
		if (~target->pathing_flags & 1) return false;
		if (u_no_collide(target)) return false;
		if (u_status_flag(target, unit_t::status_flag_400000)) {
			if (unit_target_is_enemy(u, target)) return false;
		}
		if (u_gathering(u) && !u_grounded_building(target)) {
			if (!u_gathering(target)) return false;
			if (u->order_type->id == Orders::ReturnGas) return false;
			if (target->order_type->id != Orders::WaitForGas) return false;
		}
		if (unit_is(u, UnitTypes::Zerg_Larva)) {
			if (!u_grounded_building(target)) return false;
		} else if (unit_is(target, UnitTypes::Zerg_Larva)) {
			return false;
		}
		return true;
	}

	xy rect_difference(rect a, rect b) const {
		int x, y;
		if (a.from.x > b.to.x) x = a.from.x - b.to.x;
		else if (b.from.x > a.to.x) x = b.from.x - a.to.x;
		else x = 0;
		if (a.from.y > b.to.y) y = a.from.y - b.to.y;
		else if (b.from.y > a.to.y) y = b.from.y - a.to.y;
		else y = 0;

		return xy(x, y);
	}

	xy nearest_pos_in_rect(xy pos, rect area) const {
		if (area.from.x > pos.x) pos.x = area.from.x;
		else if (area.to.x < pos.x) pos.x = area.to.x;
		if (area.from.y > pos.y) pos.y = area.from.y;
		else if (area.to.y < pos.y) pos.y = area.to.y;
		return pos;
	}

	int xy_length(xy vec) const {
		unsigned int x = std::abs(vec.x);
		unsigned int y = std::abs(vec.y);
		if (x < y) std::swap(x, y);
		if (x / 4 < y) x = x - x / 16 + y * 3 / 8 - x / 64 + y * 3 / 256;
		return x;
	}

	fp8 xy_length(xy_fp8 vec) const {
		return fp8::from_raw(xy_length({ (int)vec.x.raw_value, (int)vec.y.raw_value }));
	}

	xy_fp8 to_xy_fp8(xy position) const {
		return { fp8::integer(position.x), fp8::integer(position.y) };
	}
	xy to_xy(xy_fp8 position) const {
		return { (int)position.x.integer_part(), (int)position.y.integer_part() };
	}

	int units_distance(const unit_t* a, const unit_t* b) const {
		auto a_rect = unit_sprite_bounding_box(a);
		auto b_rect = unit_sprite_bounding_box(b);
		return xy_length(rect_difference(a_rect, b_rect));
	}

	int unit_distance_to(const unit_t* u, xy pos) const {
		return xy_length(pos - nearest_pos_in_rect(pos, unit_sprite_bounding_box(u)));
	}

	template<size_t integer_bits>
	direction_t atan(fixed_point<integer_bits, 8, true> x) const {
		bool negative = x < decltype(x)::zero();
		if (negative) x = -x;
		typename decltype(x)::raw_unsigned_type uv = x.raw_value;
		size_t r = std::lower_bound(tan_table.begin(), tan_table.end(), uv) - tan_table.begin();
		return negative ? -direction_t::from_raw((direction_t::raw_type)r) : direction_t::from_raw((direction_t::raw_type)r);
	}

	direction_t xy_direction(xy_fp8 pos) const {
		if (pos.x == 0_fp8) return pos.y <= 0_fp8 ? 0_dir : -128_dir;
		direction_t r = atan(pos.y / pos.x);
		if (pos.x > 0_fp8) r += 64_dir;
		else r = -64_dir + r;
		return r;
	}

	direction_t xy_direction(xy pos) const {
		if (pos.x == 0) return pos.y <= 0 ? 0_dir : -128_dir;
		direction_t r = atan(fp8::integer(pos.y) / pos.x);
		if (pos.x > 0) r = 64_dir + r;
		else r = -64_dir + r;
		return r;
	}

	xy_fp8 direction_xy(direction_t dir, fp8 length) const {
		return direction_table[direction_index(dir)] * length;
	}

	xy_fp8 direction_xy(direction_t dir, int length) const {
		return direction_table[direction_index(dir)] * length;
	}

	xy_fp8 direction_xy(direction_t dir) const {
		return direction_table[direction_index(dir)];
	}

	size_t direction_index(direction_t dir) const {
		auto v = dir.fractional_part();
		if (v < 0) return 256 + v;
		else return v;
	}

	direction_t direction_from_index(size_t index) const {
		int v = (int)index;
		if (v >= 128) v = -(256 - v);
		return direction_t::from_raw(v);
	}

	direction_t units_direction(const unit_t* from, const unit_t* to) const {
		xy relpos = to->sprite->position - from->sprite->position;
		return xy_direction(relpos);
	}

	bool unit_target_in_attack_angle(const unit_t* u, xy target_pos, const weapon_type_t* weapon) const {
		auto dir = xy_direction(target_pos - u->sprite->position);
		if (unit_is(u, UnitTypes::Zerg_Lurker)) {
			error("unit_target_in_attack_angle lurker: unreachable?");
			// For some reason, this field is set here for lurkers, but I would really like u to be const.
			// todo: figure out if it is necessary.
			//u->heading = dir;
			return true;
		}
		return fp8::extend(dir - u->heading).abs() <= weapon->attack_angle;
	}

	int weapon_max_range(const unit_t* u, const weapon_type_t* w) const {
		auto range_upgrade_bonus = [&]() {
			switch (u->unit_type->id) {
			case UnitTypes::Terran_Marine:
				return player_has_upgrade(u->owner, UpgradeTypes::U_238_Shells) ? 32 : 0;
			case UnitTypes::Zerg_Hydralisk:
				return player_has_upgrade(u->owner, UpgradeTypes::Grooved_Spines) ? 32 : 0;
			case UnitTypes::Protoss_Dragoon:
				return player_has_upgrade(u->owner, UpgradeTypes::Singularity_Charge) ? 64 : 0;
			case UnitTypes::Hero_Fenix_Dragoon:
				return 64;
			case UnitTypes::Terran_Goliath:
			case UnitTypes::Terran_Goliath_Turret:
				return w->id == WeaponTypes::Hellfire_Missile_Pack && player_has_upgrade(u->owner, UpgradeTypes::Charon_Boosters) ? 96 : 0;
			case UnitTypes::Hero_Alan_Schezar:
			case UnitTypes::Hero_Alan_Schezar_Turret:
				return w->id == WeaponTypes::Hellfire_Missile_Pack_Alan_Schezar ? 96 : 0;
			default:
				return 0;
			};
		};
		int r = w->max_range;
		if (u_in_bunker(u)) r += 64;
		r += range_upgrade_bonus();
		return r;
	}

	int unit_target_movement_range(const flingy_t* u, const flingy_t* target) const {
		if (!u_movement_flag(u, 2)) return 0;
		if (u_movement_flag(target, 2)) {
			if (fp8::extend(target->next_velocity_direction - u->next_velocity_direction).abs() <= 32_fp8) return 0;
		}
		return unit_halt_distance(u).integer_part();
	}

	bool unit_target_in_weapon_movement_range(const unit_t* u, const unit_t* target) const {
		if (!target) target = u->order_target.unit;
		if (!target) return true;
		if (!unit_can_see_target(u, target)) return false;
		auto* w = unit_target_weapon(u, target);
		if (!w) return false;
		int d = units_distance(unit_main_unit(u), target);
		if (w->min_range && d <= w->min_range) return false;
		int max_range = weapon_max_range(u, w);
		max_range += unit_target_movement_range(u, target);
		return d <= max_range;
	}

	bool unit_target_out_of_max_range(const unit_t* u, const unit_t* target) const {
		if (!unit_can_see_target(u, target)) return true;
		auto* w = unit_target_weapon(u, target);
		if (!w) return false;
		int d = units_distance(u, target);
		int max_range = weapon_max_range(u, w);
		max_range += unit_target_movement_range(u, target);
		return d > max_range;
	}

	unit_t* unit_first_loaded_unit(const unit_t* u) const {
		for (unit_t* nu : loaded_units(u)) {
			return nu;
		}
		return nullptr;
	}

	race_t unit_race(unit_type_autocast ut) const {
		if (ut->group_flags & GroupFlags::Zerg) return race_t::zerg;
		if (ut->group_flags & GroupFlags::Terran) return race_t::terran;
		if (ut->group_flags & GroupFlags::Protoss) return race_t::protoss;
		return race_t::none;
	}

	bool unit_provides_space(const unit_t* u) const {
		if (!u->unit_type->space_provided) return false;
		if (unit_is(u, UnitTypes::Zerg_Overlord) && !player_has_upgrade(u->owner, UpgradeTypes::Ventral_Sacs)) return false;
		if (u_hallucination(u)) return false;
		return true;
	}

	size_t unit_space_left(const unit_t* u) const {
		size_t r = u->unit_type->space_provided;
		for (const unit_t* nu : loaded_units(u)) {
			r -= nu->unit_type->space_required;
		}
		return r;
	}

	bool unit_can_load_target(const unit_t* u, const unit_t* target) const {
		if (!u_completed(u)) return false;
		if (unit_is_disabled(u)) return false;
		if (u_burrowed(target)) return false;
		if (u->owner != target->owner) return false;
		if (ut_building(u)) {
			if (unit_race(target) != race_t::terran) return false;
			if (target->unit_type->space_required > 1) return false;
		}
		return unit_space_left(u) >= target->unit_type->space_required;
	}

	size_t unit_interceptor_count(const unit_t* u) const {
		if (!unit_is_carrier(u)) return 0;
		return u->carrier.inside_count + u->carrier.outside_count;
	}
	size_t unit_scarab_count(const unit_t* u) const {
		if (!unit_is_reaver(u)) return 0;
		return u->reaver.inside_count;
	}

	size_t unit_max_interceptor_count(const unit_t* u) const {
		if (!unit_is_carrier(u)) return 0;
		if (ut_hero(u)) return 8;
		return player_has_upgrade(u->owner, UpgradeTypes::Carrier_Capacity) ? 8 : 4;
	}
	size_t unit_max_scarab_count(const unit_t* u) const {
		if (!unit_is_reaver(u)) return 0;
		if (ut_hero(u)) return 10;
		return player_has_upgrade(u->owner, UpgradeTypes::Reaver_Capacity) ? 10 : 5;
	}

	size_t unit_queued_fighter_units(const unit_t* u) const {
		size_t r = 0;
		for (const unit_type_t* ut : u->build_queue) {
			if (unit_is_fighter(ut)) ++r;
		}
		return r;
	}

	size_t unit_spider_mine_count(const unit_t* u) const {
		if (!unit_is_vulture(u)) return 0;
		return u->vulture.spider_mine_count;
	}

	bool unit_can_attack(const unit_t* u) const {
		if (unit_or_subunit_ground_weapon(u) || unit_or_subunit_air_weapon(u)) return true;
		if (unit_interceptor_count(u)) return true;
		if (unit_scarab_count(u)) return true;
		return false;
	}

	int unit_target_attack_priority(const unit_t* u, const unit_t* target) const {
		bool is_loaded_unit = false;
		if (unit_is(target, UnitTypes::Terran_Bunker)) {
			const unit_t* loaded_unit = unit_first_loaded_unit(target);
			if (loaded_unit) {
				target = loaded_unit;
				is_loaded_unit = true;
			}
		}
		if (unit_is(target, UnitTypes::Zerg_Larva)) return 5;
		if (unit_is_egg(target)) return 5;
		int r = 0;
		if (ut_worker(target)) r += 2;
		else if (!unit_can_attack_target(target, u)) {
			if (unit_can_attack(target)) r += 2;
			else if (u_can_move(target)) r += 3;
			else r += 4;
		}
		if (is_loaded_unit || !u_completed(target)) ++r;
		if (r == 0 && u_cannot_attack(target)) ++r;
		return r;
	}

	bool unit_target_in_range(const unit_t* u, const unit_t* target, int range) const {
		return units_distance(unit_main_unit(u), target) <= range;
	}

	auto get_default_priority_targets(const unit_t* u, int min_distance, int max_distance) const {
		xy pos = u->sprite->position;
		rect bounds { { pos.x - max_distance - 64, pos.y - max_distance - 64 }, { pos.x + max_distance + 64, pos.y + max_distance + 64 } };
		const unit_t* main_unit = unit_main_unit(u);
		const unit_t* attacking_unit = unit_attacking_unit(u);
		bool can_turn = u_can_turn(attacking_unit);
		std::array<static_vector<unit_t*, 0x10>, 6> targets {};
		for (unit_t* target : find_units_noexpand(bounds)) {
			if (target == u) continue;
			if (!unit_target_is_enemy(u, target)) continue;
			if (!unit_target_is_visible(u, target)) continue;
			if (!unit_can_attack_target(u, target)) continue;
			int distance = units_distance(main_unit, target);
			if (distance > max_distance) continue;
			if (min_distance && distance <= min_distance) continue;
			if (!can_turn) {
				if (!attacking_unit->unit_type->ground_weapon) error("get_default_priority_targets: null ground weapon");
				if (!unit_target_in_attack_angle(attacking_unit, target->sprite->position, attacking_unit->unit_type->ground_weapon)) continue;
			}
			int prio = unit_target_attack_priority(u, target);
			if (targets[prio].size() < 0x10) targets[prio].push_back(target);
		}
		return targets;
	}

	unit_t* find_acquire_target(const unit_t* u) const {
		int acq_range = unit_target_acquisition_range(u);
		if (u_in_bunker(u)) acq_range += 2;

		int min_distance = 0;
		int max_distance = acq_range * 32;

		auto* ground_weapon = unit_or_subunit_ground_weapon(u);
		auto* air_weapon = unit_or_subunit_air_weapon(u);
		if (ground_weapon) {
			if (!air_weapon) min_distance = ground_weapon->min_range;
			else min_distance = std::min(ground_weapon->min_range, air_weapon->min_range);
		} else {
			if (air_weapon) min_distance = air_weapon->min_range;
		}
		auto targets = get_default_priority_targets(u, min_distance, max_distance);
		for (auto& v : targets) {
			if (v.empty()) continue;
			return *get_best_score(v, [&](const unit_t* target) {
				return xy_length(target->sprite->position - u->sprite->position);
			});
		}
		return nullptr;
	}

	unit_t* find_acquire_random_target(const unit_t* u) {
		int acq_range = unit_target_acquisition_range(u);
		if (u_in_bunker(u)) acq_range += 2;

		int min_distance = 0;
		int max_distance = acq_range * 32;

		auto targets = get_default_priority_targets(u, min_distance, max_distance);
		for (auto& v : targets) {
			if (v.empty()) continue;
			if (v.size() == 1) return v.front();
			size_t index = lcg_rand(19) % v.size();
			return v[index];
		}
		return nullptr;
	}

	void stop_flingy(flingy_t* u) {
		if (u_movement_flag(u, 4)) return;
		u_unset_movement_flag(u, 0x10);
		u_unset_movement_flag(u, 0x20);
		u_unset_movement_flag(u, 0x40);
		u->move_target.pos = u->position;
		u->move_target.unit = nullptr;
		u->next_movement_waypoint = u->position;
		if (u_movement_flag(u, 2) && u->flingy_movement_type == 0) {
			fp8 d = unit_halt_distance(u);
			u->move_target.pos = to_xy(u->exact_position + direction_xy(u->next_velocity_direction, d));
			u->next_movement_waypoint = u->move_target.pos;
			if (!unit_is_at_move_target(u)) u_set_movement_flag(u, 4);
		}
	}

	void stop_unit(unit_t* u) {
		if (u->pathing_flags & 2) u->pathing_flags |= 4;
		stop_flingy(u);
		xy move_target = restrict_move_target_to_valid_bounds(u, u->move_target.pos);
		if (u->move_target.pos != move_target) {
			u->move_target.pos = move_target;
			u->next_movement_waypoint = move_target;
		}
		u->move_target_timer = 15;
	}

	void order_done(unit_t* u, order_target_t target = {}) {
		if (!u->order_queue.empty()) {
			u->user_action_flags |= 1;
			activate_next_order(u);
		} else set_unit_order(u, u->unit_type->return_to_idle, target);
	}

	void set_next_target_waypoint(flingy_t* u, xy pos) {
		if (u->next_target_waypoint == pos) return;
		u->next_target_waypoint = pos;
	}

	void set_unit_gathering(unit_t* u) {
		if (u_gathering(u)) return;
		u_set_status_flag(u, unit_t::status_flag_gathering);
		queue_order_front(u, get_order_type(Orders::ResetHarvestCollision), {});
	}

	bool any_neighbor_tile_unoccupied(const unit_t* u) const {
		auto unit_bb = unit_sprite_inner_bounding_box(u);
		rect_t<xy_t<size_t>> tile_bb;
		tile_bb.from.x = unit_bb.from.x / 32u - 1;
		tile_bb.from.y = unit_bb.from.y / 32u - 1;
		tile_bb.to.x = unit_bb.to.x / 32u + 1;
		tile_bb.to.y = unit_bb.to.y / 32u + 1;
		if (tile_bb.from.x >= game_st.map_tile_width) tile_bb.from.x = 0;
		if (tile_bb.from.y >= game_st.map_tile_height) tile_bb.from.y = 0;
		if (tile_bb.to.x >= game_st.map_tile_width) tile_bb.to.x = game_st.map_tile_width - 1;
		if (tile_bb.to.y >= game_st.map_tile_height) tile_bb.to.y = game_st.map_tile_height - 1;
		size_t tile_width = (u->unit_type->dimensions.from.x + u->unit_type->dimensions.to.x + 1) / 32u;
		size_t tile_height = (u->unit_type->dimensions.from.y + u->unit_type->dimensions.to.y + 1) / 32u;
		size_t top_index = tile_bb.from.y * game_st.map_tile_width + (tile_bb.from.x + 1);
		size_t bottom_index = tile_bb.to.y * game_st.map_tile_width + (tile_bb.from.x + 1);
		for (size_t i = 0; i != tile_width; ++i) {
			if (~st.tiles.at(top_index).flags & tile_t::flag_occupied) return true;
			if (~st.tiles.at(bottom_index).flags & tile_t::flag_occupied) return true;
			++top_index;
			++bottom_index;
		}
		size_t left_index = (tile_bb.from.y + 1) * game_st.map_tile_width + tile_bb.from.x;
		size_t right_index = (tile_bb.from.y + 1) * game_st.map_tile_width + tile_bb.to.x;
		for (size_t i = 0; i != tile_height; ++i) {
			if (~st.tiles.at(left_index).flags & tile_t::flag_occupied) return true;
			if (~st.tiles.at(right_index).flags & tile_t::flag_occupied) return true;
			left_index += game_st.map_tile_width;
			right_index += game_st.map_tile_width;
		}
		return false;
	}

	bool unit_pos_is_bordering_target(const unit_t* u, xy pos, const unit_t* target) {
		auto bb = unit_sprite_inner_bounding_box(target);
		bb.from -= u->unit_type->dimensions.to + xy(1, 1);
		bb.to.x += u->unit_type->dimensions.from.x + 1;
		// This is an original bug. Don't fix
		bb.to.x += u->unit_type->dimensions.from.y + 1;
		if (pos.x == bb.from.x || pos.x == bb.to.x) {
			if (pos.y >= bb.from.y && pos.y <= bb.to.y) {
				return true;
			}
		} else if (pos.y == bb.from.y || pos.y == bb.to.y) {
			if (pos.x >= bb.from.x && pos.x <= bb.to.x) {
				return true;
			}
		}
		return false;
	}

	void move_to_target(unit_t* u, unit_t* target) {
		if (u->move_target.unit == target && (u->move_target.pos == target->sprite->position || unit_pos_is_bordering_target(u, u->move_target.pos, target))) {
			u_unset_status_flag(u, unit_t::status_flag_immovable);
		} else {
			set_unit_move_target(u, target);
		}
	}

	void move_to_target_reset(unit_t* u, unit_t* target) {
		move_to_target(u, target);
		set_next_target_waypoint(u, target->sprite->position);
	}

	bool try_gather_resource(unit_t* u, unit_t* target) {
		if (target->building.resource.is_being_gathered) return false;
		target->building.resource.is_being_gathered = true;
		u->worker.gather_target = target;
		u->worker.is_gathering = true;
		if (u->order_type->id == Orders::WaitForGas) {
			queue_order_front(u, get_order_type(Orders::HarvestGas), {target->sprite->position, target});
		} else {
			queue_order_front(u, get_order_type(Orders::GatheringInterrupted), {});
			queue_order_front(u, get_order_type(Orders::MiningMinerals), {target->sprite->position, target});
		}
		order_done(u);
		return true;
	}

	void wait_for_resource(unit_t* u, unit_t* target) {
		u->order_process_timer = 0;
		u->order_state = 2;
		if (!try_gather_resource(u, target)) {
			if (!u->worker.gather_target) {
				u->worker.gather_target = target;
				target->building.resource.gather_queue.push_front(*u);
			}
			queue_order_front(u, get_order_type(Orders::GatherWaitInterrupted), {});
		}
	}

	bool is_facing_next_target_waypoint(const unit_t* u, fp8 angle = 0_fp8) const {
		if (u->position == u->next_target_waypoint) return true;
		if (fp8::extend(u->heading - xy_direction(u->next_target_waypoint - u->sprite->position)).abs() <= angle) return true;
		return false;
	}

	int long_path_distance(xy from, xy to) const {
		if (!is_reachable(from, to)) return 0x7fff;
		pathfinder pf;
		if (!pathfinder_find_long_path(pf, from, to)) return 0x7ffe;
		if (pf.long_path.size() <= 2) return xy_length(to - from);
		xy pos = to_xy(pf.long_path[0]->center);
		int r = 0;
		for (auto i = std::next(pf.long_path.begin()); i != pf.long_path.end(); ++i) {
			xy next_pos = to_xy((*i)->center);
			r += xy_length(next_pos - pos);
			pos = next_pos;
		}
		return r;
	}

	int unit_long_path_distance(const unit_t* u, xy from, xy to) const {
		if (~u->pathing_flags & 1) return xy_length(to - from);
		return long_path_distance(from, to);
	}

	void destroy_carrying_images(const unit_t* u) {
		destroy_image_from_to(u->sprite, ImageTypes::IMAGEID_Mineral_Chunk_Shadow, ImageTypes::IMAGEID_Psi_Emitter_Shadow_Carried);
		destroy_image_from_to(u->sprite, ImageTypes::IMAGEID_Flag, ImageTypes::IMAGEID_Terran_Gas_Tank_Type2);
		destroy_image_from_to(u->sprite, ImageTypes::IMAGEID_Uraj, ImageTypes::IMAGEID_Khalis);
	}

	void unit_gather_resources_from(unit_t* u, unit_t* resource) {
		const image_type_t* image_type = nullptr;
		bool is_minerals = false;
		switch (resource->unit_type->id) {
		case UnitTypes::Terran_Refinery:
			image_type = get_image_type(ImageTypes::IMAGEID_Terran_Gas_Tank_Type1);
			break;
		case UnitTypes::Protoss_Assimilator:
			image_type = get_image_type(ImageTypes::IMAGEID_Protoss_Gas_Orb_Type1);
			break;
		case UnitTypes::Zerg_Extractor:
			image_type = get_image_type(ImageTypes::IMAGEID_Zerg_Gas_Sac_Type1);
			break;
		case UnitTypes::Resource_Mineral_Field:
		case UnitTypes::Resource_Mineral_Field_Type_2:
		case UnitTypes::Resource_Mineral_Field_Type_3:
			image_type = get_image_type(ImageTypes::IMAGEID_Mineral_Chunk_Type1);
			is_minerals = true;
			break;
		default:
			break;
		}
		if (!image_type) return;
		int gathered = 0;
		if (resource->building.resource.resource_count < 8) {
			image_type = get_image_type((ImageTypes)((int)image_type->id + 1));
			if (is_minerals) {
				gathered = resource->building.resource.resource_count;
				kill_unit(resource);
			} else {
				resource->building.resource.resource_count = 0;
				gathered = 2;
			}
		} else {
			set_unit_resources(resource, resource->building.resource.resource_count - 8);
			gathered = 8;
			if (is_minerals) {
				if (resource->building.resource.resource_count == 0) {
					kill_unit(resource);
				}
			} else {
				if (resource->building.resource.resource_count < 8) {
					// todo: out of gas error message/sound callback?
				}
			}
		}
		if (gathered) {
			if (u->carrying_flags & 3) {
				destroy_carrying_images(u);
				u->carrying_flags = 0;
			}
			if (u->carrying_flags == 0) {
				if (is_minerals) u->carrying_flags = 2;
				else u->carrying_flags = 1;
				u->worker.resources_carried = gathered;
				image_t* image = create_image(image_type, u->sprite, {}, image_order_above);
				if (image) {
					if (!i_flag(image, image_t::flag_uses_special_offset)) {
						i_set_flag(image, image_t::flag_uses_special_offset);
						update_image_special_offset(image);
					}
				}
			}
		}
	}

	void gather_queue_next(unit_t* u, unit_t* resource) {
		u->worker.is_gathering = false;
		u->worker.gather_target = nullptr;
		if (resource) {
			resource->building.resource.is_being_gathered = false;
			unit_t* next_unit = nullptr;
			for (unit_t* queued_unit : reverse(ptr(resource->building.resource.gather_queue))) {
				if (!unit_is_disabled(queued_unit)) {
					if (queued_unit->order_type->id == Orders::WaitForGas || queued_unit->order_type->id == Orders::WaitForMinerals) {
						next_unit = queued_unit;
						break;
					}
				}
			}
			if (next_unit) {
				next_unit->worker.gather_target = nullptr;
				resource->building.resource.gather_queue.remove(*next_unit);
				remove_one_order(next_unit, get_order_type(Orders::GatherWaitInterrupted));
				try_gather_resource(next_unit, resource);
			}
		}
	}

	bool unit_is_active_resource_depot(const unit_t* u) const {
		if (!u_grounded_building(u)) return false;
		if (!ut_resource_depot(u)) return false;

		if (u_completed(u)) return true;
		if (unit_is(u, UnitTypes::Zerg_Hive)) return true;
		if (unit_is(u, UnitTypes::Zerg_Lair)) return true;
		if (unit_is(u, UnitTypes::Zerg_Hatchery) && unit_is_morphing_building(u)) return true;
		return false;
	}

	unit_t* find_nearest_active_resource_depot(const unit_t* u) const {
		return find_nearest_unit(u, map_bounds(), [&](const unit_t* target) {
			if (target == u) return false;
			if (!unit_is_active_resource_depot(target)) return false;
			if (target->owner != u->owner) return false;
			if (~u->pathing_flags & 1 || !is_reachable(u->sprite->position, target->sprite->position)) return false;
			return true;
		});
	}

	void drop_carried_items(unit_t* u) {
		if ((u->carrying_flags & ~3) == 0) return;
		error("drop_carried_items: fixme");
	}

	bool has_available_supply_for(int owner, const unit_type_t* unit_type, bool show_error = false) const {
		if (ut_flag(unit_type, (unit_type_t::flags_t)0x1000000)) return true;
		auto supply_required = unit_type->supply_required;
		if (ut_two_units_in_one_egg(unit_type)) supply_required *= 2;
		if (supply_required == fp1::zero()) return true;
		if (unit_type->id == UnitTypes::Zerg_Lurker) {
			supply_required -= get_unit_type(UnitTypes::Zerg_Hydralisk)->supply_required;
		}
		auto race = unit_race(unit_type);
		if (race == race_t::none) return false;
		size_t index = (size_t)race;
		if (st.supply_used[owner][index] + supply_required > fp1::integer(200)) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		if (st.supply_used[owner][index] + supply_required > st.supply_available[owner][index]) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		return true;
	}

	bool has_available_resources_for(int owner, const unit_type_t* unit_type, bool show_error = false) const {
		if (st.current_minerals[owner] < unit_type->mineral_cost) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		if (st.current_gas[owner] < unit_type->gas_cost) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		return true;
	}

	bool has_available_resources_for(int owner, const tech_type_t* tech_type, bool show_error = false) const {
		if (st.current_minerals[owner] < tech_type->mineral_cost) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		if (st.current_gas[owner] < tech_type->gas_cost) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		return true;
	}

	int upgrade_mineral_cost(int owner, const upgrade_type_t* upgrade_type) const {
		return upgrade_type->mineral_cost_base + upgrade_type->mineral_cost_factor * player_upgrade_level(owner, upgrade_type->id);
	}
	int upgrade_gas_cost(int owner, const upgrade_type_t* upgrade_type) const {
		return upgrade_type->gas_cost_base + upgrade_type->gas_cost_factor * player_upgrade_level(owner, upgrade_type->id);
	}
	int upgrade_time_cost(int owner, const upgrade_type_t* upgrade_type) const {
		return upgrade_type->time_cost_base + upgrade_type->time_cost_factor * player_upgrade_level(owner, upgrade_type->id);
	}

	bool has_available_resources_for(int owner, const upgrade_type_t* upgrade_type, bool show_error = false) const {
		if (st.current_minerals[owner] < upgrade_mineral_cost(owner, upgrade_type)) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		if (st.current_gas[owner] < upgrade_gas_cost(owner, upgrade_type)) {
			// todo: callback for error message/sound?
			(void)show_error;
			return false;
		}
		return true;
	}

	bool build_queue_push(unit_t* u, const unit_type_t* unit_type) {
		if (u->build_queue.size() >= 5) return false;
		if (u->build_queue.empty() && !has_available_supply_for(u->owner, unit_type, true)) return false;
		if (!has_available_resources_for(u->owner, unit_type, true)) return false;
		u->build_queue.push_back(unit_type);
		if (!ut_building(unit_type)) {
			st.current_minerals[u->owner] -= unit_type->mineral_cost;
			st.current_gas[u->owner] -= unit_type->gas_cost;
		}
		return true;
	}

	bool train_unit(unit_t* u, const unit_type_t* unit_type) {
		if (build_queue_push(u, unit_type)) {
			set_secondary_order(u, get_order_type(Orders::Train));
			return true;
		} else return false;
	}

	void refund_unit_costs(int owner, unit_type_autocast unit_type) {
		st.current_minerals[owner] += unit_type->mineral_cost;
		st.current_gas[owner] += unit_type->gas_cost;
	}

	void partially_refund_unit_costs(int owner, unit_type_autocast unit_type) {
		st.current_minerals[owner] += unit_type->mineral_cost * 3 / 4;
		st.current_gas[owner] += unit_type->gas_cost * 3 / 4;
	}

	void cancel_morphing_building(unit_t* u) {
		if (unit_is_morphing_building(u)) {
			if (u->build_queue.empty()) error("cancel_morphing_building: build queue is empty");
			const unit_type_t* build_type = u->build_queue.front();
			u->build_queue.erase(u->build_queue.begin());
			partially_refund_unit_costs(u->owner, build_type);
			u_set_status_flag(u, unit_t::status_flag_completed);
			add_completed_unit(u, -1, false);
			u_unset_status_flag(u, unit_t::status_flag_completed);
			auto hp = u->hp;
			finish_building_unit(u);
			complete_unit(u);
			set_unit_hp(u, hp);
			sprite_run_anim(u->sprite, iscript_anims::AlmostBuilt);
			play_sound(4, u);
		} else {
			partially_refund_unit_costs(u->owner, u);
			st.unit_score[u->owner] -= u->unit_type->build_score;
			--st.total_non_buildings_ever_completed[u->owner];
			auto prev_hp = fp8::integer(u->previous_hp);
			if (unit_is(u, UnitTypes::Zerg_Extractor)) {
				unit_t* drone = create_unit(UnitTypes::Zerg_Drone, u->sprite->position, u->owner);
				if (drone) {
					finish_building_unit(drone);
					complete_unit(drone);
					set_unit_hp(drone, prev_hp);
				} else {
					display_last_error_for_player(u->owner);
				}
				kill_unit(u);
			} else {
				thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Zerg_Building_Spawn_Small), u->sprite->position, 0);
				if (t) {
					t->sprite->elevation_level = u->sprite->elevation_level + 1;
					if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
				}
				const unit_type_t* build_type = u->unit_type;
				morph_unit(u, get_unit_type(UnitTypes::Zerg_Drone));
				finish_building_unit(u);
				complete_unit(u);
				if (u->sprite->images.back().modifier == 10) u->sprite->images.back().offset.y = 7;
				set_unit_order(u, get_order_type(Orders::ResetCollision));
				set_queued_order(u, false, u->unit_type->return_to_idle, {});
				set_unit_hp(u, prev_hp);
				remove_creep_provider(build_type, u->sprite->position, false);
				play_sound(4, u);
			}
		}
	}

	bool cancel_building_unit(unit_t* u) {
		if (unit_dying(u)) return false;
		if (u_completed(u)) return false;
		if (unit_is(u, UnitTypes::Zerg_Guardian)) return false;
		if (unit_is(u, UnitTypes::Zerg_Lurker)) return false;
		if (unit_is(u, UnitTypes::Zerg_Devourer)) return false;
		if (unit_is(u, UnitTypes::Zerg_Mutalisk)) return false;
		if (unit_is(u, UnitTypes::Zerg_Hydralisk)) return false;
		if (unit_is_nydus(u) && !u->building.nydus.exit) return false;
		if (u_grounded_building(u)) {
			if (unit_race(u) == race_t::zerg) {
				cancel_morphing_building(u);
				return true;
			}
			partially_refund_unit_costs(u->owner, u->unit_type);
		} else {
			const unit_type_t* refund_type = u->unit_type;
			if (unit_is_egg(u) && !u->build_queue.empty()) refund_type = u->build_queue.front();
			refund_unit_costs(u->owner, refund_type);
		}
		const unit_type_t* morph_to = nullptr;
		if (unit_is(u, UnitTypes::Zerg_Cocoon)) {
			morph_to = get_unit_type(UnitTypes::Zerg_Mutalisk);
		} else if (unit_is(u, UnitTypes::Zerg_Lurker_Egg)) {
			morph_to = get_unit_type(UnitTypes::Zerg_Hydralisk);
		}
		if (morph_to) {
			morph_unit(u, morph_to);
			if (!u->build_queue.empty()) u->build_queue.erase(u->build_queue.begin());
			u->remaining_build_time = 0;
			if (!u->previous_unit_type) error("cancel_building_unit: previous_unit_type is null");
			replace_sprite_images(u->sprite, u->previous_unit_type->flingy->sprite->image, 0_dir);
			u->order_signal &= ~4;
			sprite_run_anim(u->sprite, iscript_anims::SpecialState2);
			set_unit_order(u, get_order_type(Orders::ZergBirth));
		} else {
			if (unit_is(u, UnitTypes::Terran_Nuclear_Missile) && u->connected_unit) {
				if (unit_is_ghost(u->connected_unit)) {
					u->connected_unit->ghost.nuke_dot = nullptr;
				}
				u->connected_unit->order_state = 0;
			}
			kill_unit(u);
		}
		return true;
	}

	void cancel_build_queue(unit_t* u, size_t index) {
		if (index >= u->build_queue.size()) return;
		const unit_type_t* unit_type = u->build_queue[index];
		u->build_queue.erase(u->build_queue.begin() + index);
		if (index != 0 || !u->current_build_unit) {
			if (!ut_building(unit_type)) {
				refund_unit_costs(u->owner, unit_type);
			}
		} else {
			cancel_building_unit(u->current_build_unit);
		}
	}

	void cancel_build_queue(unit_t* u) {
		while (!u->build_queue.empty()) cancel_build_queue(u, u->build_queue.size() - 1);
	}

	void place_building(unit_t* u, const order_type_t* order_type, const unit_type_t* unit_type, xy position) {
		if (order_type->id == Orders::BuildingLand) {
			set_unit_order(u, order_type, position);
			return;
		}
		if (unit_is(u, UnitTypes::Zerg_Drone)) {
			if (u->order_type->id == Orders::DroneBuild) return;
			if (u->order_type->id == Orders::DroneLand && (u_order_not_interruptible(u) || u_iscript_nobrk(u))) return;
		}
		if (unit_is(u, UnitTypes::Zerg_Nydus_Canal) && order_type->id == Orders::BuildNydusExit) {
			set_unit_order(u, order_type, position);
			return;
		}
		if (order_type->id == Orders::CTFCOP2) {
			set_unit_order(u, order_type, position);
			return;
		}
		if (!has_available_supply_for(u->owner, unit_type, true)) return;
		if (!has_available_resources_for(u->owner, unit_type, true)) return;
		set_unit_order(u, order_type, position);
		if (!u->build_queue.empty()) {
			for (size_t i = 5; i;) {
				--i;
				cancel_build_queue(u, i);
			}
		}
		if (ut_building(u) && ut_building(unit_type)) {
			u->building.addon_build_type = unit_type;
		} else {
			build_queue_push(u, unit_type);
		}
	}

	bool player_has_supply_and_resources_for(int owner, const unit_type_t* unit_type, bool show_error = false) {
		if (!has_available_supply_for(owner, unit_type, show_error)) return false;
		if (!has_available_resources_for(owner, unit_type, show_error)) return false;
		return true;
	}

	unit_t* get_building_at_center_position(xy pos, const unit_type_t* unit_type) const {
		for (unit_t* n : find_units_noexpand({pos, pos + xy(game_st.max_unit_width, game_st.max_unit_height)})) {
			if (n->unit_type == unit_type && n->sprite->position / 32 == pos / 32) {
				return n;
			}
		}
		return nullptr;
	}

	bool is_in_psionic_matrix_range(xy rel) const {
		unsigned x = std::abs(rel.x);
		unsigned y = std::abs(rel.y);
		if (x >= 256) return false;
		if (y >= 160) return false;
		if (rel.x < 0) --x;
		if (rel.y < 0) --y;
		return psi_field_mask[y / 32u][x / 32u];
	}

	bool is_in_psionic_matrix(int owner, xy pos) const {
		for (const unit_t* u : ptr(st.psionic_matrix_units)) {
			if (u->owner != owner) continue;
			if (is_in_psionic_matrix_range(pos - u->sprite->position)) return true;
		}
		return false;
	}

	bool can_place_building(const unit_t* u, int owner, const unit_type_t* unit_type, xy pos, bool check_undetected_units, bool check_invisible_tiles) const {
		xy_t<size_t> tile_pos;
		tile_pos.x = (pos.x - unit_type->placement_size.x / 2) / 32u;
		tile_pos.y = (pos.y - unit_type->placement_size.y / 2) / 32u;
		xy_t<size_t> tile_size;
		tile_size.x = unit_type->placement_size.x / 32u;
		tile_size.y = unit_type->placement_size.y / 32u;
		if (tile_pos.x >= game_st.map_tile_width) return false;
		if (tile_pos.y >= game_st.map_tile_height) return false;
		if (tile_pos.x + tile_size.x > game_st.map_tile_width) return false;
		if (tile_pos.y + tile_size.y > game_st.map_tile_height) return false;
		if (u && !u_flying(u) && !u_grounded_building(u) && !is_reachable(u, xy(int(tile_pos.x * 32), int(tile_pos.y * 32)))) return false;

		int visibility_mask = 1 << owner;

		if (unit_is_refinery(unit_type)) {
			xy gas_placement_size = get_unit_type(UnitTypes::Resource_Vespene_Geyser)->placement_size;
			tile_size.x = gas_placement_size.x / 32u;
			tile_size.y = gas_placement_size.y / 32u;

			bool any_visible = false;
			for (size_t y = tile_pos.y; y != tile_pos.y + tile_size.y; ++y) {
				for (size_t x = tile_pos.x; x != tile_pos.x + tile_size.x; ++x) {
					auto& tile = st.tiles[y * game_st.map_tile_width + x];
					if (tile.explored & visibility_mask) return false;
					if (!any_visible && (tile.visible & visibility_mask) == 0) any_visible = true;
				}
			}
			if (!any_visible) return true;

			xy gas_pos(int(tile_pos.x * 32), int(tile_pos.y * 32));
			gas_pos += gas_placement_size / 2;
			return get_building_at_center_position(gas_pos, get_unit_type(UnitTypes::Resource_Vespene_Geyser)) != nullptr;
		}
		if (ut_requires_psionic_matrix(unit_type)) {
			if (!is_in_psionic_matrix(owner, pos)) return false;
		}
		bool is_nydus_exit = u && unit_is_nydus(u);
		if (ut_requires_creep(unit_type) || is_nydus_exit) {
			bool require_visibility = !check_invisible_tiles || is_nydus_exit;
			for (size_t y = tile_pos.y; y != tile_pos.y + tile_size.y; ++y) {
				for (size_t x = tile_pos.x; x != tile_pos.x + tile_size.x; ++x) {
					auto& tile = st.tiles[y * game_st.map_tile_width + x];
					if (tile.explored & visibility_mask) return false;
					if (require_visibility || ~tile.visible & visibility_mask) {
						if (~tile.flags & tile_t::flag_has_creep) return false;
					}
				}
			}
		} else {
			bool check_unk4 = !unit_is_non_flag_beacon(unit_type);
			bool is_resource_depot = ut_resource_depot(unit_type);
			int flags_mask = tile_t::flag_partially_walkable | tile_t::flag_unbuildable;
			if (unit_race(unit_type) != race_t::zerg) flags_mask |= tile_t::flag_has_creep;

			for (size_t y = tile_pos.y; y != tile_pos.y + tile_size.y; ++y) {
				for (size_t x = tile_pos.x; x != tile_pos.x + tile_size.x; ++x) {
					auto& tile = st.tiles[y * game_st.map_tile_width + x];
					if (tile.explored & visibility_mask) return false;
					if (tile.visible & visibility_mask || (tile.flags & flags_mask) == 0 || (check_unk4 && tile.flags & tile_t::flag_unk4)) {
						if (is_resource_depot) {
							rect bb{xy(32 * ((int)x - 3), 32 * ((int)y - 3)), xy(32 * ((int)x + 4), 32 * ((int)y + 4))};
							for (unit_t* n : find_units_noexpand(bb)) {
								if (ut_resource(n)) return false;
							}
						}
					} else return false;
				}
			}
		}

		for (size_t y = tile_pos.y; y != tile_pos.y + tile_size.y; ++y) {
			for (size_t x = tile_pos.x; x != tile_pos.x + tile_size.x; ++x) {
				auto& tile = st.tiles[y * game_st.map_tile_width + x];
				if (check_invisible_tiles || (tile.visible & visibility_mask) == 0) {
					if (tile.flags & tile_t::flag_occupied) {
						if (!u) return false;
						if (!u_grounded_building(u)) return false;
						if (unit_is_nydus(u)) return false;
						if (us_hidden(u)) return false;
						if (u->unit_finder_bounding_box.from.x >= int(x * 32 + 32)) return false;
						if (u->unit_finder_bounding_box.to.x <= int(x * 32)) return false;
						if (u->unit_finder_bounding_box.from.y >= int(y * 32 + 32)) return false;
						if (u->unit_finder_bounding_box.to.y <= int(y * 32)) return false;
					}
				}
			}
		}

		bool is_addon = ut_addon(unit_type);

		rect search_bb = unit_type_inner_bounding_box(unit_type, xy(int(tile_pos.x * 32), int(tile_pos.y * 32)) + unit_type->placement_size / 2);
		for (const unit_t* n : find_units_noexpand(search_bb)) {
			if (n == u) continue;
			if (is_addon && u_can_move(n)) continue;
			if (u_flying(n)) continue;
			if (u_grounded_building(n)) continue;
			if (unit_is(n, UnitTypes::Spell_Dark_Swarm)) continue;
			if (unit_is(n, UnitTypes::Spell_Disruption_Web)) continue;
			if (u && !check_undetected_units && unit_target_is_undetected(u, n)) continue;

			for (size_t y = tile_pos.y; y != tile_pos.y + tile_size.y; ++y) {
				for (size_t x = tile_pos.x; x != tile_pos.x + tile_size.x; ++x) {
					auto& tile = st.tiles[y * game_st.map_tile_width + x];
					if (check_invisible_tiles || (tile.visible & visibility_mask) == 0) {
						if (n->unit_finder_bounding_box.from.x >= int(x * 32 + 32)) continue;
						if (n->unit_finder_bounding_box.to.x <= int(x * 32)) continue;
						if (n->unit_finder_bounding_box.from.y >= int(y * 32 + 32)) continue;
						if (n->unit_finder_bounding_box.to.y <= int(y * 32)) continue;
						return false;
					}
				}
			}
		}

		return true;
	}

	void kill_unit(unit_t* u) {
		drop_carried_items(u);
		while (!u->order_queue.empty()) {
			remove_queued_order(u, &u->order_queue.front());
		}
		set_unit_order(u, get_order_type(Orders::Die), u->order_target.pos);
		on_kill_unit(u);
	}

	bool unit_can_enter_nydus(const unit_t* u, const unit_t* target) const {
		if (!unit_is_nydus(target)) return false;
		if (!u_completed(target)) return false;
		if (!target->building.nydus.exit || !u_completed(target->building.nydus.exit)) return false;
		if (u->owner != target->owner) return false;
		if (u_flying(u)) return false;
		if (unit_race(u) != race_t::zerg) return false;
		return true;
	}

	bool unit_can_use_shield_battery(const unit_t* u, const unit_t* target) const {
		if (!unit_is(target, UnitTypes::Protoss_Shield_Battery)) return false;
		if (!u_completed(target)) return false;
		if (!u->unit_type->has_shield) return false;
		if (!u_completed(u)) return false;
		if (!u_can_move(u)) return false;
		if (u_grounded_building(u)) return false;
		if (u->owner != target->owner) return false;
		if (unit_race(u) != race_t::protoss) return false;
		if (u->shield_points >= fp8::integer(u->unit_type->shield_points)) return false;
		if (target->energy == 0_fp8) return false;
		if (unit_is_disabled(target)) return false;
		if (unit_is(u, UnitTypes::Protoss_Interceptor) || unit_is(u, UnitTypes::Protoss_Scarab)) return false;
		return true;
	}

	bool unit_is_rescuable(const unit_t* u) const {
		if (st.players[u->owner].controller == player_t::controller_rescue_passive) return true;
		if (st.players[u->owner].controller == player_t::controller_unused_rescue_active) return true;
		return false;
	}

	bool try_follow_unit(unit_t* u, unit_t* target) {
		if (u->move_target_timer != 0) return true;
		if (!unit_target_is_visible(u, target)) {
			if (!unit_target_in_range(u, target, 32 * 14)) {
				if (unit_is_at_move_target(u) && !u_immovable(u)) {
					order_done(u);
					return false;
				}
				return true;
			}
		}
		move_to_target(u, target);
		return true;
	}

	void follow_ahead_of_unit(unit_t* u, unit_t* target) {
		if (u->move_target_timer != 0) return;
		if (unit_can_see_target(u, target) & u_movement_flag(target, 2)) {
			xy move_target = target->sprite->position;
			fp8 halt_distance = unit_halt_distance(u);
			xy pos = target->sprite->position + to_xy(direction_xy(target->next_velocity_direction, halt_distance * 3));
			if (is_in_map_bounds(pos)) move_target = pos;
			set_unit_move_target(u, move_target);
		} else {
			try_follow_unit(u, target);
		}
	}

	void auto_attack_target(unit_t* u, unit_t* target) {
		if (unit_is(target, UnitTypes::Protoss_Interceptor)) {
			if (target->fighter.parent) target = target->fighter.parent;
		}
		set_unit_order(u, u->unit_type->attack_unit, target);
		u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
		if (u->subunit) {
			if (ut_turret(u->subunit)) {
				set_unit_order(u->subunit, u->subunit->unit_type->attack_unit, target);
			}
			u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
		}
	}

	bool attack_move_movement(unit_t* u, bool stop_before_attacking = true) {
		if (u->order_state == 0) {
			set_unit_move_target(u, u->order_target.pos);
			set_next_target_waypoint(u, u->order_target.pos);
			u->order_state = 1;
		}
		unit_t* target = u->auto_target_unit;
		if (target && unit_target_is_enemy(u, target) && !us_hidden(target) && !u_invincible(target) && !unit_target_is_undetected(u, target)) {
			u->auto_target_unit = nullptr;
			if (stop_before_attacking) stop_unit(u);
			queue_order_front(u, u->order_type, {u->order_target.pos, u->order_target.unit});
			queue_order_front(u, u->unit_type->attack_unit, {target->sprite->position, target});
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
			u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
			if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
			return false;
		}
		return true;
	}

	void attack_move_acquire_target(unit_t* u, bool stop_before_attacking = true) {
		if (unit_target_acquisition_range(u) == 0) return;
		unit_t* target = find_acquire_target(u);
		if (target) {
			if (stop_before_attacking) stop_unit(u);
			queue_order_front(u, u->order_type, {u->order_target.pos, u->order_target.unit});
			queue_order_front(u, u->unit_type->attack_unit, {target->sprite->position, target});
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
			u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
			if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
		}
	}

	void attack_unit_reacquire_target(unit_t* u) {
		if (!u_ready_to_attack(u)) return;
		unit_t* target = u->order_target.unit;
		int target_priority = std::numeric_limits<int>::max();
		if (target) {
			if (unit_can_attack_target(u, target)) {
				target_priority = unit_target_attack_priority(u, target);
				if (target_priority == 0) {
					if (unit_target_in_weapon_movement_range(u, target)) return;
					target_priority = 1;
				}
			} else target = nullptr;
		}
		unit_t* auto_target = u->auto_target_unit;
		if (auto_target) {
			if (unit_can_attack_target(u, auto_target) && unit_target_is_enemy(u, auto_target)) {
				int auto_target_priority = unit_target_attack_priority(u, auto_target);
				if (auto_target_priority == 0) {
					if (!unit_target_in_weapon_movement_range(u, auto_target)) auto_target_priority = 1;
				}
				if (auto_target_priority < target_priority) {
					target_priority = auto_target_priority;
					target = auto_target;
				}
			} else u->auto_target_unit = nullptr;
		}
		if (!target || target_priority != 0) {
			unit_t* new_target = find_acquire_target(u);
			if (new_target && unit_target_attack_priority(u, new_target) < target_priority) {
				target = new_target;
			}
		}
		if (!target || unit_can_attack_target(u, target)) {
			u->order_target.unit = target;
			if (u->subunit) {
				if (u->subunit->order_type == u->subunit->unit_type->attack_unit || u->subunit->order_type->id == Orders::HoldPosition) {
					u->subunit->order_target.unit = target;
				}
			}
		}
	}

	bool attack_unit_move_in_range(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!unit_can_attack_target(u, target)) {
			if (!target || u_ready_to_attack(u)) {
				stop_unit(u);
				order_done(u);
			} else {
				queue_order_front(u, get_order_type(Orders::Move), target->sprite->position);
				order_done(u);
			}
			return false;
		}
		u->order_target.pos = target->sprite->position;
		set_next_target_waypoint(u, target->sprite->position);
		if (u->order_state == 0) {
			if (target && !unit_target_in_weapon_movement_range(u, target)) {
				move_to_target(u, target);
			}
			u->order_state = 1;
			return true;
		} else {
			if (unit_target_out_of_max_range(u, target)) {
				if ((unit_is_at_move_target(u) && u_immovable(u)) || (u_ready_to_attack(u) && !u_status_flag(u, unit_t::status_flag_8) && !u_flying(u) && u_flying(u->order_target.unit))) {
					stop_unit(u);
					set_next_target_waypoint(u, u->move_target.pos);
					order_done(u);
					return false;
				}
				if (unit_is(u, UnitTypes::Zerg_Scourge) || (u->unit_type->ground_weapon && u->unit_type->ground_weapon->flingy->id == (FlingyTypes)0)) {
					follow_ahead_of_unit(u, target);
				} else {
					try_follow_unit(u, target);
				}
				return false;
			} else {
				if (unit_is(u, UnitTypes::Zerg_Scourge)) {
					follow_ahead_of_unit(u, target);
				} else {
					stop_unit(u);
				}
				return true;
			}
		}
	}

	bool unit_can_fire_weapon(unit_t* u, const weapon_type_t* weapon) {
		if (u_movement_flag(u, 8)) return false;
		unit_t* target = u->order_target.unit;
		int distance;
		if (target) {
			distance = units_distance(unit_main_unit(u), target);
		} else {
			distance = unit_distance_to(unit_main_unit(u), u->order_target.pos);
		}
		if (weapon->min_range && distance < weapon->min_range) return false;
		if (distance > weapon_max_range(u, weapon)) return false;
		if (u->pathing_flags & 2) return false;
		if (u->position != u->next_target_waypoint) {
			if (unit_is(u, UnitTypes::Zerg_Lurker)) {
				u->heading = xy_direction(u->next_target_waypoint - u->sprite->position);
			} else if (fp8::extend(xy_direction(u->next_target_waypoint - u->sprite->position) - u->heading).abs() > weapon->attack_angle) {
				u->order_process_timer = 0;
				return false;
			}
		}
		return true;
	}

	void attack_unit_fire_weapon(unit_t* u) {
		st.prev_bullet_source_unit = nullptr;
		if (u_cannot_attack(u)) return;
		unit_t* target = u->order_target.unit;
		const weapon_type_t* weapon;
		int cooldown;
		int anim;
		if (target && u_flying(target)) {
			weapon = unit_air_weapon(u);
			cooldown = u->air_weapon_cooldown;
			anim = iscript_anims::AirAttkRpt;
		} else {
			weapon = unit_ground_weapon(u);
			cooldown = u->ground_weapon_cooldown;
			anim = iscript_anims::GndAttkRpt;
		}
		if (!weapon) return;
		if (cooldown != 0) {
			if (u->order_process_timer > cooldown - 1) u->order_process_timer = cooldown - 1;
			return;
		}
		if (!unit_can_fire_weapon(u, weapon)) return;
		u_set_movement_flag(u, 8);
		cooldown = get_modified_weapon_cooldown(u, weapon) + (lcg_rand(12) & 3) - 1;
		u->ground_weapon_cooldown = cooldown;
		u->air_weapon_cooldown = cooldown;
		sprite_run_anim(u->sprite, anim);
	}

	void set_unit_owner(unit_t* u, int owner, bool increment_score) {
		if (u->owner == owner) return;
		bool is_morphing = unit_is_morphing_building(u);
		if (is_morphing) {
			u_set_status_flag(u, unit_t::status_flag_completed);
		}
		increment_unit_counts(u, -1);
		if (u_completed(u)) add_completed_unit(u, -1, false);
		st.player_units[u->owner].remove(*u);
		u->owner = owner;
		st.player_units[owner].push_front(*u);
		increment_unit_counts(u, 1);
		if (u_completed(u)) add_completed_unit(u, 1, increment_score);
		if (is_morphing) u_unset_status_flag(u, unit_t::status_flag_completed);

		if (u->order_target.unit && u->order_type->targets_enemies && !unit_target_is_enemy(u, u->order_target.unit)) {
			u->order_target.unit = nullptr;
		}
		for (unit_t* n : ptr(st.visible_units)) {
			n = unit_attacking_unit(n);
			if (n->order_target.unit == u && n->order_type->targets_enemies && !unit_target_is_enemy(n, u)) {
				n->order_target.unit = nullptr;
			}
		}
		for (unit_t* n : ptr(st.hidden_units)) {
			n = unit_attacking_unit(n);
			if (n->order_target.unit == u && n->order_type->targets_enemies && !unit_target_is_enemy(n, u)) {
				n->order_target.unit = nullptr;
			}
		}
		if (unit_race(u) == race_t::protoss && u_grounded_building(u)) st.update_psionic_matrix = true;
		unit_t* turret = unit_turret(u);
		if (turret) set_unit_owner(u->subunit, owner, increment_score);
		update_unit_speed_upgrades(u);
		if (ut_worker(u) && u->worker.gather_target && unit_is(u->worker.gather_target, UnitTypes::Powerup_Flag)) {
			if (owner >= 8 || owner == u->worker.gather_target->owner) drop_carried_items(u);
		}
	}

	void set_sprite_owner(unit_t* u, int owner) {
		if (u->sprite->owner != owner) u->sprite->owner = owner;
		if (u->subunit && u->subunit->sprite->owner != owner) u->subunit->sprite->owner = owner;
	}

	void reinitialize_unit_type(unit_t* u, const unit_type_t* unit_type) {
		if (ut_worker(u) && u->worker.gather_target) {
			if (u->worker.is_gathering) {
				gather_queue_next(u, u->worker.gather_target);
			} else {
				u->worker.gather_target->building.resource.gather_queue.remove(*u);
				u->worker.gather_target = nullptr;
			}
		}
		xy position = u->sprite->position;
		auto prev_unit_type = u->unit_type;
		auto prev_hp = u->hp;
		auto prev_order_type = u->order_type;
		auto prev_order_target = u->order_target;
		auto prev_order_state = u->order_state;
		auto prev_sprite_owner = u->sprite->owner;
		int prev_resources = ut_resource(u) ? u->building.resource.resource_count : 0;
		std::array<int, 4> prev_larva_spawn_side_values;
		bool was_hatchery = unit_is_hatchery(u);
		if (was_hatchery) prev_larva_spawn_side_values = u->building.hatchery.larva_spawn_side_values;
		u->previous_unit_type = prev_unit_type;
		u->previous_hp = std::max((int)prev_hp.integer_part(), 1);
		u->order_type = get_order_type(Orders::Die);
		u->order_state = 1;
		destroy_sprite(u->sprite);
		u->sprite = nullptr;
		free_path(u);
		if (!initialize_unit_type(u, unit_type, position, u->owner)) error("reinitialize_unit_type: initialize_unit_type failed");
		int prev_max_hp = prev_unit_type->hitpoints.integer_part();
		int hp = prev_max_hp ? prev_hp.integer_part() * u->unit_type->hitpoints.integer_part() / prev_max_hp : 1;
		if (hp == 0) u->hp = 1_fp8;
		else u->hp = fp8::integer(hp);
		u->air_strength = get_unit_strength(u, false);
		u->ground_strength = get_unit_strength(u, true);
		u->sprite->owner = prev_sprite_owner;
		u_unset_status_flag(u, unit_t::status_flag_disabled);
		u_unset_status_flag(u, unit_t::status_flag_iscript_nobrk);
		u_set_status_flag(u, unit_t::status_flag_order_not_interruptible, !prev_order_type->can_be_interrupted);
		u->order_type = prev_order_type;
		u->order_target = prev_order_target;
		u->order_state = prev_order_state;
		if (u_grounded_building(u) && !u->build_queue.empty()) u->build_queue.erase(u->build_queue.begin());
		if (ut_resource(u)) set_unit_resources(u, prev_resources);
		if (was_hatchery && unit_is_hatchery(u)) u->building.hatchery.larva_spawn_side_values = prev_larva_spawn_side_values;
	}

	void morph_unit(unit_t* u, const unit_type_t* unit_type) {
		int visibility = u->sprite->visibility_flags;
		if (!us_hidden(u)) {
			unit_finder_remove(u);
			if (u_grounded_building(u)) set_unit_tiles_unoccupied(u, u->sprite->position);
			if (u_flying(u)) decrement_repulse_field(u);
		}
		bool requires_detector = u_requires_detector(u);
		bool cloaked = u_cloaked(u);
		int data1;
		int data2;
		if (requires_detector || cloaked) {
			data1 = u->sprite->main_image->modifier_data1;
			data2 = u->sprite->main_image->modifier_data2;
		}
		increment_unit_counts(u, -1);
		if (u_completed(u)) add_completed_unit(u, -1, true);
		reinitialize_unit_type(u, unit_type);
		increment_unit_counts(u, 1);
		if (u_completed(u)) add_completed_unit(u, 1, true);
		if (requires_detector || cloaked) {
			set_sprite_cloak_modifier(u->sprite, requires_detector, cloaked, u_burrowed(u), data1, data2);
		}
		set_sprite_visibility(u->sprite, visibility);
		if (ut_building(u) && (requires_detector || cloaked)) {
			set_secondary_order(u, get_order_type(Orders::Nothing));
			u_unset_status_flag(u, unit_t::status_flag_requires_detector);
			u_unset_status_flag(u, unit_t::status_flag_cloaked);
		}
		xy new_pos = restrict_move_target_to_valid_bounds(u, u->sprite->position);
		if (new_pos != u->sprite->position) move_unit(u, new_pos);
		if (!us_hidden(u)) {
			unit_finder_insert(u);
			if (u_grounded_building(u)) set_unit_tiles_occupied(u, u->sprite->position);
			check_unit_collision(u);
			if (u_flying(u)) increment_repulse_field(u);
		}
		if (u->subunit) {
			if (ut_turret(unit_type)) error("unit type %d has a turret but is also flagged as a turret", (int)unit_type->id);
			if (!ut_turret(unit_type->turret_unit_type)) error("unit type %d was created as a turret but is not flagged as one", (int)unit_type->turret_unit_type->id);

			requires_detector = u_requires_detector(u->subunit);
			cloaked = u_cloaked(u->subunit);
			int data1{};
			int data2{};
			if (requires_detector || cloaked) {
				data1 = u->subunit->sprite->main_image->modifier_data1;
				data2 = u->subunit->sprite->main_image->modifier_data2;
			}
			auto ius = make_thingy_setter(iscript_unit, u->subunit);
			if (!unit_type->turret_unit_type) error("unit_type->turret_unit_type is null");
			reinitialize_unit_type(u->subunit, unit_type->turret_unit_type);
			u->subunit->sprite->flags |= sprite_t::flag_turret;
			if (requires_detector || cloaked) {
				set_sprite_cloak_modifier(u->subunit->sprite, requires_detector, cloaked, u_burrowed(u), data1, data2);
			}
			set_image_offset(u->subunit->sprite->main_image, get_image_lo_offset(u->sprite->main_image, 2, 0));
			set_sprite_visibility(u->subunit->sprite, u->sprite->visibility_flags);
		}
		apply_unit_effects(u);
	}

	unit_t* build_refinery(const unit_t* u, const unit_type_t* unit_type) {
		unit_t* target = u->order_target.unit;
		if (!target || !unit_is(target, UnitTypes::Resource_Vespene_Geyser)) {
			target = get_building_at_center_position(u->order_target.pos, get_unit_type(UnitTypes::Resource_Vespene_Geyser));
			if (!target) return nullptr;
		}
		add_completed_unit(target, -1, true);
		u_unset_status_flag(target, unit_t::status_flag_completed);
		set_unit_owner(target, u->owner, false);
		set_sprite_owner(target, u->owner);
		morph_unit(target, unit_type);
		create_image(get_image_type(ImageTypes::IMAGEID_Vespene_Geyser2), target->sprite, {}, image_order_below);
		target->hp = unit_type->hitpoints / 10;
		return target;
	}

	bool unit_can_gather_gas(const unit_t* u, const unit_t* target) const {
		if (!ut_worker(u)) return false;
		if (!unit_is_refinery(target)) return false;
		if (!u_completed(target)) return false;
		if (target->owner != u->owner) return false;
		return true;
	}

	bool find_gas_exit_position(const unit_t* u, const unit_t* gas, xy& pos, direction_t& heading) {
		const unit_t* resource_depot = find_nearest_active_resource_depot(u);
		if (!resource_depot || !gas) {
			auto r = find_unit_placement(u, u->sprite->position, true);
			pos = r.second;
			heading = u->heading;
			return r.first;
		}
		heading = xy_direction(resource_depot->sprite->position - gas->sprite->position);
		xy offset = to_xy(direction_xy(heading - 64_dir, 32));
		xy from_pos = gas->sprite->position + offset;
		xy to_pos = resource_depot->sprite->position + offset;

		auto bb = unit_sprite_inner_bounding_box(gas);
		bb.from -= u->unit_type->dimensions.to + xy(1, 1);
		bb.to += u->unit_type->dimensions.from + xy(1, 1);
		get_unique_sided_positions_within_bounds(from_pos, to_pos, bb);
		auto r = find_unit_placement(u, to_pos, bb, true);
		if (!r.first) r = find_unit_placement(u, to_pos, true);
		pos = r.second;
		return r.first;
	}

	bool is_at_next_target_waypoint_or_within_attack_angle(const unit_t* u) const {
		if (u->position == u->next_target_waypoint) return true;
		auto* weapon = unit_ground_weapon(u);
		if (weapon) {
			auto heading_error = fp8::extend(u->heading - xy_direction(u->next_target_waypoint - u->sprite->position)).abs();
			if (heading_error <= weapon->attack_angle) return true;
		}
		return false;
	}

	void unit_load_target(unit_t* u, unit_t* target) {
		size_t index = (size_t)-1;
		for (size_t i = 0; i != u->unit_type->space_provided; ++i) {
			unit_t* n = get_unit(u->loaded_units.at(i));
			if (!n) {
				index = i;
				break;
			}
		}
		play_sound(40 + (int)unit_race(u), u);
		u->loaded_units.at(index) = get_unit_id(target);
		target->connected_unit = u;
		u_set_status_flag(target, unit_t::status_flag_loaded);
		hide_unit(target);
		sprite_run_anim(target->sprite, iscript_anims::WalkingToIdle);
		if (u_grounded_building(u)) {
			u_unset_status_flag(target, unit_t::status_flag_can_move);
			u_set_status_flag(target, unit_t::status_flag_in_bunker);
			reset_movement_state(target);
			unit_t* turret = unit_turret(target);
			if (turret) {
				move_unit(turret, u->sprite->position);
				u_unset_status_flag(turret, unit_t::status_flag_can_move);
				u_set_status_flag(turret, unit_t::status_flag_in_bunker);
				reset_movement_state(turret);
			} else move_unit(target, u->sprite->position);
		}
	}

	void unit_unload_impl(unit_t* u, bool container_destroyed) {
		if (!u_loaded(u)) return;
		unit_t* container = u->connected_unit;
		if (container) {
			auto uid = get_unit_id(u);
			size_t index = (size_t)-1;
			for (size_t i = 0; i != container->unit_type->space_provided; ++i) {
				if (container->loaded_units[i] == uid) {
					index = i;
					break;
				}
			}
			container->loaded_units.at(index) = unit_id();
			u->connected_unit = nullptr;
			u_unset_status_flag(u, unit_t::status_flag_loaded);

			if (container_destroyed) {
				move_unit(u, container->sprite->position);
			} else {
				show_unit(u);
				play_sound(43 + (int)unit_race(container), container);
			}
		}
		u_unset_status_flag(u, unit_t::status_flag_in_bunker);
		u_set_status_flag(u, unit_t::status_flag_can_move, ut_can_move(u));
		reset_movement_state(u);
		unit_t* turret = unit_turret(u);
		if (turret) {
			u_unset_status_flag(turret, unit_t::status_flag_in_bunker);
			u_set_status_flag(turret, unit_t::status_flag_can_move, ut_can_move(turret));
			reset_movement_state(turret);
		}
	}

	bool unit_unload(unit_t* u) {
		unit_t* container = u->connected_unit;
		if (!container || unit_is_disabled(container)) return false;
		if (!u_grounded_building(container) && container->main_order_timer) return false;
		xy pos = container->sprite->position;
		bool r;
		move_unit(u, pos);
		std::tie(r, pos) = find_unit_placement(u, pos, false);
		if (!r) return false;
		container->main_order_timer = 15;
		move_unit(u, pos);
		if (u->subunit) move_unit(u->subunit, pos);
		iscript_run_to_idle(u);
		unit_unload_impl(u, false);
		set_unit_order(u, u->unit_type->return_to_idle);
		if (!u_grounded_building(container)) {
			if (unit_is(u, UnitTypes::Protoss_Reaver)) u->main_order_timer = 30;
			else {
				auto* ground_weapon = unit_ground_weapon(u);
				auto* air_weapon = unit_air_weapon(u);
				if (ground_weapon) u->ground_weapon_cooldown = get_modified_weapon_cooldown(u, ground_weapon);
				if (air_weapon) u->air_weapon_cooldown = get_modified_weapon_cooldown(u, air_weapon);
				u->spell_cooldown = 30;
			}
		}
		return true;
	}

	bool trigger_give_unit_to(unit_t* u, int new_owner) {
		if (u_hallucination(u)) return false;
		if (unit_is_mineral_field(u)) return false;
		if (unit_is(u, UnitTypes::Resource_Vespene_Geyser)) return false;
		if (unit_is_fighter(u)) return false;
		if (ut_addon(u)) return false;
		if (u->owner == new_owner) return false;
		if (u_completed(u) && unit_is_refinery(u)) {
			for (unit_t* n : ptr(st.player_units[u->owner])) {
				if (!ut_worker(n)) continue;
				if (!us_hidden(n)) continue;
				if (n->order_type->id != Orders::HarvestGas) continue;
				if (n->order_target.unit != u) continue;
				give_unit_to(n, new_owner);
				break;
			}
		}
		give_unit_to(u, new_owner);
		if (unit_provides_space(u)) {
			for (unit_t* n : loaded_units(u)) {
				give_unit_to(n, new_owner);
			}
		}
		if (unit_is_carrier(u)) {
			for (unit_t* n : ptr(u->carrier.inside_units)) {
				give_unit_to(n, new_owner);
			}
			for (unit_t* n : ptr(u->carrier.outside_units)) {
				give_unit_to(n, new_owner);
			}
		} else if (unit_is_reaver(u)) {
			for (unit_t* n : ptr(u->reaver.inside_units)) {
				give_unit_to(n, new_owner);
			}
			for (unit_t* n : ptr(u->reaver.outside_units)) {
				give_unit_to(n, new_owner);
			}
		}
		if (u_grounded_building(u)) {
			if (u->building.addon) give_unit_to(u->building.addon, new_owner);
			if (unit_is_building_addon(u)) give_unit_to(u->current_build_unit, new_owner);
			if (unit_is_nydus(u) && u->building.nydus.exit) give_unit_to(u->building.nydus.exit, new_owner);
		}
		return true;
	}

	void give_unit_to(unit_t* u, int new_owner) {
		bool any_upgrades_granted = false;
		auto grant_tech = [&](TechTypes id) {
			if (!player_has_researched(new_owner, id)) st.tech_researched[new_owner][id] = true;
		};
		auto grant_upgrade = [&](UpgradeTypes id) {
			int level = player_upgrade_level(u->owner, id);
			if (player_upgrade_level(new_owner, id) < level) {
				any_upgrades_granted = true;
				st.upgrade_levels[new_owner][id] = level;
			}
		};
		switch (u->unit_type->id) {
		case UnitTypes::Terran_Marine:
			grant_upgrade(UpgradeTypes::U_238_Shells);
			grant_tech(TechTypes::Stim_Packs);
			break;
		case UnitTypes::Terran_Ghost:
			grant_upgrade(UpgradeTypes::Ocular_Implants);
			grant_upgrade(UpgradeTypes::Moebius_Reactor);
			grant_tech(TechTypes::Lockdown);
			grant_tech(TechTypes::Personnel_Cloaking);
			break;
		case UnitTypes::Terran_Vulture:
			grant_upgrade(UpgradeTypes::Ion_Thrusters);
			grant_tech(TechTypes::Spider_Mines);
			break;
		case UnitTypes::Terran_Goliath:
			grant_upgrade(UpgradeTypes::Charon_Boosters);
			break;
		case UnitTypes::Terran_Siege_Tank_Tank_Mode:
			grant_tech(TechTypes::Tank_Siege_Mode);
			break;
		case UnitTypes::Terran_Wraith:
			grant_upgrade(UpgradeTypes::Apollo_Reactor);
			grant_tech(TechTypes::Cloaking_Field);
			break;
		case UnitTypes::Terran_Science_Vessel:
			grant_upgrade(UpgradeTypes::Titan_Reactor);
			grant_tech(TechTypes::Defensive_Matrix);
			grant_tech(TechTypes::Irradiate);
			grant_tech(TechTypes::EMP_Shockwave);
			break;
		case UnitTypes::Terran_Battlecruiser:
			grant_upgrade(UpgradeTypes::Colossus_Reactor);
			grant_tech(TechTypes::Yamato_Gun);
			break;
		case UnitTypes::Terran_Siege_Tank_Siege_Mode:
			grant_tech(TechTypes::Tank_Siege_Mode);
			break;
		case UnitTypes::Terran_Firebat:
			grant_tech(TechTypes::Stim_Packs);
			break;
		case UnitTypes::Terran_Medic:
			grant_upgrade(UpgradeTypes::Caduceus_Reactor);
			grant_tech(TechTypes::Healing);
			grant_tech(TechTypes::Restoration);
			grant_tech(TechTypes::Optical_Flare);
			break;
		case UnitTypes::Zerg_Zergling:
			grant_upgrade(UpgradeTypes::Metabolic_Boost);
			grant_upgrade(UpgradeTypes::Adrenal_Glands);
			grant_tech(TechTypes::Burrowing);
			break;
		case UnitTypes::Zerg_Hydralisk:
			grant_upgrade(UpgradeTypes::Muscular_Augments);
			grant_upgrade(UpgradeTypes::Grooved_Spines);
			grant_tech(TechTypes::Burrowing);
			grant_tech(TechTypes::Lurker_Aspect);
			break;
		case UnitTypes::Zerg_Ultralisk:
			grant_upgrade(UpgradeTypes::Anabolic_Synthesis);
			grant_upgrade(UpgradeTypes::Chitinous_Plating);
			break;
		case UnitTypes::Zerg_Drone:
			grant_tech(TechTypes::Burrowing);
			break;
		case UnitTypes::Zerg_Overlord:
			grant_upgrade(UpgradeTypes::Ventral_Sacs);
			grant_upgrade(UpgradeTypes::Antennae);
			grant_upgrade(UpgradeTypes::Pneumatized_Carapace);
			break;
		case UnitTypes::Zerg_Queen:
			grant_upgrade(UpgradeTypes::Gamete_Meiosis);
			grant_tech(TechTypes::Infestation);
			grant_tech(TechTypes::Parasite);
			grant_tech(TechTypes::Spawn_Broodlings);
			grant_tech(TechTypes::Ensnare);
			break;
		case UnitTypes::Zerg_Defiler:
			grant_upgrade(UpgradeTypes::Metasynaptic_Node);
			grant_tech(TechTypes::Burrowing);
			grant_tech(TechTypes::Dark_Swarm);
			grant_tech(TechTypes::Plague);
			grant_tech(TechTypes::Consume);
			break;
		case UnitTypes::Zerg_Infested_Terran:
			grant_tech(TechTypes::Burrowing);
			break;
		case UnitTypes::Protoss_Corsair:
			grant_upgrade(UpgradeTypes::Argus_Jewel);
			grant_tech(TechTypes::Disruption_Web);
			break;
		case UnitTypes::Protoss_Dark_Templar:
			grant_tech(TechTypes::Dark_Archon_Meld);
			break;
		case UnitTypes::Protoss_Dark_Archon:
			grant_upgrade(UpgradeTypes::Argus_Talisman);
			grant_tech(TechTypes::Mind_Control);
			grant_tech(TechTypes::Feedback);
			grant_tech(TechTypes::Maelstrom);
			break;
		case UnitTypes::Protoss_Zealot:
			grant_upgrade(UpgradeTypes::Leg_Enhancements);
			break;
		case UnitTypes::Protoss_Dragoon:
			grant_upgrade(UpgradeTypes::Singularity_Charge);
			break;
		case UnitTypes::Protoss_High_Templar:
			grant_upgrade(UpgradeTypes::Khaydarin_Amulet);
			grant_tech(TechTypes::Archon_Warp);
			grant_tech(TechTypes::Psionic_Storm);
			grant_tech(TechTypes::Hallucination);
			break;
		case UnitTypes::Protoss_Shuttle:
			grant_upgrade(UpgradeTypes::Gravitic_Drive);
			break;
		case UnitTypes::Protoss_Scout:
			grant_upgrade(UpgradeTypes::Apial_Sensors);
			grant_upgrade(UpgradeTypes::Gravitic_Thrusters);
			break;
		case UnitTypes::Protoss_Arbiter:
			grant_upgrade(UpgradeTypes::Khaydarin_Core);
			grant_tech(TechTypes::Recall);
			grant_tech(TechTypes::Stasis_Field);
			break;
		case UnitTypes::Protoss_Carrier:
			grant_upgrade(UpgradeTypes::Carrier_Capacity);
			break;
		case UnitTypes::Protoss_Reaver:
			grant_upgrade(UpgradeTypes::Scarab_Damage);
			grant_upgrade(UpgradeTypes::Reaver_Capacity);
			break;
		case UnitTypes::Protoss_Observer:
			grant_upgrade(UpgradeTypes::Sensor_Array);
			grant_upgrade(UpgradeTypes::Gravitic_Boosters);
			break;
		case UnitTypes::Zerg_Lurker:
			grant_tech(TechTypes::Lurker_Aspect);
			break;
		default:
			break;
		}
		on_unit_deselect(u);
		if (u_grounded_building(u)) {
			if (!u->build_queue.empty() && u->build_queue.front()->id <= UnitTypes::Spell_Disruption_Web) {
				cancel_build_queue(u);
				sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
			}
			if (unit_is_researching(u)) cancel_research(u);
			if (unit_is_upgrading(u)) cancel_upgrade(u);
		}
		if (unit_is_reaver(u) || unit_is_carrier(u)) cancel_build_queue(u);
		set_unit_owner(u, new_owner, true);
		if (new_owner < 8) set_sprite_owner(u, new_owner);
		if (any_upgrades_granted) apply_upgrades_to_player_units(new_owner);
		bool idle = u_completed(u);
		if (u_loaded(u)) idle = false;
		else if (u_grounded_building(u) && u->secondary_order_type->id == Orders::BuildAddon && u->current_build_unit && !u_completed(u->current_build_unit)) idle = false;
		else if (ut_powerup(u)) idle = false;
		else if (unit_is(u, UnitTypes::Protoss_Interceptor)) idle = false;
		else if (unit_is(u, UnitTypes::Protoss_Scarab)) idle = false;
		else if (unit_is(u, UnitTypes::Terran_Nuclear_Missile)) idle = false;
		if (idle) {
			if (st.players[new_owner].controller == player_t::controller_rescue_passive) {
				set_unit_order(u, get_order_type(Orders::RescuePassive));
			} else if (st.players[new_owner].controller == player_t::controller_neutral) {
				set_unit_order(u, get_order_type(Orders::Neutral));
			} else {
				set_unit_order(u, u->unit_type->human_ai_idle);
			}
		}

	}

	void make_unit_neutral(unit_t* u) {
		if (u->secondary_order_type->id == Orders::BuildAddon && u_grounded_building(u) && u->current_build_unit) {
			if (!u_completed(u->current_build_unit)) cancel_building_unit(u->current_build_unit);
		}
		give_unit_to(u, 11);
		if (!u_hallucination(u)) set_secondary_order(u, get_order_type(Orders::Nothing));
	}

	void building_abandon_addon(unit_t* u) {
		unit_t* addon = u->building.addon;
		if (!addon) return;
		make_unit_neutral(addon);
		u->building.addon = nullptr;
		// todo: callback for sound
		auto ius = make_thingy_setter(iscript_unit, addon);
		sprite_run_anim(addon->sprite, iscript_anims::LiftOff);
	}

	void create_dust_sprite(sprite_t* sprite, SpriteTypes sprite_id, size_t lo_index, size_t offset_from, size_t offset_to, bool flipped) {
		for (size_t i = offset_from; i != offset_to + 1; ++i) {
			xy offset = get_image_lo_offset(sprite->main_image, lo_index, i);
			if (offset.x != 127 && offset.y != 127) {
				auto* t = create_thingy(get_sprite_type(sprite_id), sprite->position + offset, 0);
				if (t) {
					t->sprite->elevation_level = sprite->elevation_level + 1;
					if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
					if (flipped) {
						for (auto* image : ptr(t->sprite->images)) {
							set_image_frame_index_offset(image, image->frame_index_offset, true);
						}
					}
				}
			}
			sprite_id = SpriteTypes((int)sprite_id + 1);
		}
	}

	bool unit_can_attach_addon(const unit_t* u, const unit_t* addon) const {
		if (unit_is(addon, UnitTypes::Terran_Comsat_Station)) {
			if (unit_is(u, UnitTypes::Terran_Command_Center)) return true;
		} else if (unit_is(addon, UnitTypes::Terran_Nuclear_Silo)) {
			if (unit_is(u, UnitTypes::Terran_Command_Center)) return true;
		} else if (unit_is(addon, UnitTypes::Terran_Control_Tower)) {
			if (unit_is(u, UnitTypes::Terran_Starport)) return true;
		} else if (unit_is(addon, UnitTypes::Terran_Covert_Ops)) {
			if (unit_is(u, UnitTypes::Terran_Science_Facility)) return true;
		} else if (unit_is(addon, UnitTypes::Terran_Physics_Lab)) {
			if (unit_is(u, UnitTypes::Terran_Science_Facility)) return true;
		} else if (unit_is(addon, UnitTypes::Terran_Machine_Shop)) {
			if (unit_is(u, UnitTypes::Terran_Factory)) return true;
		}
		return false;
	}

	unit_t* find_connecting_addon(const unit_t* u) const {
		rect bb;
		bb.from = u->sprite->position - u->unit_type->placement_size / 2 - xy(32, 32);
		bb.to = bb.from + u->unit_type->placement_size + xy(64, 64);
		xy pos = u->sprite->position - u->unit_type->placement_size / 2;
		return find_unit_noexpand(bb, [&](unit_t* n) {
			if (!ut_addon(n)) return false;
			if (n->owner != 11) return false;
			if (!unit_can_attach_addon(u, n)) return false;
			xy npos = n->sprite->position - n->unit_type->placement_size / 2 - n->unit_type->addon_position;
			return pos / 32 == npos / 32;
		});
	}

	void connect_addon(unit_t* u, unit_t* addon) {
		u->building.addon = addon;
		set_unit_owner(addon, u->owner, false);
		set_sprite_owner(addon, u->owner);
		auto ius = make_thingy_setter(iscript_unit, addon);
		sprite_run_anim(addon->sprite, iscript_anims::Landing);
		if (unit_is(addon, UnitTypes::Terran_Nuclear_Silo)) {
			if (addon->building.silo.nuke) set_unit_owner(addon->building.silo.nuke, u->owner, false);
		}
	}

	void find_and_connect_addon(unit_t* u) {
		unit_t* addon = find_connecting_addon(u);
		if (addon) connect_addon(u, addon);
	}

	void cancel_research(unit_t* u, bool unit_destroyed = false) {
		if (!u->building.researching_type) return;
		auto* tech_type = u->building.researching_type;
		u->building.researching_type = nullptr;
		u->building.upgrade_research_time = 0;
		st.tech_researching[u->owner][tech_type->id] = false;
		if (unit_destroyed) {
			st.current_minerals[u->owner] += tech_type->mineral_cost * 3 / 4;
			st.current_gas[u->owner] += tech_type->gas_cost * 3 / 4;
		} else {
			st.current_minerals[u->owner] += tech_type->mineral_cost;
			st.current_gas[u->owner] += tech_type->gas_cost;
			sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
		}
	}

	void cancel_upgrade(unit_t* u, bool unit_destroyed = false) {
		if (!u->building.upgrading_type) return;
		auto* upgrade_type = u->building.upgrading_type;
		u->building.upgrading_type = nullptr;
		u->building.upgrade_research_time = 0;
		u->building.upgrading_level = 0;
		st.upgrade_upgrading[u->owner][upgrade_type->id] = false;
		if (unit_destroyed) {
			st.current_minerals[u->owner] += upgrade_mineral_cost(u->owner, upgrade_type) * 3 / 4;
			st.current_gas[u->owner] += upgrade_gas_cost(u->owner, upgrade_type) * 3 / 4;
		} else {
			st.current_minerals[u->owner] += upgrade_mineral_cost(u->owner, upgrade_type);
			st.current_gas[u->owner] += upgrade_gas_cost(u->owner, upgrade_type);
			sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
		}
	}

	void apply_upgrades_to_player_units(int owner) {
		for (unit_t* u : ptr(st.player_units[owner])) {
			update_unit_speed_upgrades(u);
		}
	}

	void set_unit_burrowed(unit_t* u) {
		reset_movement_state(u);
		u_set_status_flag(u, unit_t::status_flag_burrowed);
		u_set_status_flag(u, unit_t::status_flag_passively_cloaked);
		u->sprite->flags |= sprite_t::flag_burrowed;
		u->sprite->elevation_level = 1;
		if (unit_is(u, UnitTypes::Zerg_Lurker)) {
			u->sprite->sprite_type = get_sprite_type(SpriteTypes::SPRITEID_White_Circle2);
		} else {
			u->sprite->sprite_type = get_sprite_type(SpriteTypes::SPRITEID_White_Circle_Invisible);
		}
	}

	void set_unit_cloaked(unit_t* u) {
		++u->cloak_counter;
		if (u->cloak_counter == 1 && !u_requires_detector(u) && !u->cloaked_unit_link.first) {
			st.cloaked_units.push_front(*u);
		}
	}

	void crappy_move_to_target(unit_t* u, const unit_t* target) {
		xy target_pos = target->sprite->position;
		int speed = (u->flingy_top_speed * 2).integer_part();
		if (target->position != target->move_target.pos && speed != 0) {
			if (u_movement_flag(target, 4)) {
				target_pos = target->move_target.pos;
			} else {
				auto distance = [&](xy vec) {
					unsigned int x = std::abs(vec.x);
					unsigned int y = std::abs(vec.y);
					if (x < y) std::swap(x, y);
					if (x / 4 < y) return x + x / 32 + y * 3 / 8 - x / 32 + y * 3 / 64 - x / 128 + y * 3 / 256;
					else return x + x / 32 + y * 3 / 256;
				};
				int t = distance(target->sprite->position - u->sprite->position) / speed;
				fp8 pred_speed;
				if (u->flingy_movement_type == 2) pred_speed = target->flingy_top_speed;
				else pred_speed = std::min(target->next_speed + target->flingy_acceleration * t / 2, target->flingy_top_speed);
				auto pred_distance = pred_speed * t;
				target_pos = target->sprite->position + to_xy(direction_xy(target->next_velocity_direction, pred_distance));
				target_pos = restrict_pos_to_map_bounds(target_pos);
			}
		}
		set_flingy_move_target(u, target_pos);
		set_next_target_waypoint(u, target_pos);
	}

	const order_type_t* get_default_gather_order(const unit_t* u, const unit_t* target) const {
		if (ut_flag(target, (unit_type_t::flags_t)0x800)) return get_order_type(Orders::Move);
		if (unit_is_mineral_field(target)) {
			if ((u->carrying_flags & ~3) == 0) return get_order_type(Orders::MoveToMinerals);
			else return get_order_type(Orders::Move);
		} else if (unit_can_gather_gas(u, target) || unit_is(target, UnitTypes::Resource_Vespene_Geyser)) {
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
				if (unit_is_special_beacon(target) || ut_powerup(target)) return get_order_type(Orders::Move);
				else if (unit_target_is_enemy(u, target)) return get_order_type(Orders::AttackDefault);
				else if (unit_provides_space(target) && unit_can_load_target(target, u)) return get_order_type(Orders::EnterTransport);
				else if (u_burrowed(target)) return get_order_type(Orders::Move);
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
		case 3:
			if (!target || unit_is_special_beacon(target) || ut_powerup(target)) return u->unit_type->return_to_idle;
			else if (unit_target_is_enemy(u, target)) return get_order_type(Orders::AttackDefault);
			else return u->unit_type->return_to_idle;
		case 4:
			if (auto* o = target ? get_default_gather_order(u, target) : nullptr) return o;
			else return get_default_order(1, u, pos, target, target_unit_type);
		case 5:
			if (!target) return get_default_order(1, u, pos, target, target_unit_type);
			order = get_default_gather_order(u, target);
			if (order) return order;
			if (u_grounded_building(target) && !u_completed(target)) {
				if (target->owner == u->owner && unit_race(target) == race_t::terran) {
					if (!ut_addon(target)) {
						if (!target->connected_unit || target->connected_unit->order_target.unit != target) {
							return get_order_type(Orders::ConstructingBuilding);
						}
					}
				}
			} else if (st.alliances[u->owner][target->owner]) {
				bool can_enter = unit_provides_space(target) && unit_can_load_target(target, u);
				if ((ut_building(target) || !can_enter) && unit_race(target) == race_t::terran && ut_mechanical(target) && u_completed(target) && target->hp < target->unit_type->hitpoints) {
					return get_order_type(Orders::Repair);
				} else if (can_enter) {
					return get_order_type(Orders::EnterTransport);
				}
			}
			return get_default_order(1, u, pos, target, target_unit_type);
		case 6:
			return nullptr;
		default:
			error("get_default_order: unknown action %u", action);
		}
		return nullptr;
	}

	size_t default_action(unit_t* u) {
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
		return action;
	}

	bool spell_cast_target_movement(unit_t* u, const tech_type_t* tech, int range) {
		unit_t* target = u->order_target.unit;
		if (!target || !spell_order_valid(u)) {
			// todo: callback for error
			order_done(u);
			return false;
		}
		if (u->energy < fp8::integer(tech->energy_cost)) {
			// todo: callback for error
			order_done(u);
			return false;
		}
		u->order_target.pos = target->sprite->position;
		set_next_target_waypoint(u, target->sprite->position);
		if (unit_can_see_target(u, target)) {
			if (u_movement_flag(u, 2)) {
				if (!u_movement_flag(target, 2) || fp8::extend(target->next_velocity_direction - u->next_velocity_direction).abs() > 32_fp8) {
					range += unit_halt_distance(u).integer_part();
				}
			}
			if (unit_target_in_range(u, target, range)) {
				stop_unit(u);
				return is_facing_next_target_waypoint(u, 5_fp8);
			}
		}
		if (unit_is_at_move_target(u) && u_immovable(u)) {
			stop_unit(u);
			order_done(u);
			return false;
		}
		try_follow_unit(u, target);
		return false;
	}

	void set_remove_timer(unit_t* u, int timer) {
		if (u->remove_timer == 0 || timer < u->remove_timer) u->remove_timer = timer;
	}

	int default_remove_timer(const unit_t* u) const {
		if (u_hallucination(u)) return 1350;
		if (unit_is(u, UnitTypes::Zerg_Broodling)) return 1800;
		return 0;
	}

	void set_remove_timer(unit_t* u) {
		set_remove_timer(u, default_remove_timer(u));
	}

	void make_unit_hallucination(unit_t* u) {
		increment_unit_counts(u, -1);
		if (u_completed(u)) add_completed_unit(u, -1, true);
		u_set_status_flag(u, unit_t::status_flag_hallucination);
		u_set_status_flag(u, unit_t::status_flag_completed);
		increment_unit_counts(u, 1);
		add_completed_unit(u, 1, true);
		if (!ut_turret(u)) {
			set_remove_timer(u);
			u->air_strength = get_unit_strength(u, false);
			u->ground_strength = get_unit_strength(u, true);
			set_secondary_order(u, get_order_type(Orders::Hallucination2));
			if (u->subunit) {
				increment_unit_counts(u->subunit, -1);
				add_completed_unit(u->subunit, -1, true);
				u_set_status_flag(u->subunit, unit_t::status_flag_hallucination);
				increment_unit_counts(u->subunit, 1);
				add_completed_unit(u->subunit, 1, true);
			}
			if (unit_is_carrier(u)) {
				kill_interceptors(u);
			} else if (unit_is_reaver(u)) {
				for (unit_t* n : ptr(u->reaver.inside_units)) {
					make_unit_hallucination(n);
				}
			}
		}
	}

	bool unit_can_see_order_target(const unit_t* u) const {
		if (u->order_target.unit) return unit_can_see_target(u, u->order_target.unit);
		return player_position_is_visible(u->owner, u->order_target.pos);
	}

	bool medic_can_heal_target(const unit_t* u, const unit_t* target) const {
		if (!target || target == u) return false;
		if (!ut_organic(target) || ut_building(target)) return false;
		if (u_flying(target) || u_hallucination(target)) return false;
		if (target->hp >= target->unit_type->hitpoints) return false;
		if (unit_target_is_enemy(u, target)) return false;
		if (u->owner >= 8 || target->owner >= 8) return false;
		if (unit_is_disabled(target)) return false;
		return true;
	}

	unit_t* find_medic_target(const unit_t* u) const {
		if (!unit_can_use_tech(u, get_tech_type(TechTypes::Healing))) return nullptr;
		return find_nearest_unit(u, square_at(u->sprite->position, 160), [&](unit_t* target) {
			if (target->is_being_healed) return false;
			return medic_can_heal_target(u, target);
		});
	}

	int medic_try_heal(unit_t* u) {
		if (!unit_can_use_tech(u, get_tech_type(TechTypes::Healing))) return 0;
		unit_t* target = u->order_target.unit;
		if (!target || !medic_can_heal_target(u, target)) return 0;
		if (target->is_being_healed) {
			target = find_medic_target(u);
			if (!target) return 0;
			u->order_target.unit = target;
			u->order_target.pos = target->sprite->position;
			move_to_target(u, target);
		}
		if (!unit_target_in_range(u, target, 30)) return 3;
		fp8 heal_amount = target->unit_type->hitpoints - target->hp;
		if (heal_amount > 200_fp8) heal_amount = 200_fp8;
		fp8 energy_required = heal_amount / 2;
		if (u->energy < energy_required) return 2;
		u->energy -= energy_required;
		set_unit_hp(target, target->hp + heal_amount);
		target->is_being_healed = true;
		return 1;
	}

	bool tile_has_creep(xy pos) {
		return (st.tiles[tile_index(pos)].flags & tile_t::flag_has_creep) != 0;
	}

	void order_SelfDestructing(unit_t* u) {
		kill_unit(u);
	}

	void copy_status_effects(unit_t* to, unit_t* from) {
		to->remove_timer = from->remove_timer;
		to->defensive_matrix_hp = from->defensive_matrix_hp;
		to->defensive_matrix_timer = from->defensive_matrix_timer;
		to->stim_timer = from->stim_timer;
		to->ensnare_timer = from->ensnare_timer;
		to->lockdown_timer = from->lockdown_timer;
		to->irradiate_timer = from->irradiate_timer;
		to->stasis_timer = from->stasis_timer;
		to->plague_timer = from->plague_timer;
		to->storm_timer = from->storm_timer;
		to->irradiated_by = from->irradiated_by;
		to->irradiate_owner = from->irradiate_owner;
		to->parasite_flags = from->parasite_flags;
		to->cycle_counter = from->cycle_counter;
		to->blinded_by = from->blinded_by;
		to->maelstrom_timer = from->maelstrom_timer;
		to->acid_spore_count = from->acid_spore_count;
		to->acid_spore_time = from->acid_spore_time;
		apply_unit_effects(to);
		update_unit_speed(to);
	}

	void merge_status_effects(unit_t* to, unit_t* from) {
		to->defensive_matrix_hp = std::max(to->defensive_matrix_hp, from->defensive_matrix_hp);
		to->defensive_matrix_timer = std::max(to->defensive_matrix_timer, from->defensive_matrix_timer);
		to->stim_timer = std::max(to->stim_timer, from->stim_timer);
		to->ensnare_timer = std::max(to->ensnare_timer, from->ensnare_timer);
		to->lockdown_timer = std::max(to->lockdown_timer, from->lockdown_timer);
		to->stasis_timer = std::max(to->stasis_timer, from->stasis_timer);
		to->plague_timer = std::max(to->plague_timer, from->plague_timer);
		if (from->irradiate_timer > to->irradiate_timer) {
			to->irradiate_timer = from->irradiate_timer;
			to->irradiated_by = from->irradiated_by;
			to->irradiate_owner = from->irradiate_owner;
		}
		to->parasite_flags |= from->parasite_flags;
		to->blinded_by |= from->blinded_by;
		to->maelstrom_timer = std::max(from->maelstrom_timer, to->maelstrom_timer);
		to->acid_spore_count = from->acid_spore_count;
		to->acid_spore_time = from->acid_spore_time;
	}

	bool creep_is_being_sustained(xy_t<size_t> tile_pos) {
		xy pos((int)tile_pos.x * 32 + 16, (int)tile_pos.y * 32 + 16);
		rect area;
		area.from = pos - xy(640, 400);
		area.to = pos + xy(640, 400);
		if (area.from.x < 0) area.from.x = 0;
		else if (area.to.x >= (int)game_st.map_width) area.to.x = (int)game_st.map_width - 1;
		if (area.from.y < 0) area.from.y = 0;
		else if (area.to.y >= (int)game_st.map_height) area.to.y = (int)game_st.map_height - 1;
		for (const unit_t* u : find_units_noexpand(area)) {
			if (!u_grounded_building(u)) continue;
			if (!ut_building(u)) continue;
			if (unit_race(u) != race_t::zerg) continue;
			auto bb = get_max_creep_bb(u, u->sprite->position, u_completed(u));
			if (tile_pos.x < bb.from.x) continue;
			if (tile_pos.y < bb.from.y) continue;
			if (tile_pos.x > bb.to.x) continue;
			if (tile_pos.y > bb.to.y) continue;
			xy diff = pos - u->sprite->position;
			int d = diff.x*diff.x * 25 + diff.y*diff.y * 64;
			if (d > 320*320 * 25) continue;
			return true;
		}
		return false;
	}

	bool set_creep_receding(xy_t<size_t> tile_pos) {
		if (st.creep_life.free_list.empty()) return false;
		auto* v = &st.creep_life.free_list.front();
		st.creep_life.free_list.pop_front();
		--st.creep_life.free_list_size;
		size_t n_neighbors = count_neighboring_creep_tiles(tile_pos);
		v->n_neighboring_creep_tiles = n_neighbors;
		v->tile_pos = tile_pos;
		st.creep_life.lists[n_neighbors].push_front(*v);
		++st.creep_life.lists_size[n_neighbors];
		st.creep_life.table.insert(v);

		size_t index = tile_pos.y * game_st.map_tile_width + tile_pos.x;
		st.tiles[index].flags |= tile_t::flag_creep_receding;
		return true;
	}

	bool remove_creep_provider(unit_type_autocast unit_type, xy pos, bool is_completed) {
		int flags_mask = tile_t::flag_creep_receding;
		if (!unit_is(unit_type, UnitTypes::Zerg_Extractor)) flags_mask |= tile_t::flag_occupied;
		bool spreads_creep = unit_type_spreads_creep(unit_type, is_completed);
		auto area = get_max_creep_bb(unit_type, pos, is_completed);
		int dy = (int)area.from.y * 32 - pos.y + 16;
		for (size_t tile_y = area.from.y; tile_y != area.to.y + 1; ++tile_y, dy += 32) {
			int dx = (int)area.from.x * 32 - pos.x + 16;
			for (size_t tile_x = area.from.x; tile_x != area.to.x + 1; ++tile_x, dx += 32) {
				size_t index = tile_y * game_st.map_tile_width + tile_x;
				if (~st.tiles[index].flags & tile_t::flag_has_creep) continue;
				if (st.tiles[index].flags & flags_mask) continue;
				if (spreads_creep) {
					int d = dx*dx * 25 + dy*dy * 64;
					if (d > 320*320 * 25) continue;
				}
				if (!creep_is_being_sustained({tile_x, tile_y})) {
					if (!set_creep_receding({tile_x, tile_y})) return false;
				}
			}
		}
		st.creep_life.check_dead_unit_timer = 0;
		return true;
	}

	bool remove_creep_provider(const unit_t* u) {
		return remove_creep_provider(u, u->position, u_completed(u));
	}

	bool remove_extractor_creep(unit_type_autocast unit_type, xy pos, bool is_completed) {
		bool spreads_creep = unit_type_spreads_creep(unit_type, is_completed);
		auto area = get_max_creep_bb(unit_type, pos, is_completed);
		int dy = (int)area.from.y * 32 - pos.y + 16;
		for (size_t tile_y = area.from.y; tile_y != area.to.y + 1; ++tile_y, dy += 32) {
			int dx = (int)area.from.x * 32 - pos.x + 16;
			for (size_t tile_x = area.from.x; tile_x != area.to.x + 1; ++tile_x, dx += 32) {
				size_t index = tile_y * game_st.map_tile_width + tile_x;
				if (~st.tiles[index].flags & tile_t::flag_has_creep) continue;
				if (spreads_creep) {
					int d = dx*dx * 25 + dy*dy * 64;
					if (d > 320*320 * 25) continue;
				}
				set_tile_creep({tile_x, tile_y}, false);
			}
		}
		st.creep_life.check_dead_unit_timer = 0;
		return true;
	}

	void add_creep_provider(unit_t* u) {
		xy pos = u->position;
		bool spreads_creep = unit_type_spreads_creep(u, u_completed(u));
		auto area = get_max_creep_bb(u, pos, u_completed(u));
		int dy = (int)area.from.y * 32 - pos.y + 16;
		for (size_t tile_y = area.from.y; tile_y != area.to.y + 1; ++tile_y, dy += 32) {
			int dx = (int)area.from.x * 32 - pos.x + 16;
			for (size_t tile_x = area.from.x; tile_x != area.to.x + 1; ++tile_x, dx += 32) {
				size_t index = tile_y * game_st.map_tile_width + tile_x;
				auto flags = st.tiles[index].flags;
				if (~flags & tile_t::flag_creep_receding) continue;
				if (spreads_creep) {
					int d = dx*dx * 25 + dy*dy * 64;
					if (d > 320*320 * 25) continue;
				}
				auto* v = st.creep_life.table.find({tile_x, tile_y});
				if (!v) continue;
				if (!v) error("add_creep_provider: receding creep not found");
				st.creep_life.table.remove(v);
				st.creep_life.lists[v->n_neighboring_creep_tiles].remove(*v);
				--st.creep_life.lists_size[v->n_neighboring_creep_tiles];
				v->n_neighboring_creep_tiles = 9;
				st.creep_life.free_list.push_front(*v);
				++st.creep_life.free_list_size;

				st.tiles[index].flags &= ~tile_t::flag_creep_receding;
			}
		}
	}

	void zerg_building_start_construction(unit_t* u) {
		add_creep_provider(u);
		set_construction_graphic(u, true);
		u->hp = u->unit_type->hitpoints / 10;
		if (!u->build_queue.empty()) u->build_queue.erase(u->build_queue.begin());
		set_unit_order(u, get_order_type(Orders::IncompleteMorphing));
	}

	void set_creep_building_tiles(unit_type_autocast unit_type, xy pos) {
		if (!unit_type_spreads_creep(unit_type, true) && ut_requires_creep(unit_type)) return;

		set_unit_tiles_unoccupied(unit_type, pos);

		rect_t<xy_t<size_t>> creep_area;
		creep_area.from.x = pos.x / 32u - unit_type->placement_size.x / 32u / 2;
		creep_area.from.y = pos.y / 32u - unit_type->placement_size.y / 32u / 2;
		creep_area.to.x = creep_area.from.x + unit_type->placement_size.x / 32u;
		creep_area.to.y = creep_area.from.y + unit_type->placement_size.y / 32u;
		bool rounded_corners = false;
		if (ut_flag(unit_type, unit_type_t::flag_80000000)) {
			creep_area.from.x -= 2;
			creep_area.from.y -= 1;
			creep_area.to.x += 2;
			creep_area.to.y += 1;
			rounded_corners = true;
		}
		for (size_t y = creep_area.from.y; y != creep_area.to.y; ++y) {
			if (y >= game_st.map_tile_height) continue;
			for (size_t x = creep_area.from.x; x != creep_area.to.x; ++x) {
				if (x >= game_st.map_tile_width) continue;
				if (rounded_corners) {
					if ((x == creep_area.from.x || x == creep_area.to.x - 1) && (y == creep_area.from.y || y == creep_area.to.y - 1)) continue;
				}
				if (!tile_can_have_creep({x, y})) continue;
				if (st.tiles[y * game_st.map_tile_width + x].flags & tile_t::flag_occupied) continue;
				set_tile_creep({x, y});
			}
		}

		set_unit_tiles_occupied(unit_type, pos);
	}

	void refresh_unit_position(unit_t* u) {
		unit_t* turret = unit_turret(u);
		if (turret) {
			turret->exact_position = u->exact_position;
			turret->position = to_xy(turret->exact_position);
			move_sprite(turret->sprite, turret->position);
		}
		if (us_hidden(u)) {
			u->sprite->flags &= ~sprite_t::flag_hidden;
			if (turret) turret->sprite->flags &= ~sprite_t::flag_hidden;
			st.hidden_units.remove(*u);
			bw_insert_list(st.visible_units, *u);
		}
		refresh_unit_vision(u);
		update_unit_sprite(u);
		unit_finder_insert(u);
		if (u_grounded_building(u)) set_unit_tiles_occupied(u, u->sprite->position);
		check_unit_collision(u);
		if (u_flying(u)) increment_repulse_field(u);
		reset_movement_state(u);
		if (turret) reset_movement_state(turret);
	}

	bool units_share_unions(unit_type_autocast a, unit_type_autocast b) const {
		if (unit_is(a, UnitTypes::Protoss_Interceptor) != unit_is(b, UnitTypes::Protoss_Interceptor)) return false;
		if (unit_is(a, UnitTypes::Protoss_Scarab) != unit_is(b, UnitTypes::Protoss_Scarab)) return false;
		if (unit_is_carrier(a) != unit_is_carrier(b)) return false;
		if (unit_is_reaver(a) != unit_is_reaver(b)) return false;
		if (unit_is_ghost(a) != unit_is_ghost(b)) return false;
		if (ut_resource(a) != ut_resource(b)) return false;
		if (unit_is_nydus(a) != unit_is_nydus(b)) return false;
		if (unit_is(a, UnitTypes::Protoss_Pylon) != unit_is(b, UnitTypes::Protoss_Pylon)) return false;
		if (unit_is(a, UnitTypes::Terran_Nuclear_Silo) != unit_is(b, UnitTypes::Terran_Nuclear_Silo)) return false;
		if (ut_powerup(a) != ut_powerup(b)) return false;
		if (unit_is_hatchery(a) != unit_is_hatchery(b)) return false;
		return true;
	}

	void interceptors_attack(unit_t* u) {
		if (!unit_is_carrier(u)) return;
		for (unit_t* n : ptr(u->carrier.outside_units)) {
			n->sprite->elevation_level = u->sprite->elevation_level - 1;
			set_unit_order(n, n->unit_type->attack_unit, u->order_target.unit);
		}
	}

	bool carrier_reaver_attack(unit_t* u, int acquire_range, int max_ground_distance) {
		bool can_move = u->order_type->id != Orders::CarrierHoldPosition && u->order_type->id != Orders::ReaverHoldPosition;
		if (!can_move && u_movement_flag(u, 2)) stop_unit(u);
		unit_t* target = u->order_target.unit;
		auto can_attack_target = [&]() {
			if (unit_can_see_target(u, target) && unit_target_in_range(u, target, acquire_range + unit_target_movement_range(u, target))) {
				if (max_ground_distance == 0 || unit_long_path_distance(u, u->sprite->position, u->order_target.pos) < max_ground_distance) {
					return true;
				}
			}
			return false;
		};
		if (target) {
			if (u_invincible(target) || !unit_can_attack_target(u, target)) {
				if (u->auto_target_unit == target) u->auto_target_unit = nullptr;
				u->order_target.unit = nullptr;
			} else {
				u->order_target.pos = target->sprite->position;
				if (can_attack_target()) {
					stop_unit(u);
				} else {
					if (!can_move || u_ready_to_attack(u)) {
						if (u->auto_target_unit == target) u->auto_target_unit = nullptr;
						u->order_target.unit = nullptr;
						return false;
					}
					set_unit_move_target(u, u->order_target.pos);
					set_next_target_waypoint(u, u->order_target.pos);
				}
			}
		}
		if (unit_interceptor_count(u) == 0 && (!unit_is_reaver(u) || u->reaver.inside_count + u->reaver.outside_count == 0)) {
			stop_unit(u);
			order_done(u);
			return false;
		}
		if (u->main_order_timer) return false;
		target = u->order_target.unit;
		if (!target) {
			if (!u->order_queue.empty()) {
				activate_next_order(u);
				return false;
			}
		}
		if (!target || u_ready_to_attack(u)) {
			attack_unit_reacquire_target(u);
			if (unit_is_carrier(u) && u->order_target.unit != target) interceptors_attack(u);
			target = u->order_target.unit;
		}
		if (target) {
			if (can_move) {
				if (unit_is_reaver(u)) u->order_type = get_order_type(Orders::ReaverFight);
				else u->order_type = get_order_type(Orders::CarrierFight);
			}
			return can_attack_target();
		} else {
			unit_t* new_target = find_acquire_random_target(u);
			if (!new_target) {
				if (can_move && u->order_type->id != Orders::CarrierIgnore2) {
					if (unit_is_reaver(u)) u->order_type = get_order_type(Orders::Reaver);
					else u->order_type = get_order_type(Orders::Carrier);
				}
				return false;
			}
			if (can_move) {
				if (unit_is_reaver(u)) u->order_type = get_order_type(Orders::ReaverFight);
				else u->order_type = get_order_type(Orders::CarrierFight);
			}
			u->order_target.pos = new_target->sprite->position;
			u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
			if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
			if (max_ground_distance != 0 && unit_long_path_distance(u, u->sprite->position, u->order_target.pos) >= max_ground_distance) return false;
			u->order_target.unit = new_target;
			return true;
		}
	}

	unit_t* release_fighter(unit_t* u) {
		if (!unit_is_carrier(u) && !unit_is_reaver(u)) return nullptr;
		auto& inside_units = unit_is_reaver(u) ? u->reaver.inside_units : u->carrier.inside_units;
		unit_t* r = nullptr;
		fp8 best_hp;
		for (unit_t* n : ptr(inside_units)) {
			if (!u_completed(n)) continue;
			if (!r || n->hp > best_hp) {
				if (!r) best_hp = n->hp;
				r = n;
			}
		}
		if (!r) return nullptr;
		if (unit_is(r, UnitTypes::Protoss_Interceptor) && r->hp < r->unit_type->hitpoints / 2) return nullptr;
		xy pos = u->position;
		if (unit_is(r, UnitTypes::Protoss_Scarab)) {
			pos += to_xy(direction_xy(u->heading, 25));
			set_unit_heading(r, u->heading);
		}
		inside_units.remove(*r);
		if (unit_is_reaver(u)) {
			--u->reaver.inside_count;
			u->reaver.outside_units.push_front(*r);
			++u->reaver.outside_count;
		} else {
			--u->carrier.inside_count;
			u->carrier.outside_units.push_front(*r);
			++u->carrier.outside_count;
		}
		move_unit(r, pos);
		show_unit(r);
		r->fighter.is_outside = true;
		return r;
	}

	unit_t* create_hallucination(unit_t* source_unit, int owner) {
		unit_t* u = create_unit(source_unit->unit_type, source_unit->sprite->position, owner);
		if (!u) {
			display_last_error_for_player(owner);
			return nullptr;
		}
		u->hp = u->unit_type->hitpoints;
		make_unit_hallucination(u);
		set_unit_order(u, u->unit_type->human_ai_idle);
		if (unit_is_reaver(source_unit) && unit_is_reaver(u)) {
			for (unit_t* source_n : ptr(source_unit->reaver.inside_units)) {
				unit_t* n = create_hallucination(source_n, owner);
				if (n) {
					hide_unit(n);
					if (unit_is_fighter(n)) {
						n->fighter.parent = u;
						n->fighter.is_outside = false;
						u->reaver.inside_units.push_front(*n);
						++u->reaver.inside_count;
					}
				}
			}
			for (unit_t* source_n : ptr(source_unit->reaver.outside_units)) {
				unit_t* n = create_hallucination(source_n, owner);
				if (n) {
					hide_unit(n);
					if (unit_is_fighter(n)) {
						n->fighter.parent = u;
						n->fighter.is_outside = false;
						u->reaver.inside_units.push_front(*n);
						++u->reaver.inside_count;
					}
				}
			}
		}
		return u;
	}

	void order_Stop(unit_t* u) {
		stop_unit(u);
		set_next_target_waypoint(u, u->move_target.pos);
		iscript_run_to_idle(u);
		order_done(u);
	}

	void order_Guard(unit_t* u) {
		u->main_order_timer = lcg_rand(29, 0, 15);
		u->order_type = get_order_type(Orders::PlayerGuard);
	}

	void order_PlayerGuard(unit_t* u) {
		if (!unit_autoattack(u)) {
			if (u->main_order_timer == 0) {
				u->main_order_timer = 15;
				if (ut_turret(u)) {
					set_next_target_waypoint(u, u->subunit->next_target_waypoint);
				}
				if (unit_target_acquisition_range(u) != 0) {
					unit_t* target = find_acquire_target(u);
					if (target) auto_attack_target(u, target);
				}
			}
		}
	}

	void order_TurretGuard(unit_t* u) {
		set_next_target_waypoint(u, u->subunit->next_target_waypoint);
		order_PlayerGuard(u);
	}

	void order_Move(unit_t* u) {
		if (u->order_state == 0) {
			if (u->order_target.unit) {
				move_to_target_reset(u, u->order_target.unit);
			} else {
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
			}
			u->order_state = 1;
		} else {
			if (unit_is_at_move_target(u)) {
				order_done(u);
			}
		}
	}

	void order_Nothing(unit_t* u) {
		if (!u->order_queue.empty()) {
			activate_next_order(u);
		}
	}

	void order_MoveToMinerals(unit_t* u) {
		if (u->carrying_flags && (u->carrying_flags & 3) == 0) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			order_done(u);
			return;
		}
		set_unit_gathering(u);

		auto find_mineral_field = [&](xy pos, int range, bool require_not_being_gathered) {
			rect area{pos - xy(range, range), pos + xy(range, range)};
			int ground_height = get_ground_height_at(pos);
			return find_nearest_unit(pos, area, [&](const unit_t* target) {
				if (!unit_is_mineral_field(target)) return false;
				if (!any_neighbor_tile_unoccupied(target)) return false;
				if (!is_reachable(u->sprite->position, target->sprite->position)) return false;
				if (get_ground_height_at(target->sprite->position) != ground_height) return false;
				if (!unit_position_is_explored(u, target->sprite->position)) return false;
				if (unit_long_path_distance(target, target->sprite->position, u->sprite->position) > 2 * range) return false;
				if (require_not_being_gathered && target->building.resource.is_being_gathered) return false;
				return true;
			});
		};

		unit_t* target = u->order_target.unit;
		if (!target || !unit_is_mineral_field(target)) {
			xy search_pos = u->sprite->position;
			if (u->worker.target_resource_position != xy()) {
				search_pos = u->worker.target_resource_position;
				u->worker.target_resource_position = {};
			}
			target = find_mineral_field(search_pos, 32 * 12, false);
			if (!target) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				order_done(u);
				return;
			}
			u->order_target.pos = target->sprite->position;
			u->order_target.unit = target;
			u->order_state = 0;
		}
		if (!ut_resource(target)) error("MoveToMinerals: target is not a resource");
		if (u->order_state == 0) {
			move_to_target_reset(u, u->order_target.unit);
			u->worker.target_resource_position = u->order_target.pos;
			u->worker.target_resource_unit = u->order_target.unit;
			u->order_state = 1;
		} else {
			if (u->order_state == 1) {
				if (!unit_target_in_range(u, target, 10) && !unit_is_at_move_target(u)) return;
				if (target->building.resource.is_being_gathered) {
					unit_t* new_target = find_mineral_field(u->sprite->position, 32 * 8, true);
					if (new_target) {
						move_to_target_reset(u, new_target);
						u->worker.target_resource_position = new_target->sprite->position;
						u->worker.target_resource_unit = new_target;
						u->order_target.unit = new_target;
						u->order_target.pos = new_target->sprite->position;
						return;
					}
				}
				u->order_state = 3;
			}
			if (u->order_state == 3) {
				if (unit_is_at_move_target(u)) {
					if (u_immovable(u)) {
						target = find_mineral_field(u->sprite->position, 32 * 12, false);
						if (target) {
							move_to_target_reset(u, target);
							u->worker.target_resource_position = target->sprite->position;
							u->worker.target_resource_unit = target;
							u->order_target.unit = target;
							u->order_target.pos = target->sprite->position;
						} else order_done(u);
					} else {
						if (u->carrying_flags & 2) {
							set_unit_order(u, get_order_type(Orders::ReturnMinerals));
						} else {
							set_next_target_waypoint(u, u->order_target.pos);
							order_target_t order_target;
							order_target.position = target->sprite->position;
							order_target.unit = target;
							queue_order_front(u, get_order_type(Orders::WaitForMinerals), order_target);
							order_done(u);
						}
					}
				}
			}
		}
	}

	void order_WaitForMinerals(unit_t* u) {
		if (u->order_target.unit && unit_is_mineral_field(u->order_target.unit)) {
			if (u->order_state == 0) {
				wait_for_resource(u, u->order_target.unit);
			}
		} else {
			set_unit_order(u, get_order_type(Orders::MoveToMinerals));
		}
	}

	void order_MiningMinerals(unit_t* u) {
		if (!ut_worker(u)) error("order_MiningMinerals: unit is not a worker");
		unit_t* target = u->order_target.unit;
		if (target && unit_is_mineral_field(target)) {
			set_unit_gathering(u);
			if (u->order_state == 0) {
				if (is_facing_next_target_waypoint(u)) {
					u->order_target.pos = u->sprite->position + to_xy(direction_xy(u->heading, fp8::integer(20)));
					sprite_run_anim(u->sprite, iscript_anims::AlmostBuilt);
					u->main_order_timer = 75;
					u->order_state = 5;
				}
			} else if (u->order_state == 5) {
				if (u->main_order_timer == 0) {
					sprite_run_anim(u->sprite, iscript_anims::GndAttkToIdle);
					unit_gather_resources_from(u, target);
					gather_queue_next(u, target);
					remove_one_order(u, get_order_type(Orders::GatheringInterrupted));
					if (u->carrying_flags & 2) {
						set_unit_order(u, get_order_type(Orders::ReturnMinerals));
					} else {
						set_unit_order(u, get_order_type(Orders::MoveToMinerals));
					}
				}
			}
		} else {
			if (u->worker.is_gathering) {
				gather_queue_next(u, u->worker.gather_target);
			}
			remove_one_order(u, get_order_type(Orders::GatheringInterrupted));
			set_unit_order(u, get_order_type(Orders::MoveToMinerals));
		}
	}

	void order_ResetHarvestCollision(unit_t* u) {
		if (!u->order_queue.empty()) {
			order_t* next_order = &u->order_queue.front();
			auto nid = next_order->order_type->id;
			switch (nid) {
			case Orders::Harvest1:
			case Orders::MoveToGas:
			case Orders::WaitForGas:
			case Orders::HarvestGas:
			case Orders::MoveToMinerals:
			case Orders::WaitForMinerals:
			case Orders::MiningMinerals:
			case Orders::GatheringInterrupted:
			case Orders::GatherWaitInterrupted:
				while (u->order_queue.back().order_type->id == Orders::ResetHarvestCollision) {
					remove_queued_order(u, &u->order_queue.back());
				}
				queue_order_back(u, get_order_type(Orders::ResetHarvestCollision), {});
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
				return;
			default: break;
			}
		}
		u_unset_status_flag(u, unit_t::status_flag_gathering);
		u_unset_status_flag(u, unit_t::status_flag_no_collide);
		check_unit_collision(u);
		if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
		reset_movement_state(u);
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_ReturnGas(unit_t* u) {
		order_ReturnMinerals(u);
	}

	void order_ReturnMinerals(unit_t* u) {
		if ((u->carrying_flags & 3) == 0) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			order_done(u);
			return;
		}
		set_unit_gathering(u);
		unit_t* target = u->order_target.unit;
		if (!target || !unit_is_active_resource_depot(target)) {
			target = nullptr;
			u->order_state = 0;
		}
		if (u->order_state == 0) {
			if (!target) target = find_nearest_active_resource_depot(u);
			if (target) {
				u->order_target.unit = target;
				move_to_target_reset(u, target);
				u->order_state = 1;
			} else {
				u->order_process_timer = 75;
			}
		} else if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				if (u_immovable(u)) {
					u->order_target.unit = nullptr;
					u->order_state = 0;
				} else {
					const order_type_t* next_order_type = nullptr;
					if (u->carrying_flags & 1) next_order_type = get_order_type(Orders::MoveToGas);
					else next_order_type = get_order_type(Orders::MoveToMinerals);
					if (u->worker.target_resource_unit) {
						queue_order_front(u, next_order_type, u->worker.target_resource_unit);
					} else {
						queue_order_front(u, next_order_type, {});
					}
					order_done(u);
					destroy_carrying_images(u);
					if (u->carrying_flags & 2) {
						st.current_minerals[u->owner] += u->worker.resources_carried;
						st.total_minerals_gathered[u->owner] += u->worker.resources_carried;
					} else if (u->carrying_flags & 1) {
						st.current_gas[u->owner] += u->worker.resources_carried;
						st.total_gas_gathered[u->owner] += u->worker.resources_carried;
					}
					u->carrying_flags = 0;
					u->worker.resources_carried = 0;

					if (unit_is_hatchery(target)) {
						for (auto& v : target->building.hatchery.larva_spawn_side_values) {
							if (v) --v;
						}
						auto bb = unit_sprite_inner_bounding_box(target);
						int* val;
						if (u->sprite->position.y < bb.from.y) {
							val = &target->building.hatchery.larva_spawn_side_values[1];
						} else if (u->sprite->position.y > bb.to.y) {
							val = &target->building.hatchery.larva_spawn_side_values[3];
						} else if (u->sprite->position.x < bb.from.x) {
							val = &target->building.hatchery.larva_spawn_side_values[0];
						} else {
							val = &target->building.hatchery.larva_spawn_side_values[2];
						}
						if (*val < 100) *val += 25;
					}
				}
			}
		}
	}

	void order_Die(unit_t* u) {
		if (u->user_action_flags & 4) hide_unit(u);
		if (u->subunit) {
			image_t* main_image = u->subunit->sprite->main_image;
			if (main_image) {
				u->subunit->sprite->images.remove(*main_image);
				u->sprite->images.push_front(*main_image);
				main_image->sprite = u->sprite;
				i_unset_flag(main_image, image_t::flag_has_directional_frames);
			}
			destroy_unit(u->subunit);
			u->subunit = nullptr;
		}
		u->order_state = 1;
		if (!us_hidden(u) && (!u_hallucination(u) || u_lifetime_expired(u))) {
			sprite_run_anim(u->sprite, iscript_anims::Death);
			destroy_unit_impl(u);
		} else {
			if (u_hallucination(u)) {
				play_sound(619, u);
				thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Hallucination_Death1), u->sprite->position, 0);
				if (t) {
					t->sprite->elevation_level = u->sprite->elevation_level + 1;
					if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
				}
			}
			hide_unit(u);
			destroy_unit(u);
		}
	}

	void order_GatherWaitInterrupted(unit_t* u) {
		if (u->worker.gather_target) {
			u->worker.gather_target->building.resource.gather_queue.remove(*u);
			u->worker.gather_target = nullptr;
		}
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_GatheringInterrupted(unit_t* u) {
		if (u->worker.is_gathering) {
			gather_queue_next(u, u->worker.gather_target);
		}
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_PlaceBuilding(unit_t* u) {
		if (u->order_state == 0) {
			if (u->order_target.unit) {
				u->order_target.pos = u->order_target.unit->sprite->position;
			}
			set_unit_move_target(u, u->order_target.pos);
			set_next_target_waypoint(u, u->order_target.pos);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				if (u->build_queue.empty()) error("order_PlaceBuilding: empty build queue");
				const unit_type_t* unit_type = u->build_queue.front();
				auto can_build = [&]() {
					if (u_immovable(u)) return false;
					if (xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() > 128) return false;
					if (!player_has_supply_and_resources_for(u->owner, unit_type, true)) return false;
					if (!can_place_building(u, u->owner, unit_type, u->order_target.pos, true, false)) return false;
					return true;
				};
				if (can_build()) {
					st.current_minerals[u->owner] -= unit_type->mineral_cost;
					st.current_gas[u->owner] -= unit_type->gas_cost;
					u_unset_status_flag(u, unit_t::status_flag_ground_unit);
					u->sprite->elevation_level = u->unit_type->elevation_level + 1;
					if (u->order_queue.empty()) {
						queue_order_back(u, get_order_type(Orders::ResetCollision), {});
						queue_order_back(u, u->unit_type->return_to_idle, {});
					} else {
						queue_order_front(u, get_order_type(Orders::ResetCollision), {});
					}
					unit_t* build_unit;
					if (unit_type->id == UnitTypes::Terran_Refinery) build_unit = build_refinery(u, unit_type);
					else build_unit = create_unit(unit_type, u->order_target.pos, u->owner);
					u->build_queue.erase(u->build_queue.begin());
					if (build_unit) {
						build_unit->connected_unit = u;
						u->order_type = get_order_type(Orders::ConstructingBuilding);
						u->order_state = 3;
						u->order_target.unit = build_unit;
						set_unit_order(build_unit, get_order_type(Orders::IncompleteBuilding));
					} else {
						display_last_error_for_player(u->owner);
						set_unit_order(u, u->unit_type->return_to_idle, {});
					}
				} else {
					order_done(u);
				}
			}
		}
	}

	void order_IncompleteBuilding(unit_t* u) {
		if (u->order_state == 0) {
			if (u->hp > u->unit_type->hitpoints / 5) ++u->order_state;
		} else if (u->order_state == 1) {
			sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
			++u->order_state;
		} else if (u->order_state == 2) {
			if (u->hp > u->unit_type->hitpoints * 2 / 5) ++u->order_state;
		} else if (u->order_state == 3) {
			sprite_run_anim(u->sprite, iscript_anims::SpecialState2);
			++u->order_state;
		} else if (u->order_state == 4) {
			if (u->hp > u->unit_type->hitpoints * 3 / 5) ++u->order_state;
		} else if (u->order_state == 5) {
			set_construction_graphic(u, false);
			sprite_run_anim(u->sprite, iscript_anims::AlmostBuilt);
			++u->order_state;
		} else if (u->order_state == 6) {
			if (u->hp > u->unit_type->hitpoints * 4 / 5) set_unit_order(u, get_order_type(Orders::Nothing));
		}
	}

	void order_ConstructingBuilding(unit_t* u) {
		unit_t* target = u->order_target.unit;
		auto done = [&]() {
			sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
			if (target) {
				target->connected_unit = nullptr;
				// todo: callback for sound?
				if (unit_can_gather_gas(u, target)) {
					set_unit_order(u, get_order_type(Orders::Harvest1), target);
				} else {
					order_done(u);
				}
			} else order_done(u);
		};
		if (!target || !ut_building(target)) {
			done();
			return;
		}

		bool build = u->order_state >= 4;

		if (u->order_state == 0 || u->order_state == 2) {
			if (u->order_state == 0) {
				move_to_target_reset(u, target);
				u->order_state = 2;
			}
			if (unit_is_at_move_target(u)) {
				if (unit_is_at_move_target(u)) {
					if (u_immovable(u) || (target->connected_unit && target->connected_unit != u && target->connected_unit->order_target.unit == target)) {
						order_done(u);
					} else {
						target->connected_unit = u;
						u_unset_status_flag(u, unit_t::status_flag_ground_unit);
						u->sprite->elevation_level = u->unit_type->elevation_level + 1;
						set_queued_order(u, false, get_order_type(Orders::ResetCollision), {});
						set_queued_order(u, false, u->unit_type->return_to_idle, {});
						set_unit_move_target(u, target->sprite->position);
						set_next_target_waypoint(u, target->sprite->position);
						u->order_state = 3;
					}
				}
			}
		} else if (u->order_state == 3) {
			if (unit_is_at_move_target(u)) u->order_state = 4;
		} else if (u->order_state == 4) {
			rect sprite_rect;
			sprite_rect.from = target->sprite->position - xy(int(target->sprite->width / 2), int(target->sprite->height / 2));
			sprite_rect.to = sprite_rect.from + xy((int)target->sprite->width - 1, (int)target->sprite->height - 1);
			rect placement_rect;
			placement_rect.from = target->sprite->position - target->unit_type->placement_size / 2;
			placement_rect.to = placement_rect.from + target->unit_type->placement_size - xy(1, 1);
			rect inside_rect;
			inside_rect.from = placement_rect.from + u->unit_type->dimensions.from + xy(1, 1);
			inside_rect.to = placement_rect.to - u->unit_type->dimensions.to - xy(1, 1);
			if (inside_rect.from.x < sprite_rect.from.x) inside_rect.from.x = sprite_rect.from.x;
			if (inside_rect.from.y < sprite_rect.from.y) inside_rect.from.y = sprite_rect.from.y;
			if (inside_rect.to.x > sprite_rect.to.x) inside_rect.to.x = sprite_rect.to.x;
			if (inside_rect.to.y > sprite_rect.to.y) inside_rect.to.y = sprite_rect.to.y;
			xy pos;
			pos.x = lcg_rand(9, sprite_rect.from.x, sprite_rect.to.x);
			pos.y = lcg_rand(9, sprite_rect.from.y, sprite_rect.to.y);
			pos = restrict_move_target_to_valid_bounds(u, pos);
			if (check_unit_movement_terrain_collision(u, pos - u->sprite->position)) {
				pos.x = lcg_rand(9, inside_rect.from.x, inside_rect.to.x);
				pos.y = lcg_rand(9, inside_rect.from.y, inside_rect.to.y);
				pos = restrict_move_target_to_valid_bounds(u, pos);
			}
			set_unit_move_target(u, pos);
			set_next_target_waypoint(u, pos);
			u->order_target.pos = pos;
			u->order_state = 5;
		} else if (u->order_state == 5) {
			u->order_state = 6;
		} else if (u->order_state == 6) {
			if (xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() <= 20) {
				stop_unit(u);
				set_next_target_waypoint(u, target->sprite->position);
				u->order_target.pos = u->sprite->position + to_xy(direction_xy(xy_direction(u->next_target_waypoint - u->sprite->position), 20));
				u->order_state = 7;
			}
		} else if (u->order_state == 7) {
			if (is_at_next_target_waypoint_or_within_attack_angle(u)) {
				u->main_order_timer = 30 + (lcg_rand(10) & 63);
				sprite_run_anim(u->sprite, iscript_anims::AlmostBuilt);
				u->order_state = 8;
			}
		} else if (u->order_state == 8) {
			if (u->main_order_timer == 0) {
				sprite_run_anim(u->sprite, iscript_anims::GndAttkToIdle);
				u->order_state = 4;
			}
		}

		if (build) {
			resume_building_unit(target, false);
			if (u_completed(target)) {
				done();
			}
		}

	}

	void order_ResetCollision(unit_t* u) {
		u->sprite->elevation_level = u->unit_type->elevation_level;
		u_unset_status_flag(u, unit_t::status_flag_gathering);
		u_unset_status_flag(u, unit_t::status_flag_no_collide);
		u_set_status_flag(u, unit_t::status_flag_ground_unit);
		check_unit_collision(u);
		reset_movement_state(u);
		if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_Follow(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target) {
			order_done(u);
			return;
		}
		if (u->order_state == 0) {
			move_to_target_reset(u, target);
			if (unit_is_nydus(target)) {
				if (unit_can_enter_nydus(u, target)) {
					set_unit_order(u, get_order_type(Orders::EnterNydusCanal), target);
					return;
				}
			} else if (unit_is(target, UnitTypes::Protoss_Shield_Battery)) {
				if (unit_can_use_shield_battery(u, target)) {
					set_unit_order(u, get_order_type(Orders::RechargeShieldsUnit), target);
					return;
				}
			}
			u->order_state = 1;
		}
		if (u->main_order_timer == 0) {
			u->main_order_timer = 7;
			if (unit_target_is_enemy(u, target) && unit_can_attack_target(u, target)) {
				set_unit_order(u, get_order_type(Orders::Guard));
				return;
			}
			if (!u_flying(u)) {
				if (!is_reachable(u, target)) {
					stop_unit(u);
					set_next_target_waypoint(u, u->move_target.pos);
					return;
				}
			}
			if (unit_is_rescuable(target)) {
				if (!unit_target_in_range(u, target, 1)) {
					move_to_target_reset(u, target);
					return;
				}
			}
			if (!unit_target_in_range(u, target, 32 * 3)) {
				try_follow_unit(u, target);
				return;
			}
			if (!u_grounded_building(target) && u->order_queue.empty()) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				return;
			}
			order_done(u);
		}
	}

	void order_AttackMove(unit_t* u) {
		if (attack_move_movement(u)) {
			if (u->main_order_timer == 0) {
				u->main_order_timer = 15;
				if (!unit_is_at_move_target(u)) {
					attack_move_acquire_target(u);
				} else {
					order_done(u);
				}
			}
		}
	}

	void order_AttackUnit(unit_t* u) {
		attack_unit_reacquire_target(u);
		if (attack_unit_move_in_range(u)) {
			attack_unit_fire_weapon(u);
		}
	}

	void order_MoveToGas(unit_t* u) {
		if (u->carrying_flags && (u->carrying_flags & 3) == 0) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			order_done(u);
			return;
		}
		set_unit_gathering(u);
		unit_t* target = u->order_target.unit;
		if (u->order_state == 0) {
			if (!target) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				order_done(u);
			} else if (unit_is_mineral_field(target)) {
				queue_order_front(u, get_order_type(Orders::MoveToMinerals), {u->order_target.pos, target});
				order_done(u);
			} else {
				move_to_target_reset(u, target);
				u->worker.target_resource_position = u->order_target.pos;
				u->worker.target_resource_unit = target;
				u->order_state = 3;
			}
		} else if (u->order_state == 3) {
			if (unit_is_at_move_target(u)) {
				if (!target || !unit_can_gather_gas(u, target)) {
					stop_unit(u);
					set_next_target_waypoint(u, u->move_target.pos);
					order_done(u);
				} else {
					if (u_immovable(u)) u->order_state = 0;
					else if (u->carrying_flags & 1) set_unit_order(u, get_order_type(Orders::ReturnGas));
					else {
						queue_order_front(u, get_order_type(Orders::WaitForGas), {target->sprite->position, target});
						order_done(u);
					}
				}
			}
		}
	}

	void order_WaitForGas(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (target && unit_can_gather_gas(u, target) && (u->carrying_flags == 0 || u->carrying_flags & 3)) {
			if (u->order_state == 0) {
				wait_for_resource(u, target);
			}
		} else {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			order_done(u);
			return;
		}
	}

	void order_HarvestGas(unit_t* u) {
		if (!ut_worker(u)) error("order_HarvestGas: unit is not a worker");
		unit_t* target = u->order_target.unit;
		if (us_hidden(u) || (target && unit_can_gather_gas(u, target) && (u->carrying_flags == 0 || u->carrying_flags & 3))) {
			set_unit_gathering(u);
			if (u->order_state == 0) {
				hide_unit(u, false);
				u->order_state = 5;
				u->main_order_timer = 37;
			} else if (u->order_state == 5) {
				if (u->main_order_timer == 0) {
					xy pos;
					direction_t heading = u->heading;
					if (find_gas_exit_position(u, target, pos, heading)) {
						set_unit_heading(u, heading);
						move_unit(u, pos);
						show_unit(u);
						if (target) unit_gather_resources_from(u, target);
						if (u->worker.gather_target) gather_queue_next(u, u->worker.gather_target);
						remove_one_order(u, get_order_type(Orders::ResetHarvestCollision));
						if (u->order_queue.empty()) {
							if (u->carrying_flags & 1) {
								set_queued_order(u, false, get_order_type(Orders::ReturnGas), {});
							} else {
								set_queued_order(u, false, u->unit_type->return_to_idle, {});
							}
						}
						queue_order_front(u, get_order_type(Orders::ResetHarvestCollision), {});
						u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
						order_done(u);
					} else {
						u->main_order_timer = 18;
					}
				}
			}
		} else {
			stop_unit(u);
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
			if (u->worker.gather_target) gather_queue_next(u, u->worker.gather_target);
		}
	}

	void order_Repair(unit_t* u) {
		if (!ut_worker(u)) error("order_Repair: unit is not a worker");
		unit_t* target = u->order_target.unit;
		if (!target || target->hp >= target->unit_type->hitpoints || target->stasis_timer || u_loaded(target) || !ut_mechanical(target) || !u_completed(target)) {
			sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
			order_done(u);
			if (target && !u_loaded(target)) target->connected_unit = nullptr;
			return;
		}
		if (unit_race(target) != race_t::terran) {
			sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
			order_done(u);
			return;
		}

		int mineral_cost;
		int gas_cost;
		int time_cost;
		auto calc_repair_costs = [&]() {
			int target_mineral_cost = target->unit_type->mineral_cost;
			int target_gas_cost = target->unit_type->gas_cost;
			int lowest_cost = target_gas_cost == 0 ? target_mineral_cost : std::min(target_mineral_cost, target_gas_cost);
			if (lowest_cost) {
				auto target_max_hp = target->unit_type->hitpoints;
				auto hp_construction_rate = target->hp_construction_rate;
				if (st.cheat_operation_cwal) hp_construction_rate *= 16;
				time_cost = target_max_hp.raw_value * 3 / (lowest_cost * hp_construction_rate.raw_value);
				if (time_cost) {
					if (lowest_cost == target_mineral_cost) {
						mineral_cost = 1;
						gas_cost = target_gas_cost / target_mineral_cost;
					} else {
						mineral_cost = target_mineral_cost / target_gas_cost;
						gas_cost = 1;
					}
				} else {
					if (target_mineral_cost) {
						mineral_cost = target_mineral_cost * hp_construction_rate.raw_value / (target_max_hp.raw_value * 3);
						if (mineral_cost == 0) mineral_cost = 1;
					} else mineral_cost = 0;
					if (target_gas_cost) {
						gas_cost = target_gas_cost * hp_construction_rate.raw_value / (target_max_hp.raw_value * 3);
						if (gas_cost == 0) gas_cost = 1;
					} else gas_cost = 0;
					time_cost = 1;
				}
			} else {
				mineral_cost = 0;
				gas_cost = 0;
				time_cost = 0xffff;
			}
		};
		auto repair = [&]() {
			if (u->worker.repair_timer == 0) {
				calc_repair_costs();
				if (st.current_minerals[u->owner] >= mineral_cost && st.current_gas[u->owner] >= gas_cost) {
					u->worker.repair_timer = time_cost;
					st.current_minerals[u->owner] -= mineral_cost;
					st.current_gas[u->owner] -= gas_cost;
				} else {
					// todo: callback for error/sound
					sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
					order_done(u);
					return;
				}
			} else --u->worker.repair_timer;
			if (st.cheat_operation_cwal) set_unit_hp(target, target->hp + target->hp_construction_rate * 16);
			else set_unit_hp(target, target->hp + target->hp_construction_rate);
			if (target->hp >= target->unit_type->hitpoints) {
				sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
				order_done(u);
				if (target && !u_loaded(target)) target->connected_unit = nullptr;
			}
		};

		if (u->order_state == 0 || u->order_state == 1) {
			if (u->order_state == 0) {
				// todo: callback for error/sound if not enough resources
				u->worker.repair_timer = 0;
				u->order_state = 1;
			}
			if (u_completed(target)) {
				move_to_target_reset(u, target);
				u->order_target.pos = target->sprite->position;
				u->order_state = 6;
			} else {
				set_unit_order(u, get_order_type(Orders::ConstructingBuilding), target);
			}
		} else if (u->order_state == 6) {
			if (!unit_is_at_move_target(u)) {
				try_follow_unit(u, target);
			} else {
				if (unit_target_in_range(u, target, 5)) {
					set_next_target_waypoint(u, target->sprite->position);
					u->order_target.pos = target->sprite->position;
					u->order_state = 7;
					repair();
				} else {
					u->order_state = 1;
				}
			}
		} else if (u->order_state == 7) {
			if (unit_target_in_range(u, target, 5)) {
				set_next_target_waypoint(u, target->sprite->position);
				u->order_target.pos = target->sprite->position;
				if (is_at_next_target_waypoint_or_within_attack_angle(u)) {
					sprite_run_anim(u->sprite, iscript_anims::AlmostBuilt);
					u->order_state = 8;
				}
				repair();
			} else {
				u->order_state = 1;
			}
		} else if (u->order_state == 8) {
			set_next_target_waypoint(u, target->sprite->position);
			u->order_target.pos = target->sprite->position;
			if (!unit_target_in_range(u, target, 5) || target->order_type->id == Orders::LiftingOff) {
				sprite_run_anim(u->sprite, iscript_anims::GndAttkToIdle);
				u->order_state = 1;
			}
			repair();
		} else repair();
	}

	void order_EnterTransport(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target || u_hallucination(target) || !unit_provides_space(target) || !unit_can_load_target(target, u)) {
			order_done(u);
			return;
		}
		if (u->order_state == 0) {
			if (target->order_type->id != Orders::PickupTransport) {
				if (u_can_move(target)) {
					queue_order_front(target, get_order_type(Orders::PickupTransport), {u->sprite->position, u});
					activate_next_order(target);
				}
				u->order_state = 1;
			}
		}
		set_next_target_waypoint(u, target->sprite->position);
		try_follow_unit(u, target);
		if (unit_target_in_range(u, target, 1)) {
			unit_load_target(target, u);
		}
	}

	void order_Unload(unit_t* u) {
		if (u_grounded_building(u)) {
			for (unit_t* n : loaded_units(u)) {
				unit_unload(n);
			}
			order_done(u);
		} else {
			if (u->main_order_timer == 0) {
				if (loaded_units(u).empty()) {
					order_done(u);
				} else {
					auto lu = loaded_units(u);
					auto i = lu.begin();
					bool is_last = std::next(i) == lu.end();
					if (!unit_unload(*i) || is_last) {
						order_done(u);
					}
				}
			}
		}
	}

	void order_BuildingLiftoff(unit_t* u) {
		if (!u_grounded_building(u) || !ut_flying_building(u)) {
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
			return;
		}
		if (u->building.addon) building_abandon_addon(u);
		if (unit_is_factory(u)) u->building.rally.unit = u;
		set_unit_tiles_unoccupied(u, u->sprite->position);
		u_unset_status_flag(u, unit_t::status_flag_grounded_building);
		u_set_status_flag(u, unit_t::status_flag_can_turn);
		u_set_status_flag(u, unit_t::status_flag_can_move, !ut_turret(u));
		u_set_status_flag(u, unit_t::status_flag_flying);
		u->sprite->elevation_level = 12;
		reset_movement_state(u);
		check_unit_collision(u);
		unit_finder_remove(u);
		unit_finder_insert(u);
		if (u->sprite->images.back().modifier == 10) {
			u->sprite->images.back().frozen_y_value = get_image_map_position(&u->sprite->images.back()).y;
			u->sprite->images.back().flags |= image_t::flag_y_frozen;
		}
		set_next_target_waypoint(u, u->sprite->position - xy(0, 42));
		if (unit_is(u, UnitTypes::Zerg_Infested_Command_Center)) remove_creep_provider(u);
		u->order_type = get_order_type(Orders::LiftingOff);
	}

	void order_LiftingOff(unit_t* u) {
		if (u->order_state == 0 || u->order_state == 1) {
			if (u->order_state == 0) {
				if (u->position != u->next_target_waypoint && u->next_velocity_direction != xy_direction(u->next_target_waypoint - u->sprite->position)) {
					return;
				}
				u->order_state = 1;
			}
			xy pos = u->sprite->position + xy(0, u->sprite->images.back().offset.y - (ut_worker(u) ? 7 : 42));
			set_unit_move_target(u, pos);
			set_next_target_waypoint(u, pos);
			u->order_state = 2;
		} else if (u->order_state == 2 || u->order_state == 3) {
			if (u->order_state == 2) {
				sprite_run_anim(u->sprite, iscript_anims::LiftOff);
				if (u->sprite->main_image->image_type->lift_off_filename_index) {
					create_dust_sprite(u->sprite, SpriteTypes::SPRITEID_Building_Landing_Dust_Type1, 4, 0, 8, false);
					create_dust_sprite(u->sprite, SpriteTypes::SPRITEID_Building_Landing_Dust_Type1, 4, 16, 24, true);
				}
				u->flingy_top_speed = fp8::integer(1);
				set_next_speed(u, fp8::integer(1));
				u->order_state = 3;
			}
			if (unit_is_at_move_target(u)) {
				if (u->order_signal & 0x10) {
					u->order_signal &= ~0x10;
					if (!ut_building(u)) u->flingy_top_speed = get_modified_unit_speed(u, u->flingy_type->top_speed);
					if (u->sprite->images.back().modifier == 10) {
						u->sprite->images.back().offset.y = (ut_worker(u) ? 7 : 42);
						u->sprite->images.back().flags &= ~image_t::flag_y_frozen;
					}
					u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
					order_done(u);
				}
			}
		}
	}

	void order_PlaceAddon(unit_t* u) {
		const unit_type_t* addon_type = u->building.addon_build_type;
		if (!addon_type || !ut_addon(addon_type)) return;
		if (u->order_state == 0) {
			if (u_grounded_building(u)) {
				if (xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() == 0) {
					set_unit_move_target(u, u->order_target.pos);
					set_next_target_waypoint(u, u->order_target.pos);
					u->order_state = 1;
				} else {
					set_unit_order(u, get_order_type(Orders::BuildingLiftoff), u->order_target.pos);
					set_queued_order(u, false, get_order_type(Orders::PlaceAddon), u->order_target.pos);
				}
			} else {
				set_unit_order(u, get_order_type(Orders::BuildingLand), u->order_target.pos);
				set_queued_order(u, false, get_order_type(Orders::PlaceAddon), u->order_target.pos);
			}
		}
		if (u->order_state == 1 && unit_is_at_move_target(u)) {
			u->order_target.pos -= u->unit_type->placement_size / 2;
			u->order_target.pos += addon_type->addon_position;
			u->order_target.pos += addon_type->placement_size / 2;
			if (!player_has_supply_and_resources_for(u->owner, addon_type, true)) {
				order_done(u);
			} else if (!can_place_building(u, u->owner, addon_type, u->order_target.pos, true, false)){
				// todo: callback for error/sound
				order_done(u);
			} else {
				unit_t* addon = create_unit(addon_type, u->order_target.pos, u->owner);
				if (!addon) {
					// todo: callback for error/sound
				} else {
					st.current_minerals[u->owner] -= addon_type->mineral_cost;
					st.current_gas[u->owner] -= addon_type->gas_cost;
					set_unit_order(u, u->unit_type->return_to_idle);
					set_secondary_order(u, get_order_type(Orders::BuildAddon));
					u->current_build_unit = addon;
					set_unit_order(addon, get_order_type(Orders::IncompleteBuilding));
					addon->connected_unit = u;
				}
			}
		}
	}

	void order_BuildingLand(unit_t* u) {
		if (u_grounded_building(u)) {
			set_unit_order(u, get_order_type(Orders::BuildingLiftoff), u->order_target.pos);
			set_queued_order(u, false, get_order_type(Orders::BuildingLand), u->order_target.pos);
		} else {
			if (u->order_state == 0) {
				xy pos = u->order_target.pos - xy(0, u->sprite->images.back().offset.y);
				set_unit_move_target(u, pos);
				set_next_target_waypoint(u, pos);
				u->order_state = 1;
			} else if (u->order_state == 1) {
				if (unit_is_at_move_target(u)) {
					if (!can_place_building(u, u->owner, u->unit_type, u->order_target.pos, true, false)) {
						// todo: callback for error/sound
						if (!u->order_queue.empty() && u->order_queue.front().order_type->id == Orders::PlaceAddon) {
							remove_queued_order(u, &u->order_queue.front());
						}
						order_done(u);
					} else {
						set_unit_tiles_occupied(u, u->order_target.pos);
						u->building.is_landing = true;
						set_unit_move_target(u, u->order_target.pos);
						set_next_target_waypoint(u, u->order_target.pos);
						if (u->sprite->images.back().modifier == 10) {
							u->sprite->images.back().frozen_y_value = get_image_map_position(&u->sprite->images.back()).y;
							u->sprite->images.back().flags |= image_t::flag_y_frozen;
						}
						u_set_status_flag(u, unit_t::status_flag_order_not_interruptible);
						u->flingy_top_speed = fp8::integer(1);
						set_next_speed(u, fp8::integer(1));
						u->order_state = 2;
					}
				}
			} else if (u->order_state == 2) {
				if (u->position == u->next_target_waypoint || u->next_velocity_direction == xy_direction(u->next_target_waypoint - u->sprite->position)) {
					sprite_run_anim(u->sprite, iscript_anims::Landing);
					u->order_state = 3;
				}
			}
			if (u->order_state == 3) {
				if (u->order_signal & 0x10 && unit_is_at_move_target(u)) {
					u->order_signal &= ~0x10;
					unit_finder_remove(u);
					if (u_flying(u)) decrement_repulse_field(u);
					u_unset_status_flag(u, unit_t::status_flag_flying);
					u_unset_status_flag(u, unit_t::status_flag_can_turn);
					u_unset_status_flag(u, unit_t::status_flag_can_move);
					u_set_status_flag(u, unit_t::status_flag_grounded_building);
					u->sprite->elevation_level = 4;
					reset_movement_state(u);
					move_unit(u, u->order_target.pos);
					unit_finder_insert(u);
					check_unit_collision(u);
					u->sprite->images.back().offset.y = 0;
					if (u->sprite->images.back().modifier == 10) u->sprite->images.back().flags &= ~image_t::flag_y_frozen;
					if (u->sprite->main_image->image_type->landing_dust_filename_index) {
						create_dust_sprite(u->sprite, SpriteTypes::SPRITEID_Building_Landing_Dust_Type1, 3, 0, 8, false);
						create_dust_sprite(u->sprite, SpriteTypes::SPRITEID_Building_Landing_Dust_Type1, 3, 16, 24, true);
					}
					u->building.is_landing = false;
					for (unit_t* n : find_units_noexpand(unit_sprite_inner_bounding_box(u))) {
						if (n == u) continue;
						if (unit_target_is_enemy(n, u)) continue;
						if ((!unit_can_move(u) && unit_can_attack(u)) || u_burrowed(u)) kill_unit(n);
					}
					if (!u->order_queue.empty()) {
						auto* o = &u->order_queue.front();
						if (o->order_type->id == Orders::Move || o->order_type->id == Orders::Follow) {
							queue_order_front(u, get_order_type(Orders::BuildingLiftoff), {});
						} else {
							while (!u->order_queue.empty() && u->order_queue.back().order_type->id != Orders::PlaceAddon) {
								remove_queued_order(u, &u->order_queue.back());
							}
							if (u->order_queue.empty()) set_unit_order(u, u->unit_type->return_to_idle);
						}
					}
					for (unit_t* n : find_units_noexpand(unit_sprite_inner_bounding_box(u))) {
						if (n == u) continue;
						if (u_grounded_building(n) || !unit_can_move(n)) {
							if (unit_can_attack(n) && n->order_type->id != Orders::Die) {
								set_unit_order(u, get_order_type(Orders::BuildingLiftoff));
								set_queued_order(u, false, u->unit_type->return_to_idle, {});
								break;
							}
						}
					}
					u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
					order_done(u);
					find_and_connect_addon(u);
				}
			}
		}
	}

	void order_ResearchTech(unit_t* u) {
		auto* tech = u->building.researching_type;
		auto done = [&]() {
			u->building.researching_type = nullptr;
			if (tech) st.tech_researching[u->owner][tech->id] = false;
			sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
			order_done(u);
		};
		if (!tech) {
			done();
			return;
		}
		if (u->building.upgrade_research_time-- == 0 || player_has_researched(u->owner, tech->id) || st.cheat_operation_cwal) {
			// todo: callback for sound
			st.tech_researched[u->owner][tech->id] = true;
			done();
		}
	}

	void order_Upgrade(unit_t* u) {
		auto* upgrade = u->building.upgrading_type;
		auto done = [&]() {
			u->building.upgrading_type = nullptr;
			u->building.upgrading_level = 0;
			if (upgrade) st.upgrade_upgrading[u->owner][upgrade->id] = false;
			sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
			order_done(u);
		};
		if (!u_grounded_building(u) || !upgrade) {
			done();
			return;
		}
		bool already_upgraded = player_upgrade_level(u->owner, upgrade->id) >= u->building.upgrading_level;
		if (u->building.upgrade_research_time-- == 0 || already_upgraded || st.cheat_operation_cwal) {
			// todo: callback for sound
			if (!already_upgraded && player_max_upgrade_level(u->owner, upgrade->id) >= u->building.upgrading_level) {
				st.upgrade_levels[u->owner][upgrade->id] = u->building.upgrading_level;
				apply_upgrades_to_player_units(u->owner);
			}
			done();
		}
	}

	void order_StayInRange(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			order_done(u);
			return;
		}
		u->order_target.pos = target->sprite->position;
		if (!unit_can_attack_target(u, target)) {
			if (!u_ready_to_attack(u)) queue_order_front(u, get_order_type(Orders::Move), u->order_target.pos);
			order_done(u);
			return;
		}
		if (u->order_state == 0) {
			if (!unit_target_in_weapon_movement_range(u, target)) {
				move_to_target(u, target);
			}
			u->order_state = 1;
		} else {
			if (!unit_target_in_weapon_movement_range(u, target)) {
				if (!try_follow_unit(u, target)) return;
			} else {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
			}
		}
		if (unit_is_goliath(u) && !u_movement_flag(u, 2)) {
			auto heading_error = fp8::extend(xy_direction(u->order_target.pos - u->sprite->position) - u->heading).abs();
			if (heading_error >= 32_fp8) set_next_target_waypoint(u, u->order_target.pos);
		}
	}

	void order_TurretAttack(unit_t* u) {
		attack_unit_reacquire_target(u);
		unit_t* target = u->order_target.unit;
		if (target) u->order_target.pos = target->sprite->position;
		if (!unit_can_attack_target(u, target) || !unit_can_see_target(u, target)) {
			set_unit_order(u, get_order_type(Orders::TurretGuard));
			return;
		}
		bool attack = false;
		if (unit_can_see_target(u, target)) {
			if (unit_target_in_weapon_movement_range(u, target)) {
				attack = true;
			}
		}
		if (!attack && (u->subunit->order_type != u->subunit->unit_type->attack_unit || unit_is_immovable_attacker(u->subunit))) {
			set_unit_order(u, get_order_type(Orders::TurretGuard));
		} else {
			attack = true;
		}
		if (attack) {
			set_next_target_waypoint(u, u->order_target.pos);
			if (!u_movement_flag(u->subunit, 2)) attack_unit_fire_weapon(u);
		}
	}

	void order_Sieging(unit_t* u) {
		if (u->order_state == 0) {
			if (!unit_is_unsieged_tank(u)) {
				order_done(u);
				return;
			}
			if (u_movement_flag(u, 2)) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
			}
			u->order_state = 1;
		}
		unit_t* turret = unit_turret(u);
		if (!turret) error("order_Sieging: null turret");
		if (u->order_state == 1) {
			if (!u_movement_flag(u, 2) && !u_movement_flag(turret, 2) && !u_iscript_nobrk(turret)) {
				set_next_target_waypoint(u, to_xy(u->exact_position + direction_xy(8_dir * u->unit_type->unit_direction, 16)));
				set_next_target_waypoint(turret, to_xy(turret->exact_position + direction_xy(8_dir * turret->unit_type->unit_direction, 16)));
				set_unit_order(turret, get_order_type(Orders::NothingWait), u);
				auto ius = make_thingy_setter(iscript_unit, turret);
				sprite_run_anim(turret->sprite, iscript_anims::SpecialState1);
				u->order_state = 2;
				for (unit_t* n : find_units(unit_sprite_inner_bounding_box(u))) {
					if (n == u) continue;
					if (!u_grounded_building(n)) continue;
					if (u_no_collide(n)) continue;
					kill_unit(u);
					return;
				}
			}
		}
		if (u->order_state == 2) {
			if (is_facing_next_target_waypoint(u) && is_facing_next_target_waypoint(turret)) {
				if (unit_is(u, UnitTypes::Terran_Siege_Tank_Tank_Mode)) {
					morph_unit(u, get_unit_type(UnitTypes::Terran_Siege_Tank_Siege_Mode));
				} else if (unit_is(u, UnitTypes::Hero_Edmund_Duke_Tank_Mode)) {
					morph_unit(u, get_unit_type(UnitTypes::Hero_Edmund_Duke_Siege_Mode));
				}
				u->order_state = 3;
			}
		}
		if (u->order_state == 3) {
			if (u->order_signal & 1) {
				u->order_signal &= ~1;
				if (!u->order_queue.empty() && u->order_queue.front().order_type->id != Orders::WatchTarget) {
					set_unit_order(u, u->unit_type->return_to_idle);
				}
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
				u_unset_status_flag(turret, unit_t::status_flag_order_not_interruptible);
				order_done(turret);
			}
		}
	}

	void order_Unsieging(unit_t* u) {
		unit_t* turret = unit_turret(u);
		if (!turret) error("order_Unsieging: null turret");
		if (u->order_state == 0) {
			if (!unit_is_sieged_tank(u)) {
				order_done(u);
				return;
			}
			if (!u_iscript_nobrk(turret)) {
				set_unit_order(turret, get_order_type(Orders::NothingWait), u);
				set_next_target_waypoint(turret, to_xy(turret->exact_position + direction_xy(8_dir * turret->unit_type->unit_direction, 16)));
				auto ius = make_thingy_setter(iscript_unit, turret);
				sprite_run_anim(turret->sprite, iscript_anims::SpecialState1);
				u->order_state = 2;
			}
		}
		if (u->order_state == 2) {
			if (is_facing_next_target_waypoint(turret)) {
				sprite_run_anim(u->sprite, iscript_anims::SpecialState2);
				auto ius = make_thingy_setter(iscript_unit, turret);
				sprite_run_anim(turret->sprite, iscript_anims::SpecialState2);
				u->order_signal &= ~1;
				u->order_state = 3;
			}
		}
		if (u->order_state == 3) {
			if (u->order_signal & 1) {
				u->order_signal &= ~1;
				if (unit_is(u, UnitTypes::Terran_Siege_Tank_Siege_Mode)) {
					morph_unit(u, get_unit_type(UnitTypes::Terran_Siege_Tank_Tank_Mode));
				} else if (unit_is(u, UnitTypes::Hero_Edmund_Duke_Siege_Mode)) {
					morph_unit(u, get_unit_type(UnitTypes::Hero_Edmund_Duke_Tank_Mode));
				}
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
				u_unset_status_flag(turret, unit_t::status_flag_order_not_interruptible);
				order_done(turret);
			}
		}
	}

	void order_WatchTarget(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!unit_can_attack_target(u, target)) {
			order_done(u, u->order_target.pos);
			return;
		}
		if (!unit_can_see_target(u, target) || !unit_target_in_weapon_movement_range(u, target)) {
			if (unit_is_immovable_attacker(u)) {
				order_done(u, u->order_target.pos);
				return;
			}
		}
	}

	void order_PickupIdle(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (target) {
			queue_order_front(u, get_order_type(Orders::PickupTransport), {target->sprite->position, target});
			activate_next_order(u);
		}
	}

	void order_PickupTransport(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target) {
			order_done(u);
			return;
		}
		if (!unit_can_load_target(u, target)) {
			order_done(u);
			return;
		}
		if (u->main_order_timer == 0) {
			u->main_order_timer = 7;
			if (unit_target_in_range(u, target, 1)) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				if (target->order_type->id != Orders::EnterTransport && target->order_type->id != Orders::Pickup4) {
					set_unit_order(target, get_order_type(Orders::EnterTransport), u);
				}
				queue_order_front(u, get_order_type(Orders::PickupIdle), {target->sprite->position, target});
				activate_next_order(u);
			} else {
				move_to_target_reset(u, target);
			}
		}
	}

	void order_MoveUnload(unit_t* u) {
		if (u->order_state == 0) {
			if (u->order_target.unit) {
				u->order_target.pos = u->order_target.unit->sprite->position;
				u->order_target.unit = nullptr;
				u->order_state = 1;
			}
		}
		if (xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() <= 16) {
			queue_order_front(u, get_order_type(Orders::Unload), {});
			activate_next_order(u);
		} else {
			set_unit_move_target(u, u->order_target.pos);
			set_next_target_waypoint(u, u->order_target.pos);
		}
	}

	void order_PlaceMine(unit_t* u) {
		if (u->order_state == 0) {
			unit_t* target = u->order_target.unit;
			if (target) {
				move_to_target(u, target);
				set_next_target_waypoint(u, target->sprite->position);
				u->order_target.pos = target->sprite->position;
			} else {
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
			}
			u->order_state = 1;
		} else {
			if (!unit_can_use_tech(u, get_tech_type(TechTypes::Spider_Mines)) || (unit_is_at_move_target(u) && u_immovable(u))) {
				order_done(u);
				return;
			}
			if (xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() <= 20) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				xy pos = restrict_move_target_to_valid_bounds(get_unit_type(UnitTypes::Terran_Vulture_Spider_Mine), u->order_target.pos);
				unit_t* mine = create_unit(UnitTypes::Terran_Vulture_Spider_Mine, pos, u->owner);
				if (mine) {
					finish_building_unit(mine);
					complete_unit(mine);
					set_unit_heading(mine, u->heading);
					--u->vulture.spider_mine_count;
				} else {
					display_last_error_for_player(u->owner);
				}
				order_done(u);
			}
		}
	}

	void order_SpiderMine(unit_t* u) {
		if (u->order_state == 0) {
			u->ground_weapon_cooldown = 60;
			u->order_state = 1;
		}
		if (u->order_state == 1) {
			if (u->ground_weapon_cooldown == 0) {
				sprite_run_anim(u->sprite, iscript_anims::Burrow);
				u_set_status_flag(u, unit_t::status_flag_no_collide);
				u->order_state = 2;
			}
		}
		if (u->order_state == 2) {
			if (u->order_signal & 4) {
				u->order_signal &= ~4;
				if (!u_hallucination(u)) {
					set_unit_burrowed(u);
					set_unit_cloaked(u);
					set_sprite_cloak_modifier(u->sprite, true, true, true, 0, 0);
					u_set_status_flag(u, unit_t::status_flag_cloaked);
					u_set_status_flag(u, unit_t::status_flag_requires_detector);
					u->detected_flags = 0x80000000;
					u->secondary_order_timer = 0;
					set_secondary_order(u, get_order_type(Orders::Cloak));
				}
				u->order_state = 3;
			}
		}
		if (u->order_state == 3) {
			if (!u_cannot_attack(u)) {
				int range = 32 * unit_target_acquisition_range(u);
				unit_t* target = find_nearest_unit(u, square_at(u->sprite->position, range), [&](unit_t* target) {
					if (target == u) return false;
					if (!unit_target_is_enemy(u, target)) return false;
					if (u_invincible(target)) return false;
					if (u_grounded_building(target)) return false;
					if (u_flying(target)) return false;
					if (target->unit_type->unknown1 == 0xc1) return false; // hovering?
					return true;
				});
				if (target) {
					u->order_target.unit = target;
					sprite_run_anim(u->sprite, iscript_anims::UnBurrow);
					u->sprite->flags &= ~0x40;
					set_secondary_order(u, get_order_type(Orders::Nothing));
					u->order_state = 4;
				}
			}
		}
		unit_t* target = u->order_target.unit;
		if (u->order_state == 4) {
			if (u->order_signal & 4) {
				u->order_signal &= ~4;
				u->sprite->elevation_level = u->unit_type->elevation_level;
				u_unset_status_flag(u, unit_t::status_flag_no_collide);
				if (target) {
					crappy_move_to_target(u, target);
					u->order_state = 5;
				} else {
					u->order_state = 1;
				}
			}
		} else if (u->order_state == 5) {
			reset_movement_state(u);
			u_unset_status_flag(u, unit_t::status_flag_passively_cloaked);
			u_unset_status_flag(u, unit_t::status_flag_burrowed);
			u->sprite->sprite_type = u->unit_type->flingy->sprite;
			if (target && unit_target_in_range(u, target, 576)) {
				crappy_move_to_target(u, target);
				if (unit_target_in_range(u, target, 30)) {
					u->order_target.pos = u->sprite->position;
					u->order_target.unit = nullptr;
					sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
					u->order_state = 6;
				} else if (unit_is_at_move_target(u) && u_immovable(u)) {
					stop_unit(u);
					u->order_state = 1;
				}
			} else {
				stop_unit(u);
				u->order_state = 1;
			}
		}
		if (u->order_state == 6) {
			if (u->order_signal & 1) kill_unit(u);
		}
	}

	void order_Critter(unit_t* u) {
		if (u->order_state == 0) {
			direction_t dir = direction_from_index(lcg_rand(48, 0, 255));
			int distance = 32;
			if (unit_is(u, UnitTypes::Critter_Kakaru)) distance = lcg_rand(48, 160, 640);
			xy target_pos = u->sprite->position + to_xy(direction_xy(dir, distance));
			target_pos = restrict_move_target_to_valid_bounds(u, target_pos);
			u->order_target.pos = target_pos;
			set_unit_move_target(u, target_pos);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				if (unit_is(u, UnitTypes::Critter_Kakaru)) u->order_state = 0;
				else if (u_immovable(u)) {
					u->main_order_timer = 75;
					u->order_state = 2;
				} else {
					u->main_order_timer = lcg_rand(47, 0, 75);
					if (u->main_order_timer <= 15) u->main_order_timer = 0;
					u->order_state = 2;
				}
			}
		} else if (u->order_state == 2) {
			if (u->main_order_timer == 0) {
				u->order_state = 0;
			}
		}
	}

	void order_HoldPosition(unit_t* u) {
		if (u->order_state == 0) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
			if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
			u->order_state = 1;
		}
		if (try_attack_something(u)) {
			unit_t* turret = unit_turret(u);
			if (!turret || unit_is_goliath(u)) {
				set_next_target_waypoint(u, u->order_target.pos);
			}
		} else {
			if (u->main_order_timer == 0) {
				u->main_order_timer = 15;
				unit_t* target = find_acquire_random_target(u);
				u->order_target.unit = target;
				if (target) u->order_process_timer = 0;
			}
		}
	}

	void order_RightClickAction(unit_t* u) {
		if (u->order_state == 0) {
			set_unit_move_target(u, u->order_target.pos);
			set_next_target_waypoint(u, u->order_target.pos);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (unit_position_is_visible(u, u->order_target.pos)) {
				if (!u->order_unit_type) error("order_RightClickAction: null unit type");
				const unit_type_t* target_unit_type = u->order_unit_type;
				xy pos = u->order_target.pos;
				unit_t* target = find_unit_noexpand({pos - xy(96, 96), pos + xy(96, 96)}, [&](unit_t* n) {
					if (n->unit_type != target_unit_type) return false;
					if (n->sprite->position.x + n->unit_type->placement_size.x / 2 - (unsigned)pos.x >= (unsigned)n->unit_type->placement_size.x) return false;
					if (n->sprite->position.y + n->unit_type->placement_size.y / 2 - (unsigned)pos.y >= (unsigned)n->unit_type->placement_size.y) return false;
					return true;
				});
				const order_type_t* order = get_default_order(default_action(u), u, pos, target, nullptr);
				if (!order) {
					stop_unit(u);
					set_next_target_waypoint(u, u->move_target.pos);
					order_done(u);
				} else {
					if (order->id == Orders::AttackDefault) order = u->unit_type->attack_unit;
					if (unit_can_receive_order(u, order, u->owner)) {
						if (order->id == Orders::RallyPointUnit) {
							if (unit_is_factory(u)) {
								if (!target) target = u;
								u->building.rally.unit = target;
								u->building.rally.pos = target->sprite->position;
							}
						} else if (order->id == Orders::RallyPointTile) {
							if (unit_is_factory(u)) {
								u->building.rally.unit = nullptr;
								u->building.rally.pos = pos;
							}
						} else if (target) {
							queue_order_front(u, order, {target->sprite->position, target});
							order_done(u);
						} else {
							queue_order_front(u, order, pos);
							order_done(u);
						}
					}
				}
			}
		}
	}

	void order_Patrol(unit_t* u) {
		if (u->order_state == 0) {
			if (u->order_queue.empty() || u->order_queue.front().order_type->id != Orders::Patrol) {
				set_queued_order(u, true, get_order_type(Orders::Patrol), {u->order_target.pos, u->order_target.unit});
				activate_next_order(u);
				queue_order_back(u, get_order_type(Orders::Patrol), u->sprite->position);
				return;
			}
		}
		if (attack_move_movement(u, false)) {
			if (u->main_order_timer == 0) {
				u->main_order_timer = 15;
				if (unit_is(u, UnitTypes::Terran_Medic) && !u->order_target.unit) {
					unit_t* target = find_medic_target(u);
					if (target) {
						queue_order_front(u, get_order_type(Orders::Patrol), u->order_target.pos);
						queue_order_front(u, get_order_type(Orders::MedicHeal), {target->sprite->position, target});
						u->auto_target_unit = nullptr;
						order_done(u);
						u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
						if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
						return;
					}
				}
				if (!unit_is_at_move_target(u)) {
					attack_move_acquire_target(u, false);
				} else {
					if (u_immovable(u)) {
						queue_order_back(u, get_order_type(Orders::Patrol), u->order_target.pos);
					} else {
						queue_order_back(u, get_order_type(Orders::Patrol), u->sprite->position);
					}
					u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
					order_done(u);
				}
			}
		}
	}

	void order_CastScannerSweep(unit_t* u) {
		auto energy_cost = fp8::integer(get_tech_type(TechTypes::Scanner_Sweep)->energy_cost);
		if (u->energy < energy_cost) {
			// todo: callback for error
			order_done(u);
			return;
		}
		const unit_type_t* scan_type = get_unit_type(UnitTypes::Spell_Scanner_Sweep);
		xy pos = restrict_move_target_to_valid_bounds(scan_type, u->order_target.pos);
		unit_t* scan = create_unit(scan_type, pos, u->owner);
		if (scan) {
			finish_building_unit(scan);
			complete_unit(scan);
			order_done(u);
			u->energy -= energy_cost;
			play_sound(388, u);
		} else {
			display_last_error_for_player(u->owner);
			order_done(u);
		}
	}

	void order_Scanner(unit_t* u) {
		if (u->order_signal & 4) {
			u->order_signal &= ~4;
			kill_unit(u);
		}
	}

	void order_TowerGuard(unit_t* u) {
		if (unit_is_disabled(u)) return;
		if (u->main_order_timer == 0) {
			u->main_order_timer = 15;
			unit_t* target = find_acquire_random_target(u);
			u->order_target.unit = target;
			if (target) {
				set_unit_order(u, get_order_type(Orders::TowerAttack), target);
				u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
				if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
			}
		}
	}

	void order_TowerAttack(unit_t* u) {
		if (unit_is_disabled(u) || !try_attack_something(u)) {
			order_done(u);
		} else {
			set_unit_move_target(u, u->sprite->position);
			set_next_target_waypoint(u, u->order_target.pos);
		}
	}

	void order_CastDefensiveMatrix(unit_t* u) {
		auto* tech = get_tech_type(TechTypes::Defensive_Matrix);
		if (spell_cast_target_movement(u, tech, unit_sight_range(u, true))) {
			u->energy -= fp8::integer(tech->energy_cost);
			unit_t* target = u->order_target.unit;
			target->defensive_matrix_hp = fp8::integer(250);
			target->defensive_matrix_timer = 168;
			create_defensive_matrix_image(target);
			play_sound(349, u);
			create_image(get_image_type(ImageTypes::IMAGEID_Science_Vessel_Overlay_Part2), u->sprite, {}, image_order_above);
			order_done(u);
		}
	}

	void order_Spell(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (u->order_state == 2) {
			if (u->order_signal & 2) {
				u->order_signal &= ~2;
				order_done(u);
			} else {
				if (target) {
					u->order_target.pos = target->sprite->position;
					set_next_target_waypoint(u, target->sprite->position);
				}
			}
			return;
		}
		if (!spell_order_valid(u)) {
			// todo: callback for error
			stop_unit(u);
			order_done(u);
			return;
		}
		auto* tech = u->order_type->tech_type == TechTypes::None ? nullptr : get_tech_type(u->order_type->tech_type);
		if (tech && u->energy < fp8::integer(tech->energy_cost)) {
			// todo: callback for error
			order_done(u);
			return;
		}
		auto* weapon = get_weapon_type(u->order_type->weapon);
		if (target) u->order_target.pos = target->sprite->position;
		set_next_target_waypoint(u, u->order_target.pos);
		auto is_in_range = [&]() {
			if (!unit_can_see_order_target(u)) return false;
			if (target) {
				int range = weapon_max_range(u, weapon);
				range += unit_target_movement_range(u, target);
				return unit_target_in_range(u, target, range);
			} else {
				int range = weapon_max_range(u, weapon);
				range += unit_halt_distance(u).integer_part();
				return xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() <= range;
			 }
		};
		if (is_in_range()) {
			if (!u_movement_flag(u, 4)) stop_unit(u);
			if (u->spell_cooldown == 0 && unit_can_fire_weapon(u, weapon) && !u_movement_flag(u, 2) && unit_is_at_move_target(u)) {
				if (tech) u->energy -= fp8::integer(tech->energy_cost);
				u->spell_cooldown = get_modified_weapon_cooldown(u, weapon) + (lcg_rand(49) & 3) - 1;
				u_set_movement_flag(u, 8);
				u->order_signal &= ~2;
				sprite_run_anim(u->sprite, u->order_type->animation);
				u->order_state = 2;
			}
		} else {
			if (target) {
				if (u->order_state == 0) move_to_target(u, target);
				else try_follow_unit(u, target);
			} else {
				if (u->move_target_timer == 0) {
					set_unit_move_target(u, u->order_target.pos);
				}
			}
			set_next_target_waypoint(u, u->order_target.pos);
			if (unit_is_at_move_target(u) && u_immovable(u)) {
				order_done(u);
				return;
			}
		}
		if (u->order_state == 0) u->order_state = 1;
	}

	void order_MoveToTargetOrder(unit_t* u) {
		if (u->order_state == 0) {
			if (u_can_move(u)) {
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
			}
			u->order_state = 1;
		} else {
			if (player_position_is_visible(u->owner, u->order_target.pos)) {
				xy pos = u->order_target.pos;
				unit_t* target = find_unit_noexpand({pos - xy(96, 96), pos + xy(96, 96)}, [&](unit_t* n) {
					if (n->unit_type != u->order_unit_type) return false;
					if (n->sprite->position.x + n->unit_type->placement_size.x / 2 - (unsigned)pos.x >= (unsigned)n->unit_type->placement_size.x) return false;
					if (n->sprite->position.y + n->unit_type->placement_size.y / 2 - (unsigned)pos.y >= (unsigned)n->unit_type->placement_size.y) return false;
					return true;
				});
				if (target) {
					const order_type_t* order = get_order_type(u->order_type->target_order);
					if (order->id == Orders::AttackDefault) {
						order = u->unit_type->attack_unit;
						unit_t* turret = unit_turret(u);
						if (turret) {
							queue_order_front(turret, turret->unit_type->attack_unit, {target->sprite->position, target});
							order_done(turret);
						}
					}
					queue_order_front(u, order, {target->sprite->position, target});
				}
				order_done(u);
			}
		}
	}

	void order_PickupBunker(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (target && unit_can_load_target(u, target)) {
			set_unit_order(target, get_order_type(Orders::EnterTransport), u);
		}
		order_done(u);
	}

	void order_HealMove(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (target) {
			if (medic_can_heal_target(u, target)) {
				set_unit_order(u, get_order_type(Orders::MedicHeal), target);
				move_to_target(u, target);
				return;
			}
			u->order_target.pos = target->sprite->position;
		}
		if (u->move_target_timer == 0) {
			set_unit_move_target(u, u->order_target.pos);
		}
		if (unit_is_at_move_target(u)) {
			order_done(u);
			return;
		}
		if (u->main_order_timer == 0) {
			u->main_order_timer = 15;
			unit_t* heal_target = find_medic_target(u);
			if (heal_target) {
				auto prev_target_pos = u->order_target.pos;
				set_unit_order(u, get_order_type(Orders::MedicHeal), heal_target);
				move_to_target(u, heal_target);
				if (target) queue_order_back(u, get_order_type(Orders::HealMove), {target->sprite->position, target});
				else queue_order_back(u, get_order_type(Orders::HealMove), prev_target_pos);
			}
		}
	}

	void order_MedicHeal(unit_t* u) {
		int r = medic_try_heal(u);
		if (r == 0) {
			stop_unit(u);
			order_done(u);
			return;
		}
		unit_t* target = u->order_target.unit;
		if (u->order_state == 0) {
			if (target && try_follow_unit(u, target) && (!unit_is_at_move_target(u) || !u_immovable(u))) {
				if (unit_target_in_range(u, target, 30)) {
					stop_unit(u);
					u->order_state = 1;
				}
			} else {
				order_done(u);
			}
		} else {
			if (r == 1) {
				if (~u->order_signal & 1) {
					sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
					if (u->order_queue.empty() || u->order_queue.front().order_type->id != Orders::MedicHealToIdle) {
						queue_order_front(u, get_order_type(Orders::MedicHealToIdle), {});
					}
				}
				set_next_target_waypoint(u, target->sprite->position);
			} else {
				u->order_state = 0;
			}
		}
	}

	void order_MedicHealToIdle(unit_t* u) {
		if (~u->movement_flags & 2) {
			sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
		}
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_MedicIdle(unit_t* u) {
		unit_t* target = find_medic_target(u);
		if (target) {
			set_unit_order(u, get_order_type(Orders::MedicHeal), target);
			move_to_target(u, target);
		}
	}

	void order_MedicHoldPosition(unit_t* u) {
		if (u->order_state == 0) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
			if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
			u->order_state = 1;
			sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
		}
		int r = medic_try_heal(u);
		if (r == 1) {
			if (~u->order_signal & 1) {
				sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
				if (u->order_queue.empty() || u->order_queue.front().order_type->id != Orders::MedicHealToIdle) {
					queue_order_front(u, get_order_type(Orders::MedicHealToIdle), {});
				}
			}
			set_next_target_waypoint(u, u->order_target.unit->sprite->position);
		} else {
			if (r == 3) u->order_target.unit = nullptr;
			sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
			if (u->main_order_timer == 0) {
				u->main_order_timer = 15;
				unit_t* target = find_medic_target(u);
				u->order_target.unit = target;
				if (target) u->order_target.pos = target->sprite->position;
			}
		}
	}

	void order_NukeTrain(unit_t* u) {
		if (!unit_is(u, UnitTypes::Terran_Nuclear_Silo)) error("order_NukeTrain: unit is not a nuclear silo");
		if (u->order_state == 0) {
			if (u->current_build_unit) {
				u->building.silo.nuke = u->current_build_unit;
				u->current_build_unit->connected_unit = u;
				u->order_state = 1;
			}
		} else {
			if (u->building.silo.nuke) {
				if (u_completed(u->building.silo.nuke)) {
					u->building.silo.ready = true;
					u->order_state = 0;
				}
			} else u->order_state = 0;
		}
	}

	void order_NukePaint(unit_t* u) {
		if (u->order_target.unit) set_unit_order(u, get_order_type(Orders::NukeUnit), u);
		else set_unit_order(u, get_order_type(Orders::CastNuclearStrike), u->order_target.pos);
	}

	void order_NukeUnit(unit_t* u) {
		if (!u->order_target.unit) {
			stop_unit(u);
			order_done(u);
			return;
		}
		u->order_target.pos = u->order_target.unit->sprite->position;
		order_CastNuclearStrike(u);
	}

	void order_CastNuclearStrike(unit_t* u) {
		set_next_target_waypoint(u, u->order_target.pos);
		if (xy_length(u->exact_position - to_xy_fp8(u->order_target.pos)).integer_part() > unit_sight_range(u)) {
			if (u->move_target_timer == 0) set_unit_move_target(u, u->order_target.pos);
			return;
		}
		stop_unit(u);
		if (!is_facing_next_target_waypoint(u, 1_fp8)) return;
		unit_t* silo = nullptr;
		for (unit_t* n : ptr(st.player_units[u->owner])) {
			if (unit_is(n, UnitTypes::Terran_Nuclear_Silo) && n->building.silo.nuke && n->building.silo.ready) {
				silo = n;
				break;
			}
		}
		if (!silo) {
			order_done(u);
			return;
		}
		play_sound(239, u);
		unit_t* nuke = silo->building.silo.nuke;
		silo->building.silo.nuke = nullptr;
		silo->building.silo.ready = false;
		set_unit_order(nuke, get_order_type(Orders::NukeLaunch), u->order_target.pos);
		nuke->connected_unit = u;
		set_sprite_images_heading_by_index(nuke->sprite, 0);
		show_unit(nuke);
		u->connected_unit = nuke;
		set_unit_order(u, get_order_type(Orders::NukeTrack), u->sprite->position);
	}

	void order_NukeLaunch(unit_t* u) {
		if (!u->connected_unit && u->order_state < 5) {
			kill_unit(u);
			return;
		}
		if (u->order_state == 0) {
			play_sound(84, u);
			xy target_pos{u->sprite->position.x, u->unit_type->dimensions.from.y};
			set_unit_move_target(u, target_pos);
			set_next_target_waypoint(u, target_pos);
			u->main_order_timer = 90;
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (u->main_order_timer <= 45 || unit_is_at_move_target(u)) {
				play_sound(127, true);
				// todo: callback for message
				u->order_state = 2;
			}
		} else if (u->order_state == 2) {
			if (u->main_order_timer == 0 || unit_is_at_move_target(u)) {
				hide_unit(u);
				u->connected_unit->connected_unit = u;
				stop_unit(u);
				u->order_state = 3;
			}
		} else if (u->order_state == 3) {
			if (~u->movement_flags & 2) {
				sprite_run_anim(u->sprite, iscript_anims::WarpIn);
				u->order_state = 4;
			}
		} else if (u->order_state == 4) {
			if (u->order_signal & 2) {
				u->order_signal &= ~2;
				xy appear_at_pos;
				appear_at_pos.x = std::max(u->unit_type->dimensions.from.x, u->order_target.pos.x);
				appear_at_pos.y = std::max(u->unit_type->dimensions.from.y, u->order_target.pos.y - 320);
				move_unit(u, appear_at_pos);
				set_unit_heading(u, 128_dir);
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
				show_unit(u);
				u->order_state = 5;
			}
		} else if (u->order_state == 5) {
			if (xy_length(u->exact_position - to_xy_fp8(u->move_target.pos)).integer_part() <= 10) {
				u->order_target.unit = u;
				sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
				u->order_state = 6;
			}
		} else if (u->order_state == 6) {
			if (u->order_signal & 1) {
				u->order_signal &= ~1;
				u->user_action_flags |= 4;
				kill_unit(u);
			}
		}
	}

	void order_NukeTrack(unit_t* u) {
		if (!unit_is_ghost(u)) error("order_NukeTrack: unit is not a ghost");
		if (u->order_state == 0) {
			sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
			xy pos;
			if (u->connected_unit) pos = u->connected_unit->order_target.pos;
			thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Nuke_Target_Dot), pos, u->owner);
			u->ghost.nuke_dot = t;
			if (t) {
				t->sprite->elevation_level = u->sprite->elevation_level + 1;
				if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
			}
			u->order_state = 6;
		} else if (u->order_state == 6) {
			if (!u->connected_unit || u->connected_unit->order_state == 5) {
				if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
				sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
				if (u->ghost.nuke_dot) {
					sprite_run_anim(u->ghost.nuke_dot->sprite, iscript_anims::Death);
					u->ghost.nuke_dot = nullptr;
				}
			}
		}
	}

	void order_Larva(unit_t* u) {
		unit_t* hatchery = u->connected_unit;
		if (hatchery && !unit_target_in_range(u, hatchery, 10)) {
			move_to_target(u, hatchery);
			set_next_target_waypoint(u, hatchery->sprite->position);
		}
		if (unit_is_at_move_target(u) || (u->move_target.unit && u->move_target.pos != u->move_target.unit->sprite->position && !tile_has_creep(u->move_target.pos))) {
			if ((!hatchery || u->move_target.unit != hatchery) && !tile_has_creep(u->sprite->position)) {
				kill_unit(u);
				return;
			}
			int rv = lcg_rand(20);
			xy target_pos = u->sprite->position;
			if (rv & 8) target_pos.x += 10;
			else target_pos.x -= 10;
			if (rv & 0x80) target_pos.y += 10;
			else target_pos.y -= 10;
			if (is_in_map_bounds(target_pos) && tile_has_creep(target_pos)) {
				if (hatchery) {
					auto bb = unit_sprite_inner_bounding_box(hatchery);
					auto is_on_correct_side = [&](xy pos) {
						if (u->order_state == 0) return pos.x <= bb.from.x;
						else if (u->order_state == 1) return pos.y <= bb.from.y;
						else if (u->order_state == 2) return pos.x >= bb.to.x;
						else return pos.y >= bb.to.y;
					};
					if (!is_on_correct_side(target_pos)) {
						if (is_on_correct_side(u->sprite->position)) return;
						if (u->order_state == 0) target_pos = {bb.from.x - 10, hatchery->sprite->position.y};
						else if (u->order_state == 1) target_pos = {hatchery->sprite->position.x, bb.from.y - 10};
						else if (u->order_state == 2) target_pos = {bb.to.x + 10, hatchery->sprite->position.y};
						else target_pos = {hatchery->sprite->position.x, bb.to.y + 10};
					}
				}
				set_unit_move_target(u, target_pos);
				set_next_target_waypoint(u, target_pos);
			}
		}
	}

	void order_InitCreepGrowth(unit_t* u) {
		set_secondary_order(u, get_order_type(Orders::SpreadCreep));
		set_unit_order(u, u->unit_type->return_to_idle);
	}

	void order_ZergUnitMorph(unit_t* u) {
		if (u->build_queue.empty()) error("order_ZergUnitMorph: empty build queue");
		const unit_type_t* unit_type = u->build_queue.front();
		if (u->order_state == 0) {
			const unit_type_t* egg_type = nullptr;
			if (unit_is(u, UnitTypes::Zerg_Larva)) egg_type = get_unit_type(UnitTypes::Zerg_Egg);
			else if (unit_is(u, UnitTypes::Zerg_Hydralisk)) egg_type = get_unit_type(UnitTypes::Zerg_Lurker_Egg);
			else if (unit_is(u, UnitTypes::Zerg_Mutalisk)) egg_type = get_unit_type(UnitTypes::Zerg_Cocoon);
			if (!egg_type) error("order_ZergUnitMorph: no egg type");
			if (!has_available_supply_for(u->owner, unit_type, true)) {
				cancel_build_queue(u);
				set_unit_order(u, u->unit_type->return_to_idle);
				return;
			}
			add_completed_unit(u, -1, false);
			u_unset_status_flag(u, unit_t::status_flag_completed);
			morph_unit(u, egg_type);
			u->remaining_build_time = unit_type->build_time;
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (u->remaining_build_time) {
				if (st.cheat_operation_cwal) {
					if (u->remaining_build_time > 16) u->remaining_build_time -= 16;
					else u->remaining_build_time = 0;
				} else --u->remaining_build_time;
			} else {
				sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
				u->order_state = 2;
			}
		} else if (u->order_state == 2) {
			if (u->order_signal & 4) {
				u->order_signal &= ~4;
				morph_unit(u, unit_type);
				// todo: callback for sound
				u->build_queue.erase(u->build_queue.begin());
				if ((int)(ImageTypes)u->unit_type->construction_animation) {
					set_construction_graphic(u, true);
					sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
					set_unit_order(u, get_order_type(Orders::ZergBirth));
				} else {
					finish_building_unit(u);
					complete_unit(u);
					if (unit_is(u, UnitTypes::Zerg_Egg) && u->connected_unit) rally_unit(u, u->connected_unit);
				}
			}
		}
	}

	void order_ZergBirth(unit_t* u) {
		if (~u->order_signal & 4) return;
		u->order_signal &= ~4;
		unit_t* u2 = nullptr;
		if (ut_two_units_in_one_egg(u)) {
			xy a = u->sprite->position + get_image_lo_offset(u->sprite->main_image, 3, 0);
			xy b = u->sprite->position + get_image_lo_offset(u->sprite->main_image, 3, 1);
			a = restrict_move_target_to_valid_bounds(u, a);
			b = restrict_move_target_to_valid_bounds(u, b);
			move_unit(u, a);
			u2 = create_unit(u->unit_type, b, u->owner);
			if (u2) {
				if (unit_is(u2, UnitTypes::Zerg_Zergling)) set_unit_heading(u2, 144_dir);
				else if (unit_is(u2, UnitTypes::Zerg_Scourge)) set_unit_heading(u2, 16_dir);
				else set_unit_heading(u, 0_dir);
				finish_building_unit(u2);
				complete_unit(u2);
				copy_status_effects(u2, u);
			} else display_last_error_for_player(u->owner);
		}
		set_construction_graphic(u, false);
		finish_building_unit(u);
		complete_unit(u);
		xy pos = u->sprite->position;
		if (!ut_two_units_in_one_egg(u) && u->previous_unit_type != get_unit_type(UnitTypes::Zerg_Cocoon)) {
			if (u->unit_type->unknown1 == 0xc1) pos.y -= 7;
			else if (u_flying(u)) pos.y -= 42;
		}
		pos = restrict_move_target_to_valid_bounds(u, pos);
		if (pos != u->sprite->position) move_unit(u, pos);
		if (u->previous_unit_type != get_unit_type(UnitTypes::Zerg_Cocoon) && u->previous_unit_type != get_unit_type(UnitTypes::Zerg_Lurker_Egg)) {
			if (u->connected_unit) {
				rally_unit(u, u->connected_unit);
				if (u2) rally_unit(u2, u->connected_unit);
			}
		}
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_DroneStartBuild(unit_t* u) {
		set_unit_order(u, get_order_type(Orders::DroneLand), u->order_target.pos);
		queue_order_back(u, get_order_type(Orders::DroneBuild), u->order_target.pos);
	}

	void order_DroneLand(unit_t* u) {
		if (u->build_queue.empty()) error("order_DroneLand: empty build queue");
		const unit_type_t* build_type = u->build_queue.front();
		if (u->order_state == 0) {
			xy pos = u->order_target.pos - xy(0, 7);
			set_unit_move_target(u, pos);
			set_next_target_waypoint(u, pos);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				bool success = false;
				if (!u_immovable(u)) {
					success = can_place_building(u, u->owner, build_type, u->order_target.pos, true, true);
					if (success && unit_is(build_type, UnitTypes::Zerg_Extractor)) {
						const unit_t* gas = get_building_at_center_position(u->order_target.pos, get_unit_type(UnitTypes::Resource_Vespene_Geyser));
						if (gas && unit_target_in_range(u, gas, 32)) {
							u_unset_status_flag(u, unit_t::status_flag_ground_unit);
							u->sprite->elevation_level = u->unit_type->elevation_level + 1;
							set_queued_order(u, false, get_order_type(Orders::ResetCollision), {});
							set_queued_order(u, false, u->unit_type->return_to_idle, {});
							set_unit_move_target(u, u->order_target.pos);
							set_next_target_waypoint(u, u->order_target.pos);
							u_set_status_flag(u, unit_t::status_flag_order_not_interruptible);
							u->order_state = 2;
							return;
						}
					}
				}
				if (success) {
					if (xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() <= 128) {
						if (u->sprite->images.back().modifier == 10) {
							u->sprite->images.back().frozen_y_value = get_image_map_position(&u->sprite->images.back()).y;
							u->sprite->images.back().flags |= image_t::flag_y_frozen;
						}
						set_unit_move_target(u, u->order_target.pos);
						set_next_target_waypoint(u, u->order_target.pos);
						u_set_status_flag(u, unit_t::status_flag_order_not_interruptible);
						u->order_state = 2;
					} else success = false;
				}
				if (!success) {
					// todo: callback for sound
					remove_one_order(u, get_order_type(Orders::DroneBuild));
					set_unit_order(u, u->unit_type->return_to_idle);
				}
			}
		} else if (u->order_state == 2) {
			if (unit_is_at_move_target(u)) {
				if (u->sprite->images.back().modifier == 10) {
					u->sprite->images.back().offset.y = 0;
					u->sprite->images.back().flags &= ~image_t::flag_y_frozen;
				}
				if (!u_immovable(u)) {
					if (!u->order_queue.empty() && u->order_queue.front().order_type->id == Orders::DroneBuild) {
						move_unit(u, u->order_target.pos);
					} else {
						if (u->sprite->images.back().modifier == 10) u->sprite->images.back().offset.y = 7;
						queue_order_front(u, get_order_type(Orders::ResetCollision), {});
					}
				} else {
					// todo: callback for sound
					remove_one_order(u, get_order_type(Orders::DroneBuild));
					if (u->sprite->images.back().modifier == 10) u->sprite->images.back().offset.y = 7;
					queue_order_front(u, get_order_type(Orders::ResetCollision), {});
				}
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
			}
		}
	}

	void order_DroneBuild(unit_t* u) {
		if (u->build_queue.empty()) error("order_DroneBuild: empty build queue");
		const unit_type_t* build_type = u->build_queue.front();
		auto done = [&]() {
			if (u->sprite->images.back().modifier == 10) u->sprite->images.back().offset.y = 7;
			queue_order_front(u, get_order_type(Orders::ResetCollision), {});
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
		};
		if (u->sprite->position != u->order_target.pos) {
			done();
			return;
		}
		if (!can_place_building(u, u->owner, build_type, u->sprite->position, true, true)) {
			// todo: callback for sound
			done();
			return;
		}
		if (!has_available_supply_for(u->owner, build_type, true) || !has_available_resources_for(u->owner, build_type, true)) {
			done();
			return;
		}
		st.current_minerals[u->owner] -= build_type->mineral_cost;
		st.current_gas[u->owner] -= build_type->gas_cost;

		unit_t* powerup = ut_worker(u) ? u->worker.powerup : nullptr;
		if (powerup) drop_carried_items(u);
		if (unit_is(build_type, UnitTypes::Zerg_Extractor)) {
			unit_t* build_unit = build_refinery(u, build_type);
			if (build_unit) {
				u->user_action_flags |= 4;
				kill_unit(u);
				zerg_building_start_construction(build_unit);
				create_image(get_image_type(ImageTypes::IMAGEID_Vespene_Geyser2), build_unit->sprite, {}, image_order_below);
			} else {
				done();
			}
		} else {
			add_completed_unit(u, -1, false);
			u_unset_status_flag(u, unit_t::status_flag_completed);
			morph_unit(u, build_type);
			zerg_building_start_construction(u);
		}
		if (powerup && ut_powerup(powerup)) move_unit(powerup, powerup->building.powerup.origin);
	}

	void order_IncompleteMorphing(unit_t* u) {
		const unit_type_t* build_type = u->unit_type;
		if (!u->build_queue.empty()) {
			const unit_type_t* queued_type = u->build_queue.front();
			if (unit_type_is_morphing_building(queued_type)) build_type = queued_type;
		}
		if (u->order_state == 0) {
			if (u->remaining_build_time < build_type->build_time * 3 / 4) ++u->order_state;
		} else if (u->order_state == 1) {
			if (unit_is(u, UnitTypes::Zerg_Extractor)) {
				destroy_image_from_to(u, ImageTypes::IMAGEID_Vespene_Geyser2, ImageTypes::IMAGEID_Vespene_Geyser2);
			}
			sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
			++u->order_state;
		} else if (u->order_state == 2) {
			if (u->remaining_build_time < build_type->build_time / 2) ++u->order_state;
		} else if (u->order_state == 3) {
			sprite_run_anim(u->sprite, iscript_anims::SpecialState2);
			++u->order_state;
		} else if (u->order_state == 4) {
			if (u->remaining_build_time == 0) ++u->order_state;
		} else if (u->order_state == 5) {
			// todo: callback for sound
			sprite_run_anim(u->sprite, iscript_anims::AlmostBuilt);
			++u->order_state;
		} else if (u->order_state == 6) {
			if (u->order_signal & 4) {
				u->order_signal &= ~4;
				if (unit_type_is_morphing_building(build_type)) {
					u_set_status_flag(u, unit_t::status_flag_completed);
					add_completed_unit(u, -1, false);
					u_unset_status_flag(u, unit_t::status_flag_completed);
					auto hp = u->hp;
					morph_unit(u, build_type);
					hp += build_type->hitpoints;
					if (u->previous_unit_type) hp -= u->previous_unit_type->hitpoints;
					if (hp < fp8::integer(1)) hp = fp8::integer(1);
					set_unit_hp(u, hp);
					u->remaining_build_time = 0;
				}
				finish_building_unit(u);
				complete_unit(u);
				add_creep_provider(u);
				sprite_run_anim(u->sprite, iscript_anims::AlmostBuilt);

				set_creep_building_tiles(build_type, u->sprite->position);
			}
			return;
		}
		if (u->remaining_build_time) {
			if (st.cheat_operation_cwal) {
				if (u->remaining_build_time > 16) u->remaining_build_time -= 16;
				else u->remaining_build_time = 0;
			} else --u->remaining_build_time;
		}
		if (!unit_type_is_morphing_building(build_type)) {
			if (st.cheat_operation_cwal) set_unit_hp(u, u->hp + u->hp_construction_rate * 16);
			else set_unit_hp(u, u->hp + u->hp_construction_rate);
		}
	}

	void order_ZergBuildingMorph(unit_t* u) {
		if (u->build_queue.empty()) error("order_ZergBuildingMorph: empty build queue");
		const unit_type_t* build_type = u->build_queue.front();
		if (!has_available_supply_for(u->owner, build_type, true) || !has_available_resources_for(u->owner, build_type, true)) {
			set_queued_order(u, false, u->unit_type->return_to_idle, {});
			// shouldn't there be an order_done(u) here?
			return;
		}
		st.current_minerals[u->owner] -= build_type->mineral_cost;
		st.current_gas[u->owner] -= build_type->gas_cost;
		u_unset_status_flag(u, unit_t::status_flag_completed);
		set_construction_graphic(u, true);
		u->remaining_build_time = build_type->build_time;
		sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
		set_unit_order(u, get_order_type(Orders::IncompleteMorphing));
	}

	void order_Burrowing(unit_t* u) {
		if (u->order_state == 0) {
			stop_unit(u);
			u->order_state = 1;
		}
		if (u->order_state == 1) {
			if (u_movement_flag(u, 2)) {
				if (u->next_speed == 0_fp8) set_unit_immovable(u);
			} else {
				sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
				int dir = 12;
				if (unit_is_defiler(u)) dir = 11;
				if (unit_is(u, UnitTypes::Zerg_Drone)) dir = 13;
				set_next_target_waypoint(u, to_xy(u->exact_position + direction_xy(8_dir * dir, 16)));
				u->order_state = 2;
			}
		}
		if (u->order_state == 2) {
			if (is_facing_next_target_waypoint(u)) {
				sprite_run_anim(u->sprite, iscript_anims::Burrow);
				u->order_state = 3;
				play_sound(16, u);
			}
		}
		if (u->order_state == 3) {
			if (u->order_signal & 4) {
				u->order_signal &= ~4;
				for (image_t* image : ptr(u->sprite->images)) {
					if (image->image_type->always_visible) continue;
					hide_image(image);
				}
				image_t* carry_image = find_image(u->sprite, ImageTypes::IMAGEID_Flag, ImageTypes::IMAGEID_Terran_Gas_Tank_Type2);
				if (!carry_image) carry_image = find_image(u->sprite, ImageTypes::IMAGEID_Uraj, ImageTypes::IMAGEID_Khalis);
				if (carry_image) hide_image(carry_image);
				u_set_status_flag(u, unit_t::status_flag_no_collide);
				set_unit_burrowed(u);
				set_secondary_order(u, get_order_type(Orders::Cloak));
				u->detected_flags = 0x80000000;
				if (u->defensive_matrix_hp != 0_fp8) {
					auto hp = u->defensive_matrix_hp;
					auto time = u->defensive_matrix_timer;
					deal_defensive_matrix_damage(u, hp);
					u->defensive_matrix_hp = hp;
					u->defensive_matrix_timer = time;
				}
				if (u->irradiate_timer) destroy_image_from_to(u, ImageTypes::IMAGEID_Irradiate_Small, ImageTypes::IMAGEID_Irradiate_Large);
				if (u->ensnare_timer) destroy_image_from_to(u, ImageTypes::IMAGEID_Ensnare_Overlay_Small, ImageTypes::IMAGEID_Ensnare_Overlay_Large);
				if (u->plague_timer) destroy_image_from_to(u, ImageTypes::IMAGEID_Plague_Overlay_Small, ImageTypes::IMAGEID_Plague_Overlay_Large);
				set_unit_order(u, get_order_type(Orders::Burrowed));
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
			}
		}
	}

	void order_Burrowed(unit_t* u) {
		if (unit_is(u, UnitTypes::Zerg_Lurker)) {
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
		} else if (unit_is(u, UnitTypes::Zerg_Hydralisk) && !u->order_queue.empty() && u->order_queue.front().order_type->id == Orders::ZergUnitMorph) {
			set_unit_order(u, get_order_type(Orders::Unburrowing));
			queue_order(std::next(u->order_queue.begin()), u, get_order_type(Orders::ZergUnitMorph), {});
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
		}
	}

	void order_AttackFixedRange(unit_t* u) {
		attack_unit_reacquire_target(u);
		unit_t* target = u->order_target.unit;
		if (target) u->order_target.pos = target->sprite->position;
		if (!unit_can_attack_target(u, target) || !unit_target_in_weapon_movement_range(u, target)) {
			order_done(u);
			return;
		}
		set_next_target_waypoint(u, u->order_target.pos);
		attack_unit_fire_weapon(u);
	}

	void order_Unburrowing(unit_t* u) {
		if (u->order_state == 0) {
			u->sprite->elevation_level = u->unit_type->elevation_level;
			u_unset_status_flag(u, unit_t::status_flag_no_collide);
			check_unit_collision(u);
			u->sprite->flags &= sprite_t::flag_hidden;
			set_secondary_order(u, get_order_type(Orders::Nothing));
			sprite_run_anim(u->sprite, iscript_anims::UnBurrow);
			play_sound(17, u);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			u->detected_flags = 0x80000000;
			u->secondary_order_timer = 0;
			u->order_state = 2;
		}
		if (u->order_state == 2) {
			if (u->order_signal & 4) {
				u->order_signal &= ~4;
				reset_movement_state(u);
				u_unset_status_flag(u, unit_t::status_flag_passively_cloaked);
				u_unset_status_flag(u, unit_t::status_flag_burrowed);
				u->sprite->sprite_type = u->unit_type->flingy->sprite;
				apply_unit_effects(u);
				image_t* carry_image = find_image(u->sprite, ImageTypes::IMAGEID_Flag, ImageTypes::IMAGEID_Terran_Gas_Tank_Type2);
				if (!carry_image) carry_image = find_image(u->sprite, ImageTypes::IMAGEID_Uraj, ImageTypes::IMAGEID_Khalis);
				if (carry_image) show_image(carry_image);
				for (image_t* image : ptr(u->sprite->images)) {
					set_image_heading(image, u->heading);
				}
				u->order_state = 3;
			}
		} else if (u->order_state == 3) {
			if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
		}
	}

	void order_BuildNydusExit(unit_t* u) {
		if (!can_place_building(u, u->owner, get_unit_type(UnitTypes::Zerg_Nydus_Canal), u->order_target.pos, true, false)) {
			// todo: callback for error/sound
			order_done(u);
			return;
		}
		unit_t* exit = create_unit(UnitTypes::Zerg_Nydus_Canal, u->order_target.pos, u->owner);
		if (exit) {
			set_construction_graphic(exit, true);
			exit->hp = exit->unit_type->hitpoints / 10;
			set_unit_order(exit, get_order_type(Orders::IncompleteMorphing));
			add_creep_provider(exit);
			if (unit_is_nydus(u)) u->building.nydus.exit = exit;
			if (unit_is_nydus(exit)) exit->building.nydus.exit = u;
			order_done(u);
		} else {
			display_last_error_for_player(u->owner);
			order_done(u);
		}
	}

	void order_EnterNydusCanal(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target || !unit_can_enter_nydus(u, target)) {
			order_done(u);
			return;
		}
		unit_t* exit = unit_is_nydus(target) ? target->building.nydus.exit : nullptr;
		if (!exit) exit = target;
		if (u->order_state == 0) {
			move_to_target_reset(u, target);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				if (u_immovable(u)) {
					order_done(u);
				} else u->order_state = 2;
			}
		}
		if (u->order_state == 2) {
			hide_unit(u, false);
			auto prev_pos = u->sprite->position;
			move_unit(u, exit->sprite->position);
			bool res;
			xy pos;
			std::tie(res, pos) = find_unit_placement(u, exit->sprite->position, false);
			if (res) {
				play_sound(19, target);
				move_unit(u, pos);
				refresh_unit_position(u);
				order_done(u);
				play_sound(19, exit);
			} else {
				move_unit(u, prev_pos);
				refresh_unit_position(u);
			}
		}
	}

	void order_CastInfestation(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (u->order_state == 0) {
			if (!target) {
				order_done(u);
				return;
			}
			if (!unit_can_be_infested(target)) {
				// todo: erorr message
				order_done(u);
				return;
			}
			move_to_target_reset(u, target);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				if (target && unit_can_be_infested(target) && !u_immovable(u)) {
					set_unit_order(u, get_order_type(Orders::InfestingCommandCenter), target);
				} else order_done(u);
			} else if (target) try_follow_unit(u, target);
		}
	}

	void order_InfestingCommandCenter(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (u->order_state == 0) {
			if (target && unit_can_be_infested(target) && target->order_type->id != Orders::InfestedCommandCenter) {
				hide_unit(u);
				set_unit_order(target, get_order_type(Orders::InfestedCommandCenter), u);
				u->remaining_build_time = 60;
				u->order_state = 3;
			} else {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
			}
		} else if (u->order_state == 3) {
			if (u->remaining_build_time) {
				--u->remaining_build_time;
				return;
			}
			bool res;
			xy pos;
			std::tie(res, pos) = find_unit_placement(u, u->sprite->position, false);
			if (res) {
				move_unit(u, pos);
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				show_unit(u);
				if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
			}
		}
	}

	void order_InfestedCommandCenter(unit_t* u) {
		unit_t* queen = u->order_target.unit;
		if (!queen || !unit_can_be_infested(u)) {
			u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
			order_done(u);
			return;
		}
		if (u->order_state == 0) {
			u->remaining_build_time = 3;
			u->order_state = 2;
			return;
		}
		if (u->remaining_build_time) {
			--u->remaining_build_time;
			return;
		}
		u->remaining_build_time = 0xffff;
		if (unit_addon(u)) building_abandon_addon(u);
		if (unit_is_factory(u)) u->building.rally.unit = u;
		const unit_type_t* new_type = u->unit_type->infestation_unit;
		if (!units_share_unions(u->unit_type, new_type)) error("order_InfestedCommandCenter: can't change unit type due to unions mismatch");
		// todo: update scores
		increment_unit_counts(u, -1);
		if (u_completed(u)) add_completed_unit(u, -1, false);
		u->unit_type = new_type;
		increment_unit_counts(u, 1);
		if (u_completed(u)) add_completed_unit(u, 1, false);
		set_unit_owner(u, queen->owner, true);
		set_sprite_owner(u, queen->owner);
		u->secondary_order_type = nullptr;
		set_secondary_order(u, get_order_type(Orders::Nothing));
		cancel_build_queue(u);
		while (!u->order_queue.empty()) remove_queued_order(u, &u->order_queue.front());
		if (u_grounded_building(u)) sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
		if (u->unit_type->construction_animation) create_image(u->unit_type->construction_animation, u->sprite, {}, image_order_above);
		set_unit_hp(u, u->unit_type->hitpoints);
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_SuicideUnit(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target) {
			order_done(u);
			return;
		}
		if (!unit_can_attack_target(u, target)) {
			set_unit_order(u, get_order_type(Orders::Move), target->sprite->position);
			return;
		}
		if (u->order_state == 0) {
			move_to_target(u, target);
			u->order_state = 1;
		}
		if (u->order_state == 1) {
			if ((unit_is_at_move_target(u) && u_immovable(u)) || unit_target_in_range(u, target, 4)) {
				stop_unit(u);
				if (!unit_target_in_range(u, target, get_weapon_type(WeaponTypes::Suicide_Infested_Terran)->outer_splash_radius)) {
					order_done(u);
					return;
				}
				u->order_target.unit = u;
				sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
				u_set_status_flag(u, unit_t::status_flag_order_not_interruptible);
				u->order_state = 2;
			} else {
				if (unit_target_in_range(u, target, 256) && u_movement_flag(target, 2)) crappy_move_to_target(u, target);
				else try_follow_unit(u, target);
			}
		}
		if (u->order_state == 2) {
			if (u->order_signal & 1) {
				u_set_status_flag(u, unit_t::status_flag_lifetime_expired);
				kill_unit(u);
			}
		}
	}

	void order_SuicideLocation(unit_t* u) {
		if (u->order_state == 0) {
			set_unit_move_target(u, u->order_target.pos);
			set_next_target_waypoint(u, u->order_target.pos);
			u->order_state = 1;
		}
		if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				if (u_immovable(u) && xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() > get_weapon_type(WeaponTypes::Suicide_Infested_Terran)->outer_splash_radius) {
					order_done(u);
					return;
				}
				u->order_target.unit = u;
				sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
				u_set_status_flag(u, unit_t::status_flag_order_not_interruptible);
				u->order_state = 2;
			}
		}
		if (u->order_state == 2) {
			if (u->order_signal & 1) {
				u_set_status_flag(u, unit_t::status_flag_lifetime_expired);
				kill_unit(u);
			}
		}
	}

	void order_SuicideHoldPosition(unit_t* u) {
		if (u->order_state == 0) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			u->order_state = 1;
		}
		if (!u->order_queue.empty()) activate_next_order(u);
	}

	void order_QueenHoldPosition(unit_t* u) {
		order_SuicideHoldPosition(u);
	}

	void order_PlaceProtossBuilding(unit_t* u) {
		if (u->order_state == 0) {
			set_unit_move_target(u, u->order_target.pos);
			set_next_target_waypoint(u, u->order_target.pos);
			u->order_state = 1;
		} else if (u->order_state == 1 || u->order_state == 2) {
			if (u->build_queue.empty()) error("order_PlaceProtossBuilding: empty build queue");
			const unit_type_t* build_type = u->build_queue.front();
			if (u->order_state == 2 && u_movement_flag(u, 2)) return;
			int d = 70;
			if (unit_is(build_type, UnitTypes::Protoss_Assimilator)) d += 18;
			if (xy_length(to_xy_fp8(u->order_target.pos) - u->exact_position).integer_part() <= d) {
				if (u->order_state == 1) {
					stop_unit(u);
					set_next_target_waypoint(u, u->move_target.pos);
					u->order_state = 2;
				} else {
					auto can_build = [&]() {
						if (!player_has_supply_and_resources_for(u->owner, build_type, true)) return false;
						if (!can_place_building(u, u->owner, build_type, u->order_target.pos, true, false)) return false;
						return true;
					};
					if (can_build()) {
						st.current_minerals[u->owner] -= build_type->mineral_cost;
						st.current_gas[u->owner] -= build_type->gas_cost;
						unit_t* build_unit;
						if (build_type->id == UnitTypes::Protoss_Assimilator) build_unit = build_refinery(u, build_type);
						else build_unit = create_unit(build_type, u->order_target.pos, u->owner);
						u->build_queue.erase(u->build_queue.begin());
						if (build_unit) {
							replace_sprite_images(build_unit->sprite, get_image_type(ImageTypes::IMAGEID_Warp_Anchor), 0_dir);
							if (unit_is(build_type, UnitTypes::Protoss_Assimilator)) create_image(get_image_type(ImageTypes::IMAGEID_Vespene_Geyser), build_unit->sprite, xy(), image_order_below);
							set_unit_order(build_unit, get_order_type(Orders::IncompleteWarping));
							u->order_target.unit = build_unit;
							u->order_state = 3;
							play_sound(528, u);
						} else {
							display_last_error_for_player(u->owner);
							order_done(u);
						}
					} else {
						order_done(u);
					}
				}
			} else {
				if (unit_is_at_move_target(u)) {
					set_unit_move_target(u, u->order_target.pos);
					set_next_target_waypoint(u, u->order_target.pos);
				}
			}
		} else if (u->order_state == 3) {
			if (u->order_target.unit && unit_is(u->order_target.unit, UnitTypes::Protoss_Assimilator) && u->order_queue.empty()) {
				set_unit_order(u, get_order_type(Orders::WaitForGas), u->order_target.unit);
				sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
			} else {
				order_done(u);
			}
		}
	}

	void order_IncompleteWarping(unit_t* u) {
		if (u->order_state == 0) {
			if (u->remaining_build_time == 0) {
				sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
				play_sound(529, u);
				u->order_state = 1;
			}
		} else if (u->order_state == 1) {
			if (u->order_signal & 1) {
				u->order_signal &= ~1;
				replace_sprite_images(u->sprite, u->sprite->sprite_type->image, 0_dir);

				iscript_set_script(u->sprite->main_image, 193);
				iscript_run_anim(u->sprite->main_image, iscript_anims::Init);
				iscript_set_script(u->sprite->main_image, u->sprite->main_image->image_type->iscript_id);
				set_image_modifier(u->sprite->main_image, 12);
				iscript_execute(u->sprite->main_image, u->sprite->main_image->iscript_state);

				u->order_state = 2;
			}
			return;
		} else if (u->order_state == 2) {
			if (u->order_signal & 1) {
				u->order_signal &= ~1;
				replace_sprite_images(u->sprite, u->sprite->sprite_type->image, 0_dir);
				sprite_run_anim(u->sprite, iscript_anims::WarpIn);
				u->order_state = 3;
			}
			return;
		} else if (u->order_state == 3) {
			if (~u->order_signal & 1) return;
			u->order_signal &= ~1;
			finish_building_unit(u);
			complete_unit(u);
			check_unit_collision(u);
			if (u_disabled(u)) {
				sprite_run_anim(u->sprite, iscript_anims::Disable);
			}
		}
		if (st.cheat_operation_cwal) {
			if (u->remaining_build_time > 16) u->remaining_build_time -= 16;
			else u->remaining_build_time = 0;
			set_unit_hp(u, u->hp + u->hp_construction_rate * 16);
			set_unit_shield_points(u, u->shield_points + u->shield_construction_rate * 16);
		} else {
			if (u->remaining_build_time) --u->remaining_build_time;
			set_unit_hp(u, u->hp + u->hp_construction_rate);
			set_unit_shield_points(u, u->shield_points + u->shield_construction_rate);
		}
	}

	void order_InitializePsiProvider(unit_t* u) {
		if (!unit_is(u, UnitTypes::Protoss_Pylon)) error("order_InitializePsiProvider: unit is not a pylon");
		if (!u->building.pylon.psionic_matrix_link.first) {
			st.psionic_matrix_units.push_front(*u);
			sprite_t* s = create_sprite(get_sprite_type(SpriteTypes::SPRITEID_Psi_Field_Right_Upper), u->sprite->position, 0);
			u->building.pylon.psi_field_sprite = s;
			if (s) {
				s->flags |= sprite_t::flag_hidden;
				set_sprite_visibility(s, 0);
			}
			st.update_psionic_matrix = true;
		}
		set_unit_order(u, u->unit_type->return_to_idle);
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_InitializeArbiter(unit_t* u) {
		if (u->order_queue.empty()) {
			set_unit_order(u, get_unit_type(UnitTypes::Protoss_Arbiter)->return_to_idle);
		}
		if (!u_hallucination(u)) {
			set_secondary_order(u, get_order_type(Orders::CloakNearbyUnits));
		}
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_ArchonWarp(unit_t* u) {
		bool is_dark_archon = u->order_type->id == Orders::DarkArchonMeld;
		unit_t* target = u->order_target.unit;
		if (!target || target->owner != u->owner || target->order_type != u->order_type || target->order_target.unit != u) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			order_done(u);
			return;
		}
		if (u_ground_unit(u)) {
			if (unit_target_in_range(u, target, (u->next_speed * 2).integer_part())) {
				u_unset_status_flag(u, unit_t::status_flag_ground_unit);
				queue_order_front(u, get_order_type(Orders::ResetCollision), {});
			}
		}
		int d = xy_length(u->sprite->position - target->sprite->position);
		if (d <= (is_dark_archon ? 20 : 2)) {
			merge_status_effects(u, target);
			morph_unit(u, get_unit_type(is_dark_archon ? UnitTypes::Protoss_Dark_Archon : UnitTypes::Protoss_Archon));
			if (is_dark_archon) decloak_unit(u);
			u->sprite->flags &= ~sprite_t::flag_iscript_nobrk;
			sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
			u->kill_count += target->kill_count;
			if (is_dark_archon) u->energy = fp8::integer(50);
			target->user_action_flags |= 4;
			kill_unit(target);
			u_unset_status_flag(u, unit_t::status_flag_can_move);
			reset_movement_state(u);
			set_unit_order(u, get_order_type(Orders::CompletingArchonSummon));
		} else {
			if (u->order_state && unit_is_at_move_target(u) && u_immovable(u)) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				order_done(u);
			} else {
				u->order_state = 1;
				if (u->move_target_timer == 0) {
					if (d <= (is_dark_archon ? 19 : 10)) {
						set_unit_move_target(u, (u->sprite->position + target->sprite->position) / 2);
					} else {
						set_unit_move_target(u, target->sprite->position);
					}
				}
			}
		}
	}

	void order_CompletingArchonSummon(unit_t* u) {
		if (u->order_state == 0) {
			if (u->remaining_build_time) {
				if (st.cheat_operation_cwal) {
					if (u->remaining_build_time > 16) u->remaining_build_time -= 16;
					else u->remaining_build_time = 0;
				} else --u->remaining_build_time;
			} else {
				u->sprite->flags &= ~sprite_t::flag_iscript_nobrk;
				sprite_run_anim(u->sprite, iscript_anims::Init);
				u->order_state = 1;
			}
		} else if (u->order_state == 1) {
			if (u->order_signal & 4) {
				u->order_signal &= ~4;
				u_set_status_flag(u, unit_t::status_flag_can_move);
				reset_movement_state(u);
				// todo: callback for sound
				set_queued_order(u, false, u->unit_type->human_ai_idle, {});
				u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
				order_done(u);
			}
		}
	}

	void order_CastRecall(unit_t* u) {
		if (u->order_state == 0) {
			if (u->energy < fp8::integer(get_tech_type(TechTypes::Recall)->energy_cost)) {
				// todo: callback for error message/sound?
				order_done(u);
				return;
			}
			u->energy -= fp8::integer(get_tech_type(TechTypes::Recall)->energy_cost);
			if (u->order_target.unit) u->order_target.pos = u->order_target.unit->sprite->position;
			thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Recall_Field), u->order_target.pos, 0);
			if (t) {
				t->sprite->elevation_level = u->sprite->elevation_level + 1;
				if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
			}
			play_sound(550 + lcg_rand(17) % 2, u->order_target.pos);
			u->main_order_timer = 22;
			u->order_state = 1;
		} else if (u->order_state == 1 && u->main_order_timer == 0) {
			int n_recalled = 0;
			a_vector<unit_t*> targets;
			for (unit_t* target : find_units_noexpand(square_at(u->order_target.pos, 64))) {
				if (target == u) continue;
				if (target->owner != u->owner) continue;
				if (us_hidden(target)) continue;
				if (ut_building(target)) continue;
				if (u_invincible(target)) continue;
				if (u_hallucination(target)) continue;
				if (u_burrowed(target)) continue;
				if (unit_is(target, UnitTypes::Zerg_Larva)) continue;
				if (unit_is(target, UnitTypes::Zerg_Egg)) continue;
				if (unit_is(target, UnitTypes::Zerg_Lurker_Egg)) continue;

				targets.push_back(target);
			}
			for (unit_t* target : targets) {

				xy target_pos = target->sprite->position;
				move_unit(target, u->sprite->position);
				auto r = find_unit_placement(target, u->sprite->position, false);
				if (!r.first) {
					move_unit(target, target_pos);
					continue;
				}

				for (unit_t* n : ptr(st.visible_units)) {
					remove_target_references(n, target);
				}
				for (bullet_t* n : ptr(st.active_bullets)) {
					remove_target_references(n, target);
				}
				unit_finder_remove(target);
				if (u_grounded_building(target)) set_unit_tiles_unoccupied(target, target->sprite->position);
				if (u_flying(target)) decrement_repulse_field(target);
				reset_movement_state(target);

				move_unit(target, r.second);
				refresh_unit_position(target);
				if (!unit_is(target, UnitTypes::Zerg_Cocoon)) set_unit_order(target, target->unit_type->return_to_idle);
				thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Recall_Field), r.second, 0);
				if (t) {
					t->sprite->elevation_level = target->sprite->elevation_level + 1;
					if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
				}
				if (unit_is_ghost(target) && target->connected_unit && unit_is(target->connected_unit, UnitTypes::Terran_Nuclear_Missile)) {
					target->connected_unit->connected_unit = nullptr;
					target->connected_unit = nullptr;
				}
				++n_recalled;
			}
			if (n_recalled) {
				play_sound(552 + lcg_rand(18) % 2, u);
			}
			order_done(u);
		}
	}

	void order_Carrier(unit_t* u) {
		if (carrier_reaver_attack(u, 32 * unit_target_acquisition_range(u), 0))  {
			unit_t* fighter = release_fighter(u);
			if (fighter) {
				fighter->shield_points = fp8::integer(fighter->unit_type->shield_points);
				fighter->sprite->elevation_level = u->sprite->elevation_level - 1;
				set_unit_order(fighter, fighter->unit_type->attack_unit, u->order_target.unit);
				u->main_order_timer = 7;
			}
		}
	}

	void order_CarrierStop(unit_t* u) {
		stop_unit(u);
		set_next_target_waypoint(u, u->move_target.pos);
		return_interceptors(u);
		set_unit_order(u, get_order_type(Orders::Carrier), u->order_target.unit);
	}

	void order_CarrierAttack(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target) {
			order_done(u);
			return;
		}
		if (!unit_can_attack_target(u, target)) {
			set_unit_order(u, get_order_type(Orders::Move), u->order_target.pos);
			return;
		}
		bool ready_to_attack = u_ready_to_attack(u);
		bool flag_8 = u_status_flag(u, unit_t::status_flag_8);
		interceptors_attack(u);
		queue_order_front(u, get_order_type(Orders::CarrierFight), target);
		activate_next_order(u);
		u_set_status_flag(u, unit_t::status_flag_ready_to_attack, ready_to_attack);
		u_set_status_flag(u, unit_t::status_flag_8, flag_8);
	}

	void order_InterceptorAttack(unit_t* u) {
		if (!unit_is_fighter(u)) error("order_InterceptorAttack: unit is not a fighter");
		unit_t* parent = u->fighter.parent;
		if (parent && u->shield_points < fp8::integer(u->unit_type->shield_points) / 4) {
			set_unit_order(u, get_order_type(Orders::InterceptorReturn));
			return;
		}
		unit_t* target = u->order_target.unit;
		if (!target || u_invincible(target)) {
			if (!unit_autoattack(u)) {
				set_unit_order(u, get_order_type(Orders::InterceptorReturn));
				return;
			}
			target = u->order_target.unit;
		}
		u->order_target.pos = target->sprite->position;
		if (parent) {
			int range = 32 * (unit_target_acquisition_range(parent) + 2);
			if (!unit_target_in_range(parent, target, range + unit_target_movement_range(parent, target))) {
				set_unit_order(u, get_order_type(Orders::InterceptorReturn));
				return;
			}
		}

		auto set_interceptor_move_target = [&](int mult) {
			int x = lcg_rand(11, -127, 128) * mult;
			int y = lcg_rand(11, -127, 128) * mult;
			xy pos = u->order_target.pos + xy(x, y);
			auto dims = u->unit_type->dimensions;
			if (pos.x < dims.from.x || pos.x >= (int)game_st.map_width - dims.to.x) pos.x = u->order_target.pos.x - x;
			if (pos.y < dims.from.y || pos.y >= (int)game_st.map_height - dims.to.y) pos.y = u->order_target.pos.y - y;
			pos = restrict_move_target_to_valid_bounds(u, pos);
			set_unit_move_target(u, pos);
			set_next_target_waypoint(u, pos);
		};

		if (u->order_state == 0) {
			if (!parent) {
				kill_unit(u);
				return;
			}
			play_sound(616, u);
			set_interceptor_move_target(3);
			u->main_order_timer = 15;
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (u->main_order_timer == 0) {
				u->sprite->elevation_level += 2;
				u->order_state = 3;
			}
		}
		if (u->order_state == 3) {
			if (unit_target_in_weapon_movement_range(u, target)) attack_unit_fire_weapon(u);
			if (unit_target_in_range(u, target, 50 + unit_target_movement_range(u, target))) {
				set_interceptor_move_target(1);
				u->order_state = 4;
			} else {
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
			}
		} else if (u->order_state == 4) {
			if (xy_length(to_xy_fp8(u->move_target.pos) - u->exact_position).integer_part() <= 50) {
				set_interceptor_move_target(2);
				u->order_state = 5;
			}
		} else if (u->order_state == 5) {
			if (xy_length(to_xy_fp8(u->move_target.pos) - u->exact_position).integer_part() <= 50) {
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
				u->order_state = 3;
			}
		}
	}

	void order_InterceptorReturn(unit_t* u) {
		if (!unit_is_fighter(u)) error("order_InterceptorReturn: unit is not a fighter");
		unit_t* parent = u->fighter.parent;
		if (!parent) {
			kill_unit(u);
			return;
		}
		if (u->order_state == 0) {
			if (xy_length(to_xy_fp8(parent->sprite->position) - u->exact_position).integer_part() <= 60) {
				u->order_state = 1;
				if (unit_is(u, UnitTypes::Protoss_Interceptor)) u->sprite->elevation_level = parent->sprite->elevation_level - 2;
			}
		}
		if (xy_length(to_xy_fp8(parent->sprite->position) - u->exact_position).integer_part() <= 10) {
			if (unit_is_carrier(parent)) {
				parent->carrier.outside_units.remove(*u);
				--parent->carrier.outside_count;
				parent->carrier.inside_units.push_front(*u);
				++parent->carrier.inside_count;
			} else if (unit_is_reaver(parent)) {
				parent->reaver.outside_units.remove(*u);
				--parent->reaver.outside_count;
				parent->reaver.inside_units.push_front(*u);
				++parent->reaver.inside_count;
			}
			u->fighter.is_outside = false;
			hide_unit(u);
			set_unit_order(u, get_order_type(Orders::Nothing));
		} else {
			set_unit_move_target(u, parent->sprite->position);
			set_next_target_waypoint(u, parent->sprite->position);
		}
	}

	void order_DarkArchonMeld(unit_t* u) {
		order_ArchonWarp(u);
	}

	void order_CastMindControl(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target || target->owner == u->owner) {
			order_done(u);
			return;
		}
		auto* tech = get_tech_type(TechTypes::Mind_Control);
		if (spell_cast_target_movement(u, tech, unit_sight_range(u, true))) {
			if (u_hallucination(target)) kill_unit(target);
			else {
				create_sized_image(target, ImageTypes::IMAGEID_Mind_Control_Hit_Small);
				trigger_give_unit_to(target, u->owner);
				if (unit_is(target, UnitTypes::Protoss_Dark_Archon)) target->energy = 0_fp8;
				order_done(target);
			}
			u->energy -= fp8::integer(tech->energy_cost);
			u->shield_points = 0_fp8;
			order_done(u);
			play_sound(1062, target);
		}
	}

	void order_Reaver(unit_t* u) {
		if (!unit_is_reaver(u)) error("order_Reaver: unit is not a reaver");
		int range = 32 * unit_target_acquisition_range(u);
		if (carrier_reaver_attack(u, range, range * 2))  {
			set_next_target_waypoint(u, u->order_target.pos);
			if (direction_index(u->heading) - direction_index(xy_direction(u->next_target_waypoint - u->sprite->position)) <= 1) {
				if (!u_cannot_attack(u) && u->reaver.outside_count == 0) {
					unit_t* fighter = release_fighter(u);
					if (fighter) {
						set_unit_order(fighter, fighter->unit_type->attack_unit, u->order_target.unit);
						u->main_order_timer = 60;
					}
				}
			} else {
				u->order_process_timer = 0;
			}
		}
	}

	void order_ReaverAttack(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target) {
			order_done(u);
			return;
		}
		if (!unit_can_attack_target(u, target)) {
			set_unit_order(u, get_order_type(Orders::Move), u->order_target.pos);
			return;
		}
		bool ready_to_attack = u_ready_to_attack(u);
		bool flag_8 = u_status_flag(u, unit_t::status_flag_8);
		queue_order_front(u, get_order_type(Orders::ReaverFight), target);
		activate_next_order(u);
		u_set_status_flag(u, unit_t::status_flag_ready_to_attack, ready_to_attack);
		u_set_status_flag(u, unit_t::status_flag_8, flag_8);
	}

	void order_ScarabAttack(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (u->order_state == 0) {
			set_remove_timer(u, 90);
			u_set_status_flag(u, unit_t::status_flag_no_collide);
			u->main_order_timer = 7;
			if (target) move_to_target(u, target);
			else {
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
			}
			u->order_state = 2;
		} else if (u->order_state == 2 || u->order_state == 3) {
			if (u->order_state == 2) {
				if (u->main_order_timer == 0) {
					u_unset_status_flag(u, unit_t::status_flag_no_collide);
					u->order_state = 3;
				}
			}
			bool is_at_target;
			if (target) {
				if (unit_target_in_range(u, target, 256) && u_movement_flag(target, 2)) crappy_move_to_target(u, target);
				else try_follow_unit(u, target);
				const weapon_type_t* w = u->unit_type->ground_weapon;
				if (!w) error("order_ScarabAttack: no ground weapon");
				is_at_target = unit_target_in_range(u, target, w->inner_splash_radius / 2);
			} else {
				is_at_target = unit_is_at_move_target(u);
			}
			if (is_at_target) {
				u->order_target.pos = u->sprite->position;
				sprite_run_anim(u->sprite, iscript_anims::SpecialState1);
				u->order_state = 6;
			}
		} else if (u->order_state == 6) {
			if (u->order_signal & 1) {
				u_set_status_flag(u, unit_t::status_flag_lifetime_expired);
				kill_unit(u);
			}
		}
	}

	void order_ReaverStop(unit_t* u) {
		if (!unit_is_reaver(u)) error("order_ReaverStop: unit is not a reaver");
		stop_unit(u);
		set_next_target_waypoint(u, u->move_target.pos);
		for (unit_t* n : ptr(u->reaver.outside_units)) {
			set_unit_order(n, get_order_type(Orders::SelfDestructing));
		}
		set_unit_order(u, get_order_type(Orders::Reaver), u->order_target.unit);
	}

	void order_RechargeShieldsUnit(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target || target->energy == 0_fp8 || unit_is_disabled(target)) {
			order_done(u);
			return;
		}
		if (u->order_state == 0) {
			move_to_target_reset(u, target);
			u->order_state = 1;
		} else if (u->order_state == 1) {
			if (unit_is_at_move_target(u)) {
				if (u_immovable(u)) {
					order_done(u);
					return;
				}
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				u->order_state = 2;
			} else if (unit_target_in_range(u, target, 128)) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				u->order_state = 2;
			}
		}
		if (u->order_state == 2) {
			set_secondary_order(target, get_order_type(Orders::ShieldBattery));
			create_sized_image(u, ImageTypes::IMAGEID_Recharge_Shields_Small, false);
			queue_order_front(u, get_order_type(Orders::RechargeShieldsUnitRemoveOverlay), {});
			u->order_state = 3;
		}
		if (u->order_state == 3) {
			if (!target->order_target.unit) target->order_target.unit = u;

			fp8 shield_recharge = fp8::integer(u->unit_type->shield_points) - u->shield_points;
			if (shield_recharge > fp8::integer(5)) shield_recharge = fp8::integer(5);
			fp8 energy_cost = shield_recharge / 2;
			if (energy_cost > target->energy) {
				energy_cost = target->energy;
				shield_recharge = energy_cost * 2;
			}
			set_unit_shield_points(u, u->shield_points + shield_recharge);
			target->energy -= energy_cost;

			if (u->shield_points >= fp8::integer(u->unit_type->shield_points) || target->energy == 0_fp8) {
				if (target->order_target.unit == u) target->order_target.unit = nullptr;
				order_done(u);
			}
		}
	}

	void order_RechargeShieldsUnitRemoveOverlay(unit_t* u) {
		destroy_image_from_to(u, ImageTypes::IMAGEID_Recharge_Shields_Small, ImageTypes::IMAGEID_Recharge_Shields_Large);
		if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	void order_RechargeShieldsBattery(unit_t* u) {
		unit_t* target = u->order_target.unit;
		set_secondary_order(u, get_order_type(Orders::ShieldBattery));
		if (target && unit_can_use_shield_battery(target, u)) {
			set_unit_order(target, get_order_type(Orders::Follow), u);
		} else {
			for (unit_t* n : find_units_noexpand(square_at(u->order_target.pos, 128))) {
				if (unit_can_use_shield_battery(n, u)) set_unit_order(n, get_order_type(Orders::Follow), u);
			}
		}
		order_done(u);
	}

	void order_Feedback(unit_t*  u) {
		unit_t* target = u->order_target.unit;
		if (!target || !ut_has_energy(target) || u_hallucination(target)) {
			order_done(u);
			return;
		}
		auto* tech = get_tech_type(TechTypes::Feedback);
		if (spell_cast_target_movement(u, tech, unit_sight_range(u, true))) {
			if (target->energy != 0_fp8) {
				weapon_deal_damage(get_weapon_type(WeaponTypes::Feedback), target->energy, 1, target, 1_dir, u, u->owner);
				target->energy = 0_fp8;
				u->energy -= fp8::integer(tech->energy_cost);
				play_sound(1061, target);
				if (unit_dying(target)) {
					SpriteTypes sprite_id = (SpriteTypes)((int)SpriteTypes::SPRITEID_Feedback_Hit_Small + unit_sprite_size(target));
					thingy_t* t = create_thingy(get_sprite_type(sprite_id), u->sprite->position, 0);
					if (t) {
						t->sprite->elevation_level = u->sprite->elevation_level + 1;
						if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
					}
				} else {
					create_sized_image(u, ImageTypes::IMAGEID_Feedback_Small);
				}
			}
			order_done(u);
		}
	}

	void order_CastHallucination(unit_t* u) {
		auto* tech = get_tech_type(TechTypes::Hallucination);
		if (spell_cast_target_movement(u, tech, unit_sight_range(u, true))) {
			unit_t* target = u->order_target.unit;
			for (int i = 0; i != 2; ++i) {
				unit_t* n = create_hallucination(target, u->owner);
				if (!n) break;
				auto r = find_unit_placement(n, n->sprite->position, false);
				if (!r.first) {
					destroy_unit(n);
					break;
				}
				move_unit(n, r.second);
				if (u_can_turn(n)) {
					size_t index = lcg_rand(32, 0, 32);
					set_sprite_images_heading_by_index(n->sprite, index);
					set_unit_heading(n, direction_from_index(8 * index));
				}
				show_unit(n);
			}
			u->energy -= fp8::integer(tech->energy_cost);
			play_sound(618, u->order_target.unit);
			create_image(get_image_type(ImageTypes::IMAGEID_Hallucination_Hit), (target->subunit ? target->subunit : target)->sprite, {}, image_order_top);
			order_done(u);
		}
	}

	void order_Neutral(unit_t* u) {
		unit_t* target = u->auto_target_unit;
		if (target) {
			u->auto_target_unit = nullptr;
			if (!us_hidden(target) && !unit_target_is_undetected(u, target) && target->owner != u->owner) {
				st.players[u->owner].controller = player_t::controller_computer_game;
				st.alliances[u->owner][target->owner] = 0;
				st.alliances[target->owner][u->owner] = 0;
				error("a neutral unit attacked something. this triggers computer player behavior which is not supported yet :(");
			}
		}
	}

	void execute_main_order(unit_t* u) {
		switch (u->order_type->id) {
		case Orders::Die:
			order_Die(u);
			return;
		case Orders::IncompleteWarping:
			order_IncompleteWarping(u);
			return;
		case Orders::NukeTrack:
			order_NukeTrack(u);
			return;
		case Orders::WarpIn:
			error("WarpIn");
			return;
		default:
			break;
		}

		if (unit_is_disabled(u) || (!u_can_move(u) && u_cannot_attack(u))) {
			if (u->main_order_timer == 0) u->main_order_timer = 15;
			if (unit_is_disabled(u)) return;
		}

		switch (u->order_type->id) {
		case Orders::TurretGuard:
			order_TurretGuard(u);
			break;
		case Orders::TurretAttack:
			order_TurretAttack(u);
			break;
		case Orders::DroneBuild:
			order_DroneBuild(u);
			break;
		case Orders::PlaceBuilding:
			order_PlaceBuilding(u);
			break;
		case Orders::PlaceProtossBuilding:
			order_PlaceProtossBuilding(u);
			break;
		case Orders::ConstructingBuilding:
			order_ConstructingBuilding(u);
			break;
		case Orders::Repair:
			order_Repair(u);
			break;
		case Orders::ZergBirth:
			order_ZergBirth(u);
			break;
		case Orders::ZergUnitMorph:
			order_ZergUnitMorph(u);
			break;
		case Orders::IncompleteBuilding:
			order_IncompleteBuilding(u);
			break;
		case Orders::IncompleteMorphing:
			order_IncompleteMorphing(u);
			break;
		case Orders::ScarabAttack:
			order_ScarabAttack(u);
			break;
		case Orders::RechargeShieldsUnit:
			order_RechargeShieldsUnit(u);
			break;
		case Orders::BuildingLand:
			order_BuildingLand(u);
			break;
		case Orders::BuildingLiftoff:
			order_BuildingLiftoff(u);
			break;
		case Orders::ResearchTech:
			order_ResearchTech(u);
			break;
		case Orders::Upgrade:
			order_Upgrade(u);
			break;
		case Orders::GatheringInterrupted:
			order_GatheringInterrupted(u);
			break;
		case Orders::GatherWaitInterrupted:
			order_GatherWaitInterrupted(u);
			break;
		case Orders::RechargeShieldsUnitRemoveOverlay:
			order_RechargeShieldsUnitRemoveOverlay(u);
			break;
		case Orders::Sieging:
			order_Sieging(u);
			break;
		case Orders::Unsieging:
			order_Unsieging(u);
			break;
		case Orders::ArchonWarp:
			order_ArchonWarp(u);
			break;
		case Orders::CompletingArchonSummon:
			order_CompletingArchonSummon(u);
			break;
		case Orders::NukeTrain:
			order_NukeTrain(u);
			break;
		case Orders::InitializeArbiter:
			order_InitializeArbiter(u);
			break;
		case Orders::ResetCollision:
			order_ResetCollision(u);
			break;
		case Orders::ResetHarvestCollision:
			order_ResetHarvestCollision(u);
			break;
		case Orders::CTFCOP2:
			error("CTFCOP2");
			break;
		case Orders::SelfDestructing:
			order_SelfDestructing(u);
			break;
		case Orders::Critter:
			order_Critter(u);
			break;
		case Orders::MedicHeal:
			order_MedicHeal(u);
			break;
		case Orders::HealMove:
			order_HealMove(u);
			break;
		case Orders::MedicHoldPosition:
			order_MedicHoldPosition(u);
			break;
		case Orders::MedicHealToIdle:
			order_MedicHealToIdle(u);
			break;
		case Orders::DarkArchonMeld:
			order_DarkArchonMeld(u);
			break;
		default:
			break;
		}
		if (u->order_process_timer) {
			--u->order_process_timer;
			return;
		}
		u->order_process_timer = 8;
		switch (u->order_type->id) {
		case Orders::Stop:
			order_Stop(u);
			break;
		case Orders::Guard:
			order_Guard(u);
			break;
		case Orders::PlayerGuard:
			order_PlayerGuard(u);
			break;
		case Orders::BunkerGuard:
			error("BunkerGuard");
			break;
		case Orders::Move:
			order_Move(u);
			break;
		case Orders::ReaverStop:
			order_ReaverStop(u);
			break;
		case Orders::MoveToAttack:
			order_MoveToTargetOrder(u);
			break;
		case Orders::AttackUnit:
			order_AttackUnit(u);
			break;
		case Orders::AttackFixedRange:
			order_AttackFixedRange(u);
			break;
		case Orders::Hover:
			error("Hover");
			break;
		case Orders::AttackMove:
			order_AttackMove(u);
			break;
		case Orders::InfestedCommandCenter:
			order_InfestedCommandCenter(u);
			break;
		case Orders::UnusedNothing:
			error("UnusedNothing");
			break;
		case Orders::UnusedPowerup:
			error("UnusedPowerup");
			break;
		case Orders::TowerGuard:
			order_TowerGuard(u);
			break;
		case Orders::TowerAttack:
			order_TowerAttack(u);
			break;
		case Orders::SpiderMine:
			order_SpiderMine(u);
			break;
		case Orders::StayInRange:
			order_StayInRange(u);
			break;
		case Orders::Nothing:
			order_Nothing(u);
			break;
		case Orders::DroneStartBuild:
			order_DroneStartBuild(u);
			break;
		case Orders::CastInfestation:
			order_CastInfestation(u);
			break;
		case Orders::MoveToInfest:
			order_MoveToTargetOrder(u);
			break;
		case Orders::InfestingCommandCenter:
			order_InfestingCommandCenter(u);
			break;
		case Orders::MoveToRepair:
			order_MoveToTargetOrder(u);
			break;
		case Orders::PlaceAddon:
			order_PlaceAddon(u);
			break;
		case Orders::ZergBuildingMorph:
			order_ZergBuildingMorph(u);
			break;
		case Orders::BuildNydusExit:
			order_BuildNydusExit(u);
			break;
		case Orders::EnterNydusCanal:
			order_EnterNydusCanal(u);
			break;
		case Orders::IncompleteWarping:
			error("IncompleteWarping");
			break;
		case Orders::Follow:
			order_Follow(u);
			break;
		case Orders::Carrier:
			order_Carrier(u);
			break;
		case Orders::ReaverCarrierMove:
			order_Move(u);
			break;
		case Orders::CarrierStop:
			order_CarrierStop(u);
			break;
		case Orders::CarrierAttack:
			order_CarrierAttack(u);
			break;
		case Orders::CarrierMoveToAttack:
			order_MoveToTargetOrder(u);
			break;
		case Orders::CarrierIgnore2:
			error("CarrierIgnore2");
			break;
		case Orders::CarrierFight:
			order_Carrier(u);
			break;
		case Orders::CarrierHoldPosition:
			order_Carrier(u);
			break;
		case Orders::Reaver:
			order_Reaver(u);
			break;
		case Orders::ReaverAttack:
			order_ReaverAttack(u);
			break;
		case Orders::ReaverMoveToAttack:
			order_MoveToTargetOrder(u);
			break;
		case Orders::ReaverFight:
			order_Reaver(u);
			break;
		case Orders::ReaverHoldPosition:
			order_Reaver(u);
			break;
		case Orders::InterceptorAttack:
			order_InterceptorAttack(u);
			break;
		case Orders::RechargeShieldsBattery:
			order_RechargeShieldsBattery(u);
			break;
		case Orders::InterceptorReturn:
			order_InterceptorReturn(u);
			break;
		case Orders::DroneLand:
			order_DroneLand(u);
			break;
		case Orders::DroneLiftOff:
			error("DroneLiftOff");
			break;
		case Orders::LiftingOff:
			order_LiftingOff(u);
			break;
		case Orders::Larva:
			order_Larva(u);
			break;
		case Orders::Harvest1:
			order_MoveToGas(u);
			break;
		case Orders::Harvest2:
			order_MoveToTargetOrder(u);
			break;
		case Orders::MoveToGas:
			order_MoveToGas(u);
			break;
		case Orders::WaitForGas:
			order_WaitForGas(u);
			break;
		case Orders::HarvestGas:
			order_HarvestGas(u);
			break;
		case Orders::ReturnGas:
			order_ReturnGas(u);
			break;
		case Orders::MoveToMinerals:
			order_MoveToMinerals(u);
			break;
		case Orders::WaitForMinerals:
			order_WaitForMinerals(u);
			break;
		case Orders::MiningMinerals:
			order_MiningMinerals(u);
			break;
		case Orders::ReturnMinerals:
			order_ReturnMinerals(u);
			break;
		case Orders::EnterTransport:
			order_EnterTransport(u);
			break;
		case Orders::PickupIdle:
			order_PickupIdle(u);
			break;
		case Orders::PickupTransport:
			order_PickupTransport(u);
			break;
		case Orders::PickupBunker:
			order_PickupBunker(u);
			break;
		case Orders::Pickup4:
			error("Pickup4");
			break;
		case Orders::WatchTarget:
			order_WatchTarget(u);
			break;
		case Orders::InitCreepGrowth:
			order_InitCreepGrowth(u);
			break;
		case Orders::HoldPosition:
			order_HoldPosition(u);
			break;
		case Orders::QueenHoldPosition:
			order_QueenHoldPosition(u);
			break;
		case Orders::Unload:
			order_Unload(u);
			break;
		case Orders::MoveUnload:
			order_MoveUnload(u);
			break;
		case Orders::FireYamatoGun:
			order_Spell(u);
			break;
		case Orders::MoveToFireYamatoGun:
			order_MoveToTargetOrder(u);
			break;
		case Orders::CastLockdown:
			order_Spell(u);
			break;
		case Orders::Burrowing:
			order_Burrowing(u);
			break;
		case Orders::Burrowed:
			order_Burrowed(u);
			break;
		case Orders::Unburrowing:
			order_Unburrowing(u);
			break;
		case Orders::CastDarkSwarm:
			order_Spell(u);
			break;
		case Orders::CastParasite:
			order_Spell(u);
			break;
		case Orders::CastSpawnBroodlings:
			order_Spell(u);
			break;
		case Orders::CastEMPShockwave:
			order_Spell(u);
			break;
		case Orders::NukeLaunch:
			order_NukeLaunch(u);
			break;
		case Orders::NukePaint:
			order_NukePaint(u);
			break;
		case Orders::NukeUnit:
			order_NukeUnit(u);
			break;
		case Orders::CastNuclearStrike:
			order_CastNuclearStrike(u);
			break;
		case Orders::PlaceMine:
			order_PlaceMine(u);
			break;
		case Orders::RightClickAction:
			order_RightClickAction(u);
			break;
		case Orders::SuicideUnit:
			order_SuicideUnit(u);
			break;
		case Orders::SuicideLocation:
			order_SuicideLocation(u);
			break;
		case Orders::SuicideHoldPosition:
			order_SuicideHoldPosition(u);
			break;
		case Orders::CastRecall:
			order_CastRecall(u);
			break;
		case Orders::Teleport:
			error("Teleport");
			break;
		case Orders::CastScannerSweep:
			order_CastScannerSweep(u);
			break;
		case Orders::Scanner:
			order_Scanner(u);
			break;
		case Orders::CastDefensiveMatrix:
			order_CastDefensiveMatrix(u);
			break;
		case Orders::CastPsionicStorm:
			order_Spell(u);
			break;
		case Orders::CastIrradiate:
			order_Spell(u);
			break;
		case Orders::CastPlague:
			order_Spell(u);
			break;
		case Orders::CastConsume:
			order_Spell(u);
			break;
		case Orders::CastEnsnare:
			order_Spell(u);
			break;
		case Orders::CastStasisField:
			order_Spell(u);
			break;
		case Orders::CastHallucination:
			order_CastHallucination(u);
			break;
		case Orders::Patrol:
			order_Patrol(u);
			break;
		case Orders::CTFCOPInit:
			error("CTFCOPInit");
			break;
		case Orders::CTFCOP2:
			error("CTFCOP2");
			break;
		case Orders::ComputerAI:
			error("ComputerAI");
			break;
		case Orders::AtkMoveEP:
			error("AtkMoveEP");
			break;
		case Orders::HarassMove:
			error("HarassMove");
			break;
		case Orders::AIPatrol:
			error("AIPatrol");
			break;
		case Orders::GuardPost:
			error("GuardPost");
			break;
		case Orders::RescuePassive:
			error("RescuePassive");
			break;
		case Orders::Neutral:
			order_Neutral(u);
			break;
		case Orders::ComputerReturn:
			error("ComputerReturn");
			break;
		case Orders::InitializePsiProvider:
			order_InitializePsiProvider(u);
			break;
		case Orders::HiddenGun:
			error("HiddenGun");
			break;
		case Orders::OpenDoor:
			error("OpenDoor");
			break;
		case Orders::CloseDoor:
			error("CloseDoor");
			break;
		case Orders::HideTrap:
			error("HideTrap");
			break;
		case Orders::RevealTrap:
			error("RevealTrap");
			break;
		case Orders::EnableDoodad:
			error("EnableDoodad");
			break;
		case Orders::WarpIn:
			error("WarpIn");
			break;
		case Orders::MedicIdle:
			order_MedicIdle(u);
			break;
		case Orders::CastRestoration:
			order_Spell(u);
			break;
		case Orders::CastDisruptionWeb:
			order_Spell(u);
			break;
		case Orders::CastMindControl:
			order_CastMindControl(u);
			break;
		case Orders::CastFeedback:
			order_Feedback(u);
			break;
		case Orders::CastOpticalFlare:
			order_Spell(u);
			break;
		case Orders::CastMaelstrom:
			order_Spell(u);
			break;
		default:
			break;
		}
	}

	bool resume_building_unit(unit_t* u, bool place_when_completed) {
		if (st.cheat_operation_cwal) {
			set_unit_hp(u, u->hp + u->hp_construction_rate * 16);
			if (u->remaining_build_time) {
				if (u->remaining_build_time > 16) u->remaining_build_time -= 16;
				else u->remaining_build_time = 0;
				return true;
			}
		} else {
			set_unit_hp(u, u->hp + u->hp_construction_rate);
			if (u->remaining_build_time) {
				--u->remaining_build_time;
				return true;
			}
		}
		finish_building_unit(u);
		if (place_when_completed) {
			if (!place_completed_unit(u)) return false;
		}
		complete_unit(u);
		if (u_grounded_building(u)) {
			if (unit_race(u) == race_t::terran) find_and_connect_addon(u);
		} else {
			// todo: callback for sound (morph)
		}
		return true;
	}

	void rally_unit(unit_t* u, const unit_t* factory_unit) {
		if (!unit_is_factory(factory_unit)) return;
		auto target = factory_unit->building.rally;
		if (target.unit == factory_unit) return;
		if (target.pos.x) {
			if (target.unit && target.unit->owner == u->owner) {
				set_unit_order(u, get_order_type(Orders::Follow), target.unit);
			} else {
				set_unit_order(u, get_order_type(Orders::Move), target.pos);
			}
		}
	}

	void secondary_order_Train(unit_t* u) {
		if (unit_is_disabled(u)) return;
		if (unit_race(u) == race_t::zerg && !unit_is(u, UnitTypes::Zerg_Infested_Command_Center)) return;
		if (u->secondary_order_state == 0 || u->secondary_order_state == 1) {
			if (u->build_queue.empty()) {
				set_secondary_order(u, get_order_type(Orders::Nothing));
				sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
			} else {
				unit_t* build_unit = nullptr;
				const unit_type_t* build_unit_type = u->build_queue.front();
				if (u_grounded_building(u) || unit_is_carrier(u) || unit_is_reaver(u)) {
					if (has_available_supply_for(u->owner, build_unit_type, u->secondary_order_state == 0)) {
						build_unit = create_unit(build_unit_type, u->sprite->position, u->owner);
						if (!build_unit) display_last_error_for_player(u->owner);
					}
				}
				u->current_build_unit = build_unit;
				if (build_unit) {
					u->secondary_order_state = 2;
					sprite_run_anim(u->sprite, iscript_anims::IsWorking);
				} else {
					u->secondary_order_state = 1;
				}
			}
		} else if (u->secondary_order_state == 2) {
			unit_t* build_unit = u->current_build_unit;
			if (build_unit) {
				if (resume_building_unit(build_unit, true)) {
					if (u_completed(build_unit)) {
						if (unit_is(build_unit, UnitTypes::Terran_Nuclear_Missile)) hide_unit(build_unit);
						else rally_unit(build_unit, u);
						if (!u->build_queue.empty()) u->build_queue.erase(u->build_queue.begin());
						u->current_build_unit = nullptr;
						u->secondary_order_state = 0;
					}
				} else {
					if (!u->build_queue.empty()) {
						if (u->current_build_unit) cancel_building_unit(u->current_build_unit);
						else if (!ut_building(u->build_queue.front())) refund_unit_costs(u->owner, u->build_queue.front());
						u->build_queue.erase(u->build_queue.begin());
					}
					u->current_build_unit = nullptr;
					u->secondary_order_state = 0;
				}
			} else u->secondary_order_state = 0;
		}
	}

	void secondary_order_BuildAddon(unit_t* u) {
		if (unit_is_disabled(u)) return;
		unit_t* addon = u->current_build_unit;
		if (!addon) {
			u->building.addon_build_type = nullptr;
			set_secondary_order(u, get_order_type(Orders::Nothing));
			return;
		}
		if (!u_grounded_building(u) && !u_completed(addon)) {
			cancel_building_unit(addon);
			u->building.addon_build_type = nullptr;
			set_secondary_order(u, get_order_type(Orders::Nothing));
			return;
		}
		resume_building_unit(addon, false);
		if (u_completed(addon)) {
			// callback for sound
			connect_addon(u, addon);
			u->building.addon_build_type = nullptr;
			set_secondary_order(u, get_order_type(Orders::Nothing));
		}
	}

	void secondary_order_Cloak(unit_t* u) {
		set_unit_cloaked(u);
	}

	void secondary_order_Decloak(unit_t* u) {
		set_secondary_order(u, get_order_type(Orders::Nothing));
	}

	void secondary_order_SpawningLarva(unit_t* u) {
		if (u->order_process_timer) return;
		if (u->owner == 11) return;
		int larva_count = 0;
		for (unit_t* n : find_units_noexpand(square_at(u->sprite->position, 32 * 8))) {
			if (!unit_is(n, UnitTypes::Zerg_Larva)) continue;
			if (n->connected_unit != u) continue;
			if (us_hidden(n)) continue;
			++larva_count;
			if (larva_count >= 3) return;
		}
		if (u->building.larva_timer) {
			--u->building.larva_timer;
			return;
		}
		if (spawn_larva(u)) u->building.larva_timer = 37;
		else u->building.larva_timer = 3;
	}

	void secondary_order_SpreadCreep(unit_t* u) {
		if (unit_is_hatchery(u)) secondary_order_SpawningLarva(u);
		if (u->building.creep_timer) {
			--u->building.creep_timer;
			return;
		}
		u->building.creep_timer = 15;
		bool any_tiles_occupied = false;
		if (!spread_creep(get_unit_type(UnitTypes::Zerg_Hive), u->sprite->position, &any_tiles_occupied) && !any_tiles_occupied) {
			if (unit_is_hatchery(u)) set_secondary_order(u, get_order_type(Orders::SpawningLarva));
			else set_secondary_order(u, get_order_type(Orders::Nothing));
		}
	}

	void secondary_order_CloakNearbyUnits(unit_t* u) {
		auto* w = unit_air_weapon(u);
		if (!w) error("secondary_order_CloakNearbyUnits: no air weapon");
		int range = weapon_max_range(u, w);
		auto area = square_at(u->sprite->position, range);
		if (area.from.x < 0) area.from.x = 0;
		else if (area.to.x > (int)game_st.map_width) area.to.x = (int)game_st.map_width;
		if (area.from.y < 0) area.from.y = 0;
		else if (area.to.y > (int)game_st.map_height) area.to.y = (int)game_st.map_height;
		for (unit_t* target : find_units_noexpand(area)) {
			if (target->owner != u->owner) continue;
			if (ut_building(target)) continue;
			if (unit_is_arbiter(target)) continue;
			if (u_hallucination(target)) continue;
			if (ut_powerup(target)) continue;
			if (unit_is(target, UnitTypes::Terran_Nuclear_Missile)) continue;
			if (target->order_type->id == Orders::WarpIn) continue;
			if (!unit_target_in_range(u, target, range)) continue;
			set_unit_cloaked(target);
			if (!u_passively_cloaked(target)) u_set_status_flag(target, unit_t::status_flag_passively_cloaked);
		}
	}

	void secondary_order_TrainFighter(unit_t* u) {
		if (!unit_is_carrier(u) && !unit_is_reaver(u)) return;
		if (u->secondary_order_state == 0 || u->secondary_order_state == 1) {
			if (u->build_queue.empty()) {
				u->secondary_order_state = 3;
				u->current_build_unit = nullptr;
			} else {
				unit_t* build_unit = nullptr;
				const unit_type_t* build_unit_type = u->build_queue.front();
				if (u_grounded_building(u) || unit_is_carrier(u) || unit_is_reaver(u)) {
					if (has_available_supply_for(u->owner, build_unit_type, u->secondary_order_state == 0)) {
						build_unit = create_unit(build_unit_type, u->sprite->position, u->owner);
						if (!build_unit) display_last_error_for_player(u->owner);
					}
				}
				u->current_build_unit = build_unit;
				if (build_unit) {
					if (unit_is_fighter(build_unit)) build_unit->fighter.parent = u;
					u->secondary_order_state = 2;
				} else u->secondary_order_state = 1;
			}
		} else if (u->secondary_order_state == 2) {
			unit_t* build_unit = u->current_build_unit;
			if (!build_unit) {
				u->secondary_order_state = 0;
			} else {
				resume_building_unit(build_unit, false);
				if (u_completed(build_unit)) {
					hide_unit(build_unit);
					if (unit_is_fighter(build_unit)) {
						if (unit_is_carrier(u)) {
							u->carrier.inside_units.push_front(*build_unit);
							++u->carrier.inside_count;
						} else if (unit_is_reaver(u)) {
							u->reaver.inside_units.push_front(*build_unit);
							++u->reaver.inside_count;
						}
						build_unit->fighter.is_outside = false;
					}
					u->build_queue.erase(u->build_queue.begin());
					u->current_build_unit = nullptr;
					u->secondary_order_state = 0;
				}
			}
		} else if (u->secondary_order_state == 3) {
			if (unit_is_carrier(u)) {
				for (unit_t* n : ptr(u->carrier.inside_units)) {
					if (n->hp >= n->unit_type->hitpoints) continue;
					set_unit_hp(n, n->hp + 128_fp8);
					break;
				}
			} else u->secondary_order_state = 4;
		}
	}

	void secondary_order_ShieldBattery(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (u->secondary_order_state == 0) {
			if (target) {
				sprite_run_anim(u->sprite, iscript_anims::IsWorking);
				u->secondary_order_state = 1;
			}
		} else if (u->secondary_order_state == 1) {
			if (target) {
				if (target->order_type->id != Orders::RechargeShieldsUnit) {
					u->order_target.unit = nullptr;
					u->secondary_order_state = 2;
				}
			} else u->secondary_order_state = 2;
		} else if (u->secondary_order_state == 2) {
			if (target) u->secondary_order_state = 1;
			else {
				sprite_run_anim(u->sprite, iscript_anims::WorkingToIdle);
				u->secondary_order_state = 0;
			}
		}
	}

	void execute_secondary_order(unit_t* u) {
		if (u->secondary_order_type->id == Orders::Hallucination2) {
			if (u->defensive_matrix_hp != 0_fp8 || u->stim_timer || u->ensnare_timer || u->lockdown_timer || u->irradiate_timer || u->stasis_timer || u->parasite_flags || u->storm_timer || u->plague_timer || u->blinded_by || u->maelstrom_timer) {
				kill_unit(u);
			}
			return;
		}
		if (unit_is_disabled(u)) return;
		switch (u->secondary_order_type->id) {
		case Orders::BuildAddon:
			secondary_order_BuildAddon(u);
			break;
		case Orders::Train:
			secondary_order_Train(u);
			break;
		case Orders::TrainFighter:
			secondary_order_TrainFighter(u);
			break;
		case Orders::ShieldBattery:
			secondary_order_ShieldBattery(u);
			break;
		case Orders::SpawningLarva:
			secondary_order_SpawningLarva(u);
			break;
		case Orders::SpreadCreep:
			secondary_order_SpreadCreep(u);
			break;
		case Orders::Cloak:
			secondary_order_Cloak(u);
			break;
		case Orders::Decloak:
			secondary_order_Decloak(u);
			break;
		case Orders::CloakNearbyUnits:
			secondary_order_CloakNearbyUnits(u);
			break;
		default:
			break;
		}
	}

	void update_unit(unit_t* u) {

		update_unit_values(u);

		execute_main_order(u);
		execute_secondary_order(u);

		if (u->subunit && !ut_turret(u)) {
			auto ius = make_thingy_setter(iscript_unit, u->subunit);
			update_unit(u->subunit);
		}

		if (u->sprite) {
			if (!iscript_execute_sprite(u->sprite)) u->sprite = nullptr;
		}

		if (!u->sprite && !unit_dead(u)) error("update_unit: unit has null sprite");

	}


	bool unit_is_at_move_target(const flingy_t* u) const {
		return u->sprite->position == u->move_target.pos;
	}

	bool unit_dead(const unit_t* u) const {
		if (!u->order_type) return true;
		return u->order_type->id == Orders::Die && u->order_state == 1;
	}

	bool unit_dying(const unit_t* u) const {
		if (!u->order_type) return true;
		return u->order_type->id == Orders::Die;
	}

	bool rectangle_can_fit_at(xy pos, int width, int height) const {
		rect area;
		area.from = pos - xy(width, width) / 2;
		area.to = area.from + xy(width, height);
		if (!is_in_inner_map_bounds(area)) return false;
		if (!is_walkable(pos)) return false;
		std::array<int, 4> inner;
		inner[0] = (height - 1) / 2;
		inner[1] = -(width - (width / 2) - 1);
		inner[2] = -(height - (height / 2) - 1);
		inner[3] = (width - 1) / 2;
		return can_fit_at(pos, inner);
	}

	bool unit_type_can_fit_at(const unit_type_t* unit_type, xy pos) const {
		if (!is_in_map_bounds(unit_type, pos)) return false;
		if (!is_walkable(pos)) return false;
		std::array<int, 4> inner;
		inner[0] = unit_type->dimensions.from.y;
		inner[1] = -unit_type->dimensions.to.x;
		inner[2] = -unit_type->dimensions.to.y;
		inner[3] = unit_type->dimensions.from.x;
		return can_fit_at(pos, inner);
	}

	bool can_fit_at(xy pos, std::array<int, 4> inner) const {
		auto cmp_u = [&](int v, const regions_t::contour& c) {
			return v < c.v[0];
		};
		auto cmp_l = [&](const regions_t::contour& c, int v) {
			return c.v[0] < v;
		};

		auto& c0 = game_st.regions.contours[0];
		for (auto i = std::lower_bound(c0.begin(), c0.end(), pos.y, cmp_l); i != c0.begin();) {
			--i;
			if (inner[0] + i->v[0] < pos.y) break;
			if (inner[1] + i->v[1] <= pos.x && inner[3] + i->v[2] >= pos.x) return false;
		}
		auto& c1 = game_st.regions.contours[1];
		for (auto i = std::upper_bound(c1.begin(), c1.end(), pos.x, cmp_u); i != c1.end(); ++i) {
			if (inner[1] + i->v[0] > pos.x) break;
			if (inner[2] + i->v[1] <= pos.y && inner[0] + i->v[2] >= pos.y) return false;
		}
		auto& c2 = game_st.regions.contours[2];
		for (auto i = std::upper_bound(c2.begin(), c2.end(), pos.y, cmp_u); i != c2.end(); ++i) {
			if (inner[2] + i->v[0] > pos.y) break;
			if (inner[1] + i->v[1] <= pos.x && inner[3] + i->v[2] >= pos.x) return false;
		}
		auto& c3 = game_st.regions.contours[3];
		for (auto i = std::lower_bound(c3.begin(), c3.end(), pos.x, cmp_l); i != c3.begin();) {
			--i;
			if (inner[3] + i->v[0] < pos.x) break;
			if (inner[2] + i->v[1] <= pos.y && inner[0] + i->v[2] >= pos.y) return false;
		}

		return true;
	}

	bool unit_target_is_enemy(const unit_t* u, const unit_t* target) const {
		int n_owner = target->owner;
		if (n_owner == 11) n_owner = target->sprite->owner;
		return st.alliances[u->owner][n_owner] == 0;
	}

	unit_t* get_largest_blocking_unit(const unit_t* u, rect bounds) const {
		int largest_unit_area = 0;
		unit_t* largest_unit = nullptr;
		for (unit_t* nu : find_units(bounds)) {
			if (unit_can_collide_with(u, nu) && unit_finder_unit_in_bounds(nu, bounds)) {
				rect n_bb = unit_type_bounding_box(nu->unit_type);
				int p = (n_bb.to.x - n_bb.from.x) * (n_bb.to.y - n_bb.from.y);
				if (p > largest_unit_area) {
					largest_unit_area = p;
					largest_unit = nu;
				}
			}
		}
		return largest_unit;
	}
	std::pair<bool, unit_t*> is_blocked(const unit_t* u, xy pos) const {
		rect bounds = unit_inner_bounding_box(u, pos);
		if (!is_in_inner_map_bounds(bounds)) return std::make_pair(true, nullptr);
		unit_t* largest_unit = get_largest_blocking_unit(u, bounds);
		if (!largest_unit) return std::make_pair(!unit_type_can_fit_at(u->unit_type, pos), nullptr);
		return std::make_pair(true, largest_unit);
	}

	void set_flingy_move_target(flingy_t* u, xy target_pos, unit_t* target_unit = nullptr) {
		if (u->move_target.pos == target_pos && (!target_unit || u->move_target.unit == target_unit)) return;
		u->move_target.pos = target_pos;
		u->move_target.unit = target_unit;
		u->next_movement_waypoint = target_pos;
		u_unset_movement_flag(u, 4);
		u_set_movement_flag(u, 1);
	}

	void set_unit_move_target(unit_t* u, xy move_target) {
		if (u->move_target.pos == move_target) return;
		if (u->path) u->path->state_flags |= 1;
		move_target = restrict_move_target_to_valid_bounds(u, move_target);
		set_flingy_move_target(u, move_target);
		if (u_immovable(u)) u_unset_status_flag(u, unit_t::status_flag_immovable);
		u->move_target_timer = 15;
		if (!u->order_queue.empty() && !u->order_queue.front().order_type->unk7) {
			u_set_movement_flag(u, 0x20);
		} else {
			u_unset_movement_flag(u, 0x20);
		}
	}

	void set_unit_move_target(unit_t* u, unit_t* target) {
		xy target_pos = target->sprite->position;
		if (u->move_target.pos == target_pos && u->move_target.unit == target) return;
		if (u->path) u->path->state_flags |= 1;
		set_flingy_move_target(u, target_pos, target);
		if (u_immovable(u)) u_unset_status_flag(u, unit_t::status_flag_immovable);
		u->move_target_timer = 15;
		if (!u->order_queue.empty() && !u->order_queue.front().order_type->unk7) {
			u_set_movement_flag(u, 0x20);
		} else {
			u_unset_movement_flag(u, 0x20);
		}
	}

	void set_current_velocity_direction(flingy_t* u, direction_t current_velocity_direction) {
		if (u->current_velocity_direction == current_velocity_direction) return;
		u->current_velocity_direction = current_velocity_direction;
		u->velocity = direction_xy(current_velocity_direction, u->current_speed);
	}
	void set_current_speed(flingy_t* u, fp8 current_speed) {
		if (u->current_speed == current_speed) return;
		u->current_speed = current_speed;
		u->velocity = direction_xy(u->current_velocity_direction, u->current_speed);
	}

	void set_next_speed(flingy_t* u, fp8 next_speed) {
		u->next_speed = next_speed;
		set_current_speed(u, next_speed);
	}

	direction_t unit_turn_rate(const flingy_t* u, direction_t desired_turn) const {
		ufp8 uturn_rate = u->flingy_turn_rate.as_unsigned();
		if (u->flingy_movement_type != 2) uturn_rate /= 2u;
		fp8 turn_rate = uturn_rate.as_signed();
		fp8 turn = fp8::extend(desired_turn);
		if (turn > turn_rate) turn = turn_rate;
		else if (turn < -turn_rate) turn = -turn_rate;
		return direction_t::truncate(turn);
	}

	void set_desired_velocity_direction(flingy_t* u, direction_t desired_velocity_direction) {
		u->desired_velocity_direction = desired_velocity_direction;
		if (u->next_velocity_direction != desired_velocity_direction) {
			auto turn = unit_turn_rate(u, desired_velocity_direction - u->next_velocity_direction);
			set_current_velocity_direction(u, u->next_velocity_direction + turn);
		} else {
			set_current_velocity_direction(u, desired_velocity_direction);
		}
	}

	void update_current_velocity_direction_towards_waypoint(flingy_t* u) {
		if (u->position != u->next_movement_waypoint) {
			set_desired_velocity_direction(u, xy_direction(u->next_movement_waypoint - u->position));
		} else {
			if (u->position != u->next_target_waypoint) {
				set_desired_velocity_direction(u, xy_direction(u->next_target_waypoint - u->position));
			} else {
				set_desired_velocity_direction(u, u->heading);
			}
		}
	}

	void update_unit_heading(flingy_t* u, direction_t velocity_direction) {
		u->next_velocity_direction = velocity_direction;
		if (!u_movement_flag(u, 2) || u_movement_flag(u, 1)) {
			direction_t turn = u->desired_velocity_direction - u->heading;
			if (turn > direction_t::truncate(u->flingy_turn_rate)) turn = direction_t::truncate(u->flingy_turn_rate);
			else if (turn < direction_t::truncate(-u->flingy_turn_rate)) turn = -direction_t::truncate(u->flingy_turn_rate);
			u->heading += turn;
			if (u->flingy_type->id >= (FlingyTypes)0x8d && u->flingy_type->id <= (FlingyTypes)0xab) {
				u->flingy_turn_rate += 1_fp8;
			} else if (u->flingy_type->id >= (FlingyTypes)0xc9 && u->flingy_type->id <= (FlingyTypes)0xce) {
				u->flingy_turn_rate += 1_fp8;
			}
			if (velocity_direction == u->desired_velocity_direction) {
				if (u->heading == u->desired_velocity_direction) {
					u_unset_movement_flag(u, 1);
				}
			}
		}
		auto heading = u->heading;
		for (image_t* image : ptr(u->sprite->images)) {
			set_image_heading(image, heading);
		}
	}

	void update_current_speed_towards_waypoint(flingy_t* u) {
		if (u->flingy_movement_type == 0) {
			if (unit_is_at_move_target(u)) {
				if (u->next_speed < 192_fp8) {
					if (!u_movement_flag(u, 0x20) && !u_movement_flag(u, 0x10)) {
						u_unset_movement_flag(u, 4);
						set_current_speed(u, 0_fp8);
						return;
					}
				}
			}
		} else if (u->flingy_movement_type == 1) {
			if (unit_is_at_move_target(u)) {
				u_unset_movement_flag(u, 4);
				set_current_speed(u, 0_fp8);
				return;
			}
		} else if (u->flingy_movement_type == 2) {
			if (unit_is_at_move_target(u)) {
				u_unset_movement_flag(u, 4);
				set_current_speed(u, 0_fp8);
			} else {
				if (!u_movement_flag(u, 1)) {
					set_current_speed(u, u->next_speed);
				} else {
					auto heading_error = fp8::extend(u->heading - u->desired_velocity_direction).abs();
					if (heading_error >= 32_fp8) {
						if (u_movement_flag(u, 2)) {
							u_unset_movement_flag(u, 2);
							u_unset_movement_flag(u, 4);
							set_current_speed(u, 0_fp8);
						}
					} else {
						set_current_speed(u, u->next_speed);
					}
				}
			}
			return;
		}
		set_current_speed(u, u->next_speed);
		int d = xy_length(u->next_movement_waypoint - u->position);
		bool accelerate = false;
		if (u->current_velocity_direction == u->desired_velocity_direction) accelerate = true;
		else if (d >= 32) accelerate = true;
		else {
			fp8 turn_rate = u->flingy_turn_rate;
			fp8 diff = fp8::extend(u->desired_velocity_direction - u->current_velocity_direction).abs();

			unsigned int val = fp8::divide_multiply(diff * 2 + turn_rate - 1_fp8, turn_rate, u->next_speed).integer_part();
			if (val * 3 / 2 <= (unsigned int)d) accelerate = true;
		}
		if (accelerate) {
			if (u->flingy_movement_type == 0) {
				if (!u_movement_flag(u, 0x20) || u->next_movement_waypoint != u->move_target.pos) {
					if (!u_movement_flag(u, 0x10) || u_movement_flag(u, 0x40)) {
						fp8 remaining_d = xy_length(to_xy_fp8(u->next_movement_waypoint) - u->exact_position);
						if (unit_halt_distance(u) >= remaining_d) accelerate = false;
					}
				}
			}
		}
		fp8 speed = u->current_speed;
		if (accelerate) speed += u->flingy_acceleration;
		else speed -= u->flingy_acceleration;
		if (speed < 0_fp8) speed = 0_fp8;
		else if (speed > u->flingy_top_speed) speed = u->flingy_top_speed;
		set_current_speed(u, speed);
	}

	struct execute_movement_struct {
		bool refresh_vision = false;
		bool starting_movement = false;
		bool stopping_movement = false;
		fp8 speed;
		xy position;
		xy_fp8 exact_position;
		int pre_movement_flags;
		int post_movement_flags;
	};

	void set_movement_flags(flingy_t* u, execute_movement_struct& ems) {
		ems.starting_movement = false;
		ems.stopping_movement = false;
		if (!unit_is_at_move_target(u)) {
			if (!u_movement_flag(u, 2)) {
				if (u->flingy_movement_type != 2 || !u_movement_flag(u, 8)) u_set_movement_flag(u, 2);
				if (!u_movement_flag(u, 8)) ems.starting_movement = true;
			}
		} else {
			if (u_movement_flag(u, 2)) {
				u_unset_movement_flag(u, 2);
				if (!u_movement_flag(u, 8)) ems.stopping_movement = true;
			}
		}
	}

	void set_movement_values(flingy_t* u, execute_movement_struct& ems) {
		ems.speed = u->current_speed;
		if (!u_movement_flag(u, 2) || u->current_speed == 0_fp8) {
			ems.position = u->position;
			ems.exact_position = u->exact_position;
		} else {
			fp8 remaining_d = xy_length(to_xy_fp8(u->next_movement_waypoint) - u->exact_position);
			if (u->current_speed >= remaining_d) {
				ems.position = u->next_movement_waypoint;
				ems.exact_position = to_xy_fp8(ems.position);
				ems.speed = remaining_d;
			} else {
				ems.exact_position = u->exact_position + u->velocity;
				ems.position = to_xy(ems.exact_position);
			}
			if (u->flingy_movement_type == 2) {
				set_current_speed(u, 0_fp8);
			}
		}
	}

	void update_unit_movement_values(flingy_t* u, execute_movement_struct& ems) {
		ems.pre_movement_flags = u->movement_flags;
		update_current_velocity_direction_towards_waypoint(u);
		update_current_speed_towards_waypoint(u);
		set_movement_flags(u, ems);
		set_movement_values(u, ems);
		ems.post_movement_flags = u->movement_flags;
		u->movement_flags = ems.pre_movement_flags;
	}

	bool check_unit_movement_terrain_collision(unit_t* u, xy movement) {
		xy new_pos = u->sprite->position + movement;
		if (new_pos.x >= u->terrain_no_collision_bounds.from.x && new_pos.y >= u->terrain_no_collision_bounds.from.y) {
			if (new_pos.x <= u->terrain_no_collision_bounds.to.x && new_pos.y <= u->terrain_no_collision_bounds.to.y) return false;
		}
		xy pos = u->sprite->position;
		rect bounds = {pos - xy(128, 128), pos + xy(128, 128)};

		std::array<int, 4> inner;
		inner[0] = u->unit_type->dimensions.from.y;
		inner[1] = -u->unit_type->dimensions.to.x;
		inner[2] = -u->unit_type->dimensions.to.y;
		inner[3] = u->unit_type->dimensions.from.x;

		auto cmp_u = [&](int v, const regions_t::contour& c) {
			return v < c.v[0];
		};
		auto cmp_l = [&](const regions_t::contour& c, int v) {
			return c.v[0] < v;
		};

		auto& c0 = game_st.regions.contours[0];
		auto& c1 = game_st.regions.contours[1];
		auto& c2 = game_st.regions.contours[2];
		auto& c3 = game_st.regions.contours[3];

		auto i0 = std::upper_bound(c0.begin(), c0.end(), pos.y - inner[0], cmp_u);
		auto i1 = std::lower_bound(c1.begin(), c1.end(), pos.x - inner[1], cmp_l);
		auto i2 = std::lower_bound(c2.begin(), c2.end(), pos.y - inner[2], cmp_l);
		auto i3 = std::upper_bound(c3.begin(), c3.end(), pos.x - inner[3], cmp_u);

		auto expand = [&](regions_t::contour c) {
			c.v[0] += inner[c.dir];
			c.v[1] += inner[c.flags & 3];
			c.v[2] += inner[(c.flags >> 2) & 3];
			return c;
		};

		while (true) {
			bool done = true;
			if (i0 != c0.begin()) {
				done = false;
				--i0;
				auto c = expand(*i0);
				if (c.v[0] >= bounds.from.y) {
					if (c.v[2] >= bounds.from.x && c.v[1] <= bounds.to.x) {
						if (pos.x < c.v[1] && pos.y - c.v[0] < c.v[1] - pos.x) {
							bounds.to.x = c.v[1] - 1;
						} else if (pos.x > c.v[2] && pos.y - c.v[0] < pos.x - c.v[2]) {
							bounds.from.x = c.v[2] + 1;
						} else {
							bounds.from.y = c.v[0] + 1;
						}
					}
				} else {
					i0 = c0.begin();
				}
			}
			if (i1 != c1.end()) {
				done = false;
				auto c = expand(*i1);
				++i1;
				if (c.v[0] <= bounds.to.x) {
					if (c.v[2] >= bounds.from.y && c.v[1] <= bounds.to.y) {
						if (pos.y < c.v[1] && c.v[0] - pos.x < c.v[1] - pos.y) {
							bounds.to.y = c.v[1] - 1;
						} else if (pos.y > c.v[2] && c.v[0] - pos.x < pos.y - c.v[2]) {
							bounds.from.y = c.v[2] + 1;
						} else {
							bounds.to.x = c.v[0] - 1;
						}
					}
				} else {
					i1 = c1.end();
				}
			}
			if (i2 != c2.end()) {
				done = false;
				auto c = expand(*i2);
				++i2;
				if (c.v[0] <= bounds.to.y) {
					if (c.v[2] >= bounds.from.x && c.v[1] <= bounds.to.x) {
						if (pos.x < c.v[1] && c.v[0] - pos.y < c.v[1] - pos.x) {
							bounds.to.x = c.v[1] - 1;
						} else if (pos.x > c.v[2] && c.v[0] - pos.y < pos.x - c.v[2]) {
							bounds.from.x = c.v[2] + 1;
						} else {
							bounds.to.y = c.v[0] - 1;
						}
					}
				} else {
					i2 = c2.end();
				}
			}
			if (i3 != c3.begin()) {
				done = false;
				--i3;
				auto c = expand(*i3);
				if (c.v[0] >= bounds.from.x) {
					if (c.v[2] >= bounds.from.y && c.v[1] <= bounds.to.y) {
						if (pos.y < c.v[1] && pos.x - c.v[0] < c.v[1] - pos.y) {
							bounds.to.y = c.v[1] - 1;
						} else if (pos.y > c.v[2] && pos.x - c.v[0] < pos.y - c.v[2]) {
							bounds.from.y = c.v[2] + 1;
						} else {
							bounds.from.x = c.v[0] + 1;
						}
					}
				} else {
					i3 = c3.begin();
				}
			}
			if (done) break;
		}

		u->terrain_no_collision_bounds = bounds;
		if (new_pos.x >= u->terrain_no_collision_bounds.from.x && new_pos.y >= u->terrain_no_collision_bounds.from.y) {
			if (new_pos.x <= u->terrain_no_collision_bounds.to.x && new_pos.y <= u->terrain_no_collision_bounds.to.y) return false;
		}

		xy min_pos = pos;
		xy max_pos = new_pos;
		if (max_pos.x < min_pos.x) std::swap(min_pos.x, max_pos.x);
		if (max_pos.y < min_pos.y) std::swap(min_pos.y, max_pos.y);

		if (new_pos.y < pos.y) {
			for (auto i = std::upper_bound(c0.begin(), c0.end(), pos.y - inner[0], cmp_u); i != c0.begin();) {
				--i;
				auto c = expand(*i);
				if (c.v[0] < new_pos.y) break;
				if (c.v[1] <= max_pos.x && c.v[2] >= min_pos.x) {
					if (~c.flags & 0x30) return true;
					if (c.v[1] <= min_pos.x) return true;
					if (c.v[2] >= max_pos.x) return true;
				}
			}
		}
		if (new_pos.x > pos.x) {
			for (auto i = std::lower_bound(c1.begin(), c1.end(), pos.x - inner[1], cmp_l); i != c1.end(); ++i) {
				auto c = expand(*i);
				if (c.v[0] > new_pos.x) break;
				if (c.v[1] <= max_pos.y && c.v[2] >= min_pos.y) {
					if (~c.flags & 0x30) return true;
					if (c.v[1] <= min_pos.y) return true;
					if (c.v[2] >= max_pos.y) return true;
				}
			}
		}
		if (new_pos.y > pos.y) {
			for (auto i = std::lower_bound(c2.begin(), c2.end(), pos.y - inner[2], cmp_l); i != c2.end(); ++i) {
				auto c = expand(*i);
				if (c.v[0] > new_pos.y) break;
				if (c.v[1] <= max_pos.x && c.v[2] >= min_pos.x) {
					if (~c.flags & 0x30) return true;
					if (c.v[1] <= min_pos.x) return true;
					if (c.v[2] >= max_pos.x) return true;
				}
			}
		}
		if (new_pos.x < pos.x) {
			for (auto i = std::upper_bound(c3.begin(), c3.end(), pos.x - inner[3], cmp_u); i != c3.begin();) {
				--i;
				auto c = expand(*i);
				if (c.v[0] < new_pos.x) break;
				if (c.v[1] <= max_pos.y && c.v[2] >= min_pos.y) {
					if (~c.flags & 0x30) return true;
					if (c.v[1] <= min_pos.y) return true;
					if (c.v[2] >= max_pos.y) return true;
				}
			}
		}
		return false;
	}

	bool check_unit_movement_terrain_collision(unit_t* u, const execute_movement_struct& ems) {
		if (u_ground_unit(u)) {
			return check_unit_movement_terrain_collision(u, ems.position - u->sprite->position);
		} else {
			return !is_in_map_bounds(unit_bounding_box(u, ems.position));
		}
	}

	unit_t* check_unit_movement_unit_collision(unit_t* u, const execute_movement_struct& ems) {
		if (us_hidden(u)) return nullptr;
		xy movement = ems.position - u->sprite->position;

		auto cmp_u = [&](int a, auto& b) {
			return a < b.value;
		};
		auto cmp_l = [&](auto& a, int b) {
			return a.value < b;
		};

		auto new_bb = u->unit_finder_bounding_box;
		new_bb.from += movement;
		new_bb.to += movement;

		if (movement.x < 0) {
			auto& arr = st.unit_finder_x;
			for (auto i = std::upper_bound(arr.begin(), arr.end(), u->unit_finder_bounding_box.from.x, cmp_u); i != arr.begin();) {
				--i;
				if (i->value < new_bb.from.x) break;
				if (i->u->unit_finder_bounding_box.from.y <= new_bb.to.y && i->u->unit_finder_bounding_box.to.y >= new_bb.from.y) {
					if (unit_can_collide_with(u, i->u) && u_ground_unit(i->u)) {
						return i->u;
					}
				}
			}
		} else if (movement.x > 0) {
			auto& arr = st.unit_finder_x;
			for (auto i = std::lower_bound(arr.begin(), arr.end(), u->unit_finder_bounding_box.to.x, cmp_l); i != arr.end(); ++i) {
				if (i->value > new_bb.to.x) break;
				if (i->u->unit_finder_bounding_box.from.y <= new_bb.to.y && i->u->unit_finder_bounding_box.to.y >= new_bb.from.y) {
					if (unit_can_collide_with(u, i->u) && u_ground_unit(i->u)) {
						return i->u;
					}
				}
			}
		}
		if (movement.y < 0) {
			auto& arr = st.unit_finder_y;
			for (auto i = std::upper_bound(arr.begin(), arr.end(), u->unit_finder_bounding_box.from.y, cmp_u); i != arr.begin();) {
				--i;
				if (i->value < new_bb.from.y) break;
				if (i->u->unit_finder_bounding_box.from.x <= new_bb.to.x && i->u->unit_finder_bounding_box.to.x >= new_bb.from.x) {
					if (unit_can_collide_with(u, i->u) && u_ground_unit(i->u)) {
						return i->u;
					}
				}
			}
		} else if (movement.y > 0) {
			auto& arr = st.unit_finder_y;
			for (auto i = std::lower_bound(arr.begin(), arr.end(), u->unit_finder_bounding_box.to.y, cmp_l); i != arr.end(); ++i) {
				if (i->value > new_bb.to.y) break;
				if (i->u->unit_finder_bounding_box.from.x <= new_bb.to.x && i->u->unit_finder_bounding_box.to.x >= new_bb.from.x) {
					if (unit_can_collide_with(u, i->u) && u_ground_unit(i->u)) {
						return i->u;
					}
				}
			}
		}
		return nullptr;
	}

	unit_t* check_ground_unit_movement_unit_collision(unit_t* u, const execute_movement_struct& ems) {
		if (u_ground_unit(u)) {
			return check_unit_movement_unit_collision(u, ems);
		} else {
			return nullptr;
		}
	}

	void update_repulse_direction(unit_t* u) {
		direction_t repulse_dir = u->repulse_direction;
		if (!unit_is_at_move_target(u)) {
			direction_t target_dir = xy_direction(u->move_target.pos - u->position);
			size_t index = direction_index(repulse_dir - target_dir);
			repulse_dir = target_dir + direction_from_index(index % 32 + (index < 128 ? 32 : -64));
		}
		auto bb = unit_sprite_bounding_box(u);
		if (bb.from.x < 32) {
			if (repulse_dir < 0_dir) repulse_dir = -repulse_dir;
			u->repulse_flags &= 0xf;
		} else if ((size_t)bb.to.x > game_st.map_width - 32) {
			if (repulse_dir > 0_dir) repulse_dir = -repulse_dir;
			u->repulse_flags &= 0xf;
		}
		if (bb.from.y < 32) {
			if (repulse_dir + 64_dir >= 0_dir) repulse_dir = -repulse_dir - 128_dir;
			u->repulse_flags &= 0xf;
		} else if ((size_t)bb.to.y > game_st.map_height - 32) {
			if (repulse_dir + 64_dir < 0_dir) repulse_dir = -repulse_dir - 128_dir;
			u->repulse_flags &= 0xf;
		}
		u->repulse_direction = repulse_dir;
	}

	bool apply_repulse_field(unit_t* u, execute_movement_struct& ems) {
		if (!u_can_move(u)) return false;
		if (ut_building(u)) return false;
		if (unit_is(u, UnitTypes::Protoss_Interceptor)) return false;
		if (u_order_not_interruptible(u)) return false;
		if (u_iscript_nobrk(u)) return false;
		int strength = st.repulse_field[u->repulse_index];
		if (strength <= 1) {
			u->repulse_flags &= ~7;
			return false;
		}
		if (u->repulse_flags & 7) {
			--u->repulse_flags;
		} else {
			u->repulse_flags |= 7;
			update_repulse_direction(u);
		}
		xy_fp8 repulse = direction_xy(u->repulse_direction) / 2;
		if (unit_is_at_move_target(u)) {
			repulse *= strength / 4 + 1;
			repulse /= 2;
		}
		size_t index = u->repulse_flags >> 4;
		xy_fp8 adjust = { fp8::from_raw(repulse_adjust_table[index * 2]), fp8::from_raw(repulse_adjust_table[index * 2 + 1]) };
		if (repulse.x > adjust.x * 4) repulse.x -= adjust.x;
		else if (repulse.x > 0_fp8) repulse.x += adjust.x;
		else if (repulse.x < -adjust.x * 4) repulse.x += adjust.x;
		else if (repulse.x < 0_fp8) repulse.x -= adjust.x;
		if (repulse.y > adjust.y * 4) repulse.y -= adjust.y;
		else if (repulse.y > 0_fp8) repulse.y += adjust.y;
		else if (repulse.y < -adjust.y * 4) repulse.y += adjust.y;
		else if (repulse.y < 0_fp8) repulse.y -= adjust.y;
		ems.exact_position += repulse;
		ems.position = to_xy(ems.exact_position);
		return true;
	}

	bool finish_flingy_movement(flingy_t* u, execute_movement_struct& ems) {
		bool moved = u->position != ems.position;
		u->movement_flags = ems.post_movement_flags;
		u->next_speed = u->current_speed;
		u->position = ems.position;
		u->exact_position = ems.exact_position;
		move_sprite(u->sprite, u->position);
		update_unit_heading(u, u->current_velocity_direction);
		if (ems.stopping_movement) {
			if (!s_flag(u->sprite, sprite_t::flag_iscript_nobrk)) {
				sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
			}
		} else if (ems.starting_movement) {
			sprite_run_anim(u->sprite, iscript_anims::Walking);
		}
		return moved;
	}

	bool finish_unit_movement(unit_t* u, execute_movement_struct& ems) {
		auto prev_pos = u->position;
		if (!finish_flingy_movement(u, ems)) return false;
		if (tile_index(prev_pos) != tile_index(u->position)) ems.refresh_vision = true;
		unit_finder_reinsert(u);
		return true;
	}

	bool is_moving_along_path(const unit_t* u) const {
		if (!u->path) return false;
		if (u->path->next == u->sprite->position) return false;
		if (u->path->next == u->move_target.pos) return true;
		if (u->flingy_movement_type != 0) return true;
		int remaining_distance = xy_length(u->path->next - u->sprite->position);
		if (remaining_distance > 16) return true;
		auto remaining_turn = fp8::extend(u->desired_velocity_direction - u->next_velocity_direction).abs();
		if (remaining_turn.raw_value <= remaining_distance * 2) return true;
		if (u->path->current_short_path_index >= u->path->short_path.size() - 1) {
			if (get_region_at(u->sprite->position) != u->path->long_path[u->path->current_long_path_index]) return true;
		}
		return false;
	}

	struct pathfinder {
		const unit_t* u = nullptr;
		const unit_t* target_unit = nullptr;
		xy source;
		const regions_t::region* source_region = nullptr;
		const regions_t::region* destination_region = nullptr;
		rect unit_bb;
		rect target_unit_bb;
		xy destination;

		a_circular_vector<const regions_t::region*> long_path;
		size_t full_long_path_size;
		a_circular_vector<xy> short_path;

		size_t current_long_path_index = 0;
		size_t current_short_path_index = 0;

		size_t short_highest_open_size = 0;
		size_t short_all_nodes_size = 0;
		size_t long_highest_open_size = 0;
		size_t long_all_nodes_size = 0;

		bool destination_reached = false;
		bool is_stuck = false;

		const unit_t* consider_collision_with_unit = nullptr;
		bool consider_collision_with_moving_units = false;
	};

	bool pathfinder_find_long_path(pathfinder& pf) const {
		if (pf.source_region == pf.destination_region) return false;

		struct node_t {
			node_t* prev = nullptr;
			xy_fp8 pos;
			const regions_t::region* region = nullptr;
			fp8 total_cost{};
			fp8 estimated_remaining_cost{};
			fp8 estimated_final_cost{};
			bool visited = false;
		};
		struct cmp_node {
			bool operator()(const node_t* a, const node_t* b) const {
				return a->estimated_final_cost < b->estimated_final_cost;
			}
		};
		a_vector<node_t*> open;

		a_list<node_t> all_nodes;

		node_t* goal_node = nullptr;

		auto region_pos = [&](const regions_t::region* r) {
			if (r == pf.source_region) return to_xy_fp8(pf.source);
			if (r == pf.destination_region) return to_xy_fp8(pf.destination);
			return r->center;
		};

		auto find = [&](const regions_t::region* from_region, const regions_t::region* to_region) {

			xy_fp8 to_pos = region_pos(to_region);

			all_nodes.emplace_back();
			node_t* start_node = &all_nodes.back();
			start_node->pos = region_pos(from_region);
			start_node->region = from_region;
			start_node->estimated_remaining_cost = fp8::integer(128 * 128);
			start_node->estimated_final_cost = start_node->estimated_remaining_cost;
			start_node->region->pathfinder_node = (void*)start_node;

			open.push_back(start_node);
			binary_heap_up(std::prev(open.end()), open.begin(), open.end(), cmp_node());

			while (!open.empty()) {
				node_t* cur = open.front();
				std::swap(open.front(), open.back());
				open.pop_back();
				binary_heap_down(open.begin(), open.begin(), open.end(), cmp_node());

				if (cur->region == to_region) {
					goal_node = cur;
					break;
				}
				cur->visited = true;

				auto add = [&](const regions_t::region* r) {
					if (open.size() == 125) return;
					if (all_nodes.size() == 350) return;
					xy_fp8 pos = region_pos(r);
					fp8 cost = xy_length(pos - cur->pos);
					if (r->group_index != pf.source_region->group_index) {
						cost *= 2;
					}
					fp8 total_cost = cur->total_cost + cost;
					node_t* n = (node_t*)r->pathfinder_node;
					if (!n) {
						all_nodes.emplace_back();
						n = &all_nodes.back();
						n->prev = cur;
						n->pos = pos;
						n->region = r;
						n->total_cost = total_cost;
						n->estimated_remaining_cost = xy_length(to_pos - pos);
						n->estimated_final_cost = n->total_cost + n->estimated_remaining_cost;
						n->visited = false;
						r->pathfinder_node = (void*)n;
						open.push_back(n);
						binary_heap_up(std::prev(open.end()), open.begin(), open.end(), cmp_node());
					} else if (cur->prev != n) {
						if (total_cost < n->total_cost) {
							n->prev = cur;
							n->total_cost = total_cost;
							fp8 estimated_final_cost = n->total_cost + n->estimated_remaining_cost;
							if (n->visited) {
								n->estimated_final_cost = estimated_final_cost;
								n->visited = false;
								open.push_back(n);
								binary_heap_up(std::prev(open.end()), open.begin(), open.end(), cmp_node());
							} else {
								if (estimated_final_cost >= n->estimated_final_cost) error("unreachable; cost did not decrease");
								n->estimated_final_cost = estimated_final_cost;
								binary_heap_up(std::find(open.begin(), open.end(), n), open.begin(), open.end(), cmp_node());
							}
						}
					}
				};
				for (auto* n : cur->region->walkable_neighbors) {
					add(n);
				}
				if (cur->region->group_index != pf.source_region->group_index) {
					for (auto* n : cur->region->non_walkable_neighbors) {
						add(n);
					}
				}
				if (open.size() > pf.long_highest_open_size) {
					pf.long_highest_open_size = open.size();
				}
				if (open.size() == 125) break;
				if (all_nodes.size() == 350) break;

			}
			if (!goal_node && !open.empty()) {
				goal_node = open.front();
			}

			if (!goal_node) {
				start_node->estimated_remaining_cost = xy_length(to_pos - start_node->region->center);
				fp8 best_cost = start_node->estimated_remaining_cost;
				node_t* best_node = start_node;
				for (auto i = std::next(all_nodes.begin()); i != all_nodes.end(); ++i) {
					if (i->estimated_remaining_cost < best_cost) {
						best_cost = i->estimated_remaining_cost;
						best_node = &*i;
					}
				}
				goal_node = best_node;
			}

		};

		bool path_is_reversed;
		if (pf.source_region->group_index == pf.destination_region->group_index) {
			find(pf.source_region, pf.destination_region);
			path_is_reversed = false;
		} else {
			find(pf.destination_region, pf.source_region);
			path_is_reversed = true;
			if (goal_node->region != pf.source_region) {
				for (auto& v : all_nodes) {
					v.region->pathfinder_node = nullptr;
				}
				if (pf.source_region->group_index == goal_node->region->group_index) {
					find(pf.source_region, goal_node->region);
					path_is_reversed = false;
				} else {
					find(goal_node->region, pf.source_region);
				}
			}
		}

		pf.long_all_nodes_size = all_nodes.size();

		pf.long_path.clear();
		pf.full_long_path_size = 0;
		pf.current_long_path_index = (size_t)0 - 1;
		size_t full_path_size = 0;
		if (path_is_reversed) {
			for (auto* n = goal_node; n; n = n->prev) {
				if (n->region->group_index != pf.source_region->group_index) break;
				if (pf.long_path.size() != 50) pf.long_path.push_back(n->region);
				++full_path_size;
			}
		} else {
			for (auto* n = goal_node; n; n = n->prev) {
				++full_path_size;
			}
			for (size_t i = full_path_size; i > 50; --i) {
				goal_node = goal_node->prev;
			}
			for (auto* n = goal_node; n; n = n->prev) {
				pf.long_path.push_front(n->region);
			}
		}
		pf.full_long_path_size = full_path_size;
		for (auto& v : all_nodes) {
			v.region->pathfinder_node = nullptr;
		}
		return !pf.long_path.empty();
	}

	template<typename random_iterator_T, typename compare_T>
	void binary_heap_up(random_iterator_T element, random_iterator_T begin, random_iterator_T end, compare_T compare) const {
		auto index = element - begin;
		while (element != begin) {
			auto parent_index = (index - 1) / 2;
			auto parent = begin + parent_index;
			if (compare(*parent, *element)) break;
			std::swap(*parent, *element);
			index = parent_index;
			element = parent;
		}
	}
	template<typename random_iterator_T, typename compare_T>
	void binary_heap_down(random_iterator_T element, random_iterator_T begin, random_iterator_T end, compare_T compare) const {
		auto index = element - begin;
		auto end_index = end - begin;
		while (true) {
			auto child_index = index * 2 + 1;
			if (child_index >= end_index) break;
			auto child = begin + child_index;
			if (child + 1 != end) {
				if (compare(*(child + 1), *child)) {
					++child_index;
					++child;
				}
			}
			if (!compare(*child, *element)) break;
			std::swap(*element, *child);
			index = child_index;
			element = child;
		}
	}

	bool pathfinder_unit_can_collide_with(const unit_t* u, const unit_t* target, const unit_t* consider_collision_with_unit, bool consider_collision_with_moving_units) const {
		if (target != consider_collision_with_unit && !consider_collision_with_moving_units) {
			if (!unit_is_at_move_target(target)) return false;
		}
		if (unit_is(target, UnitTypes::Special_Upper_Level_Door)) return false;
		if (unit_is(target, UnitTypes::Special_Right_Upper_Level_Door)) return false;
		if (unit_is(target, UnitTypes::Special_Pit_Door)) return false;
		if (unit_is(target, UnitTypes::Special_Right_Pit_Door)) return false;
		return unit_can_collide_with(u, target);
	}

	bool pathfinder_unit_can_collide_with(const pathfinder& pf, const unit_t* target) const {
		return pathfinder_unit_can_collide_with(pf.u, target, pf.consider_collision_with_unit, pf.consider_collision_with_moving_units);
	}

	void pathfinder_find_short_path(pathfinder& pf, xy target, const regions_t::region* target_region) const {
		bool target_is_destination = target == pf.destination;
		bool target_region_walkable = target_region && target_region->walkable();

		const regions_t::region* source_region = pf.source_region;
		const regions_t::region* destination_region = get_region_at(pf.destination);

		const regions_t::region* move_to_region = target_region ? target_region : destination_region;

		for (auto* nr : move_to_region->walkable_neighbors) {
			if (nr == source_region) continue;
			nr->pathfinder_flag = 1;
		}

		struct pf_search {
			const unit_t* u = nullptr;
			const unit_t* target_unit = nullptr;
			std::array<int, 4> inner;
			std::array<int, 4> outer;
			rect target_unit_bb;
			xy target;

			bool has_found_goal;
			xy cur_pos;
			xy cur_pos_max;
			xy cur_pos_min;

			std::array<a_vector<regions_t::contour>, 4> local_edges;

			std::array<const regions_t::contour*, 4> nearest_edge;

			struct neighbor_t {
				xy pos;
				int flags;
				bool is_goal;
			};
			static_vector<neighbor_t, 32> neighbors;

			a_vector<rect> visited_areas;
		};

		pf_search w;

		w.u = pf.u;
		w.target_unit = pf.target_unit;
		w.target = target;
		w.inner[0] = pf.u->unit_type->dimensions.from.y;
		w.outer[0] = pf.u->unit_type->dimensions.from.y + 1;
		w.inner[1] = -pf.u->unit_type->dimensions.to.x;
		w.outer[1] = -pf.u->unit_type->dimensions.to.x - 1;
		w.inner[2] = -pf.u->unit_type->dimensions.to.y;
		w.outer[2] = -pf.u->unit_type->dimensions.to.y - 1;
		w.inner[3] = pf.u->unit_type->dimensions.from.x;
		w.outer[3] = pf.u->unit_type->dimensions.from.x + 1;
		w.target_unit_bb = pf.target_unit_bb;

		struct node_t {
			node_t* prev = nullptr;
			xy pos;
			fp8 total_cost{};
			fp8 estimated_remaining_cost{};
			fp8 estimated_final_cost{};
			bool is_goal = false;
			int depth = 0;
			int directional_flags = 0;
			const regions_t::region* region = nullptr;
			bool is_target_region = false;
			bool is_neighbor_region = false;
			bool visited = false;
		};
		struct cmp_node {
			bool operator()(const node_t* a, const node_t* b) const {
				return a->estimated_final_cost < b->estimated_final_cost;
			}
		};
		static_vector<node_t*, 150> open;

		static_vector<node_t, 250> all_nodes;
		all_nodes.emplace_back();
		node_t* start_node = &all_nodes.back();
		start_node->pos = pf.source;
		start_node->estimated_remaining_cost = fp8::integer(128 * 128);
		start_node->estimated_final_cost = start_node->estimated_remaining_cost;
		start_node->directional_flags = 0xff;
		start_node->region = source_region;

		open.push_back(start_node);
		binary_heap_up(std::prev(open.end()), open.begin(), open.end(), cmp_node());

		struct visited {
			int x;
			a_vector<std::pair<int, int>> y;
		};

		a_vector<visited> pf_area_visited;
		pf_area_visited.push_back({0, {}});
		pf_area_visited.push_back({(int)game_st.map_width, {}});

		auto pf_remove_visited_flags = [&](xy pos, int flags) {
			auto cur = std::upper_bound(pf_area_visited.begin(), pf_area_visited.end(), pos.x, [&](int a, auto& b) {
				return a < b.x;
			});
			--cur;

			auto i = cur->y.begin();
			for (;i != cur->y.end(); ++i) {
				if (pos.y <= i->second) break;
			}
			if (i == cur->y.end()) return flags;
			int mask = 0;
			if (pos.y <= i->first) mask |= 0x19;
			if (pos.y >= i->second) mask |= 0x46;
			if (pos.x == cur->x) {
				mask |= 0x8c;
				if (cur != pf_area_visited.begin()) {
					auto prev = std::prev(cur);
					i = prev->y.begin();
					for (;i != prev->y.end(); ++i) {
						if (pos.y <= i->second) break;
					}
					if (i != prev->y.end()) {
						if (pos.y >= i->first && pos.y <= i->second) {
							mask &= 0x7f;
							if (pos.y > i->first) mask &= 0xf7;
							if (pos.y < i->second) mask &= 0xfb;
						}
					}
				}
			}
			auto next = std::next(cur);
			if (pos.x == next->x - 1) {
				mask |= 0x23;
				i = next->y.begin();
				for (;i != next->y.end(); ++i) {
					if (pos.y <= i->second) break;
				}
				if (i != next->y.end()) {
					if (pos.y >= i->first && pos.y <= i->second) {
						mask &= 0xdf;
						if (pos.y > i->first) mask &= 0xfe;
						if (pos.y < i->second) mask &= 0xfd;
					}
				}
			}
			return flags & mask;
		};

		auto merge_local_edge = [&](auto i, const auto& c) {
			if (c.v[1] + w.inner[c.flags & 3] <= i->v[2] + w.inner[(i->flags >> 2) & 3]) {
				if (c.v[2] + w.inner[(c.flags >> 2) & 3] >= i->v[1] + w.inner[i->flags & 3] - 1) {
					if (c.v[1] < i->v[1]) i->v[1] = c.v[1];
					if (c.v[2] > i->v[2]) i->v[2] = c.v[2];
					return true;
				}
			}
			return false;
		};

		auto pf_add_local_edge = [&](int n, const regions_t::contour& c) {
			auto& local_edges = w.local_edges[n];
			auto cmp_l = [&](auto& a, int b) {
				return a.v[0] < b;
			};
			auto i = std::lower_bound(local_edges.begin(), local_edges.end(), c.v[0], cmp_l);
			while (i != local_edges.end() && i->v[0] == c.v[0]) {
				if (merge_local_edge(i, c)) return;
				if (c.v[1] + w.inner[c.flags & 3] <= i->v[2] + w.inner[(i->flags >> 2) & 3]) break;
				++i;
			}
			local_edges.insert(i, c);
		};

		auto pf_push_back_local_edge = [&](int n, const regions_t::contour& c) {
			auto& local_edges = w.local_edges[n];
			if (!local_edges.empty() && local_edges.back().v[0] == c.v[0]) {
				if (merge_local_edge(std::prev(local_edges.end()), c)) return;
				auto i = std::prev(local_edges.end());
				if (c.v[1] + w.inner[c.flags & 3] <= i->v[2] + w.inner[(i->flags >> 2) & 3]) local_edges.insert(i, c);
				else local_edges.push_back(c);
			} else local_edges.push_back(c);
		};

		auto pf_add_local_terrain = [&]() {
			auto cmp_l = [&](const regions_t::contour& c, int v) {
				return c.v[0] < v;
			};
			auto cmp_u = [&](int v, const regions_t::contour& c) {
				return v < c.v[0];
			};
			auto& c0 = game_st.regions.contours[0];
			for (auto i = std::lower_bound(c0.begin(), c0.end(), w.cur_pos_min.y - w.inner[0] - 1, cmp_l); i != c0.end(); ++i) {
				if (i->v[0] > w.cur_pos.y - w.inner[0] - 1) break;
				if (i->v[1] + w.inner[i->flags & 3] <= w.cur_pos_max.x) {
					if (i->v[2] + w.inner[(i->flags >> 2) & 3] >= w.cur_pos_min.x) {
						pf_push_back_local_edge(0, *i);
					}
				}
			}
			auto& c1 = game_st.regions.contours[1];
			for (auto i = std::upper_bound(c1.begin(), c1.end(), w.cur_pos.x - w.inner[1], cmp_u); i != c1.end(); ++i) {
				if (i->v[0] > w.cur_pos_max.x - w.inner[1] + 1) break;
				if (i->v[1] + w.inner[i->flags & 3] <= w.cur_pos_max.y) {
					if (i->v[2] + w.inner[(i->flags >> 2) & 3] >= w.cur_pos_min.y) {
						pf_push_back_local_edge(1, *i);
					}
				}
			}
			auto& c2 = game_st.regions.contours[2];
			for (auto i = std::upper_bound(c2.begin(), c2.end(), w.cur_pos.y - w.inner[2], cmp_u); i != c2.end(); ++i) {
				if (i->v[0] > w.cur_pos_max.y - w.inner[2] + 1) break;
				if (i->v[1] + w.inner[i->flags & 3] <= w.cur_pos_max.x) {
					if (i->v[2] + w.inner[(i->flags >> 2) & 3] >= w.cur_pos_min.x) {
						pf_push_back_local_edge(2, *i);
					}
				}
			}
			auto& c3 = game_st.regions.contours[3];
			for (auto i = std::lower_bound(c3.begin(), c3.end(), w.cur_pos_min.x - w.inner[3] - 1, cmp_l); i != c3.end(); ++i) {
				if (i->v[0] > w.cur_pos.x - w.inner[3] - 1) break;
				if (i->v[1] + w.inner[i->flags & 3] <= w.cur_pos_max.y) {
					if (i->v[2] + w.inner[(i->flags >> 2) & 3] >= w.cur_pos_min.y) {
						pf_push_back_local_edge(3, *i);
					}
				}
			}
		};

		auto pf_add_local_units = [&]() {
			auto cmp_l = [&](auto& a, int b) {
				return a.value < b;
			};
			for (auto i = std::lower_bound(st.unit_finder_y.begin(), st.unit_finder_y.end(), w.cur_pos_min.y - w.inner[0] - 1, cmp_l); i != st.unit_finder_y.end(); ++i) {
				auto& bb = i->u->unit_finder_bounding_box;
				if (i->value >= w.cur_pos.y - w.inner[0]) break;
				if (i->value == bb.to.y) {
					regions_t::contour c;
					c.v[0] = bb.to.y;
					c.v[1] = bb.from.x;
					c.v[2] = bb.to.x;
					c.dir = 0;
					c.flags = 0x3d;
					if (c.v[1] + w.inner[1] <= w.cur_pos_max.x && c.v[2] + w.inner[3] >= w.cur_pos_min.x) {
						if (i->u == w.target_unit || pathfinder_unit_can_collide_with(pf, i->u)) {
							pf_add_local_edge(0, c);
						}
					}
				}
			}
			for (auto i = std::lower_bound(st.unit_finder_x.begin(), st.unit_finder_x.end(), w.cur_pos.x - w.inner[1], cmp_l); i != st.unit_finder_x.end(); ++i) {
				auto& bb = i->u->unit_finder_bounding_box;
				if (i->value > w.cur_pos_max.x - w.inner[1] + 1) break;
				if (i->value == bb.from.x) {
					regions_t::contour c;
					c.v[0] = bb.from.x;
					c.v[1] = bb.from.y;
					c.v[2] = bb.to.y;
					c.dir = 1;
					c.flags = 0x32;
					if (c.v[1] + w.inner[2] <= w.cur_pos_max.y && c.v[2] + w.inner[0] >= w.cur_pos_min.y) {
						if (i->u == w.target_unit || pathfinder_unit_can_collide_with(pf, i->u)) {
							pf_add_local_edge(1, c);
						}
					}
				}
			}
			for (auto i = std::lower_bound(st.unit_finder_y.begin(), st.unit_finder_y.end(), w.cur_pos.y - w.inner[2], cmp_l); i != st.unit_finder_y.end(); ++i) {
				auto& bb = i->u->unit_finder_bounding_box;
				if (i->value > w.cur_pos_max.y - w.inner[2] + 1) break;
				if (i->value == bb.from.y) {
					regions_t::contour c;
					c.v[0] = bb.from.y;
					c.v[1] = bb.from.x;
					c.v[2] = bb.to.x;
					c.dir = 2;
					c.flags = 0x3d;
					if (c.v[1] + w.inner[1] <= w.cur_pos_max.x && c.v[2] + w.inner[3] >= w.cur_pos_min.x) {
						if (i->u == w.target_unit || pathfinder_unit_can_collide_with(pf, i->u)) {
							pf_add_local_edge(2, c);
						}
					}
				}
			}
			for (auto i = std::lower_bound(st.unit_finder_x.begin(), st.unit_finder_x.end(), w.cur_pos_min.x - w.inner[3] - 1, cmp_l); i != st.unit_finder_x.end(); ++i) {
				auto& bb = i->u->unit_finder_bounding_box;
				if (i->value >= w.cur_pos.x - w.inner[3]) break;
				if (i->value == bb.to.x) {
					regions_t::contour c;
					c.v[0] = bb.to.x;
					c.v[1] = bb.from.y;
					c.v[2] = bb.to.y;
					c.dir = 3;
					c.flags = 0x32;
					if (c.v[1] + w.inner[2] <= w.cur_pos_max.y && c.v[2] + w.inner[0] >= w.cur_pos_min.y) {
						if (i->u == w.target_unit || pathfinder_unit_can_collide_with(pf, i->u)) {
							pf_add_local_edge(3, c);
						}
					}
				}
			}
		};

		auto pf_local_edges_find = [&](int dir, int v, int v0, int v1, int v2) -> const regions_t::contour* {

			auto cmp_u = [&](int v, const regions_t::contour& c) {
				return v < c.v[0];
			};
			auto cmp_l = [&](const regions_t::contour& c, int v) {
				return c.v[0] < v;
			};

			if (dir == 0) {
				auto& c0 = w.local_edges[0];
				for (auto i = std::upper_bound(c0.begin(), c0.end(), v - w.inner[0], cmp_u); i != c0.begin();) {
					--i;
					if (i->v[0] + w.inner[0] < v2) break;
					if (std::max(i->v[1] + w.inner[i->flags & 3], v0) <= std::min(i->v[2] + w.inner[(i->flags >> 2) & 3], v1)) return &*i;
				}
			} else if (dir == 1) {
				auto& c1 = w.local_edges[1];
				for (auto i = std::lower_bound(c1.begin(), c1.end(), v - w.inner[1], cmp_l); i != c1.end(); ++i) {
					if (i->v[0] + w.inner[1] > v2) break;
					if (std::max(i->v[1] + w.inner[i->flags & 3], v0) <= std::min(i->v[2] + w.inner[(i->flags >> 2) & 3], v1)) return &*i;
				}
			} else if (dir == 2) {
				auto& c2 = w.local_edges[2];
				for (auto i = std::lower_bound(c2.begin(), c2.end(), v - w.inner[2], cmp_l); i != c2.end(); ++i) {
					if (i->v[0] + w.inner[2] > v2) break;
					if (std::max(i->v[1] + w.inner[i->flags & 3], v0) <= std::min(i->v[2] + w.inner[(i->flags >> 2) & 3], v1)) return &*i;
				}
			} else if (dir == 3) {
				auto& c3 = w.local_edges[3];
				for (auto i = std::upper_bound(c3.begin(), c3.end(), v - w.inner[3], cmp_u); i != c3.begin();) {
					--i;
					if (i->v[0] + w.inner[3] < v2) break;
					if (std::max(i->v[1] + w.inner[i->flags & 3], v0) <= std::min(i->v[2] + w.inner[(i->flags >> 2) & 3], v1)) return &*i;
				}
			}
			return nullptr;
		};

		auto pf_add_neighbor = [&](xy pos, int flags) {
			if (pos == w.cur_pos) return;
			bool is_goal = false;
			if (w.target_unit) {
				if (pos.x == w.target_unit_bb.from.x || pos.x == w.target_unit_bb.to.x) {
					if (pos.y >= w.target_unit_bb.from.y && pos.y <= w.target_unit_bb.to.y) {
						is_goal = true;
					}
				}
				if (pos.y == w.target_unit_bb.from.y || pos.y == w.target_unit_bb.to.y) {
					if (pos.x >= w.target_unit_bb.from.x && pos.x <= w.target_unit_bb.to.x) {
						is_goal = true;
					}
				}
			}
			if (pos == target) is_goal = true;
			if (is_goal) {
				flags = 0xff;
				w.has_found_goal = true;
			}
			if (w.neighbors.size() < 32) {
				if (is_goal || w.neighbors.empty()) {
					w.neighbors.push_back({pos, flags, is_goal});
				} else {
					auto i = std::find_if(w.neighbors.begin(), w.neighbors.end(), [&](auto& v) {
						return v.pos == pos;
					});
					if (i != w.neighbors.end()) i->flags &= flags;
					else w.neighbors.push_back({pos, flags, is_goal});
				}
			}
		};

		auto visit_area = [&](int from_x, int from_y, int to_x, int to_y) {
			if (to_x < from_x) std::swap(from_x, to_x);
			if (to_y < from_y) std::swap(from_y, to_y);
			if (w.target.x >= from_x && w.target.x <= to_x) {
				if (w.target.y >= from_y && w.target.y <= to_y) {
					w.neighbors.push_back({w.target, 0xff, true});
					w.has_found_goal = true;
				}
			}
			w.visited_areas.push_back({xy{from_x, from_y}, xy{to_x, to_y}});
		};

		auto add_neighbors = [&](int n) {
			regions_t::contour c{};
			xy pos = w.cur_pos;
			xy pos_max = w.cur_pos_max;
			xy pos_min = w.cur_pos_min;
			auto expand_inner = [&](regions_t::contour c) {
				c.v[0] += w.inner[c.dir];
				c.v[1] += w.inner[c.flags & 3];
				c.v[2] += w.inner[(c.flags >> 2) & 3];
				return c;
			};
			auto expand_outer = [&](regions_t::contour c) {
				c.v[0] += w.outer[c.dir];
				c.v[1] += w.outer[c.flags & 3];
				c.v[2] += w.outer[(c.flags >> 2) & 3];
				return c;
			};
			if (n == 0) {
				if (pos.x < pos_max.x) {
					int xval = pos_max.x;
					int yval = pos_min.y;
					int edge_y = pos_min.y;
					if (w.nearest_edge[1]) {
						auto* i = w.nearest_edge[1];
						edge_y = i->v[1] + w.inner[i->flags & 3] - 1;
						if (edge_y > yval) yval = edge_y;
					}
					int flags = 0x3b;
					auto* i = pf_local_edges_find(0, pos.y - 1, pos.x, xval, yval - 1);
					if (i) {
						c = expand_outer(*i);
						yval = c.v[0];
						if (xval == c.v[2]) {
							if (c.flags & 0x20) flags = 0x33;
							else flags = 0;
						} else if (xval < c.v[2]) {
							flags = 0x22;
						}
					}
					if (w.nearest_edge[1]) {
						flags &= 0xfd;
						if (edge_y < yval) flags &= 0xde;
					}
					if (yval >= pos.y) {
						flags |= 0x44;
					} else {
						pf_add_neighbor({xval, pos.y}, 0xe7);
					}
					pf_add_neighbor({xval, yval}, flags);
					if (yval == pos_min.y) pf_add_neighbor({pos.x, yval}, 0xdd);
					if (i) {
						if (xval > c.v[2]) pf_add_neighbor({c.v[2], c.v[0]}, 0x11);
						while (true) {
							if (c.v[1] < pos.x) {
								pos_max.x = pos.x;
								break;
							}
							pos_max.x = c.v[1];
							pf_add_neighbor({c.v[1], c.v[0]}, 0x18);
							if (i == &w.local_edges[0].front()) break;
							--i;
							c = expand_outer(*i);
							if (c.v[0] != yval) break;
							if (c.v[2] < pos.x) break;
							if (c.v[2] <= xval) pf_add_neighbor({c.v[2], c.v[0]}, 0x11);
						}
					}
					visit_area(pos.x, pos.y, xval, yval);
					pos.y = yval;
				}
				if (pos.y > pos_min.y) {
					int xval = pos_max.x;
					int yval = pos_min.y;
					int edge_x = pos_min.y;
					if (w.nearest_edge[0]) {
						auto* i = w.nearest_edge[0];
						edge_x = i->v[2] + w.inner[(i->flags >> 2) & 3] + 1;
						if (edge_x < xval) xval = edge_x;
					}
					int flags = 0x3b;
					auto* i = pf_local_edges_find(1, pos.x + 1, yval, pos.y, xval + 1);
					if (i) {
						c = expand_outer(*i);
						xval = c.v[0];
						if (yval == c.v[1]) {
							if (c.flags & 0x10) flags = 0x39;
							else flags = 0;
						} else if (yval > c.v[1]) {
							flags = 0x18;
						}
					}
					if (w.nearest_edge[0]) {
						flags &= 0xf7;
						if (edge_x > xval) flags &= 0xde;
					}
					if (xval <= pos.x) {
						flags |= 0x84;
					} else {
						pf_add_neighbor({pos.x, yval}, 0xdd);
					}
					pf_add_neighbor({xval, yval}, flags);
					if (xval < pos_max.x) pf_add_neighbor({xval, pos.y}, 1);
					if (i) {
						if (yval < c.v[1]) pf_add_neighbor({c.v[0], c.v[1]}, 0x21);
						while (c.v[2] <= pos.y) {
							pf_add_neighbor({c.v[0], c.v[2]}, 0x22);
							if (i == &w.local_edges[1].back()) break;
							++i;
							c = expand_outer(*i);
							if (c.v[0] != xval || c.v[1] > pos.y) {
								pf_add_neighbor({xval, pos.y}, 0x21);
								break;
							}
							if (c.v[1] >= yval) pf_add_neighbor({c.v[0], c.v[1]}, 0x21);
						}
					}
					visit_area(pos.x, pos.y, xval, yval);
				}
				return false;
			} else if (n == 1) {
				if (pos.x < pos_max.x) {
					int xval = pos_max.x;
					int yval = pos_max.y;
					int edge_y = pos_max.y;
					if (w.nearest_edge[1]) {
						auto* i = w.nearest_edge[1];
						edge_y = i->v[2] + w.inner[(i->flags >> 2) & 3] + 1;
						if (edge_y < yval) yval = edge_y;
					}
					int flags = 0x67;
					auto* i = pf_local_edges_find(2, pos.y + 1, pos.x, xval, yval + 1);
					if (i) {
						c = expand_outer(*i);
						if (pos.x < c.v[1]) pf_add_neighbor({c.v[1], c.v[0]}, 0x44);
						yval = c.v[0];
						pos_max.x = c.v[1];
						while (c.v[2] < xval) {
							pf_add_neighbor({c.v[2], c.v[0]}, 0x42);
							if (i == &w.local_edges[2].back()) break;
							++i;
							auto ic = expand_inner(*i);
							if (ic.v[0] - 1 != yval) break;
							if (ic.v[1] - 1 >= xval) break;
							c = expand_outer(*i);
							if (c.v[1] <= xval) pf_add_neighbor({c.v[1], c.v[0]}, 0x44);
						}
						if (xval == c.v[2]) {
							if (c.flags & 0x20) flags = 0x63;
							else flags = 0;
						} else if (xval < c.v[2]) {
							flags = 0x21;
						}
					}
					if (w.nearest_edge[1]) {
						flags &= 0xfe;
						if (edge_y > yval) flags &= 0xdd;
					}
					if (yval <= pos.y) {
						flags |= 0x18;
					} else {
						pf_add_neighbor({xval, pos.y}, 0xbb);
					}
					pf_add_neighbor({xval, yval}, flags);
					if (yval == pos_max.y) pf_add_neighbor({pos.x, yval}, 0xde);
					visit_area(pos.x, pos.y, xval, yval);
					pos.y = yval;
				}
				if (pos.y < pos_max.y) {
					int xval = pos_max.x;
					int yval = pos_max.y;
					int edge_x = pos_max.x;
					if (w.nearest_edge[2]) {
						auto* i = w.nearest_edge[2];
						edge_x = i->v[2] + w.inner[(i->flags >> 2) & 3] + 1;
						if (edge_x < xval) xval = edge_x;
					}
					int flags = 0x67;
					auto* i = pf_local_edges_find(1, pos.x + 1, pos.y, yval, xval + 1);
					if (i) {
						c = expand_outer(*i);
						if (c.v[0] < pos_max.x) pf_add_neighbor({c.v[0], pos.y}, 2);
						if (c.v[1] > pos.y) pf_add_neighbor({c.v[0], c.v[1]}, 0x21);
						xval = c.v[0];
						while (c.v[2] < yval) {
							pf_add_neighbor({c.v[0], c.v[2]}, 0x22);
							if (i == &w.local_edges[1].back()) break;
							++i;
							auto ic = expand_inner(*i);
							if (ic.v[0] - 1 != xval) break;
							if (ic.v[1] - 1 >= yval) break;
							c = expand_outer(*i);
							if (pos.y <= c.v[1] && c.v[1] <= pos_max.y) pf_add_neighbor({c.v[0], c.v[1]}, 0x21);
						}
						if (yval == c.v[2]) {
							if (c.flags & 0x20) flags = 0x66;
							else flags = 0;
						} else if (yval < c.v[2]) {
							flags = 0x44;
						}
					}
					if (w.nearest_edge[2]) {
						flags &= 0xfb;
						if (edge_x > xval) flags &= 0xbd;
					}
					if (xval <= pos.x) {
						flags |= 0x88;
					} else {
						pf_add_neighbor({pos.x, yval}, 0xde);
					}
					pf_add_neighbor({xval, yval}, flags);
					visit_area(pos.x, pos.y, xval, yval);
				}
				return false;
			} else if (n == 2) {
				if (pos.x > pos_min.x) {
					int xval = pos_min.x;
					int yval = pos_max.y;
					int edge_y = pos_max.y;
					if (w.nearest_edge[3]) {
						auto* i = w.nearest_edge[3];
						edge_y = i->v[2] + w.inner[(i->flags >> 2) & 3] + 1;
						if (edge_y < yval) yval = edge_y;
					}
					int flags = 0xce;
					auto* i = pf_local_edges_find(2, pos.y + 1, xval, pos.x, yval + 1);
					if (i) {
						c = expand_outer(*i);
						yval = c.v[0];
						if (xval == c.v[1]) {
							if (c.flags & 0x10) flags = 0xcc;
							else flags = 0;
						} else if (xval > c.v[1]) {
							flags = 0x88;
						}
					}
					if (w.nearest_edge[3]) {
						flags &= 0xf7;
						if (edge_y > yval) flags &= 0x7b;
					}
					if (yval <= pos.y) {
						flags |= 0x11;
					} else {
						pf_add_neighbor({xval, pos.y}, 0xbd);
					}
					pf_add_neighbor({xval, yval}, flags);
					if (yval == pos_max.y) pf_add_neighbor({pos.x, yval}, 0x77);
					if (i) {
						if (xval < c.v[1]) pf_add_neighbor({c.v[1], c.v[0]}, 0x44);
						while (true) {
							if (c.v[2] > pos.x) {
								pf_add_neighbor({pos.x, c.v[0]}, 0x21);
								pos_min.x = pos.x;
								break;
							}
							pos_min.x = c.v[2];
							pf_add_neighbor({c.v[2], c.v[0]}, 0x42);
							if (i == &w.local_edges[2].back()) break;
							++i;
							c = expand_outer(*i);
							if (c.v[0] != yval) break;
							if (c.v[1] > pos.x) break;
							pf_add_neighbor({c.v[1], c.v[0]}, 0x44);
						}
					}
					visit_area(pos.x, pos.y, xval, yval);
					pos.y = yval;
				}
				if (pos.y < pos_max.y) {
					int xval = pos_min.x;
					int yval = pos_max.y;
					int edge_x = pos_min.y;
					if (w.nearest_edge[2]) {
						auto* i = w.nearest_edge[2];
						edge_x = i->v[1] + w.inner[i->flags & 3] - 1;
						if (edge_x > xval) xval = edge_x;
					}
					int flags = 0xce;
					auto* i = pf_local_edges_find(3, pos.x - 1, pos.y, yval, xval - 1);
					if (i) {
						c = expand_outer(*i);
						xval = c.v[0];
						if (yval == c.v[2]) {
							if (c.flags & 0x20) flags = 0xc6;
							else flags = 0;
						} else if (yval < c.v[2]) {
							flags = 0x42;
						}
					}
					if (w.nearest_edge[2]) {
						flags &= 0xfd;
						if (edge_x < xval) flags &= 0xbb;
					}
					if (xval >= pos.x) {
						flags |= 0x22;
					} else {
						pf_add_neighbor({pos.x, yval}, 0x77);
					}
					pf_add_neighbor({xval, yval}, flags);
					if (xval > pos_min.x) pf_add_neighbor({xval, pos.y}, 4);
					if (i) {
						if (yval > c.v[2]) pf_add_neighbor({c.v[0], c.v[2]}, 0x84);
						while (c.v[1] >= pos.y) {
							pf_add_neighbor({c.v[0], c.v[1]}, 0x88);
							if (i == &w.local_edges[3].front()) break;
							--i;
							c = expand_outer(*i);
							if (c.v[0] != xval || c.v[2] < pos.y) {
								pf_add_neighbor({xval, pos.y}, 0x84);
								break;
							}
							if (c.v[2] <= yval) pf_add_neighbor({c.v[0], c.v[2]}, 0x84);
						}
					}
					visit_area(pos.x, pos.y, xval, yval);
				}
				return false;
			} else {
				if (pos.x > pos_min.x) {
					int xval = pos_min.x;
					int yval = pos_min.y;
					int edge_y = pos_min.y;
					if (w.nearest_edge[3]) {
						auto* i = w.nearest_edge[3];
						edge_y = i->v[1] + w.inner[i->flags & 3] - 1;
						if (edge_y > yval) yval = edge_y;
					}
					int flags = 0x9d;
					auto* i = pf_local_edges_find(0, pos.y - 1, xval, pos.x, yval - 1);
					if (i) {
						c = expand_outer(*i);
						if (pos.x <= c.v[2]) {
							pf_add_neighbor({pos.x, c.v[0]}, 0x22);
						} else {
							pf_add_neighbor({c.v[2], c.v[0]}, 0x11);
						}
						yval = c.v[0];
						pos_min.x = c.v[2];
						while (c.v[1] > xval) {
							pf_add_neighbor({c.v[1], c.v[0]}, 0x18);
							if (i == &w.local_edges[0].front()) break;
							--i;
							auto ic = expand_inner(*i);
							if (ic.v[0] + 1 != yval) break;
							if (ic.v[2] + 1 <= xval) break;
							c = expand_outer(*i);
							if (c.v[2] >= xval) pf_add_neighbor({c.v[2], c.v[0]}, 0x11);
						}
						if (xval == c.v[1]) {
							if (c.flags & 0x10) flags = 0x9c;
							else flags = 0;
						} else if (xval > c.v[1]) {
							flags = 0x84;
						}
					}
					if (w.nearest_edge[3]) {
						flags &= 0xfb;
						if (edge_y < yval) flags &= 0x77;
					}
					if (yval >= pos.y) {
						flags |= 0x44;
					} else {
						pf_add_neighbor({xval, pos.y}, 0xee);
					}
					pf_add_neighbor({xval, yval}, flags);
					if (yval == pos_min.y) pf_add_neighbor({pos.x, yval}, 0x7b);
					visit_area(pos.x, pos.y, xval, yval);
					pos.y = yval;
				}
				if (pos.y > pos_min.y) {
					int xval = pos_min.x;
					int yval = pos_min.y;
					int edge_x = pos_min.x;
					if (w.nearest_edge[0]) {
						auto* i = w.nearest_edge[0];
						edge_x = i->v[1] + w.inner[i->flags & 3] - 1;
						if (edge_x > xval) xval = edge_x;
					}
					int flags = 0x9d;
					auto* i = pf_local_edges_find(3, pos.x - 1, yval, pos.y, xval - 1);
					if (i) {
						c = expand_outer(*i);
						if (c.v[0] > pos_min.x) pf_add_neighbor({c.v[0], pos.y}, 8);
						if (c.v[2] < pos.y) pf_add_neighbor({c.v[0], c.v[2]}, 0x84);
						xval = c.v[0];
						while (c.v[1] > yval) {
							pf_add_neighbor({c.v[0], c.v[1]}, 0x88);
							if (i == &w.local_edges[3].front()) break;
							--i;
							auto ic = expand_inner(*i);
							if (ic.v[0] + 1 != xval) break;
							if (ic.v[2] + 1 <= yval) break;
							c = expand_outer(*i);
							if (c.v[2] >= yval) pf_add_neighbor({c.v[0], c.v[2]}, 0x84);
						}
						if (yval == c.v[1]) {
							if (c.flags & 0x10) flags = 0x99;
							else flags = 0;
						} else if (yval > c.v[1]) {
							flags = 0x11;
						}
					}
					if (w.nearest_edge[0]) {
						flags &= 0xfe;
						if (edge_x < xval) flags &= 0xe7;
					}
					if (xval >= pos.x) {
						flags |= 0x21;
					} else {
						pf_add_neighbor({pos.x, yval}, 0x7b);
					}
					pf_add_neighbor({xval, yval}, flags);
					visit_area(pos.x, pos.y, xval, yval);
				}
				return false;
			}
		};

		auto pf_mark_visited = [&](rect area) {
			auto next = std::upper_bound(pf_area_visited.begin(), pf_area_visited.end(), area.from.x, [&](int a, auto& b) {
				return a < b.x;
			});
			auto cur = std::prev(next);
			if (area.from.x > cur->x) {
				if (pf_area_visited.size() == 128 + 1) return;
				cur = pf_area_visited.insert(next, {area.from.x, cur->y});
				next = std::next(cur);
			}
			if (area.from.x > area.to.x) return;
			do {
				if (next->x > area.to.x + 1) {
					if (pf_area_visited.size() == 128 + 1) return;
					next = pf_area_visited.insert(next, {area.to.x + 1, cur->y});
					cur = std::prev(next);
				}
				auto i = cur->y.begin();
				for (;i != cur->y.end(); ++i) {
					if (area.from.y <= i->second + 1) break;
				}
				if (i == cur->y.end()) {
					if (cur->y.size() == 10) return;
					cur->y.insert(i, {area.from.y, area.to.y});
				} else {
					if (area.to.y < i->first - 1 || area.from.y > i->second + 1)  {
						if (cur->y.size() == 10) return;
						cur->y.insert(i, {area.from.y, area.to.y});
					} else {
						if (i->first > area.from.y) i->first = area.from.y;
						if (i->second < area.to.y) i->second = area.to.y;
						auto t = cur->y.begin();
						for (auto n = std::next(t); n != cur->y.end(); n = std::next(t)) {
							if (n->first - 1 <= t->second) {
								if (n->second > t->second) t->second = n->second;
								t = std::prev(cur->y.erase(n));
							} else t = n;
						}
					}
				}
				++cur;
				++next;
			} while (cur->x <= area.to.x);
		};

		auto pf_generate_neighbors  = [&](xy pos, int flags) {
			w.has_found_goal = false;
			w.neighbors.clear();
			w.visited_areas.clear();

			w.cur_pos = pos;
			w.cur_pos_max = pos + xy(64, 64);
			w.cur_pos_min = pos - xy(64, 64);
			if (pos.x < 64) w.cur_pos_min.x = 0;
			if (pos.y < 64) w.cur_pos_min.y = 0;
			if ((size_t)pos.x + 64 >= game_st.map_width) w.cur_pos_max.x = (int)game_st.map_width - 1;
			if ((size_t)pos.y + 64 >= game_st.map_height) w.cur_pos_max.y = (int)game_st.map_height - 1;
			if ((flags & (0x10 | (8|1))) == 0) w.cur_pos_min.y = pos.y;
			if ((flags & (0x20 | (1|2))) == 0) w.cur_pos_max.x = pos.x;
			if ((flags & (0x40 | (2|4))) == 0) w.cur_pos_max.y = pos.y;
			if ((flags & (0x80 | (4|8))) == 0) w.cur_pos_min.x = pos.x;

			for (auto& v : w.local_edges) v.clear();

			pf_add_local_terrain();
			pf_add_local_units();

			w.nearest_edge[0] = pf_local_edges_find(0, pos.y, pos.x, pos.x, w.cur_pos_min.y - 1);
			if (w.nearest_edge[0]) w.cur_pos_min.y = w.nearest_edge[0]->v[0] + w.inner[0] + 1;
			w.nearest_edge[1] = pf_local_edges_find(1, pos.x, pos.y, pos.y, w.cur_pos_max.x + 1);
			if (w.nearest_edge[1]) w.cur_pos_max.x = w.nearest_edge[1]->v[0] + w.inner[1] - 1;
			w.nearest_edge[2] = pf_local_edges_find(2, pos.y, pos.x, pos.x, w.cur_pos_max.y + 1);
			if (w.nearest_edge[2]) w.cur_pos_max.y = w.nearest_edge[2]->v[0] + w.inner[2] - 1;
			w.nearest_edge[3] = pf_local_edges_find(3, pos.x, pos.y, pos.y, w.cur_pos_min.x - 1);
			if (w.nearest_edge[3]) w.cur_pos_min.x = w.nearest_edge[3]->v[0] + w.inner[3] + 1;

			if (w.cur_pos_min.y == pos.y && w.cur_pos_max.x == pos.x) flags &= ~1;
			if (w.cur_pos_max.y == pos.y) {
				if (w.cur_pos_max.x == pos.x) flags &= ~2;
				if (w.cur_pos_min.x == pos.x) flags &= ~4;
			}
			if (w.cur_pos_min.y == pos.y) {
				if (w.cur_pos_min.x == pos.x) flags &= ~8;
				if (w.cur_pos_min.y == pos.y) flags &= ~0x10;
			}
			if (w.cur_pos_max.x == pos.x) flags &= ~0x20;
			if (w.cur_pos_max.y == pos.y) flags &= ~0x40;
			if (w.cur_pos_min.x == pos.x) flags &= ~0x80;

			if (flags & 0x10) {
				flags |= 9;
				pf_add_neighbor({pos.x, w.cur_pos_min.y}, w.nearest_edge[0] ? 0xa6 : 0xbf);
			}
			if (flags & 0x20) {
				flags |= 3;
				pf_add_neighbor({w.cur_pos_max.x, pos.y}, w.nearest_edge[1] ? 0x5c : 0x7f);
			}
			if (flags & 0x40) {
				flags |= 6;
				pf_add_neighbor({pos.x, w.cur_pos_max.y}, w.nearest_edge[2] ? 0xa9 : 0xef);
			}
			if (flags & 0x80) {
				flags |= 0xc;
				pf_add_neighbor({w.cur_pos_min.x, pos.y}, w.nearest_edge[3] ? 0x53 : 0xdf);
			}

			for (int dir = 0; dir != 4; ++dir) {
				if (flags & (1 << dir)) {
					if (add_neighbors(dir)) break;
				}
			}

			for (auto& v : w.visited_areas) {
				pf_mark_visited(v);
			}

		};

		bool has_found_goal = false;
		bool is_tired = false;
		int n_open_nodes_in_neighbor_region = 0;
		int n_open_nodes_in_target_region = 0;

		node_t* goal_node = nullptr;
		while (!open.empty()) {

			node_t* cur = open.front();
			std::swap(open.front(), open.back());
			open.pop_back();
			binary_heap_down(open.begin(), open.begin(), open.end(), cmp_node());
			if (cur->is_goal) {
				goal_node = cur;
				pf.destination_reached = true;
				break;
			}

			if (cur->directional_flags != 0) {
				cur->directional_flags = pf_remove_visited_flags(cur->pos, cur->directional_flags);
			}

			bool is_exhausted;
			if (n_open_nodes_in_target_region <= 0 || has_found_goal || (target_is_destination && all_nodes.size() < 150)) {
				is_exhausted = is_tired;
			} else {
				is_exhausted = true;
				is_tired = true;
			}
			if (n_open_nodes_in_neighbor_region && !has_found_goal) {
				if (all_nodes.size() >= 200) {
					is_exhausted = true;
					is_tired = true;
				}
			}
			if (cur->is_target_region && is_exhausted) {
				goal_node = cur;
				break;
			}
			cur->visited = true;
			if (cur->is_target_region) --n_open_nodes_in_target_region;
			if (cur->is_neighbor_region) {
				--n_open_nodes_in_neighbor_region;
				if (is_exhausted) break;
			}
			if (has_found_goal || is_exhausted || cur->directional_flags == 0) {
				continue;
			}
			pf_generate_neighbors(cur->pos, cur->directional_flags);

			bool found_goal = false;
			for (auto& v : w.neighbors) {
				if (v.is_goal) {
					found_goal = true;
				} else {
					if (v.flags) {
						v.flags = pf_remove_visited_flags(v.pos, v.flags);
						if (!v.flags) continue;
					}
				}
				auto* n_region = get_region_at(v.pos);
				fp8 cost = xy_length(to_xy_fp8(v.pos) - to_xy_fp8(cur->pos));
				if (target_region_walkable && n_region != target_region && n_region != source_region) {
					cost *= 2;
				}
				fp8 total_cost = cur->total_cost + cost;
				node_t* n = nullptr;
				for (auto i = std::next(all_nodes.begin()); i != all_nodes.end(); ++i) {
					if (i->pos == v.pos) {
						n = &*i;
						break;
					}
				}
				if (!n) {
					all_nodes.emplace_back();
					n = &all_nodes.back();
					n->prev = cur;
					n->pos = v.pos;
					n->region = n_region;
					n->depth = cur->depth + 1;
					n->directional_flags = v.flags;
					n->total_cost = total_cost;
					n->estimated_remaining_cost = xy_length(to_xy_fp8(target) - to_xy_fp8(n->pos));
					n->estimated_final_cost = n->total_cost + n->estimated_remaining_cost;
					n->visited = n->directional_flags == 0 && !n->is_goal;
					n->is_target_region = n->region == target_region;
					n->is_neighbor_region = n->region->pathfinder_flag != 0;
					n->is_goal = v.is_goal;
					if (!n->visited) {
						open.push_back(n);
						binary_heap_up(std::prev(open.end()), open.begin(), open.end(), cmp_node());
						if (n->is_target_region) ++n_open_nodes_in_target_region;
						if (n->is_neighbor_region) ++n_open_nodes_in_neighbor_region;
					}
					if (open.size() == 150) break;
					if (all_nodes.size() == 250) break;
				} else if (cur->prev != n) {
					if (total_cost < n->total_cost) {
						n->prev = cur;
						n->depth = cur->depth + 1;
						n->total_cost = total_cost;
						fp8 estimated_final_cost = n->total_cost + n->estimated_remaining_cost;
						if (n->visited) {
							n->directional_flags = v.flags;
							n->estimated_final_cost = estimated_final_cost;
							n->visited = false;
							n->is_goal = v.is_goal;
							open.push_back(n);
							binary_heap_up(std::prev(open.end()), open.begin(), open.end(), cmp_node());
							if (n->is_target_region) ++n_open_nodes_in_target_region;
							if (n->is_neighbor_region) ++n_open_nodes_in_neighbor_region;
						} else {
							n->estimated_final_cost = estimated_final_cost;
							binary_heap_up(std::find(open.begin(), open.end(), n), open.begin(), open.end(), cmp_node());
						}
						if (open.size() == 150) break;
					} else if (!n->visited) n->directional_flags &= v.flags;
				}
			}

			if (open.size() > pf.short_highest_open_size) {
				pf.short_highest_open_size = open.size();
			}

			has_found_goal = found_goal;
			if (open.size() == 150 || all_nodes.size() == 250) {
				if (!found_goal && !n_open_nodes_in_target_region) break;
			}
		}

		if (!goal_node) {
			int n_unvisited_nodes = 0;
			int n_unvisited_destination_region_nodes = 0;

			for (auto i = std::next(all_nodes.begin()); i != all_nodes.end(); ++i) {
				if (i->region->pathfinder_flag) ++i->region->pathfinder_flag;
				if (!i->visited) {
					if (i->directional_flags) i->directional_flags = pf_remove_visited_flags(i->pos, i->directional_flags);
					if (i->directional_flags) {
						++n_unvisited_nodes;
						if (i->region == destination_region) ++n_unvisited_destination_region_nodes;
					} else {
						i->visited = true;
						if (i->is_target_region) --n_open_nodes_in_target_region;
					}
				}
			}

			if (target_is_destination) {
				n_unvisited_nodes = n_unvisited_destination_region_nodes;
				for (auto* nr : move_to_region->walkable_neighbors) {
					if (nr == source_region) continue;
					if (nr->pathfinder_flag < 2) {
						++n_unvisited_nodes;
						break;
					} else {
						n_unvisited_nodes -= nr->pathfinder_flag / 2;
						if (n_unvisited_nodes < 0) n_unvisited_nodes = 0;
					}
				}
			}
			goal_node = start_node;
			start_node->estimated_remaining_cost = xy_length(to_xy_fp8(target - start_node->pos));
			if (n_unvisited_nodes == 0 && n_open_nodes_in_target_region == 0) {
				fp8 best_cost = fp8::integer(128 * 128);
				if (n_open_nodes_in_target_region == 0) best_cost = start_node->estimated_remaining_cost;
				node_t* best_node = start_node;
				for (auto i = std::next(all_nodes.begin()); i != all_nodes.end(); ++i) {
					if (i->estimated_remaining_cost < best_cost) {
						if (target_is_destination || i->region == source_region || (n_open_nodes_in_target_region && i->region == destination_region)) {
							best_cost = i->estimated_remaining_cost;
							best_node = &*i;
						}
					}
				}
				goal_node = best_node;
				pf.destination_reached = true;
				pf.is_stuck = true;
			} else {
				fp8 best_cost = fp8::integer(128 * 128);
				node_t* best_node = start_node;
				for (auto i = std::next(all_nodes.begin()); i != all_nodes.end(); ++i) {
					fp8 cost = i->estimated_remaining_cost;
					if (i->region == destination_region || i->region == target_region) {
						cost += i->total_cost / 2;
					} else {
						if (i->region->pathfinder_flag) {
							cost = cost * 3 / 2;
						} else {
							if (i->region == source_region) cost *= 2;
							else cost *= 4;
						}
					}
					if (i->visited) cost *= 32;
					if (cost < best_cost) {
						best_cost = cost;
						best_node = &*i;
					}
				}
				goal_node = best_node;
			}
		}

		pf.short_all_nodes_size = all_nodes.size();

		pf.short_path.clear();
		pf.current_short_path_index = 0;
		for (auto* n = goal_node; n != start_node && pf.short_path.size() < 128; n = n->prev) {
			pf.short_path.push_front(n->pos);
		}
		if (pf.short_path.size() > 50) {
			pf.short_path[49] = pf.short_path.back();
			pf.short_path.resize(50);
		}
		for (auto* nr : move_to_region->walkable_neighbors) {
			if (nr == source_region) continue;
			nr->pathfinder_flag = 0;
		}
	}

	std::pair<bool, xy> pathfinder_adjust_target_pos(rect unit_inner_bb, xy target) const {
		for (size_t width = 1; width < 10; width += 2) {
			for (size_t dir = 0; dir != 4; ++dir) {
				for (size_t n = 0; n != width; ++n) {
					auto bb = translate_rect(unit_inner_bb, target);
					if (is_in_inner_map_bounds(bb)) {
						bool entirely_walkable = true;
						size_t from_x = bb.from.x / 32u;
						size_t from_y = bb.from.y / 32u;
						size_t to_x = bb.to.x / 32u;
						size_t to_y = bb.to.y / 32u;
						for (size_t y = from_y; entirely_walkable && y <= to_y; ++y) {
							for (size_t x = from_x; x <= to_x; ++x) {
								size_t index = game_st.regions.tile_region_index.at(256 * y + x);
								if (index < 0x2000) {
									auto* region = &game_st.regions.regions[index];
									if (!region->walkable()) {
										entirely_walkable = false;
										break;
									}
								}
							}
						}
						if (entirely_walkable) return {true, target};
					}
					target += cardinal_direction_xy[dir] * 8;
				}
				target -= cardinal_direction_xy[dir] * 8;
			}
			target -= xy(8, 8);
		}
		return {false, target};
	}

	xy pathfinder_adjust_destination(const regions_t::region* source_region, xy destination) const {
		xy target = nearest_pos_in_rect(destination, source_region->area) / 32;
		for (size_t width = 1; width < 16; width += 2) {
			for (size_t dir = 0; dir != 4; ++dir) {
				for (size_t n = 0; n != width; ++n) {
					if ((size_t)target.x < game_st.map_tile_width && (size_t)target.y < game_st.map_tile_height) {
						const regions_t::region* r = get_region_at_prefer_walkable(target * 32u);
						if (r == source_region) return target * 32 + xy(16, 16);
					}
					target += cardinal_direction_xy[dir];
				}
				target -= cardinal_direction_xy[dir];
			}
			target -= xy(1, 1);
		}
		return to_xy(source_region->center);
	}

	bool pathfinder_find_next_short_path(pathfinder& pf) const {
		++pf.current_long_path_index;
		if (pf.current_long_path_index >= pf.long_path.size()) return false;
		const regions_t::region* source_region = pf.long_path[pf.current_long_path_index];
		while (source_region != pf.source_region) {
			++pf.current_long_path_index;
			if (pf.current_long_path_index >= pf.long_path.size()) return false;
			source_region = pf.long_path[pf.current_long_path_index];
		}
		xy target = pf.destination;
		bool is_near_destination = true;
		if (pf.long_path.size() - pf.current_long_path_index > 3) {
			target = to_xy(pf.long_path[pf.current_long_path_index + 2]->center);
			is_near_destination = false;
		}
		const regions_t::region* target_region = nullptr;
		if (pf.current_long_path_index != pf.long_path.size() - 1) {
			target_region = pf.long_path[pf.current_long_path_index + 1];
		} else {
			if (source_region != pf.destination_region) {
				pf.destination = pathfinder_adjust_destination(source_region, pf.destination);
				pf.is_stuck = true;
				is_near_destination = true;
				target_region = nullptr;
			}
		}
		if (is_near_destination) {
			pf.target_unit = pf.u->move_target.unit;
			if (!pf.target_unit) {
				auto unit_bb = unit_bounding_box(pf.u, pf.destination);
				auto add = xy(game_st.max_unit_width / 2 + 1, game_st.max_unit_height / 2 + 1);
				rect search_bb = {pf.destination - add, pf.destination + add};
				for (unit_t* u : find_units_noexpand(search_bb)) {
					if (is_intersecting(unit_bb, unit_sprite_bounding_box(u))) {
						if (pathfinder_unit_can_collide_with(pf, u)) {
							pf.target_unit = u;
							break;
						}
					}
				}
			}
			if (pf.target_unit) {
				if (unit_is_special_beacon(pf.target_unit)) {
					pf.target_unit = nullptr;
				}
			}
			if (pf.target_unit) {
				pf.target_unit_bb = unit_sprite_inner_bounding_box(pf.target_unit);
				pf.target_unit_bb.from -= pf.u->unit_type->dimensions.to + xy(1, 1);
				pf.target_unit_bb.to += pf.u->unit_type->dimensions.from + xy(1, 1);
				bool is_goal = false;
				if (pf.source.x == pf.target_unit_bb.from.x || pf.source.x == pf.target_unit_bb.to.x) {
					if (pf.source.y >= pf.target_unit_bb.from.y && pf.source.y <= pf.target_unit_bb.to.y) {
						is_goal = true;
					}
				}
				if (pf.source.y == pf.target_unit_bb.from.y || pf.source.y == pf.target_unit_bb.to.y) {
					if (pf.source.x >= pf.target_unit_bb.from.x && pf.source.x <= pf.target_unit_bb.to.x) {
						is_goal = true;
					}
				}
				if (is_goal) {
					pf.short_path = { pf.source };
					pf.destination = pf.source;
					pf.current_short_path_index = 0;
					pf.destination_reached = true;
					pf.is_stuck = false;
					return true;
				}
			} else {
				pf.target_unit_bb = {{-32000, -32000}, {-32000, -32000}};
				auto adjusted_target = pathfinder_adjust_target_pos(pf.unit_bb, target);
				if (adjusted_target.first && adjusted_target.second != target) {
					target = adjusted_target.second;
					pf.destination = target;
				}
			}

		}
		pathfinder_find_short_path(pf, target, target_region);
		if (!pf.short_path.empty()) {
			xy last_path_pos = pf.short_path.back();
			if (pf.destination_reached) {
				if (is_near_destination || last_path_pos != target) pf.destination = last_path_pos;
				else pf.destination_reached = false;
			} else {
				if (!target_region) {
					pf.destination = last_path_pos;
					pf.destination_reached = true;
					pf.is_stuck = true;
				}
			}
			return true;
		} else return false;
	}

	bool pathfinder_find_long_path(pathfinder& pf, xy from, xy to) const {
		pf.source = from;
		pf.destination = to;
		pf.source_region = get_region_at(pf.source);
		pf.destination_region = get_region_at(pf.destination);
		pf.long_all_nodes_size = 0;
		pf.long_highest_open_size = 0;
		pf.destination_reached = false;
		pf.is_stuck = false;
		if (pf.source_region == pf.destination_region) {
			pf.long_path = { pf.source_region };
			pf.full_long_path_size = 1;
			pf.current_long_path_index = (size_t)0 - 1;
			return true;
		} else {
			return pathfinder_find_long_path(pf);
		}
	}

	bool pathfinder_find(pathfinder& pf, bool short_path_only = false) {
		pf.source_region = get_region_at(pf.source);
		pf.destination_region = get_region_at(pf.destination);
		pf.unit_bb = unit_type_inner_bounding_box(pf.u->unit_type);
		pf.short_highest_open_size = 0;
		pf.short_all_nodes_size = 0;
		pf.long_all_nodes_size = 0;
		pf.long_highest_open_size = 0;
		pf.destination_reached = false;
		pf.is_stuck = false;
		if (short_path_only) return pathfinder_find_next_short_path(pf);
		if (pf.source_region == pf.destination_region) {
			pf.long_path = { pf.source_region };
			pf.full_long_path_size = 1;
			pf.current_long_path_index = (size_t)0 - 1;
			return pathfinder_find_next_short_path(pf);
		} else {
			if (!pathfinder_find_long_path(pf)) return false;
			return pathfinder_find_next_short_path(pf);
		}
	}

	path_t* new_path() {
		// We need a (first-in-last-out) free list of paths since
		// last_collision_unit is reused.
		if (!st.free_paths.empty()) {
			path_t* r = &st.free_paths.front();
			st.free_paths.pop_front();
			return r;
		}
		if (st.paths.size() >= 1024) return nullptr;
		return &*st.paths.emplace(st.paths.end());
	}

	void free_path(unit_t* u) {
		if (!u->path) return;
		free_path(u->path);
		u->path = nullptr;
	}

	void free_path(path_t* path) {
		st.free_paths.push_front(*path);
	}

	bool path_progress(unit_t* u, xy to, const unit_t* consider_collision_with_unit = nullptr, bool consider_collision_with_moving_units = false) {
		u_unset_movement_flag(u, 0x40);
		u_set_movement_flag(u, 0x10);
		pathfinder pf;
		pf.consider_collision_with_unit = consider_collision_with_unit;
		pf.consider_collision_with_moving_units = consider_collision_with_moving_units;
		bool find_new_path = true;
		if (u->path) {
			++u->path->current_short_path_index;
			if (u->path->current_short_path_index < u->path->short_path.size()) {
				find_new_path = false;
			} else {
				pf.u = u;
				pf.source = u->sprite->position;
				pf.destination = u->path->destination;
				pf.long_path = std::move(u->path->long_path);
				pf.full_long_path_size = u->path->full_long_path_size;
				pf.current_long_path_index = u->path->current_long_path_index;
				pf.short_path = std::move(u->path->short_path);
				pf.current_short_path_index = u->path->current_short_path_index;
				free_path(u->path);
				u->path = nullptr;
				if (pf.current_long_path_index + 1 < pf.long_path.size()) {
					if (pf.full_long_path_size == pf.long_path.size() || pf.current_long_path_index + 4 < pf.long_path.size()) {
						if (pf.source != pf.destination) {
							if (pathfinder_find(pf, true)) {
								if (!pf.long_path.empty() && !pf.short_path.empty()) find_new_path = false;
							}
						}
					}
				}
			}
		}
		if (find_new_path) {
			pf.u = u;
			pf.source = u->sprite->position;
			pf.destination = to;
			pathfinder_find(pf);
			if (pf.long_path.empty() || pf.short_path.empty()) return false;
		}
		path_t* path = u->path;
		if (!path) {
			if (pf.destination_reached) {
				if (to != pf.destination) {
					to = pf.destination;
					xy move_target = to;
					auto* move_target_unit = u->move_target.unit;
					if (move_target_unit && u_movement_flag(move_target_unit, 2)) {
						fp8 halt_distance = unit_halt_distance(u);
						xy pos = move_target_unit->sprite->position + to_xy(direction_xy(move_target_unit->next_velocity_direction, halt_distance * 3));
						if (is_in_map_bounds(pos)) move_target = pos;
					}
					set_flingy_move_target(u, move_target);
					u->move_target.unit = move_target_unit;
					u_set_status_flag(u, unit_t::status_flag_immovable, pf.is_stuck);
				}
			}

			if ((pf.long_path.size() + pf.short_path.size() * 2 + 2 - 1) / 2 * 2 > 48) {
				if (pf.short_path.size() >= 24) {
					pf.short_path.resize(23);
					pf.long_path.resize(1);
				} else {
					pf.long_path.resize((24 - pf.short_path.size()) * 2);
				}
			}
			path = new_path();
			if (!path) return false;
			path->delay = 0;
			path->creation_frame = st.current_frame;
			path->state_flags = 0;
			path->long_path = std::move(pf.long_path);
			path->full_long_path_size = pf.full_long_path_size;
			path->current_long_path_index = std::min(pf.current_long_path_index, path->long_path.size());
			path->short_path = std::move(pf.short_path);
			path->current_short_path_index = 0;

			path->source = u->sprite->position;
			path->destination = to;

			u->path = path;
		}
		path->next = path->short_path.at(path->current_short_path_index);
		if (path->next == u->move_target.pos) u_unset_movement_flag(u, 0x10);
		else if (xy_length(path->destination - path->next) < 16) u_set_movement_flag(u, 0x40);
		return true;
	}

	path_t* create_single_step_path(xy source, xy destination) {
		path_t* path = new_path();
		if (!path) return nullptr;
		path->delay = 0;
		// creation_frame is not set here :(
		path->state_flags = 0;

		// long_path[0] is uninitialized here, so we reproduce the uninitialized behavior
		if (path->short_path.size() <= 1) {
			path->long_path.resize(1);
		} else {
			size_t index = path->short_path[1].x;
			if (index < game_st.regions.regions.size()) {
				path->long_path = {&game_st.regions.regions[index]};
			} else path->long_path = {nullptr};
		}

		path->full_long_path_size = 1;
		path->current_long_path_index = 0;
		path->short_path = {destination};
		path->current_short_path_index = 0;

		path->source = source;
		path->destination = destination;
		path->next = destination;

		return path;
	}

	bool unit_path_to(unit_t* u, xy to, const unit_t* consider_collision_with_unit = nullptr, bool consider_collision_with_moving_units = false) {
		if (is_moving_along_path(u)) return true;
		if (!u_ground_unit(u)) {
			set_unit_move_target(u, to);
			free_path(u);
			u->path = create_single_step_path(u->sprite->position, to);
			if (!u->path) return false;
			return true;
		}
		if (!path_progress(u, to, consider_collision_with_unit, consider_collision_with_moving_units)) return false;
		set_next_target_waypoint(u, u->path->next);
		if (u->next_movement_waypoint != u->path->next) {
			u->next_movement_waypoint = u->path->next;
			u_set_movement_flag(u, 1);
		}
		return true;
	}

	bool unit_update_path_movement_state(unit_t* u, bool allow_new_path) {
		if (unit_is_at_move_target(u) || u_movement_flag(u, 4)) {
			u->movement_state = movement_states::UM_AtMoveTarget;
			return true;
		}
		if (u_collision(u) && u_ground_unit(u)) {
			if (u->path) {
				free_path(u->path);
				u->path = nullptr;
			}
			u->movement_state = movement_states::UM_CheckIllegal;
			return true;
		}
		auto next_movement_state = movement_states::UM_AnotherPath;
		if (!allow_new_path || u->path) {
			if (!u->path || ~u->path->state_flags & 1) return false;
			next_movement_state = movement_states::UM_NewMoveTarget;
		}
		if (u->movement_state >= movement_states::UM_FixCollision && u->movement_state <= movement_states::UM_TerrainSlide) {
			next_movement_state = movement_states::UM_TurnAndStart;
		}
		u->movement_state = next_movement_state;
		return true;
	}

	void set_unit_immovable(unit_t* u) {
		set_next_speed(u, 0_fp8);
		stop_unit(u);
		set_unit_move_target(u, u->sprite->position);
		u_set_status_flag(u, unit_t::status_flag_immovable);
		u->user_action_flags &= ~2;
	}

	void update_unit_pathing_collision(unit_t* u) {
		if (~u->movement_flags & 2) return;
		if (u->pathing_collision_counter <= 2) return;
		if (u_order_not_interruptible(u)) return;
		if (u_iscript_nobrk(u)) return;
		u->movement_flags &= ~2;
		sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
	}


	int cardinal_direction_from_to(const unit_t* from_u, const unit_t* to_u) const {
		auto a_bb = unit_sprite_inner_bounding_box(to_u);
		auto b_bb = unit_sprite_inner_bounding_box(from_u);
		if (a_bb.to.x < b_bb.from.x) return 3;
		if (a_bb.from.x > b_bb.to.x) return 1;
		if (a_bb.to.y < b_bb.from.y) return 0;
		if (a_bb.from.y > b_bb.to.y) return 2;
		int up_distance = std::abs(a_bb.to.y - b_bb.from.y);
		int right_distance = std::abs(a_bb.from.x - b_bb.to.x);
		int down_distance = std::abs(a_bb.from.y - b_bb.to.y);
		int left_distance = std::abs(a_bb.to.x - b_bb.from.x);
		std::array<int, 4> distances = {right_distance, left_distance, down_distance, up_distance};
		std::array<int, 4> r = {1, 3, 2, 0};
		return r[get_best_score(distances, identity()) - distances.begin()];
	}

	bool collision_get_slide_free_direction(const unit_t* u, const unit_t* collision_unit, direction_t& slide_free_direction) const {
		slide_free_direction = -1_dir;
		if (us_hidden(collision_unit)) return false;
		auto target_dir = xy_direction(u->next_movement_waypoint - u->sprite->position);
		auto target_dir_quadrant = direction_index(target_dir) / 64;
		auto dir_err = fp8::extend(target_dir - u->current_velocity_direction).abs();
		if (dir_err >= 80_fp8) return false;
		int left_x = collision_unit->unit_finder_bounding_box.from.x - u->unit_type->dimensions.to.x - 1;
		int right_x = collision_unit->unit_finder_bounding_box.to.x + u->unit_type->dimensions.from.x + 1;
		int up_y = collision_unit->unit_finder_bounding_box.from.y - u->unit_type->dimensions.to.y - 1;
		int down_y = collision_unit->unit_finder_bounding_box.to.y + u->unit_type->dimensions.from.y + 1;
		xy target_pos;
		switch (cardinal_direction_from_to(u, collision_unit)) {
		case 0:
			target_pos.y = down_y;
			if (target_dir_quadrant == 3) slide_free_direction = -64_dir;
			else if (target_dir_quadrant == 0) slide_free_direction = 64_dir;
			break;
		case 1:
			target_pos.x = left_x;
			if (target_dir_quadrant == 0) slide_free_direction = 0_dir;
			else if (target_dir_quadrant == 1) slide_free_direction = -128_dir;
			break;
		case 2:
			target_pos.y = up_y;
			if (target_dir_quadrant == 1) slide_free_direction = 64_dir;
			else if (target_dir_quadrant == 2) slide_free_direction = -64_dir;
			break;
		case 3:
			target_pos.x = right_x;
			if (target_dir_quadrant == 2) slide_free_direction = -128_dir;
			else if (target_dir_quadrant == 3) slide_free_direction = 0_dir;
			break;
		}
		for (int i = 0; i != 2; ++i) {
			if (slide_free_direction == 0_dir) {
				target_pos.y = up_y;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = -128_dir;
			} else if (slide_free_direction == -128_dir) {
				target_pos.y = down_y;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = 0_dir;
			} else if (slide_free_direction == 64_dir) {
				target_pos.x = right_x;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = -64_dir;
			} else if (slide_free_direction == -64_dir) {
				target_pos.x = left_x;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = 64_dir;
			}
		}

		return false;
	}

	void reset_movement_state(unit_t* u) {
		free_path(u);
		u->movement_state = movement_states::UM_Init;
		if (u->sprite->elevation_level < 12) u->pathing_flags |= 1;
		else u->pathing_flags &= ~1;
	}

	bool movement_UM_Init(unit_t* u, execute_movement_struct& ems) {
		u->pathing_flags &= ~(1 | 2);
		if (u->sprite->elevation_level < 12) u->pathing_flags |= 1;
		u->terrain_no_collision_bounds = {{0, 0}, {0, 0}};
		int next_state = movement_states::UM_Lump;
		if (!ut_turret(u) && u_iscript_nobrk(u)) {
			next_state = movement_states::UM_InitSeq;
		} else if (!u->sprite || unit_dead(u)) {
			next_state = movement_states::UM_Lump;
		} else if (u_in_bunker(u)) {
			next_state = movement_states::UM_Bunker;
		} else if (us_hidden(u)) {
			if (u_movement_flag(u, 2) || !unit_is_at_move_target(u)) {
				set_unit_immovable(u);
				update_unit_movement_values(u, ems);
				finish_unit_movement(u, ems);
			}
			next_state = movement_states::UM_Hidden;
		} else if (u_burrowed(u)) {
			next_state = movement_states::UM_Lump;
		} else if (u_can_move(u)) {
			next_state = u->pathing_flags & 1 ? movement_states::UM_AtRest : movement_states::UM_Flyer;
		} else if (u_can_turn(u)) {
			next_state = ut_turret(u) ? movement_states::UM_Turret : movement_states::UM_BldgTurret;
		} else if (u->pathing_flags & 1 && (u_movement_flag(u, 2) || !unit_is_at_move_target(u))) {
			next_state = movement_states::UM_LumpWannabe;
		}
		u->movement_state = next_state;
		return true;
	}

	bool movement_UM_AtRest(unit_t* u, execute_movement_struct& ems) {
		if (!unit_is_at_move_target(u)) {
			if (u->pathing_collision_counter) {
				if (u->pathing_collision_counter > 2) u->pathing_collision_counter = 2;
				else --u->pathing_collision_counter;
			}
		} else u->pathing_collision_counter = 0;
		auto go_to_next_waypoint = [&]() {
			if (u_movement_flag(u, 4)) return true;
			if (unit_is_at_move_target(u)) {
				if (u_movement_flag(u, 2)) return true;
				if (u->position != u->next_target_waypoint) {
					auto dir = xy_direction(u->next_target_waypoint - u->position);
					if (u->heading != dir) return true;
					if (u->next_velocity_direction != dir) return true;
				}
			}
			return false;
		};
		bool going_to_next_waypoint = false;
		if (go_to_next_waypoint()) {
			going_to_next_waypoint = true;
			update_unit_movement_values(u, ems);
			if (check_ground_unit_movement_unit_collision(u, ems) || check_unit_movement_terrain_collision(u, ems)) {
				set_next_speed(u, 0_fp8);
				u->movement_flags = ems.post_movement_flags;
				update_unit_heading(u, u->current_velocity_direction);
				if (ems.stopping_movement) {
					if (!s_flag(u->sprite, sprite_t::flag_iscript_nobrk)) {
						sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
					}
				} else if (ems.starting_movement) {
					sprite_run_anim(u->sprite, iscript_anims::Walking);
				}

				set_flingy_move_target(u, u->sprite->position);
				stop_unit(u);
			} else {
				finish_unit_movement(u, ems);
			}
		}
		if (u_collision(u) && u_ground_unit(u)) {
			u->movement_state = movement_states::UM_CheckIllegal;
			return false;
		}
		if (!unit_is_at_move_target(u) && !u_movement_flag(u, 4)) {
			u->movement_state = movement_states::UM_StartPath;
			return true;
		}
		if (!going_to_next_waypoint) {
			u->next_speed = 0_fp8;
			if (u->current_speed != 0_fp8) {
				u->current_speed = 0_fp8;
				u->velocity = {};
			}
			set_next_target_waypoint(u, u->sprite->position);
			u->movement_state = movement_states::UM_Dormant;
		}
		return false;
	}

	bool get_unique_sided_positions_within_bounds(xy& from, xy& to, rect bounds) const {
		auto get_flags = [&](xy pos) {
			int r = 0;
			if (pos.y < bounds.from.y) r |= 8;
			if (pos.y > bounds.to.y) r |= 4;
			if (pos.x > bounds.to.x) r |= 2;
			if (pos.x < bounds.from.x) r |= 1;
			return r;
		};

		xy initial_from = from;
		xy initial_to = to;

		int from_flags = get_flags(from);
		int to_flags = get_flags(to);


		while (true) {
			if ((from_flags | to_flags) == 0) return true;
			if (from_flags & to_flags) {
				from = initial_from;
				to = initial_to;
				return false;
			}
			auto visit = [&](int flags) {
				xy pos;
				if (flags & 8) {
					pos.x = from.x + (bounds.from.y - from.y) * (to.x - from.x) / (to.y - from.y);
					pos.y = bounds.from.y;
				} else if (flags & 4) {
					pos.x = from.x + (bounds.to.y - from.y) * (to.x - from.x) / (to.y - from.y);
					pos.y = bounds.to.y;
				} else if (flags & 2) {
					pos.x = bounds.to.x;
					pos.y = from.y + (bounds.to.x - from.x) * (to.y - from.y) / (to.x - from.x);
				} else {
					pos.x = bounds.from.x;
					pos.y = from.y + (bounds.from.x - from.x) * (to.y - from.y) / (to.x - from.x);
				}
				return pos;
			};
			if (from_flags) {
				from = visit(from_flags);
				from_flags = get_flags(from);
			} else {
				to = visit(to_flags);
				to_flags = get_flags(to);
			}
		}

	}

	bool movement_UM_CheckIllegal(unit_t* u, execute_movement_struct& ems) {
		u_unset_status_flag(u, unit_t::status_flag_collision);
		auto check_illegal = [&]() {
			if (!u_ground_unit(u)) return false;
			bool blocked;
			unit_t* blocking_unit;
			std::tie(blocked, blocking_unit) = is_blocked(u, u->sprite->position);
			if (!blocked) return false;
			if (u_order_not_interruptible(u) || u_iscript_nobrk(u) || u_movement_flag(u, 8)) {
				u_set_status_flag(u, unit_t::status_flag_collision);
				return false;
			}
			xy move_to = u->sprite->position;

			if (unit_type_can_fit_at(u->unit_type, move_to)) {
				if (blocking_unit) {
					check_unit_movement_terrain_collision(u, xy());
					auto find_move_to = [&]() {
						bool other_unit_is_moving = false;
						if (u_can_move(blocking_unit) && (!unit_is_at_move_target(blocking_unit) || blocking_unit->movement_state == movement_states::UM_CheckIllegal || blocking_unit->movement_state == movement_states::UM_MoveToLegal)) {
							other_unit_is_moving = true;
							if (lcg_rand(50, 0, 31) < 24) return;
						} else {
							auto super_bb = [&](const unit_t* a, const unit_t* b) {
								rect r = unit_sprite_inner_bounding_box(a);
								r.from.x -= b->unit_type->dimensions.to.x + 1;
								r.from.y -= b->unit_type->dimensions.to.y + 1;
								r.to.x += b->unit_type->dimensions.from.x + 1;
								r.to.y += b->unit_type->dimensions.from.y + 1;
								return r;
							};

							std::array<rect, 4> initial_rects;
							rect initial_rect = super_bb(blocking_unit, u);
							initial_rects.fill(initial_rect);
							initial_rects[0].to.y = initial_rect.from.y;
							initial_rects[1].from.x = initial_rect.to.x;
							initial_rects[2].from.y = initial_rect.to.y;
							initial_rects[3].to.x = initial_rect.from.x;

							auto find_rects = [&](const auto& rects, rect bounds) {
								static_vector<rect, 4> r;
								for (auto v : rects) {
									if (this->get_unique_sided_positions_within_bounds(v.from, v.to, bounds)) {
										r.emplace_back(v.from, v.to);
									}
								}
								return r;
							};

							auto vec = find_rects(initial_rects, u->terrain_no_collision_bounds);

							rect find_bb = initial_rect;
							find_bb.from.x -= u->unit_type->dimensions.from.x;
							find_bb.from.y -= u->unit_type->dimensions.from.y;
							find_bb.to.x += u->unit_type->dimensions.to.x;
							find_bb.to.y += u->unit_type->dimensions.to.y;
							for (const unit_t* n : find_units_noexpand(find_bb)) {
								if (pathfinder_unit_can_collide_with(u, n, nullptr, true)) {
									auto bounds = super_bb(n, u);
									vec = find_rects(vec, bounds);
								}
							}

							xy best_pos = u->sprite->position;
							int best_distance = 9999;
							for (auto& v : vec) {
								xy pos = u->sprite->position;
								if (pos.x > v.to.x) {
									pos.x = v.to.x;
								} else if (pos.x < v.from.x) {
									pos.x = v.from.x;
								}
								if (pos.y > v.to.y) {
									pos.y = v.to.y;
								} else if (pos.y < v.from.y) {
									pos.y = v.from.y;
								}
								int distance = xy_length(pos - u->sprite->position);
								if (distance < best_distance) {
									best_distance = distance;
									best_pos = pos;
								}
							}
							if (best_distance != 9999) {
								if (unit_type_can_fit_at(u->unit_type, best_pos) && !is_blocked(u, best_pos).first && is_reachable(u->sprite->position, best_pos)) {
									move_to = best_pos;
									return;
								}
							}
						}
						direction_t dir = u->next_velocity_direction + 16_dir * lcg_rand(50, -3, 3);
						fp8 length = fp8::integer(lcg_rand(50, 2, 4) * 4);
						if (!other_unit_is_moving && u_grounded_building(blocking_unit)) length *= 3;
						move_to = u->sprite->position + to_xy(direction_xy(dir, length));
						if (!is_blocked(u, move_to).first && is_reachable(u->sprite->position, move_to)) {
							return;
						}
						if (xy_length(u->move_target.pos - u->sprite->position) <= 32 || lcg_rand(50, 0, 31) >= 24) {
							move_to = u->sprite->position;
							move_to.x += 16 * lcg_rand(50, -2, 2);
							move_to.y += 16 * lcg_rand(50, -2, 2);
						} else {
							dir = xy_direction(u->move_target.pos - u->sprite->position) + 16_dir * lcg_rand(50, -1, 1);
							length = fp8::integer(8 + lcg_rand(50, 0, 2) * 4);
							if (!other_unit_is_moving && u_grounded_building(blocking_unit)) length *= 3;
							move_to = u->sprite->position + to_xy(direction_xy(dir, length));
						}
						if (!is_in_inner_map_bounds(unit_inner_bounding_box(u, move_to)) || !is_reachable(u->sprite->position, move_to)) {
							move_to = u->sprite->position;
						} else {
							auto is_blocked_r = is_blocked(u, move_to);
							if (is_blocked_r.first) {
								if (!is_blocked_r.second) {
									move_to = u->sprite->position;
								} else if (other_unit_is_moving && !u_can_move(is_blocked_r.second)) {
									move_to = u->sprite->position;
								}
							}
						}
					};
					find_move_to();
				}
			} else {
				auto* r = get_region_at(u->sprite->position);
				if (r->walkable()) {
					direction_t dir = xy_direction(to_xy(r->center) - u->sprite->position) + 16_dir * lcg_rand(50, -2, 2);
					fp8 length = fp8::integer(lcg_rand(50, 2, 4) * 8);
					fp8 center_length = fp8::integer(xy_length(to_xy(r->center) - u->sprite->position));
					if (length > center_length - 1_fp8) length = center_length - 1_fp8;
					move_to = u->sprite->position + to_xy(direction_xy(dir, length));
					if (!is_reachable(u->sprite->position, move_to)) move_to = u->sprite->position;
				} else {
					int best_distance = std::numeric_limits<int>::max();
					regions_t::region* best_region = nullptr;
					xy last_center_pos;
					for (auto* n : r->walkable_neighbors) {
						last_center_pos = to_xy(n->center);
						int distance = xy_length(to_xy(n->center) - u->sprite->position);
						if (distance < best_distance) {
							best_distance = distance;
							best_region = n;
						}
					}
					if (best_region) {
						direction_t dir;
						bool maybe_outside = false;
						if (u->sprite->position.x + 32 < best_region->area.from.x) maybe_outside = true;
						if (u->sprite->position.x - 32 >= best_region->area.to.x) maybe_outside = true;
						if (u->sprite->position.y + 32 < best_region->area.from.y) maybe_outside = true;
						if (u->sprite->position.y - 32 >= best_region->area.to.y) maybe_outside = true;
						if (maybe_outside) {
							// bug? seems like it should be best_region->center, not last_center_pos
							dir = xy_direction(last_center_pos - u->sprite->position);
							move_to = u->sprite->position + to_xy(direction_xy(dir, 64));
						} else {
							move_to = pathfinder_adjust_destination(best_region, u->sprite->position);
							if (move_to == u->sprite->position) move_to = u->sprite->position - xy(0, 64);
						}
					} else {
						if (!r->non_walkable_neighbors.empty()) {
							size_t random_index = lcg_rand(50, 0, (int)r->non_walkable_neighbors.size() - 1);
							auto* n = r->non_walkable_neighbors[random_index];
							int length = xy_length(to_xy(n->center) - u->sprite->position);
							if (length > 64) length = 64;
							direction_t dir = xy_direction(to_xy(n->center) - u->sprite->position);
							move_to = u->sprite->position + to_xy(direction_xy(dir, length));
						}
					}
				}
			}

			move_to = restrict_move_target_to_valid_bounds(u, move_to);
			if (move_to != u->sprite->position) {
				free_path(u);
				if (unit_is_at_move_target(u) || u->movement_flags & 4) {
					set_unit_move_target(u, move_to);
				} else {
					set_next_target_waypoint(u, move_to);
					if (u->next_movement_waypoint != move_to) {
						u->next_movement_waypoint = move_to;
						u_set_movement_flag(u, 1);
					}
				}
				u->path = create_single_step_path(u->sprite->position, move_to);
				u->terrain_no_collision_bounds = {};
				return true;
			} else {
				u_set_status_flag(u, unit_t::status_flag_collision);
				return false;
			}
		};
		if (check_illegal()) {
			u->pathing_flags |= 2;
			u->movement_state = movement_states::UM_MoveToLegal;
			return false;
		} else {
			u->pathing_flags &= ~(2 | 4);
			if (unit_is_at_move_target(u) || u_movement_flag(u, 4)) {
				u->movement_state = movement_states::UM_AtRest;
			} else {
				u->movement_state = movement_states::UM_AnotherPath;
			}
			return true;
		}
	}

	bool movement_UM_Dormant(unit_t* u, execute_movement_struct& ems) {
		bool rest = false;
		if (u_collision(u) && u_ground_unit(u)) rest = true;
		if (!unit_is_at_move_target(u)) rest = true;
		if (u->position != u->next_target_waypoint) rest = true;
		if (rest) {
			u->movement_state = movement_states::UM_AtRest;
			return true;
		}
		return false;
	}

	bool movement_UM_Turret(unit_t* u, execute_movement_struct& ems) {
		ems.refresh_vision = false;
		set_unit_move_target(u, u->sprite->position);
		auto dir_error = u->desired_velocity_direction - u->heading;
		if (dir_error != -128_dir) {
			if (dir_error >= -10_dir && dir_error <= 10_dir) {
				u_unset_movement_flag(u, 1);
			}
		}
		if (u_status_flag(u, (unit_t::status_flags_t)0x2000000)) {
			set_movement_flags(u, ems);
		} else {
			update_current_velocity_direction_towards_waypoint(u);
			set_movement_flags(u, ems);
			update_unit_heading(u, u->current_velocity_direction);
		}
		return false;
	}

	bool movement_UM_Flyer(unit_t* u, execute_movement_struct& ems) {
		if (u->sprite->position != u->move_target.pos) {
			xy move_target = restrict_unit_pos_to_bounds(u->move_target.pos, u->unit_type, map_bounds());
			if (move_target != u->move_target.pos) {
				set_flingy_move_target(u, move_target);
				u_set_status_flag(u, unit_t::status_flag_immovable);
			}
		}
		update_unit_movement_values(u, ems);
		bool being_repulsed = apply_repulse_field(u, ems);
		ems.position = restrict_unit_pos_to_bounds(ems.position, u->unit_type, map_bounds());
		finish_unit_movement(u, ems);
		if (u_can_move(u) && !ut_building(u) && u->unit_type->id != UnitTypes::Protoss_Interceptor) {
			size_t index = repulse_index(u->position);
			if (index != u->repulse_index) {
				decrement_repulse_field(u);
				increment_repulse_field(u);
			}
			if (being_repulsed) {
				if (std::max(std::abs(u->move_target.pos.x - u->position.x), std::abs(u->move_target.pos.y - u->position.y)) < 24) {
					u->move_target.pos = u->position;
					u->next_movement_waypoint = u->position;
				}
			}
		}
		return false;
	}

	bool movement_UM_AnotherPath(unit_t* u, execute_movement_struct& ems) {
		if (unit_path_to(u, u->move_target.pos, st.consider_collision_with_unit_bug)) u->movement_state = movement_states::UM_FollowPath;
		else u->movement_state = movement_states::UM_FailedPath;
		return false;
	}

	bool movement_UM_StartPath(unit_t* u, execute_movement_struct& ems) {
		if (unit_path_to(u, u->move_target.pos, st.consider_collision_with_unit_bug)) {
			auto next_movement_state = movement_states::UM_FollowPath;
			if (u->user_action_flags & 2) {
				u->user_action_flags &= ~2;
				if (u->flingy_movement_type == 2) {
					u->path->delay = lcg_rand(51, 0, 2);
					next_movement_state = movement_states::UM_UIOrderDelay;
				}
			}
			u->movement_state = next_movement_state;
			return true;
		} else {
			u->movement_state = movement_states::UM_FailedPath;
			return false;
		}
	}

	bool movement_UM_FollowPath(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, true)) return true;
		if (!unit_path_to(u, u->move_target.pos, st.consider_collision_with_unit_bug)) {
			u->movement_state = movement_states::UM_AnotherPath;
			return true;
		}
		fp8 speed = u->next_speed;
		update_unit_movement_values(u, ems);
		if (u->flingy_movement_type == 0) speed = u->current_speed;
		const unit_t* collision_unit = nullptr;
		collision_unit = check_ground_unit_movement_unit_collision(u, ems);
		if (collision_unit) {
			bool fix_collision = false;
			if (check_unit_movement_terrain_collision(u, ems)) fix_collision = true;
			else if (u->velocity.x < 32_fp8 && u->velocity.y < 32_fp8) fix_collision = true;
			else if (ems.speed != u->current_speed) fix_collision = true;
			else {
				auto pos = ems.position;
				auto exact_pos = ems.exact_position;
				ems.exact_position = u->exact_position + u->velocity / 2;
				ems.position = to_xy(ems.exact_position);
				if (check_unit_movement_unit_collision(u, ems)) {
					ems.exact_position = u->exact_position + u->velocity / 4;
					ems.position = to_xy(ems.exact_position);
					if (check_unit_movement_unit_collision(u, ems)) {
						ems.position = pos;
						ems.exact_position = exact_pos;
						fix_collision = true;
					}
				}
			}
			if (fix_collision) {
				u->path->last_collision_unit = get_unit_id(collision_unit);
				u->path->last_collision_speed = speed;
				u->movement_state = movement_states::UM_FixCollision;
				return true;
			}
		} else {
			if (check_unit_movement_terrain_collision(u, ems)) {
				u->path->last_collision_speed = speed;
				u->movement_state = movement_states::UM_FixTerrain;
				return true;
			}
		}
		if (u->sprite->position != ems.position) {
			if (u->pathing_collision_counter) {
				if (u->pathing_collision_counter > 2) u->pathing_collision_counter = 2;
				else --u->pathing_collision_counter;
			}
		}
		finish_unit_movement(u, ems);
		if (unit_is_at_move_target(u)) {
			u->movement_state = movement_states::UM_AtMoveTarget;
		} else {
			if (u->path) {
				if (u->path->delay) --u->path->delay;
				else {
					u->path->delay = 30;
					u->movement_state = movement_states::UM_ScoutPath;
				}
			}
		}
		return false;
	}

	bool movement_UM_ScoutPath(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, true)) return true;

		u->movement_state = movement_states::UM_FollowPath;
		return true;
	}

	bool movement_UM_AtMoveTarget(unit_t* u, execute_movement_struct& ems) {
		if (u->path) {
			free_path(u->path);
			u->path = nullptr;
		}
		if (u->next_movement_waypoint != u->move_target.pos) u->next_movement_waypoint = u->move_target.pos;
		if (!u_ground_unit(u) || u_movement_flag(u, 4)) {
			u->movement_state = movement_states::UM_AtRest;
		} else {
			u->movement_state = movement_states::UM_CheckIllegal;
		}
		return true;
	}

	bool movement_UM_NewMoveTarget(unit_t* u, execute_movement_struct& ems) {
		u->path->state_flags &= ~1;
		if (!unit_update_path_movement_state(u, true)) {
			if (u->path) {
				free_path(u->path);
				u->path = nullptr;
			}
			u->movement_state = movement_states::UM_Repath;
		}
		return true;
	}

	bool movement_UM_Repath(unit_t* u, execute_movement_struct& ems) {
		const unit_t* collision_unit = st.consider_collision_with_unit_bug;
		if (u->path) {
			collision_unit = get_unit(u->path->last_collision_unit);
			free_path(u->path);
			u->path = nullptr;
		}
		st.consider_collision_with_unit_bug = nullptr;
		if (unit_path_to(u, u->move_target.pos, collision_unit)) u->movement_state = movement_states::UM_FollowPath;
		else u->movement_state = movement_states::UM_FailedPath;
		return true;
	}

	bool movement_UM_UIOrderDelay(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, true)) return true;
		if (u->path->delay == 0) {
			u->movement_state = movement_states::UM_FollowPath;
			return true;
		}
		--u->path->delay;
		return false;
	}

	bool movement_UM_FixTerrain(unit_t* u, execute_movement_struct& ems) {
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		if (unit_update_path_movement_state(u, true)) return true;
		set_next_speed(u, u->path->last_collision_speed);
		update_unit_movement_values(u, ems);
		if (check_unit_movement_terrain_collision(u, ems) || check_unit_movement_unit_collision(u, ems)) {
			update_unit_pathing_collision(u);
			if (u->next_velocity_direction == xy_direction(u->next_movement_waypoint - u->sprite->position)) {
				direction_t slide_free_direction = -1_dir;
				xy movement = ems.position - u->sprite->position;
				auto desired_quadrant = direction_index(u->current_velocity_direction) / 64;
				if (ems.position.x != u->sprite->position.x && (ems.position.y == u->sprite->position.y || check_unit_movement_terrain_collision(u, xy{movement.x, 0}))) {
					if (movement.x >= 0) {
						if (desired_quadrant == 0) slide_free_direction = 0_dir;
						if (desired_quadrant == 1) slide_free_direction = -128_dir;
					} else {
						if (desired_quadrant == 3) slide_free_direction = 0_dir;
						if (desired_quadrant == 2) slide_free_direction = -128_dir;
					}
				} else {
					if (movement.y < 0) {
						if (desired_quadrant == 0) slide_free_direction = 64_dir;
						if (desired_quadrant == 3) slide_free_direction = -64_dir;
					} else {
						if (desired_quadrant == 1) slide_free_direction = 64_dir;
						if (desired_quadrant == 2) slide_free_direction = -64_dir;
					}
				}
				u->path->slide_free_direction = slide_free_direction;

				u->movement_state = movement_states::UM_TerrainSlide;
			} else {
				update_unit_heading(u, u->current_velocity_direction);
			}
		} else {
			finish_unit_movement(u, ems);
			u->movement_flags |= 1;
			u->movement_state = movement_states::UM_FollowPath;
		}
		return false;
	}

	bool movement_UM_FailedPath(unit_t* u, execute_movement_struct& ems) {
		u->pathing_collision_counter = 10;
		update_unit_pathing_collision(u);
		u->movement_state = movement_states::UM_RetryPath;
		return false;
	}

	bool movement_UM_TerrainSlide(unit_t* u, execute_movement_struct& ems) {
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		if (unit_update_path_movement_state(u, true)) return true;
		auto move = [&](direction_t direction) {
			ems.pre_movement_flags = u->movement_flags;
			set_current_velocity_direction(u, direction);
			update_current_speed_towards_waypoint(u);
			set_movement_flags(u, ems);
			set_movement_values(u, ems);
			ems.post_movement_flags = u->movement_flags;
			u->movement_flags = ems.pre_movement_flags;
		};
		u->movement_flags &= ~1;
		auto next_velocity_direction = u->next_velocity_direction;
		move(u->path->slide_free_direction);
		u->movement_flags |= 1;
		u->next_velocity_direction = next_velocity_direction;
		set_desired_velocity_direction(u, u->path->slide_free_direction);
		if (check_unit_movement_unit_collision(u, ems) || check_unit_movement_terrain_collision(u, ems)) {
			u->movement_state = movement_states::UM_ForceMoveFree;
		} else {
			finish_unit_movement(u, ems);
			auto next_speed = u->next_speed;
			set_next_speed(u, u->path->last_collision_speed);
			move(xy_direction(u->next_movement_waypoint - u->sprite->position));
			set_next_speed(u, next_speed);
			if (!check_unit_movement_terrain_collision(u, ems)) {
				u->movement_flags |= 1;
				u->movement_state = movement_states::UM_FollowPath;
			}
		}
		return false;
	}

	bool movement_UM_TurnAndStart(unit_t* u, execute_movement_struct& ems) {
		const unit_t* collision_unit = st.consider_collision_with_unit_bug;
		if (u->path) {
			collision_unit = get_unit(u->path->last_collision_unit);
			free_path(u->path);
			u->path = nullptr;
			st.consider_collision_with_unit_bug = collision_unit;
		}
		if (unit_is_at_move_target(u) || u->movement_flags & 4) {
			u->movement_state = movement_states::UM_AtMoveTarget;
			return true;
		}
		st.consider_collision_with_unit_bug = nullptr;
		if (unit_path_to(u, u->move_target.pos, collision_unit)) {
			u->movement_state = movement_states::UM_FaceTarget;
			return false;
		} else {
			u->movement_state = movement_states::UM_FailedPath;
			return true;
		}
	}

	bool movement_UM_FaceTarget(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, true)) return true;
		if (unit_is_at_move_target(u) || u->movement_flags & 4) {
			u->movement_state = movement_states::UM_AtMoveTarget;
			return true;
		}
		if (u->next_velocity_direction == xy_direction(u->next_movement_waypoint - u->sprite->position)) {
			if (u->path->state_flags & 1) u->movement_state = movement_states::UM_NewMoveTarget;
			else u->movement_state = movement_states::UM_FollowPath;
			return true;
		} else {
			update_unit_movement_values(u, ems);
			update_unit_heading(u, u->current_velocity_direction);
			return false;
		}
	}

	bool movement_UM_RetryPath(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, false)) return true;
		if (u->pathing_collision_counter < 20) {
			++u->pathing_collision_counter;
			return false;
		}
		if (unit_path_to(u, u->move_target.pos, st.consider_collision_with_unit_bug)) {
			u->movement_state = movement_states::UM_FollowPath;
			return true;
		}
		set_unit_immovable(u);
		u->movement_state = movement_states::UM_AtMoveTarget;
		return false;
	}

	bool movement_UM_InitSeq(unit_t* u, execute_movement_struct& ems) {
		if (u_status_flag(u, unit_t::status_flag_iscript_nobrk)) return false;
		u->movement_state = movement_states::UM_Init;
		return true;
	}

	bool movement_UM_ForceMoveFree(unit_t* u, execute_movement_struct& ems) {
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		if (unit_is_at_move_target(u) || u_movement_flag(u, 4)) {
			u->movement_state = movement_states::UM_AtMoveTarget;
			return false;
		}
		set_next_speed(u, 0_fp8);
		free_path(u);
		u->movement_state = movement_states::UM_TurnAndStart;
		return true;
	}

	bool movement_UM_FixCollision(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, false)) return true;
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		const unit_t* collision_unit = get_unit(u->path->last_collision_unit);
		if (!collision_unit || us_hidden(collision_unit) || !unit_can_collide_with(u, collision_unit)) {
			u->movement_state = movement_states::UM_FollowPath;
			return false;
		}
		direction_t slide_free_direction;
		int state = 1;
		if (!u->move_target.unit || !is_intersecting(unit_sprite_bounding_box(unit_main_unit(u)), unit_sprite_bounding_box(u->move_target.unit))) {
			if (is_intersecting(unit_sprite_inner_bounding_box(collision_unit), unit_inner_bounding_box(u, u->move_target.pos))) {
				if (collision_unit->pathing_collision_counter >= 30) state = u->move_target.unit ? 3 : 2;
				else if (is_intersecting(unit_inner_bounding_box(collision_unit, collision_unit->move_target.pos), unit_inner_bounding_box(u, u->move_target.pos))) {
					state = u->move_target.unit ? 3 : 2;
				}
			} else {
				if (!is_intersecting(unit_sprite_inner_bounding_box(collision_unit), unit_inner_bounding_box(u, u->next_movement_waypoint))) {
					if (u_movement_flag(collision_unit, 2)) {
						auto index = direction_index(collision_unit->next_velocity_direction);
						switch (cardinal_direction_from_to(u, collision_unit)) {
						case 0:
							if (index <= 64 || index >= 192) state = 6;
							break;
						case 1:
							if (index <= 128) state = 6;
							break;
						case 2:
							if (index >= 64 && index <= 192) state = 6;
							break;
						case 3:
							if (index == 0 || index >= 128) state = 6;
							break;
						}
						if (state == 1) {
							if (collision_unit->pathing_collision_counter <= 2) {
								if (!unit_is_at_move_target(collision_unit)) {
									if (collision_unit->movement_state == movement_states::UM_FollowPath || collision_unit->movement_state == movement_states::UM_ScoutPath) {
										state = 6;
									} else if (collision_unit->movement_state != movement_states::UM_WaitFree) {
										state = 4;
									}
								}
							}
						}
					}
					if (state == 1) {
						if (collision_get_slide_free_direction(u, collision_unit, slide_free_direction)) state = 7;
						else state = 5;
					}
				}
				if (state == 1) {
					if (collision_unit->pathing_collision_counter >= 30) state = 3;
					else if (is_intersecting(unit_inner_bounding_box(collision_unit, collision_unit->move_target.pos), unit_inner_bounding_box(u, u->next_movement_waypoint))) {
						state = 3;
					}
				}
			}
			if (state == 1) {
				direction_t collision_dir = xy_direction(collision_unit->sprite->position - u->sprite->position);
				direction_t cmp_dir = 0_dir;
				auto index = direction_index(collision_unit->next_velocity_direction);
				if (index < 32) cmp_dir = 0_dir;
				else if (index < 96) cmp_dir = 64_dir;
				else if (index < 160) cmp_dir = -128_dir;
				else if (index < 224) cmp_dir = -64_dir;
				if (fp8::extend(cmp_dir - collision_dir).abs() > 64_fp8) {
					state = 4;
				} else {
					state = 6;
				}
			}
		}
		if (st.current_frame - u->path->creation_frame >= 7 || state < 3 || state > 5) {
			switch (state) {
			case 1:
				set_unit_immovable(u);
				u_unset_status_flag(u, unit_t::status_flag_immovable);
				u->movement_state = movement_states::UM_AtMoveTarget;
				break;
			case 2:
				set_unit_immovable(u);
				u->movement_state = movement_states::UM_AtMoveTarget;
				break;
			case 3:
				u->movement_state = movement_states::UM_Repath;
				break;
			case 4:
				u->movement_state = movement_states::UM_RepathMovers;
				break;
			case 5:
				u->movement_state = movement_states::UM_TurnAndStart;
				set_next_speed(u, 0_fp8);
				break;
			case 6:
				u->movement_state = movement_states::UM_WaitFree;
				break;
			case 7:
				u->path->slide_free_direction = slide_free_direction;
				u->movement_state = movement_states::UM_SlidePrep;
				break;
			case 8:
				u->movement_state = movement_states::UM_GetFree;
				break;
			}
		} else {
			update_unit_pathing_collision(u);
		}
		return false;
	}

	bool movement_UM_WaitFree(unit_t* u, execute_movement_struct& ems) {
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		if (unit_update_path_movement_state(u, true)) return true;
		set_next_speed(u, u->path->last_collision_speed);
		update_unit_movement_values(u, ems);
		if (check_unit_movement_unit_collision(u, ems)) {
			update_unit_pathing_collision(u);
			set_next_speed(u, 0_fp8);
			if (u->next_velocity_direction != xy_direction(u->next_movement_waypoint - u->sprite->position)) {
				update_unit_heading(u, u->current_velocity_direction);
			}
			if (u->pathing_collision_counter >= 25) {
				if (lcg_rand(51) < 0x7fff / 2) {
					u->movement_state = movement_states::UM_ForceMoveFree;
				} else {
					u->movement_state = movement_states::UM_RepathMovers;
				}
			}
		} else {
			if (check_unit_movement_terrain_collision(u, ems)) {
				u->movement_state = movement_states::UM_Repath;
			} else {
				finish_unit_movement(u, ems);
				u->movement_state = movement_states::UM_FollowPath;
			}
		}
		return false;
	}

	bool movement_UM_SlidePrep(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, true)) return true;
		if (u->flingy_movement_type == 0) {
			if (u->path->last_collision_speed >= fp8::integer(2)) u->path->last_collision_speed /= 2;
			else if (u->path->last_collision_speed > fp8::integer(1)) u->path->last_collision_speed = fp8::integer(1);
		}
		set_current_velocity_direction(u, u->next_velocity_direction);
		set_next_speed(u, u->path->last_collision_speed);
		u->movement_state = movement_states::UM_SlideFree;
		return true;
	}

	bool movement_UM_SlideFree(unit_t* u, execute_movement_struct& ems) {
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		if (unit_update_path_movement_state(u, true)) return true;
		u->movement_flags |= 1;
		set_desired_velocity_direction(u, u->path->slide_free_direction);
		update_unit_heading(u, u->current_velocity_direction);
		if (u->flingy_movement_type == 2) u->current_speed = u->flingy_top_speed;
		else update_current_speed_towards_waypoint(u);

		auto move = [&]() {
			ems.pre_movement_flags = u->movement_flags;
			set_movement_flags(u, ems);
			set_movement_values(u, ems);
			ems.post_movement_flags = u->movement_flags;
			u->movement_flags = ems.pre_movement_flags;
		};

		fp8 original_speed = u->current_speed;
		auto original_velocity_direction = u->next_velocity_direction;
		const unit_t* collision_unit = get_unit(u->path->last_collision_unit);
		const unit_t* next_collision_unit = nullptr;
		bool force_move_free = false;
		for (fp8 speed = original_speed; speed > 0_fp8; speed -= fp8::integer(1)) {
			u_unset_movement_flag(u, 1);
			set_current_velocity_direction(u, u->path->slide_free_direction);
			set_next_speed(u, std::min(speed, fp8::integer(1)));
			move();
			if (check_unit_movement_unit_collision(u, ems) || check_unit_movement_terrain_collision(u, ems)) {
				force_move_free = true;
				break;
			}
			finish_unit_movement(u, ems);
			u->next_velocity_direction = original_velocity_direction;
			update_current_velocity_direction_towards_waypoint(u);
			set_next_speed(u, original_speed);
			move();
			next_collision_unit = check_unit_movement_unit_collision(u, ems);
			if (next_collision_unit != collision_unit) break;
		}
		u->next_velocity_direction = original_velocity_direction;
		set_current_velocity_direction(u, u->next_velocity_direction);
		set_next_speed(u, original_speed);
		if (force_move_free) {
			u->movement_state = movement_states::UM_ForceMoveFree;
			return false;
		}
		if (!next_collision_unit) {
			u_set_movement_flag(u, 1);
			if (st.current_frame - u->path->creation_frame < 150) {
				u->movement_state = movement_states::UM_FollowPath;
			} else {
				u->movement_state = movement_states::UM_TurnAndStart;
			}
		}
		return false;
	}

	bool movement_UM_RepathMovers(unit_t* u, execute_movement_struct& ems) {
		free_path(u);
		auto move_target = u->move_target;
		if (!unit_path_to(u, move_target.pos, st.consider_collision_with_unit_bug, true) || u_immovable(u)) {
			u->move_target = move_target;
			u->movement_state = movement_states::UM_TurnAndStart;
			return false;
		} else {
			u->movement_state = movement_states::UM_FaceTarget;
			return true;
		}
	}

	bool movement_UM_GetFree(unit_t* u, execute_movement_struct& ems) {
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		if (unit_update_path_movement_state(u, true)) return true;
		set_next_speed(u, u->path->last_collision_speed);
		update_unit_movement_values(u, ems);
		const unit_t* collision_unit = check_unit_movement_unit_collision(u, ems);
		if (collision_unit) {
			if (u->next_velocity_direction == xy_direction(u->next_movement_waypoint - u->sprite->position)) {
				free_path(u->path);
				u->path = nullptr;
			}
		} else {
			if (check_unit_movement_terrain_collision(u, ems)) {
				u->movement_state = movement_states::UM_Repath;
			} else {
				finish_unit_movement(u, ems);
				u->movement_state = movement_states::UM_FollowPath;
			}
		}
		return false;
	}

	bool movement_UM_Lump(unit_t* u, execute_movement_struct& ems) {
		return false;
	}

	bool movement_UM_MoveToLegal(unit_t* u, execute_movement_struct& ems) {
		if (!u->path) {
			u->movement_state = movement_states::UM_CheckIllegal;
			return true;
		}

		auto move = [&]() {
			if (!is_moving_along_path(u)) return true;
			update_unit_movement_values(u, ems);
			if (is_in_map_bounds(u->unit_type, ems.position)) {
				finish_unit_movement(u, ems);
				return false;
			} else {
				if (u->next_velocity_direction == xy_direction(u->next_movement_waypoint - u->sprite->position)) {
					return true;
				} else {
					update_unit_movement_values(u, ems);
					update_unit_heading(u, u->current_velocity_direction);
					return false;
				}
			}
		};

		if (u->pathing_flags & 4) {
			u->pathing_flags &= ~4;
			u_set_status_flag(u, unit_t::status_flag_collision);
			if (u->path) {
				free_path(u->path);
				u->path = nullptr;
			}
			u->movement_state = movement_states::UM_AtRest;
			return true;
		} else {
			if (u->path->state_flags & 1 || u->path->next != u->next_movement_waypoint) {
				u->next_movement_waypoint = u->path->next;
			}
			if (move()) {
				if (u->path) {
					free_path(u->path);
					u->path = nullptr;
				}
				check_unit_collision(u);
				u_set_status_flag(u, unit_t::status_flag_collision);
				u->movement_state = movement_states::UM_CheckIllegal;
			}
			return false;
		}
	}

	bool movement_UM_Hidden(unit_t* u, execute_movement_struct& ems) {
		ems.refresh_vision = false;
		return false;
	}

	bool movement_UM_Bunker(unit_t* u, execute_movement_struct& ems) {
		ems.refresh_vision = false;
		update_current_velocity_direction_towards_waypoint(u);
		set_movement_flags(u, ems);
		update_unit_heading(u, u->current_velocity_direction);
		return false;
	}

	bool movement_UM_LumpWannabe(unit_t* u, execute_movement_struct& ems) {
		if (!unit_is_at_move_target(u) || u_movement_flag(u, 2)) {
			update_unit_movement_values(u, ems);
			if (check_ground_unit_movement_unit_collision(u, ems) || check_unit_movement_terrain_collision(u, ems)) {
				set_next_speed(u, 0_fp8);
				u->movement_flags = ems.post_movement_flags;
				update_unit_heading(u, u->current_velocity_direction);
				if (ems.stopping_movement) {
					if (!s_flag(u->sprite, sprite_t::flag_iscript_nobrk)) {
						sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
					}
				} else if (ems.starting_movement) {
					sprite_run_anim(u->sprite, iscript_anims::Walking);
				}

				set_flingy_move_target(u, u->sprite->position);
				stop_unit(u);
			} else {
				finish_unit_movement(u, ems);
			}
			return false;
		} else {
			u->movement_state = movement_states::UM_Lump;
			return true;
		}
	}

	bool movement_UM_BldgTurret(unit_t* u, execute_movement_struct& ems) {
		update_current_velocity_direction_towards_waypoint(u);
		set_movement_flags(u, ems);
		update_unit_heading(u, u->current_velocity_direction);
		return false;
	}

	bool execute_movement(unit_t* u) {
		execute_movement_struct ems;
		ems.refresh_vision = update_tiles;

		while (true) {
			bool cont = false;
			switch (u->movement_state) {
			case movement_states::UM_Init:
				cont = movement_UM_Init(u, ems);
				break;
			case movement_states::UM_InitSeq:
				cont = movement_UM_InitSeq(u, ems);
				break;
			case movement_states::UM_Lump:
				cont = movement_UM_Lump(u, ems);
				break;
			case movement_states::UM_Turret:
				cont = movement_UM_Turret(u, ems);
				break;
			case movement_states::UM_Bunker:
				cont = movement_UM_Bunker(u, ems);
				break;
			case movement_states::UM_BldgTurret:
				cont = movement_UM_BldgTurret(u, ems);
				break;
			case movement_states::UM_Hidden:
				cont = movement_UM_Hidden(u, ems);
				break;
			case movement_states::UM_Flyer:
				cont = movement_UM_Flyer(u, ems);
				break;

			case movement_states::UM_AtRest:
				cont = movement_UM_AtRest(u, ems);
				break;
			case movement_states::UM_Dormant:
				cont = movement_UM_Dormant(u, ems);
				break;
			case movement_states::UM_AtMoveTarget:
				cont = movement_UM_AtMoveTarget(u, ems);
				break;
			case movement_states::UM_CheckIllegal:
				cont = movement_UM_CheckIllegal(u, ems);
				break;
			case movement_states::UM_MoveToLegal:
				cont = movement_UM_MoveToLegal(u, ems);
				break;
			case movement_states::UM_LumpWannabe:
				cont = movement_UM_LumpWannabe(u, ems);
				break;
			case movement_states::UM_FailedPath:
				cont = movement_UM_FailedPath(u, ems);
				break;
			case movement_states::UM_RetryPath:
				cont = movement_UM_RetryPath(u, ems);
				break;
			case movement_states::UM_StartPath:
				cont = movement_UM_StartPath(u, ems);
				break;
			case movement_states::UM_UIOrderDelay:
				cont = movement_UM_UIOrderDelay(u, ems);
				break;
			case movement_states::UM_TurnAndStart:
				cont = movement_UM_TurnAndStart(u, ems);
				break;
			case movement_states::UM_FaceTarget:
				cont = movement_UM_FaceTarget(u, ems);
				break;
			case movement_states::UM_NewMoveTarget:
				cont = movement_UM_NewMoveTarget(u, ems);
				break;
			case movement_states::UM_AnotherPath:
				cont = movement_UM_AnotherPath(u, ems);
				break;
			case movement_states::UM_Repath:
				cont = movement_UM_Repath(u, ems);
				break;
			case movement_states::UM_RepathMovers:
				cont = movement_UM_RepathMovers(u, ems);
				break;
			case movement_states::UM_FollowPath:
				cont = movement_UM_FollowPath(u, ems);
				break;
			case movement_states::UM_ScoutPath:
				cont = movement_UM_ScoutPath(u, ems);
				break;
			case movement_states::UM_FixCollision:
				cont = movement_UM_FixCollision(u, ems);
				break;
			case movement_states::UM_WaitFree:
				cont = movement_UM_WaitFree(u, ems);
				break;
			case movement_states::UM_GetFree:
				cont = movement_UM_GetFree(u, ems);
				break;
			case movement_states::UM_SlidePrep:
				cont = movement_UM_SlidePrep(u, ems);
				break;
			case movement_states::UM_SlideFree:
				cont = movement_UM_SlideFree(u, ems);
				break;
			case movement_states::UM_ForceMoveFree:
				cont = movement_UM_ForceMoveFree(u, ems);
				break;
			case movement_states::UM_FixTerrain:
				cont = movement_UM_FixTerrain(u, ems);
				break;
			case movement_states::UM_TerrainSlide:
				cont = movement_UM_TerrainSlide(u, ems);
				break;

			default:
				error("fixme: movement state %d\n", u->movement_state);
			}
			if (!cont) break;
		}
		return ems.refresh_vision;
	}

	bool unit_type_is_morphing_building(unit_type_autocast ut) const {
		if (unit_is(ut, UnitTypes::Zerg_Hive)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Lair)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Greater_Spire)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Spore_Colony)) return true;
		if (unit_is(ut, UnitTypes::Zerg_Sunken_Colony)) return true;
		return false;
	}

	bool unit_is_morphing_building(const unit_t* u) const {
		if (u_completed(u)) return false;
		if (u->build_queue.empty()) return false;
		return unit_type_is_morphing_building(u->build_queue.front());
	}

	int unit_sight_range(const unit_t* u, bool ignore_blindness = false) const {
		if (u_grounded_building(u) && !u_completed(u) && !unit_is_morphing_building(u)) return 32 * 4;
		if (!ignore_blindness && u->blinded_by) return 32 * 2;
		if (unit_is(u, UnitTypes::Terran_Ghost) && player_has_upgrade(u->owner, UpgradeTypes::Ocular_Implants)) return 32 * 11;
		if (unit_is(u, UnitTypes::Zerg_Overlord) && player_has_upgrade(u->owner, UpgradeTypes::Antennae)) return 32 * 11;
		if (unit_is(u, UnitTypes::Protoss_Observer) && player_has_upgrade(u->owner, UpgradeTypes::Sensor_Array)) return 32 * 11;
		if (unit_is(u, UnitTypes::Protoss_Scout) && player_has_upgrade(u->owner, UpgradeTypes::Apial_Sensors)) return 32 * 11;
		return 32 * u->unit_type->sight_range;
	}
	int unit_sight_range_ignore_blindness(const unit_t* u) const {
		return unit_sight_range(u, true);
	}

	int unit_target_acquisition_range(const unit_t* u) const {
		if ((u_cloaked(u) || u_requires_detector(u)) && u->order_type->id != Orders::HoldPosition) {
			if (unit_is_ghost(u)) return 0;
		}
		int bonus = 0;
		if (unit_is(u, UnitTypes::Terran_Marine) && player_has_upgrade(u->owner, UpgradeTypes::U_238_Shells)) bonus = 1;
		if (unit_is(u, UnitTypes::Zerg_Hydralisk) && player_has_upgrade(u->owner, UpgradeTypes::Grooved_Spines)) bonus = 1;
		if (unit_is(u, UnitTypes::Protoss_Dragoon) && player_has_upgrade(u->owner, UpgradeTypes::Singularity_Charge)) bonus = 2;
		if (unit_is(u, UnitTypes::Hero_Fenix_Dragoon)) bonus = 2;
		if (unit_is(u, UnitTypes::Terran_Goliath) && player_has_upgrade(u->owner, UpgradeTypes::Charon_Boosters)) bonus = 3;
		if (unit_is(u, UnitTypes::Terran_Goliath_Turret) && player_has_upgrade(u->owner, UpgradeTypes::Charon_Boosters)) bonus = 3;
		if (unit_is(u, UnitTypes::Hero_Alan_Schezar)) bonus = 3;
		if (unit_is(u, UnitTypes::Hero_Alan_Schezar_Turret)) bonus = 3;
		return u->unit_type->target_acquisition_range + bonus;
	}

	fp8 unit_max_energy(const unit_t* u) const {
		if (ut_hero(u)) return fp8::integer(250);
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
			default: return UpgradeTypes::None;
			};
		};
		auto upg = energy_upgrade();
		if (upg != UpgradeTypes::None && player_has_upgrade(u->owner, upg)) return fp8::integer(250);
		else return fp8::integer(200);
	}


	bool visible_to_everyone(const unit_t* u) const {
		if (ut_worker(u)) {
			if (u->worker.powerup && unit_is(u->worker.powerup, UnitTypes::Powerup_Flag)) return true;
			else return false;
		}
		if (!unit_provides_space(u)) return false;
		for (const unit_t* lu : loaded_units(u)) {
			if (lu->worker.powerup && unit_is(lu->worker.powerup, UnitTypes::Powerup_Flag)) return true;
		}
		return false;
	}

	size_t tile_index(xy pos) const {
		size_t ux = (size_t)pos.x / 32;
		size_t uy = (size_t)pos.y / 32;
		if (ux >= game_st.map_tile_width || uy >= game_st.map_tile_height) error("attempt to get tile index for invalid position %d %d", pos.x, pos.y);
		return uy * game_st.map_tile_width + ux;
	}

	uint8_t tile_visibility(xy pos) const {
		return ~st.tiles[tile_index(pos)].visible;
	}

	uint8_t tile_explored(xy pos) const {
		return ~st.tiles[tile_index(pos)].explored;
	}

	int get_ground_height_at(xy pos) const {
		size_t index = tile_index(pos);
		tile_id tile_id = game_st.gfx_tiles.at(index);
		size_t megatile_index = game_st.cv5.at(tile_id.group_index()).mega_tile_index[tile_id.subtile_index()];
		size_t ux = pos.x;
		size_t uy = pos.y;
		int flags = game_st.vf4.at(megatile_index).flags[uy / 8 % 4 * 4 + ux / 8 % 4];
		if (flags & vf4_entry::flag_high) return 2;
		if (flags & vf4_entry::flag_middle) return 1;
		return 0;
	}

	void reveal_sight_at(xy pos, int range, int reveal_to, bool in_air) {
		int visibility_mask = ~reveal_to;
		int height_mask = 0;
		if (!in_air) {
			int height = get_ground_height_at(pos);
			if (height == 2) height_mask = tile_t::flag_very_high;
			else if (height == 1) height_mask = tile_t::flag_very_high | tile_t::flag_high;
			else height_mask = tile_t::flag_very_high | tile_t::flag_high | tile_t::flag_middle;
		}
		const size_t max_width = 11 * 2 + 3;
		std::array<uint32_t, max_width * max_width> vision_propagation;
		uint32_t required_tile_mask = (uint32_t)height_mask << 16 | (uint32_t)(uint8_t)~visibility_mask << 8 | (uint32_t)(uint8_t)~visibility_mask;
		const auto& sight_vals = game_st.sight_values.at(range);
		size_t tile_x = (size_t)pos.x / 32;
		size_t tile_y = (size_t)pos.y / 32;
		tile_t* base_tile = &st.tiles[tile_x + tile_y*game_st.map_tile_width];
		if (!in_air) {
			size_t index = 0;
			size_t end = sight_vals.min_mask_size;
			for (; index != end; ++index) {
				const auto& cur = sight_vals.maskdat[index];
				vision_propagation[index] = 0xff;
				if (tile_x + cur.x >= game_st.map_tile_width) continue;
				if (tile_y + cur.y >= game_st.map_tile_height) continue;
				auto& tile = base_tile[cur.relative_tile_index];
				tile.visible &= visibility_mask;
				tile.explored &= visibility_mask;
				vision_propagation[index] = (uint32_t)tile.flags << 16 | (uint32_t)tile.explored << 8 | (uint32_t)tile.visible;
			}
			end += sight_vals.ext_masked_count;
			for (; index != end; ++index) {
				const auto& cur = sight_vals.maskdat[index];
				vision_propagation[index] = 0xff;
				if (tile_x + cur.x >= game_st.map_tile_width) continue;
				if (tile_y + cur.y >= game_st.map_tile_height) continue;
				if (vision_propagation[cur.prev] & required_tile_mask) {
					if (cur.prev2 == (size_t)~0 || (vision_propagation[cur.prev2] & required_tile_mask)) continue;
				}
				auto& tile = base_tile[cur.relative_tile_index];
				tile.visible &= visibility_mask;
				tile.explored &= visibility_mask;
				vision_propagation[index] = (uint32_t)tile.flags << 16 | (uint32_t)tile.explored << 8 | (uint32_t)tile.visible;
			}
		} else {
			// This seems bugged; even for air units, if you only traverse ext_masked_count nodes,
			// then you will still miss out on the min_mask_size (9) last ones
			auto* cur = sight_vals.maskdat.data();
			auto* end = cur + sight_vals.ext_masked_count;
			for (; cur != end; ++cur) {
				if (tile_x + cur->x >= game_st.map_tile_width) continue;
				if (tile_y + cur->y >= game_st.map_tile_height) continue;
				auto& tile = base_tile[cur->relative_tile_index];
				tile.visible &= visibility_mask;
				tile.explored &= visibility_mask;
			}
		}
	}

	void refresh_unit_vision(unit_t* u) {
		if (u->owner >= 8 && !u->parasite_flags) return;
		if (unit_is(u, UnitTypes::Terran_Nuclear_Missile)) return;
		int visible_to = 0;
		if (visible_to_everyone(u) || (unit_is(u, UnitTypes::Powerup_Flag) && u->order_type->id == Orders::UnusedPowerup)) visible_to = 0xff;
		else {
			visible_to = st.shared_vision[u->owner] | u->parasite_flags;
			if (u->parasite_flags) {
				visible_to |= u->parasite_flags;
				for (size_t i = 0; i != 12; ++i) {
					if (~u->parasite_flags&(1 << i)) continue;
					visible_to |= st.shared_vision[i];
				}
			}
		}
		reveal_sight_at(u->sprite->position, unit_sight_range(u) / 32u, visible_to, u_flying(u));
	}

	void turn_turret(unit_t* u, direction_t turn) {
		if (u->order_target.unit) u_unset_status_flag(u, (unit_t::status_flags_t)0x2000000);
		else {
			if (u->heading == u->subunit->heading) u_set_status_flag(u, (unit_t::status_flags_t)0x2000000);
		}
		if (u_status_flag(u, (unit_t::status_flags_t)0x2000000)) set_unit_heading(u, u->subunit->heading);
		else {
			u->next_velocity_direction = (u->next_velocity_direction + turn);
			u->heading = u->next_velocity_direction;
		}
		if (unit_is(u, UnitTypes::Terran_Goliath_Turret) || unit_is(u, UnitTypes::Hero_Alan_Schezar_Turret)) {
			auto diff = u->subunit->heading - u->heading;
			if (diff > 32_dir) {
				u->heading = u->subunit->heading - 32_dir;
			} else if (diff < -32_dir) {
				u->heading = u->subunit->heading + 32_dir;
			}
		}
	}

	void update_unit_movement(unit_t* u) {

		auto prev_velocity_direction = u->next_velocity_direction;
		bool refresh_vision = execute_movement(u);
		if (refresh_vision) refresh_unit_vision(u);

		if (u_completed(u) && u->subunit && !ut_turret(u)) {
			turn_turret(u->subunit, u->next_velocity_direction - prev_velocity_direction);
			u->subunit->exact_position = u->exact_position;
			u->subunit->position = to_xy(u->exact_position);
			move_sprite(u->subunit->sprite, u->subunit->position);
			set_image_offset(u->subunit->sprite->main_image, get_image_lo_offset(u->sprite->main_image, 2, 0));
			auto ius = make_thingy_setter(iscript_unit, u->subunit);
			if (!u_movement_flag(u, 2)) {
				if (u_status_flag(u->subunit, unit_t::status_flag_turret_walking)) {
					u_unset_status_flag(u->subunit, unit_t::status_flag_turret_walking);
					if (u_can_move(u) && !u_movement_flag(u->subunit, 8)) {
						sprite_run_anim(u->subunit->sprite, iscript_anims::WalkingToIdle);
					}
				}
			} else {
				if (!u_status_flag(u->subunit, unit_t::status_flag_turret_walking)) {
					u_set_status_flag(u->subunit, unit_t::status_flag_turret_walking);
					if (u_can_move(u) && !u_movement_flag(u->subunit, 8)) {
						sprite_run_anim(u->subunit->sprite, iscript_anims::Walking);
					}
				}
			}
			update_unit_movement(u->subunit);
		}
	}

	bool update_thingy_visibility(thingy_t* t, xy size) {
		if (!t->sprite || s_flag(t->sprite, sprite_t::flag_hidden)) return false;
		int tile_from_x = (t->sprite->position.x - size.x / 2) / 32;
		int tile_from_y = (t->sprite->position.y - size.y / 2) / 32;
		size_t tile_to_x = tile_from_x + (size.x + 31) / 32u;
		size_t tile_to_y = tile_from_y + (size.y + 31) / 32u;
		if (tile_from_x < 0) tile_from_x = 0;
		if (tile_from_y < 0) tile_from_y = 0;
		if (tile_to_x > game_st.map_tile_width) tile_to_x = game_st.map_tile_width;
		if (tile_to_y > game_st.map_tile_height) tile_to_y = game_st.map_tile_height;

		if ((size_t)tile_from_x == tile_to_x && (size_t)tile_from_y == tile_to_y) return false;

		uint8_t visibility_flags = 0;
		for (size_t y = tile_from_y; y != tile_to_y; ++y) {
			for (size_t x = tile_from_x; x != tile_to_x; ++x) {
				visibility_flags |= ~st.tiles[y * game_st.map_tile_width + x].visible;
			}
		}
		if (t->sprite->visibility_flags != visibility_flags) {
			set_sprite_visibility(t->sprite, visibility_flags);
			return true;
		}
		return false;
	}

	void update_unit_sprite(unit_t* u) {
		if (update_thingy_visibility(u, u->unit_type->placement_size)) {
			if (u->subunit && !us_hidden(u->subunit)) {
				set_sprite_visibility(u->subunit->sprite, u->sprite->visibility_flags);
			}
		}
	}

	bool unit_can_fire_from_bunker(unit_t* u) {
		if (unit_is_marine(u)) return true;
		if (unit_is_firebat(u)) return true;
		if (unit_is_ghost(u)) return true;
		return false;
	}

	void create_bunker_fire_animation(unit_t* u) {
		const sprite_type_t* sprite;
		size_t index = (direction_index(u->heading) + 16) / 32 & 7;
		direction_t heading;
		if (unit_is_firebat(u)) {
			sprite = get_sprite_type(SpriteTypes::SPRITEID_FlameThrower);
			heading = direction_from_index(16 * ((direction_index(u->heading) + 8) / 16 & 15));
		} else {
			sprite = get_sprite_type(SpriteTypes::SPRITEID_Bunker_Overlay);
			heading = direction_from_index(16 * 2 * index);
		}
		xy offset = get_image_lo_offset(u->connected_unit->sprite->main_image, 0, index);
		auto* t = create_thingy(sprite, u->sprite->position + offset, 0);
		if (t) {
			t->sprite->elevation_level = u->sprite->elevation_level + 1;
			if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
			for (image_t* i : ptr(t->sprite->images)) {
				set_image_heading(i, heading);
			}
		}
	}

	bool try_attack_something(unit_t* u) {
		if (u->order_target.unit) attack_unit_reacquire_target(u);
		if (!u->order_target.unit) return false;
		if (!unit_can_see_target(u, u->order_target.unit)) return false;
		unit_t* target = u->order_target.unit;
		if (!unit_can_attack_target(u, target)) return false;
		u->order_target.pos = target->sprite->position;
		if (unit_turret(u)) return true;

		const weapon_type_t* weapon;
		int cooldown;
		int anim;
		if (target && u_flying(target)) {
			weapon = unit_air_weapon(u);
			cooldown = u->air_weapon_cooldown;
			anim = iscript_anims::AirAttkRpt;
		} else {
			weapon = unit_ground_weapon(u);
			cooldown = u->ground_weapon_cooldown;
			anim = iscript_anims::GndAttkRpt;
		}
		if (!weapon) return false;
		if (cooldown != 0) {
			if (u->order_process_timer > cooldown - 1) u->order_process_timer = cooldown - 1;
			return true;
		}

		if (u_movement_flag(u, 8)) return true;
		int distance = units_distance(unit_main_unit(u), target);
		if (weapon->min_range && distance < weapon->min_range) return false;
		if (distance > weapon_max_range(u, weapon)) return false;

		auto heading_error = fp8::extend(u->heading - xy_direction(target->sprite->position - u->sprite->position)).abs();
		if (heading_error > weapon->attack_angle) {
			if (u_can_turn(u)) {
				u->order_process_timer = 0;
				return true;
			}
			return false;
		}

		if (u_cannot_attack(u)) return false;
		if (u_in_bunker(u)) {
			create_bunker_fire_animation(u);
		}

		u_set_movement_flag(u, 8);
		cooldown = get_modified_weapon_cooldown(u, weapon) + (lcg_rand(12) & 3) - 1;
		u->ground_weapon_cooldown = cooldown;
		u->air_weapon_cooldown = cooldown;
		sprite_run_anim(u->sprite, anim);
		return true;
	}

	void order_hidden_BunkerGuard(unit_t* u) {
		if (!unit_can_fire_from_bunker(u)) return;
		u_set_status_flag(u, unit_t::status_flag_ready_to_attack);
		if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_ready_to_attack);
		if (try_attack_something(u)) {
			set_next_target_waypoint(u, u->order_target.pos);
		} else {
			if (u->main_order_timer == 0) {
				u->main_order_timer = 15;
				u->order_target.unit = find_acquire_random_target(u);
				if (u->order_target.unit) u->order_process_timer = 0;
			}
		}
	}

	void execute_hidden_unit_main_order(unit_t* u) {
		switch (u->order_type->id) {
		case Orders::Die:
			order_Die(u);
			return;
		case Orders::PlayerGuard:
		case Orders::TurretGuard:
		case Orders::TurretAttack:
		case Orders::EnterTransport:
			set_unit_order(u, get_order_type(u_in_bunker(u) ? Orders::BunkerGuard : Orders::Nothing));
			return;
		case Orders::UnusedPowerup:
			error("hidden UnusedPowerup");
			return;
		case Orders::Nothing:
			return;
		case Orders::NothingWait:
			return;
		case Orders::InfestingCommandCenter:
			order_InfestingCommandCenter(u);
			return;
		case Orders::HarvestGas:
			order_HarvestGas(u);
			return;
		case Orders::PowerupIdle:
			error("hidden PowerupIdle");
			return;
		case Orders::NukeLaunch:
			order_NukeLaunch(u);
			return;
		case Orders::ResetCollision:
			order_ResetCollision(u);
			return;
		case Orders::ResetHarvestCollision:
			order_ResetHarvestCollision(u);
			return;
		case Orders::Neutral:
			return;
		case Orders::MedicIdle:
			return;
		case Orders::MedicHeal:
			return;
		default:
			break;
		}
		if (u->order_process_timer) {
			--u->order_process_timer;
			return;
		}
		u->order_process_timer = 8;
		switch (u->order_type->id) {
		case Orders::BunkerGuard:
			order_hidden_BunkerGuard(u);
			break;
		case Orders::EnterTransport:
			error("hidden EnterTransport");
			break;
		case Orders::ComputerAI:
			error("hidden ComputerAI");
			break;
		case Orders::RescuePassive:
			error("hidden RescuePassive");
			break;
		default:
			break;
		}
	}

	void execute_hidden_unit_secondary_order(unit_t* u) {
		switch (u->secondary_order_type->id) {
		case Orders::TrainFighter:
			secondary_order_TrainFighter(u);
			break;
		case Orders::Cloak:
			secondary_order_Cloak(u);
			break;
		case Orders::Decloak:
			secondary_order_Decloak(u);
			break;
		default:
			break;
		}
	}

	void update_hidden_unit(unit_t* u) {
		if (u->subunit && !ut_turret(u)) {
			auto ius = make_thingy_setter(iscript_unit, u->subunit);
			update_hidden_unit(u->subunit);
		}
		execute_movement(u);
		update_unit_values(u);

		execute_hidden_unit_main_order(u);
		execute_hidden_unit_secondary_order(u);

		if (u->sprite) {
			if (!iscript_execute_sprite(u->sprite)) u->sprite = nullptr;
		}
	}

	bool unit_can_detect(const unit_t* u) const {
		if (!ut_detector(u)) return false;
		if (!u_completed(u)) return false;
		if (unit_is_disabled(u)) return false;
		if (u->blinded_by) return false;
		return true;
	}

	uint32_t unit_calculate_detected_flags(const unit_t* u) const {
		if (u->defensive_matrix_hp != 0_fp8 || u->lockdown_timer || u->maelstrom_timer || u->irradiate_timer || u->ensnare_timer || u->stasis_timer || u->plague_timer || u->acid_spore_count) {
			return 0xff;
		} else if (visible_to_everyone(u)) {
			return 0xff;
		}

		uint32_t detected_flags = 0;

		detected_flags |= 1 << u->owner;
		detected_flags |= st.shared_vision[u->owner];
		detected_flags |= u->parasite_flags;

		for (size_t i = 0; i != st.shared_vision.size(); ++i) {
			if (u->parasite_flags & (1 << i)) detected_flags |= st.shared_vision[i];
		}

		auto test = [&](const unit_t* detector) {
			if (!unit_can_detect(detector)) return;
			if (detector == u) return;
			if (u_hallucination(detector)) return;
			if (~u->sprite->visibility_flags & (1 << detector->owner)) return;
			int range = u_grounded_building(detector) ? 32 * 7 : unit_sight_range(detector);
			if (!unit_target_in_range(detector, u, range)) return;
			detected_flags |= (1 << detector->owner) | st.shared_vision[detector->owner] | detector->parasite_flags;
		};
		for (const unit_t* nu : find_units_noexpand(square_at(u->sprite->position, 32 * 11))) {
			test(nu);
		}
		for (const unit_t* nu : ptr(st.map_revealer_units)) {
			test(nu);
		}

		return detected_flags;
	}

	void remove_target_references(unit_t* u, const unit_t* target) {
		auto test = [&](auto*& ref) {
			if (ref == target) {
				ref = nullptr;
				return true;
			} else return false;
		};
		test(u->current_build_unit);
		test(u->order_target.unit);
		test(u->move_target.unit);
		test(u->auto_target_unit);
		test(u->connected_unit);
		test(u->irradiated_by);
		if (ut_worker(u)) {
			test(u->worker.target_resource_unit);
			if (u->worker.is_gathering && test(u->worker.gather_target)) {
				u->worker.is_gathering = false;
			}
		}
		if (u_grounded_building(u)) {
			test(u->building.addon);
		}
		if (unit_is_factory(u)) {
			if (u->building.rally.unit == target) {
				u->building.rally.pos = target->sprite->position;
			}
			test(u->building.rally.unit);
		}
		if (unit_turret(u)) remove_target_references(unit_turret(u), target);
		for (auto i = u->order_queue.begin(); i != u->order_queue.end();) {
			order_t* o = &*i++;
			if (o->target.unit == target) {
				remove_queued_order(u, o);
			}
		}
	}

	void remove_target_references(bullet_t* b, const unit_t* target) {
		if (b->bullet_target == target) b->bullet_target = nullptr;
		if (b->bullet_owner_unit == target) b->bullet_owner_unit = nullptr;
	}

	void update_unit_detected_flags(unit_t* u) {
		uint32_t new_flags = unit_calculate_detected_flags(u);
		if (u->detected_flags == new_flags) return;
		uint32_t old_flags = u->detected_flags;
		if (old_flags == 0x80000000) {
			old_flags = ~new_flags & 0xff;
		}
		uint32_t removed_flags = old_flags & ~new_flags;
		u->detected_flags = new_flags;
		for (size_t i = 0; removed_flags; ++i) {
			if (removed_flags & (1 << i)) {
				removed_flags &= ~(1 << i);
				for (unit_t* nu : ptr(st.player_units.at(i))) {
					remove_target_references(nu, u);
				}
			}
		}
	}

	void update_psionic_matrix() {
		st.update_psionic_matrix = false;
		for (unit_t* u : ptr(st.visible_units)) {
			if (!u_grounded_building(u)) continue;
			if (unit_race(u) != race_t::protoss) continue;
			if (st.players[u->owner].controller == player_t::controller_rescue_passive) continue;
			if (!ut_requires_psionic_matrix(u)) continue;
			bool was_disabled = u_disabled(u);
			if (is_in_psionic_matrix(u->owner, u->sprite->position)) {
				u_unset_status_flag(u, unit_t::status_flag_disabled);
				if (u_completed(u) && was_disabled) {
					sprite_run_anim(u->sprite, iscript_anims::Enable);
					if (unit_is_building_protoss_thing(u) || unit_is_upgrading(u)) sprite_run_anim(u->sprite, iscript_anims::IsWorking);
				}
			} else {
				u_set_status_flag(u, unit_t::status_flag_disabled);
				iscript_run_to_idle(u);
				if (u_completed(u) && !was_disabled) {
					sprite_run_anim(u->sprite, iscript_anims::Disable);
				}
			}
		}
	}

	void update_dead_unit(unit_t* u) {

		if (u->sprite) {
			if (us_hidden(u)) {
				destroy_sprite(u->sprite);
				u->sprite = nullptr;
			} else if (!iscript_execute_sprite(u->sprite)) {
				u->sprite = nullptr;
			}
		}
		if (u_status_flag(u, (unit_t::status_flags_t)0x4000)) {
			error("update_dead_unit: fixme some creep stuff?");
		}
		if (u->sprite) {
			if (update_tiles && st.players[u->owner].controller == player_t::controller_occupied) {
				reveal_sight_at(u->sprite->position, 1, st.shared_vision[u->owner], u_flying(u));
			}
			update_unit_sprite(u);
		} else {
			while (!u->order_queue.empty()) {
				remove_queued_order(u, &u->order_queue.front());
			}
			st.dead_units.remove(*u);
			st.units_container.push(u);
		}
	}

	void update_disruption_web() {
		if (st.disruption_webbed_units) {
			for (unit_t* u : ptr(st.visible_units)) {
				if (!u_flying(u) || u_cannot_attack(u)) {
					if (u_cannot_attack(u)) {
						u_unset_status_flag(u, unit_t::status_flag_cannot_attack);
						--st.disruption_webbed_units;
					}
					if (unit_provides_space(u)) {
						for (unit_t* nu : loaded_units(u)) {
							if (u_cannot_attack(nu)) {
								u_unset_status_flag(nu, unit_t::status_flag_cannot_attack);
								--st.disruption_webbed_units;
							}
						}
					} else if (u->subunit) {
						if (u_cannot_attack(u->subunit)) {
							u_unset_status_flag(u->subunit, unit_t::status_flag_cannot_attack);
							--st.disruption_webbed_units;
						}
					}
				}
			}
		}
		if (st.completed_unit_counts[11][UnitTypes::Spell_Disruption_Web]) {
			for (unit_t* u : ptr(st.player_units[11])) {
				if (!unit_is(u, UnitTypes::Spell_Disruption_Web)) continue;
				for (unit_t* target : find_units_noexpand(unit_sprite_inner_bounding_box(u))) {
					if (u_flying(target)) continue;
					if (!u_cannot_attack(target)) {
						u_set_status_flag(target, unit_t::status_flag_cannot_attack);
						++st.disruption_webbed_units;
					}
					if (unit_provides_space(target)) {
						for (unit_t* n : loaded_units(target)) {
							if (!u_cannot_attack(n)) {
								u_set_status_flag(n, unit_t::status_flag_cannot_attack);
								++st.disruption_webbed_units;
							}
						}
					} else if (target->subunit) {
						if (!u_cannot_attack(target->subunit)) {
							u_set_status_flag(target->subunit, unit_t::status_flag_cannot_attack);
							++st.disruption_webbed_units;
						}
					}
				}
			}
		}
	}

	void update_units() {
		--st.order_timer_counter;
		if (!st.order_timer_counter) {
			st.order_timer_counter = 150;
			int v = 0;
			for (unit_t* u : ptr(st.visible_units)) {
				u->order_process_timer = v;
				++v;
				if (v == 8) v = 0;
			}
		}
		--st.secondary_order_timer_counter;
		if (!st.secondary_order_timer_counter) {
			st.secondary_order_timer_counter = 300;
			int v = 0;
			for (unit_t* u : ptr(st.visible_units)) {
				u->secondary_order_timer = v;
				++v;
				if (v == 30) v = 0;
			}
		}

		update_disruption_web();

		for (auto i = st.dead_units.begin(); i != st.dead_units.end();) {
			unit_t* u = &*i++;
			iscript_flingy = u;
			iscript_unit = u;
			update_dead_unit(u);
		}

		for (unit_t* u : ptr(st.visible_units)) {
			iscript_flingy = u;
			iscript_unit = u;
			update_unit_movement(u);
		}

		if (update_tiles) {
			for (unit_t* u : ptr(st.map_revealer_units)) {
				refresh_unit_vision(u);
			}
		}

		for (unit_t* u : ptr(st.visible_units)) {
			update_unit_sprite(u);
			if (u_cloaked(u) || u_requires_detector(u)) {
				u->cloak_counter = 0;
				if (u->secondary_order_timer) --u->secondary_order_timer;
				else {
					update_unit_detected_flags(u);
					u->secondary_order_timer = 30;
				}
			}
		}

		for (auto i = st.visible_units.begin(); i != st.visible_units.end();) {
			unit_t* u = &*i++;
			iscript_flingy = u;
			iscript_unit = u;
			update_unit(u);
		}

		for (auto i = st.hidden_units.begin(); i != st.hidden_units.end();) {
			unit_t* u = &*i++;
			if (u_cloaked(u) || u_requires_detector(u)) u->cloak_counter = 0;
			iscript_flingy = u;
			iscript_unit = u;
			update_hidden_unit(u);
		}

		for (auto i = st.cloaked_units.begin(); i != st.cloaked_units.end();) {
			unit_t* u = &*i++;
			if (u->cloak_counter == 0) {
				st.cloaked_units.remove(*u);
				u->cloaked_unit_link = {nullptr, nullptr};
				u_unset_status_flag(u, unit_t::status_flag_passively_cloaked);
				decloak_unit(u);
			} else {
				if (!u_burrowed(u) && u->secondary_order_type->id == Orders::Cloak && u->cloak_counter == 1 && u_passively_cloaked(u)) {
					u_unset_status_flag(u, unit_t::status_flag_passively_cloaked);
				}
				if (!u_requires_detector(u)) cloak_unit(u);
			}
		}

		if (st.update_psionic_matrix) update_psionic_matrix();

		st.recent_lurker_hit_current_index = (st.recent_lurker_hit_current_index + 1) % st.recent_lurker_hits.size();
		st.recent_lurker_hits[st.recent_lurker_hit_current_index].clear();

		for (auto i = st.map_revealer_units.begin(); i != st.map_revealer_units.end();) {
			unit_t* u = &*i++;
			iscript_flingy = u;
			iscript_unit = u;
			update_unit(u);
		}

		iscript_flingy = nullptr;
		iscript_unit = nullptr;
	}

	void update_bullets() {

		for (auto i = st.active_bullets.begin(); i != st.active_bullets.end();) {
			bullet_t* b = &*i++;
			iscript_bullet = b;
			iscript_flingy = b;
			if (b->sprite) {
				if (!us_hidden(b)) set_sprite_visibility(b->sprite, tile_visibility(b->sprite->position));
				if (!iscript_execute_sprite(b->sprite)) b->sprite = nullptr;
			}
			if (!b->sprite && b->bullet_state != bullet_t::state_dying) error("non-dying bullet has null sprite");
			bullet_execute(b);
		}
		iscript_bullet = nullptr;
		iscript_flingy = nullptr;

	}

	bool sprite_is_doodad(const sprite_type_t* sprite_type) {
		if (sprite_type->id <= SpriteTypes::SPRITEID_1_6_Badlands11) return true;
		if (sprite_type->id >= SpriteTypes::SPRITEID_7_13_Twilight && sprite_type->id <= SpriteTypes::SPRITEID_11_3_Desert) return true;
		return false;
	}

	void update_thingy(thingy_t* t) {
		if (sprite_is_doodad(t->sprite->sprite_type)) set_sprite_visibility(t->sprite, ~0);
		else if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
		if (!iscript_execute_sprite(t->sprite)) {
			t->sprite = nullptr;
			--st.active_thingies_size;
			st.active_thingies.remove(*t);
			bw_insert_list(st.free_thingies, *t);
		}
	}

	void update_thingies() {
		for (auto i = st.active_thingies.begin(); i != st.active_thingies.end();) {
			thingy_t* t = &*i++;
			update_thingy(t);
		}
	}

	void recede_creep() {
		if (st.creep_life.recede_timer) {
			--st.creep_life.recede_timer;
			return;
		}
		std::array<int, 9> lut{1, 3, 5, 6, 7, 8, 9, 9, 9};
		st.creep_life.recede_timer = lut.at(st.creep_life.free_list_size >> 7);

		for (size_t i = 0; i != st.creep_life.lists.size(); ++i) {
			auto& list = st.creep_life.lists[i];
			auto& size = st.creep_life.lists_size[i];
			if (list.empty()) continue;
			auto it = list.begin();
			std::advance(it, lcg_rand(27, 0, (int)size - 1));
			auto* v = &*it;
			list.remove(*v);
			--size;
			st.creep_life.table.remove(v);
			v->n_neighboring_creep_tiles = 9;
			st.creep_life.free_list.push_front(*v);
			++st.creep_life.free_list_size;

			size_t index = v->tile_pos.y * game_st.map_tile_width + v->tile_pos.x;
			st.tiles[index].flags &= ~tile_t::flag_creep_receding;
			set_tile_creep(v->tile_pos, false);
			break;
		}
	}

	auto active_players() const {
		return make_filter_range(all_player_slots, [this](int n) {
			return player_slot_active(n);
		});
	}

	void player_left(int owner) {
		auto& p = st.players.at(owner);
		st.running_triggers[owner].clear();
		if (p.victory_state) return;
		p.victory_state = 2;
		on_victory_state(owner, 2);
		remove_player(owner);
	}

	void player_dropped(int owner) {
		auto& p = st.players.at(owner);
		st.running_triggers[owner].clear();
		if (p.victory_state) return;
		p.victory_state = 1;
		on_victory_state(owner, 1);
		remove_player(owner);
	}

	void remove_player(int owner) {
		auto& p = st.players.at(owner);
		if (p.controller == player_t::controller_occupied) {
			p.controller = player_t::controller_user_left;
		} else if (p.controller == player_t::controller_computer) {
			p.controller = player_t::controller_computer_defeated;
		}
		for (auto i = st.player_units[owner].begin(); i != st.player_units[owner].end();) {
			unit_t* u = &*i++;
			make_unit_neutral(u);
		}
	}

	void process_frame() {
		recede_creep();

		if (st.update_tiles_countdown == 0) st.update_tiles_countdown = 100;
		--st.update_tiles_countdown;
		update_tiles = st.update_tiles_countdown == 0;

		if (update_tiles) {
			for (auto& v : st.tiles) {
				v.visible = 0xff;
			}
		}

		update_units();
		update_bullets();
		update_thingies();
	}

	void process_triggers() {
		int timer_step = 42;

		for (size_t i = 0; i != 12; ++i) {
			if (!st.trigger_waiting[i]) continue;
			auto& t = st.trigger_wait_timers[i];
			if (t == -1) continue;
			if (t < timer_step) {
				t = 0;
				st.trigger_waiting[i] = false;
				st.trigger_timer = 0;
			} else {
				t -= timer_step;
			}
		}

		if (st.trigger_timer) {
			--st.trigger_timer;
			return;
		}
		st.trigger_timer = 30;

		execute_trigger_struct ets;

		bool any_triggers_executed = false;
		for (int i : active_players()) {
			for (auto& rt : st.running_triggers[i]) {
				if (rt.flags & 8) continue;
				auto& t = *rt.t;
				bool execute_now = true;
				if (~rt.flags & 1) {
					for (auto& c : t.conditions) {
						if (c.type == 0) break;
						if (!test_trigger_condition(c, i)) {
							execute_now = false;
							break;
						}
					}
				}
				if (execute_now) {
					rt.current_action_index = 0;
					execute_trigger(ets, i, rt, t);
					any_triggers_executed = true;
				}
			}
		}

		if (any_triggers_executed) {
			int winners = 0;
			for (int p : active_players()) {
				if (player_won(p)) {
					++winners;
				}
			}
			if (winners) {
				for (int p : active_players()) {
					if (ets.victory_state[p] == 0) ets.victory_state[p] = 2;
				}
				int initially_active_count = 0;
				for (auto& v : st.players) {
					if (v.initially_active) ++initially_active_count;
				}
				if (winners == initially_active_count) {
					for (auto& v : st.players) {
						if (v.initially_active) ets.victory_state.at(&v - st.players.data()) = 5;
					}
				}
			}
			for (int i = 0; i != 8; ++i) {
				int s = ets.victory_state[i];
				if (s != st.players[i].victory_state) {
					st.players[i].victory_state = s;
					on_victory_state(i, s);
					if (s == 2) {
						on_player_eliminated(i);
						remove_player(i);
					}
				}
				if (s && s != 4) {
					st.running_triggers[i].clear();
				}
			}
		}
	}

	void next_frame() {
		++st.current_frame;
		process_frame();
		process_triggers();
	}

	int lcg_rand(int source) {
		++st.random_counts[source];
		++st.total_random_counts;
		st.lcg_rand_state = st.lcg_rand_state * 22695477 + 1;
		return (st.lcg_rand_state >> 16) & 0x7fff;
	}
	int lcg_rand(int source, int from, int to) {
		return from + ((lcg_rand(source) * (to - from + 1)) >> 15);
	}

	void local_unit_status_error(unit_t* u, int err) {
		// todo?
	}

	size_t get_sprite_tile_line_index(int y) {
		if (y < 0) return 0;
		size_t r = y / 32u;
		if (r >= game_st.map_tile_height) return game_st.map_tile_height - 1;
		return r;
	}
	void add_sprite_to_tile_line(sprite_t* sprite) {
		size_t index = get_sprite_tile_line_index(sprite->position.y);
		bw_insert_list(st.sprites_on_tile_line[index], *sprite);
	}
	void remove_sprite_from_tile_line(sprite_t* sprite) {
		size_t index = get_sprite_tile_line_index(sprite->position.y);
		st.sprites_on_tile_line[index].remove(*sprite);
	}

	void move_sprite(sprite_t* sprite, xy new_position) {
		if (sprite->position == new_position) return;
		size_t old_index = get_sprite_tile_line_index(sprite->position.y);
		size_t new_index = get_sprite_tile_line_index(new_position.y);
		sprite->position = new_position;
		if (old_index != new_index) {
			st.sprites_on_tile_line[old_index].remove(*sprite);
			bw_insert_list(st.sprites_on_tile_line[new_index], *sprite);
		}
	}

	void set_sprite_visibility(sprite_t* sprite, int visibility_flags) {
		if (sprite->visibility_flags == visibility_flags) return;
		sprite->visibility_flags = visibility_flags;
	}

	void set_image_offset(image_t* image, xy offset) {
		if (image->offset == offset) return;
		image->offset = offset;
		image->flags |= image_t::flag_redraw;
	}

	void set_image_modifier(image_t* image, int modifier) {
		image->modifier = modifier;
		if (modifier == 17) {
			image->modifier_data1 = 48;
			image->modifier_data2 = 2;
		}
		image->flags |= image_t::flag_redraw;
	}

	void show_image(image_t* image) {
		if (~image->flags&image_t::flag_hidden) return;
		image->flags &= ~image_t::flag_hidden;
		image->flags |= image_t::flag_redraw;
	}

	void hide_image(image_t* image) {
		if (image->flags&image_t::flag_hidden) return;
		image->flags |= image_t::flag_hidden;
	}

	void update_image_special_offset(image_t* image) {
		set_image_offset(image, get_image_lo_offset(image->sprite->main_image, 2, 0));
	}

	void update_image_frame_index(image_t* image) {
		size_t frame_index = image->frame_index_base + image->frame_index_offset;
		if (image->frame_index != frame_index) {
			image->frame_index = frame_index;
			image->flags |= image_t::flag_redraw;
		}
	}

	void set_image_frame_index_offset(image_t* image, size_t frame_index_offset, bool flipped) {
		image->frame_index_offset = frame_index_offset;
		i_set_flag(image, image_t::flag_horizontally_flipped, flipped);
		set_image_modifier(image, image->modifier);
		update_image_frame_index(image);
	}

	void set_image_heading(image_t* image, direction_t heading) {
		if (image->flags & image_t::flag_uses_special_offset) update_image_special_offset(image);
		if (image->flags & image_t::flag_has_directional_frames) {
			size_t frame_index_offset = (direction_index(heading) + 4) / 8;
			bool flipped = false;
			if (frame_index_offset > 16) {
				frame_index_offset = 32 - frame_index_offset;
				flipped = true;
			}
			if (image->frame_index_offset != frame_index_offset || i_flag(image, image_t::flag_horizontally_flipped) != flipped) {
				set_image_frame_index_offset(image, frame_index_offset, flipped);
			}
		}
	}

	void set_image_heading_by_index(image_t* image, size_t frame_index_offset) {
		if (image->flags & image_t::flag_has_directional_frames) {
			bool flipped = false;
			if (frame_index_offset > 16) {
				frame_index_offset = 32 - frame_index_offset;
				flipped = true;
			}
			if (image->frame_index_offset != frame_index_offset || i_flag(image, image_t::flag_horizontally_flipped) != flipped) {
				set_image_frame_index_offset(image, frame_index_offset, flipped);
				if (image->flags & image_t::flag_uses_special_offset) update_image_special_offset(image);
			}
		}
	}

	void set_sprite_images_heading_by_index(sprite_t* sprite, size_t frame_index_offset) {
		for (image_t* i : ptr(sprite->images)) {
			set_image_heading_by_index(i, frame_index_offset);
		}
	}

	void set_sprite_images_heading_by_image_index(sprite_t* sprite, const image_t* image) {
		size_t index = image->frame_index_offset;
		if (i_flag(image, image_t::flag_horizontally_flipped)) index = 32 - index;
		for (image_t* i : ptr(sprite->images)) {
			set_image_heading_by_index(i, index);
		}
	}

	xy get_image_center_map_position(const image_t* image) const {
		return image->sprite->position + image->offset;
	}

	xy get_image_map_position(const image_t* image) const {
		xy map_pos = image->sprite->position + image->offset;
		auto& frame = image->grp->frames.at(image->frame_index);
		if (image->flags & image_t::flag_horizontally_flipped) {
			map_pos.x += int(image->grp->width / 2 - (frame.offset.x + frame.size.x));
		} else {
			map_pos.x += int(frame.offset.x - image->grp->width / 2);
		}
		if (image->flags & image_t::flag_y_frozen) map_pos.y = image->frozen_y_value;
		else map_pos.y += int(frame.offset.y - image->grp->height / 2);
		return map_pos;
	}

	xy get_image_lo_offset(const image_t* image, size_t lo_index, size_t offset_index, bool use_frame_index_offset = false) const {
		size_t frame = use_frame_index_offset ? image->frame_index_offset : image->frame_index;
		auto& lo_offsets = global_st.image_lo_offsets.at((size_t)image->image_type->id);
		if ((size_t)lo_index >= lo_offsets.size()) error("invalid lo index %d\n", lo_index);
		auto& frame_offsets = *lo_offsets[lo_index];
		if ((size_t)frame >= frame_offsets.size()) error("image %d lo_index %d does not have offsets for frame %d (frame_offsets.size() is %d)", (int)image->image_type->id, lo_index, frame, frame_offsets.size());
		if ((size_t)offset_index >= frame_offsets[frame].size()) error("image %d lo_index %d frame %d does not contain an offset at index %d", (int)image->image_type->id, lo_index, frame, offset_index);
		xy r = frame_offsets[frame][offset_index];
		if (image->flags & image_t::flag_horizontally_flipped) r.x = -r.x;
		return r;
	}

	fp8 get_modified_unit_speed(const unit_t* u, fp8 base_speed) const {
		ufp8 speed = base_speed.as_unsigned();
		int mod = 0;
		if (u->stim_timer) ++mod;
		if (u_speed_upgrade(u)) ++mod;
		if (u->ensnare_timer) --mod;
		if (mod < 0) speed /= 2u;
		if (mod > 0) {
			if (unit_is_scout(u)) {
				speed = ufp8::integer(6) + (ufp8::integer(1) - ufp8::integer(1) / 3u);
			} else {
				speed += speed / 2u;
				ufp8 min_speed = ufp8::integer(3) + ufp8::integer(1) / 3u;
				if (speed < min_speed) speed = min_speed;
			}
		}
		return speed.as_signed();
	}

	fp8 get_modified_unit_acceleration(const unit_t* u, fp8 base_acceleration) const {
		ufp8 acceleration = base_acceleration.as_unsigned();
		int mod = 0;
		if (u->stim_timer) ++mod;
		if (u_speed_upgrade(u)) ++mod;
		if (u->ensnare_timer) --mod;
		if (mod < 0) acceleration -= acceleration / 4u;
		if (mod > 0) acceleration *= 2u;
		return acceleration.as_signed();
	}

	fp8 get_modified_unit_turn_rate(const unit_t* u, fp8 base_turn_rate) const {
		ufp8 turn_rate = base_turn_rate.as_unsigned();
		int mod = 0;
		if (u->stim_timer) ++mod;
		if (u_speed_upgrade(u)) ++mod;
		if (u->ensnare_timer) --mod;
		if (mod < 0) turn_rate -= turn_rate / 4u;
		if (mod > 0) turn_rate *= 2u;
		return turn_rate.as_signed();
	}

	int get_modified_weapon_cooldown(const unit_t* u, const weapon_type_t* weapon) const {
		int cooldown = weapon->cooldown;
		if (u->acid_spore_count) {
			cooldown += std::max(cooldown / 8, 3) * u->acid_spore_count;
		}
		int mod = 0;
		if (u->stim_timer) ++mod;
		if (u_cooldown_upgrade(u)) ++mod;
		if (u->ensnare_timer) --mod;
		if (mod > 0) cooldown /= 2;
		if (mod < 0) cooldown += cooldown / 4;
		if (cooldown > 250) cooldown = 250;
		if (cooldown < 5) cooldown = 5;
		return cooldown;
	}

	fp8 unit_halt_distance(const flingy_t* u) const {
		ufp8 speed = u->next_speed.as_unsigned();
		if (speed == ufp8::zero()) return 0_fp8;
		if (u->flingy_movement_type != 0) return 0_fp8;
		if (speed == u->flingy_type->top_speed.as_unsigned() && u->flingy_acceleration == u->flingy_type->acceleration) {
			return u->flingy_type->halt_distance;
		} else {
			return (ufp8::multiply_divide(speed, speed, u->flingy_acceleration.as_unsigned()) / 2u).as_signed();
		}
	}

	void iscript_set_script(image_t* image, int script_id) {
		auto i = global_st.iscript.scripts.find(script_id);
		if (i == global_st.iscript.scripts.end()) {
			error("script %d does not exist", script_id);
		}
		image->iscript_state.current_script = &i->second;
	}

	bool is_spell(const weapon_type_t* weapon_type) {
		auto id = weapon_type->id;
		if (id == WeaponTypes::Spider_Mines) return true;
		if (id == WeaponTypes::Lockdown) return true;
		if (id == WeaponTypes::EMP_Shockwave) return true;
		if (id == WeaponTypes::Irradiate) return true;
		if (id == WeaponTypes::unk_50) return true;
		if (id == WeaponTypes::unk_51) return true;
		if (id == WeaponTypes::Suicide_Infested_Terran) return true;
		if (id == WeaponTypes::Parasite) return true;
		if (id == WeaponTypes::Spawn_Broodlings) return true;
		if (id == WeaponTypes::Ensnare) return true;
		if (id == WeaponTypes::Dark_Swarm) return true;
		if (id == WeaponTypes::Plague) return true;
		if (id == WeaponTypes::Consume) return true;
		if (id == WeaponTypes::unk_68) return true;
		if (id == WeaponTypes::Psi_Assault) return true;
		if (id == WeaponTypes::Scarab) return true;
		if (id == WeaponTypes::Stasis_Field) return true;
		if (id == WeaponTypes::Psionic_Storm) return true;
		if (id == WeaponTypes::Restoration) return true;
		if (id == WeaponTypes::Mind_Control) return true;
		if (id == WeaponTypes::Feedback) return true;
		if (id == WeaponTypes::Optical_Flare) return true;
		if (id == WeaponTypes::Maelstrom) return true;
		return false;
	}

	fp8 weapon_damage_amount(const weapon_type_t* w, int owner) const {
		return fp8::integer(w->damage_amount + w->damage_bonus * player_upgrade_level(owner, w->damage_upgrade));
	}

	fp8 bullet_damage_amount(const bullet_t* b, const unit_t* target) const {
		if (b->weapon_type->hit_type == weapon_type_t::hit_type_nuclear_missile) {
			int damage = max_visible_hp_plus_shields(target) * 2 / 3;
			if (damage < 500) damage = 500;
			return fp8::integer(damage);
		} else {
			return weapon_damage_amount(b->weapon_type, b->owner);
		}
	}

	int unit_sprite_size(unit_type_autocast ut) const {
		if (ut_flag(ut, unit_type_t::flag_sprite_size_medium)) return 1;
		if (ut_flag(ut, unit_type_t::flag_sprite_size_large)) return 2;
		return 0;
	}

	image_t* create_sized_image(unit_t* u, ImageTypes small_image, bool on_subunit = true, int image_order = image_order_top) {
		ImageTypes image = (ImageTypes)((int)small_image + unit_sprite_size(u));
		return create_image(get_image_type(image), (on_subunit ? u->subunit ? u->subunit : u : u)->sprite, {}, image_order);
	}

	void create_defensive_matrix_image(unit_t* u) {
		if (u->defensive_matrix_timer && !u_burrowed(u)) {
			create_sized_image(u, ImageTypes::IMAGEID_Defensive_Matrix_Front_Small);
			create_sized_image(u, ImageTypes::IMAGEID_Defensive_Matrix_Back_Small, true, image_order_below);
		}
	}

	void deal_defensive_matrix_damage(unit_t* u, fp8 damage) {
		if (damage < u->defensive_matrix_hp) {
			u->defensive_matrix_hp -= damage;
		} else {
			u->defensive_matrix_hp = 0_fp8;
			u->defensive_matrix_timer = 0;
			destroy_image_from_to(u, ImageTypes::IMAGEID_Defensive_Matrix_Front_Small, ImageTypes::IMAGEID_Defensive_Matrix_Front_Large);
			destroy_image_from_to(u->sprite, ImageTypes::IMAGEID_Defensive_Matrix_Back_Small, ImageTypes::IMAGEID_Defensive_Matrix_Back_Large);
		}
		if (u->defensive_matrix_timer && !u_burrowed(u)) {
			create_sized_image(u, ImageTypes::IMAGEID_Defensive_Matrix_Hit_Small);
		}
	}

	fp8 unit_armor(const unit_t* u) const {
		fp8 r = fp8::integer(u->unit_type->armor);
		if (unit_is_ultralisk(u) && player_has_upgrade(u->owner, UpgradeTypes::Chitinous_Plating)) r += fp8::integer(2);
		r += fp8::integer(player_upgrade_level(u->owner, u->unit_type->armor_upgrade));
		return r;
	}

	bool on_hit_should_change_target(const unit_t* u, const unit_t* old_target, const unit_t* new_target) const {
		if (old_target == new_target) return false;
		if (!unit_target_is_enemy(u, new_target)) return false;

		if (!old_target) return true;
		if (!unit_can_attack_target(u, old_target)) return true;
		if (!unit_target_is_enemy(u, old_target)) return true;
		if (unit_target_in_weapon_movement_range(u, new_target) && !unit_target_in_weapon_movement_range(u, old_target)) return true;
		return false;
	}

	void unburrow_unit(unit_t* u) {
		if (u->order_type->id != Orders::Burrowing && u->order_type->id != Orders::Burrowed && !unit_is(u, UnitTypes::Zerg_Lurker)) return;
		if (unit_is(u, UnitTypes::Zerg_Lurker)) u_set_status_flag(u, unit_t::status_flag_order_not_interruptible);
		set_unit_order(u, get_order_type(Orders::Unburrowing));
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		order_done(u);
	}

	bool unit_can_move_from_to_region(int owner, const regions_t::region* from, const regions_t::region* to) {
		if (from == to) return true;
		if (owner >= 8) return true;
		if (from->group_index != to->group_index) return false;
		for (auto* r : to->walkable_neighbors) {
			if (r == from) return true;
			if (r->group_index == from->group_index) return true;
		}
		for (auto* r : to->non_walkable_neighbors) {
			if (r == &game_st.regions.regions.front()) continue;
			if (r == to) return true;
		}
		return false;
	}

	void on_hit_change_target(unit_t* u, unit_t* target) {
		if (unit_is(u, UnitTypes::Zerg_Larva)) return;
		if (u_burrowed(u) && !unit_is(u, UnitTypes::Zerg_Lurker) && u->irradiate_timer == 0) {
			if (ut_can_burrow(u)) unburrow_unit(u);
			return;
		}
		if (ut_building(u) || (!ut_worker(u) && unit_can_attack_target(u, target))) {
			u->auto_target_unit = target;
			unit_t* turret = unit_turret(u);
			if (turret) {
				turret->last_attacking_player = u->last_attacking_player;
				turret->auto_target_unit = target;
			}
		} else {
			if (!u_burrowed(u) || !unit_is(u, UnitTypes::Zerg_Lurker)) {
				u = unit_main_unit(u);
				if (u->order_type->unk11 && u_can_move(u)) {
					if (unit_dodge_chance(u) < 255_fp8) {
						auto flee_dir = direction_xy(xy_direction(u->sprite->position - target->sprite->position));
						xy flee_pos = u->sprite->position;
						std::array<int, 5> length = {192, 128, 64, 32, 16};
						auto* r = get_region_at(u->sprite->position);
						for (auto v : length) {
							xy pos = restrict_pos_to_map_bounds(u->sprite->position + to_xy(flee_dir * v));
							if (~u->pathing_flags & 1 || unit_can_move_from_to_region(u->owner, r, get_region_at(pos))) {
								flee_pos = pos;
								break;
							}
						}
						set_unit_order(u, get_order_type(Orders::Move), flee_pos);
					}
				}
			}
		}
	}

	void on_hit_aoe_change_target(unit_t* u, unit_t* target) {
		if (unit_is_arbiter(u)) return;
		if (ut_worker(u)) return;
		if (u_burrowed(u)) return;
		if (!unit_can_attack(u)) return;
		if (on_hit_should_change_target(u, u->auto_target_unit, target)) {
			u->auto_target_unit = target;
			unit_t* turret = unit_turret(u);
			if (turret) {
				turret->last_attacking_player = u->last_attacking_player;
				turret->auto_target_unit = target;
			}
		}
	}

	void on_unit_damage(unit_t* u, unit_t* source_unit, bool reveal_source) {
		if (!source_unit || source_unit->owner == u->owner) return;
		u->last_attacking_player = source_unit->owner;
		if (reveal_source && !unit_target_is_undetected(u, source_unit) && u->owner < 8) {
			reveal_sight_at(source_unit->sprite->position, 1, 1 << u->owner, u_flying(source_unit));
		}
		if (u_in_bunker(source_unit)) {
			source_unit = source_unit->connected_unit;
		}
		if (on_hit_should_change_target(u, u->auto_target_unit, source_unit)) {
			on_hit_change_target(u, source_unit);
		}
		rect bb{u->sprite->position - xy(96, 96), u->sprite->position + xy(96, 96)};
		for (unit_t* n : find_units_noexpand(bb)) {
			if (n != u && n->owner == u->owner) {
				on_hit_aoe_change_target(n, source_unit);
			}
		}
	}

	void unit_deal_damage(unit_t* u, fp8 damage, unit_t* source_unit, int source_owner, bool reveal_source = true) {
		(void)source_owner;
		if (u->hp == 0_fp8) return;
		on_unit_damage(u, source_unit, reveal_source);
		if (damage < u->hp) {
			u->hp -= damage;
			u->air_strength = get_unit_strength(u, false);
			u->ground_strength = get_unit_strength(u, true);
			if (u_completed(u)) {
				update_unit_damage_overlay(u);
			}
			if (source_unit && source_unit->owner != u->owner && reveal_source) {
				// todo: callback for notifications?
			}
		} else {
			if (unit_provides_space(u) && !ut_building(u)) {
				for (unit_t* n : loaded_units(u)) {
					kill_unit(n);
					// todo: units lost scores
				}
			}
			u->hp = 0_fp8;
			kill_unit(u);
			// todo: units lost scores
			if (source_unit && unit_target_is_enemy(source_unit, u)) {
				// todo: kill scores
			}
		}
	}

	void create_shield_damage_effect(unit_t* target, direction_t heading) {
		size_t index = (direction_index(heading) - 124) / 8 % 32;
		xy offset = get_image_lo_offset(target->sprite->main_image, 5, index, true);
		auto* image = create_image(get_image_type(ImageTypes::IMAGEID_Shield_Overlay), target->sprite, offset, image_order_above);
		if (image) set_image_heading_by_index(image, index);
	}

	void weapon_deal_damage(const weapon_type_t* weapon, fp8 damage, int damage_divisor, unit_t* target, direction_t heading, unit_t* source_unit, int source_owner) {
		if (target->hp == 0_fp8) return;
		if (u_invincible(target)) return;
		if (u_hallucination(target)) damage *= 2;
		damage /= damage_divisor;
		damage += fp8::integer(target->acid_spore_count);
		if (damage < 128_fp8) damage = 128_fp8;
		if (target->defensive_matrix_hp != 0_fp8) {
			fp8 defensive_matrix_damage = damage;
			if (damage > target->defensive_matrix_hp) defensive_matrix_damage = target->defensive_matrix_hp;
			deal_defensive_matrix_damage(target, defensive_matrix_damage);
			damage -= defensive_matrix_damage;
		}
		fp8 shield_damage = 0_fp8;
		if (target->unit_type->has_shield) {
			if (target->shield_points >= 256_fp8) {
				if (weapon->damage_type != weapon_type_t::damage_type_ignore_armor) {
					fp8 shield_armor = fp8::integer(player_upgrade_level(target->owner, UpgradeTypes::Protoss_Plasma_Shields));
					if (damage > shield_armor) damage -= shield_armor;
					else damage = 128_fp8;
				}
				shield_damage = damage;
				if (shield_damage > target->shield_points) shield_damage = target->shield_points;
				damage -= shield_damage;
			}
		}
		if (weapon->damage_type != weapon_type_t::damage_type_ignore_armor) {
			fp8 armor = unit_armor(target);
			if (damage > armor) damage -= armor;
			else damage = 0_fp8;
		}
		switch (weapon->damage_type) {
		case weapon_type_t::damage_type_none:
			damage = 0_fp8;
			break;
		case weapon_type_t::damage_type_explosive:
			if (target->unit_type->unit_size == 0) damage = 0_fp8;
			else if (target->unit_type->unit_size == 1) damage *= 128_fp8;
			else if (target->unit_type->unit_size == 2) damage *= 192_fp8;
			break;
		case weapon_type_t::damage_type_concussive:
			if (target->unit_type->unit_size == 0) damage = 0_fp8;
			else if (target->unit_type->unit_size == 2) damage *= 128_fp8;
			else if (target->unit_type->unit_size == 3) damage *= 64_fp8;
			break;
		case weapon_type_t::damage_type_normal:
			break;
		case weapon_type_t::damage_type_ignore_armor:
			break;
		}
		if (shield_damage == 0_fp8 && damage < 128_fp8) damage = 128_fp8;
		unit_deal_damage(target, damage, source_unit, source_owner, weapon->id != WeaponTypes::Irradiate);
		if (shield_damage != 0_fp8) {
			target->shield_points -= shield_damage;
			if (weapon->damage_type != weapon_type_t::damage_type_none && target->shield_points != 0_fp8) {
				create_shield_damage_effect(target, heading);
			}
		}
		target->air_strength = get_unit_strength(target, false);
		target->ground_strength = get_unit_strength(target, true);
	}

	void hallucinated_weapon_hit(const weapon_type_t* weapon, unit_t* target, direction_t heading, unit_t* source_unit) {
		if (target->hp == 0_fp8 || u_invincible(target)) return;
		if (source_unit) {
			on_unit_damage(target, source_unit, weapon->id != WeaponTypes::Irradiate);
			if (weapon->id != WeaponTypes::Irradiate && target->owner != source_unit->owner) {
				// todo: callback for notifications?
			}
			if (weapon->damage_type != weapon_type_t::damage_type_none) {
				if (target->unit_type->has_shield && target->shield_points >= fp8::integer(1)) {
					create_shield_damage_effect(target, heading);
				}
			}
		}
	}

	void bullet_deal_damage(const bullet_t* b, unit_t* target, int damage_divisor) {
		if (b->hit_flags & 2) {
			hallucinated_weapon_hit(b->weapon_type, target, b->heading, b->bullet_owner_unit);
		} else {
			weapon_deal_damage(b->weapon_type, bullet_damage_amount(b, target), damage_divisor, target, b->heading, b->bullet_owner_unit, b->owner);
		}
	}
	void bullet_deal_damage(const bullet_t* b, unit_t* target) {
		bullet_deal_damage(b, target, 1);
	}

	void melee_deal_damage(unit_t* u) {
		unit_t* target = u->order_target.unit;
		if (!target) return;
		auto* w = unit_ground_weapon(u);
		if (u_hallucination(u)) {
			hallucinated_weapon_hit(w, target, u->heading, u);
		} else {
			weapon_deal_damage(w, weapon_damage_amount(w, u->owner), 1, target, u->heading, u, u->owner);
		}
	}

	bool weapon_can_target_unit(const weapon_type_t* weapon, const unit_t* target) const {
		if (!target) return (weapon->target_flags & 0x40) != 0;
		if (u_invincible(target)) return false;
		if (~weapon->target_flags & (u_flying(target) ? 1 : 2)) return false;
		if (weapon->target_flags & 4 && !ut_mechanical(target)) return false;
		if (weapon->target_flags & 8 && !ut_organic(target)) return false;
		if (weapon->target_flags & 0x10 && ut_building(target)) return false;
		if (weapon->target_flags & 0x20 && ut_robotic(target)) return false;
		if (weapon->target_flags & 0x80 && !ut_mechanical(target) && !ut_organic(target)) return false;
		return true;
	}
	bool weapon_can_target_unit(const weapon_type_t* weapon, const unit_t* target, const unit_t* source_unit) const {
		if (!weapon_can_target_unit(weapon, target)) return false;
		if (target && weapon->target_flags & 0x100 && target->owner != source_unit->owner) return false;
		return true;
	}

	template<bool is_air_splash>
	void bullet_deal_splash_damage(bullet_t* b) {
		auto bb = square_at(b->sprite->position, b->weapon_type->outer_splash_radius);
		if (is_air_splash) {
			a_vector<unit_t*> targets;
			for (unit_t* target : find_units(bb)) {
				if (target == b->bullet_owner_unit) continue;
				if (target->owner == b->owner && target != b->bullet_target) continue;
				if (!weapon_can_target_unit(b->weapon_type, target)) continue;
				int distance = unit_distance_to(target, b->sprite->position);
				if (distance > b->weapon_type->outer_splash_radius) continue;
				if (target == b->bullet_target && distance <= b->weapon_type->inner_splash_radius) {
					targets = {target};
					break;
				}
				targets.push_back(target);
			}
			if (!targets.empty()) {
				size_t random_index = 0;
				if (targets.size() != 1) random_index = lcg_rand(58) % targets.size();
				bullet_deal_damage(b, targets[random_index]);
			}
		}
		bool damages_allies = !is_air_splash && b->weapon_type->hit_type != weapon_type_t::hit_type_enemy_splash;
		for (unit_t* target : find_units(bb)) {
			if (target == b->bullet_owner_unit && b->weapon_type->id != WeaponTypes::Psionic_Storm) continue;
			if (!damages_allies && target->owner == b->owner && target != b->bullet_target) continue;
			if (!weapon_can_target_unit(b->weapon_type, target)) continue;
			int distance = unit_distance_to(target, b->sprite->position);
			if (distance > b->weapon_type->outer_splash_radius) continue;
			if (b->weapon_type->id == WeaponTypes::Psionic_Storm) {
				if (target->storm_timer) continue;
				target->storm_timer = 1;
			}
			if (is_air_splash) {
				if (target == b->bullet_target) continue;
				if (unit_is(target, UnitTypes::Protoss_Interceptor)) continue;
			}
			if (!is_air_splash && distance <= b->weapon_type->inner_splash_radius) {
				bullet_deal_damage(b, target, 1);
			} else if (!u_burrowed(target)) {
				if (distance <= b->weapon_type->medium_splash_radius) {
					bullet_deal_damage(b, target, 2);
				} else {
					bullet_deal_damage(b, target, 4);
				}
			}
		}
	}

	void irradiate_unit(unit_t* u, unit_t* source_unit, int source_owner) {
		if (u->irradiate_timer == 0 && !u_burrowed(u)) {
			create_sized_image(u, ImageTypes::IMAGEID_Irradiate_Small);
		}
		u->irradiate_timer = get_weapon_type(WeaponTypes::Irradiate)->cooldown;
		u->irradiated_by = source_unit;
		u->irradiate_owner = source_owner;
	}

	void return_interceptors(unit_t* u) {
		if (!unit_is_carrier(u)) return;
		for (unit_t* n : ptr(u->carrier.outside_units)) {
			set_unit_order(n, get_order_type(Orders::InterceptorReturn));
		}
	}

	void set_unit_disabled(unit_t* u) {
		if (u->subunit) u_set_status_flag(u->subunit, unit_t::status_flag_disabled);
		stop_unit(u);
		if (unit_is_ghost(u) && u->connected_unit && unit_is(u->connected_unit, UnitTypes::Terran_Nuclear_Missile)) {
			u->connected_unit->connected_unit = nullptr;
			u->connected_unit = nullptr;
		}
		if (unit_is_carrier(u)) return_interceptors(u);
		if (u->order_type->id == Orders::Repair || u->order_type->id == Orders::DroneLand) u->order_state = 0;
		if (!u->order_type->unk9) u->order_target.unit = nullptr;
		iscript_run_to_idle(u);
	}

	void lockdown_unit(unit_t* u) {
		if (u->lockdown_timer == 0) {
			create_sized_image(u, ImageTypes::IMAGEID_Lockdown_Field_Small);
		}
		if (u->lockdown_timer < 131) u->lockdown_timer = 131;
		set_unit_disabled(u);
	}

	void blind_unit(unit_t* u, int source_owner) {
		if (u_hallucination(u)) {
			kill_unit(u);
			return;
		}
		u->blinded_by |= 1 << source_owner;
		play_sound(1019, u);
		create_sized_image(u, ImageTypes::IMAGEID_Optical_Flare_Hit_Small);
		st.update_tiles_countdown = 1;
	}

	void restore_unit(unit_t* u) {
		if (u_hallucination(u)) {
			kill_unit(u);
			return;
		}
		create_sized_image(u, ImageTypes::IMAGEID_Restoration_Hit_Small);
		u->parasite_flags = 0;
		u->blinded_by = 0;
		if (u->ensnare_timer) remove_ensnare(u);
		if (u->plague_timer) remove_plague(u);
		if (u->irradiate_timer) remove_irradiate(u);
		if (u->lockdown_timer) remove_lockdown(u);
		if (u->maelstrom_timer) remove_maelstrom(u);
		if (u->acid_spore_count) remove_acid_spores(u);
	}

	void emp_shockwave(xy position, const unit_t* source_unit) {
		int range = get_weapon_type(WeaponTypes::EMP_Shockwave)->inner_splash_radius;
		for (unit_t* target : find_units_noexpand(square_at(position, range))) {
			if (target == source_unit) continue;
			if (source_unit && target == source_unit->subunit) continue;
			if (u_hallucination(target)) {
				kill_unit(target);
				continue;
			}
			if (target->stasis_timer) continue;
			target->energy = 0_fp8;
			target->shield_points = 0_fp8;
		}
	}

	void spawn_broodlings(unit_t* target, const unit_t* source_unit) {
		if (!u_hallucination(target)) {
			const unit_type_t* broodling_type = get_unit_type(UnitTypes::Zerg_Broodling);
			auto spawn = [&](xy pos) {
				if (!unit_type_can_fit_at(broodling_type, target->sprite->position)) {
					if (!find_unit_placement(source_unit, pos, {pos - xy(32, 32), pos + xy(32, 32)}, false).first) return;
				}
				pos = restrict_move_target_to_valid_bounds(broodling_type, pos);
				unit_t* broodling = create_unit(broodling_type, pos, source_unit->owner);
				if (broodling) {
					finish_building_unit(broodling);
					complete_unit(broodling);
					set_remove_timer(broodling);
				} else display_last_error_for_player(source_unit->owner);
			};
			spawn(target->sprite->position - xy(2, 2));
			spawn(target->sprite->position + xy(2, 2));
		}
		// todo: increment scores and kill count
		kill_unit(target);
	}

	void ensnare_unit(unit_t* u) {
		if (!u->ensnare_timer && !u_burrowed(u)) {
			create_sized_image(u, ImageTypes::IMAGEID_Ensnare_Overlay_Small);
		}
		u->ensnare_timer = 75;
		update_unit_speed(u);
	}

	void ensnare(xy pos, const unit_t* source_unit) {
		thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Ensnare), pos, 0);
		t->sprite->elevation_level = 19;
		if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
		for (unit_t* target : find_units_noexpand(square_at(pos, 64))) {
			if (target == source_unit) continue;
			if (ut_building(target)) continue;
			if (u_invincible(target)) continue;
			if (u_burrowed(target)) continue;
			ensnare_unit(target);
		}
	}

	void consume_unit(unit_t* target, unit_t* source_unit) {
		if (!target || u_invincible(target)) return;
		if (!unit_tech_target_valid(source_unit, get_tech_type(TechTypes::Consume), target)) return;
		// todo: scores
		kill_unit(target);
		if (!u_hallucination(target)) {
			source_unit->energy = std::min(source_unit->energy + fp8::integer(50), unit_max_energy(source_unit));
		}
	}

	void dark_swarm(xy pos, int owner) {
		const unit_type_t* unit_type = get_unit_type(UnitTypes::Spell_Dark_Swarm);
		pos = restrict_move_target_to_valid_bounds(unit_type, pos);
		unit_t* u = create_unit(unit_type, pos, 11);
		if (u) {
			u_set_status_flag(u, unit_t::status_flag_no_collide);
			u->sprite->elevation_level = 11;
			finish_building_unit(u);
			complete_unit(u);
			set_remove_timer(u, 900);
		} else display_last_error_for_player(owner);
	}

	void plague_unit(unit_t* u) {
		if (!u->plague_timer && !u_burrowed(u)) {
			create_sized_image(u, ImageTypes::IMAGEID_Plague_Overlay_Small);
		}
		u->plague_timer = 75;
	}

	void plague(xy pos, unit_t* source_unit) {
		for (unit_t* target : find_units_noexpand(square_at(pos, 64))) {
			if (target == source_unit) continue;
			if (u_invincible(target)) continue;
			if (u_burrowed(target)) continue;
			plague_unit(target);
			if (source_unit) on_unit_damage(target, source_unit, true);
		}
	}

	void parasite_unit(unit_t* u, int owner) {
		u->parasite_flags |= 1 << owner;
	}

	void stasis_unit(unit_t* u) {
		if (u->stasis_timer == 0) {
			create_sized_image(u, ImageTypes::IMAGEID_Stasis_Field_Small);
			u_set_status_flag(u, unit_t::status_flag_invincible);
		}
		if (u->stasis_timer < 131) u->stasis_timer = 131;
		set_unit_disabled(u);
	}

	void stasis_field(xy pos, unit_t* source_unit) {
		thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Stasis_Field_Hit), pos, 0);
		if (t) {
			t->sprite->elevation_level = 17;
			if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
		}
		for (unit_t* target : find_units_noexpand(square_at(pos, 48))) {
			if (target == source_unit) continue;
			if (u_invincible(target)) continue;
			if (u_burrowed(target)) continue;
			if (ut_building(target)) continue;
			stasis_unit(target);
		}
	}

	void maelstrom_unit(unit_t* u) {
		if (u->maelstrom_timer == 0) {
			create_sized_image(u, ImageTypes::IMAGEID_Maelstorm_Overlay_Small);
		}
		if (u->maelstrom_timer < 22) u->maelstrom_timer = 22;
		set_unit_disabled(u);
	}

	void maelstrom(xy pos, unit_t* source_unit) {
		thingy_t* t = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Maelstrom_Hit), pos, 0);
		if (t) {
			t->sprite->elevation_level = 17;
			if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
		}
		play_sound(1064, source_unit);
		for (unit_t* target : find_units_noexpand(square_at(pos, 48))) {
			if (u_hallucination(target)) {
				kill_unit(target);
				continue;
			}
			if (target == source_unit) continue;
			if (u_invincible(target)) continue;
			if (u_burrowed(target)) continue;
			if (ut_building(target)) continue;
			if (!ut_organic(target)) continue;
			maelstrom_unit(target);
		}
	}

	void disruption_web(xy pos, int owner) {
		const unit_type_t* unit_type = get_unit_type(UnitTypes::Spell_Disruption_Web);
		pos = restrict_move_target_to_valid_bounds(unit_type, pos);
		unit_t* u = create_unit(unit_type, pos, 11);
		if (u) {
			u_set_status_flag(u, unit_t::status_flag_no_collide);
			u->sprite->elevation_level = 11;
			finish_building_unit(u);
			complete_unit(u);
			set_remove_timer(u, 360);
		} else display_last_error_for_player(owner);
	}

	ImageTypes acid_spore_image(const unit_t* u) const {
		size_t n = std::min((size_t)(u->acid_spore_count / 2), (size_t)3);
		return (ImageTypes)((int)ImageTypes::IMAGEID_Acid_Spores_1_Overlay_Small + 4 * unit_sprite_size(u) + n);
	}

	void update_acid_spore_image(unit_t* u) {
		ImageTypes image_id = acid_spore_image(u);
		if (!find_image(u, image_id, image_id)) {
			destroy_image_from_to(u, ImageTypes::IMAGEID_Acid_Spores_1_Overlay_Small, ImageTypes::IMAGEID_Acid_Spores_6_9_Overlay_Large);
			create_image(get_image_type(image_id), (u->subunit ? u->subunit : u)->sprite, {}, image_order_top);
		}
	}

	void add_acid_spore(unit_t* u) {
		if (u->acid_spore_count < 9) ++u->acid_spore_count;
		update_acid_spore_image(u);
		*get_best_score(u->acid_spore_time, identity()) = 150;
	}

	void add_acid_spore(xy pos, int owner) {
		for (unit_t* target : find_units_noexpand(square_at(pos, 64))) {
			if (target->owner == owner) continue;
			if (ut_building(target)) continue;
			if (!u_flying(target)) continue;
			if (u_invincible(target)) continue;
			if (unit_is_undetected(target, owner)) continue;
			add_acid_spore(target);
		}
	}

	void bullet_hit(bullet_t* b) {
		switch (b->weapon_type->hit_type) {
		case weapon_type_t::hit_type_none:
			break;
		case weapon_type_t::hit_type_normal:
			if (b->bullet_target && ~b->hit_flags & 1) {
				int div = 1;
				if (b->weapon_type->bullet_type == weapon_type_t::bullet_type_bounce) {
					int bounces = 2 - b->remaining_bounces;
					for (int i = 0; i < bounces; ++i) {
						div *= 3;
					}
				}
				bullet_deal_damage(b, b->bullet_target, div);
			}
			break;
		case weapon_type_t::hit_type_radial_splash:
		case weapon_type_t::hit_type_enemy_splash:
		case weapon_type_t::hit_type_nuclear_missile:
			if (b->weapon_type->id == WeaponTypes::Subterranean_Spines) {
				for (unit_t* target : find_units(square_at(b->sprite->position, b->weapon_type->outer_splash_radius))) {
					if (target == b->bullet_owner_unit) continue;
					if (target->owner == b->owner) continue;
					if (!weapon_can_target_unit(b->weapon_type, target)) continue;
					if (unit_distance_to(target, b->sprite->position) > b->weapon_type->inner_splash_radius) continue;
					if (b->bullet_owner_unit) {
						bool found = false;
						for (auto& arr : st.recent_lurker_hits) {
							if (std::find(arr.begin(), arr.end(), std::make_pair(b->bullet_owner_unit->index, target->index)) != arr.end()) {
								found = true;
								break;
							}
						}
						if (found) continue;
						auto& arr = st.recent_lurker_hits[st.recent_lurker_hit_current_index];
						if (arr.size() != 16) {
							arr.emplace_back(b->bullet_owner_unit->index, target->index);
						}
					}
					bullet_deal_damage(b, target);
				}
			} else {
				bullet_deal_splash_damage<false>(b);
			}
			break;
		case weapon_type_t::hit_type_lockdown:
			if (b->bullet_target && !unit_dying(b->bullet_target)) lockdown_unit(b->bullet_target);
			break;
		case weapon_type_t::hit_type_parasite:
			if (b->bullet_target && !unit_dying(b->bullet_target)) {
				play_sound(921, b->bullet_target);
				parasite_unit(b->bullet_target, b->owner);
			}
			break;
		case weapon_type_t::hit_type_broodlings:
			if (b->bullet_target && b->bullet_owner_unit && !unit_dying(b->bullet_target)) spawn_broodlings(b->bullet_target, b->bullet_owner_unit);
			break;
		case weapon_type_t::hit_type_emp_shockwave:
			emp_shockwave(b->sprite->position, b->bullet_owner_unit);
			break;
		case weapon_type_t::hit_type_irradiate:
			if (b->bullet_target && !unit_dying(b->bullet_target)) {
				play_sound(351, b->bullet_target);
				irradiate_unit(b->bullet_target, b->bullet_owner_unit, b->owner);
			}
			break;
		case weapon_type_t::hit_type_ensnare:
			ensnare(b->sprite->position, b->bullet_owner_unit);
			break;
		case weapon_type_t::hit_type_plague:
			plague(b->bullet_target_pos, b->bullet_owner_unit);
			break;
		case weapon_type_t::hit_type_stasis_field:
			if (b->bullet_target_pos != xy()) stasis_field(b->bullet_target_pos, b->bullet_owner_unit);
			break;
		case weapon_type_t::hit_type_dark_swarm:
			dark_swarm(b->bullet_target_pos, b->owner);
			break;
		case weapon_type_t::hit_type_yamato_gun:
			if (b->bullet_target) bullet_deal_damage(b, b->bullet_target);
			break;
		case weapon_type_t::hit_type_restoration:
			if (b->bullet_target) {
				play_sound(998, b->bullet_target);
				restore_unit(b->bullet_target);
			}
			break;
		case weapon_type_t::hit_type_optical_flare:
			if (b->bullet_target && !unit_dying(b->bullet_target)) blind_unit(b->bullet_target, b->owner);
			break;
		case weapon_type_t::hit_type_air_splash:
			bullet_deal_splash_damage<true>(b);
			break;
		case weapon_type_t::hit_type_consume:
			if (b->bullet_target && b->bullet_owner_unit && !unit_dying(b->bullet_target)) consume_unit(b->bullet_target, b->bullet_owner_unit);
			break;
		case weapon_type_t::hit_type_disruption_web:
			disruption_web(b->bullet_target_pos, b->owner);
			break;
		case weapon_type_t::hit_type_corrosive_acid:
			if (b->bullet_target && ~b->hit_flags & 1) {
				bullet_deal_damage(b, b->bullet_target);
				if (~b->hit_flags & 2) {
					add_acid_spore(b->sprite->position, b->owner);
				}
			}
			break;
		case weapon_type_t::hit_type_maelstrom:
			if (b->bullet_target_pos != xy()) maelstrom(b->bullet_target_pos, b->bullet_owner_unit);
			break;
		default: error("unknown bullet hit type %d", b->weapon_type->hit_type);
		}
	}

	void bullet_kill(bullet_t* b) {
		b->bullet_state = bullet_t::state_dying;
		sprite_run_anim(b->sprite, iscript_anims::Death);
	}

	void bullet_move(bullet_t* b, execute_movement_struct& ems) {
		update_unit_movement_values(b, ems);
		finish_flingy_movement(b, ems);
		if (!is_in_bounds(b->position, map_bounds())) b->remaining_time = 0;
		xy pos = restrict_pos_to_map_bounds(b->position);
		move_sprite(b->sprite, pos);
		if (b->position != pos) {
			b->position = pos;
			b->exact_position = to_xy_fp8(pos);
		}
	}

	void set_bullet_move_target(bullet_t* b, xy target) {
		set_next_target_waypoint(b, target);
		set_flingy_move_target(b, target);
	}

	bool bullet_state_init(bullet_t* b, execute_movement_struct& ems) {
		if (~b->order_signal & 1) return false;
		b->order_signal &= ~1;
		switch (b->weapon_type->bullet_type) {
		case weapon_type_t::bullet_type_fly:
		case weapon_type_t::bullet_type_extend_to_max_range:
			b->bullet_state = bullet_t::state_move;
			sprite_run_anim(b->sprite, iscript_anims::GndAttkInit);
			break;
		case weapon_type_t::bullet_type_appear_at_target_unit:
		case weapon_type_t::bullet_type_appear_at_target_pos:
		case weapon_type_t::bullet_type_appear_at_source_unit:
		case weapon_type_t::bullet_type_self_destruct:
			bullet_kill(b);
			break;
		case weapon_type_t::bullet_type_follow_target:
			b->bullet_state = bullet_t::state_follow;
			sprite_run_anim(b->sprite, iscript_anims::GndAttkInit);
			break;
		case weapon_type_t::bullet_type_persist_at_target_pos:
			b->bullet_state = bullet_t::state_damage_over_time;
			sprite_run_anim(b->sprite, iscript_anims::SpecialState2);
			break;
		case weapon_type_t::bullet_type_bounce:
			b->bullet_state = bullet_t::state_bounce;
			sprite_run_anim(b->sprite, iscript_anims::GndAttkInit);
			break;
		case weapon_type_t::bullet_type_attack_target_pos:
			b->bullet_state = bullet_t::state_hit_near_target;
			sprite_run_anim(b->sprite, iscript_anims::GndAttkInit);
			break;
		default: error("unknown bullet type %d", b->weapon_type->bullet_type);
		}
		return true;
	}

	bool bullet_state_dying(bullet_t* b, execute_movement_struct& ems) {
		if (b->sprite) return false;
		--st.active_bullets_size;
		st.active_bullets.remove(*b);
		st.bullets_container.push(b);
		return false;
	}

	bool bullet_state_move(bullet_t* b, execute_movement_struct& ems) {
		bullet_move(b, ems);
		if (b->remaining_time-- == 0 || b->position == b->move_target.pos) {
			bullet_kill(b);
		}
		return false;
	}

	bool bullet_state_follow(bullet_t* b, execute_movement_struct& ems) {
		unit_t* target = b->bullet_target;
		if (target) {
			if (~b->hit_flags & 1) set_bullet_move_target(b, target->sprite->position);
			else b->bullet_state = bullet_t::state_move;
		} else {
			b->bullet_state = bullet_t::state_move;
			b->bullet_target = nullptr;
		}
		return bullet_state_move(b, ems);
	}

	bool bullet_state_hit_near_target(bullet_t* b, execute_movement_struct& ems) {
		unit_t* target = b->bullet_target;
		if (target) {
			if (~b->hit_flags & 1) {
				set_flingy_move_target(b, restrict_pos_to_map_bounds(target->sprite->position + hit_near_target_positions[b->hit_near_target_position_index]));
				set_next_target_waypoint(b, b->move_target.pos);
			} else b->bullet_state = bullet_t::state_move;
		} else {
			b->bullet_state = bullet_t::state_move;
			b->bullet_target = nullptr;
		}
		return bullet_state_move(b, ems);
	}

	bool bullet_state_bounce(bullet_t* b, execute_movement_struct& ems) {
		unit_t* target = b->bullet_target;
		if (target && ~b->hit_flags & 1) {
			set_flingy_move_target(b, target->sprite->position);
			set_next_target_waypoint(b, b->move_target.pos);
		}
		bullet_move(b, ems);
		if (unit_is_at_move_target(b)) {
			--b->remaining_bounces;
			unit_t* new_target = nullptr;
			if (b->remaining_bounces) {
				if (b->bullet_owner_unit) {
					new_target = find_unit_noexpand(square_at(b->sprite->position, 96), [&](unit_t* u) {
						if (!unit_can_attack_target(b->bullet_owner_unit, u)) return false;
						if (!unit_target_is_enemy(b->bullet_owner_unit, u)) return false;
						if (u == target || u == b->prev_bounce_unit) return false;
						return true;
					});
				}
				b->prev_bounce_unit = target;
			}
			if (new_target) {
				sprite_run_anim(b->sprite, iscript_anims::SpecialState1);
				b->bullet_target = new_target;
			} else {
				bullet_kill(b);
			}
		}
		return false;
	}

	bool bullet_state_damage_over_time(bullet_t* b, execute_movement_struct& ems) {
		if (b->remaining_time-- == 0) {
			bullet_kill(b);
		} else {
			if (b->remaining_time % 7 == 0) bullet_hit(b);
		}
		return false;
	}

	void bullet_execute(bullet_t* b) {
		execute_movement_struct ems;
		while (true) {
			bool cont = false;
			switch (b->bullet_state) {
			case bullet_t::state_init:
				cont = bullet_state_init(b, ems);
				break;
			case bullet_t::state_move:
				cont = bullet_state_move(b, ems);
				break;
			case bullet_t::state_follow:
				cont = bullet_state_follow(b, ems);
				break;
			case bullet_t::state_bounce:
				cont = bullet_state_bounce(b, ems);
				break;
			case bullet_t::state_damage_over_time:
				cont = bullet_state_damage_over_time(b, ems);
				break;
			case bullet_t::state_dying:
				cont = bullet_state_dying(b, ems);
				break;
			case bullet_t::state_hit_near_target:
				cont = bullet_state_hit_near_target(b, ems);
				break;
			default: error("unknown bullet state %d", b->bullet_state);
			}
			if (!cont) break;
		}
	}

	bool unit_is_under_dark_swarm(const unit_t* u) const {
		if (ut_building(u)) return false;
		if (st.completed_unit_counts[11][UnitTypes::Spell_Dark_Swarm] == 0) return false;
		return find_unit(unit_sprite_inner_bounding_box(u), [&](const unit_t* n) {
			return unit_is(n, UnitTypes::Spell_Dark_Swarm);
		}) != nullptr;
	}

	fp8 unit_dodge_chance(const unit_t* u) const {
		if (u_flying(u)) return 0_fp8;
		if (unit_is_under_dark_swarm(u)) return 255_fp8;
		if (st.tiles[tile_index(u->sprite->position)].flags & tile_t::flag_provides_cover) return 119_fp8;
		return 0_fp8;
	}

	fp8 unit_target_miss_chance(const unit_t* u, const unit_t* target) const {
		fp8 r = unit_dodge_chance(target);
		if (!u_flying(u) && !u_flying(target)) {
			if (get_ground_height_at(target->sprite->position) > get_ground_height_at(u->sprite->position)) {
				if (r < 119_fp8) r = 119_fp8;
			}
		}
		return r;
	}

	bool initialize_bullet(bullet_t* b, const weapon_type_t* weapon_type, unit_t* source_unit, xy pos, int owner, direction_t heading) {
		const flingy_type_t* flingy_type = weapon_type->flingy;
		if (!flingy_type) error("attempt to create bullet with null flingy");
		if (!initialize_flingy(b, flingy_type, pos, owner, heading)) return false;

		b->movement_flags |= 8;
		b->bullet_state = 0;
		b->bullet_target = nullptr;
		b->order_signal = 0;
		b->weapon_type = weapon_type;
		b->remaining_time = weapon_type->lifetime;
		b->hit_flags = 0;
		b->remaining_bounces = 0;
		b->owner = owner;

		if (weapon_type->bullet_heading_offset != 0_dir) {
			bool clockwise;
			if (source_unit == st.prev_bullet_source_unit) clockwise = !st.prev_bullet_heading_offset_clockwise;
			else clockwise = lcg_rand((int)weapon_type->id) & 1;
			direction_t heading_offset = weapon_type->bullet_heading_offset;
			if (!clockwise) heading_offset = -heading_offset;
			b->next_velocity_direction += heading_offset;
			b->heading = b->next_velocity_direction;
			st.prev_bullet_source_unit = source_unit;
			st.prev_bullet_heading_offset_clockwise = clockwise;
		}

		unit_t* bullet_owner_unit = source_unit;
		if (ut_turret(source_unit)) bullet_owner_unit = source_unit->subunit;
		if (unit_is(source_unit, UnitTypes::Protoss_Scarab)) bullet_owner_unit = source_unit->fighter.parent;
		b->bullet_owner_unit = bullet_owner_unit;

		if (u_hallucination(source_unit)) b->hit_flags |= 2;
		b->prev_bounce_unit = nullptr;

		unit_t* target_unit = source_unit->order_target.unit;
		xy target_pos;
		if (target_unit) {
			b->sprite->elevation_level = target_unit->sprite->elevation_level + 1;
			b->bullet_target = target_unit;
			target_pos = target_unit->sprite->position;
		} else {
			b->sprite->elevation_level = source_unit->sprite->elevation_level + 1;
			b->bullet_target = nullptr;
			target_pos = source_unit->order_target.pos;
		}
		switch (weapon_type->bullet_type) {
		case weapon_type_t::bullet_type_fly:
		case weapon_type_t::bullet_type_follow_target:
		case weapon_type_t::bullet_type_bounce:
			if (weapon_type->bullet_type == weapon_type_t::bullet_type_bounce) b->remaining_bounces = 3;
			if (target_unit && bullet_owner_unit && fp8::from_raw(lcg_rand(1) & 0xff) <= unit_target_miss_chance(bullet_owner_unit, target_unit)) {
				b->hit_flags |= 1;
				target_pos -= to_xy(direction_xy(heading, 30));
			}
			set_flingy_move_target(b, target_pos);
			break;
		case weapon_type_t::bullet_type_appear_at_target_unit:
		case weapon_type_t::bullet_type_appear_at_target_pos:
			if (target_unit && bullet_owner_unit && fp8::from_raw(lcg_rand(1) & 0xff) <= unit_target_miss_chance(bullet_owner_unit, target_unit)) {
				b->hit_flags |= 1;
				xy pos = b->sprite->position - to_xy(direction_xy(b->heading, 30));
				b->exact_position = to_xy_fp8(pos);
				b->position = pos;
				move_sprite(b->sprite, pos);
			}
			break;
		case weapon_type_t::bullet_type_persist_at_target_pos:
			b->exact_position = to_xy_fp8(target_pos);
			b->position = target_pos;
			move_sprite(b->sprite, target_pos);
			break;
		case weapon_type_t::bullet_type_appear_at_source_unit:
			break;
		case weapon_type_t::bullet_type_self_destruct:
			if (b->bullet_owner_unit) {
				u_set_status_flag(b->bullet_owner_unit, unit_t::status_flag_lifetime_expired);
				b->bullet_owner_unit->user_action_flags |= 4;
				kill_unit(b->bullet_owner_unit);
			}
			break;
		case weapon_type_t::bullet_type_attack_target_pos:
			b->hit_near_target_position_index = source_unit->next_hit_near_target_position_index;
			if (source_unit->next_hit_near_target_position_index == 13) source_unit->next_hit_near_target_position_index = 0;
			else ++source_unit->next_hit_near_target_position_index;
			set_flingy_move_target(b, target_pos + hit_near_target_positions[b->hit_near_target_position_index]);
			break;
		case weapon_type_t::bullet_type_extend_to_max_range:
			target_pos = source_unit->sprite->position + to_xy(direction_xy(b->heading, b->weapon_type->max_range + 20));
			set_flingy_move_target(b, target_pos);
			break;
		default: error("unknown bullet_type %d", weapon_type->bullet_type);
		}
		b->bullet_target_pos = target_pos;
		return true;
	}

	bullet_t* create_bullet(const weapon_type_t* weapon_type, unit_t* source_unit, xy pos, int owner, direction_t heading) {
		if (weapon_type->id == WeaponTypes::Halo_Rockets && st.active_bullets_size >= 80) return nullptr;
		if (u_cannot_attack(source_unit) && !is_spell(weapon_type)) return nullptr;
		bullet_t* b = st.bullets_container.top();
		if (!b) return nullptr;
		if (!initialize_bullet(b, weapon_type, source_unit, pos, owner, heading)) {
			return nullptr;
		}
		st.bullets_container.pop();
		++st.active_bullets_size;
		bw_insert_list(st.active_bullets, *b);
		return b;
	}

	xy get_bullet_appear_at_target_pos(const unit_t* u, const unit_t* target) const {
		auto target_bb = unit_sprite_inner_bounding_box(u->order_target.unit);
		int margin_w = (target_bb.to.x - target_bb.from.x) / 4;
		int margin_h = (target_bb.to.y - target_bb.from.y) / 4;
		rect bb = target_bb + rect{{margin_w, margin_h}, {-margin_w, -margin_h}};
		xy a = bb.from + xy((bb.to.x - bb.from.x) / 2, (bb.to.y - bb.from.y) / 2);
		xy b = u->sprite->position;
		if (get_unique_sided_positions_within_bounds(a, b, bb)) return b;
		else return a;
	}

	void fire_weapon(unit_t* u, const weapon_type_t* weapon_type, int forward_offset = -1) {
		if (!weapon_type->flingy) return;
		xy pos;
		if (weapon_type->bullet_type == weapon_type_t::bullet_type_appear_at_target_unit) {
			if (!u->order_target.unit) return;
			pos = get_bullet_appear_at_target_pos(u, u->order_target.unit);
		} else {
			if (weapon_type->bullet_type == weapon_type_t::bullet_type_appear_at_target_pos) {
				pos = u->order_target.pos;
			} else {
				pos = u->sprite->position + to_xy(direction_xy(u->heading, forward_offset == -1 ? weapon_type->forward_offset : forward_offset));
				pos.y -= weapon_type->upward_offset;
			}
		}
		create_bullet(weapon_type, u, pos, u->owner, u->heading);
	}

	thingy_t* new_thingy() {
		if (!st.free_thingies.empty()) {
			thingy_t* r = &st.free_thingies.front();
			st.free_thingies.pop_front();
			return r;
		}
		if (st.thingies.size() >= 500) return nullptr;
		return &*st.thingies.emplace(st.thingies.end());
	}

	bool initialize_thingy(thingy_t* t, const sprite_type_t* sprite_type, xy pos, int owner) {
		t->hp = 1_fp8;
		t->sprite = create_sprite(sprite_type, pos, owner);
		if (!t->sprite) return false;
		return true;
	}

	thingy_t* create_thingy(const sprite_type_t* sprite_type, xy pos, int owner) {
		thingy_t* t = new_thingy();
		if (!t) return nullptr;
		if (!initialize_thingy(t, sprite_type, pos, owner)) {
			st.free_thingies.push_front(*t);
			return nullptr;
		}
		++st.active_thingies_size;
		bw_insert_list(st.active_thingies, *t);
		return t;
	}

	thingy_t* create_thingy_at_image(const image_t* parent_image, const sprite_type_t* sprite_type, xy offset, int elevation_level) {
		thingy_t* t = create_thingy(sprite_type, parent_image->sprite->position + parent_image->offset + offset, 0);
		if (!t) return nullptr;
		t->sprite->elevation_level = elevation_level;
		if (!us_hidden(t)) set_sprite_visibility(t->sprite, tile_visibility(t->sprite->position));
		return t;
	}

	bool unit_tech_target_valid(const unit_t* u, const tech_type_t* tech, const unit_t* target) const {
		if (target->stasis_timer) return false;
		switch (tech->id) {
		case TechTypes::Feedback:
			if (ut_building(target)) return false;
			if (!ut_has_energy(target)) return false;
			if (u_hallucination(target)) return false;
			return true;
		case TechTypes::Mind_Control:
			if (target->owner == u->owner) return false;
			if (ut_building(target)) return false;
			if (unit_is(target, UnitTypes::Terran_Vulture_Spider_Mine)) return false;
			if (unit_is(target, UnitTypes::Zerg_Larva)) return false;
			if (unit_is_egg(target)) return false;
			if (unit_is(target, UnitTypes::Protoss_Interceptor)) return false;
			if (unit_is(target, UnitTypes::Protoss_Scarab)) return false;
			return true;
		case TechTypes::Hallucination:
			if (ut_building(target)) return false;
			if (unit_is(target, UnitTypes::Protoss_Interceptor)) return false;
			return true;
		case TechTypes::Defensive_Matrix:
		case TechTypes::Irradiate:
		case TechTypes::Restoration:
		case TechTypes::Optical_Flare:
			if (ut_building(target)) return false;
			return true;
		case TechTypes::Consume:
			if (ut_building(target)) return false;
			if (target->owner != u->owner) return false;
			if (unit_race(target) == race_t::terran) return false;
			if (unit_is(target, UnitTypes::Zerg_Larva)) return false;
			return true;
		default:
			return false;
		}
	}

	bool spell_order_valid(const unit_t* u) const {
		if (u->order_type->targets_enemies) {
			if (iscript_unit->order_type->weapon == WeaponTypes::None) return false;
			return weapon_can_target_unit(get_weapon_type(iscript_unit->order_type->weapon), u->order_target.unit, u);
		}
		switch (u->order_type->tech_type) {
		case TechTypes::None:
			return true;
		case TechTypes::Spider_Mines:
			return (st.tiles[tile_index(u->order_target.pos)].flags & (tile_t::flag_walkable | tile_t::flag_has_creep)) != 0;
		case TechTypes::EMP_Shockwave:
		case TechTypes::Scanner_Sweep:
		case TechTypes::Dark_Swarm:
		case TechTypes::Plague:
		case TechTypes::Ensnare:
		case TechTypes::Psionic_Storm:
		case TechTypes::Recall:
		case TechTypes::Stasis_Field:
		case TechTypes::Disruption_Web:
		case TechTypes::Maelstrom:
			return true;
		default:
			if (!u->order_target.unit || u_invincible(u->order_target.unit)) return false;
			return unit_tech_target_valid(u, get_tech_type(u->order_type->tech_type), u->order_target.unit);
		}
	}

	bool iscript_execute(image_t* image, iscript_state_t& state, bool noop = false, fp8* distance_moved = nullptr, bool allow_main_image_destruction = false) {
		if (state.wait) {
			--state.wait;
			return true;
		}

		auto play_frame = [&](size_t frame_index) {
			if (image->frame_index_base == frame_index) return;
			image->frame_index_base = frame_index;
			update_image_frame_index(image);
		};

		auto add_image = [&](ImageTypes image_id, xy offset, int order) {
			const image_type_t* image_type = get_image_type(image_id);
			image_t* script_image = image;
			image_t* image = create_image(image_type, script_image->sprite, offset, order, script_image);
			if (!image) return (image_t*)nullptr;

			if (image->flags & image_t::flag_has_directional_frames) {
				size_t index = script_image->flags & image_t::flag_horizontally_flipped ? 32 - script_image->frame_index_offset : script_image->frame_index_offset;
				set_image_heading_by_index(image, index);
			}
			update_image_frame_index(image);
			if (iscript_unit && (u_burrowed(iscript_unit) || u_in_bunker(iscript_unit))) {
				if (!image_type->always_visible) {
					hide_image(image);
				} else if (image->modifier == 0) {
					if (script_image->modifier >= 2 && script_image->modifier <= 7) {
						set_image_modifier(image, script_image->modifier);
						image->modifier_data1 = script_image->modifier_data1;
						image->modifier_data2 = script_image->modifier_data2;
					}
				}
			}
			return image;
		};

		auto use_weapon = [&](unit_t* u, WeaponTypes weapon_id) {
			const weapon_type_t* weapon_type = get_weapon_type(weapon_id);
			if (!weapon_type->flingy) return;
			create_bullet(weapon_type, u, u->order_target.pos, u->owner, u->heading);
			if (weapon_type->bullet_count == 2) {
				create_bullet(weapon_type, u, u->order_target.pos, u->owner, u->heading);
			}
		};

		auto attack_with_weapon = [&](const weapon_type_t* weapon) {
			fire_weapon(iscript_unit, weapon);
			if (weapon->bullet_count == 2) fire_weapon(iscript_unit, weapon);
		};

		auto attack_with = [&](int weapon_n) {
			if (!iscript_unit->order_target.unit) return;
			auto* weapon = weapon_n == 1 ? unit_ground_weapon(iscript_unit) : unit_air_weapon(iscript_unit);
			if (!weapon) error("attack_with %d: null weapon", weapon_n);
			attack_with_weapon(weapon);
		};

		auto attack_with_forward_offset = [&](int weapon_n, int forward_offset) {
			if (!iscript_unit->order_target.unit) return;
			auto* weapon = weapon_n == 1 ? unit_ground_weapon(iscript_unit) : unit_air_weapon(iscript_unit);
			if (!weapon) error("attack_with %d: null weapon", weapon_n);
			fire_weapon(iscript_unit, weapon, forward_offset);
			if (weapon->bullet_count == 2) fire_weapon(iscript_unit, weapon, forward_offset);
		};

		const int* program_data = global_st.iscript.program_data.data();
		const int* p = program_data + state.program_counter;

		auto playsndrand = [&]() {
			int n = *p++;
			if (!noop) {
				int index = lcg_rand(4) % n;
				play_sound(p[index], image->sprite->position);
			}
			p += n;
		};

		while (true) {
			using namespace iscript_opcodes;
			size_t pc = p - program_data;
			if (pc == 0) error("iscript: program counter is null");
			int opc = *p++ - 0x808091;
			int a, b, c;
			switch (opc) {
			case opc_playfram:
				a = *p++;
				if (noop) break;
				play_frame(a);
				break;
			case opc_playframtile:
				a = *p++;
				if (noop) break;
				if ((size_t)a + game_st.tileset_index < image->grp->frames.size()) play_frame(a + game_st.tileset_index);
				break;
			case opc_sethorpos:
				a = *p++;
				if (noop) break;
				if (image->offset.x != a) {
					image->offset.x = a;
					image->flags |= image_t::flag_redraw;
				}
				break;
			case opc_setvertpos:
				a = *p++;
				if (noop) break;
				if (!iscript_unit || (!u_requires_detector(iscript_unit) && !u_cloaked(iscript_unit))) {
					if (image->offset.y != a) {
						image->offset.y = a;
						image->flags |= image_t::flag_redraw;
					}
				}
				break;
			case opc_setpos:
				a = *p++;
				b = *p++;
				if (noop) break;
				set_image_offset(image, xy(a, b));
				break;
			case opc_wait:
				state.wait = *p++ - 1;
				state.program_counter = p - program_data;
				return true;
			case opc_waitrand:
				a = *p++;
				b = *p++;
				if (noop) break;
				state.wait = a + ((lcg_rand(3) & 0xff) % (b - a + 1)) - 1;
				state.program_counter = p - program_data;
				return true;
			case opc_goto:
				p = program_data + *p;
				break;
			case opc_imgol:
			case opc_imgul:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				add_image((ImageTypes)a, image->offset + xy(b, c), opc == opc_imgol ? image_order_above : image_order_below);
				break;
			case opc_imgolorig:
			case opc_switchul:
				a = *p++;
				if (noop) break;
				if (image_t* new_image = add_image((ImageTypes)a, xy(), opc == opc_imgolorig ? image_order_above : image_order_below)) {
					if (!i_flag(new_image, image_t::flag_uses_special_offset)) {
						i_set_flag(new_image, image_t::flag_uses_special_offset);
						update_image_special_offset(new_image);
					}
				}
				break;
			case opc_imgoluselo:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				add_image((ImageTypes)a, get_image_lo_offset(image, (size_t)b, (size_t)c), image_order_above);
				break;

			case opc_sprol:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				if (iscript_bullet && iscript_bullet->bullet_owner_unit && unit_is_goliath(iscript_bullet->bullet_owner_unit) && player_has_upgrade(iscript_bullet->bullet_owner_unit->owner, UpgradeTypes::Charon_Boosters)) {
					create_thingy_at_image(image, get_sprite_type(SpriteTypes::SPRITEID_Halo_Rockets_Trail), {b, c}, image->sprite->elevation_level + 1);
				} else {
					create_thingy_at_image(image, get_sprite_type((SpriteTypes)a), {b, c}, image->sprite->elevation_level + 1);
				}
				break;

			case opc_lowsprul:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				create_thingy_at_image(image, get_sprite_type((SpriteTypes)a), {b, c}, 1);
				break;

			case opc_spruluselo:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				if (auto* sprite = get_sprite_type((SpriteTypes)a)) {
					if (iscript_unit && (u_requires_detector(iscript_unit) || u_cloaked(iscript_unit)) && !sprite->image->always_visible) break;
					auto* t = create_thingy_at_image(image, sprite, {b, c}, image->sprite->elevation_level);
					if (t) set_sprite_images_heading_by_image_index(t->sprite, image);
				}
				break;
			case opc_sprul:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				if (auto* sprite = get_sprite_type((SpriteTypes)a)) {
					if (iscript_unit && (u_requires_detector(iscript_unit) || u_cloaked(iscript_unit)) && !sprite->image->always_visible) break;
					auto* t = create_thingy_at_image(image, sprite, {b, c}, image->sprite->elevation_level - 1);
					if (t) set_sprite_images_heading_by_image_index(t->sprite, image);
				}
				break;
			case opc_sproluselo:
				a = *p++;
				b = *p++;
				if (noop) break;
				if (auto* sprite = get_sprite_type((SpriteTypes)a)) {
					auto* t = create_thingy_at_image(image, sprite, get_image_lo_offset(image, (size_t)b, 0), image->sprite->elevation_level + 1);
					if (t) set_sprite_images_heading_by_image_index(t->sprite, image);
				}
				break;
			case opc_end:
				if (noop) break;
				if (image == image->sprite->main_image && !allow_main_image_destruction) error("iscript_execute: main image not allowed to be destroyed here");
				state.program_counter = 0;
				destroy_image(image);
				return false;
			case opc_setflipstate:
				a = *p++;
				if (noop) break;
				if (i_flag(image, image_t::flag_horizontally_flipped) != (a != 0)) {
					i_set_flag(image, image_t::flag_horizontally_flipped, a != 0);
					set_image_modifier(image, image->modifier);
					if (image->flags & image_t::flag_uses_special_offset) update_image_special_offset(image);
				}
				break;
			case opc_playsnd:
				a = *p++;
				if (noop) break;
				play_sound(a, image->sprite->position);
				break;
			case opc_playsndrand:
				playsndrand();
				break;
			case opc_playsndbtwn:
				a = *p++;
				b = *p++;
				if (noop) break;
				play_sound(a + lcg_rand(5) % (b - a + 1), image->sprite->position);
				break;
			case opc_domissiledmg:
			case opc_dogrddamage:
				if (noop) break;
				if (iscript_bullet) bullet_hit(iscript_bullet);
				break;
			case opc_attackmelee:
				if (!noop && iscript_unit) melee_deal_damage(iscript_unit);
				playsndrand();
				break;

			case opc_followmaingraphic:
				if (noop) break;
				if (image_t* main_image = image->sprite->main_image) {
					auto frame_index = main_image->frame_index;
					bool flipped = i_flag(main_image, image_t::flag_horizontally_flipped);
					if (image->frame_index != frame_index  || i_flag(image, image_t::flag_horizontally_flipped) != flipped) {
						image->frame_index_base = main_image->frame_index_base;
						set_image_frame_index_offset(image, main_image->frame_index_offset, flipped);
					}
				}
				break;
			case opc_randcondjmp:
				a = *p++;
				b = *p++;
				if ((lcg_rand(7) & 0xff) <= a) {
					p = program_data + b;
				}
				break;

			case opc_turnccwise:
				a = *p++;
				if (noop) break;
				if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading - 8_dir * a);
				break;
			case opc_turncwise:
				a = *p++;
				if (noop) break;
				if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading + 8_dir * a);
				break;
			case opc_turn1cwise:
				if (noop) break;
				if (iscript_unit && !iscript_unit->order_target.unit) set_unit_heading(iscript_unit, iscript_unit->heading + 8_dir);
				break;
			case opc_turnrand:
				a = *p++;
				if (noop) break;
				if (lcg_rand(6) % 4 == 1) {
					if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading - 8_dir * a);
				} else {
					if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading + 8_dir * a);
				}
				break;

			case opc_sigorder:
				a = *p++;
				if (noop) break;
				if (iscript_flingy) iscript_flingy->order_signal |= a;
				break;
			case opc_attackwith:
				a = *p++;
				if (noop) break;
				if (iscript_unit) attack_with(a);
				break;
			case opc_attack:
				if (noop) break;
				if (iscript_unit && iscript_unit->order_target.unit) {
					if (!u_flying(iscript_unit->order_target.unit)) {
						attack_with(1);
					} else attack_with(2);
				}
				break;
			case opc_castspell:
				if (noop) break;
				if (iscript_unit && iscript_unit->order_type->weapon != WeaponTypes::None) {
					if (spell_order_valid(iscript_unit)) {
						attack_with_weapon(get_weapon_type(iscript_unit->order_type->weapon));
					}
				}
				break;
			case opc_useweapon:
				a = *p++;
				if (noop) break;
				if (iscript_unit) use_weapon(iscript_unit, (WeaponTypes)a);
				break;
			case opc_move:
				a = *p++;
				if (distance_moved) {
					if (iscript_unit) *distance_moved = get_modified_unit_speed(iscript_unit, fp8::integer(a));
				}
				if (noop) break;
				if (iscript_unit) set_next_speed(iscript_unit, get_modified_unit_speed(iscript_unit, fp8::integer(a)));
				break;
			case opc_gotorepeatattk:
				if (noop) break;
				if (iscript_unit) u_unset_movement_flag(iscript_unit, 8);
				break;
			case opc_engframe:
				a = *p++;
				if (noop) break;
				image->frame_index_base = a;
				set_image_frame_index_offset(image, image->sprite->main_image->frame_index_offset, i_flag(image->sprite->main_image, image_t::flag_horizontally_flipped));
				break;
			case opc_engset:
				a = *p++;
				if (noop) break;
				image->frame_index_base = image->sprite->main_image->frame_index_base + (image->sprite->main_image->grp->frames.size() & 0x7fff) * a;
				set_image_frame_index_offset(image, image->sprite->main_image->frame_index_offset, i_flag(image->sprite->main_image, image_t::flag_horizontally_flipped));
				break;

			case opc_nobrkcodestart:
				if (noop) break;
				if (iscript_unit) {
					u_set_status_flag(iscript_unit, unit_t::status_flag_iscript_nobrk);
					iscript_unit->sprite->flags |= sprite_t::flag_iscript_nobrk;
				}
				break;
			case opc_nobrkcodeend:
				if (noop) break;
				if (iscript_unit) {
					u_unset_status_flag(iscript_unit, unit_t::status_flag_iscript_nobrk);
					iscript_unit->sprite->flags &= ~sprite_t::flag_iscript_nobrk;
					if (!iscript_unit->order_queue.empty() && iscript_unit->user_action_flags & 1) {
						iscript_run_to_idle(iscript_unit);
						activate_next_order(iscript_unit);
					}
				}
				break;
			case opc_ignorerest:
				if (noop) break;
				if (iscript_unit && !iscript_unit->order_target.unit) {
					iscript_run_to_idle(iscript_unit);
					break;
				}
				state.wait = 10;
				state.program_counter = p - 1 - program_data;
				return true;
			case opc_attkshiftproj:
				a = *p++;
				if (noop) break;
				if (iscript_unit) attack_with_forward_offset(1, a);
				break;
			case opc_tmprmgraphicstart:
				if (noop) break;
				hide_image(image);
				break;
			case opc_tmprmgraphicend:
				if (noop) break;
				show_image(image);
				break;

			case opc_setfldirect:
				a = *p++;
				if (noop) break;
				if (iscript_unit) set_unit_heading(iscript_unit, 8_dir * a);
				break;

			case opc_setflspeed:
				a = *p++;
				if (noop) break;
				if (iscript_unit) iscript_unit->flingy_top_speed = fp8::from_raw(a);
				break;

			case opc_call:
				a = *p++;
				state.return_address = p - program_data;
				p = program_data + a;
				break;
			case opc_return:
				p = program_data + state.return_address;
				break;

			case opc_creategasoverlays:
				a = *p++;
				if (noop) break;
				if (iscript_unit && ut_resource(iscript_unit)) {
					ImageTypes image_id = iscript_unit->building.resource.resource_count ? ImageTypes::IMAGEID_Vespene_Geyser_Smoke1 : ImageTypes::IMAGEID_Vespene_Geyser_Smoke1_Overlay;
					image_id = (ImageTypes)((size_t)image_id + a);
					create_image(get_image_type(image_id), image->sprite, image->offset + get_image_lo_offset(image, 2, a), image_order_above);
				}
				break;
			case opc_pwrupcondjmp:
				a = *p++;
				if (image->sprite && image->sprite->main_image != image) {
					p = program_data + a;
				}
				break;
			case opc_trgtrangecondjmp:
				a = *p++;
				b = *p++;
				if (noop) continue;
				if (iscript_unit && iscript_unit->order_target.unit) {
					xy pos = get_bullet_appear_at_target_pos(iscript_unit, iscript_unit->order_target.unit);
					if (xy_length(to_xy_fp8(pos) - iscript_unit->exact_position).integer_part() <= a) {
						p = program_data + b;
					}
				}
				break;
			case opc_trgtarccondjmp:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				if (iscript_unit && iscript_unit->order_target.unit) {
					if (fp8::extend(direction_t::from_raw(a) - xy_direction(iscript_unit->order_target.unit->sprite->position - iscript_unit->sprite->position)).abs() < fp8::from_raw(b)) {
						p = program_data + c;
					}
				}
				break;
			case opc_curdirectcondjmp:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				if (iscript_unit && fp8::extend(iscript_unit->heading - direction_t::from_raw(a)).abs() < fp8::from_raw(b)) {
					p = program_data + c;
				}
				break;
			case opc_imgulnextid:
				a = *p++;
				b = *p++;
				if (noop) break;
				add_image((ImageTypes)((int)image->image_type->id + 1), image->offset + xy(a, b), image_order_below);
				break;

			case opc_liftoffcondjmp:
				a = *p++;
				if (noop) break;
				if (iscript_unit && u_flying(iscript_unit)) {
					p = program_data + a;
				}
				break;
			case opc_warpoverlay:
				a = *p++;
				if (noop) break;
				image->modifier_data1 = a & 0xff;
				image->modifier_data2 = (a >> 8) & 0xff;
				break;
			case opc_orderdone:
				a = *p++;
				if (noop) break;
				if (iscript_flingy) iscript_flingy->order_signal &= ~a;
				break;
			case opc_grdsprol:
				a = *p++;
				b = *p++;
				c = *p++;
				if (noop) break;
				if (unit_type_can_fit_at(get_unit_type(UnitTypes::Terran_Marine), image->sprite->position + image->offset + xy(b, c))) {
					create_thingy_at_image(image, get_sprite_type((SpriteTypes)a), xy(b, c), image->sprite->elevation_level + 1);
				}
				break;
			default:
				error("iscript: unhandled opcode %d", opc);
			}
		}

	}

	bool iscript_run_anim(image_t* image, int new_anim) {
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
		auto* script = image->iscript_state.current_script;
		if (!script) error("attempt to start animation without a script");
		auto& anims_pc = script->animation_pc;
		if ((size_t)new_anim >= anims_pc.size()) error("script %d does not have animation %d", script->id, new_anim);
		image->iscript_state.animation = new_anim;
		image->iscript_state.program_counter = anims_pc[new_anim];
		image->iscript_state.return_address = 0;
		image->iscript_state.wait = 0;
		return iscript_execute(image, image->iscript_state);
	}

	void image_update_cloak(image_t* image) {
		if (image->modifier_data2) --image->modifier_data2;
		else {
			++image->modifier_data1;
			image->modifier_data2 = 3;
			if (image->modifier_data1 >= 8) {
				set_image_modifier(image, image->modifier + 1);
				if (iscript_unit) {
					u_set_status_flag(iscript_unit, unit_t::status_flag_cloaked);
					u_set_status_flag(iscript_unit, unit_t::status_flag_requires_detector);
				}
			}
		}
	}

	void image_update_decloak(image_t* image) {
		if (image->modifier_data2) --image->modifier_data2;
		else {
			image->modifier_data2 = 3;
			if (image->modifier_data1) --image->modifier_data1;
			else {
				set_image_modifier(image, 0);
				if (iscript_unit) {
					u_unset_status_flag(iscript_unit, unit_t::status_flag_cloaked);
					u_unset_status_flag(iscript_unit, unit_t::status_flag_requires_detector);
				}
			}
		}
	}

	void image_update_warpin(image_t* image) {
		if (image->modifier_data2) --image->modifier_data2;
		else {
			image->modifier_data2 = 2;
			if (image->modifier_data1 < 63) ++image->modifier_data1;
			else {
				iscript_run_anim(image, iscript_anims::Death);
				if (iscript_flingy) iscript_flingy->order_signal |= 1;
			}
		}
	}

	bool iscript_execute_sprite(sprite_t* sprite) {
		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t* image = &*i++;
			if (image->modifier == 2 || image->modifier == 5) image_update_cloak(image);
			else if (image->modifier == 4 || image->modifier == 7) image_update_decloak(image);
			else if (image->modifier == 17) image_update_warpin(image);
			bool is_main_image = image == sprite->main_image;
			bool destroyed = !iscript_execute(image, image->iscript_state, false, nullptr, true);
			if (is_main_image && destroyed && !sprite->images.empty()) {
				sprite->main_image = &sprite->images.front();
			}
		}
		if (!sprite->images.empty()) return true;

		remove_sprite_from_tile_line(sprite);
		st.sprites_container.push(sprite);

		return false;
	}

	void sprite_run_anim(sprite_t* sprite, int anim) {
		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t* image = &*i++;
			iscript_run_anim(image, anim);
		}
	}

	void initialize_image(image_t* image, const image_type_t* image_type, sprite_t* sprite, xy offset) {
		image->image_type = image_type;
		image->grp = global_st.image_grp[(size_t)image_type->id];
		int flags = 0;
		if (image_type->has_directional_frames) flags |= image_t::flag_has_directional_frames;
		if (image_type->is_clickable) flags |= image_t::flag_clickable;
		image->flags = flags;
		image->frame_index_base = 0;
		image->frame_index_offset = 0;
		image->frame_index = 0;
		image->sprite = sprite;
		image->offset = offset;
		image->modifier_data1 = 0;
		image->modifier_data2 = 0;
		image->iscript_state.current_script = nullptr;
		image->iscript_state.program_counter = 0;
		image->iscript_state.return_address = 0;
		image->iscript_state.animation = 0;
		image->iscript_state.wait = 0;
		int modifier = image_type->modifier;
		if (modifier == 14) {
			image->modifier_data1 = sprite->owner;
			image->modifier_data2 = 0;
		} else if (modifier == 9) {
			// some color shift stuff based on the tileset
			// see 4BDE60
			//image->coloring_data = 0; // fixme
		}
	}

	void destroy_image(image_t* image) {
		image->grp = nullptr;
		image->sprite->images.remove(*image);
		st.images_container.push(image);
	}

	enum {
		image_order_top,
		image_order_bottom,
		image_order_above,
		image_order_below
	};
	image_t* create_image(const image_type_t* image_type, sprite_t* sprite, xy offset, int order, image_t* relimg = nullptr) {
		if (!image_type)  error("attempt to create image of null type");

		image_t* image = st.images_container.top();
		if (!image) return nullptr;
		st.images_container.pop();

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
		set_image_modifier(image, image->image_type->modifier);
		i_set_flag(image, image_t::flag_has_iscript_animations, image->image_type->has_iscript_animations);
		iscript_set_script(image, image->image_type->iscript_id);
		if (!iscript_run_anim(image, iscript_anims::Init)) error("create_image: image destroyed");
		return image;
	}

	void destroy_sprite(sprite_t* sprite) {
		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t* image = &*i++;
			destroy_image(image);
		}
		remove_sprite_from_tile_line(sprite);
		st.sprites_container.push(sprite);
	}

	sprite_t* create_sprite(const sprite_type_t* sprite_type, xy pos, int owner) {
		if (!sprite_type)  error("attempt to create sprite of null type");

		sprite_t* sprite = st.sprites_container.top();
		if (!sprite) return nullptr;
		st.sprites_container.pop();

		auto initialize_sprite = [&]() {
			if ((size_t)pos.x >= game_st.map_width || (size_t)pos.y >= game_st.map_height) return false;
			sprite->owner = owner;
			sprite->sprite_type = sprite_type;
			sprite->flags = 0;
			sprite->position = pos;
			sprite->visibility_flags = ~0;
			sprite->elevation_level = 4;
			sprite->selection_timer = 0;
			sprite->images.clear();
			if (!sprite_type->visible) {
				sprite->flags |= sprite_t::flag_hidden;
				set_sprite_visibility(sprite, 0);
			}
			if (!create_image(sprite_type->image, sprite, {0, 0}, image_order_above)) return false;
			sprite->width = std::min(sprite->main_image->grp->width, (size_t)0xff);
			sprite->height = std::min(sprite->main_image->grp->height, (size_t)0xff);
			return true;
		};

		if (!initialize_sprite()) {
			st.sprites_container.push(sprite);
			return nullptr;
		}
		add_sprite_to_tile_line(sprite);

		return sprite;
	}

	bool initialize_flingy(flingy_t* f, const flingy_type_t* flingy_type, xy pos, int owner, direction_t heading) {
		f->flingy_type = flingy_type;
		f->movement_flags = 0;
		f->next_speed = 0_fp8;
		f->flingy_top_speed = flingy_type->top_speed;
		f->flingy_acceleration = flingy_type->acceleration;
		f->flingy_turn_rate = flingy_type->turn_rate;
		f->flingy_movement_type = flingy_type->movement_type;

		f->position = pos;
		f->exact_position = to_xy_fp8(pos);

		set_flingy_move_target(f, pos);
		set_next_target_waypoint(f, pos);
		f->heading = heading;
		f->next_velocity_direction = heading;
		f->hp = 1_fp8;

		f->sprite = create_sprite(flingy_type->sprite, pos, owner);
		if (!f->sprite) return false;
		auto dir = f->heading;
		for (image_t* i : ptr(f->sprite->images)) {
			set_image_heading(i, dir);
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
			default:
				return UpgradeTypes::None;
			};
		};
		bool cooldown = false;
		if (unit_is(u, UnitTypes::Hero_Devouring_One)) cooldown = true;
		if (unit_is(u, UnitTypes::Zerg_Zergling) && player_has_upgrade(u->owner, UpgradeTypes::Adrenal_Glands)) cooldown = true;
		bool speed = false;
		auto speed_upg = speed_upgrade();
		if (speed_upg != UpgradeTypes::None && player_has_upgrade(u->owner, speed_upg)) speed = true;
		if (unit_is(u, UnitTypes::Hero_Hunter_Killer)) speed = true;
		if (unit_is(u, UnitTypes::Hero_Yggdrasill)) speed = true;
		if (unit_is(u, UnitTypes::Hero_Fenix_Zealot)) speed = true;
		if (unit_is(u, UnitTypes::Hero_Mojo)) speed = true;
		if (unit_is(u, UnitTypes::Hero_Artanis)) speed = true;
		if (unit_is(u, UnitTypes::Zerg_Lurker)) speed = true;
		if (cooldown != u_cooldown_upgrade(u) || speed != u_speed_upgrade(u)) {
			if (cooldown) u->status_flags |= unit_t::status_flag_cooldown_upgrade;
			if (speed) u->status_flags |= unit_t::status_flag_speed_upgrade;
			update_unit_speed(u);
		}
	}

	void update_unit_speed(unit_t* u) {

		if (u->flingy_movement_type == 2) {
			image_t* image = u->sprite->main_image;
			if (!image) error("null image");
			auto* script = image->iscript_state.current_script;
			auto& anims_pc = script->animation_pc;
			int anim = iscript_anims::Walking;
			if ((size_t)anim < anims_pc.size() && anims_pc[anim] != 0) {
				auto ius = make_thingy_setter(iscript_unit, u);
				iscript_state_t st;
				st.current_script = script;
				st.animation = anim;
				st.program_counter = anims_pc[anim];
				st.return_address = 0;
				st.wait = 0;
				fp8 total_distance_moved {};
				for (int i = 0; i < 32; ++i) {
					fp8 distance_moved {};
					if (!iscript_execute(image, st, true, &distance_moved)) error("update_unit_speed: image destroyed");
					// This get_modified_unit_acceleration is very out of place, and
					// it makes the stored flingy_top_speed value wrong. But BroodWar does it.
					// It's probably a bug, but the value might not be used for anything
					// significant.
					total_distance_moved += get_modified_unit_acceleration(u, distance_moved);
				}
				auto avg_distance_moved = total_distance_moved / 32;
				u->flingy_top_speed = avg_distance_moved;
			}
		} else {
			u->flingy_top_speed = get_modified_unit_speed(u, u->flingy_type->top_speed);
			u->flingy_acceleration = get_modified_unit_acceleration(u, u->flingy_type->acceleration);
			u->flingy_turn_rate = get_modified_unit_turn_rate(u, u->flingy_type->turn_rate);
		}

	}

	void increment_unit_counts(unit_t* u, int count) {
		if (u_hallucination(u)) return;
		if (ut_turret(u)) return;

		st.unit_counts[u->owner][u->unit_type->id] += count;
		auto supply_required = u->unit_type->supply_required;
		auto race = unit_race(u);
		if (race == race_t::zerg) {
			if (unit_is_egg(u)) {
				if (!u->build_queue.empty()) {
					const unit_type_t* ut = u->build_queue.front();
					supply_required = ut->supply_required;
					if (ut_two_units_in_one_egg(ut)) supply_required *= 2;
				}
			} else {
				if (ut_two_units_in_one_egg(u) && !u_completed(u)) supply_required *= 2;
			}
			st.supply_used[u->owner][0] += supply_required * count;
		} else if (race == race_t::terran) {
			st.supply_used[u->owner][1] += supply_required * count;
		} else if (race == race_t::protoss) {
			st.supply_used[u->owner][2] += supply_required * count;
		}
		if (u->unit_type->group_flags & GroupFlags::Factory) st.factory_counts[u->owner] += count;
		if (u->unit_type->group_flags & GroupFlags::Men) {
			st.non_building_counts[u->owner] += count;
		} else if (u->unit_type->group_flags & GroupFlags::Building) {
			st.building_counts[u->owner] += count;
		} else if (unit_is_egg(u)) {
			st.non_building_counts[u->owner] += count;
		}
		if (st.unit_counts[u->owner][u->unit_type->id] < 0) st.unit_counts[u->owner][u->unit_type->id] = 0;
	}

	void unit_finder_insert(unit_t* u) {
		if (ut_turret(u)) return;

		rect bb = unit_sprite_inner_bounding_box(u);
		unit_finder_insert(u, bb);
	}

	void unit_finder_reinsert(unit_t* u) {
		if (u->unit_finder_bounding_box.from.x == -1) return;
		rect bb = unit_sprite_inner_bounding_box(u);
		unit_finder_reinsert(u, bb);
	}

	void unit_finder_remove(unit_t* u) {
		if (u->unit_finder_bounding_box.from.x == -1) return;
		if (unit_finder_search_index) error("attempt to modify unit finder while search is active");
		auto remove = [&](auto& vec, int value) {
			auto cmp_l = [&](auto& a, int b) {
				return a.value < b;
			};
			auto i = std::lower_bound(vec.begin(), vec.end(), value, cmp_l);
			while (i->u != u) ++i;
			vec.erase(i);
		};
		remove(st.unit_finder_x, u->unit_finder_bounding_box.from.x);
		remove(st.unit_finder_x, u->unit_finder_bounding_box.to.x);
		remove(st.unit_finder_y, u->unit_finder_bounding_box.from.y);
		remove(st.unit_finder_y, u->unit_finder_bounding_box.to.y);
		u->unit_finder_bounding_box = {{-1, -1}, {-1, -1}};
	}

	void unit_finder_insert(unit_t* u, rect bb) {
		if (unit_finder_search_index) error("attempt to modify unit finder while search is active");
		auto insert = [&](auto& vec, int from_value, int to_value) {
			auto cmp_l = [&](auto& a, int b) {
				return a.value < b;
			};
			auto from_i = std::lower_bound(vec.begin(), vec.end(), from_value, cmp_l);
			vec.insert(from_i, {u, from_value});
			auto to_i = std::lower_bound(vec.begin(), vec.end(), to_value, cmp_l);
			vec.insert(to_i, {u, to_value});
		};
		insert(st.unit_finder_x, bb.from.x, bb.to.x);
		insert(st.unit_finder_y, bb.from.y, bb.to.y);
		u->unit_finder_bounding_box = bb;
	}
	void unit_finder_reinsert(unit_t* u, rect bb) {
		if (unit_finder_search_index) error("attempt to modify unit finder while search is active");
		auto reinsert = [&](auto& vec, int old_value, int new_value) {
			if (old_value == new_value) return;
			auto cmp_l = [&](auto& a, int b) {
				return a.value < b;
			};
			auto i = std::lower_bound(vec.begin(), vec.end(), old_value, cmp_l);
			while (i->u != u) ++i;
			if (new_value > old_value) {
				auto ni = std::next(i);
				while (ni != vec.end() && ni->value < new_value) {
					*i = *ni;
					++i;
					++ni;
				}
				*i = {u, new_value};
			} else {
				while (i != vec.begin()) {
					auto ni = i;
					--i;
					if (i->value <= new_value) {
						++i;
						break;
					}
					*ni = *i;
				}
				*i = {u, new_value};
			}
		};
		if (bb.from.x <= u->unit_finder_bounding_box.from.x) {
			reinsert(st.unit_finder_x, u->unit_finder_bounding_box.from.x, bb.from.x);
			reinsert(st.unit_finder_x, u->unit_finder_bounding_box.to.x, bb.to.x);
		} else {
			reinsert(st.unit_finder_x, u->unit_finder_bounding_box.to.x, bb.to.x);
			reinsert(st.unit_finder_x, u->unit_finder_bounding_box.from.x, bb.from.x);
		}
		if (bb.from.y <= u->unit_finder_bounding_box.from.y) {
			reinsert(st.unit_finder_y, u->unit_finder_bounding_box.from.y, bb.from.y);
			reinsert(st.unit_finder_y, u->unit_finder_bounding_box.to.y, bb.to.y);
		} else {
			reinsert(st.unit_finder_y, u->unit_finder_bounding_box.to.y, bb.to.y);
			reinsert(st.unit_finder_y, u->unit_finder_bounding_box.from.y, bb.from.y);
		}
		u->unit_finder_bounding_box = bb;
	}


	struct unit_finder_search {
		using value_type = unit_t*;

		struct iterator {
			using value_type = unit_t*;
			using iterator_category = std::forward_iterator_tag;
		private:
			const unit_finder_search* search;
			a_vector<state::unit_finder_entry>::iterator i;
			friend unit_finder_search;
			iterator(const unit_finder_search* search, a_vector<state::unit_finder_entry>::iterator i) : search(search), i(i) {}
			bool in_bounds() {
				unit_t* u = i->u;
				if (u->unit_finder_bounding_box.from.x >= search->area.to.x) return false;
				if (u->unit_finder_bounding_box.from.y >= search->area.to.y) return false;
				if (u->unit_finder_bounding_box.to.y < search->area.from.y) return false;
				return true;
			}
		public:

			unit_t* operator*() const {
				return i->u;
			}

			unit_t* operator->() const {
				return i->u;
			}

			iterator& operator++() {
				do {
					++i;
					if (i == search->i_end) return *this;
				} while (!in_bounds() || i->u->unit_finder_visited[search->search_index]);
				i->u->unit_finder_visited[search->search_index] = true;
				return *this;
			}

			iterator operator++(int) {
				auto r = *this;
				++*this;
				return r;
			}

			bool operator==(const iterator& n) const {
				return i == n.i;
			}
			bool operator!=(const iterator& n) const {
				return i != n.i;
			}
		};

	private:
		friend state_functions;
		const state_functions& funcs;
		a_vector<state::unit_finder_entry>::iterator i_begin;
		a_vector<state::unit_finder_entry>::iterator i_end;
		rect area;
		size_t search_index;
		unit_finder_search(const state_functions& funcs, rect area, bool expand) : funcs(funcs), area(area) {
			if (funcs.unit_finder_search_index == 4) error("unit_finder_search maximum recursive depth reached");
			search_index = funcs.unit_finder_search_index;
			++funcs.unit_finder_search_index;

			auto cmp_l = [&](auto& a, int b) {
				return a.value < b;
			};
			int begin_x = area.from.x;
			int end_x = area.to.x;
			if (expand) {
				if (end_x - begin_x + 1 < funcs.game_st.max_unit_width) {
					end_x = begin_x + funcs.game_st.max_unit_width - 1;
					++this->area.to.x;
				}
				if (area.to.y - area.from.y + 1 < funcs.game_st.max_unit_height) {
					++this->area.to.y;
				}
			}
			i_begin = std::lower_bound(funcs.st.unit_finder_x.begin(), funcs.st.unit_finder_x.end(), begin_x, cmp_l);
			i_end = std::lower_bound(funcs.st.unit_finder_x.begin(), funcs.st.unit_finder_x.end(), end_x, cmp_l);
		}
	public:
		~unit_finder_search() {
			--funcs.unit_finder_search_index;
			for (auto i = i_begin; i != i_end; ++i) {
				i->u->unit_finder_visited[search_index] = false;
			}
		}

		iterator begin() {
			auto r = iterator(this, i_begin);
			if (i_begin != i_end) {
				if (r.in_bounds()) r->unit_finder_visited[search_index] = true;
				else ++r;
			}
			return r;
		}
		iterator end() {
			return iterator(this, i_end);
		}
	};

	unit_finder_search find_units(rect area) const {
		return unit_finder_search(*this, area, true);
	}

	unit_finder_search find_units_noexpand(rect area) const {
		return unit_finder_search(*this, area, false);
	}

	template<typename F>
	unit_t* find_unit(rect area, F&& predicate) const {
		for (unit_t* u : find_units(area)) {
			if (predicate(u)) return u;
		}
		return nullptr;
	}

	template<typename F>
	unit_t* find_unit_noexpand(rect area, F&& predicate) const {
		for (unit_t* u : find_units_noexpand(area)) {
			if (predicate(u)) return u;
		}
		return nullptr;
	}

	template<typename F, typename i_T>
	unit_t* find_nearest_unit(xy pos, rect search_area, i_T left_i, i_T up_i, i_T right_i, i_T down_i, F&& predicate) const {

		const auto x_begin = st.unit_finder_x.begin();
		const auto y_begin = st.unit_finder_y.begin();
		const auto x_end = st.unit_finder_x.end();
		const auto y_end = st.unit_finder_y.end();

		int best_distance = xy_length({std::max(pos.x - search_area.from.x, search_area.to.x - pos.x), std::max(pos.y - search_area.from.y, search_area.to.y - pos.y)});
		unit_t* best_unit = nullptr;

		while (true) {
			bool done = true;
			int prev_best_distance = best_distance;
			if (left_i != x_begin) {
				--left_i;
				done = false;
				unit_t* u = left_i->u;
				if (u->sprite->position.x >= search_area.from.x) {
					if (u->sprite->position.y >= search_area.from.y && u->sprite->position.y < search_area.to.y) {
						int d = xy_length(pos - u->sprite->position);
						if (d < best_distance && predicate(u)) {
							best_distance = d;
							best_unit = u;
						}
					}
				} else {
					left_i = x_begin;
				}
			}
			if (right_i != x_end) {
				done = false;
				unit_t* u = right_i->u;
				if (u->sprite->position.x < search_area.to.x) {
					if (u->sprite->position.y >= search_area.from.y && u->sprite->position.y < search_area.to.y) {
						int d = xy_length(pos - u->sprite->position);
						if (d < best_distance && predicate(u)) {
							best_distance = d;
							best_unit = u;
						}
					}
					++right_i;
				} else {
					right_i = x_end;
				}
			}
			if (up_i != y_begin) {
				done = false;
				--up_i;
				unit_t* u = up_i->u;
				if (u->sprite->position.y >= search_area.from.y) {
					if (u->sprite->position.x >= search_area.from.x && u->sprite->position.x < search_area.to.x) {
						int d = xy_length(pos - u->sprite->position);
						if (d < best_distance && predicate(u)) {
							best_distance = d;
							best_unit = u;
						}
					}
				} else {
					up_i = y_begin;
				}
			}
			if (down_i != y_end) {
				done = false;
				unit_t* u = down_i->u;
				if (u->sprite->position.y < search_area.to.y) {
					if (u->sprite->position.x >= search_area.from.x && u->sprite->position.x < search_area.to.x) {
						int d = xy_length(pos - u->sprite->position);
						if (d < best_distance && predicate(u)) {
							best_distance = d;
							best_unit = u;
						}
					}
					++down_i;
				} else {
					down_i = y_end;
				}
			}
			if (best_distance != prev_best_distance) {
				if (search_area.from.x < pos.x - best_distance) search_area.from.x = pos.x - best_distance;
				if (search_area.from.y < pos.y - best_distance) search_area.from.y = pos.y - best_distance;
				if (search_area.to.x > pos.x + best_distance) search_area.to.x = pos.x + best_distance;
				if (search_area.to.y > pos.y + best_distance) search_area.to.y = pos.y + best_distance;
			}
			if (done) break;
		}
		return best_unit;
	}

	template<typename F>
	unit_t* find_nearest_unit(xy pos, rect search_area, F&& predicate) const {

		auto cmp_l = [&](auto& a, int b) {
			return a.value < b;
		};
		auto x_i = std::lower_bound(st.unit_finder_x.begin(), st.unit_finder_x.end(), pos.x, cmp_l);
		auto y_i = std::lower_bound(st.unit_finder_y.begin(), st.unit_finder_y.end(), pos.y, cmp_l);

		return find_nearest_unit(pos, search_area, x_i, y_i, x_i, y_i, predicate);
	}

	template<typename F>
	unit_t* find_nearest_unit(const unit_t* u, rect search_area, F&& predicate) const {
		if (us_hidden(u)) {
			return find_nearest_unit(u->sprite->position, search_area, std::forward<F>(predicate));
		} else {
			auto cmp_l = [&](auto& a, int b) {
				return a.value < b;
			};
			auto get = [&](auto& vec, int value) {
				auto i = std::lower_bound(vec.begin(), vec.end(), value, cmp_l);
				while (i->u != u) ++i;
				return i;
			};
			auto left_i = get(st.unit_finder_x, u->unit_finder_bounding_box.to.x);
			auto up_i = get(st.unit_finder_y, u->unit_finder_bounding_box.to.y);
			auto right_i = std::next(get(st.unit_finder_x, u->unit_finder_bounding_box.from.x));
			auto down_i = std::next(get(st.unit_finder_y, u->unit_finder_bounding_box.from.y));
			return find_nearest_unit(u->sprite->position, search_area, left_i, up_i, right_i, down_i, std::forward<F>(predicate));
		}
	}

	bool unit_is_factory(const unit_t* u) const {
		if (unit_is(u, UnitTypes::Terran_Command_Center)) return true;
		if (unit_is(u, UnitTypes::Terran_Barracks)) return true;
		if (unit_is(u, UnitTypes::Terran_Factory)) return true;
		if (unit_is(u, UnitTypes::Terran_Starport)) return true;
		if (unit_is(u, UnitTypes::Zerg_Infested_Command_Center)) return true;
		if (unit_is(u, UnitTypes::Zerg_Hatchery)) return true;
		if (unit_is(u, UnitTypes::Zerg_Lair)) return true;
		if (unit_is(u, UnitTypes::Zerg_Hive)) return true;
		if (unit_is(u, UnitTypes::Protoss_Nexus)) return true;
		if (unit_is(u, UnitTypes::Protoss_Gateway)) return true;
		if (unit_is(u, UnitTypes::Protoss_Stargate)) return true;
		if (unit_is(u, UnitTypes::Protoss_Robotics_Facility)) return true;
		return false;
	}

	void set_unit_tiles_occupied(unit_type_autocast ut, xy position) {
		xy size(ut->placement_size.x / 32u, ut->placement_size.y / 32u);
		xy pos(position.x / 32u, position.y / 32u);
		tiles_flags_or(pos.x - size.x / 2, pos.y - size.y / 2, size.x, size.y, tile_t::flag_occupied);
	}

	void set_unit_tiles_unoccupied(unit_type_autocast ut, xy position) {
		xy size(ut->placement_size.x / 32u, ut->placement_size.y / 32u);
		xy pos(position.x / 32u, position.y / 32u);
		tiles_flags_and(pos.x - size.x / 2, pos.y - size.y / 2, size.x, size.y, ~tile_t::flag_occupied);
	}

	bool initialize_unit_type(unit_t* u, const unit_type_t* unit_type, xy pos, int owner) {

		auto ius = make_thingy_setter(iscript_unit, u);
		if (!initialize_flingy(u, unit_type->flingy, pos, owner, 0_dir)) return false;

		u->owner = owner;
		u->order_type = get_order_type(Orders::Fatal);
		u->order_state = 0;
		u->order_signal = 0;
		u->order_unit_type = nullptr;
		u->main_order_timer = 0;
		u->ground_weapon_cooldown = 0;
		u->air_weapon_cooldown = 0;
		u->spell_cooldown = 0;
		u->order_target = {};
		u->unit_type = unit_type;
		u->carrying_flags = 0;
		u->secondary_order_timer = 0;

		if (!iscript_execute_sprite(u->sprite)) {
			u->sprite = nullptr;
			return false;
		}
		u->last_attacking_player = 8;
		u->shield_points = fp8::integer(u->unit_type->shield_points);
		if (unit_is(u, UnitTypes::Protoss_Shield_Battery)) u->energy = fp8::integer(100);
		else u->energy = unit_max_energy(u) / 4;

		u->sprite->elevation_level = unit_type->elevation_level;
		u_set_status_flag(u, unit_t::status_flag_grounded_building, ut_building(u));
		u_set_status_flag(u, unit_t::status_flag_flying, ut_flyer(u));
		u_set_status_flag(u, unit_t::status_flag_can_turn, ut_can_turn(u));
		u_set_status_flag(u, unit_t::status_flag_can_move, ut_can_move(u));
		u_set_status_flag(u, unit_t::status_flag_ground_unit, !ut_flyer(u));
		if (u->unit_type->elevation_level < 12) u->pathing_flags |= 1;
		else u->pathing_flags &= ~1;
		u->building.addon = nullptr;
		u->building.addon_build_type = nullptr;
		u->building.upgrade_research_time = 0;
		u->building.researching_type = nullptr;
		u->building.upgrading_type = nullptr;
		u->building.larva_timer = 0;
		u->building.is_landing = false;
		u->building.creep_timer = 0;
		u->building.upgrading_level = 0;
		bool building_union_used = false;
		if (ut_resource(u)) {
			if (building_union_used) error("building union already used");
			building_union_used = true;
			u->building.resource.resource_count = 0;
			u->building.resource.resource_iscript = 0;
			u->building.resource.is_being_gathered = false;
			u->building.resource.gather_queue.clear();
		} else if (unit_is_mineral_field(unit_type)) error("mineral field is not a resource");
		if (unit_is(u, UnitTypes::Terran_Nuclear_Silo)) {
			if (building_union_used) error("building union already used");
			building_union_used = true;
			u->building.silo = {};
		}
		if (unit_is_hatchery(u)) {
			if (building_union_used) error("building union already used");
			building_union_used = true;
			u->building.hatchery.larva_spawn_side_values = {};
		}
		if (unit_is_nydus(u)) {
			if (building_union_used) error("building union already used");
			building_union_used = true;
			u->building.nydus.exit = nullptr;
		}
		if (unit_is(u, UnitTypes::Protoss_Pylon)) {
			if (building_union_used) error("building union already used");
			building_union_used = true;
			u->building.pylon.psionic_matrix_link = {nullptr, nullptr};
			u->building.pylon.psi_field_sprite = nullptr;
		}
		bool unit_union_used = false;
		if (unit_is_ghost(u)) {
			if (unit_union_used) error("unit union already used");
			unit_union_used = true;
			u->ghost.nuke_dot = nullptr;
		}
		if (unit_is_vulture(u)) {
			if (unit_union_used) error("unit union already used");
			unit_union_used = true;
			u->vulture.spider_mine_count = 0;
		}
		if (unit_is_carrier(u)) {
			if (unit_union_used) error("unit union already used");
			unit_union_used = true;
			u->carrier.inside_units.clear();
			u->carrier.outside_units.clear();
			u->carrier.inside_count = 0;
			u->carrier.outside_count = 0;
		}
		if (unit_is_reaver(u)) {
			if (unit_union_used) error("unit union already used");
			unit_union_used = true;
			u->reaver.inside_units.clear();
			u->reaver.outside_units.clear();
			u->reaver.inside_count = 0;
			u->reaver.outside_count = 0;
		}
		if (unit_is_fighter(u)) {
			if (unit_union_used) error("unit union already used");
			unit_union_used = true;
			u->fighter.parent = nullptr;
			u->fighter.fighter_link = {nullptr, nullptr};
			u->fighter.is_outside = false;
		}

		if (ut_worker(u)) {
			u->worker.powerup = nullptr;
			u->worker.target_resource_position = {};
			u->worker.target_resource_unit = nullptr;
			u->worker.repair_timer = 0;
			u->worker.is_gathering = false;
			u->worker.resources_carried = 0;
			u->worker.gather_target = nullptr;
		}

		u->path = nullptr;
		u->movement_state = movement_states::UM_Init;
		u->move_target_timer = 0;
		u_set_status_flag(u, unit_t::status_flag_invincible, ut_invincible(u));

		u->damage_overlay_state = damage_overlay_states(u);

		if (u->unit_type->build_time == 0) {
			u->remaining_build_time = 1;
			u->hp_construction_rate = 1_fp8;
		} else {
			u->remaining_build_time = u->unit_type->build_time;
			u->hp_construction_rate = (u->unit_type->hitpoints - u->unit_type->hitpoints / 10 + fp8::from_raw(u->unit_type->build_time) - 1_fp8) / u->unit_type->build_time;
			if (u->hp_construction_rate == 0_fp8) u->hp_construction_rate = 1_fp8;
		}
		if (u->unit_type->has_shield && u_grounded_building(u)) {
			fp8 max_shields = fp8::integer(u->unit_type->shield_points);
			u->shield_points = max_shields / 10;
			if (u->unit_type->build_time == 0) {
				u->shield_construction_rate = fp8::integer(1);
			} else {
				u->shield_construction_rate = (max_shields - u->shield_points) / u->unit_type->build_time;
				if (u->shield_construction_rate == 0_fp8) u->shield_construction_rate = fp8::integer(1);
			}
		}
		update_unit_speed_upgrades(u);
		update_unit_speed(u);

		return true;
	}

	void kill_interceptors(unit_t* u) {
		if (!unit_is_carrier(u)) return;
		for (auto i = u->carrier.inside_units.begin(); i != u->carrier.inside_units.end();) {
			unit_t* n = &*i++;
			kill_unit(n);
		}
		for (auto i = u->carrier.outside_units.begin(); i != u->carrier.outside_units.end();) {
			unit_t* n = &*i++;
			n->fighter.parent = nullptr;
			set_remove_timer(n, lcg_rand(39, 15, 45));
			u->carrier.outside_units.remove(*n);
			--u->carrier.outside_count;
			n->fighter.fighter_link = {nullptr, nullptr};
		}
	}

	void destroy_unit_impl(unit_t* u) {
		if (unit_is_carrier(u)) {
			kill_interceptors(u);
		} else if (unit_is_reaver(u)) {
			for (auto i = u->reaver.inside_units.begin(); i != u->reaver.inside_units.end();) {
				unit_t* n = &*i++;
				kill_unit(n);
			}
			for (auto i = u->reaver.outside_units.begin(); i != u->reaver.outside_units.end();) {
				unit_t* n = &*i++;
				n->fighter.parent = nullptr;
				u->reaver.outside_units.remove(*n);
				--u->reaver.outside_count;
				n->fighter.fighter_link = {nullptr, nullptr};
			}
		} else if (unit_is_ghost(u)) {
			if (u->ghost.nuke_dot) {
				sprite_run_anim(u->ghost.nuke_dot->sprite, iscript_anims::Death);
				u->ghost.nuke_dot = nullptr;
			}
		} else if (ut_resource(u)) {
			u->building.resource.is_being_gathered = false;
			while (!u->building.resource.gather_queue.empty()) {
				unit_t* gatherer = &u->building.resource.gather_queue.front();
				gatherer->worker.gather_target = nullptr;
				u->building.resource.gather_queue.pop_front();
			}
		} else {
			switch (u->unit_type->id) {
			case UnitTypes::Protoss_Interceptor:
			case UnitTypes::Protoss_Scarab:
				if (u_completed(u)) {
					unit_t* parent = u->fighter.parent;
					if (u->fighter.parent) {
						if (u->fighter.is_outside) {
							if (unit_is(parent, UnitTypes::Protoss_Carrier)) {
								parent->carrier.outside_units.remove(*u);
								--parent->carrier.outside_count;
							} else if (unit_is(parent, UnitTypes::Protoss_Reaver)) {
								parent->reaver.outside_units.remove(*u);
								--parent->reaver.outside_count;
							}
						} else {
							if (unit_is(parent, UnitTypes::Protoss_Carrier)) {
								parent->carrier.inside_units.remove(*u);
								--parent->carrier.inside_count;
							} else if (unit_is(parent, UnitTypes::Protoss_Reaver)) {
								parent->reaver.inside_units.remove(*u);
								--parent->reaver.inside_count;
							}
						}
					}
					u->fighter.parent = nullptr;
					u->fighter.fighter_link = {nullptr, nullptr};
				}
				break;
			case UnitTypes::Terran_Nuclear_Silo:
				if (u->building.silo.nuke) {
					kill_unit(u->building.silo.nuke);
					u->building.silo.nuke = nullptr;
				}
				break;
			case UnitTypes::Terran_Nuclear_Missile:
				if (u->connected_unit && unit_is(u->connected_unit, UnitTypes::Terran_Nuclear_Silo)) {
					u->connected_unit->building.silo = {};
					u->connected_unit = nullptr;
				}
				break;
			case UnitTypes::Protoss_Pylon:
				if (u->building.pylon.psionic_matrix_link.first) {
					st.psionic_matrix_units.remove(*u);
					u->building.pylon.psionic_matrix_link = {nullptr, nullptr};
				}
				st.update_psionic_matrix = true;
				if (u->building.pylon.psi_field_sprite) {
					destroy_sprite(u->building.pylon.psi_field_sprite);
					u->building.pylon.psi_field_sprite = nullptr;
				}
			case UnitTypes::Zerg_Nydus_Canal:
				if (u->building.nydus.exit) {
					unit_t* exit = u->building.nydus.exit;
					u->building.nydus.exit = nullptr;
					exit->building.nydus.exit = nullptr;
					kill_unit(exit);
				}
			default:
				break;
			}
		}
		if (u_loaded(u)) unit_unload_impl(u, true);
		for (unit_t* n : loaded_units(u)) {
			if (unit_is(u, UnitTypes::Terran_Bunker)) {
				if (!unit_unload(n)) kill_unit(n);
			} else kill_unit(n);
		}
		u->loaded_units = {};
		for (unit_t* n : ptr(st.visible_units)) {
			remove_target_references(n, u);
		}
		for (unit_t* n : ptr(st.hidden_units)) {
			remove_target_references(n, u);
		}
		for (bullet_t* n : ptr(st.active_bullets)) {
			remove_target_references(n, u);
		}
		if (ut_worker(u)) {
			if (u->worker.gather_target) {
				if (u->worker.is_gathering) {
					gather_queue_next(u, u->worker.gather_target);
				} else {
					u->worker.gather_target->building.resource.gather_queue.remove(*u);
					u->worker.gather_target = nullptr;
				}
			}
		}
		if (ut_turret(u)) {
			increment_unit_counts(u, -1);
			u->subunit = nullptr;
			st.player_units[u->owner].remove(*u);
			bw_insert_list(st.dead_units, *u);
		} else {
			auto tid = u->unit_type->id;
			if (tid == UnitTypes::Terran_Refinery || tid == UnitTypes::Zerg_Extractor || tid == UnitTypes::Protoss_Assimilator) {
				if (tid == UnitTypes::Zerg_Extractor) remove_extractor_creep(u, u->sprite->position, true);
				if (u_completed(u)) add_completed_unit(u, -1, false);
				u_unset_status_flag(u, unit_t::status_flag_completed);
				morph_unit(u, get_unit_type(UnitTypes::Resource_Vespene_Geyser));
				u->order_type = u->unit_type->human_ai_idle;
				u->order_target = {};
				u->order_state = 0;
				u->order_unit_type = nullptr;
				u->hp = u->unit_type->hitpoints;
				set_unit_owner(u, 11, false);
				set_sprite_owner(u, 11);
				u_set_status_flag(u, unit_t::status_flag_completed);
				add_completed_unit(u, 1, true);
			} else {
				stop_unit(u);
				if (!us_hidden(u)) {
					unit_finder_remove(u);
					if (u_grounded_building(u)) set_unit_tiles_unoccupied(u, u->sprite->position);
					if (u_flying(u)) decrement_repulse_field(u);
				}
				if (!ut_building(u)) {
					if (u->cloak_counter) u->cloak_counter = 0;
					if (u->cloaked_unit_link.first) {
						st.cloaked_units.remove(*u);
						u->cloaked_unit_link = {nullptr, nullptr};
					}
					if (u_passively_cloaked(u)) u_unset_status_flag(u, unit_t::status_flag_passively_cloaked);
					if (u_requires_detector(u)) {
						// todo: callback for sound
					}
				}
				if (u_grounded_building(u)) {
					if (u->secondary_order_type->id == Orders::BuildAddon) {
						if (u->current_build_unit && !u_completed(u->current_build_unit)) {
							cancel_building_unit(u->current_build_unit);
						}
					}
					if (u->building.addon) building_abandon_addon(u);
					if (u->building.researching_type) cancel_research(u, true);
					if (u->building.upgrading_type) cancel_upgrade(u, true);
					if (unit_race(u) == race_t::zerg) {
						if (unit_is_morphing_building(u)) u_set_status_flag(u, unit_t::status_flag_completed);
						if (!remove_creep_provider(u)) u_set_status_flag(u, unit_t::status_flag_4000);
					}
					if (!u->build_queue.empty()) cancel_build_queue(u);
				}
				if (ut_flying_building(u) && u->building.is_landing) {
					set_unit_tiles_unoccupied(u, u->order_target.pos);
				}
				if (u->path) {
					free_path(u->path);
					u->path = nullptr;
				}
				if (u->current_build_unit) {
					if (u->secondary_order_type->id == Orders::Train || u->secondary_order_type->id == Orders::TrainFighter) {
						kill_unit(u->current_build_unit);
					}
				}
				set_secondary_order(u, get_order_type(Orders::Nothing));
				drop_carried_items(u);
				on_unit_deselect(u);
				on_unit_destroy(u);
				increment_unit_counts(u, -1);
				if (u_completed(u)) add_completed_unit(u, -1, false);
				st.player_units[u->owner].remove(*u);
				if (unit_is_map_revealer(u)) st.map_revealer_units.remove(*u);
				else if (us_hidden(u)) st.hidden_units.remove(*u);
				else st.visible_units.remove(*u);
				bw_insert_list(st.dead_units, *u);
			}
		}
	}

	void destroy_unit(unit_t* u) {
		unit_t* turret = unit_turret(u);
		if (turret) {
			destroy_unit(turret);
			u->subunit = nullptr;
		}
		destroy_unit_impl(u);
		u->order_type = get_order_type(Orders::Die);
		u->order_state = 1;
		destroy_sprite(u->sprite);
		u->sprite = nullptr;
	}

	bool initialize_unit(unit_t* u, const unit_type_t* unit_type, xy pos, int owner) {

		u->order_queue.clear();

		u->auto_target_unit = nullptr;
		u->connected_unit = nullptr;

		u->order_queue_count = 0;
		u->order_process_timer = 0;
		u->unknown_0x086 = 0;
		u->attack_notify_timer = 0;
		u->previous_unit_type = nullptr;
		u->last_event_timer = 0;
		u->last_event_color = 0;
		u->rank_increase = 0;
		u->kill_count = 0;

		u->remove_timer = 0;
		u->defensive_matrix_hp = 0_fp8;
		u->defensive_matrix_timer = 0;
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
		u->blinded_by = 0;
		u->maelstrom_timer = 0;
		u->acid_spore_count = 0;
		u->acid_spore_time = {};
		u->status_flags = 0;
		u->user_action_flags = 0;
		u->pathing_flags = 0;
		u->previous_hp = 1;

		u->building.rally = {};

		if (!initialize_unit_type(u, unit_type, pos, owner)) return false;

		new (&u->build_queue) static_vector<const unit_type_t*, 5>();
		++u->unit_id_generation;
		u->wireframe_randomizer = lcg_rand(15) & 0xff;
		if (ut_turret(u)) u->hp = 1_fp8;
		else u->hp = u->unit_type->hitpoints / 10;
		if (u_grounded_building(u)) u->order_type = u->unit_type->human_ai_idle;
		else u->order_type = get_order_type(Orders::Nothing);
		set_secondary_order(u, get_order_type(Orders::Nothing));
		u->unit_finder_bounding_box = { {-1, -1}, {-1, -1} };
		st.player_units[owner].push_front(*u);
		increment_unit_counts(u, 1);

		if (u_grounded_building(u)) {
			unit_finder_insert(u);
			set_unit_tiles_occupied(u, u->sprite->position);
			check_unit_collision(u);
			if (u_flying(u)) increment_repulse_field(u);
			set_construction_graphic(u, true);
			set_sprite_visibility(u->sprite, 0);
		} else {
			u->sprite->flags |= sprite_t::flag_hidden;
			set_sprite_visibility(u->sprite, 0);
		}
		u->detected_flags = 0xffffffff;
		if (ut_turret(u)) {
			u->sprite->flags |= sprite_t::flag_turret;
		} else {
			if (!us_hidden(u)) {
				refresh_unit_vision(u);
			}
		}

		if (unit_is_vulture(u)) {
			u->vulture.spider_mine_count = 3;
		}

		return true;
	}

	unit_t* create_unit(const unit_type_t* unit_type, xy pos, int owner) {
		if (!unit_type) error("attempt to create unit of null type");

		lcg_rand(14);
		auto get_new = [&](const unit_type_t* unit_type) {
			unit_t* u = st.units_container.top();
			if (!u) {
				st.last_error = 61; // Cannot create more units
				return (unit_t*)nullptr;
			}
			if (!is_in_map_bounds(unit_type, pos)) {
				st.last_error = 0;
				return (unit_t*)nullptr;
			}
			if (!initialize_unit(u, unit_type, pos, owner)) {
				st.last_error = 62; // Unable to create unit
				return (unit_t*)nullptr;
			}
			st.units_container.pop();
			return u;
		};
		unit_t* u = get_new(unit_type);
		if (!u) return nullptr;
		if (u_grounded_building(u)) bw_insert_list(st.visible_units, *u);
		else bw_insert_list(st.hidden_units, *u);

		if (unit_type->id < UnitTypes::Terran_Command_Center && unit_type->turret_unit_type) {
			unit_t* su = get_new(unit_type->turret_unit_type);
			if (!su) {
				destroy_unit(u);
				return nullptr;
			}
			u->subunit = su;
			su->subunit = u;
			set_image_offset(su->sprite->main_image, get_image_lo_offset(u->sprite->main_image, 2, 0));
			if (ut_turret(u)) error("unit type %d has a turret but is also flagged as a turret", (int)u->unit_type->id);
			if (!ut_turret(su)) error("unit type %d was created as a turret but is not flagged as one", (int)su->unit_type->id);
		} else {
			u->subunit = nullptr;
		}
		return u;
	}

	unit_t* create_unit(UnitTypes unit_type_id, xy pos, int owner) {
		if ((size_t)unit_type_id >= 228) error("attempt to create unit with invalid id %d", unit_type_id);
		return create_unit(get_unit_type(unit_type_id), pos, owner);
	}

	void replace_sprite_images(sprite_t* sprite, const image_type_t* new_image_type, direction_t heading) {
		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t* image = &*i++;
			destroy_image(image);
		}

		create_image(new_image_type, sprite, {}, image_order_above);

		for (image_t* img : ptr(sprite->images)) {
			set_image_heading(img, heading);
		}
	}

	void apply_unit_effects(unit_t*u) {
		if (u->defensive_matrix_timer) {
			if (u_invincible(u) || !unit_tech_target_valid(u, get_tech_type(TechTypes::Defensive_Matrix), u)) {
				u->defensive_matrix_hp = 0_fp8;
				u->defensive_matrix_timer = 0;
			} else {
				create_defensive_matrix_image(u);
			}
		}

		if (u->lockdown_timer) {
			auto timer = u->lockdown_timer;
			u->lockdown_timer = 0;
			lockdown_unit(u);
			u->lockdown_timer = timer;
		}
		if (u->maelstrom_timer) {
			auto timer = u->maelstrom_timer;
			u->maelstrom_timer = 0;
			maelstrom_unit(u);
			u->maelstrom_timer = timer;
		}
		if (u->irradiate_timer) {
			if (!weapon_can_target_unit(get_weapon_type(WeaponTypes::Irradiate), u)) {
				u->irradiate_timer = 0;
				u->irradiated_by = nullptr;
			} else {
				int timer = u->irradiate_timer;
				u->irradiate_timer = 0;
				irradiate_unit(u, u->irradiated_by, u->irradiate_owner);
				u->irradiate_timer = timer;
			}
		}
		if (u->ensnare_timer) {
			auto timer = u->ensnare_timer;
			u->ensnare_timer = 0;
			ensnare_unit(u);
			u->ensnare_timer = timer;
		}
		if (u->stasis_timer) {
			auto timer = u->stasis_timer;
			u->stasis_timer = 0;
			stasis_unit(u);
			u->stasis_timer = timer;
		}
		if (u->plague_timer) {
			auto timer = u->plague_timer;
			u->plague_timer = 0;
			plague_unit(u);
			u->plague_timer = timer;
		}
		if (u->acid_spore_count) {
			create_image(get_image_type(acid_spore_image(u)), (u->subunit ? u->subunit : u)->sprite, {}, image_order_top);
		}
	}

	int damage_overlay_states(const unit_t* u) const {
		if (u->sprite->main_image->image_type->damage_filename_index == 0) return 0;
		int n = 0;
		for (auto& v : global_st.image_lo_offsets.at((size_t)u->sprite->main_image->image_type->id).at(1)->at(0)) {
			if (v != xy(127, 127)) ++n;
		}
		return 2 * n;
	}

	void set_construction_graphic(unit_t* u, bool animated) {

		bool requires_detector_or_cloaked = u_requires_detector(u) || u_cloaked(u);
		int modifier_data1 = 0;
		int modifier_data2 = 0;
		if (requires_detector_or_cloaked) {
			modifier_data1 = u->sprite->main_image->modifier_data1;
			modifier_data2 = u->sprite->main_image->modifier_data2;
		}
		auto ius = make_thingy_setter(iscript_unit, u);
		const image_type_t* construction_image = u->unit_type->construction_animation;
		if (!animated || !construction_image || (int)construction_image->id == 0) construction_image = u->sprite->sprite_type->image;
		replace_sprite_images(u->sprite, construction_image, u->heading);

		if (requires_detector_or_cloaked) {
			set_sprite_cloak_modifier(u->sprite, u_requires_detector(u), u_cloaked(u), u_burrowed(u), modifier_data1, modifier_data2);
		}

		apply_unit_effects(u);
		u->damage_overlay_state = damage_overlay_states(u);
	}

	void set_unit_heading(unit_t* u, direction_t heading) {
		u->next_velocity_direction = heading;
		u->heading = heading;
		u->current_velocity_direction = heading;
		u->velocity = direction_xy(heading, u->current_speed);
		set_next_target_waypoint(u, u->sprite->position);
		for (image_t* img : ptr(u->sprite->images)) {
			set_image_heading(img, heading);
		}

	}

	void finish_building_unit(unit_t*u) {
		if (u->remaining_build_time) {
			u->hp = u->unit_type->hitpoints;
			u->shield_points = fp8::integer(u->unit_type->shield_points);
			u->remaining_build_time = 0;
		}
		if (u_grounded_building(u)) {
			u->parasite_flags = 0;
			u->blinded_by = 0;
			set_construction_graphic(u, false);
			sprite_run_anim(u->sprite, iscript_anims::Built);
		} else {
			if (u_can_turn(u)) {
				int dir = u->unit_type->unit_direction;
				if (dir == 32) dir = lcg_rand(36) % 32;
				set_unit_heading(u, 8_dir * dir);
			}
			if (unit_is_trap(u)) {
				show_unit(u);
			}
		}

	}

	std::pair<bool, xy> find_unit_placement(const unit_t* u, xy pos, rect bounds, bool terrain_displaces_unit) const {
		if (bounds.from.x < 0) bounds.from.x = 0;
		if (bounds.from.y < 0) bounds.from.y = 0;
		if (bounds.to.x >= (int)game_st.map_width) bounds.to.x = (int)game_st.map_width - 1;
		if (bounds.to.y >= (int)game_st.map_height - 32) bounds.to.y = (int)game_st.map_height - 32 - 1;
		auto blocking_unit_pred = [&](const unit_t* target) {
			if (unit_dead(target)) return false;
			if (u_no_collide(u) || u_no_collide(target)) return false;
			if (ut_powerup(u)) {
				return u_grounded_building(target) || ut_powerup(target);
			}
			if (unit_is(target, UnitTypes::Terran_Starport)) {
				if (unit_is(u, UnitTypes::Terran_Wraith)) return true;
				if (unit_is(u, UnitTypes::Terran_Dropship)) return true;
				if (unit_is(u, UnitTypes::Terran_Science_Vessel)) return true;
				if (unit_is(u, UnitTypes::Terran_Battlecruiser)) return true;
				if (unit_is(u, UnitTypes::Terran_Valkyrie)) return true;
			}
			if (unit_is(target, UnitTypes::Protoss_Robotics_Facility)) {
				if (unit_is(u, UnitTypes::Protoss_Shuttle)) return true;
				if (unit_is(u, UnitTypes::Protoss_Observer)) return true;
			}
			if (unit_is(target, UnitTypes::Protoss_Stargate)) {
				if (unit_is(u, UnitTypes::Protoss_Scout)) return true;
				if (unit_is(u, UnitTypes::Protoss_Carrier)) return true;
				if (unit_is(u, UnitTypes::Protoss_Arbiter)) return true;
				if (unit_is(u, UnitTypes::Protoss_Corsair)) return true;
			}
			return u_flying(u) == u_flying(target);
		};
		const unit_t* blocking_unit = find_unit(unit_inner_bounding_box(u, pos), blocking_unit_pred);
		if (!blocking_unit && is_in_bounds(unit_inner_bounding_box(u, pos), {{0, 0}, {(int)game_st.map_width, (int)game_st.map_height - 32}})) {
			if (u_flying(u) || unit_type_can_fit_at(u->unit_type, pos)) return {true, pos};
			if (!terrain_displaces_unit) {
				st.last_error = 60;
				return {false, {}};
			}
		}

		xy find_result;

		auto find = [&](xy pos, int width, int height) {
			rect bb;
			bb.from.x = pos.x / 8u * 8u;
			bb.from.y = pos.y / 8u * 8u;
			bb.to.x = (pos.x + width + 7) / 8u * 8u;
			bb.to.y = (pos.y + height + 7) / 8u * 8u;
			if (bb.from.x < bounds.from.x) bb.from.x = bounds.from.x;
			if (bb.from.y < bounds.from.y) bb.from.y = bounds.from.y;
			if (bb.to.x > bounds.to.x) bb.to.x = bounds.to.x;
			if (bb.to.y > bounds.to.y) bb.to.y = bounds.to.y;

			if (width > (u->unit_type->dimensions.from.x + u->unit_type->dimensions.to.x + 1) * 2) {
				bb.from.x += (u->unit_type->dimensions.from.x + u->unit_type->dimensions.to.x + 1 + 7) / 8u * 8u;
			}

			rect search_bb = unit_inner_bounding_box(u, {bb.from.x, bb.to.y});

			for (int x = bb.from.x; x <= bb.to.x;) {
				if (is_inner_bb_move_target_in_valid_bounds(search_bb)) {
					const unit_t* n = find_unit(search_bb, blocking_unit_pred);
					if (n) {
						int inc = n->sprite->position.x + n->unit_type->dimensions.to.x + 1 - search_bb.from.x;
						inc += (8 - ((x + inc) & 7)) & 7;
						search_bb.from.x += inc;
						search_bb.to.x += inc;
						x += inc;
						continue;
					} else {
						xy pos{x, bb.to.y};
						if (is_reachable(u, pos)) {
							if (u_flying(u) || unit_type_can_fit_at(u->unit_type, pos)) {
								find_result = pos;
								return true;
							}
						}
					}
				}
				search_bb.from.x += 8;
				search_bb.to.x += 8;
				x += 8;
			}

			search_bb = unit_inner_bounding_box(u, {bb.to.x, bb.to.y});

			for (int y = bb.to.y; y >= bb.from.y;) {
				if (is_inner_bb_move_target_in_valid_bounds(search_bb)) {
					const unit_t* n = find_unit(search_bb, blocking_unit_pred);
					if (n) {
						int dec = search_bb.to.y - (n->sprite->position.y - n->unit_type->dimensions.from.y - 1);
						dec += (8 - ((y - dec) & 7)) & 7;
						search_bb.from.y -= dec;
						search_bb.to.y -= dec;
						y -= dec;
						continue;
					} else {
						xy pos{bb.to.x, y};
						if (is_reachable(u, pos)) {
							if (u_flying(u) || unit_type_can_fit_at(u->unit_type, pos)) {
								find_result = pos;
								return true;
							}
						}
					}
				}
				search_bb.from.y -= 8;
				search_bb.to.y -= 8;
				y -= 8;
			}

			if (width > (u->unit_type->dimensions.from.x + u->unit_type->dimensions.to.x + 1) * 2) {
				bb.from.x -= (u->unit_type->dimensions.from.x + u->unit_type->dimensions.to.x + 1 + 7) / 8u * 8u;
			}

			search_bb = unit_inner_bounding_box(u, {bb.to.x, bb.from.y});

			for (int x = bb.to.x; x >= bb.from.x;) {
				if (is_inner_bb_move_target_in_valid_bounds(search_bb)) {
					const unit_t* n = find_unit(search_bb, blocking_unit_pred);
					if (n) {
						int dec = search_bb.to.x - (n->sprite->position.x - n->unit_type->dimensions.from.x - 1);
						dec += (8 - ((x - dec) & 7)) & 7;
						search_bb.from.x -= dec;
						search_bb.to.x -= dec;
						x -= dec;
						continue;
					} else {
						xy pos{x, bb.from.y};
						if (is_reachable(u, pos)) {
							if (u_flying(u) || unit_type_can_fit_at(u->unit_type, pos)) {
								find_result = pos;
								return true;
							}
						}
					}
				}
				search_bb.from.x -= 8;
				search_bb.to.x -= 8;
				x -= 8;
			}

			search_bb = unit_inner_bounding_box(u, {bb.from.x, bb.from.y});

			for (int y = bb.from.y; y <= bb.to.y;) {
				if (is_inner_bb_move_target_in_valid_bounds(search_bb)) {
					const unit_t* n = find_unit(search_bb, blocking_unit_pred);
					if (n) {
						int inc = n->sprite->position.y + n->unit_type->dimensions.to.y + 1 - search_bb.from.y;
						inc += (8 - ((y + inc) & 7)) & 7;
						search_bb.from.y += inc;
						search_bb.to.y += inc;
						y += inc;
						continue;
					} else {
						xy pos{bb.from.x, y};
						if (is_reachable(u, pos)) {
							if (u_flying(u) || unit_type_can_fit_at(u->unit_type, pos)) {
								find_result = pos;
								return true;
							}
						}
					}
				}
				search_bb.from.y += 8;
				search_bb.to.y += 8;
				y += 8;
			}

			return false;
		};


		int width = 8;
		int height = 8;
		if (blocking_unit) {
			int new_width = u->unit_type->dimensions.from.x + blocking_unit->unit_type->dimensions.to.x + 2;
			int new_height = u->unit_type->dimensions.from.y + blocking_unit->unit_type->dimensions.to.y + 2;
			if (new_width > width) width = new_width;
			if (new_height > height) height = new_height;
		}
		while (true) {
			xy npos = pos - xy(width, height);
			if (npos.x < bounds.from.x && npos.y < bounds.from.y && pos.x + width > bounds.to.x && pos.y + height > bounds.to.y)  break;
			if (find(npos, width * 2, height * 2)) return {true, find_result};
			width += 16;
			height += 16;
		}
		st.last_error = 60;
		return {false, {}};
	}

	std::pair<bool, xy> find_unit_placement(const unit_t* u, xy pos, bool terrain_displaces_unit) const {
		return find_unit_placement(u, pos, {pos - xy(128, 128), pos + xy(127, 127)}, terrain_displaces_unit);
	}

	void move_unit(unit_t* u, xy pos) {
		pos = restrict_move_target_to_valid_bounds(u, pos);
		u->position = pos;
		u->exact_position = to_xy_fp8(pos);
		move_sprite(u->sprite, pos);
		if (u->order_type->id != Orders::Die) {
			set_unit_move_target(u, pos);
			set_next_target_waypoint(u, pos);
		}
		if (!ut_turret(u)) {
			unit_finder_reinsert(u);
			if (!us_hidden(u)) check_unit_collision(u);
		}
	}

	bool place_completed_unit(unit_t* u) {
		if (!us_hidden(u)) return true;
		bool res;
		xy pos;
		std::tie(res, pos) = find_unit_placement(u, u->sprite->position, false);
		if (!res) {
			display_last_error_for_player(u->owner);
			return false;
		}
		move_unit(u, pos);
		unit_t* turret = unit_turret(u);
		if (turret) {
			add_completed_unit(turret, 1, false);
			u_set_status_flag(turret, unit_t::status_flag_completed);
			turret->hp = turret->unit_type->hitpoints;
			move_unit(turret, pos);
		}
		return true;
	}

	void add_completed_unit(unit_t* u, int count, bool increment_score) {
		if (u_hallucination(u)) return;
		if (ut_turret(u)) return;

		st.completed_unit_counts[u->owner][u->unit_type->id] += count;
		auto race = unit_race(u);
		if (race != race_t::none) {
			st.supply_available[u->owner][(size_t)race] += u->unit_type->supply_provided * count;
		}

		if (u->unit_type->group_flags & GroupFlags::Factory) {
			st.completed_factory_counts[u->owner] += count;
		}
		if (u->unit_type->group_flags & GroupFlags::Men) {
			st.completed_non_building_counts[u->owner] += count;
		} else if (u->unit_type->group_flags & GroupFlags::Building) {
			st.completed_building_counts[u->owner] += count;
		}

		if (increment_score) {
			if (u->owner != 11) {
				if (u->unit_type->group_flags & GroupFlags::Men) {
					bool morphed = false;
					if (unit_is(u, UnitTypes::Zerg_Guardian)) morphed = true;
					if (unit_is(u, UnitTypes::Zerg_Devourer)) morphed = true;
					if (unit_is(u, UnitTypes::Protoss_Dark_Archon)) morphed = true;
					if (unit_is(u, UnitTypes::Protoss_Archon)) morphed = true;
					if (unit_is(u, UnitTypes::Zerg_Lurker)) morphed = true;
					if (!morphed) st.total_non_buildings_ever_completed[u->owner] += count;
					st.unit_score[u->owner] += u->unit_type->build_score * count;
				} else if (u->unit_type->group_flags & GroupFlags::Building) {
					if (!unit_type_is_morphing_building(u)) st.total_buildings_ever_completed[u->owner] += count;
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
		st.orders_container.push(o);
		--st.active_orders_size;
	}

	order_t* create_order(const order_type_t* order_type, order_target_t target) {
		order_t* o = st.orders_container.top();
		if (!o) return nullptr;
		st.orders_container.pop();
		++st.active_orders_size;
		o->order_type = order_type;
		o->target = target;
		return o;
	}

	void queue_order(intrusive_list<order_t, default_link_f>::iterator pos, unit_t* u, const order_type_t* order_type, order_target_t target) {
		order_t* o = create_order(order_type, target);
		if (!o) {
			local_unit_status_error(u, 872);
			return;
		}
		if (o->order_type->highlight != -1) ++u->order_queue_count;
		u->order_queue.insert(pos, *o);
	}

	void queue_order_back(unit_t* u, const order_type_t* order_type, order_target_t target) {
		queue_order(u->order_queue.end(), u, order_type, target);
	}

	void queue_order_front(unit_t* u, const order_type_t* order_type, order_target_t target) {
		queue_order(u->order_queue.begin(), u, order_type, target);
	}

	void remove_one_order(unit_t* u, const order_type_t* order_type) {
		for (order_t* o : ptr(u->order_queue)) {
			if (o->order_type == order_type) {
				remove_queued_order(u, o);
				break;
			}
		}
	}

	void set_queued_order(unit_t* u, bool interrupt, const order_type_t* order_type, order_target_t target) {
		if (u->order_type->id == Orders::Die) return;
		while (!u->order_queue.empty()) {
			order_t* o = &u->order_queue.back();
			if (!o) break;
			if ((!interrupt || !o->order_type->can_be_interrupted) && o->order_type != order_type) break;
			remove_queued_order(u, o);
		}
		if (order_type->id == Orders::Cloak) {
			error("cloak fixme");
		} else {
			queue_order_back(u, order_type, target);
		}
	}

	void issue_order(unit_t* u, bool queue, const order_type_t* order_type, order_target_t target) {
		if (target.unit) {
			target.position = target.unit->sprite->position;
		}
		if (queue && !order_type->can_be_queued) queue = false;
		if (queue) {
			switch (u->order_type->id) {
			case Orders::Guard:
			case Orders::PlayerGuard:
			case Orders::Nothing:
			case Orders::PickupIdle:
			case Orders::Patrol:
			case Orders::MedicIdle:
				queue = false;
				break;
			default:
				break;
			}
		}
		if (queue) {
			if (u->order_queue_count < 8 && st.active_orders_size < 1800) {
				queue_order_back(u, order_type, target);
			}
		} else {
			set_unit_order(u, order_type, target);
		}
	}

	void iscript_run_to_idle(unit_t* u) {
		u->status_flags &= ~unit_t::status_flag_iscript_nobrk;
		u->sprite->flags &= ~sprite_t::flag_iscript_nobrk;
		auto ius = make_thingy_setter(iscript_unit, u);
		auto ifs = make_thingy_setter(iscript_flingy, u);
		int anim = -1;
		auto sid = u->sprite->sprite_type->id;
		switch (u->sprite->main_image->iscript_state.animation) {
		case iscript_anims::AirAttkInit:
		case iscript_anims::AirAttkRpt:
			anim = iscript_anims::AirAttkToIdle;
			break;
		case iscript_anims::AlmostBuilt:
			if (sid == SpriteTypes::SPRITEID_SCV || sid == SpriteTypes::SPRITEID_Drone || sid == SpriteTypes::SPRITEID_Probe) {
				anim = iscript_anims::GndAttkToIdle;
			}
			break;
		case iscript_anims::GndAttkInit:
		case iscript_anims::GndAttkRpt:
			anim = iscript_anims::GndAttkToIdle;
			break;
		case iscript_anims::SpecialState1:
			if (sid == SpriteTypes::SPRITEID_Medic) anim = iscript_anims::WalkingToIdle;
			break;
		case iscript_anims::CastSpell:
			anim = iscript_anims::WalkingToIdle;
			break;
		}
		if (anim != -1) {
			sprite_run_anim(u->sprite, anim);
		}
		u_unset_movement_flag(u, 8);
	}

	void activate_next_order(unit_t* u) {
		if (u->order_queue.empty()) return;
		if ((u_order_not_interruptible(u) || u_iscript_nobrk(u)) && u->order_queue.front().order_type->id != Orders::Die) return;
		const order_type_t* order_type = u->order_queue.front().order_type;
		order_target_t target = u->order_queue.front().target;
		remove_queued_order(u, &u->order_queue.front());

		u->user_action_flags &= ~1;
		u_unset_status_flag(u, unit_t::status_flag_8);
		u_unset_status_flag(u, unit_t::status_flag_ready_to_attack);
		u_set_status_flag(u, unit_t::status_flag_order_not_interruptible, !order_type->can_be_interrupted);
		u->order_process_timer = 0;
		u->move_target_timer = 0;

		u->order_type = order_type;
		u->order_state = 0;

		if (target.unit) {
			if (unit_dead(target.unit) || !target.unit->sprite) error("attempt to activate order with dead target");
			u->order_target.unit = target.unit;
			u->order_target.pos = target.unit->sprite->position;
			u->order_unit_type = nullptr;
		} else {
			u->order_target.unit = nullptr;
			u->order_target.pos = target.position;
			u->order_unit_type = target.unit_type;
		}
		u->auto_target_unit = nullptr;
		iscript_run_to_idle(u);
		if (!ut_turret(u) && u->subunit && ut_turret(u->subunit)) {
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

	void set_unit_order(unit_t* u, const order_type_t* order_type) {
		u->user_action_flags |= 1;
		set_queued_order(u, true, order_type, order_target_t {});
		activate_next_order(u);
	}

	void set_unit_order(unit_t* u, const order_type_t* order_type, order_target_t target) {
		u->user_action_flags |= 1;
		set_queued_order(u, true, order_type, target);
		activate_next_order(u);
	}

	void set_unit_order(unit_t* u, const order_type_t* order_type, unit_t* target_unit) {
		u->user_action_flags |= 1;
		order_target_t target;
		target.unit = target_unit;
		if (target_unit) {
			target.position = target_unit->sprite->position;
		}
		set_queued_order(u, true, order_type, target);
		activate_next_order(u);
	}

	void set_unit_order(unit_t* u, const order_type_t* order_type, xy position) {
		u->user_action_flags |= 1;
		order_target_t target;
		target.position = position;
		set_queued_order(u, true, order_type, target);
		activate_next_order(u);
	}

	bool is_intersecting(rect a, rect b) const {
		if (b.from.x > a.to.x) return false;
		if (b.from.y > a.to.y) return false;
		if (a.from.x > b.to.x) return false;
		if (a.from.y > b.to.y) return false;
		return true;
	}

	bool unit_finder_units_intersecting(unit_t* a, unit_t* b) const {
		return is_intersecting(a->unit_finder_bounding_box, b->unit_finder_bounding_box);
	}

	bool unit_finder_unit_in_bounds(unit_t* u, rect bounds) const {
		return is_intersecting(u->unit_finder_bounding_box, bounds);
	}

	void check_unit_collision(unit_t* u) {
		for (unit_t* nu : find_units(unit_sprite_inner_bounding_box(u))) {
			if (u_grounded_building(nu)) {
				u_set_status_flag(u, unit_t::status_flag_collision);
			}
			else if (!u_flying(nu) && (!u_gathering(nu) || u_grounded_building(u))) {
				if (unit_finder_units_intersecting(u, nu)) {
					u_set_status_flag(nu, unit_t::status_flag_collision);
				}
			}
		}
	}

	size_t repulse_index(xy pos) {
		size_t ux = pos.x;
		size_t uy = pos.y;
		ux /= 48;
		uy /= 48;
		return uy * game_st.repulse_field_width + ux;
	}

	void increment_repulse_field(unit_t* u) {
		if (!u_can_move(u)) return;
		if (ut_building(u)) return;
		if (unit_is(u, UnitTypes::Protoss_Interceptor)) return;
		if ((u->repulse_flags & 0xf0) == 0) {
			unsigned int v = lcg_rand(37);
			u->repulse_direction = direction_from_index(v & 0xff);
			v = (v >> 8) & 0xf0;
			if (v == 0) v = 0xf0;
			u->repulse_flags = v;
		}
		u->repulse_flags &= ~7;
		u->repulse_index = repulse_index(u->sprite->position);
		auto& v = st.repulse_field.at(u->repulse_index);
		if (v < 0xff) ++v;
	}

	void decrement_repulse_field(unit_t* u) {
		if (!u_can_move(u)) return;
		if (ut_building(u)) return;
		if (unit_is(u, UnitTypes::Protoss_Interceptor)) return;
		auto& v = st.repulse_field.at(u->repulse_index);
		if (v > 0) --v;
	}

	void show_unit(unit_t* u) {
		if (!us_hidden(u)) return;
		u->sprite->flags &= ~sprite_t::flag_hidden;
		unit_t* turret = unit_turret(u);
		if (turret) turret->sprite->flags &= ~sprite_t::flag_hidden;
		refresh_unit_vision(u);
		update_unit_sprite(u);
		unit_finder_insert(u);
		if (u_grounded_building(u)) set_unit_tiles_occupied(u, u->sprite->position);
		check_unit_collision(u);
		if (u_flying(u)) increment_repulse_field(u);
		reset_movement_state(u);
		if (turret) reset_movement_state(turret);
		st.hidden_units.remove(*u);
		bw_insert_list(st.visible_units, *u);
	}

	void hide_unit(unit_t* u, bool deselect = true) {
		if (us_hidden(u)) return;
		for (unit_t* n : ptr(st.visible_units)) {
			remove_target_references(n, u);
		}
		for (bullet_t* n : ptr(st.active_bullets)) {
			remove_target_references(n, u);
		}
		unit_finder_remove(u);
		if (u_grounded_building(u)) set_unit_tiles_unoccupied(u, u->sprite->position);
		if (u_flying(u)) decrement_repulse_field(u);
		reset_movement_state(u);
		st.visible_units.remove(*u);
		bw_insert_list(st.hidden_units, *u);

		u->sprite->flags |= sprite_t::flag_hidden;
		set_sprite_visibility(u->sprite, 0);
		unit_t* turret = unit_turret(u);
		if (turret) {
			turret->sprite->flags |= sprite_t::flag_hidden;
			set_sprite_visibility(turret->sprite, 0);
		}

		if (deselect) {
			on_unit_deselect(u);
		}
	}

	void set_sprite_cloak_modifier(sprite_t* sprite, bool requires_detector, bool cloaked, bool burrowed, int data1, int data2) {
		for (image_t* image : ptr(sprite->images)) {
			if (image->image_type->always_visible) continue;
			hide_image(image);
		}
		if (burrowed) {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier >= 2 && image->modifier <= 7) set_image_modifier(image, 0);
			}
		} else if (requires_detector && !cloaked) {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier == 0) {
					image->modifier = 2;
					image->modifier_data1 = data1;
					image->modifier_data2 = data2;
				}
			}
		} else if (!requires_detector && cloaked) {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier == 0) {
					image->modifier = 4;
					image->modifier_data1 = data1;
					image->modifier_data2 = data2;
				}
			}
		} else {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier == 0 || image->modifier == 2 || image->modifier == 4) {
					image->modifier = 3;
				}
			}
		}
	}

	void cloak_unit(unit_t* u) {
		if (u_burrowed(u)) {
			if (!u_requires_detector(u)) {
				set_unit_cloaked(u);
				u_set_status_flag(u, unit_t::status_flag_requires_detector);
				u_set_status_flag(u, unit_t::status_flag_cloaked);
				set_sprite_cloak_modifier(u->sprite, true, true, true, 0, 0);
				set_secondary_order(u, get_order_type(Orders::Cloak));
				u->detected_flags = 0x80000000;
			}
			update_unit_detected_flags(u);
			u->secondary_order_timer = 30;
		} else {
			play_sound(273, u);
			auto cloak = [&](unit_t* u) {
				u_unset_status_flag(u, unit_t::status_flag_cloaked);
				u_set_status_flag(u, unit_t::status_flag_requires_detector);
				for (image_t* image : ptr(u->sprite->images)) {
					if (image->modifier == 0 || image->modifier == 4 || image->modifier == 7) {
						image->modifier = 2;
						image->modifier_data1 = 0;
						image->modifier_data2 = 3;
					}
					if (!image->image_type->always_visible) hide_image(image);
				}
				u->detected_flags = 0x80000000;
			};
			cloak(u);
			if (u->subunit) cloak(u->subunit);
			update_unit_detected_flags(u);
			u->secondary_order_timer = 30;
		}
	}

	void decloak_unit(unit_t* u) {
		if (u_burrowed(u)) {
			u_unset_status_flag(u, unit_t::status_flag_requires_detector);
			u_unset_status_flag(u, unit_t::status_flag_cloaked);
			for (image_t* image : ptr(u->sprite->images)) {
				if (!image->image_type->always_visible) show_image(image);
				if (image->modifier >= 2 && image->modifier <= 7) {
					set_image_modifier(image, 0);
				}
			}
		} else {
			play_sound(274, u);
			auto decloak = [&](unit_t* u) {
				u_unset_status_flag(u, unit_t::status_flag_requires_detector);
				u_set_status_flag(u, unit_t::status_flag_cloaked);
				for (image_t* image : ptr(u->sprite->images)) {
					show_image(image);
					if (image->modifier == 2 || image->modifier == 3) {
						set_image_modifier(image, 4);
						image->modifier_data1 = 8;
						image->modifier_data2 = 3;
					} else if (image->modifier == 5 || image->modifier == 6) {
						set_image_modifier(image, 7);
						image->modifier_data1 = 8;
						image->modifier_data2 = 3;
					}
				}
			};
			decloak(u);
			if (u->subunit) decloak(u->subunit);
		}
	}

	void complete_unit(unit_t* u) {
		if (ut_two_units_in_one_egg(u)) {
			increment_unit_counts(u, -1);
			u_set_status_flag(u, unit_t::status_flag_completed);
			increment_unit_counts(u, 1);
		} else {
			u_set_status_flag(u, unit_t::status_flag_completed);
		}
		add_completed_unit(u, 1, true);
		if (ut_initially_cloaked(u)) cloak_unit(u);
		if (unit_is_map_revealer(u)) {
			st.hidden_units.remove(*u);
			bw_insert_list(st.map_revealer_units, *u);
			refresh_unit_vision(u);
		} else {
			if (us_hidden(u)) {
				if (!unit_is(u, UnitTypes::Protoss_Interceptor) && !unit_is(u, UnitTypes::Protoss_Scarab)) {
					show_unit(u);
				}
			}
		}
		if (unit_is_trap(u)) {
			u_set_status_flag(u, unit_t::status_flag_cloaked);
			u_set_status_flag(u, unit_t::status_flag_requires_detector);
			u->detected_flags = 0x80000000;
			u->secondary_order_timer = 0;
		}
		if (st.players[u->owner].controller == player_t::controller_rescue_passive) {
			error("fixme rescue passive");
		} else {
			if (st.players[u->owner].controller == player_t::controller_neutral) set_unit_order(u, get_order_type(Orders::Neutral));
			else if (st.players[u->owner].controller == player_t::controller_computer_game) set_unit_order(u, u->unit_type->computer_ai_idle);
			else set_unit_order(u, u->unit_type->human_ai_idle);
		}
		if (ut_flag(u, (unit_type_t::flags_t)0x800)) {
			error("fixme unknown flag");
		}
		u->air_strength = get_unit_strength(u, false);
		u->ground_strength = get_unit_strength(u, true);
	}

	unit_t* create_initial_unit(const unit_type_t* unit_type, xy pos, int owner) {
		unit_t* u = create_unit(unit_type, pos, owner);
		if (!u) {
			display_last_error_for_player(owner);
			return nullptr;
		}
		if (unit_type_spreads_creep(unit_type, true) || ut_requires_creep(unit_type)) {
			spread_creep_completely(u, u->sprite->position);
		}
		finish_building_unit(u);
		if (!place_completed_unit(u)) {
			error("place_completed_unit failed");
		}

		complete_unit(u);

		return u;
	}

	unit_t* create_completed_unit(const unit_type_t* unit_type, xy pos, int owner) {
		unit_t* u = create_unit(unit_type, pos, owner);
		if (!u) return nullptr;
		finish_building_unit(u);
		complete_unit(u);
		return u;
	}

	void display_last_error_for_player(int player) {
		// todo
	}

	bool player_has_unit(int owner, UnitTypes unit_type_id) const {
		return st.unit_counts.at(owner).at(unit_type_id) != 0;
	}
	bool player_has_completed_unit(int owner, UnitTypes unit_type_id) const {
		return st.completed_unit_counts.at(owner).at(unit_type_id) != 0;
	}
	bool player_tech_available(int owner, TechTypes tech_id) const {
		return game_st.tech_available.at(owner).at(tech_id);
	}
	bool player_has_researched(int owner, TechTypes tech_id) const {
		return st.tech_researched.at(owner).at(tech_id);
	}
	bool player_is_researching(int owner, TechTypes tech_id) const {
		return st.tech_researching.at(owner).at(tech_id);
	}
	int player_max_upgrade_level(int owner, UpgradeTypes upgrade_id) const {
		return game_st.max_upgrade_levels.at(owner).at(upgrade_id);
	}
	bool player_has_upgrade(int owner, UpgradeTypes upgrade_id) const {
		return st.upgrade_levels.at(owner).at(upgrade_id) != 0;
	}
	bool player_is_upgrading(int owner, UpgradeTypes upgrade_id) const {
		return st.upgrade_upgrading.at(owner).at(upgrade_id);
	}
	int player_upgrade_level(int owner, UpgradeTypes upgrade_id) const {
		return st.upgrade_levels.at(owner).at(upgrade_id);
	}
	bool unit_is(unit_type_autocast ut, UnitTypes unit_type_id) const {
		return ut->id == unit_type_id;
	}
	unit_t* unit_addon(const unit_t* u) const {
		return ut_building(u) ? u->building.addon : nullptr;
	}
	bool unit_has_addon(const unit_t* u, UnitTypes unit_type_id) const {
		return ut_building(u) && u->building.addon && unit_is(u->building.addon, unit_type_id);
	}
	bool unit_is_building_addon(const unit_t* u) const {
		if (u->secondary_order_type->id == Orders::BuildAddon) {
			if (u_grounded_building(u) && u->current_build_unit && !u_completed(u->current_build_unit)) return true;
		}
		return false;
	}
	bool unit_is_building_protoss_thing(const unit_t* u) const {
		if (u->secondary_order_type->id == Orders::PlaceProtossBuilding) {
			if (u_grounded_building(u) && u->order_target.unit && !u_completed(u->order_target.unit)) return true;
		}
		return false;
	}
	bool unit_is_constructing(const unit_t* u) const {
		if (!u->build_queue.empty()) return true;
		if (unit_is_building_addon(u)) return true;
		if (unit_is_building_protoss_thing(u)) return true;
		return false;
	}
	bool unit_is_researching(const unit_t* u) const {
		return u->building.researching_type != nullptr;
	}
	bool unit_is_upgrading(const unit_t* u) const {
		return u->building.upgrading_type != nullptr;
	}
	bool unit_can_move(const unit_t* u) const {
		if (u_grounded_building(u)) return false;
		if (u->unit_type->right_click_action == 0) return false;
		if (u->unit_type->right_click_action == 3) return false;
		if (unit_is(u, UnitTypes::Zerg_Lurker) && u_burrowed(u)) return false;
		return true;
	}
	bool unit_can_hold_position(const unit_t* u) const {
		if (u->unit_type->right_click_action != 0) return true;
		if (u_grounded_building(u) && unit_is_factory(u)) return true;
		return false;
	}
	bool unit_is_immovable_attacker(const unit_t* u) const {
		if (unit_is(u, UnitTypes::Zerg_Lurker) && u_burrowed(u)) return true;
		if (u->unit_type->right_click_action == 3) return true;
		return false;
	}

	bool unit_can_build(const unit_t* u, const unit_type_t* unit_type) const {
		if (!u_completed(u)) return false;
		if (unit_is_disabled(u)) return false;
		if (u_hallucination(u)) return false;
		int owner = u->owner;
		if (!game_st.unit_type_allowed[owner][unit_type->id]) return false;
		if (u->build_queue.size() >= 5) {
			if (u->build_queue.back()->id <= UnitTypes::Spell_Disruption_Web) return false;
		}

		switch (unit_type->id) {
		case UnitTypes::Terran_Marine:
			if (!unit_is(u, UnitTypes::Terran_Barracks)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case UnitTypes::Terran_Ghost:
			if (!unit_is(u, UnitTypes::Terran_Barracks)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Covert_Ops)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Academy)) return false;
			return true;
		case UnitTypes::Terran_Vulture:
			if (!unit_is(u, UnitTypes::Terran_Factory)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			return true;
		case UnitTypes::Terran_Goliath:
			if (!unit_is(u, UnitTypes::Terran_Factory)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Armory)) return false;
			return true;
		case UnitTypes::Terran_Siege_Tank_Tank_Mode:
			if (!unit_is(u, UnitTypes::Terran_Factory)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!unit_has_addon(u, UnitTypes::Terran_Machine_Shop)) return false;
			return true;
		case UnitTypes::Terran_SCV:
			if (!unit_is(u, UnitTypes::Terran_Command_Center)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case UnitTypes::Terran_Wraith:
			if (!unit_is(u, UnitTypes::Terran_Starport)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case UnitTypes::Terran_Science_Vessel:
			if (!unit_is(u, UnitTypes::Terran_Starport)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (!unit_has_addon(u, UnitTypes::Terran_Control_Tower)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Science_Facility)) return false;
			return true;
		case UnitTypes::Terran_Dropship:
			if (!unit_is(u, UnitTypes::Terran_Starport)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (!unit_has_addon(u, UnitTypes::Terran_Control_Tower)) return false;
			return true;
		case UnitTypes::Terran_Battlecruiser:
			if (!unit_is(u, UnitTypes::Terran_Starport)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (!unit_has_addon(u, UnitTypes::Terran_Control_Tower)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Physics_Lab)) return false;
			return true;
		case UnitTypes::Terran_Firebat:
			if (!unit_is(u, UnitTypes::Terran_Barracks)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Academy)) return false;
			return true;
		case UnitTypes::Terran_Nuclear_Missile:
			if (!unit_is(u, UnitTypes::Terran_Nuclear_Silo)) return false;
			if (ut_building(u) && u->building.silo.nuke) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UnitTypes::Terran_Medic:
			if (!unit_is(u, UnitTypes::Terran_Barracks)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Academy)) return false;
			return true;
		case UnitTypes::Terran_Valkyrie:
			if (!unit_is(u, UnitTypes::Terran_Starport)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (!unit_has_addon(u, UnitTypes::Terran_Control_Tower)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Armory)) return false;
			return true;
		case UnitTypes::Zerg_Zergling:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Spawning_Pool)) return false;
			return true;
		case UnitTypes::Zerg_Hydralisk:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hydralisk_Den)) return false;
			return true;
		case UnitTypes::Zerg_Ultralisk:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Ultralisk_Cavern)) return false;
			return true;
		case UnitTypes::Zerg_Drone:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			return true;
		case UnitTypes::Zerg_Overlord:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			return true;
		case UnitTypes::Zerg_Mutalisk:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Spire) && !player_has_unit(owner, UnitTypes::Zerg_Greater_Spire)) return false;
			return true;
		case UnitTypes::Zerg_Guardian:
			if (!unit_is(u, UnitTypes::Zerg_Mutalisk)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Greater_Spire)) return false;
			return true;
		case UnitTypes::Zerg_Queen:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Queens_Nest)) return false;
			return true;
		case UnitTypes::Zerg_Defiler:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Defiler_Mound)) return false;
			return true;
		case UnitTypes::Zerg_Scourge:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Spire) && !player_has_unit(owner, UnitTypes::Zerg_Greater_Spire)) return false;
			return true;
		case UnitTypes::Zerg_Infested_Terran:
			if (!unit_is(u, UnitTypes::Zerg_Infested_Command_Center)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case UnitTypes::Zerg_Devourer:
			if (!unit_is(u, UnitTypes::Zerg_Mutalisk)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Greater_Spire)) return false;
			return true;
		case UnitTypes::Zerg_Lurker:
			if (!unit_is(u, UnitTypes::Zerg_Hydralisk)) return false;
			if (!player_has_researched(owner, TechTypes::Lurker_Aspect)) return false;
			return true;
		case UnitTypes::Protoss_Probe:
			if (!unit_is(u, UnitTypes::Protoss_Nexus)) return false;
			if (unit_is_building_addon(u) || unit_is_building_protoss_thing(u)) return false;
			return true;
		case UnitTypes::Protoss_Zealot:
			if (!unit_is(u, UnitTypes::Protoss_Gateway)) return false;
			return true;
		case UnitTypes::Protoss_Dragoon:
			if (!unit_is(u, UnitTypes::Protoss_Gateway)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Cybernetics_Core)) return false;
			return true;
		case UnitTypes::Protoss_High_Templar:
			if (!unit_is(u, UnitTypes::Protoss_Gateway)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Templar_Archives)) return false;
			return true;
		case UnitTypes::Protoss_Shuttle:
			if (!unit_is(u, UnitTypes::Protoss_Robotics_Facility)) return false;
			return true;
		case UnitTypes::Protoss_Scout:
			if (!unit_is(u, UnitTypes::Protoss_Stargate)) return false;
			return true;
		case UnitTypes::Protoss_Arbiter:
			if (!unit_is(u, UnitTypes::Protoss_Stargate)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Arbiter_Tribunal)) return false;
			return true;
		case UnitTypes::Protoss_Carrier:
			if (!unit_is(u, UnitTypes::Protoss_Stargate)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Fleet_Beacon)) return false;
			return true;
		case UnitTypes::Protoss_Interceptor:
			if (!unit_is_carrier(u)) return false;
			if (unit_interceptor_count(u) + unit_queued_fighter_units(u) >= unit_max_interceptor_count(u)) return false;
			return true;
		case UnitTypes::Protoss_Reaver:
			if (!unit_is(u, UnitTypes::Protoss_Robotics_Facility)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Robotics_Support_Bay)) return false;
			return true;
		case UnitTypes::Protoss_Scarab:
			if (!unit_is_reaver(u)) return false;
			if (unit_scarab_count(u) + unit_queued_fighter_units(u) >= unit_max_scarab_count(u)) return false;
			return true;
		case UnitTypes::Protoss_Observer:
			if (!unit_is(u, UnitTypes::Protoss_Robotics_Facility)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Observatory)) return false;
			return true;
		case UnitTypes::Protoss_Corsair:
			if (!unit_is(u, UnitTypes::Protoss_Stargate)) return false;
			return true;
		case UnitTypes::Protoss_Dark_Templar:
			if (!unit_is(u, UnitTypes::Protoss_Gateway)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Templar_Archives)) return false;
			return true;
		case UnitTypes::Terran_Command_Center:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			return true;
		case UnitTypes::Terran_Comsat_Station:
			if (!unit_is(u, UnitTypes::Terran_Command_Center)) return false;
			if (unit_addon(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Academy)) return false;
			return true;
		case UnitTypes::Terran_Nuclear_Silo:
			if (!unit_is(u, UnitTypes::Terran_Command_Center)) return false;
			if (unit_addon(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Covert_Ops)) return false;
			return true;
		case UnitTypes::Terran_Supply_Depot:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			return true;
		case UnitTypes::Terran_Refinery:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			return true;
		case UnitTypes::Terran_Barracks:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Command_Center)) return false;
			return true;
		case UnitTypes::Terran_Academy:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Barracks)) return false;
			return true;
		case UnitTypes::Terran_Factory:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Barracks)) return false;
			return true;
		case UnitTypes::Terran_Starport:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Factory)) return false;
			return true;
		case UnitTypes::Terran_Control_Tower:
			if (!unit_is(u, UnitTypes::Terran_Starport)) return false;
			if (unit_addon(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UnitTypes::Terran_Science_Facility:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Starport)) return false;
			return true;
		case UnitTypes::Terran_Covert_Ops:
			if (!unit_is(u, UnitTypes::Terran_Science_Facility)) return false;
			if (unit_addon(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UnitTypes::Terran_Physics_Lab:
			if (!unit_is(u, UnitTypes::Terran_Science_Facility)) return false;
			if (unit_addon(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UnitTypes::Terran_Machine_Shop:
			if (!unit_is(u, UnitTypes::Terran_Factory)) return false;
			if (unit_addon(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UnitTypes::Terran_Engineering_Bay:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Command_Center)) return false;
			return true;
		case UnitTypes::Terran_Armory:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Factory)) return false;
			return true;
		case UnitTypes::Terran_Missile_Turret:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Engineering_Bay)) return false;
			return true;
		case UnitTypes::Terran_Bunker:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Barracks)) return false;
			return true;
		case UnitTypes::Zerg_Hatchery:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			return true;
		case UnitTypes::Zerg_Lair:
			if (!unit_is(u, UnitTypes::Zerg_Hatchery)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Spawning_Pool)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UnitTypes::Zerg_Hive:
			if (!unit_is(u, UnitTypes::Zerg_Lair)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Queens_Nest)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UnitTypes::Zerg_Spawning_Pool:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hatchery) && !player_has_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Hydralisk_Den:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Spawning_Pool)) return false;
			return true;
		case UnitTypes::Zerg_Defiler_Mound:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Greater_Spire:
			if (!unit_is(u, UnitTypes::Zerg_Spire)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Queens_Nest:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Evolution_Chamber:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hatchery) && !player_has_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Ultralisk_Cavern:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Spire:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Nydus_Canal:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UnitTypes::Zerg_Creep_Colony:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_burrowed(u)) return false;
			return true;
		case UnitTypes::Zerg_Spore_Colony:
			if (!unit_is(u, UnitTypes::Zerg_Creep_Colony)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Evolution_Chamber)) return false;
			return true;
		case UnitTypes::Zerg_Sunken_Colony:
			if (!unit_is(u, UnitTypes::Zerg_Creep_Colony)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Spawning_Pool)) return false;
			return true;
		case UnitTypes::Zerg_Extractor:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			return true;
		case UnitTypes::Protoss_Nexus:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			return true;
		case UnitTypes::Protoss_Robotics_Facility:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Cybernetics_Core)) return false;
			return true;
		case UnitTypes::Protoss_Pylon:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			return true;
		case UnitTypes::Protoss_Assimilator:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			return true;
		case UnitTypes::Protoss_Observatory:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Robotics_Facility)) return false;
			return true;
		case UnitTypes::Protoss_Gateway:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Nexus)) return false;
			return true;
		case UnitTypes::Protoss_Photon_Cannon:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Forge)) return false;
			return true;
		case UnitTypes::Protoss_Citadel_of_Adun:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Cybernetics_Core)) return false;
			return true;
		case UnitTypes::Protoss_Cybernetics_Core:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Gateway)) return false;
			return true;
		case UnitTypes::Protoss_Templar_Archives:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Citadel_of_Adun)) return false;
			return true;
		case UnitTypes::Protoss_Forge:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Nexus)) return false;
			return true;
		case UnitTypes::Protoss_Stargate:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Cybernetics_Core)) return false;
			return true;
		case UnitTypes::Protoss_Fleet_Beacon:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Stargate)) return false;
			return true;
		case UnitTypes::Protoss_Arbiter_Tribunal:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Stargate)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Templar_Archives)) return false;
			return true;
		case UnitTypes::Protoss_Robotics_Support_Bay:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Robotics_Facility)) return false;
			return true;
		case UnitTypes::Protoss_Shield_Battery:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Protoss_Gateway)) return false;
			return true;
		default:
			return false;
		}
	}

	bool unit_can_receive_order(const unit_t* u, const order_type_t* order, int owner) const {
		if (u->owner != owner) return false;
		if (order->id != Orders::RallyPointUnit && order->id != Orders::RallyPointTile) {
			if (!u_completed(u)) return false;
		}
		if (unit_is_disabled(u)) return false;
		if (!unit_is(u, UnitTypes::Zerg_Lurker) && u_burrowed(u)) {
			if (order->id != Orders::Unburrowing) return false;
		}
		if (unit_is(u, UnitTypes::Terran_SCV) && u->order_type->id == Orders::ConstructingBuilding && order->id != Orders::Stop) return false;
		if (unit_is_ghost(u) && u->order_type->id == Orders::NukeTrack) return false;
		if (unit_is(u, UnitTypes::Protoss_Archon) || unit_is(u, UnitTypes::Protoss_Dark_Archon)) {
			if (u->order_type->id == Orders::CompletingArchonSummon) return false;
		}
		return unit_order_allowed(u, order);
	}

	bool unit_order_allowed(const unit_t* u, const order_type_t* order) const {
		switch (order->id) {
		case Orders::Die:
			return true;
		case Orders::Stop:
			return true;
		case Orders::Guard:
			if (u_grounded_building(u)) return false;
			return true;
		case Orders::PlayerGuard:
			if (u_grounded_building(u)) return false;
			return true;
		case Orders::TurretGuard:
			if (!ut_turret(u)) return false;
			return true;
		case Orders::BunkerGuard:
			if (!unit_is(u, UnitTypes::Terran_Bunker)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Move:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::InterceptorAttack:
			if (!unit_is(u, UnitTypes::Protoss_Interceptor)) return false;
			return true;
		case Orders::ScarabAttack:
			if (!unit_is(u, UnitTypes::Protoss_Scarab)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::SpiderMine:
			if (!unit_is(u, UnitTypes::Terran_Vulture_Spider_Mine)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Carrier:
			if (!unit_is_carrier(u)) return false;
			return true;
		case Orders::Reaver:
			if (!unit_is_reaver(u)) return false;
			return true;
		case Orders::ReaverCarrierMove:
			if (!unit_is_carrier(u) && !unit_is_reaver(u)) return false;
			return true;
		case Orders::CarrierStop:
			if (!unit_is_carrier(u)) return false;
			return true;
		case Orders::ReaverStop:
			if (!unit_is_reaver(u)) return false;
			return true;
		case Orders::CarrierHoldPosition:
			if (!unit_is_carrier(u)) return false;
			return true;
		case Orders::ReaverHoldPosition:
			if (!unit_is_reaver(u)) return false;
			return true;
		case Orders::CarrierAttack:
			if (!unit_is_carrier(u)) return false;
			return true;
		case Orders::ReaverAttack:
			if (!unit_is_reaver(u)) return false;
			return true;
		case Orders::CarrierMoveToAttack:
			if (!unit_is_carrier(u)) return false;
			return true;
		case Orders::ReaverMoveToAttack:
			if (!unit_is_reaver(u)) return false;
			return true;
		case Orders::CarrierIgnore2:
			if (!unit_is_carrier(u) && !unit_is_reaver(u)) return false;
			return true;
		case Orders::CarrierFight:
			if (!unit_is_carrier(u)) return false;
			return true;
		case Orders::ReaverFight:
			if (!unit_is_reaver(u)) return false;
			return true;
		case Orders::AttackDefault:
			if (u_grounded_building(u)) return false;
			return true;
		case Orders::MoveToAttack:
			if (u_grounded_building(u)) return false;
			return true;
		case Orders::AttackUnit:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::AttackFixedRange:
			if (u_grounded_building(u)) return false;
			return true;
		case Orders::AttackTile:
			if (u_grounded_building(u)) return false;
			return true;
		case Orders::Hover:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::AttackMove:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::AtkMoveEP:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::StayInRange:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::TowerGuard:
			if (!unit_is(u, UnitTypes::Terran_Missile_Turret) && !unit_is(u, UnitTypes::Zerg_Spore_Colony) && !unit_is(u, UnitTypes::Zerg_Sunken_Colony) && !unit_is(u, UnitTypes::Protoss_Photon_Cannon)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::TowerAttack:
			if (!unit_is(u, UnitTypes::Terran_Missile_Turret) && !unit_is(u, UnitTypes::Zerg_Spore_Colony) && !unit_is(u, UnitTypes::Zerg_Sunken_Colony) && !unit_is(u, UnitTypes::Protoss_Photon_Cannon) && !unit_is_trap(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::TurretAttack:
			if (!ut_turret(u)) return false;
			return true;
		case Orders::Nothing:
			return true;
		case Orders::DroneStartBuild:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::DroneBuild:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::DroneLand:
			if (!unit_is(u, UnitTypes::Zerg_Drone)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::PlaceBuilding:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::PlaceProtossBuilding:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::CreateProtossBuilding:
			if (!unit_is(u, UnitTypes::Protoss_Probe)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::PlaceAddon:
			if (u_hallucination(u)) return false;
			return true;
		case Orders::RallyPointUnit:
			if (!u_grounded_building(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::RallyPointTile:
			if (!u_grounded_building(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::InfestedCommandCenter:
			if (!unit_is(u, UnitTypes::Terran_Command_Center)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::BuildNydusExit:
			if (!unit_is_nydus(u)) return false;
			if (u->building.nydus.exit) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::QueenHoldPosition:
			if (!unit_is_queen(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::CastInfestation:
			if (!unit_is_queen(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MoveToInfest:
			if (!unit_is_queen(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::InfestingCommandCenter:
			if (!unit_is_queen(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::ConstructingBuilding:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Repair:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MoveToRepair:
			if (!unit_is(u, UnitTypes::Terran_SCV)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::ZergBirth:
			if (!unit_is(u, UnitTypes::Zerg_Egg) && !unit_is(u, UnitTypes::Zerg_Cocoon)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Follow:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::InterceptorReturn:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::BuildingLand:
			if (!ut_flying_building(u)) return false;
			if (u_grounded_building(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::BuildingLiftoff:
			if (!ut_flying_building(u)) return false;
			if (!u_grounded_building(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::LiftingOff:
			if (!ut_flying_building(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::CTFCOP2:
			if (!unit_is(u, UnitTypes::Special_Zerg_Flag_Beacon) && !unit_is(u, UnitTypes::Special_Terran_Flag_Beacon) && !unit_is(u, UnitTypes::Special_Protoss_Flag_Beacon)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Larva:
			if (!unit_is(u, UnitTypes::Zerg_Larva)) return false;
			return true;
		case Orders::SpawningLarva:
			if (unit_is_hatchery(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Harvest1:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Harvest2:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MoveToGas:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::WaitForGas:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::ReturnGas:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MoveToMinerals:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::WaitForMinerals:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::ReturnMinerals:
			if (!ut_worker(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::EnterTransport:
			return true;
		case Orders::Unload:
			if (!unit_provides_space(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MoveUnload:
			if (!unit_provides_space(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::PickupTransport:
			if (!unit_provides_space(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::PickupBunker:
			if (!unit_is(u, UnitTypes::Terran_Bunker)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::PickupIdle:
			if (!unit_provides_space(u) && !unit_is(u, UnitTypes::Zerg_Overlord)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::PowerupIdle:
			if (!ut_powerup(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::WatchTarget:
			return true;
		case Orders::InitCreepGrowth:
			if (!unit_is(u, UnitTypes::Zerg_Creep_Colony) && !unit_is(u, UnitTypes::Zerg_Hatchery) && !unit_is(u, UnitTypes::Zerg_Lair) && !unit_is(u, UnitTypes::Zerg_Hive)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::StoppingCreepGrowth:
			if (!unit_is(u, UnitTypes::Zerg_Spore_Colony) && !unit_is(u, UnitTypes::Zerg_Sunken_Colony)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::SpreadCreep:
			if (u_hallucination(u)) return false;
			return true;
		case Orders::GuardianAspect:
			if (!unit_is(u, UnitTypes::Zerg_Mutalisk)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::HoldPosition:
			if (!unit_can_hold_position(u)) return false;
			return true;
		case Orders::Patrol:
			if (!unit_can_move(u)) return false;
			return true;
		case Orders::RechargeShieldsBattery:
			if (!unit_is(u, UnitTypes::Protoss_Shield_Battery)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Scanner:
			if (!unit_is(u, UnitTypes::Spell_Scanner_Sweep)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::NukeWait:
			if (!unit_is(u, UnitTypes::Terran_Nuclear_Missile)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::NukeLaunch:
			if (!unit_is(u, UnitTypes::Terran_Nuclear_Missile)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::NukePaint:
			if (!unit_is(u, UnitTypes::Terran_Ghost)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::NukeUnit:
			if (!unit_is(u, UnitTypes::Terran_Nuclear_Missile)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::CastNuclearStrike:
			if (!unit_is(u, UnitTypes::Terran_Nuclear_Missile)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::NukeTrack:
			if (!unit_is(u, UnitTypes::Terran_Nuclear_Missile)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::InitializeArbiter:
			if (!unit_is_arbiter(u)) return false;
			return true;
		case Orders::CloakNearbyUnits:
			if (!unit_is_arbiter(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::RightClickAction:
			return true;
		case Orders::SuicideUnit:
			if (!unit_is(u, UnitTypes::Zerg_Infested_Terran)) return false;
			return true;
		case Orders::SuicideLocation:
			if (!unit_is(u, UnitTypes::Zerg_Infested_Terran)) return false;
			return true;
		case Orders::SuicideHoldPosition:
			if (!unit_is(u, UnitTypes::Zerg_Infested_Terran) && !unit_is(u, UnitTypes::Zerg_Scourge)) return false;
			return true;
		case Orders::SelfDestructing:
			if (!unit_is(u, UnitTypes::Protoss_Scarab)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::Critter:
			if (!unit_is_critter(u)) return false;
			return true;
		case Orders::HiddenGun:
			if (!unit_is_trap(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::OpenDoor:
			if (!unit_is(u, UnitTypes::Special_Upper_Level_Door) && !unit_is(u, UnitTypes::Special_Right_Upper_Level_Door) && !unit_is(u, UnitTypes::Special_Pit_Door) && !unit_is(u, UnitTypes::Special_Right_Pit_Door)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::CloseDoor:
			if (!unit_is(u, UnitTypes::Special_Upper_Level_Door) && !unit_is(u, UnitTypes::Special_Right_Upper_Level_Door) && !unit_is(u, UnitTypes::Special_Pit_Door) && !unit_is(u, UnitTypes::Special_Right_Pit_Door)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::HideTrap:
			if (!unit_is_trap(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::RevealTrap:
			if (!unit_is_trap(u)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MedicIdle:
			if (!unit_is(u, UnitTypes::Terran_Medic)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MedicHeal:
			if (!unit_is(u, UnitTypes::Terran_Medic)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::HealMove:
			if (!unit_is(u, UnitTypes::Terran_Medic)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::MedicHoldPosition:
			if (!unit_is(u, UnitTypes::Terran_Medic)) return false;
			if (u_hallucination(u)) return false;
			return true;
		case Orders::CastMindControl:
			if (!unit_is(u, UnitTypes::Protoss_Dark_Archon)) return false;
			if (u_hallucination(u)) return false;
			return true;
		default:
			return false;
		}
	}

	bool unit_build_order_valid(const unit_t* u, const order_type_t* order, const unit_type_t* unit_type, int owner) const {
		switch (order->id) {
		case Orders::DroneStartBuild:
		case Orders::PlaceBuilding:
		case Orders::PlaceProtossBuilding:
		case Orders::CreateProtossBuilding:
		case Orders::PlaceAddon:
			if (!unit_can_build(u, unit_type)) return false;
			if (!unit_can_receive_order(u, order, owner)) return false;
			return true;
		case Orders::CTFCOP2:
			if (st.current_frame > 600) return false;
			if (!unit_is(u, unit_type->id)) return false;
			if (!unit_can_receive_order(u, order, owner)) return false;
			return true;
		case Orders::BuildNydusExit:
		case Orders::BuildingLand:
			if (!unit_is(u, unit_type->id)) return false;
			if (!unit_can_receive_order(u, order, owner)) return false;
			return true;
		default:
			return false;
		}
	}

	bool unit_can_research(const unit_t* u, const tech_type_t* tech, int owner) const {
		if (u->owner != owner) return false;
		return unit_can_research(u, tech);
	}

	bool unit_can_research(const unit_t* u, const tech_type_t* tech) const {
		if (!u_completed(u)) return false;
		if (u_hallucination(u)) return false;
		if (unit_is_disabled(u)) return false;
		int owner = u->owner;
		if (!player_tech_available(owner, tech->id)) return false;
		if (player_has_researched(owner, tech->id)) return false;
		if (player_is_researching(owner, tech->id)) return false;
		switch (tech->id) {
		case TechTypes::Stim_Packs:
			if (!unit_is(u, UnitTypes::Terran_Academy)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Lockdown:
			if (!unit_is(u, UnitTypes::Terran_Covert_Ops)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Spider_Mines:
			if (!unit_is(u, UnitTypes::Terran_Machine_Shop)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Tank_Siege_Mode:
			if (!unit_is(u, UnitTypes::Terran_Machine_Shop)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::EMP_Shockwave:
			if (!unit_is(u, UnitTypes::Terran_Science_Facility)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Irradiate:
			if (!unit_is(u, UnitTypes::Terran_Science_Facility)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Yamato_Gun:
			if (!unit_is(u, UnitTypes::Terran_Physics_Lab)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Cloaking_Field:
			if (!unit_is(u, UnitTypes::Terran_Control_Tower)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Personnel_Cloaking:
			if (!unit_is(u, UnitTypes::Terran_Covert_Ops)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Restoration:
			if (!unit_is(u, UnitTypes::Terran_Academy)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Optical_Flare:
			if (!unit_is(u, UnitTypes::Terran_Academy)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!u_grounded_building(u)) return false;
			return true;
		case TechTypes::Burrowing:
			if (!unit_is(u, UnitTypes::Zerg_Hatchery) && !unit_is(u, UnitTypes::Zerg_Lair) && !unit_is(u, UnitTypes::Zerg_Hive)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Spawn_Broodlings:
			if (!unit_is(u, UnitTypes::Zerg_Queens_Nest)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Parasite:
			if (!unit_is(u, UnitTypes::Zerg_Queens_Nest)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Plague:
			if (!unit_is(u, UnitTypes::Zerg_Defiler_Mound)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Consume:
			if (!unit_is(u, UnitTypes::Zerg_Defiler_Mound)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Ensnare:
			if (!unit_is(u, UnitTypes::Zerg_Queens_Nest)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Lurker_Aspect:
			if (!unit_is(u, UnitTypes::Zerg_Hydralisk_Den)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Psionic_Storm:
			if (!unit_is(u, UnitTypes::Protoss_Templar_Archives)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Hallucination:
			if (!unit_is(u, UnitTypes::Protoss_Templar_Archives)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Recall:
			if (!unit_is(u, UnitTypes::Protoss_Arbiter_Tribunal)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Stasis_Field:
			if (!unit_is(u, UnitTypes::Protoss_Arbiter_Tribunal)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Disruption_Web:
			if (!unit_is(u, UnitTypes::Protoss_Fleet_Beacon)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Mind_Control:
			if (!unit_is(u, UnitTypes::Protoss_Templar_Archives)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Feedback:
			if (!unit_is(u, UnitTypes::Protoss_Templar_Archives)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case TechTypes::Maelstrom:
			if (!unit_is(u, UnitTypes::Protoss_Templar_Archives)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		default:
			return false;
		}
	}

	bool unit_can_upgrade(const unit_t* u, const upgrade_type_t* upgrade, int owner) const {
		if (u->owner != owner) return false;
		return unit_can_upgrade(u, upgrade);
	}

	bool unit_can_upgrade(const unit_t* u, const upgrade_type_t* upgrade) const {
		if (!u_completed(u)) return false;
		if (u_hallucination(u)) return false;
		if (unit_is_disabled(u)) return false;
		int owner = u->owner;
		if (player_upgrade_level(owner, upgrade->id) >= player_max_upgrade_level(u->owner, upgrade->id)) return false;
		if (player_is_upgrading(owner, upgrade->id)) return false;
		switch (upgrade->id) {
		case UpgradeTypes::Terran_Infantry_Armor:
			if (!unit_is(u, UnitTypes::Terran_Engineering_Bay)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Terran_Infantry_Armor) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Terran_Science_Facility)) return false;
			}
			return true;
		case UpgradeTypes::Terran_Vehicle_Plating:
			if (!unit_is(u, UnitTypes::Terran_Armory)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Terran_Vehicle_Plating) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Terran_Science_Facility)) return false;
			}
			return true;
		case UpgradeTypes::Terran_Ship_Plating:
			if (!unit_is(u, UnitTypes::Terran_Armory)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Terran_Ship_Plating) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Terran_Science_Facility)) return false;
			}
			return true;
		case UpgradeTypes::Zerg_Carapace:
			if (!unit_is(u, UnitTypes::Zerg_Evolution_Chamber)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			switch (player_upgrade_level(owner, UpgradeTypes::Zerg_Carapace)) {
			case 1:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			case 2:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			}
			return true;
		case UpgradeTypes::Zerg_Flyer_Carapace:
			if (!unit_is(u, UnitTypes::Zerg_Spire) && !unit_is(u, UnitTypes::Zerg_Greater_Spire)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			switch (player_upgrade_level(owner, UpgradeTypes::Zerg_Flyer_Carapace)) {
			case 1:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			case 2:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			}
			return true;
		case UpgradeTypes::Protoss_Ground_Armor:
			if (!unit_is(u, UnitTypes::Protoss_Forge)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Protoss_Ground_Armor) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Protoss_Templar_Archives)) return false;
			}
			return true;
		case UpgradeTypes::Protoss_Air_Armor:
			if (!unit_is(u, UnitTypes::Protoss_Cybernetics_Core)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Protoss_Air_Armor) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Protoss_Fleet_Beacon)) return false;
			}
			return true;
		case UpgradeTypes::Terran_Infantry_Weapons:
			if (!unit_is(u, UnitTypes::Terran_Engineering_Bay)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Terran_Infantry_Weapons) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Terran_Science_Facility)) return false;
			}
			return true;
		case UpgradeTypes::Terran_Vehicle_Weapons:
			if (!unit_is(u, UnitTypes::Terran_Armory)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Terran_Vehicle_Weapons) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Terran_Science_Facility)) return false;
			}
			return true;
		case UpgradeTypes::Terran_Ship_Weapons:
			if (!unit_is(u, UnitTypes::Terran_Armory)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Terran_Ship_Weapons) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Terran_Science_Facility)) return false;
			}
			return true;
		case UpgradeTypes::Zerg_Melee_Attacks:
			if (!unit_is(u, UnitTypes::Zerg_Evolution_Chamber)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			switch (player_upgrade_level(owner, UpgradeTypes::Zerg_Melee_Attacks)) {
			case 1:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			case 2:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			}
			return true;
		case UpgradeTypes::Zerg_Missile_Attacks:
			if (!unit_is(u, UnitTypes::Zerg_Evolution_Chamber)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			switch (player_upgrade_level(owner, UpgradeTypes::Zerg_Missile_Attacks)) {
			case 1:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			case 2:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			}
			return true;
		case UpgradeTypes::Zerg_Flyer_Attacks:
			if (!unit_is(u, UnitTypes::Zerg_Spire) && !unit_is(u, UnitTypes::Zerg_Greater_Spire)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			switch (player_upgrade_level(owner, UpgradeTypes::Zerg_Flyer_Attacks)) {
			case 1:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Lair) && !player_has_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			case 2:
				if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
				break;
			}
			return true;
		case UpgradeTypes::Protoss_Ground_Weapons:
			if (!unit_is(u, UnitTypes::Protoss_Forge)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Protoss_Ground_Weapons) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Protoss_Templar_Archives)) return false;
			}
			return true;
		case UpgradeTypes::Protoss_Air_Weapons:
			if (!unit_is(u, UnitTypes::Protoss_Cybernetics_Core)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Protoss_Air_Weapons) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Protoss_Fleet_Beacon)) return false;
			}
			return true;
		case UpgradeTypes::Protoss_Plasma_Shields:
			if (!unit_is(u, UnitTypes::Protoss_Forge)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (player_upgrade_level(owner, UpgradeTypes::Protoss_Plasma_Shields) != 0) {
				if (!player_has_completed_unit(owner, UnitTypes::Protoss_Cybernetics_Core)) return false;
			}
			return true;
		case UpgradeTypes::U_238_Shells:
			if (!unit_is(u, UnitTypes::Terran_Academy)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Ion_Thrusters:
			if (!unit_is(u, UnitTypes::Terran_Machine_Shop)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Titan_Reactor:
			if (!unit_is(u, UnitTypes::Terran_Science_Facility)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Ocular_Implants:
			if (!unit_is(u, UnitTypes::Terran_Covert_Ops)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Moebius_Reactor:
			if (!unit_is(u, UnitTypes::Terran_Covert_Ops)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Apollo_Reactor:
			if (!unit_is(u, UnitTypes::Terran_Control_Tower)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Colossus_Reactor:
			if (!unit_is(u, UnitTypes::Terran_Physics_Lab)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Ventral_Sacs:
			if (!unit_is(u, UnitTypes::Zerg_Lair) && !unit_is(u, UnitTypes::Zerg_Hive)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Antennae:
			if (!unit_is(u, UnitTypes::Zerg_Lair) && !unit_is(u, UnitTypes::Zerg_Hive)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Pneumatized_Carapace:
			if (!unit_is(u, UnitTypes::Zerg_Lair) && !unit_is(u, UnitTypes::Zerg_Hive)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Metabolic_Boost:
			if (!unit_is(u, UnitTypes::Zerg_Spawning_Pool)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Adrenal_Glands:
			if (!unit_is(u, UnitTypes::Zerg_Spawning_Pool)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Zerg_Hive)) return false;
			return true;
		case UpgradeTypes::Muscular_Augments:
			if (!unit_is(u, UnitTypes::Zerg_Hydralisk_Den)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Grooved_Spines:
			if (!unit_is(u, UnitTypes::Zerg_Hydralisk_Den)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Gamete_Meiosis:
			if (!unit_is(u, UnitTypes::Zerg_Queens_Nest)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Metasynaptic_Node:
			if (!unit_is(u, UnitTypes::Zerg_Defiler_Mound)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Singularity_Charge:
			if (!unit_is(u, UnitTypes::Protoss_Cybernetics_Core)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Leg_Enhancements:
			if (!unit_is(u, UnitTypes::Protoss_Citadel_of_Adun)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Scarab_Damage:
			if (!unit_is(u, UnitTypes::Protoss_Robotics_Support_Bay)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Reaver_Capacity:
			if (!unit_is(u, UnitTypes::Protoss_Robotics_Support_Bay)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Gravitic_Drive:
			if (!unit_is(u, UnitTypes::Protoss_Robotics_Support_Bay)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Sensor_Array:
			if (!unit_is(u, UnitTypes::Protoss_Observatory)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Gravitic_Boosters:
			if (!unit_is(u, UnitTypes::Protoss_Observatory)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Khaydarin_Amulet:
			if (!unit_is(u, UnitTypes::Protoss_Templar_Archives)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Apial_Sensors:
			if (!unit_is(u, UnitTypes::Protoss_Fleet_Beacon)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Gravitic_Thrusters:
			if (!unit_is(u, UnitTypes::Protoss_Fleet_Beacon)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Carrier_Capacity:
			if (!unit_is(u, UnitTypes::Protoss_Fleet_Beacon)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Khaydarin_Core:
			if (!unit_is(u, UnitTypes::Protoss_Arbiter_Tribunal)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Argus_Jewel:
			if (!unit_is(u, UnitTypes::Protoss_Fleet_Beacon)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Caduceus_Reactor:
			if (!unit_is(u, UnitTypes::Terran_Academy)) return false;
			if (!u_grounded_building(u)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Argus_Talisman:
			if (!unit_is(u, UnitTypes::Protoss_Templar_Archives)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Chitinous_Plating:
			if (!unit_is(u, UnitTypes::Zerg_Ultralisk_Cavern)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Anabolic_Synthesis:
			if (!unit_is(u, UnitTypes::Zerg_Ultralisk_Cavern)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			return true;
		case UpgradeTypes::Charon_Boosters:
			if (!unit_is(u, UnitTypes::Terran_Machine_Shop)) return false;
			if (unit_is_constructing(u) || unit_is_researching(u) || unit_is_upgrading(u)) return false;
			if (!player_has_completed_unit(owner, UnitTypes::Terran_Armory)) return false;
			return true;
		default:
			return false;
		}
	}

	bool unit_can_use_tech(const unit_t* u, const tech_type_t* tech, int owner) const {
		if (u->owner != owner) return false;
		return unit_can_use_tech(u, tech);
	}

	bool unit_can_use_tech(const unit_t* u, const tech_type_t* tech) const {
		if (!u_completed(u)) return false;
		if (u_hallucination(u)) return false;
		if (unit_is_disabled(u)) return false;
		int owner = u->owner;
		if (!player_tech_available(owner, tech->id)) return false;
		switch (tech->id) {
		case TechTypes::Stim_Packs:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Stim_Packs)) return false;
			if (!unit_is_marine(u) && !unit_is_firebat(u)) return false;
			return true;
		case TechTypes::Lockdown:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Lockdown)) return false;
			if (!unit_is_ghost(u)) return false;
			return true;
		case TechTypes::Spider_Mines:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Spider_Mines)) return false;
			if (unit_spider_mine_count(u) == 0) return false;
			if (!unit_is_vulture(u)) return false;
			return true;
		case TechTypes::Scanner_Sweep:
			if (!unit_is(u, UnitTypes::Terran_Comsat_Station)) return false;
			return true;
		case TechTypes::Tank_Siege_Mode:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Tank_Siege_Mode)) return false;
			if (!unit_is_tank(u)) return false;
			return true;
		case TechTypes::EMP_Shockwave:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::EMP_Shockwave)) return false;
			if (!unit_is(u, UnitTypes::Terran_Science_Vessel) && !unit_is(u, UnitTypes::Hero_Magellan)) return false;
			return true;
		case TechTypes::Defensive_Matrix:
			if (!unit_is(u, UnitTypes::Terran_Science_Vessel) && !unit_is(u, UnitTypes::Hero_Magellan)) return false;
			return true;
		case TechTypes::Irradiate:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Irradiate)) return false;
			if (!unit_is(u, UnitTypes::Terran_Science_Vessel) && !unit_is(u, UnitTypes::Hero_Magellan)) return false;
			return true;
		case TechTypes::Yamato_Gun:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Yamato_Gun)) return false;
			if (!unit_is(u, UnitTypes::Terran_Battlecruiser) && !unit_is(u, UnitTypes::Hero_Hyperion) && !unit_is(u, UnitTypes::Hero_Norad_II) && !unit_is(u, UnitTypes::Hero_Gerard_DuGalle)) return false;
			return true;
		case TechTypes::Cloaking_Field:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Cloaking_Field)) return false;
			if (!unit_is_wraith(u)) return false;
			return true;
		case TechTypes::Personnel_Cloaking:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Personnel_Cloaking)) return false;
			if (!unit_is_ghost(u)) return false;
			return true;
		case TechTypes::Restoration:
			if (!player_has_researched(owner, TechTypes::Restoration)) return false;
			if (!unit_is(u, UnitTypes::Terran_Medic)) return false;
			return true;
		case TechTypes::Healing:
			if (!unit_is(u, UnitTypes::Terran_Medic)) return false;
			return true;
		case TechTypes::Burrowing:
			if (!ut_hero(u) && !unit_is(u, UnitTypes::Zerg_Lurker) && !player_has_researched(owner, TechTypes::Burrowing)) return false;
			if (!unit_is(u, UnitTypes::Zerg_Lurker) && !unit_is(u, UnitTypes::Zerg_Drone) && !unit_is(u, UnitTypes::Zerg_Infested_Terran) && !unit_is(u, UnitTypes::Zerg_Defiler) && !unit_is(u, UnitTypes::Zerg_Zergling) && !unit_is(u, UnitTypes::Zerg_Hydralisk) && !unit_is(u, UnitTypes::Hero_Unclean_One) && !unit_is(u, UnitTypes::Hero_Devouring_One) && !unit_is(u, UnitTypes::Hero_Hunter_Killer)) return false;
			return true;
		case TechTypes::Infestation:
			if (!unit_is_queen(u)) return false;
			return true;
		case TechTypes::Spawn_Broodlings:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Spawn_Broodlings)) return false;
			if (!unit_is_queen(u)) return false;
			return true;
		case TechTypes::Parasite:
			if (!unit_is_queen(u)) return false;
			return true;
		case TechTypes::Dark_Swarm:
			if (!unit_is_defiler(u)) return false;
			if (u_burrowed(u)) return false;
			return true;
		case TechTypes::Plague:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Plague)) return false;
			if (!unit_is_defiler(u)) return false;
			if (u_burrowed(u)) return false;
			return true;
		case TechTypes::Consume:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Consume)) return false;
			if (!unit_is_defiler(u) && !unit_is(u, UnitTypes::Hero_Infested_Duran) && !unit_is(u, UnitTypes::Hero_Infested_Kerrigan)) return false;
			if (u_burrowed(u)) return false;
			return true;
		case TechTypes::Ensnare:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Ensnare)) return false;
			if (!unit_is_queen(u) && !unit_is(u, UnitTypes::Hero_Infested_Kerrigan)) return false;
			if (u_burrowed(u)) return false;
			return true;
		case TechTypes::Psionic_Storm:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Psionic_Storm)) return false;
			if (!unit_is(u, UnitTypes::Protoss_High_Templar) && !unit_is(u, UnitTypes::Hero_Tassadar) && !unit_is(u, UnitTypes::Hero_Infested_Kerrigan)) return false;
			return true;
		case TechTypes::Hallucination:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Hallucination)) return false;
			if (!unit_is(u, UnitTypes::Protoss_High_Templar) && !unit_is(u, UnitTypes::Hero_Tassadar)) return false;
			return true;
		case TechTypes::Archon_Warp:
			if (!unit_is(u, UnitTypes::Protoss_High_Templar)) return false;
			return true;
		case TechTypes::Recall:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Recall)) return false;
			if (!unit_is_arbiter(u)) return false;
			return true;
		case TechTypes::Stasis_Field:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Stasis_Field)) return false;
			if (!unit_is_arbiter(u)) return false;
			return true;
		case TechTypes::Disruption_Web:
			if (!ut_hero(u) && !player_has_researched(owner, TechTypes::Disruption_Web)) return false;
			if (!unit_is(u, UnitTypes::Protoss_Corsair) && !unit_is(u, UnitTypes::Hero_Raszagal)) return false;
			return true;
		case TechTypes::Dark_Archon_Meld:
			if (!unit_is(u, UnitTypes::Protoss_Dark_Templar)) return false;
			return true;
		case TechTypes::Mind_Control:
			if (!player_has_researched(owner, TechTypes::Mind_Control)) return false;
			if (!unit_is(u, UnitTypes::Protoss_Dark_Archon)) return false;
			return true;
		case TechTypes::Feedback:
			if (!unit_is(u, UnitTypes::Protoss_Dark_Archon)) return false;
			return true;
		case TechTypes::Optical_Flare:
			if (!unit_is(u, UnitTypes::Terran_Medic)) return false;
			if (!player_has_researched(owner, TechTypes::Optical_Flare)) return false;
			return true;
		case TechTypes::Maelstrom:
			if (!player_has_researched(owner, TechTypes::Maelstrom)) return false;
			if (!unit_is(u, UnitTypes::Protoss_Dark_Archon)) return false;
			return true;
		case TechTypes::Lurker_Aspect:
			if (!player_has_researched(owner, TechTypes::Lurker_Aspect)) return false;
			if (!unit_is(u, UnitTypes::Zerg_Hydralisk)) return false;
			return true;
		default:
			return false;
		}
	}

	auto trigger_players(int owner, int player) const {
		return make_filter_range(all_player_slots, [this, owner, player](int n) {
			return trigger_players_pred(owner, player, n);
		});
	}

	template<typename T>
	int trigger_command_count(const T& count_obj, int owner, int player, int unit_id, bool completed_units) const {
		int r = 0;
		for (int p : trigger_players(owner, player)) {
			if (completed_units) {
				if (unit_id == 229) r += count_obj.non_building_counts[p] + count_obj.building_counts[p];
				else if (unit_id == 230) r += count_obj.non_building_counts[p];
				else if (unit_id == 231) r += count_obj.building_counts[p];
				else if (unit_id == 232) r += count_obj.factory_counts[p];
				else r += count_obj.unit_counts[owner].at((UnitTypes)unit_id);
			} else {
				if (unit_id == 229) r += count_obj.completed_non_building_counts[p] + count_obj.completed_building_counts[p];
				else if (unit_id == 230) r += count_obj.completed_non_building_counts[p];
				else if (unit_id == 231) r += count_obj.completed_building_counts[p];
				else if (unit_id == 232) r += count_obj.completed_factory_counts[p];
				else r += count_obj.completed_unit_counts[p].at((UnitTypes)unit_id);
			}
		}
		return r;
	}

	bool trigger_count_comparison(const trigger::condition& c, int count) const {
		if (c.num_n == 0) return count >= c.count_n;
		else if (c.num_n == 1) return count <= c.count_n;
		else if (c.num_n == 2) return count == c.count_n;
		else return false;
	}

	struct bring_unit_counters {
		std::array<type_indexed_array<int, UnitTypes>, 12> unit_counts;
		std::array<type_indexed_array<int, UnitTypes>, 12> completed_unit_counts;

		std::array<int, 12> factory_counts;
		std::array<int, 12> building_counts;
		std::array<int, 12> non_building_counts;

		std::array<int, 12> completed_factory_counts;
		std::array<int, 12> completed_building_counts;
		std::array<int, 12> completed_non_building_counts;
	};

	bring_unit_counters trigger_bring_count(const location& loc) const {
		bring_unit_counters r;
		r.unit_counts = {};
		r.completed_unit_counts = {};
		r.factory_counts = {};
		r.building_counts = {};
		r.non_building_counts = {};
		r.completed_factory_counts = {};
		r.completed_building_counts = {};
		r.completed_non_building_counts = {};
		std::function<void(const unit_t*)> add_completed = [&](const unit_t* u) {
			++r.completed_unit_counts[u->owner][u->unit_type->id];
			if (unit_provides_space(u)) {
				for (const unit_t* n : loaded_units(u)) add_completed(n);
			}
			if (unit_is_carrier(u)) {
				r.completed_unit_counts[u->owner][UnitTypes::Protoss_Interceptor] += u->carrier.inside_count;
			} else if (unit_is_reaver(u)) {
				r.completed_unit_counts[u->owner][UnitTypes::Protoss_Reaver] += u->carrier.inside_count;
			}
			if (ut_worker(u) && u->worker.powerup) {
				if (unit_is(u->worker.powerup, UnitTypes::Powerup_Flag)) add_completed(u->worker.powerup);
				else ++r.completed_unit_counts[u->owner][u->worker.powerup->unit_type->id];
			}
			if (unit_is(u, UnitTypes::Terran_Nuclear_Silo) && u->building.silo.ready) {
				++r.completed_unit_counts[u->owner][UnitTypes::Terran_Nuclear_Missile];
			}
			if (u->unit_type->group_flags & GroupFlags::Factory) ++r.completed_factory_counts[u->owner];
			if (u->unit_type->group_flags & GroupFlags::Men) ++r.completed_non_building_counts[u->owner];
			else if (u->unit_type->group_flags & GroupFlags::Building) ++r.completed_building_counts[u->owner];
		};
		for (const unit_t* u : find_units(loc.area)) {
			if (ut_turret(u)) continue;
			if (!unit_is_at_elevation_flags(u, loc.elevation_flags)) continue;

			++r.unit_counts[u->owner][u->unit_type->id];
			if (u->unit_type->group_flags & GroupFlags::Factory) ++r.factory_counts[u->owner];
			if (u->unit_type->group_flags & GroupFlags::Men) ++r.non_building_counts[u->owner];
			else if (u->unit_type->group_flags & GroupFlags::Building) ++r.building_counts[u->owner];
			else if (unit_is_egg(u)) ++r.non_building_counts[u->owner];

			if (u_completed(u)) add_completed(u);

		}
		return r;
	}

	int trigger_opponent_count(int owner, int player) const {
		int r = 0;
		for (int p : trigger_players(owner, player)) {
			r += range_size(trigger_players(p, 26));
		}
		return r;
	}

	bool test_trigger_condition(const trigger::condition& c, int owner) const {
		switch (c.type) {
		case 2: // command
			return trigger_count_comparison(c, trigger_command_count(st, owner, c.group, c.unit_id, c.num_n != 1));
		case 3: // bring
			return trigger_count_comparison(c, trigger_command_count(trigger_bring_count(st.locations.at(c.location - 1)), owner, c.group, c.unit_id, c.num_n != 1));
		case 12: // elapsed time
			return trigger_count_comparison(c, st.current_frame);
		case 14: // opponents
			return trigger_count_comparison(c, trigger_opponent_count(owner, c.group));
		case 22: // always
			return true;
		default:
			error("unknown trigger condition %d\n", c.type);
			return false;
		}
	}

	unit_t* trigger_create_unit(const unit_type_t* unit_type, xy pos, int owner) {
		if (~unit_type->staredit_availability_flags & 2) return nullptr;
		pos = restrict_unit_pos_to_bounds(pos, unit_type, map_bounds());
		if (ut_building(unit_type)) {
			xy top_left = restrict_pos_to_map_bounds(pos - unit_type->placement_size / 2);
			pos = top_left / 32 * 32 + unit_type->placement_size / 2;
			if (!can_place_building(nullptr, owner, unit_type, pos, true, false)) {
				// todo: show error?
				return nullptr;
			}
		}
		unit_t* u = create_unit(unit_type, pos, owner);
		if (!u) {
			display_last_error_for_player(owner);
			return nullptr;
		}
		bool spreads_creep = unit_type_spreads_creep(u);
		if (spreads_creep) {
			set_creep_building_tiles(u, u->sprite->position);
			add_creep_provider(u);
		} else if (ut_requires_creep(u)) {
			spread_creep_completely(u, u->sprite->position);
			add_creep_provider(u);
		}

		finish_building_unit(u);
		if (!place_completed_unit(u)) {
			// todo: show error?
			kill_unit(u);
			return nullptr;
		}
		complete_unit(u);

		if (spreads_creep) add_creep_provider(u);
		if (ut_addon(u)) set_unit_owner(u, 11, false);
		if (u_grounded_building(u) && unit_race(u) == race_t::terran) find_and_connect_addon(u);
		st.update_psionic_matrix = true;
		if (unit_is(u, UnitTypes::Resource_Vespene_Geyser) || unit_is_refinery(u)) {
			set_unit_resources(u, 5000);
		} else if (unit_is_mineral_field(u)) {
			set_unit_resources(u, 1500);
		}
		return u;
	}

	bool player_won(int owner) const {
		return st.players.at(owner).victory_state >= 3;
	}

	bool player_defeated(int owner) const {
		int s = st.players.at(owner).victory_state;
		return s && !player_won(owner);
	}

	bool player_slot_active(int n) const {
		if (n >= 8) return false;
		auto c = st.players.at(n).controller;
		if (c == player_t::controller_occupied) return !player_defeated(n);
		if (c == player_t::controller_computer_game) return !player_defeated(n);
		if (c == player_t::controller_neutral) return true;
		if (c == player_t::controller_rescue_passive) return true;
		if (c == player_t::controller_unused_rescue_active) return true;
		return false;
	}

	bool trigger_players_pred(int owner, int player, int n) const {
		if (!player_slot_active(n)) return false;
		if (player < 12) return n == player; // player index
		switch (player) {
		case 12: return false; // no players
		case 13: return n == owner; // current player
		case 14: return owner != n && st.alliances[owner][n] == 0; // enemy
		case 15: return owner != n && st.alliances[owner][n] == 2; // ally
		case 16: return owner != n && st.alliances[owner][n] == 1; // neutral
		case 17: return true; // all players
		case 18: return n < 8 && st.players[n].force == 1;
		case 19: return n < 8 && st.players[n].force == 2;
		case 20: return n < 8 && st.players[n].force == 3;
		case 21: return n < 8 && st.players[n].force == 4;
		case 26: return owner != n && (st.alliances[owner][n] != 2 || st.alliances[n][owner] != 2); // non allied victory
		default: return false;
		}
	}

	bool unit_is_at_elevation_flags(const unit_t* u, int elevation_flags) const {
		int ground_height = get_ground_height_at(u->sprite->position);
		if (u_flying(u)) {
			if (ground_height == 0) return (elevation_flags & 1) == 0;
			if (ground_height == 1) return (elevation_flags & 2) == 0;
			if (ground_height == 2) return (elevation_flags & 4) == 0;
		} else {
			if (ground_height == 0) return (elevation_flags & 8) == 0;
			if (ground_height == 1) return (elevation_flags & 0x10) == 0;
			if (ground_height == 2) return (elevation_flags & 0x20) == 0;
		}
		return true;
	}

	bool trigger_unit_pred(int owner, const unit_t* u, int player, int uid) const {
		if (unit_provides_space(u)) {
			for (unit_t* n : loaded_units(u)) {
				if (trigger_unit_pred(owner, n, player, uid)) return true;
			}
		}
		if (ut_worker(u) && u->worker.powerup) {
			if (trigger_unit_pred(owner, u->worker.powerup, player, uid)) return true;
		}

		if (unit_dying(u)) return false;
		if (ut_powerup(u)) error("trigger_unit_pred: powerup");
		if (!trigger_players_pred(owner, player, u->owner)) return false;
		if (uid == 229) return true;
		else if (uid == 230) {
			if (u->unit_type->group_flags & GroupFlags::Men) return true;
		} else if (uid == 231) {
			if (u->unit_type->group_flags & GroupFlags::Building) return true;
		} else if (uid == 232) {
			if (u->unit_type->group_flags & GroupFlags::Factory) return true;
		} else {
			if (unit_is(u, (UnitTypes)uid)) return true;
		}
		return false;
	}

	unit_t* trigger_find_unit(int owner, const location& loc, int player, int unit_id) {
		for (unit_t* u : find_units(loc.area)) {
			if (!unit_is_at_elevation_flags(u, loc.elevation_flags)) continue;
			if (trigger_unit_pred(owner, u, player, unit_id)) return u;
		}
		return nullptr;
	}

	struct execute_trigger_struct {
		std::array<int, 12> victory_state{};
	};

	bool execute_trigger_action(execute_trigger_struct& ets, int owner, running_trigger& rt, running_trigger::action& ra, const trigger::action& a) {
		switch (a.type) {
		case 1: // victory
			if (st.players[owner].controller == player_t::controller_occupied || st.players[owner].controller == player_t::controller_computer_game) {
				ets.victory_state[owner] = 3;
			}
			return true;
		case 2: // defeat
			if (st.players[owner].controller == player_t::controller_occupied || st.players[owner].controller == player_t::controller_computer_game) {
				ets.victory_state[owner] = 2;
			}
			return true;
		case 3: // preserve trigger
			rt.flags |= 4;
			return true;
		case 4: // wait
			if (st.trigger_waiting[owner]) return false;
			if (ra.flags & 1) {
				ra.flags &= ~1;
				return true;
			}
			if (rt.flags & 0x10) return true;
			st.trigger_waiting[owner] = true;
			st.trigger_wait_timers[owner] = a.time_n;
			ra.flags |= 1;
			return false;
		case 15: // ai script
			switch (a.group2_n) {
			case 0x3069562b:
				st.shared_vision[0] |= 1 << owner;
				break;
			case 0x3169562b:
				st.shared_vision[1] |= 1 << owner;
				break;
			case 0x3269562b:
				st.shared_vision[2] |= 1 << owner;
				break;
			case 0x3369562b:
				st.shared_vision[3] |= 1 << owner;
				break;
			case 0x3469562b:
				st.shared_vision[4] |= 1 << owner;
				break;
			case 0x3569562b:
				st.shared_vision[5] |= 1 << owner;
				break;
			case 0x3669562b:
				st.shared_vision[6] |= 1 << owner;
				break;
			case 0x3769562b:
				st.shared_vision[7] |= 1 << owner;
				break;
			default:
				error("unknown ai script");
			}
			return true;
		case 22: // kill unit
			for (int p : trigger_players(owner, a.group_n)) {
				int uid = a.extra_n;
				for (auto i = st.player_units[p].begin(); i != st.player_units[p].end();) {
					unit_t* u = &*i++;
					if (ut_turret(u)) continue;
					if (unit_dying(u)) continue;
					if (ut_powerup(u)) error("trigger kill unit fixme: powerup");
					if (u->owner != p) continue;
					if (uid == 229) kill_unit(u);
					else if (uid == 230) {
						if (u->unit_type->group_flags & GroupFlags::Men) kill_unit(u);
					} else if (uid == 231) {
						if (u->unit_type->group_flags & GroupFlags::Building) kill_unit(u);
					} else if (uid == 232) {
						if (u->unit_type->group_flags & GroupFlags::Factory) kill_unit(u);
					} else {
						if (unit_is(u, (UnitTypes)uid)) kill_unit(u);
					}
				}
			}
			return true;
		case 24: // remove unit
			for (int p : trigger_players(owner, a.group_n)) {
				int uid = a.extra_n;
				for (auto i = st.player_units[p].begin(); i != st.player_units[p].end();) {
					unit_t* u = &*i++;
					if (ut_turret(u)) continue;
					if (unit_dying(u)) continue;
					if (ut_powerup(u)) error("trigger kill unit fixme: powerup");
					if (u->owner != p) continue;
					bool ok = false;
					if (uid == 229) ok = true;
					else if (uid == 230) {
						if (u->unit_type->group_flags & GroupFlags::Men) ok = true;
					} else if (uid == 231) {
						if (u->unit_type->group_flags & GroupFlags::Building) ok = true;
					} else if (uid == 232) {
						if (u->unit_type->group_flags & GroupFlags::Factory) ok = true;
					} else {
						if (unit_is(u, (UnitTypes)uid)) ok = true;
					}
					if (!ok) continue;
					hide_unit(u);
					kill_unit(u);
				}
			}
			return true;
		case 25: // remove unit at location
			//for (int p : trigger_players(owner, a.group_n)) {
			if (true) {
				int uid = a.extra_n;
				std::function<void(unit_t*)> proc = [&](unit_t* u) {
					if (!unit_is_at_elevation_flags(u, st.locations[a.location - 1].elevation_flags)) return;

					if (unit_provides_space(u)) {
						for (unit_t* n : loaded_units(u)) {
							proc(n);
						}
					}
					if (ut_worker(u) && u->worker.powerup) {
						proc(u->worker.powerup);
					}

					if (ut_turret(u)) return;
					if (unit_dying(u)) return;
					if (ut_powerup(u)) error("trigger remove unit fixme: powerup");
					if (!trigger_players_pred(owner, a.group_n, u->owner)) return;
					bool ok = false;
					if (uid == 229) ok = true;
					else if (uid == 230) {
						if (u->unit_type->group_flags & GroupFlags::Men) ok = true;
					} else if (uid == 231) {
						if (u->unit_type->group_flags & GroupFlags::Building) ok = true;
					} else if (uid == 232) {
						if (u->unit_type->group_flags & GroupFlags::Factory) ok = true;
					} else {
						if (unit_is(u, (UnitTypes)uid)) ok = true;
					}
					if (!ok) return;
					hide_unit(u);
					kill_unit(u);

				};
				for (unit_t* u : find_units(st.locations.at(a.location - 1).area)) {
					proc(u);
				}
			}
			return true;
		case 26: // set resources
			for (int p : trigger_players(owner, a.group_n)) {
				if (p >= 8) continue;
				if (a.num_n == 7) {
					if (a.extra_n == 0 || a.extra_n == 2) {
						st.current_minerals[p] = a.group2_n;
						st.total_minerals_gathered[p] = a.group2_n;
					}
					if (a.extra_n == 1 || a.extra_n == 2) {
						st.current_gas[p] = a.group2_n;
						st.total_gas_gathered[p] = a.group2_n;
					}
				} else if (a.num_n == 8) {
					if (a.extra_n == 0 || a.extra_n == 2) {
						st.current_minerals[p] += a.group2_n;
						st.total_minerals_gathered[p] += a.group2_n;
					}
					if (a.extra_n == 1 || a.extra_n == 2) {
						st.current_gas[p] += a.group2_n;
						st.total_gas_gathered[p] += a.group2_n;
					}
				} else if (a.num_n == 8) {
					if (a.extra_n == 0 || a.extra_n == 2) {
						if (st.current_minerals[p] < a.group2_n) st.current_minerals[p] = 0;
						else st.current_minerals[p] -= a.group2_n;
					}
					if (a.extra_n == 1 || a.extra_n == 2) {
						if (st.current_gas[p] < a.group2_n) st.current_gas[p] = 0;
						else st.current_gas[p] -= a.group2_n;
					}
				}
			}
			return true;
		case 38: // move location
			if (true) {
				auto& loc = st.locations.at(a.location - 1);
				xy pos = (loc.area.from + loc.area.to) / 2;
				unit_t* u = trigger_find_unit(owner, loc, a.group_n, a.extra_n);
				if (u) pos = u->sprite->position;
				auto& target_loc = st.locations.at(a.group2_n - 1);
				xy diff = pos - (loc.area.from + loc.area.to) / 2;
				xy from = target_loc.area.from + diff;
				xy to = target_loc.area.to + diff;
				if (from.x < 0) {
					to.x -= from.x;
					from.x = 0;
				}
				if (from.y < 0) {
					to.y -= from.y;
					from.y = 0;
				}
				if (to.x >= (int)game_st.map_width) {
					from.x += (int)game_st.map_width - 1 - to.x;
					to.x = (int)game_st.map_width - 1;
				}
				if (to.y >= (int)game_st.map_height) {
					from.y += (int)game_st.map_height - 1 - to.y;
					to.y = (int)game_st.map_height - 1;
				}
				target_loc.area.from = from;
				target_loc.area.to = to;
			}
			return true;
		case 44: // create unit
			for (int p : trigger_players(owner, a.group_n)) {
				const unit_type_t* ut = get_unit_type((UnitTypes)a.extra_n);
				if (unit_is_mineral_field(ut) || unit_is(ut, UnitTypes::Resource_Vespene_Geyser)) p = 11;
				auto& loc = st.locations.at(a.location - 1);
				xy pos = (loc.area.from + loc.area.to) / 2;
				for (int i = 0; i != a.num_n; ++i) {
					trigger_create_unit(ut, pos, p);
				}
			}
			return true;
		case 46: // order
			for (unit_t* u : find_units(st.locations.at(a.location - 1).area)) {
				if (!trigger_players_pred(owner, a.group_n, u->owner)) continue;
				if (!unit_is_at_elevation_flags(u, st.locations[a.location - 1].elevation_flags)) continue;
				if (ut_building(u)) {
					if (u->order_type->id == Orders::BuildingLand || u->order_type->id == Orders::BuildingLiftoff) continue;
				}
				if (!u_can_move(u)) continue;
				if (unit_is(u, UnitTypes::Terran_Nuclear_Missile)) continue;
				if (unit_is_fighter(u)) continue;
				if (unit_is_rescuable(u)) continue;
				bool ok = false;
				if (a.extra_n == 229) ok = true;
				else if (a.extra_n == 230) ok = (u->unit_type->group_flags & GroupFlags::Men) != 0;
				else if (a.extra_n == 231) ok = (u->unit_type->group_flags & GroupFlags::Building) != 0;
				else if (a.extra_n == 232) ok = (u->unit_type->group_flags & GroupFlags::Factory) != 0;
				else ok = (int)u->unit_type->id == a.extra_n;
				if (!ok) continue;
				Orders o = Orders::Nothing;
				if (a.num_n == 0) o = Orders::Move;
				else if (a.num_n == 1) o = Orders::Patrol;
				else if (a.num_n == 2) o = Orders::AttackMove;
				const order_type_t* order = get_order_type(o);
				auto& target_loc = st.locations.at(a.group2_n - 1);
				xy target_pos = (target_loc.area.from + target_loc.area.to) / 2;
				if (u_burrowed(u)) {
					unburrow_unit(u);
					set_queued_order(u, false, order, target_pos);
				} else {
					if (!unit_can_receive_order(u, order, u->owner)) continue;
					set_unit_order(u, order, target_pos);
				}
			}
			return true;
		case 52: // set unit resources
			for (unit_t* u : find_units(st.locations.at(a.location - 1).area)) {
				if (!trigger_players_pred(owner, a.group_n, u->owner)) continue;
				if (!unit_is_at_elevation_flags(u, st.locations[a.location - 1].elevation_flags)) continue;
				set_unit_resources(u, a.group2_n);
			}
			return true;
		default:
			error("unknown trigger action %d", a.type);
			return false;
		}
	}

	void execute_trigger(execute_trigger_struct& ets, int owner, running_trigger& rt, const trigger& t) {
		rt.flags |= 1;
		size_t index = rt.current_action_index;
		for (;index != 64; ++index) {
			auto& a = t.actions[index];
			if (a.flags & 2) continue;
			if (a.type == 0) index = 63;
			else if (!execute_trigger_action(ets, owner, rt, rt.actions[index], a)) break;
		}
		rt.current_action_index = index;
		if (index == 64) {
			if (rt.flags & 4) {
				rt.current_action_index = 0;
				rt.flags &= ~0x51;
			} else rt.flags |= 8;
		}
	}

	bool tile_can_have_creep(xy_t<size_t> tile_pos) {
		size_t index = tile_pos.y * game_st.map_tile_width + tile_pos.x;
		if (st.tiles[index].flags & (tile_t::flag_unbuildable | tile_t::flag_partially_walkable)) return false;
		if (tile_pos.y == game_st.map_tile_height - 1) return true;
		if (st.tiles[index + game_st.map_tile_width].flags & tile_t::flag_unbuildable) return false;
		return true;
	}

	rect_t<xy_t<size_t>> get_max_creep_bb(unit_type_autocast unit_type, xy pos, bool unit_is_completed) {
		rect r;
		if (unit_type_spreads_creep(unit_type, unit_is_completed)) {
			r.from = pos - xy(320, 200);
			r.to = pos + xy(320, 200);
		} else {
			r.from = (pos / 32 - unit_type->placement_size / 32 / 2) * 32;
			r.to = (r.from / 32 + unit_type->placement_size / 32 - xy(1, 1)) * 32;
		}
		rect_t<xy_t<size_t>> rt;
		if (r.from.x <= 0) rt.from.x = 0;
		else rt.from.x = r.from.x / 32u;
		if (r.from.y <= 0) rt.from.y = 0;
		else rt.from.y = r.from.y / 32u;
		if (r.to.x >= (int)game_st.map_width) rt.to.x = game_st.map_tile_width - 1;
		else rt.to.x = r.to.x / 32u;
		if (r.to.y >= (int)game_st.map_height) rt.to.y = game_st.map_tile_height - 1;
		else rt.to.y = r.to.y / 32u;
		return rt;
	}

	size_t count_neighboring_creep_tiles(xy_t<size_t> tile_pos) {
		size_t r = 0;
		size_t width = game_st.map_tile_width;
		size_t index = tile_pos.y * width + tile_pos.x;
		index -= width;
		index -= 1;
		auto test = [&]() {
			if (st.tiles[index].flags & tile_t::flag_has_creep) ++r;
		};
		if (tile_pos.y != 0) {
			if (tile_pos.x != 0) test();
			++index;
			test();
			++index;
			if (tile_pos.x != width - 1) test();
			index -= 2;
		}
		index += width;
		if (tile_pos.x != 0) test();
		index += 2;
		if (tile_pos.x != width - 1) test();
		index -= 2;
		index += width;
		if (tile_pos.y != game_st.map_tile_height - 1) {
			if (tile_pos.x != 0) test();
			++index;
			test();
			++index;
			if (tile_pos.x != width - 1) test();
		}
		return r;
	}

	void set_tile_creep(xy_t<size_t> tile_pos, bool has_creep = true) {
		size_t index = tile_pos.y * game_st.map_tile_width + tile_pos.x;
		if (has_creep) st.tiles[index].flags |= tile_t::flag_has_creep;
		else st.tiles[index].flags &= ~tile_t::flag_has_creep;

		size_t width = game_st.map_tile_width;
		size_t height = game_st.map_tile_height;
		index -= width;
		--tile_pos.x;
		index -= 1;
		--tile_pos.y;
		auto test = [&]() {
			if (~st.tiles[index].flags & tile_t::flag_has_creep) return;
			if (~st.tiles[index].flags & tile_t::flag_creep_receding) return;
			auto* v = st.creep_life.table.find(tile_pos);
			if (!v) error("set_tile_creep: receding creep not found");
			size_t n_neighbors = count_neighboring_creep_tiles(tile_pos);
			if (v->n_neighboring_creep_tiles == n_neighbors) return;

			st.creep_life.lists[v->n_neighboring_creep_tiles].remove(*v);
			--st.creep_life.lists_size[v->n_neighboring_creep_tiles];
			v->n_neighboring_creep_tiles = n_neighbors;
			st.creep_life.lists[n_neighbors].push_front(*v);
			++st.creep_life.lists_size[n_neighbors];
		};
		if (tile_pos.y < height) {
			if (tile_pos.x < width) test();
			++index;
			++tile_pos.x;
			test();
			++index;
			++tile_pos.x;
			if (tile_pos.x < width) test();
			index -= 2;
			tile_pos.x -= 2;
		}
		index += width;
		++tile_pos.y;
		if (tile_pos.x < width) test();
		index += 2;
		tile_pos.x += 2;
		if (tile_pos.x < width) test();
		index -= 2;
		tile_pos.x -= 2;
		index += width;
		++tile_pos.y;
		if (tile_pos.y < height) {
			if (tile_pos.x < width) test();
			++index;
			++tile_pos.x;
			test();
			++index;
			++tile_pos.x;
			if (tile_pos.x < width) test();
		}
	}

	bool spread_creep(unit_type_autocast unit_type, xy pos, bool* out_any_tiles_occupied = nullptr) {
		std::array<static_vector<size_t, 240>, 8> target_tiles;
		bool spreads_creep = unit_type_spreads_creep(unit_type, true);
		auto area = get_max_creep_bb(unit_type, pos, true);
		int dy = (int)area.from.y * 32 - pos.y + 16;
		bool any_tiles_occupied = false;
		for (size_t tile_y = area.from.y; tile_y != area.to.y + 1; ++tile_y, dy += 32) {
			int dx = (int)area.from.x * 32 - pos.x + 16;
			for (size_t tile_x = area.from.x; tile_x != area.to.x + 1; ++tile_x, dx += 32) {
				size_t index = tile_y * game_st.map_tile_width + tile_x;
				auto flags = st.tiles[index].flags;
				if (flags & tile_t::flag_has_creep) continue;
				if (!tile_can_have_creep({tile_x, tile_y})) continue;
				if (flags & tile_t::flag_occupied) {
					if (!any_tiles_occupied) any_tiles_occupied = true;
					continue;
				}
				if (spreads_creep) {
					int d = dx*dx * 25 + dy*dy * 64;
					if (d > 320*320 * 25) continue;
				}
				size_t n = count_neighboring_creep_tiles({tile_x, tile_y});
				if (n == 0) continue;
				target_tiles[n - 1].push_back(index);
			}
		}
		if (out_any_tiles_occupied) *out_any_tiles_occupied = any_tiles_occupied;
		for (auto& v : reverse(target_tiles)) {
			if (v.empty()) continue;
			size_t index = v[(lcg_rand(26) >> 4) % v.size()];
			set_tile_creep({index % game_st.map_tile_width, index / game_st.map_tile_width});
			return true;
		}
		return false;
	}

	void spread_creep_completely(unit_type_autocast unit_type, xy pos) {
		rect_t<xy_t<size_t>> unit_area;
		unit_area.from.x = pos.x / 32u - unit_type->placement_size.x / 32u / 2;
		unit_area.from.y = pos.y / 32u - unit_type->placement_size.y / 32u / 2;
		unit_area.to.x = unit_area.from.x + unit_type->placement_size.x / 32u;
		unit_area.to.y = unit_area.from.y + unit_type->placement_size.y / 32u;
		st.tiles.at(unit_area.from.y * game_st.map_tile_width + unit_area.from.x);
		if (unit_area.from != unit_area.to) st.tiles.at((unit_area.to.y - 1) * game_st.map_tile_width + unit_area.to.x - 1);
		for (size_t y = unit_area.from.y; y != unit_area.to.y; ++y) {
			for (size_t x = unit_area.from.x; x != unit_area.to.x; ++x) {
				if (!tile_can_have_creep({x, y})) continue;
				set_tile_creep({x, y});
			}
		}
		while (spread_creep(unit_type, pos));
	}

	xy get_spawn_larva_position(unit_t* u) {
		if (!unit_is_hatchery(u)) error("get_spawn_larva_position: unit is not a hatchery");
		int best_score = 101;
		xy best_pos(-1, -1);
		auto test = [&](size_t index, xy pos, xy neighbor_offset) {
			int val = u->building.hatchery.larva_spawn_side_values[index];
			if (val >= best_score) return false;
			if (restrict_move_target_to_valid_bounds(get_unit_type(UnitTypes::Zerg_Larva), pos) != pos) return false;
			if (~st.tiles[tile_index(pos)].flags & tile_t::flag_has_creep) return false;
			auto op = pos + neighbor_offset;
			if (restrict_move_target_to_valid_bounds(get_unit_type(UnitTypes::Zerg_Larva), op) != op) return false;
			auto flags = st.tiles[tile_index(op)].flags;
			if (flags & tile_t::flag_occupied) return false;
			if ((flags & (tile_t::flag_walkable | tile_t::flag_has_creep)) == 0) return false;
			best_score = val;
			best_pos = pos;
			return true;
		};
		auto bb = unit_sprite_inner_bounding_box(u);
		test(3, {u->sprite->position.x, bb.to.y + 10}, {0, 22});
		test(0, {bb.from.x - 10, u->sprite->position.y}, {-22, 0});
		test(2, {bb.to.x + 10, u->sprite->position.y}, {22, 0});
		test(1, {u->sprite->position.x, bb.from.y - 10}, {0, -22});
		if (best_pos != xy(-1, -1)) return best_pos;
		int w = (u->unit_type->dimensions.from.x + u->unit_type->dimensions.to.x + 1) / 2;
		int h = (u->unit_type->dimensions.from.y + u->unit_type->dimensions.to.y + 1) / 2;
		best_score = 0x10000;
		if (!test(3, {u->sprite->position.x - w, bb.to.y + 10}, {0, 22}) && !test(3, {u->sprite->position.x + w, bb.to.y + 10}, {0, 22})) {
			best_score = 101;
		}
		test(0, {bb.from.x - 10, u->sprite->position.y - h}, {-22, 0}) || test(0, {bb.from.x - 10, u->sprite->position.y + h}, {-22, 0});
		test(2, {bb.to.x + 10, u->sprite->position.y - h}, {22, 0}) || test(2, {bb.to.x + 10, u->sprite->position.y + h}, {22, 0});
		test(1, {u->sprite->position.x - w, bb.from.y - 10}, {0, -22}) || test(1, {u->sprite->position.x + w, bb.from.y - 10}, {0, -22});
		return best_pos;
	}

	unit_t* spawn_larva(unit_t* u) {
		xy pos = get_spawn_larva_position(u);
		if (pos == xy(-1, -1)) return nullptr;
		unit_t* larva = create_unit(get_unit_type(UnitTypes::Zerg_Larva), pos, u->owner);
		if (larva) {
			finish_building_unit(larva);
			complete_unit(larva);
			larva->connected_unit = u;
			if (larva->sprite->position.x < u->sprite->position.x - u->unit_type->dimensions.from.x) {
				larva->order_state = 0;
			} else if (larva->sprite->position.y < u->sprite->position.y - u->unit_type->dimensions.from.y) {
				larva->order_state = 1;
			} else if (larva->sprite->position.x > u->sprite->position.x + u->unit_type->dimensions.to.x) {
				larva->order_state = 2;
			} else {
				larva->order_state = 3;
			}
		}
		return larva;
	}

};

struct state_copier {
	const state& st;
	state& r;
	state_functions funcs;
	state_copier(const state&st, state& r) : st(st), r(r), funcs(r) {}

	std::array<bool, 1700> unit_copied{};
	std::array<bool, 100> bullet_copied{};
	std::array<bool, 2500> sprite_copied{};
	std::array<bool, 5000> image_copied{};
	std::array<bool, 2000> order_copied{};
	unit_t* unit(const unit_t* v) {
		size_t index = v->index;
		auto* u = r.units_container.get(index, false);
		if (!unit_copied[index]) {
			unit_copied[index] = true;
			memcpy(u, v, sizeof(*v));
			remap_sprite(u->sprite);
			remap_unit(u->move_target.unit);
			remap_unit(u->order_target.unit);
			remap_unit(u->subunit);
			assemble(u->order_queue, v->order_queue, &state_copier::order);
			remap_unit(u->auto_target_unit);
			remap_unit(u->connected_unit);
			new (&u->build_queue) static_vector<const unit_type_t*, 5>(v->build_queue);
			if (u->unit_type) {
				if (funcs.unit_is(u, UnitTypes::Protoss_Interceptor) || funcs.unit_is(u, UnitTypes::Protoss_Scarab)) {
					remap_unit(u->fighter.parent);
				} else if (funcs.unit_is_carrier(u)) {
					assemble(u->carrier.inside_units, v->carrier.inside_units, &state_copier::unit);
					assemble(u->carrier.outside_units, v->carrier.outside_units, &state_copier::unit);
				} else if (funcs.unit_is_reaver(u)) {
					assemble(u->reaver.inside_units, v->reaver.inside_units, &state_copier::unit);
					assemble(u->reaver.outside_units, v->reaver.outside_units, &state_copier::unit);
				} else if (funcs.unit_is_ghost(u)) {
					u->ghost.nuke_dot = thingy(u->ghost.nuke_dot);
				}
			}
			remap_unit(u->worker.powerup);
			remap_unit(u->worker.target_resource_unit);
			remap_unit(u->worker.gather_target);
			remap_unit(u->building.addon);
			remap_unit(u->building.rally.unit);
			if (u->unit_type) {
				if (funcs.ut_resource(u)) {
					assemble(u->building.resource.gather_queue, v->building.resource.gather_queue, &state_copier::unit);
				} else if (funcs.unit_is_nydus(u)) {
					remap_unit(u->building.nydus.exit);
				} else if (funcs.unit_is(u, UnitTypes::Terran_Nuclear_Silo)) {
					remap_unit(u->building.silo.nuke);
				} else if (funcs.unit_is(u, UnitTypes::Protoss_Pylon)) {
					remap_sprite(u->building.pylon.psi_field_sprite);
				}
			}
			remap_unit(u->current_build_unit);
			u->path = path(u->path);
			remap_unit(u->irradiated_by);

		}
		return u;
	}
	template<typename T>
	void remap_unit(T& v) {
		if (v) v = unit(v);
	}
	bullet_t* bullet(const bullet_t* v) {
		if (!v) return nullptr;
		size_t index = v->index;
		auto* rv = r.bullets_container.get(index, false);
		if (!bullet_copied[index]) {
			bullet_copied[index] = true;
			memcpy(rv, v, sizeof(*v));
			remap_sprite(rv->sprite);
			remap_unit(rv->move_target.unit);
			remap_unit(rv->bullet_target);
			remap_unit(rv->bullet_owner_unit);
			remap_unit(rv->prev_bounce_unit);
		}
		return rv;
	}
	sprite_t* sprite(const sprite_t* v) {
		if (!v) return nullptr;
		size_t index = v->index;
		auto* rv = r.sprites_container.get(index, false);
		if (!sprite_copied[index]) {
			sprite_copied[index] = true;
			memcpy(rv, v, sizeof(*v));
			remap_image(rv->main_image);
			assemble(rv->images, v->images, &state_copier::image);
		}
		return rv;
	}
	template<typename T>
	void remap_sprite(T& v) {
		if (v) v = sprite(v);
	}
	image_t* image(const image_t* v) {
		if (!v) return nullptr;
		size_t index = v->index;
		auto* rv = r.images_container.get(index, false);
		if (!image_copied[index]) {
			image_copied[index] = true;
			*rv = *v;
			remap_sprite(rv->sprite);
		}
		return rv;
	}
	template<typename T>
	void remap_image(T& v) {
		if (v) v = image(v);
	}
	order_t* order(const order_t* v) {
		if (!v) return nullptr;
		size_t index = v->index;
		auto* rv = r.orders_container.get(index, false);
		if (!order_copied[index]) {
			order_copied[index] = true;
			*rv = *v;
			remap_unit(rv->target.unit);
		}
		return rv;
	}
	template<typename T>
	void remap_order(T& v) {
		if (v) v = order(v);
	}
	a_unordered_map<const path_t*, path_t*> path_remap;
	path_t* path(const path_t* v) {
		if (!v) return nullptr;
		auto*& rv = path_remap[v];
		if (!rv) {
			r.paths.emplace_back(*v);
			rv = &r.paths.back();
		}
		return rv;
	}
	a_unordered_map<const thingy_t*, thingy_t*> thingy_remap;
	thingy_t* thingy(const thingy_t* v) {
		if (!v) return nullptr;
		auto*& rv = thingy_remap[v];
		if (!rv) {
			r.thingies.emplace_back(*v);
			rv = &r.thingies.back();
			remap_sprite(rv->sprite);
		}
		return rv;
	}
	template<typename list_T, typename F>
	void assemble(list_T& dst_list, const list_T& src_list, F&& func) {
		list_T new_list;
		for (auto* v : ptr(src_list)) {
			new_list.push_back(*(this->*func)(v));
		}
		dst_list = std::move(new_list);
	}

	void operator()() {
		(state_base_copyable&)r = (state_base_copyable&)st;

		assemble(r.free_thingies, st.free_thingies, &state_copier::thingy);
		assemble(r.active_thingies, st.active_thingies, &state_copier::thingy);

		assemble(r.free_paths, st.free_paths, &state_copier::path);

		assemble(r.orders_container.free_list, st.orders_container.free_list, &state_copier::order);

		assemble(r.images_container.free_list, st.images_container.free_list, &state_copier::image);

		assemble(r.sprites_container.free_list, st.sprites_container.free_list, &state_copier::sprite);
		r.sprites_on_tile_line.resize(st.sprites_on_tile_line.size());
		for (size_t i = 0; i != r.sprites_on_tile_line.size(); ++i) {
			assemble(r.sprites_on_tile_line[i], st.sprites_on_tile_line[i], &state_copier::sprite);
		}

		assemble(r.bullets_container.free_list, st.bullets_container.free_list, &state_copier::bullet);
		assemble(r.active_bullets, st.active_bullets, &state_copier::bullet);

		assemble(r.cloaked_units, st.cloaked_units, &state_copier::unit);
		assemble(r.psionic_matrix_units, st.psionic_matrix_units, &state_copier::unit);
		for (size_t i = 0; i != 12; ++i) {
			assemble(r.player_units[i], st.player_units[i], &state_copier::unit);
		}
		assemble(r.units_container.free_list, st.units_container.free_list, &state_copier::unit);
		assemble(r.dead_units, st.dead_units, &state_copier::unit);
		assemble(r.map_revealer_units, st.map_revealer_units, &state_copier::unit);
		assemble(r.hidden_units, st.hidden_units, &state_copier::unit);
		assemble(r.visible_units, st.visible_units, &state_copier::unit);

		r.unit_finder_x = st.unit_finder_x;
		for (auto& v : r.unit_finder_x) remap_unit(v.u);
		r.unit_finder_y = st.unit_finder_y;
		for (auto& v : r.unit_finder_y) remap_unit(v.u);

		r.consider_collision_with_unit_bug = st.consider_collision_with_unit_bug;
		remap_unit(r.consider_collision_with_unit_bug);
		r.prev_bullet_source_unit = st.prev_bullet_source_unit;
		remap_unit(r.prev_bullet_source_unit);
	}
};

static inline state copy_state(const state& st) {
	state r;
	state_copier(st, r)();
	return r;
}


struct game_load_functions : state_functions {

	explicit game_load_functions(state& st) : state_functions(st) {}

	game_state& game_st = *st.game;

	struct setup_info_t {
		std::array<bool, 12> create_melee_units_for_player{};
		int victory_condition = 0;
		int tournament_mode = 0;
		int starting_units = 0;
		int resource_type = 0;
		int starting_minerals = 50;
		bool create_no_units = false;
	};
	setup_info_t setup_info;

	unit_type_t* get_unit_type(UnitTypes id) const {
		if ((size_t)id >= 228) error("invalid unit id %d", (size_t)id);
		return &game_st.unit_types.vec[(size_t)id];
	}
	const weapon_type_t* get_weapon_type(WeaponTypes id) const {
		if ((size_t)id >= 130) error("invalid weapon id %d", (size_t)id);
		return &game_st.weapon_types.vec[(size_t)id];
	}
	upgrade_type_t* get_upgrade_type(UpgradeTypes id) const {
		if ((size_t)id >= 61) error("invalid upgrade id %d", (size_t)id);
		return &game_st.upgrade_types.vec[(size_t)id];
	}
	tech_type_t* get_tech_type(TechTypes id) const {
		if ((size_t)id >= 44) error("invalid tech id %d", (size_t)id);
		return &game_st.tech_types.vec[(size_t)id];
	}
	const flingy_type_t* get_flingy_type(FlingyTypes id) const {
		if ((size_t)id >= 209) error("invalid flingy id %d", (size_t)id);
		return &global_st.flingy_types.vec[(size_t)id];
	}

	void reset() {

		game_st.unit_types = data_loading::load_units_dat(global_st.units_dat);
		game_st.weapon_types = data_loading::load_weapons_dat(global_st.weapons_dat);
		game_st.upgrade_types = data_loading::load_upgrades_dat(global_st.upgrades_dat);
		game_st.tech_types = data_loading::load_techdata_dat(global_st.techdata_dat);

		auto fixup_unit_type = [&](auto& ptr) {
			UnitTypes index{ptr};
			if (index == UnitTypes::None) ptr = nullptr;
			else ptr = get_unit_type(index);
		};
		auto fixup_weapon_type = [&](auto& ptr) {
			WeaponTypes index{ptr};
			if (index == WeaponTypes::None) ptr = nullptr;
			else ptr = get_weapon_type(index);
		};
		auto fixup_upgrade_type = [&](auto& ptr) {
			UpgradeTypes index{ptr};
			if (index == UpgradeTypes::None) ptr = nullptr;
			else ptr = get_upgrade_type(index);
		};
		auto fixup_flingy_type = [&](auto& ptr) {
			FlingyTypes index{ptr};
			ptr = get_flingy_type(index);
		};
		auto fixup_order_type = [&](auto& ptr) {
			Orders index{ptr};
			ptr = get_order_type(index);
		};
		auto fixup_image_type = [&](auto& ptr) {
			ImageTypes index{ptr};
			if (index == ImageTypes::None) ptr = nullptr;
			else ptr = get_image_type(index);
		};

		auto pos_coord = [&](int& v) {
			if (v < 0) v = 0;
			if (v >= 256 * 32) v = 256 * 32 - 1;
		};

		for (auto& v : game_st.unit_types.vec) {
			fixup_flingy_type(v.flingy);
			fixup_unit_type(v.turret_unit_type);
			fixup_unit_type(v.subunit2);
			fixup_unit_type(v.infestation_unit);
			fixup_image_type(v.construction_animation);
			fixup_weapon_type(v.ground_weapon);
			fixup_weapon_type(v.air_weapon);
			fixup_upgrade_type(v.armor_upgrade);
			fixup_order_type(v.computer_ai_idle);
			fixup_order_type(v.human_ai_idle);
			fixup_order_type(v.return_to_idle);
			fixup_order_type(v.attack_unit);
			fixup_order_type(v.attack_move);

			pos_coord(v.placement_size.x);
			pos_coord(v.placement_size.y);
			pos_coord(v.dimensions.from.x);
			pos_coord(v.dimensions.from.y);
			pos_coord(v.dimensions.to.x);
			pos_coord(v.dimensions.to.y);
		}
		for (auto& v : game_st.weapon_types.vec) {
			fixup_flingy_type(v.flingy);
			fixup_upgrade_type(v.damage_upgrade);
		}

		for (auto& v : game_st.unit_type_allowed) v.fill(true);
		for (auto& v : game_st.tech_available) v.fill(true);
		for (auto& v : game_st.max_upgrade_levels) {
			for (size_t i = 0; i != 61; ++i) {
				v[(UpgradeTypes)i] = get_upgrade_type((UpgradeTypes)i)->max_level;
			}
		}

		st.alliances = {};
		for (size_t i = 0; i != 12; ++i) {
			st.alliances[i] = {};
			st.alliances[i][i] = 1;
		}
		for (size_t i = 0; i != 12; ++i) {
			st.alliances[i][11] = 1;
			st.alliances[11][i] = 1;
		}

		st.upgrade_levels = {};
		st.upgrade_upgrading = {};
		st.tech_researched = {};
		st.tech_researching = {};

		st.unit_counts = {};
		st.completed_unit_counts = {};

		st.factory_counts = {};
		st.building_counts = {};
		st.non_building_counts = {};

		st.completed_factory_counts = {};
		st.completed_building_counts = {};
		st.completed_non_building_counts = {};

		st.total_buildings_ever_completed = {};
		st.total_non_buildings_ever_completed = {};

		st.unit_score = {};
		st.building_score = {};

		st.supply_used = {};
		st.supply_available = {};

		auto set_acquisition_ranges = [&]() {
			for (size_t i = 0; i != 228; ++i) {
				unit_type_t* unit_type = get_unit_type((UnitTypes)i);
				const unit_type_t* attacking_type = unit_type;
				if (unit_type->turret_unit_type) attacking_type = unit_type->turret_unit_type;
				const weapon_type_t* ground_weapon = attacking_type->ground_weapon;
				const weapon_type_t* air_weapon = attacking_type->air_weapon;
				int acq_range = attacking_type->target_acquisition_range;
				if (ground_weapon) acq_range = std::max(acq_range, ground_weapon->max_range / 32);
				if (air_weapon) acq_range = std::max(acq_range, air_weapon->max_range / 32);
				unit_type->target_acquisition_range = acq_range;
			}
		};
		set_acquisition_ranges();

		calculate_unit_strengths();

		generate_sight_values();

		load_tile_stuff();

		st.tiles.clear();
		st.tiles.resize(game_st.map_tile_width*game_st.map_tile_height);
		for (auto& v : st.tiles) {
			v.visible = 0xff;
			v.explored = 0xff;
		}
		st.tiles_mega_tile_index.clear();
		st.tiles_mega_tile_index.resize(st.tiles.size());

		st.update_tiles_countdown = 1;

		st.order_timer_counter = 10;
		st.secondary_order_timer_counter = 150;
		st.current_frame = 0;

		st.visible_units.clear();
		st.hidden_units.clear();
		st.map_revealer_units.clear();
		st.dead_units.clear();
		for (auto& v : st.player_units) v.clear();

		st.units_container = {};

		st.active_bullets_size = 0;
		st.active_bullets.clear();
		st.bullets_container = {};

		st.sprites_container = {};
		st.sprites_on_tile_line.clear();
		st.sprites_on_tile_line.resize(game_st.map_tile_height);

		st.images_container = {};

		st.active_orders_size = 0;
		st.orders_container = {};

		st.active_thingies_size = 0;
		st.active_thingies.clear();
		st.free_thingies.clear();
		st.thingies.clear();

		auto* cursor = create_thingy(get_sprite_type(SpriteTypes::SPRITEID_Cursor_Marker), {}, 0);
		if (cursor) {
			cursor->sprite->flags |= sprite_t::flag_hidden;
		}

		st.last_error = 0;
		st.recent_lurker_hit_current_index = 0;

		st.trigger_timer = 1;
		st.running_triggers = {};
		st.trigger_wait_timers = {};
		st.trigger_waiting = {};

		int max_unit_width = 0;
		int max_unit_height = 0;
		for (auto& v : game_st.unit_types.vec) {
			int width = v.dimensions.from.x + 1 + v.dimensions.to.x;
			int height = v.dimensions.from.y + 1 + v.dimensions.to.y;
			if (width > max_unit_width) max_unit_width = width;
			if (height > max_unit_height) max_unit_height = height;
		}
		game_st.max_unit_width = max_unit_width;
		game_st.max_unit_height = max_unit_height;

		st.random_counts = {};
		st.total_random_counts = 0;
		st.lcg_rand_state = 42;

		game_st.repulse_field_width = (game_st.map_width + 47) / 48;
		game_st.repulse_field_height = (game_st.map_height + 47) / 48;
		st.repulse_field.resize(game_st.repulse_field_width * game_st.repulse_field_height);

		game_st.triggers.clear();

		st.prev_bullet_source_unit = nullptr;
		st.consider_collision_with_unit_bug = nullptr;

		st.current_minerals = {};
		st.current_gas = {};
		st.total_minerals_gathered = {};
		st.total_gas_gathered = {};

		st.creep_life = {};
		st.update_psionic_matrix = false;
		st.disruption_webbed_units = 0;
		st.cheats_enabled = false;
		st.cheat_operation_cwal = false;

		st.locations.clear();
	}

	regions_t::region* get_new_region() {
		if (game_st.regions.regions.capacity() != 5000) game_st.regions.regions.reserve(5000);
		if (game_st.regions.regions.size() >= 5000) error("too many regions");
		game_st.regions.regions.emplace_back();
		regions_t::region* r = &game_st.regions.regions.back();
		r->index = game_st.regions.regions.size() - 1;
		return r;
	}

	void regions_create() {

		a_vector<uint8_t> unwalkable_flags(256 * 4 * 256 * 4);

		auto is_unwalkable = [&](size_t walk_x, size_t walk_y) {
			return unwalkable_flags[walk_y * 256 * 4 + walk_x] & 0x80 ? true : false;
		};
		auto is_walkable = [&](size_t walk_x, size_t walk_y) {
			return ~unwalkable_flags[walk_y * 256 * 4 + walk_x] & 0x80 ? true : false;
		};
		auto set_unwalkable = [&](size_t walk_x, size_t walk_y) {
			unwalkable_flags[walk_y * 256 * 4 + walk_x] |= 0x80;
		};
		auto is_dir_walkable = [&](size_t walk_x, size_t walk_y, size_t dir) {
			return ~unwalkable_flags[walk_y * 256 * 4 + walk_x] & (1 << dir) ? true : false;
		};
		auto is_dir_unwalkable = [&](size_t walk_x, size_t walk_y, size_t dir) {
			return unwalkable_flags[walk_y * 256 * 4 + walk_x] & (1 << dir) ? true : false;
		};
		auto set_dir_unwalkable = [&](size_t walk_x, size_t walk_y, size_t dir) {
			unwalkable_flags[walk_y * 256 * 4 + walk_x] |= 1 << dir;
		};
		auto flip_dir_walkable = [&](size_t walk_x, size_t walk_y, size_t dir) {
			unwalkable_flags[walk_y * 256 * 4 + walk_x] ^= 1 << dir;
		};
		auto is_every_dir_walkable = [&](size_t walk_x, size_t walk_y) {
			return unwalkable_flags[walk_y * 256 * 4 + walk_x] & 0x7f ? false : true;
		};

		auto set_unwalkable_flags = [&]() {

			for (size_t y = 0; y != game_st.map_tile_height; ++y) {
				for (size_t x = 0; x != game_st.map_tile_width; ++x) {
					uint16_t mega_tile_index = st.tiles_mega_tile_index[y * game_st.map_tile_width + x];

					auto& mt = game_st.vf4[mega_tile_index & 0x7fff];
					for (size_t sy = 0; sy < 4; ++sy) {
						for (size_t sx = 0; sx < 4; ++sx) {
							if (~mt.flags[sy * 4 + sx] & vf4_entry::flag_walkable) {
								set_unwalkable(x * 4 + sx, y * 4 + sy);
							}
						}
					}
				}
			}
			// Mark bottom part of map which is covered by the UI as unwalkable.
			if (game_st.map_walk_height >= 8) {
				for (size_t y = game_st.map_walk_height - 8; y != game_st.map_walk_height; ++y) {
					for (size_t x = 0; x != 20; ++x) {
						set_unwalkable(x, y);
					}
					if (game_st.map_walk_width >= 20) {
						for (size_t x = game_st.map_walk_width - 20; x != game_st.map_walk_width; ++x) {
							set_unwalkable(x, y);
						}
					}
					if (y >= game_st.map_walk_height - 4) {
						for (size_t x = 0; x != game_st.map_walk_width; ++x) {
							set_unwalkable(x, y);
						}
					}
				}
			}

			if (game_st.map_walk_width == 0 || game_st.map_walk_height == 0) error("map width/height is zero");

			for (size_t y = 0; y != game_st.map_walk_height; ++y) {
				for (size_t x = 0; x != game_st.map_walk_width; ++x) {
					if (is_unwalkable(x, y)) continue;
					if (y == 0 || is_unwalkable(x, y - 1)) set_dir_unwalkable(x, y, 0);
					if (x == game_st.map_walk_width - 1 || is_unwalkable(x + 1, y)) set_dir_unwalkable(x, y, 1);
					if (y == game_st.map_walk_height - 1 || is_unwalkable(x, y + 1)) set_dir_unwalkable(x, y, 2);
					if (x == 0 || is_unwalkable(x - 1, y)) set_dir_unwalkable(x, y, 3);
				}
			}
		};

		auto create_region = [&](rect_t<xy_t<size_t>> area) {
			auto* r = get_new_region();
			uint16_t flags = (uint16_t)game_st.regions.tile_region_index[area.from.y * 256 + area.from.x];
			if (flags < 5000) error("attempt to create region inside another region");
			r->flags = flags;
			r->tile_area = area;
			r->tile_center.x = (area.from.x + area.to.x) / 2;
			r->tile_center.y = (area.from.y + area.to.y) / 2;
			size_t tile_count = 0;
			size_t index = r->index;
			for (size_t y = area.from.y; y != area.to.y; ++y) {
				for (size_t x = area.from.x; x != area.to.x; ++x) {
					if (game_st.regions.tile_region_index[y * 256 + x] < 5000) error("attempt to create overlapping region");
					game_st.regions.tile_region_index[y * 256 + x] = (uint16_t)index;
					++tile_count;
				}
			}
			r->tile_count = tile_count;
			return r;
		};

		auto create_unreachable_bottom_region = [&]() {
			auto* r = create_region({ {0, game_st.map_tile_height - 1}, {game_st.map_tile_width, game_st.map_tile_height} });
			r->area = { {0, (int)game_st.map_height - 32}, {(int)game_st.map_width, (int)game_st.map_height} };
			r->center.x = ((fp8::integer(r->area.from.x) + fp8::integer(r->area.to.x)) / 2);
			r->center.y = ((fp8::integer(r->area.from.y) + fp8::integer(r->area.to.y)) / 2);
			r->flags = 0x1ffd;
			r->group_index = 0x4000;
		};

		auto create_regions = [&]() {

			size_t region_tile_index = 0;
			size_t region_x = 0;
			size_t region_y = 0;

			auto bb = game_st.regions.tile_bounding_box;

			auto find_empty_region = [&](size_t x, size_t y) {
				if (x >= bb.to.x) {
					x = bb.from.x;
					y = y + 1 >= bb.to.y ? bb.from.y : y + 1;
				}
				size_t start_x = x;
				size_t start_y = y;
				while (true) {
					size_t index = game_st.regions.tile_region_index[y * 256 + x];
					if (index >= 5000) {
						region_tile_index = index;
						region_x = x;
						region_y = y;
						return true;
					}
					++x;
					if (x >= bb.to.x) {
						x = bb.from.x;
						y = y + 1 >= bb.to.y ? bb.from.y : y + 1;
					}
					if (x == start_x && y == start_y) return false;
				}

			};

			size_t next_x = bb.from.x;
			size_t next_y = bb.from.y;

			bool has_expanded_all = false;
			size_t initial_regions_size = game_st.regions.regions.size();

			size_t prev_size = 7 * 8;

			while (true) {
				size_t start_x = next_x;
				size_t start_y = next_y;
				if (start_x >= bb.to.x) {
					start_x = bb.from.x;
					++start_y;
					if (start_y >= bb.to.y) start_y = bb.from.y;
				}
				if (find_empty_region(start_x, start_y)) {

					auto find_area = [this](size_t begin_x, size_t begin_y, size_t index) {
						size_t max_end_x = std::min(begin_x + 8, game_st.map_tile_width);
						size_t max_end_y = std::min(begin_y + 7, game_st.map_tile_height);

						size_t end_x = begin_x + 1;
						size_t end_y = begin_y + 1;
						bool x_is_good = true;
						bool y_is_good = true;
						bool its_all_good = true;
						while ((x_is_good || y_is_good) && (end_x != max_end_x && end_y != max_end_y)) {
							if (x_is_good) {
								for (size_t y = begin_y; y != end_y; ++y) {
									if (game_st.regions.tile_region_index[y * 256 + end_x] != index) {
										x_is_good = false;
										break;
									}
								}
							}
							if (y_is_good) {
								for (size_t x = begin_x; x != end_x; ++x) {
									if (game_st.regions.tile_region_index[end_y * 256 + x] != index) {
										y_is_good = false;
										break;
									}
								}
							}

							if (game_st.regions.tile_region_index[end_y * 256 + end_x] != index) {
								its_all_good = false;
							}
							if (its_all_good) {
								if (y_is_good) ++end_y;
								if (x_is_good) ++end_x;
							} else {
								if (y_is_good) ++end_y;
								else if (x_is_good) ++end_x;
							}
						}

						size_t width = end_x - begin_x;
						size_t height = end_y - begin_y;
						if (width > height * 3) width = height * 3;
						else if (height > width * 3) height = width * 3;
						end_x = begin_x + width;
						end_y = begin_y + height;

						return rect_t<xy_t<size_t>>{ {begin_x, begin_y}, {end_x, end_y} };
					};

					auto area = find_area(region_x, region_y, region_tile_index);

					size_t size = (area.to.x - area.from.x) * (area.to.y - area.from.y);
					if (size < prev_size) {
						auto best_area = area;
						size_t best_size = size;

						for (size_t n = 0; n != 25; ++n) {
							if (!find_empty_region(area.to.x, region_y)) break;
							area = find_area(region_x, region_y, region_tile_index);
							size_t size = (area.to.x - area.from.x) * (area.to.y - area.from.y);
							if (size > best_size) {
								best_size = size;
								best_area = area;
								if (size >= prev_size) break;
							}
						}

						area = best_area;
						size = best_size;
					}

					prev_size = size;

					next_x = area.to.x;
					next_y = area.from.y;

					if (game_st.regions.regions.size() >= 5000) error("too many regions (nooks and crannies)");

					auto* r = create_region(area);

					auto expand = [this](regions_t ::region* r) {

						size_t& begin_x = r->tile_area.from.x;
						if (begin_x > 0) --begin_x;
						size_t& begin_y = r->tile_area.from.y;
						if (begin_y > 0) --begin_y;
						size_t& end_x = r->tile_area.to.x;
						if (end_x < game_st.map_tile_width) ++end_x;
						size_t& end_y = r->tile_area.to.y;
						if (end_y < game_st.map_tile_height) ++end_y;

						uint16_t flags = r->flags;
						size_t index = r->index;

						auto is_neighbor = [&](size_t x, size_t y) {
							if (x != 0 && game_st.regions.tile_region_index[y * 256 + x - 1] == index) return true;
							if (x != game_st.map_tile_width - 1 && game_st.regions.tile_region_index[y * 256 + x + 1] == index) return true;
							if (y != 0 && game_st.regions.tile_region_index[(y - 1) * 256 + x] == index) return true;
							if (y != game_st.map_tile_height - 1 && game_st.regions.tile_region_index[(y + 1) * 256 + x] == index) return true;
							return false;
						};

						for (int i = 0; i < 2; ++i) {
							for (size_t y = begin_y; y != end_y; ++y) {
								for (size_t x = begin_x; x != end_x; ++x) {
									if (game_st.regions.tile_region_index[y * 256 + x] == flags && is_neighbor(x, y)) {
										game_st.regions.tile_region_index[y * 256 + x] = index;
									}
								}
							}
						}

					};

					expand(r);

					if (size <= 6 && !has_expanded_all) {
						has_expanded_all = true;
						for (size_t i = initial_regions_size; i != game_st.regions.regions.size(); ++i) {
							expand(&game_st.regions.regions[i]);
						}
					}

				} else {
					if (game_st.regions.regions.size() >= 5000) error("too many regions (nooks and crannies)");
					break;
				}
			}

			auto get_neighbors = [&](size_t tile_x, size_t tile_y) {
				std::array<size_t, 8> r;
				size_t n = 0;
				auto test = [&](bool cond, size_t x, size_t y) {
					if (!cond) r[n++] = 0x1fff;
					else r[n++] = game_st.regions.tile_region_index[y * 256 + x];
				};
				test(tile_y > 0, tile_x, tile_y - 1);
				test(tile_x > 0, tile_x - 1, tile_y);
				test(tile_x + 1 < game_st.map_tile_width, tile_x + 1, tile_y);
				test(tile_y + 1 < game_st.map_tile_height, tile_x, tile_y + 1);
				test(tile_y > 0 && tile_x > 0, tile_x - 1, tile_y - 1);
				test(tile_y > 0 && tile_x + 1 < game_st.map_tile_width, tile_x + 1, tile_y - 1);
				test(tile_y + 1 < game_st.map_tile_height && tile_x > 0, tile_x - 1, tile_y + 1);
				test(tile_y + 1 < game_st.map_tile_height && tile_x + 1 < game_st.map_tile_width, tile_x + 1, tile_y + 1);
				return r;
			};

			auto refresh_regions = [&]() {

				for (auto* r : ptr(game_st.regions.regions)) {
					int max = std::numeric_limits<int>::max();
					int min = std::numeric_limits<int>::min();
					r->area = { { max, max }, { min, min } };
					r->tile_count = 0;
				}
				for (size_t y = 0; y != game_st.map_tile_height; ++y) {
					for (size_t x = 0; x != game_st.map_tile_width; ++x) {
						size_t index = game_st.regions.tile_region_index[y * 256 + x];
						if (index < 5000) {
							auto* r = &game_st.regions.regions[index];
							++r->tile_count;
							if (r->area.from.x >(int)x * 32) r->area.from.x = int(32 * x);
							if (r->area.from.y > (int)y * 32) r->area.from.y = int(32 * y);
							if (r->area.to.x < ((int)x + 1) * 32) r->area.to.x = int(32 * (x + 1));
							if (r->area.to.y < ((int)y + 1) * 32) r->area.to.y = int(32 * (y + 1));
						}
					}
				}

				for (auto* r : ptr(game_st.regions.regions)) {
					if (r->tile_count == 0) r->flags = 0x1fff;
				}

				for (auto* r : ptr(game_st.regions.regions)) {
					if (r->tile_count == 0) continue;

					r->walkable_neighbors.clear();
					r->non_walkable_neighbors.clear();

					for (int y = r->area.from.y / 32; y != r->area.to.y / 32; ++y) {
						for (int x = r->area.from.x / 32; x != r->area.to.x / 32; ++x) {
							if (game_st.regions.tile_region_index[y * 256 + x] != r->index) continue;
							auto neighbors = get_neighbors(x, y);
							for (size_t i = 0; i != 8; ++i) {
								size_t nindex = neighbors[i];
								if (nindex == 0x1fff || nindex == r->index) continue;
								auto* nr = &game_st.regions.regions[nindex];
								bool add = false;
								if (i < 4 || !r->walkable() || !nr->walkable()) {
									add = true;
								} else {
									auto is_2x2_walkable = [&](size_t walk_x, size_t walk_y) {
										if (!is_walkable(walk_x, walk_y)) return false;
										if (!is_walkable(walk_x + 1, walk_y)) return false;
										if (!is_walkable(walk_x, walk_y + 1)) return false;
										if (!is_walkable(walk_x + 1, walk_y + 1)) return false;
										return true;
									};


									size_t walk_x = x * 4;
									size_t walk_y = y * 4;
									if (i == 4) {
										if (is_2x2_walkable(walk_x - 2, walk_y - 2) && is_2x2_walkable(walk_x, walk_y)) {
											if (is_2x2_walkable(walk_x - 2, walk_y)) add = true;
											else if (is_2x2_walkable(walk_x, walk_y - 2)) add = true;
										}
									} else if (i == 5) {
										if (is_2x2_walkable(walk_x + 4, walk_y - 2) && is_2x2_walkable(walk_x + 2, walk_y)) {
											if (is_2x2_walkable(walk_x + 2, walk_y - 2)) add = true;
											else if (is_2x2_walkable(walk_x + 4, walk_y)) add = true;
										}
									} else if (i == 6) {
										if (is_2x2_walkable(walk_x, walk_y + 2) && is_2x2_walkable(walk_x - 2, walk_y + 4)) {
											if (is_2x2_walkable(walk_x - 2, walk_y + 2)) add = true;
											else if (is_2x2_walkable(walk_x, walk_y + 4)) add = true;
										}
									} else if (i == 7) {
										if (is_2x2_walkable(walk_x + 2, walk_y + 2) && is_2x2_walkable(walk_x + 4, walk_y + 4)) {
											if (is_2x2_walkable(walk_x + 4, walk_y + 2)) add = true;
											else if (is_2x2_walkable(walk_x + 2, walk_y + 4)) add = true;
										}
									}
								}
								if (add) {
									if (nr->walkable()) {
										if (std::find(r->walkable_neighbors.begin(), r->walkable_neighbors.end(), nr) == r->walkable_neighbors.end()) {
											r->walkable_neighbors.push_back(nr);
										}
									} else {
										if (std::find(r->non_walkable_neighbors.begin(), r->non_walkable_neighbors.end(), nr) == r->non_walkable_neighbors.end()) {
											r->non_walkable_neighbors.push_back(nr);
										}
									}
								}
							}
						}
					}

					if (!r->non_walkable_neighbors.empty()) {
						for (auto& v : r->non_walkable_neighbors) {
							if (v == &game_st.regions.regions.front() && &v != &r->non_walkable_neighbors.back()) std::swap(v, r->non_walkable_neighbors.back());
						}
					}

				}

				for (auto* r : ptr(game_st.regions.regions)) {
					r->center = {fp8::integer(r->tile_center.x * 32 + 16), fp8::integer(r->tile_center.y * 32 + 16)};
				}

				for (auto* r : ptr(game_st.regions.regions)) {
					if (r->group_index < 0x4000) r->group_index = 0;
				}
				a_vector<regions_t::region*> stack;
				size_t next_group_index = 1;
				for (auto* r : ptr(game_st.regions.regions)) {
					if (r->group_index == 0 && r->tile_count) {
						size_t group_index = next_group_index++;
						r->group_index = group_index;
						stack.push_back(r);
						while (!stack.empty()) {
							auto* cr = stack.back();
							stack.pop_back();
							for (auto* nr : (cr->walkable() ? cr->walkable_neighbors : cr->non_walkable_neighbors)) {
								if (nr->group_index == 0) {
									nr->group_index = group_index;
									stack.push_back(nr);
								}
							}
						}
					}
				}

			};

			refresh_regions();

			for (size_t n = 6;; n += 2) {

				for (auto* r : reverse(ptr(game_st.regions.regions))) {
					if (r->tile_count == 0 || r->tile_count >= n || r->group_index >= 0x4000) continue;
					regions_t::region* smallest_neighbor = nullptr;
					auto eval = [&](auto* nr) {
						if (nr->tile_count == 0 || nr->group_index >= 0x4000 || nr->flags != r->flags) return;
						if (!smallest_neighbor || nr->tile_count < smallest_neighbor->tile_count) {
							smallest_neighbor = nr;
						}
					};
					for (auto* nr : r->walkable_neighbors) eval(nr);
					for (auto* nr : r->non_walkable_neighbors) eval(nr);
					if (smallest_neighbor) {
						auto* merge_into = smallest_neighbor;
						for (size_t y = r->area.from.y / 32u; y != r->area.to.y / 32u; ++y) {
							for (size_t x = r->area.from.x / 32u; x != r->area.to.x / 32u; ++x) {
								size_t& index = game_st.regions.tile_region_index[y * 256 + x];
								if (index == r->index) index = merge_into->index;
							}
						}
						merge_into->tile_count += r->tile_count;
						r->tile_count = 0;
						r->flags = 0x1fff;
						if (r->area.from.x < merge_into->area.from.x) merge_into->area.from.x = r->area.from.x;
						if (r->area.from.y < merge_into->area.from.y) merge_into->area.from.y = r->area.from.y;
						if (r->area.to.x > merge_into->area.to.x) merge_into->area.to.x = r->area.to.x;
						if (r->area.to.y > merge_into->area.to.y) merge_into->area.to.y = r->area.to.y;
					}
				}

				size_t n_non_empty_regions = 0;
				for (auto* r : ptr(game_st.regions.regions)) {
					if (r->tile_count) ++n_non_empty_regions;
				}
				if (n_non_empty_regions < 2500) break;
			}

			a_vector<size_t> reindex(5000);
			size_t new_region_count = 0;
			for (size_t i = 0; i != game_st.regions.regions.size(); ++i) {
				auto* r = &game_st.regions.regions[i];
				r->walkable_neighbors.clear();
				r->non_walkable_neighbors.clear();
				if (r->tile_count == 0) continue;
				size_t new_index = new_region_count++;
				reindex[i] = new_index;
				if (i != new_index) {
					r->index = new_index;
					game_st.regions.regions[new_index] = std::move(*r);
				}
			}
			for (size_t y = 0; y != game_st.map_tile_height; ++y) {
				for (size_t x = 0; x != game_st.map_tile_width; ++x) {
					size_t& index = game_st.regions.tile_region_index[y * 256 + x];
					index = reindex[index];
				}
			}
			game_st.regions.regions.resize(new_region_count);

			refresh_regions();

			game_st.regions.split_regions.clear();

			for (size_t y = 0; y != game_st.map_tile_height; ++y) {
				for (size_t x = 0; x != game_st.map_tile_width; ++x) {
					size_t index = y * game_st.map_tile_width + x;
					auto tile = st.tiles[index];
					if (~tile.flags & tile_t::flag_partially_walkable) continue;
					auto neighbors = get_neighbors(x, y);
					auto* r = &game_st.regions.regions[game_st.regions.tile_region_index[y * 256 + x]];
					auto count_4x1_walkable = [&](size_t walk_x, size_t walk_y) {
						size_t r = 0;
						if (is_walkable(walk_x, walk_y)) ++r;
						if (is_walkable(walk_x + 1, walk_y)) ++r;
						if (is_walkable(walk_x + 2, walk_y)) ++r;
						if (is_walkable(walk_x + 3, walk_y)) ++r;
						return r;
					};
					auto count_1x4_walkable = [&](size_t walk_x, size_t walk_y) {
						size_t r = 0;
						if (is_walkable(walk_x, walk_y)) ++r;
						if (is_walkable(walk_x, walk_y + 1)) ++r;
						if (is_walkable(walk_x, walk_y + 2)) ++r;
						if (is_walkable(walk_x, walk_y + 3)) ++r;
						return r;
					};
					uint16_t mask = 0;
					size_t megatile_index = st.tiles_mega_tile_index[index];
					for (size_t sy = 0; sy != 4; ++sy) {
						for (size_t sx = 0; sx != 4; ++sx) {
							int flags = game_st.vf4.at(megatile_index).flags[sy * 4 + sx];
							mask >>= 1;
							if (~flags & vf4_entry::flag_walkable) {
								mask |= 0x8000;
							}
						}
					}

					size_t walk_x = x * 4;
					size_t walk_y = y * 4;
					regions_t::region* r2 = nullptr;
					if (!r->walkable()) {
						std::array<size_t, 4> n_walkable {};
						n_walkable[0] = count_4x1_walkable(walk_x, walk_y);
						n_walkable[1] = count_1x4_walkable(walk_x, walk_y);
						n_walkable[2] = count_1x4_walkable(walk_x + 3, walk_y);
						n_walkable[3] = count_4x1_walkable(walk_x, walk_y + 3);
						size_t highest_n = 0;
						size_t highest_nindex;
						for (size_t i = 4; i != 0;) {
							--i;
							size_t n = n_walkable[i];
							if (n <= highest_n) continue;
							size_t nindex = neighbors[i];
							if (nindex == r->index) continue;
							if (nindex < 0x2000) {
								if (nindex >= 5000) continue;
								auto* nr = &game_st.regions.regions[nindex];
								if (nr->walkable()) {
									highest_n = n;
									highest_nindex = nindex;
								}
							} else {
								bool set = false;
								if (i == 0 && count_4x1_walkable(walk_x, walk_y - 1)) set = true;
								if (i == 1 && count_1x4_walkable(walk_x - 1, walk_y)) set = true;
								if (i == 2 && count_1x4_walkable(walk_x + 4, walk_y)) set = true;
								if (i == 3 && count_4x1_walkable(walk_x, walk_y + 4)) set = true;
								if (set) {
									highest_n = n;
									highest_nindex = nindex;
								}
							}
						}
						if (highest_n) {
							if (highest_nindex < 0x2000) r2 = &game_st.regions.regions[highest_nindex];
							else r2 = game_st.regions.split_regions[highest_nindex - 0x2000].a;
						}
					} else {
						std::array<size_t, 8> n_unwalkable {};
						n_unwalkable[0] = 4 - count_4x1_walkable(walk_x, walk_y);
						n_unwalkable[1] = 4 - count_1x4_walkable(walk_x, walk_y);
						n_unwalkable[2] = 4 - count_1x4_walkable(walk_x + 3, walk_y);
						n_unwalkable[3] = 4 - count_4x1_walkable(walk_x, walk_y + 3);
						n_unwalkable[4] = is_walkable(walk_x, walk_y) ? 0 : 1;
						n_unwalkable[5] = is_walkable(walk_x + 3, walk_y) ? 0 : 1;
						n_unwalkable[6] = is_walkable(walk_x, walk_y + 3) ? 0 : 1;
						n_unwalkable[7] = is_walkable(walk_x + 3, walk_y + 3) ? 0 : 1;
						size_t highest_n = 0;
						size_t highest_nindex;
						for (size_t i = 8; i != 0;) {
							--i;
							size_t n = n_unwalkable[i];
							if (n <= highest_n) continue;
							size_t nindex = neighbors[i];
							if (nindex == r->index) continue;
							if (nindex < 0x2000) {
								if (nindex >= 5000) continue;
								auto* nr = &game_st.regions.regions[nindex];
								if (!nr->walkable()) {
									highest_n = n;
									highest_nindex = nindex;
								}
							} else {
								bool set = false;
								if (i == 0 && count_4x1_walkable(walk_x, walk_y - 1) != 4) set = true;
								if (i == 1 && count_1x4_walkable(walk_x - 1, walk_y) != 4) set = true;
								if (i == 2 && count_1x4_walkable(walk_x + 4, walk_y) != 4) set = true;
								if (i == 3 && count_4x1_walkable(walk_x, walk_y + 4) != 4) set = true;
								if (i == 4 && !is_walkable(walk_x - 1, walk_y - 1)) set = true;
								if (i == 5 && !is_walkable(walk_x + 4, walk_y - 1)) set = true;
								if (i == 6 && !is_walkable(walk_x - 1, walk_y + 4)) set = true;
								if (i == 7 && !is_walkable(walk_x + 4, walk_y + 4)) set = true;
								if (set) {
									highest_n = n;
									highest_nindex = nindex;
								}
							}
						}
						if (highest_n) {
							if (highest_nindex < 0x2000) r2 = &game_st.regions.regions[highest_nindex];
							else r2 = game_st.regions.split_regions[highest_nindex - 0x2000].b;
						}
					}
					if (!r2 || r2 == r) mask = r->walkable() ? 0 : 0xffff;
					else if (!r->walkable() && !r2->walkable()) mask = 0xffff;
					game_st.regions.tile_region_index[y * 256 + x] = 0x2000 + game_st.regions.split_regions.size();
					if (r->walkable()) {
						game_st.regions.split_regions.push_back({ mask, r, r2 ? r2 : r });
					} else {
						game_st.regions.split_regions.push_back({ mask, r2 ? r2 : r, r });
					}
				}
			}

		};

		auto create_contours = [&]() {

			game_st.regions.contours = {};

			size_t next_x = 0;
			size_t next_y = 0;
			auto next = [&]() {
				size_t x = next_x;
				size_t y = next_y;
				if (x >= game_st.map_walk_width) {
					error("create_contours::next: unreachable");
					x = 0;
					++y;
					if (y >= game_st.map_walk_height) y = 0;
				}
				x = x / 4 * 4;
				size_t start_x = x;
				size_t start_y = y;
				while (is_every_dir_walkable(x, y) && is_every_dir_walkable(x + 1, y) && is_every_dir_walkable(x + 2, y) && is_every_dir_walkable(x + 3, y)) {
					x += 4;
					if (x == game_st.map_walk_width) {
						x = 0;
						++y;
						if (y == game_st.map_walk_height) {
							y = 0;
						}
					}
					if (x == start_x && y == start_y) return false;
				}
				while (!is_walkable(x, y) || is_every_dir_walkable(x, y)) {
					++x;
					if (x == game_st.map_walk_width) error("create_contours: out of bounds");
				}
				next_x = x;
				next_y = y;
				return true;
			};

			while (next()) {

				std::array<int, 16> clut = { -1,  -1, 8, -1, 8, 8, -1, 8, 0, -1, 8, 0, 7, 8, -1, 7 };
				std::array<int, 16> nlut = { 8, -1, 8, 8, -1, 8, -1, -1, 7, -1, 8, 7, 0, 8, -1, 0 };

				size_t x = next_x;
				size_t y = next_y;

				if (is_dir_walkable(x, y, 0) && is_dir_walkable(x, y, 1) && is_dir_unwalkable(x, y, 2)) {
					++y;
					--x;
				}
				size_t first_unwalkable_dir = 0;
				int lut1val = is_dir_unwalkable(x, y, 0) && is_dir_unwalkable(x, y, 3) ? 0 : 1;
				for (int i = 0; i < 4; ++i) {
					if (is_dir_unwalkable(x, y, i)) {
						first_unwalkable_dir = i;
						break;
					}
				}

				size_t clut_index = first_unwalkable_dir + 4 * lut1val;
				int start_cx = int(clut[clut_index * 2] + x * 8);
				int start_cy = int(clut[clut_index * 2 + 1] + y * 8);

				flip_dir_walkable(x, y, first_unwalkable_dir);

				int cx = start_cx;
				int cy = start_cy;

				auto cur_dir = first_unwalkable_dir;
				while (true) {
					auto next_dir = (cur_dir + 1) & 3;
					int relx = next_dir == 1 ? 1 : next_dir == 3 ? -1 : 0;
					int rely = next_dir == 0 ? -1 : next_dir == 2 ? 1 : 0;
					flip_dir_walkable(x, y, cur_dir);
					int next_walkable = 0;
					if (is_dir_walkable(x, y, next_dir)) {
						next_walkable = 1;
						while (is_dir_unwalkable(x + relx, y + rely, cur_dir)) {
							x += relx;
							y += rely;
							flip_dir_walkable(x, y, cur_dir);
							if (is_dir_unwalkable(x, y, next_dir)) {
								next_walkable = 0;
								break;
							}
						}
					}
					size_t nlut_index = cur_dir + 4 * next_walkable;

					int nx = int(nlut[nlut_index * 2] + x * 8);
					int ny = int(nlut[nlut_index * 2 + 1] + y * 8);

					uint8_t flags0 = (uint8_t)(cur_dir ^ 2 * lut1val ^ (~(2 * cur_dir) & 3));
					uint8_t flags1 = (uint8_t)(cur_dir ^ 2 * (next_walkable ^ (cur_dir & 1)) ^ 1);
					if (cur_dir == 0) {
						uint8_t flags = (flags0 & 3) | 4 * ((flags1 & 3) | 4 * (lut1val | 2 * next_walkable));
						game_st.regions.contours[cur_dir].push_back({ {cy, cx, nx}, cur_dir, flags });
					} else if (cur_dir == 1) {
						uint8_t flags = (flags0 & 3) | 4 * ((flags1 & 3) | 4 * (lut1val | 2 * next_walkable));
						game_st.regions.contours[cur_dir].push_back({ { cx, cy, ny}, cur_dir, flags });
					} else if (cur_dir == 2) {
						uint8_t flags = (flags1 & 3) | 4 * ((flags0 & 3) | 4 * (next_walkable | 2 * lut1val));
						game_st.regions.contours[cur_dir].push_back({ { cy, nx, cx}, cur_dir, flags });
					} else {
						uint8_t flags = (flags1 & 3) | 4 * ((flags0 & 3) | 4 * (next_walkable | 2 * lut1val));
						game_st.regions.contours[cur_dir].push_back({ { cx, ny, cy}, cur_dir, flags });
					}

					if (!next_walkable) cur_dir = next_dir;
					else {
						int nrel[4][2] = { { 1, -1 }, { 1, 1 }, { -1, 1 }, { -1, -1 } };
						x += nrel[cur_dir][0];
						y += nrel[cur_dir][1];
						if (cur_dir == 0) cur_dir = 3;
						else --cur_dir;
					}
					cx = nx;
					cy = ny;
					lut1val = next_walkable;

					if (cx == start_cx && cy == start_cy) break;
				}

				flip_dir_walkable(x, y, cur_dir);

			}

			for (auto& v : game_st.regions.contours) {
				std::sort(v.begin(), v.end(), [&](auto& a, auto& b) {
					if (a.v[0] != b.v[0]) return a.v[0] < b.v[0];
					return a.v[1] < b.v[1];
				});
			}

		};

		game_st.regions.tile_bounding_box = { {0, 0}, {game_st.map_tile_width, game_st.map_tile_height} };

		set_unwalkable_flags();

		for (size_t y = 0; y != game_st.map_tile_height; ++y) {
			for (size_t x = 0; x != game_st.map_tile_width; ++x) {
				auto& index = game_st.regions.tile_region_index[y * 256 + x];
				auto& t = st.tiles[y * game_st.map_tile_width + x];
				if (~t.flags & tile_t::flag_walkable) index = 0x1ffd;
				else if (t.flags & tile_t::flag_middle) index = 0x1ff9;
				else if (t.flags & tile_t::flag_high) index = 0x1ffa;
				else index = 0x1ffb;
			}
		}

		create_unreachable_bottom_region();

		create_regions();

		create_contours();

	}

	int get_unit_strength(const unit_type_t* unit_type, const weapon_type_t* weapon_type) {
		switch (unit_type->id) {
		case UnitTypes::Terran_Vulture_Spider_Mine:
		case UnitTypes::Protoss_Interceptor:
		case UnitTypes::Protoss_Scarab:
			return 0;
		default:
			break;
		}
		int hp = unit_type->hitpoints.integer_part();
		if (unit_type->has_shield) hp += unit_type->shield_points;
		if (hp == 0) return 0;
		int bullet_count = weapon_type->bullet_count;
		int cd = weapon_type->cooldown;
		int dmg = weapon_type->damage_amount;
		int range = weapon_type->max_range;
		unsigned int a = (range / (unsigned)cd) * bullet_count * dmg;
		unsigned int b = (hp * ((int64_t)(bullet_count*dmg << 11) / cd)) >> 8;
		// This function calculates (int)(sqrt(x)*7.58)
		auto sqrt_x_times_7_58 = [&](unsigned x) {
			if (x == 0) return 0u;
			unsigned value = isqrt(x) * 758 / 100;
			unsigned n = 8;
			while (n > 0) {
				unsigned nv = value + n / 2 + 1;
				unsigned r = (unsigned)(((uint64_t)nv * nv * 10000) / (758 * 758));
				if (r < x) {
					value = nv;
					n -= n / 2 + 1;
				} else n /= 2;
			}
			return value;
		};
		int score = sqrt_x_times_7_58(a + b);
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
	}

	void calculate_unit_strengths() {

		for (int idx = 0; idx < 228; ++idx) {

			const unit_type_t* unit_type = get_unit_type((UnitTypes)idx);
			const unit_type_t* attacking_type = unit_type;
			int air_strength = 0;
			int ground_strength = 0;
			if (attacking_type->id != UnitTypes::Zerg_Larva && !unit_is_egg(attacking_type)) {
				if (unit_is_carrier(attacking_type)) attacking_type = get_unit_type(UnitTypes::Protoss_Interceptor);
				else if (unit_is_reaver(attacking_type)) attacking_type = get_unit_type(UnitTypes::Protoss_Scarab);
				else if (attacking_type->turret_unit_type) attacking_type = attacking_type->turret_unit_type;

				const weapon_type_t* air_weapon = attacking_type->air_weapon;
				if (!air_weapon) air_strength = 1;
				else air_strength = get_unit_strength(unit_type, air_weapon);

				const weapon_type_t* ground_weapon = attacking_type->ground_weapon;
				if (!ground_weapon) ground_strength = 1;
				else ground_strength = get_unit_strength(unit_type, ground_weapon);
			}
			if (air_strength == 1 && ground_strength > air_strength) air_strength = 0;
			if (ground_strength == 1 && air_strength > ground_strength) ground_strength = 0;

			game_st.unit_air_strength[(UnitTypes)idx] = air_strength;
			game_st.unit_ground_strength[(UnitTypes)idx] = ground_strength;

		}

	}

	void generate_sight_values() {
		for (size_t i = 0; i != game_st.sight_values.size(); ++i) {
			auto& v = game_st.sight_values[i];
			v.max_width = 3 + (int)i * 2;
			v.max_height = 3 + (int)i * 2;
			v.min_width = 3;
			v.min_height = 3;
			v.min_mask_size = 0;
			v.ext_masked_count = 0;
		}

		struct base_mask_t {
			sight_values_t::maskdat_node_t* maskdat_node;
			bool masked;
		};
		a_vector<base_mask_t> base_mask;
		for (auto& v : game_st.sight_values) {
			base_mask.clear();
			base_mask.resize(v.max_width*v.max_height);
			auto mask = [&](size_t index) {
				if (index >= base_mask.size()) error("attempt to mask invalid base mask index %d (size %d)", index, base_mask.size());
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
				int max_x2 = half_width;
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
			for (auto& v : base_mask) {
				if (v.masked) ++masked_count;
			}

			v.ext_masked_count = masked_count - v.min_mask_size;
			v.maskdat.clear();
			v.maskdat.resize(masked_count);

			size_t center_index = v.max_height / 2 * v.max_width + v.max_width / 2;
			base_mask[center_index].maskdat_node = &v.maskdat.front();

			auto at = [&](int relative_index) -> base_mask_t& {
				size_t index = center_index + relative_index;
				if (index >= base_mask.size()) error("attempt to access invalid base mask center-relative index %d (size %d)", index, base_mask.size());
				return base_mask[index];
			};

			size_t next_entry_index = 1;

			int cur_x = -1;
			int cur_y = -1;
			int added_count = 1;
			for (int i = 2; added_count < masked_count; i += 2) {
				for (int dir = 0; dir < 4; ++dir) {
					static const std::array<int, 4> direction_x = {1, 0, -1, 0};
					static const std::array<int, 4> direction_y = {0, 1, 0, -1};
					int this_x;
					int this_y;
					auto do_n = [&](int n) {
						for (int i = 0; i < n; ++i) {
							if (at(this_y*v.max_width + this_x).masked) {
								if (this_x || this_y) {
									auto* this_entry = &v.maskdat.at(next_entry_index++);

									auto index = [&](auto* n) {
										if (!n) return (size_t)-1;
										return (size_t)(n - v.maskdat.data());
									};

									int prev_x = this_x;
									int prev_y = this_y;
									if (prev_x > 0) --prev_x;
									else if (prev_x < 0) ++prev_x;
									if (prev_y > 0) --prev_y;
									else if (prev_y < 0) ++prev_y;
									if (std::abs(prev_x) == std::abs(prev_y) || (this_x == 0 && direction_x[dir]) || (this_y == 0 && direction_y[dir])) {
										this_entry->prev = index(at(prev_y * v.max_width + prev_x).maskdat_node);
										this_entry->prev2 = (size_t)-1;
									} else {
										this_entry->prev = index(at(prev_y * v.max_width + prev_x).maskdat_node);
										int prev2_x = prev_x;
										int prev2_y = prev_y;
										if (std::abs(prev2_x) <= std::abs(prev2_y)) {
											if (this_x >= 0) ++prev2_x;
											else --prev2_x;
										} else {
											if (this_y >= 0) ++prev2_y;
											else --prev2_y;
										}
										this_entry->prev2 = index(at(prev2_y * v.max_width + prev2_x).maskdat_node);
									}
									this_entry->relative_tile_index = this_y * (int)game_st.map_tile_width + this_x;
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

	}

	void load_tile_stuff() {

		auto set_mega_tile_flags = [&]() {
			game_st.mega_tile_flags.resize(game_st.vf4.size());
			for (size_t i = 0; i != game_st.mega_tile_flags.size(); ++i) {
				int flags = 0;
				auto& mt = game_st.vf4[i];
				int walkable_count = 0;
				int middle_count = 0;
				int high_count = 0;
				int very_high_count = 0;
				for (size_t y = 0; y < 4; ++y) {
					for (size_t x = 0; x < 4; ++x) {
						if (mt.flags[y * 4 + x] & vf4_entry::flag_walkable) ++walkable_count;
						if (mt.flags[y * 4 + x] & vf4_entry::flag_middle) ++middle_count;
						if (mt.flags[y * 4 + x] & vf4_entry::flag_high) ++high_count;
						if (mt.flags[y * 4 + x] & vf4_entry::flag_very_high) ++very_high_count;
					}
				}
				if (walkable_count > 12) flags |= tile_t::flag_walkable;
				else flags |= tile_t::flag_unwalkable;
				if (walkable_count && walkable_count != 0x10) flags |= tile_t::flag_partially_walkable;
				if (high_count < 12 && middle_count + high_count >= 12) flags |= tile_t::flag_middle;
				if (high_count >= 12) flags |= tile_t::flag_high;
				if (very_high_count) flags |= tile_t::flag_very_high;
				game_st.mega_tile_flags[i] = flags;
			}

		};

		auto& vf4_data = global_st.tileset_vf4.at(game_st.tileset_index);
		data_loading::data_reader_le r(vf4_data.data(), vf4_data.data() + vf4_data.size());
		game_st.vf4.reserve(vf4_data.size() / 32);
		while (r.left()) {
			game_st.vf4.emplace_back();
			auto& e = game_st.vf4.back();
			for (size_t i = 0; i != 16; ++i) {
				e.flags[i] = r.get<uint16_t>();
			}
		}
		auto& cv5_data = global_st.tileset_cv5.at(game_st.tileset_index);
		r = data_loading::data_reader_le(cv5_data.data(), cv5_data.data() + cv5_data.size());
		game_st.cv5.reserve(cv5_data.size() / 52);
		while (r.left()) {
			game_st.cv5.emplace_back();
			auto& e = game_st.cv5.back();
			r.get<uint16_t>();
			e.flags = r.get<uint16_t>();
			r.get<uint16_t>();
			r.get<uint16_t>();
			r.get<uint16_t>();
			r.get<uint16_t>();
			r.get<uint16_t>();
			r.get<uint16_t>();
			r.get<uint16_t>();
			r.get<uint16_t>();
			for (size_t i = 0; i != 16; ++i) {
				e.mega_tile_index[i] = r.get<uint16_t>();
			}
		}

		set_mega_tile_flags();
	}

	unit_t* create_starting_unit(const unit_type_t* unit_type, xy pos, int owner) {
		unit_t* u = create_unit(unit_type, pos, owner);
		if (!u) return nullptr;
		finish_building_unit(u);
		if (place_completed_unit(u)) complete_unit(u);
		return u;
	}

	void create_starting_units(int owner, xy position, race_t race) {

		const unit_type_t* resource_depot_type;
		if (race == race_t::zerg) resource_depot_type = get_unit_type(UnitTypes::Zerg_Hatchery);
		else if (race == race_t::terran) resource_depot_type = get_unit_type(UnitTypes::Terran_Command_Center);
		else resource_depot_type = get_unit_type(UnitTypes::Protoss_Nexus);

		xy resource_depot_pos = (position - resource_depot_type->placement_size / 2) / 32u * 32u + resource_depot_type->placement_size / 2;
		rect placement_rect;
		placement_rect.from = resource_depot_pos - resource_depot_type->placement_size / 2;
		placement_rect.to = placement_rect.from + resource_depot_type->placement_size - xy(1, 1);
		for (unit_t* u : find_units(placement_rect)) {
			u->user_action_flags |= 4;
			kill_unit(u);
		}
		unit_t* resource_depot = create_starting_unit(resource_depot_type, resource_depot_pos, owner);
		if (resource_depot) {
			if (unit_type_spreads_creep(resource_depot_type, true) || ut_requires_creep(resource_depot)) {
				spread_creep_completely(resource_depot, resource_depot->sprite->position);
				spawn_larva(resource_depot);
				spawn_larva(resource_depot);
			}
		}
		if (race == race_t::zerg) {
			xy overlord_pos = position;
			xy add = resource_depot_type->dimensions.from + resource_depot_type->dimensions.to + xy(1, 1);
			if (overlord_pos.x >= int(game_st.map_width / 2)) overlord_pos.x -= add.x;
			else overlord_pos.x += add.x;
			if (overlord_pos.y >= int(game_st.map_height / 2)) overlord_pos.y -= add.y;
			else overlord_pos.y += add.y;
			create_starting_unit(get_unit_type(UnitTypes::Zerg_Overlord), overlord_pos, owner);
		}

		const unit_type_t* worker_type;
		if (race == race_t::zerg) worker_type = get_unit_type(UnitTypes::Zerg_Drone);
		else if (race == race_t::terran) worker_type = get_unit_type(UnitTypes::Terran_SCV);
		else worker_type = get_unit_type(UnitTypes::Protoss_Probe);

		for (size_t i = 0; i != 4; ++i) {
			create_starting_unit(worker_type, position, owner);
		}

	}

	a_string get_map_string(size_t index) const {
		if (index == 0) return "<null string>";
		--index;
		if (index >= game_st.map_strings.size()) return "<invalid string index>";
		const a_string& str = game_st.map_strings[index];
		a_string kn;
		if (korean::korean_locale_to_utf8(str, kn)) return kn;
		return str;
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
			return std::hash<uint32_t>()((uint32_t)v.data[0] | (uint32_t)v.data[1] << 8 | (uint32_t)v.data[2] << 16 | (uint32_t)v.data[3] << 24);
		}
	};

	void load_map_file(a_string filename, std::function<void()> setup_f = {}, bool initial_processing = true) {
		load_map(data_loading::mpq_file<>(std::move(filename)), std::move(setup_f), initial_processing);
	}

	template<typename load_data_file_F>
	void load_map(load_data_file_F&& load_data_file, std::function<void()> setup_f = {}, bool initial_processing = true) {
		a_vector<uint8_t> data;
		load_data_file(data, "staredit/scenario.chk");
		load_map_data(data.data(), data.size(), std::move(setup_f), initial_processing);
	}

	void load_map_data(uint8_t* data, size_t data_size, std::function<void()> setup_f = {}, bool initial_processing = true) {

		using data_loading::data_reader_le;

		a_unordered_map<tag_t, std::function<void(data_reader_le)>, tag_t> tag_funcs;

		auto tagstr = [&](tag_t tag) {
			return a_string(tag.data.data(), 4);
		};

		using tag_list_t = a_vector<std::pair<tag_t, bool>>;
		auto read_chunks = [&](const tag_list_t&tags) {
			data_reader_le r(data, data + data_size);
			a_unordered_map<tag_t, a_vector<data_reader_le>, tag_t> chunks;
			while (r.left()) {
				tag_t tag = r.get<std::array<char, 4>>();
				uint32_t len = r.get<uint32_t>();
				if (len > r.left()) break;
				const uint8_t* chunk_data = r.ptr;
				r.skip(len);
				chunks[tag].emplace_back(chunk_data, r.ptr);
			}
			for (auto& v : tags) {
				tag_t tag = std::get<0>(v);
				auto i = chunks.find(tag);
				if (i == chunks.end()) {
					if (std::get<1>(v)) error("map is missing required chunk '%s'", tagstr(tag));
				} else {
					if (!tag_funcs[tag]) error("tag '%s' is missing a function", tagstr(tag));
					for (auto& v : i->second) {
						tag_funcs[tag](v);
					}
				}
			}
		};

		int version = 0;
		tag_funcs["VER "] = [&](data_reader_le r) {
			version = r.get<uint16_t>();
		};
		tag_funcs["DIM "] = [&](data_reader_le r) {
			game_st.map_tile_width = r.get<uint16_t>();
			game_st.map_tile_height = r.get<uint16_t>();
			game_st.map_width = game_st.map_tile_width * 32;
			game_st.map_height = game_st.map_tile_height * 32;
			game_st.map_walk_width = game_st.map_tile_width * 4;
			game_st.map_walk_height = game_st.map_tile_height * 4;
		};
		tag_funcs["ERA "] = [&](data_reader_le r) {
			game_st.tileset_index = r.get<uint16_t>() % 8;
		};
		tag_funcs["OWNR"] = [&](data_reader_le r) {
			for (size_t i = 0; i != 12; ++i) {
				st.players[i].controller = r.get<int8_t>();
			}
		};
		tag_funcs["SIDE"] = [&](data_reader_le r) {
			for (size_t i = 0; i != 12; ++i) {
				st.players[i].race = (race_t)r.get<int8_t>();
			}
		};
		tag_funcs["STR "] = [&](data_reader_le r) {
			if (r.left() < 2) return;
			auto start = r;
			size_t num = r.get<uint16_t>();
			game_st.map_strings.clear();
			game_st.map_strings.resize(num);
			for (size_t i = 0; i != num; ++i) {
				size_t offset = r.get<uint16_t>();
				auto t = start;
				if (offset < t.left()) {
					t.skip(offset);
					char* b = (char*)t.ptr;
					while (t.get<char>());
					game_st.map_strings[i] = a_string(b, (char*)t.ptr - b - 1);
				} else {
					game_st.map_strings[i] = "<invalid string offset>";
				}
			}
		};
		tag_funcs["SPRP"] = [&](data_reader_le r) {
			game_st.scenario_name = get_map_string(r.get<uint16_t>());
			game_st.scenario_description = get_map_string(r.get<uint16_t>());
		};
		tag_funcs["FORC"] = [&](data_reader_le r) {
			for (size_t i = 0; i != 12; ++i) st.players[i].force = 0;
			for (size_t i = 0; i != 4; ++i) {
				game_st.forces[i].name = "";
				game_st.forces[i].flags = 0;
			}
			if (r.left()) {
				for (size_t i = 0; i != 8; ++i) {
					st.players[i].force = r.get<uint8_t>();
				}
				for (size_t i = 0; i != 4; ++i) {
					game_st.forces[i].name = get_map_string(r.get<uint16_t>());
				}
				for (size_t i = 0; i != 4; ++i) {
					game_st.forces[i].flags = r.get<uint8_t>();
				}
			}
		};
		tag_funcs["VCOD"] = [&](data_reader_le r) {
			(void)r;
			// Starcraft does some verification/checksum stuff here
		};


		tag_funcs["MTXM"] = [&](data_reader_le r) {
			game_st.gfx_tiles.resize(game_st.map_tile_width * game_st.map_tile_height);
			for (size_t i = 0; r.left(); ++i) {
				if (r.left() == 1) {
					game_st.gfx_tiles.at(i).raw_value &= 0xff00;
					game_st.gfx_tiles.at(i).raw_value |= r.get<uint8_t>();
					break;
				}
				if (i >= game_st.gfx_tiles.size()) break;
				game_st.gfx_tiles.at(i) = tile_id(r.get<uint16_t>());
			}
			for (size_t i = 0; i != game_st.gfx_tiles.size(); ++i) {
				tile_id tile_id = game_st.gfx_tiles[i];
				if (tile_id.group_index() >= game_st.cv5.size()) tile_id = {};
				size_t megatile_index = game_st.cv5.at(tile_id.group_index()).mega_tile_index[tile_id.subtile_index()];
				int cv5_flags = game_st.cv5.at(tile_id.group_index()).flags & ~(tile_t::flag_walkable | tile_t::flag_unwalkable | tile_t::flag_very_high | tile_t::flag_middle | tile_t::flag_high | tile_t::flag_partially_walkable);
				st.tiles_mega_tile_index[i] = (uint16_t)megatile_index;
				st.tiles[i].flags = game_st.mega_tile_flags.at(megatile_index) | cv5_flags;
				if (tile_id.has_creep()) {
					st.tiles_mega_tile_index[i] |= 0x8000;
					st.tiles[i].flags |= tile_t::flag_has_creep;
				}
			}

			tiles_flags_and(0, game_st.map_tile_height - 2, 5, 1, ~(tile_t::flag_walkable | tile_t::flag_has_creep | tile_t::flag_partially_walkable));
			tiles_flags_or(0, game_st.map_tile_height - 2, 5, 1, tile_t::flag_unbuildable);
			tiles_flags_and(game_st.map_tile_width - 5, game_st.map_tile_height - 2, 5, 1, ~(tile_t::flag_walkable | tile_t::flag_has_creep | tile_t::flag_partially_walkable));
			tiles_flags_or(game_st.map_tile_width - 5, game_st.map_tile_height - 2, 5, 1, tile_t::flag_unbuildable);

			tiles_flags_and(0, game_st.map_tile_height - 1, game_st.map_tile_width, 1, ~(tile_t::flag_walkable | tile_t::flag_has_creep | tile_t::flag_partially_walkable));
			tiles_flags_or(0, game_st.map_tile_height - 1, game_st.map_tile_width, 1, tile_t::flag_unbuildable);

			regions_create();
		};

		bool use_map_settings = false;

		tag_funcs["THG2"] = [&](data_reader_le r) {
			while (r.left()) {
				int id = r.get<uint16_t>();
				int x = r.get<uint16_t>();
				int y = r.get<uint16_t>();
				int owner = r.get<uint8_t>();
				r.get<uint8_t>();
				r.get<uint8_t>();
				int flags = r.get<uint8_t>();
				if (flags & 0x10) {
					if (id == 0xffff) continue;
					const sprite_type_t* sprite_type = get_sprite_type((SpriteTypes)id);
					create_thingy(sprite_type, {x, y}, owner);
				} else {
					auto unit_type = (UnitTypes)id;
					if (unit_type == UnitTypes::Special_Upper_Level_Door) owner = 11;
					if (unit_type == UnitTypes::Special_Right_Upper_Level_Door) owner = 11;
					if (unit_type == UnitTypes::Special_Pit_Door) owner = 11;
					if (unit_type == UnitTypes::Special_Right_Pit_Door) owner = 11;
					if (use_map_settings || owner == 11) {
						create_initial_unit(get_unit_type(unit_type), xy(x, y), owner);
						if (flags & 0x80) error("disable thingy unit");
					}
				}
			}
		};
		tag_funcs["MASK"] = [&](data_reader_le r) {
			auto mask = r.get_vec<uint8_t>(std::min(game_st.map_tile_width*game_st.map_tile_height, r.left()));
			for (size_t i = 0; i != mask.size(); ++i) {
				st.tiles[i].visible = mask[i];
				st.tiles[i].explored = mask[i];
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
			for (size_t i = 0; i != 228; ++i) {
				if (uses_default_settings[i]) continue;
				unit_type_t* unit_type = get_unit_type((UnitTypes)i);
				unit_type->hitpoints = fp8::from_raw(hp[i]);
				unit_type->shield_points = shield_points[i];
				unit_type->armor = armor[i];
				unit_type->build_time = build_time[i];
				unit_type->mineral_cost = mineral_cost[i];
				unit_type->gas_cost = gas_cost[i];
				unit_type->unit_map_string_index = string_index[i];
				const unit_type_t* attacking_type = unit_type->turret_unit_type ? (const unit_type_t*)unit_type->turret_unit_type : unit_type;
				weapon_type_t* ground_weapon = (weapon_type_t*)(const weapon_type_t*)attacking_type->ground_weapon;
				weapon_type_t* air_weapon = (weapon_type_t*)(const weapon_type_t*)attacking_type->air_weapon;
				if (ground_weapon && (size_t)ground_weapon->id < weapon_damage.size()) {
					ground_weapon->damage_amount = weapon_damage.at((size_t)ground_weapon->id);
					ground_weapon->damage_bonus =  weapon_bonus_damage.at((size_t)ground_weapon->id);
				}
				if (air_weapon && (size_t)air_weapon->id < weapon_damage.size()) {
					air_weapon->damage_amount = weapon_damage.at((size_t)air_weapon->id);
					air_weapon->damage_bonus = weapon_bonus_damage.at((size_t)air_weapon->id);
				}
			}
		};

		auto upgrades = [&](data_reader_le r, bool broodwar) {
			auto uses_default_settings = r.get_vec<uint8_t>(broodwar ? 61 : 46);
			if (broodwar) r.get<uint8_t>();
			auto mineral_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto mineral_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto gas_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto gas_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto time_cost = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			auto time_cost_factor = r.get_vec<uint16_t>(broodwar ? 61 : 46);
			for (size_t i = 0; i != (broodwar ? 61 : 46); ++i) {
				if (uses_default_settings[i]) continue;
				upgrade_type_t* upg = get_upgrade_type((UpgradeTypes)i);
				upg->mineral_cost_base = mineral_cost[i];
				upg->mineral_cost_factor = mineral_cost_factor[i];
				upg->gas_cost_base = gas_cost[i];
				upg->gas_cost_factor = gas_cost_factor[i];
				upg->time_cost_base = time_cost[i];
				upg->time_cost_factor = time_cost_factor[i];
			}
		};

		auto techdata = [&](data_reader_le r, bool broodwar) {
			auto uses_default_settings = r.get_vec<uint8_t>(broodwar ? 44 : 24);
			auto mineral_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			auto gas_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			auto build_time = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			auto energy_cost = r.get_vec<uint16_t>(broodwar ? 44 : 24);
			for (size_t i = 0; i != (broodwar ? 44 : 24); ++i) {
				if (uses_default_settings[i]) continue;
				tech_type_t* tech = get_tech_type((TechTypes)i);
				tech->mineral_cost = mineral_cost[i];
				tech->gas_cost = gas_cost[i];
				tech->research_time = build_time[i];
				tech->energy_cost = energy_cost[i];
			}
		};

		auto upgrade_restrictions = [&](data_reader_le r, bool broodwar) {
			size_t count = broodwar ? 61 : 46;
			auto player_max_level = r.get_vec<uint8_t>(12 * count);
			auto player_cur_level = r.get_vec<uint8_t>(12 * count);
			auto global_max_level = r.get_vec<uint8_t>(count);
			auto global_cur_level = r.get_vec<uint8_t>(count);
			auto player_uses_global_default = r.get_vec<uint8_t>(12 * count);
			for (size_t player = 0; player != 12; ++player) {
				for (size_t upgrade = 0; upgrade != count; ++upgrade) {
					game_st.max_upgrade_levels[player][(UpgradeTypes)upgrade] = !!player_uses_global_default[player*count + upgrade] ? global_max_level[upgrade] : player_max_level[player*count + upgrade];
					st.upgrade_levels[player][(UpgradeTypes)upgrade] = !!player_uses_global_default[player*count + upgrade] ? global_cur_level[upgrade] : player_cur_level[player*count + upgrade];
				}
			}
		};
		auto tech_restrictions = [&](data_reader_le r, bool broodwar) {
			size_t count = broodwar ? 44 : 24;
			auto player_available = r.get_vec<uint8_t>(12 * count);
			auto player_researched = r.get_vec<uint8_t>(12 * count);
			auto global_available = r.get_vec<uint8_t>(count);
			auto global_researched = r.get_vec<uint8_t>(count);
			auto player_uses_global_default = r.get_vec<uint8_t>(12 * count);
			for (size_t player = 0; player != 12; ++player) {
				for (size_t tech = 0; tech != count; ++tech) {
					game_st.tech_available[player][(TechTypes)tech] = !!(!!player_uses_global_default[player*count + tech] ? global_available[tech] : player_available[player*count + tech]);
					st.tech_researched[player][(TechTypes)tech] = !!(!!player_uses_global_default[player*count + tech] ? global_researched[tech] : player_researched[player*count + tech]);
				}
			}
		};

		tag_funcs["UNIS"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			units(r, false);
		};
		tag_funcs["UPGS"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			upgrades(r, false);
		};
		tag_funcs["TECS"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			techdata(r, false);
		};
		tag_funcs["PUNI"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			auto player_available = r.get_vec<std::array<uint8_t, 228>>(12);
			auto global_available = r.get_vec<uint8_t>(228);
			auto player_uses_global_default = r.get_vec<std::array<uint8_t, 228>>(12);
			for (size_t player = 0; player != 12; ++player) {
				for (size_t unit = 0; unit != 228; ++unit) {
					game_st.unit_type_allowed[player][(UnitTypes)unit] = !!(!!player_uses_global_default[player][unit] ? global_available[unit] : player_available[player][unit]);
				}
			}
		};
		tag_funcs["UPGR"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			upgrade_restrictions(r, false);
		};
		tag_funcs["PTEC"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			tech_restrictions(r, false);
		};

		tag_funcs["UNIx"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			units(r, true);
		};
		tag_funcs["UPGx"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			upgrades(r, true);
		};
		tag_funcs["TECx"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			techdata(r, true);
		};
		tag_funcs["PUPx"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			upgrade_restrictions(r, true);
		};
		tag_funcs["PTEx"] = [&](data_reader_le r) {
			if (!use_map_settings) error("wrong game mode");
			tech_restrictions(r, true);
		};

		tag_funcs["UNIT"] = [&](data_reader_le r) {
			while (r.left()) {

				int id = r.get<uint32_t>();
				int x = r.get<uint16_t>();
				int y = r.get<uint16_t>();
				UnitTypes unit_type_id = (UnitTypes)r.get<uint16_t>();
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
				bool is_invalid = (size_t)unit_type_id == 0xffff;
				if (is_invalid) {
					unit_type_id = (UnitTypes)0;
					owner = 8;
				}

				(void)id; (void)link; (void)valid_flags; (void)units_in_hangar; (void)flags; (void)related_unit_id;

				if ((size_t)unit_type_id >= 228) error("UNIT: invalid unit type %d", (int)unit_type_id);
				if ((size_t)owner >= 12) error("UNIT: invalid owner %d", owner);

				const unit_type_t* unit_type = get_unit_type(unit_type_id);

				if (unit_type->id == UnitTypes::Special_Start_Location) {
					game_st.start_locations[owner] = { x, y };
					// todo: some callback to set initial screen position?
					continue;
				}
				auto should_create_units_for_this_player = [&]() {
					if (setup_info.create_no_units) return false;
					if (owner >= 8) return true;
					int controller = st.players[owner].controller;
					if (controller == player_t::controller_computer_game) return true;
					if (controller == player_t::controller_occupied) return true;
					if (controller == player_t::controller_rescue_passive) return true;
					if (controller == player_t::controller_unused_rescue_active) return true;
					if (controller == player_t::controller_neutral) return true;
					return false;
				};
				auto is_neutral_unit = [&]() {
					if (owner == 11) return true;
					if (unit_is_mineral_field(unit_type)) return true;
					if (unit_type->id == UnitTypes::Resource_Vespene_Geyser) return true;
					if (unit_is_critter(unit_type)) return true;
					return false;
				};
				if (!should_create_units_for_this_player()) continue;
				if (!use_map_settings && !is_neutral_unit()) continue;
				if (use_map_settings) {
					if (owner < 8 && setup_info.create_melee_units_for_player[owner] && ~unit_type->group_flags & GroupFlags::Neutral) continue;
				}

				if (is_invalid) {
					lcg_rand(14);
					if (is_in_map_bounds(xy(x, y))) error("attempt to create invalid initial unit in valid map bounds");
					continue;
				}

				unit_t* u = create_initial_unit(unit_type, {x, y}, owner);

				if (!u) continue;

				if (valid_properties & 0x2) {
					using tmp_t = fixed_point<32, 8, true>;
					tmp_t tmp = tmp_t::extend(u->unit_type->hitpoints);
					tmp = std::max(tmp * hp_percent / 100, tmp_t::from_raw(1));
					set_unit_hp(u, fp8::truncate(tmp));
				}
				if (valid_properties & 0x4) set_unit_shield_points(u, fp8::integer(u->unit_type->shield_points * shield_percent / 100));
				if (valid_properties & 0x8) set_unit_energy(u, unit_max_energy(u) * energy_percent / 100);
				if (valid_properties & 0x10) set_unit_resources(u, resources);
				// todo: more stuff...

				if (valid_flags & 8 && flags & 8) {
					make_unit_hallucination(u);
				}

				if (unit_is(u, UnitTypes::Zerg_Broodling)) {
					set_remove_timer(u);
				}

			}

			for (unit_t* u : ptr(st.visible_units)) {
				update_unit_sprite(u);
			}
			st.update_psionic_matrix = true;
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
				(void)valid_flags;
				(void)valid_properties;
				(void)owner;
				(void)hp_percent;
				(void)shield_percent;
				(void)energy_percent;
				(void)resources;
				(void)units_in_hangar;
				(void)flags;
			}
		};

		tag_funcs["MRGN"] = [&](data_reader_le r) {
			// 64 or 256 entries
			while (r.left()) {
				int left = r.get<int32_t>();
				int top = r.get<int32_t>();
				int right = r.get<int32_t>();
				int bottom = r.get<int32_t>();
				a_string name = get_map_string(r.get<uint16_t>());
				(void)name;
				int elevation_flags = r.get<uint16_t>();
				st.locations.emplace_back();
				auto& loc = st.locations.back();
				loc.area = { {left, top}, {right, bottom} };
				loc.elevation_flags = elevation_flags;
			}
		};

		tag_funcs["TRIG"] = [&](data_reader_le r) {
			while (r.left()) {
				game_st.triggers.emplace_back();
				auto& t = game_st.triggers.back();
				for (size_t i = 0; i != 16; ++i) {
					auto& c = t.conditions[i];
					c.location = r.get<uint32_t>();
					c.group = r.get<uint32_t>();
					c.count_n = r.get<uint32_t>();
					c.unit_id = r.get<uint16_t>();
					c.num_n = r.get<uint8_t>();
					c.type = r.get<uint8_t>();
					c.extra_n = r.get<uint8_t>();
					c.flags = r.get<uint8_t>();
					c.unk = r.get<uint16_t>();
				}
				for (size_t i = 0; i != 64; ++i) {
					auto& a = t.actions[i];
					a.location = r.get<uint32_t>();
					a.string_index = r.get<uint32_t>();
					a.sound_index = r.get<uint32_t>();
					a.time_n = r.get<uint32_t>();
					a.group_n = r.get<uint32_t>();
					a.group2_n = r.get<uint32_t>();
					a.extra_n = r.get<uint16_t>();
					a.type = r.get<uint8_t>();
					a.num_n = r.get<uint8_t>();
					a.flags = r.get<uint8_t>();
					a.unk = r.get<uint16_t>();
					r.get<uint8_t>(); // padding
				}
				t.execution_flags = r.get<uint32_t>();

				bool enabled_for_any = false;
				for (size_t i = 0; i != 28; ++i) {
					t.enabled[i] = r.get<uint8_t>() != 0;
					if (!enabled_for_any && t.enabled[i]) enabled_for_any = true;
				}
				if (!enabled_for_any) game_st.triggers.pop_back();
			}
			for (auto& t : game_st.triggers) {
				for (int i = 0; i != 8; ++i) {
					if (st.players[i].controller != player_t::controller_occupied) continue;
					if (!t.enabled[i] && !t.enabled.at(17 + st.players[i].force) && !t.enabled[17]) continue;
					st.running_triggers[i].emplace_back();
					auto& rt = st.running_triggers[i].back();
					rt.t = &t;
					rt.flags = t.execution_flags;
				}
			}
		};

		tag_funcs["COLR"] = [&](data_reader_le r) {
			// todo
			for (size_t i = 0; i != 8; ++i) {
				st.players[i].color = r.get<uint8_t>();
			}
		};

		st.players = {};

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

		for (size_t i = 0; i != 12; ++i) {
			st.shared_vision[i] = 1 << i;
			st.players[i].color = (int)i;
			if (st.players[i].controller == player_t::controller_rescue_passive || st.players[i].controller == player_t::controller_neutral) {
				for (int i2 = 0; i2 < 12; ++i2) {
					st.alliances[i][i2] = 1;
					st.alliances[i2][i] = 1;
				}
			}
			if (!use_map_settings) {
				if (st.players[i].controller == player_t::controller_open || st.players[i].controller == player_t::controller_computer) {
					st.players[i].race = (race_t)5;
				}
			}
		}

		if (setup_f) {
			setup_f();
		} else {
			for (size_t i = 0; i != 12; ++i) {
				if (st.players[i].controller == player_t::controller_open) st.players[i].controller = player_t::controller_occupied;
				if (st.players[i].controller == player_t::controller_computer) st.players[i].controller = player_t::controller_computer_game;
			}
		}

		use_map_settings = setup_info.victory_condition == 0 && setup_info.tournament_mode == 0 && setup_info.starting_units == 0;

		if (version == 59 || version == 63) {
			if (use_map_settings) {
				read_chunks({
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
				});
			} else {
				read_chunks({
					{"STR ", true},
					{"MTXM", true},
					{"THG2", true},
					{"UNIT", true}
				});
			}
		} else if (version == 205) {
			if (use_map_settings) {
				read_chunks({
					{"STR ", true},
					{"MTXM", true},
					{"THG2", true},
					{"MASK", true},
					{"UNIx", true},
					{"UPGx", true},
					{"TECx", true},
					{"PUNI", true},
					{"PUPx", true},
					{"PTEx", true},
					{"UNIT", true},
					{"UPRP", true},
					{"MRGN", true},
					{"TRIG", true},
					{"COLR", true},
				});
			} else {
				read_chunks({
					{"STR ", true},
					{"MTXM", true},
					{"THG2", true},
					{"UNIT", true},
					{"COLR", true}
				});
			}
		} else error("unsupported map version %d", version);

		if (!use_map_settings) {
			if (setup_info.victory_condition == 1) {
				data_reader_le r(global_st.melee_trg.data(), global_st.melee_trg.data() + global_st.melee_trg.size());
				tag_funcs["TRIG"](r);
			}
		}

		for (auto& v : st.players) v.initially_active = false;
		for (int p : active_players()) {
			st.players[p].initially_active = true;
		}

		for (size_t i = 8; i;) {
			--i;
			int controller = st.players[i].controller;
			auto race = st.players[i].race;
			if (controller != player_t::controller_occupied && controller != player_t::controller_computer_game) continue;
			if ((!use_map_settings || setup_info.create_melee_units_for_player[i]) && !setup_info.create_no_units) {
				create_starting_units((int)i, game_st.start_locations[i], race);
			}
		}
		if (setup_info.resource_type == 1) {
			for (size_t i = 0; i != 12; ++i) {
				st.current_minerals[i] = setup_info.starting_minerals;
				st.total_minerals_gathered[i] = setup_info.starting_minerals;
			}
		}
		for (size_t i = 0; i != 12; ++i) {
			if (st.players[i].controller != player_t::controller_occupied) continue;
			if (st.players[i].force >= 1 && st.players[i].force <= 4) {
				if (game_st.forces[st.players[i].force - 1].flags & 8) {
					for (size_t i2 = 0; i2 != 12; ++i2) {
						if (st.players[i2].controller != player_t::controller_occupied) continue;
						if (st.players[i2].force == st.players[i].force) st.shared_vision[i] |= 1 << i2;
					}
				}
			}
		}

		if (initial_processing) {
			process_frame();
			process_frame();
		}

	}
};

template<typename reader_T>
grp_t read_grp(reader_T&& r) {
	auto base_r = r;
	grp_t grp;
	size_t frame_count = r.template get<uint16_t>();
	grp.width = r.template get<uint16_t>();
	grp.height = r.template get<uint16_t>();
	grp.frames.resize(frame_count);
	for (size_t i = 0; i != frame_count; ++i) {
		auto& f = grp.frames[i];
		f.offset.x = r.template get<uint8_t>();
		f.offset.y = r.template get<uint8_t>();
		f.size.x = r.template get<uint8_t>();
		f.size.y = r.template get<uint8_t>();
		size_t file_offset = r.template get<uint32_t>();
		auto line_offset_r = base_r;
		line_offset_r.skip(file_offset);
		f.line_data_offset.reserve(f.size.y);
		for (size_t y = 0; y != f.size.y; ++y) {
			auto line_r = base_r;
			line_r.skip(file_offset + line_offset_r.template get<uint16_t>());
			f.line_data_offset.push_back(f.data_container.size());
			for (size_t x = 0; x != f.size.x;) {
				auto v = line_r.template get<uint8_t>();
				if (v & 0x80) {
					v &= 0x7f;
					if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
					f.data_container.push_back(0x80 | v);
					x += v;
				} else if (v & 0x40) {
					v &= 0x3f;
					if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
					f.data_container.push_back(0x40 | v);
					f.data_container.push_back(line_r.template get<uint8_t>());
					x += v;
				} else {
					if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
					f.data_container.push_back(v);
					for (size_t i = 0; i != v; ++i) {
						f.data_container.push_back(line_r.template get<uint8_t>());
					}
					x += v;
				}
			}
		}
		f.data_container.shrink_to_fit();
	}
	return grp;
}

struct string_table_data {
	a_vector<uint8_t> data;
	a_string operator[](size_t index) const {
		data_loading::data_reader_le r(data.data(), data.data() + data.size());
		r.seek(2 + (index - 1) * 2);
		size_t offset = r.get<uint16_t>();
		r.seek(offset);
		a_string str;
		while (char c = r.get<char>()) str += c;
		return str;
	}
	a_string at(size_t index) const {
		return (*this)[index];
	}
};

template<typename load_data_file_F>
void global_init(global_state& st, load_data_file_F&& load_data_file) {

	auto get_sprite_type = [&](SpriteTypes id) {
		if ((size_t)id >= 517) error("invalid sprite id %d", (size_t)id);
		return &st.sprite_types.vec[(size_t)id];
	};
	auto get_image_type = [&](ImageTypes id) {
		if ((size_t)id >= 999) error("invalid image id %d", (size_t)id);
		return &st.image_types.vec[(size_t)id];
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
		ins_data[opc_sprol] = "2s1s1";
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
		load_data_file(data, "scripts/iscript.bin");
		data_reader_le base_r(data.data(), data.data() + data.size());
		auto r = base_r;
		size_t id_list_offset = r.get<uint32_t>();
		r.seek(id_list_offset);
		while (r.left()) {
			int id = r.get<int16_t>();
			if (id == -1) break;
			size_t script_address = r.get<uint16_t>();
			auto script_r = base_r;
			script_r.skip(script_address);
			auto signature = script_r.get<std::array<char, 4>>();
			(void)signature;

			a_unordered_map<size_t, size_t> decode_map;

			auto decode_at = [&](size_t initial_address) {
				a_circular_vector<std::tuple<size_t, size_t>> branches;
				std::function<size_t(size_t)> decode = [&](size_t initial_address) {
					if (!initial_address) error("iscript load: attempt to decode instruction at null address");
					auto in = decode_map.emplace(initial_address, 0);
					if (!in.second) {
						return in.first->second;
					}
					size_t initial_pc = program_data.size();
					in.first->second = initial_pc;
					auto r = base_r;
					r.skip(initial_address);
					bool done = false;
					while (!done) {
						size_t pc = program_data.size();
						size_t cur_address = r.ptr - base_r.ptr;
						if ((size_t)(int)pc != pc || (size_t)(int)cur_address != cur_address) error("iscript too big");
						if (cur_address != initial_address) {
							auto in = decode_map.emplace(cur_address, pc);
							if (!in.second) {
								program_data.push_back(opc_goto + 0x808091);
								program_data.push_back((int)in.first->second);
								break;
							}
						}
						int opcode = r.get<uint8_t>();
						if ((size_t)opcode >= ins_data.size()) error("iscript load: at 0x%04x: invalid instruction %d", cur_address, opcode);
						program_data.push_back(opcode + 0x808091);
						const char* c = ins_data[opcode];
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
									program_data.push_back((int)jump_pc_it->second);
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
					size_t pc = decode(std::get<0>(v));
					if ((size_t)(int)pc != pc) error("iscript load: 0x%x does not fit in an int", pc);
					program_data[std::get<1>(v)] = (int)pc;
				}
				return initial_pc;
			};

			auto& anim_funcs = animation_pc[id];

			size_t highest_animation = script_r.get<uint32_t>();
			size_t animations = (highest_animation + 1 + 1)&-2;
			for (size_t i = 0; i != animations; ++i) {
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
		for (auto& v : animation_pc) {
			auto& s = st.iscript.scripts[v.first];
			s.id = v.first;
			s.animation_pc = std::move(v.second);
		}
	};

	auto load_images = [&]() {

		using data_loading::data_reader_le;

		a_vector<uint8_t> data;
		load_data_file(data, "arr/images.tbl");
		data_reader_le base_r(data.data(), data.data() + data.size());

		auto r = base_r;
		size_t file_count = r.get<uint16_t>();
		(void)file_count;

		a_vector<grp_t> grps;
		a_vector<a_vector<a_vector<xy>>> lo_offsets;

		auto load_grp = [&](data_reader_le r) {
			size_t index = grps.size();
			grps.push_back(read_grp(r));
			return index;
		};
		auto load_offsets = [&](data_reader_le r) {
			auto base_r = r;
			lo_offsets.emplace_back();
			auto& offs = lo_offsets.back();

			size_t frame_count = r.get<uint32_t>();
			size_t offset_count = r.get<uint32_t>();
			for (size_t f = 0; f < frame_count; ++f) {
				size_t file_offset = r.get<uint32_t>();
				auto r2 = base_r;
				r2.skip(file_offset);
				offs.emplace_back();
				auto& vec = offs.back();
				vec.resize(offset_count);
				for (size_t i = 0; i != offset_count; ++i) {
					int x = r2.get<int8_t>();
					int y = r2.get<int8_t>();
					vec[i] = {x, y};
				}
			}

			return lo_offsets.size() - 1;
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
			load_data_file(data, format("unit/%s", fn));
			data_reader_le data_r(data.data(), data.data() + data.size());
			size_t loaded_index = f(data_r);
			in.first->second = loaded_index;
			return loaded_index;
		};

		a_vector<size_t> image_grp_index;
		std::array<a_vector<size_t>, 6> lo_indices;

		grps.emplace_back(); // null/invalid entry
		lo_offsets.emplace_back();

		for (size_t i = 0; i != 999; ++i) {
			const image_type_t* image_type = get_image_type((ImageTypes)i);
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
		for (size_t i = 0; i != image_grp_index.size(); ++i) {
			st.image_grp[i] = &st.grps.at(image_grp_index[i]);
		}
		st.lo_offsets = std::move(lo_offsets);
		st.image_lo_offsets.resize(999);
		for (size_t i = 0; i != 6; ++i) {
			for (size_t i2 = 0; i2 != 999; ++i2) {
				st.image_lo_offsets.at(i2).at(i) = &st.lo_offsets.at(lo_indices[i].at(i2));
			}
		}

	};

	load_data_file(st.units_dat, "arr/units.dat");
	load_data_file(st.weapons_dat, "arr/weapons.dat");
	load_data_file(st.upgrades_dat, "arr/upgrades.dat");
	load_data_file(st.techdata_dat, "arr/techdata.dat");

	load_data_file(st.melee_trg, "triggers/Melee.trg");

	a_vector<uint8_t> buf;
	load_data_file(buf, "arr/flingy.dat");
	st.flingy_types = data_loading::load_flingy_dat(buf);
	load_data_file(buf, "arr/sprites.dat");
	st.sprite_types = data_loading::load_sprites_dat(buf);
	load_data_file(buf, "arr/images.dat");
	st.image_types = data_loading::load_images_dat(buf);
	load_data_file(buf, "arr/orders.dat");
	st.order_types = data_loading::load_orders_dat(buf);

	auto fixup_sprite_type = [&](auto& ptr) {
		SpriteTypes index{ptr};
		if (index == SpriteTypes::None) ptr = nullptr;
		else ptr = get_sprite_type(index);
	};
	auto fixup_image_type = [&](auto& ptr) {
		ImageTypes index{ptr};
		if (index == ImageTypes::None) ptr = nullptr;
		else ptr = get_image_type(index);
	};

	for (auto& v : st.flingy_types.vec) {
		fixup_sprite_type(v.sprite);
	}
	for (auto& v : st.sprite_types.vec) {
		fixup_image_type(v.image);
	}

	load_iscript_bin();
	load_images();

	std::array<const char*, 8> tileset_names = {
		"badlands", "platform", "install", "AshWorld", "Jungle", "Desert", "Ice", "Twilight"
	};

	for (size_t i = 0; i != 8; ++i) {
		load_data_file(st.tileset_vf4[i], format("Tileset/%s.vf4", tileset_names.at(i)));
		load_data_file(st.tileset_cv5[i], format("Tileset/%s.cv5", tileset_names.at(i)));
	}

}

struct game_player {
private:
	std::unique_ptr<global_state> uptr_global_st;
	std::unique_ptr<game_state> uptr_game_st;
	std::unique_ptr<state> uptr_st;
	optional<state_functions> opt_funcs;
public:
	game_player() = default;

	template<typename T>
	explicit game_player(T&& init_arg) {
		init(std::forward<T>(init_arg));
	}
	void init(const char* data_path) {
		init(data_loading::data_files_directory(data_path));
	}
	void init(a_string data_path) {
		init(data_loading::data_files_directory(std::move(data_path)));
	}
	template<typename load_data_file_F>
	void init(load_data_file_F&& load_data_file) {
		uptr_global_st = std::make_unique<global_state>();
		uptr_game_st = std::make_unique<game_state>();
		uptr_st = std::make_unique<state>();
		state& st = *uptr_st;
		st.global = uptr_global_st.get();
		st.game = uptr_game_st.get();
		global_init(*uptr_global_st, std::forward<load_data_file_F>(load_data_file));
		set_st(st);
	}
	void load_map_file(const a_string& filename, bool initial_processing = true) {
		if (!opt_funcs) error("game_player: not initialized");
		game_load_functions game_load_funcs(st());
		game_load_funcs.load_map_file(std::move(filename), {}, initial_processing);
	}
	void next_frame() {
		if (!opt_funcs) error("game_player: not initialized");
		funcs().next_frame();
	}
	void set_st(state& st) {
		opt_funcs.emplace(st);
	}
	state_functions& funcs() {
		return *opt_funcs;
	}
	const state_functions& funcs() const {
		return *opt_funcs;
	}
	state& st() {
		return funcs().st;
	}
	state& st() const {
		return funcs().st;
	}
};

}

#endif
