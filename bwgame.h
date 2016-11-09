
#include "util.h"
#include "data_types.h"
#include "game_types.h"
#include "data_loading.h"
#include "bwenums.h"

#include <algorithm>
#include <utility>
#include <cstdlib>
#include <cmath>

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

// Broodwar linked lists insert new elements between the first and second entry.
template<typename cont_T, typename T>
static void bw_insert_list(cont_T& cont, T& v) {
	if (cont.empty()) cont.push_front(v);
	else cont.insert(++cont.begin(), v);
}

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

	std::array<xy_fp8, 256> direction_table;
	std::array<direction_t, 256> repulse_direction_table;

	a_vector<uint8_t> units_dat;
	a_vector<uint8_t> weapons_dat;
	a_vector<uint8_t> upgrades_dat;
	a_vector<uint8_t> techdata_dat;

	std::array<a_vector<uint8_t>, 8> tileset_vf4;
	std::array<a_vector<uint8_t>, 8> tileset_cv5;
};

struct game_state {

	game_state() = default;
	game_state(game_state&) = delete;
	game_state(game_state&&) = default;
	game_state& operator=(game_state&) = delete;
	game_state& operator=(game_state&&) = default;

	size_t map_tile_width;
	size_t map_tile_height;
	size_t map_walk_width;
	size_t map_walk_height;
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

	size_t repulse_field_width;
	size_t repulse_field_height;

	regions_t regions;
};

struct state_base_copyable {

	const global_state* global;
	game_state* game;

	int update_tiles_countdown;

	std::array<int, 12> selection_circle_color;

	int order_timer_counter;
	int secondary_order_timer_counter;
	int current_frame;

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

	std::array<std::array<int, 12>, 12> alliances;

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

	std::array<uint32_t, 12> shared_vision;

	a_vector<tile_id> gfx_creep_tiles;
	a_vector<tile_t> tiles;
	a_vector<uint16_t> tiles_mega_tile_index;

	std::array<int, 0x100> random_counts;
	int total_random_counts;
	uint32_t lcg_rand_state;

	int last_net_error;

	rect viewport;

	size_t allocated_order_count;
	size_t active_bullets_size;

	a_vector<uint8_t> repulse_field;

	const unit_t* prev_bullet_source_unit;
	bool prev_bullet_heading_offset_clockwise;

	std::array<int, 12> current_minerals;
	std::array<int, 12> current_gas;
	std::array<int, 12> total_minerals_gathered;
	std::array<int, 12> total_gas_gathered;
};

struct state_base_non_copyable {

	state_base_non_copyable() = default;
	state_base_non_copyable(state_base_non_copyable&) = delete;
	state_base_non_copyable(state_base_non_copyable&&) = default;
	state_base_non_copyable& operator=(state_base_non_copyable&) = delete;
	state_base_non_copyable& operator=(state_base_non_copyable&&) = default;

	intrusive_list<unit_t, default_link_f> visible_units;
	intrusive_list<unit_t, default_link_f> hidden_units;
	intrusive_list<unit_t, default_link_f> scanner_sweep_units;
	intrusive_list<unit_t, default_link_f> dead_units;
	intrusive_list<unit_t, default_link_f> free_units;

	intrusive_list<bullet_t, default_link_f> active_bullets;

	a_vector<unit_t> units = a_vector<unit_t>(1700);

	intrusive_list<bullet_t, default_link_f> free_bullets;
	a_list<bullet_t> bullets;

	std::array<intrusive_list<unit_t, intrusive_list_member_link<unit_t, &unit_t::player_units_link>>, 12> player_units;

	a_vector<intrusive_list<sprite_t, default_link_f>> sprites_on_tile_line;
	intrusive_list<sprite_t, default_link_f> free_sprites;
	a_vector<sprite_t> sprites = a_vector<sprite_t>(2500);

	intrusive_list<image_t, default_link_f> free_images;
	a_vector<image_t> images = a_vector<image_t>(5000);

	intrusive_list<order_t, default_link_f> free_orders;
	a_vector<order_t> orders = a_vector<order_t>(2000);

	intrusive_list<path_t, default_link_f> free_paths;
	a_list<path_t> paths;

	struct unit_finder_entry {
		unit_t* u;
		int value;
	};
	a_vector<unit_finder_entry> unit_finder_x;
	a_vector<unit_finder_entry> unit_finder_y;
};

struct state : state_base_copyable, state_base_non_copyable {
};

struct state_functions {

	state& st;
	const global_state& global_st = *st.global;
	const game_state& game_st = *st.game;

	explicit state_functions(state& st) : st(st) {}

	bool allow_random = false;
	bool update_tiles = false;
	flingy_t* iscript_flingy = nullptr;
	bullet_t* iscript_bullet = nullptr;
	unit_t* iscript_unit = nullptr;
	mutable bool unit_finder_search_active = false;

	const order_type_t* get_order_type(int id) const {
		if ((size_t)id >= 189) xcept("invalid order id %d", id);
		return &global_st.order_types.vec[id];
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

	bool ut_flag(const unit_t* u, unit_type_t::flags_t flag) const {
		return (u->unit_type->flags & flag) != 0;
	}
	bool u_status_flag(const unit_t* u, unit_t::status_flags_t flag) const {
		return (u->status_flags & flag) != 0;
	}
	bool us_flag(const unit_t* u, sprite_t::flags_t flag) const {
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
	bool u_hidden(const unit_t* u) const {
		return u_status_flag(u, unit_t::status_flag_hidden);
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

	bool ut_turret(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_turret);
	}
	bool ut_worker(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_worker);
	}
	bool ut_hero(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_hero);
	}
	bool ut_building(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_building);
	}
	bool ut_flyer(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_flyer);
	}
	bool ut_can_turn(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_can_turn);
	}
	bool ut_can_move(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_can_move);
	}
	bool ut_invincible(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_invincible);
	}
	bool ut_two_units_in_one_egg(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_two_units_in_one_egg);
	}
	bool ut_regens_hp(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_regens_hp);
	}
	bool ut_flying_building(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_flying_building);
	}
	bool ut_has_energy(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_has_energy);
	}
	bool ut_resource_depot(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_resource_depot);
	}
	bool ut_resource(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_resource);
	}
	bool ut_initially_cloaked(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_initially_cloaked);
	}
	bool ut_detector(const unit_t* u) const {
		return ut_flag(u, unit_type_t::flag_detector);
	}

	bool us_selected(const unit_t* u) const {
		return us_flag(u, sprite_t::flag_selected);
	}
	bool us_hidden(const unit_t* u) const {
		return us_flag(u, sprite_t::flag_hidden);
	}

	const unit_type_t* get_unit_type(int id) const {
		if ((size_t)id >= 228) xcept("invalid unit id %d", id);
		return &game_st.unit_types.vec[id];
	}
	const image_type_t* get_image_type(int id) const {
		if ((size_t)id >= 999) xcept("invalid image id %d", id);
		return &global_st.image_types.vec[id];
	}
	const weapon_type_t* get_weapon_type(int id) const {
		if ((size_t)id >= 130) xcept("invalid weapon id %d", id);
		return &game_st.weapon_types.vec[id];
	}

	unit_t* get_unit(unit_id id) const {
		size_t idx = id.index();
		if (!idx) return nullptr;
		size_t actual_index = idx - 1;
		if (actual_index >= st.units.size()) xcept("attempt to dereference invalid unit id %d (actual index %d)", idx, actual_index);
		unit_t* u = &st.units[actual_index];
		if (u->unit_id_generation != id.generation()) return nullptr;
		if (unit_dead(u)) return nullptr;
		return u;
	}

	unit_id get_unit_id(const unit_t* u) const {
		if (!u) return unit_id{};
		return unit_id(u - st.units.data() + 1, u->unit_id_generation);
	}

	bool is_in_map_bounds(const unit_type_t* unit_type, xy pos) const {
		if (pos.x - unit_type->dimensions.from.x < 0) return false;
		if (pos.y - unit_type->dimensions.from.y < 0) return false;
		if ((size_t)(pos.x + unit_type->dimensions.to.x) >= game_st.map_width) return false;
		if ((size_t)(pos.y + unit_type->dimensions.to.y) >= game_st.map_height) return false;
		return true;
	}
	bool is_in_map_bounds(rect bounds) const {
		if (bounds.from.x < 0) return false;
		if ((size_t)bounds.to.x > game_st.map_width) return false;
		if (bounds.from.y < 0) return false;
		if ((size_t)bounds.to.y > game_st.map_height) return false;
		return true;
	}
	bool is_in_map_bounds(xy pos) const {
		return (size_t)pos.x < game_st.map_width && (size_t)pos.y < game_st.map_height;
	}

	bool is_in_bounds(xy pos, rect bounds) const {
		return pos >= bounds.from && pos < bounds.to;
	}

	rect translate_rect(rect src, xy translation) const {
		return {src.from + translation, src.to + translation};
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

	void tiles_flags_and(int offset_x, int offset_y, int width, int height, int flags) {
		if (std::max((size_t)offset_x, (size_t)(offset_x + width)) > game_st.map_tile_width) xcept("attempt to mask tile out of bounds");
		if (std::max((size_t)offset_y, (size_t)(offset_y + height)) > game_st.map_tile_height) xcept("attempt to mask tile out of bounds");
		for (int y = offset_y; y != offset_y + height; ++y) {
			for (int x = offset_x; x != offset_x + width; ++x) {
				st.tiles[x + y * game_st.map_tile_width].flags &= flags;
			}
		}
	}
	void tiles_flags_or(int offset_x, int offset_y, int width, int height, int flags) {
		if (std::max((size_t)offset_x, (size_t)(offset_x + width)) > game_st.map_tile_width) xcept("attempt to mask tile out of bounds");
		if (std::max((size_t)offset_y, (size_t)(offset_y + height)) > game_st.map_tile_height) xcept("attempt to mask tile out of bounds");
		for (int y = offset_y; y != offset_y + height; ++y) {
			for (int x = offset_x; x != offset_x + width; ++x) {
				st.tiles[x + y * game_st.map_tile_width].flags |= flags;
			}
		}
	}

	bool unit_type_spreads_creep(const unit_type_t* ut, bool include_non_evolving) const {
		if (ut->id == UnitTypes::Zerg_Hatchery && include_non_evolving) return true;
		if (ut->id == UnitTypes::Zerg_Lair) return true;
		if (ut->id == UnitTypes::Zerg_Hive) return true;
		if (ut->id == UnitTypes::Zerg_Creep_Colony && include_non_evolving) return true;
		if (ut->id == UnitTypes::Zerg_Spore_Colony) return true;
		if (ut->id == UnitTypes::Zerg_Sunken_Colony) return true;
		return false;
	}

	void update_sprite_some_images_set_redraw(sprite_t* sprite) {
		for (image_t* img : ptr(sprite->images)) {
			if (img->modifier == 11) img->flags |= image_t::flag_redraw;
		}
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

	size_t unit_space_occupied(const unit_t* u) const {
		size_t r = 0;
		for (auto id : u->loaded_units) {
			unit_t* nu = get_unit(id);
			if (!nu) continue;
			r += nu->unit_type->space_required;;
		}
		return r;
	}

	int get_unit_strength(unit_t* u, bool ground) const {
		if (u->unit_type->id == UnitTypes::Zerg_Larva || u->unit_type->id == UnitTypes::Zerg_Egg || u->unit_type->id == UnitTypes::Zerg_Cocoon || u->unit_type->id == UnitTypes::Zerg_Lurker_Egg) return 0;
		int vis_hp_shields = visible_hp_plus_shields(u);
		int max_vis_hp_shields = max_visible_hp_plus_shields(u);
		if (u_hallucination(u)) {
			if (vis_hp_shields < max_vis_hp_shields) return 0;
		}

		int r = ground ? game_st.unit_ground_strength[u->unit_type->id] : game_st.unit_air_strength[u->unit_type->id];
		if (u->unit_type->id == UnitTypes::Terran_Bunker) {
			r = ground ? game_st.unit_ground_strength[UnitTypes::Terran_Marine] : game_st.unit_air_strength[UnitTypes::Terran_Marine];
			r *= unit_space_occupied(u);
		}
		if (ut_has_energy(u) && !u_hallucination(u)) {
			r += u->energy.integer_part() / 2;
		}
		return r * vis_hp_shields / max_vis_hp_shields;
	}

	void set_unit_hp(unit_t* u, fp8 hitpoints) {
		u->hp = std::min(hitpoints, u->unit_type->hitpoints);
		if (us_selected(u) && u->sprite->visibility_flags&st.local_mask) {
			update_sprite_some_images_set_redraw(u->sprite);
		}
		if (u_completed(u)) {
			// damage overlay stuff

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

	bool is_mineral_field(const unit_type_t* unit_type) {
		if (unit_type->id == UnitTypes::Resource_Mineral_Field) return true;
		if (unit_type->id == UnitTypes::Resource_Mineral_Field_Type_2) return true;
		if (unit_type->id == UnitTypes::Resource_Mineral_Field_Type_3) return true;
		return false;
	}

	void set_unit_resources(unit_t* u, int resources) {
		if (!ut_resource(u)) return;
		u->building.resource.resource_count = resources;
		if (is_mineral_field(u->unit_type)) {
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

	bool is_disabled(const unit_t* u) const {
		if (u_disabled(u)) return true;
		if (u->lockdown_timer) return true;
		if (u->stasis_timer) return true;
		if (u->maelstrom_timer) return true;
		return false;
	}

	void set_current_button_set(unit_t* u, int type) {
		if (type != UnitTypes::None && !ut_building(u)) {
			if (is_disabled(u)) return;
		}
		u->current_button_set = type;
	}

	image_t* find_image(sprite_t* sprite, int first_id, int last_id) const {
		for (image_t* i : ptr(sprite->images)) {
			if (i->image_type->id >= first_id && i->image_type->id <= last_id) return i;
		}
		return nullptr;
	}

	void destroy_image_from_to(sprite_t* sprite, int first_id, int last_id) {
		image_t* image = find_image(sprite, first_id, last_id);
		if (image) destroy_image(image);
	}

	void disable_effect_end(unit_t* u, int first, int last) {
		bool still_disabled = is_disabled(u);
		if (u->subunit && !still_disabled) {
			u_unset_status_flag(u, unit_t::status_flag_disabled);
			set_unit_order(u, u->unit_type->return_to_idle);
		}
		image_t* image = find_image(u->sprite, first, last);
		if (!image && u->subunit) image = find_image(u->subunit->sprite, first, last);
		if (image) iscript_run_anim(image, iscript_anims::Death);
		if (ut_worker(u) && !still_disabled) {
			unit_t* target = u->worker.gather_target;
			if (target && ut_resource(target)) {
				if (u->worker.is_gathering) {
					if (target->building.resource.is_being_gathered) {
						xcept("disable_effect_end fixme");
					}
				}
			}
		}
		u->order_queue_timer = 15;
	}

	void remove_stasis(unit_t* u) {
		xcept("remove_stasis fixme");
		u->stasis_timer = 0;
		set_current_button_set(u, u->unit_type->id);
		u_set_status_flag(u, unit_t::status_flag_invincible, ut_invincible(u));
		disable_effect_end(u, idenums::IMAGEID_Stasis_Field_Small, idenums::IMAGEID_Stasis_Field_Large);
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
				xcept("remove stim");
			}
		}
		if (u->ensnare_timer) {
			--u->ensnare_timer;
			if (!u->ensnare_timer) {
				xcept("remove ensnare");
			}
		}
		if (u->defensive_matrix_timer) {
			--u->defensive_matrix_timer;
			if (!u->defensive_matrix_timer) {
				xcept("remove defensive matrix");
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
		for (auto& v : u->acid_spore_time) {
			if (!v) continue;
			--v;
			if (!v) --u->acid_spore_count;
		}
		if (u->acid_spore_count) {
			xcept("acid spore stuff");
		} else if (prev_acidSporeCount) {
			xcept("RemoveOverlays(u, IMAGEID_Acid_Spores_1_Overlay_Small, IMAGEID_Acid_Spores_6_9_Overlay_Large);");
		}

	}

	bool create_selection_circle(sprite_t* sprite, int color, int imageid) {
		return false;
	}

	void remove_selection_circle(sprite_t* sprite) {

	}

	// todo: does this stuff belong here, or should it be part of graphics stuff?
	void update_selection_sprite(sprite_t* sprite, int color) {
		if (!sprite->selection_timer) return;
		--sprite->selection_timer;
		if (~sprite->visibility_flags&st.local_mask) sprite->selection_timer = 0;
		if (sprite->selection_timer & 8 || (sprite->selection_timer == 0 && s_flag(sprite, sprite_t::flag_selected))) {
			if (!s_flag(sprite, (sprite_t::flags_t)1)) {
				if (create_selection_circle(sprite, color, idenums::IMAGEID_Selection_Circle_22pixels)) {
					sprite->flags |= 1;
				}
			}
		} else remove_selection_circle(sprite);
	}

	fp8 unit_cloak_energy_cost(const unit_t* u) const {
		switch (u->unit_type->id) {
		case UnitTypes::Terran_Ghost:
		case UnitTypes::Hero_Sarah_Kerrigan:
		case UnitTypes::Hero_Alexei_Stukov:
		case UnitTypes::Hero_Samir_Duran:
		case UnitTypes::Hero_Infested_Duran:
		case UnitTypes::Hero_Infested_Kerrigan:
			return fp8::integer(10) / 256;
		case UnitTypes::Terran_Wraith:
		case UnitTypes::Hero_Tom_Kazansky:
			return fp8::integer(13) / 256;
		}
		return fp8::zero();
	}

	void set_secondary_order(unit_t* u, const order_type_t* order_type) {
		if (u->secondary_order_type == order_type) return;
		u->secondary_order_type = order_type;
		u->secondary_order_state = 0;
		u->secondary_order_unk_a = 0;
		u->secondary_order_unk_b = 0;
		u->current_build_unit = nullptr;
	}

	void update_unit_energy(unit_t* u) {
		if (!ut_has_energy(u)) return;
		if (u_hallucination(u)) return;
		if (!u_completed(u)) return;
		if (u_cloaked(u) || u_requires_detector(u)) {
			fp8 cost = unit_cloak_energy_cost(u);
			if (u->energy < cost) {
				if (u->secondary_order_type->id == Orders::Cloak) set_secondary_order(u, get_order_type(Orders::Nothing));
			} else {
				u->energy -= cost;
				if (us_selected(u)) {
					update_sprite_some_images_set_redraw(u->sprite);
				}
			}
		} else {
			fp8 max_energy = unit_max_energy(u);
			if (u->unit_type->id == UnitTypes::Protoss_Dark_Archon && u->order_type->id == Orders::CompletingArchonSummon && u->order_state) {
				max_energy = fp8::integer(50);
			}
			u->energy = std::min(u->energy + fp8::integer(8) / 256, max_energy);
			if (us_selected(u)) {
				update_sprite_some_images_set_redraw(u->sprite);
			}
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
				u->shield_points += fp8::integer(7) / 256;
				if (u->shield_points > max_shields) u->shield_points = max_shields;
				if (us_selected(u)) {
					update_sprite_some_images_set_redraw(u->sprite);
				}
			}
		}
		if (u->unit_type->id == UnitTypes::Zerg_Zergling || u->unit_type->id == UnitTypes::Hero_Devouring_One) {
			if (u->ground_weapon_cooldown == 0) u->order_queue_timer = 0;
		}
		u->is_being_healed = false;
		if (u_completed(u) || ~u->sprite->flags & sprite_t::flag_hidden) {
			++u->cycle_counter;
			if (u->cycle_counter >= 8) {
				u->cycle_counter = 0;
				update_unit_status_timers(u);
			}
		}
		if (u_completed(u)) {
			if (ut_regens_hp(u)) {
				if (u->hp > fp8::zero() && u->hp != u->unit_type->hitpoints) {
					set_unit_hp(u, u->hp + fp8::integer(4) / 256);
				}
			}
			update_unit_energy(u);
			if (u->recent_order_timer) --u->recent_order_timer;
			if (u->remove_timer) {
				--u->remove_timer;
				if (!u->remove_timer) {
					xcept("orders_SelfDestructing...");
					return;
				} else {
					int gf = u->unit_type->group_flags;
					if (gf & GroupFlags::Terran && (gf & (GroupFlags::Zerg | GroupFlags::Protoss)) == 0) {
						if (u_grounded_building(u) || ut_flying_building(u)) {
							if (unit_hp_percent(u) <= 33) {
								xcept("killTargetUnitCheck(...)");
							}
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

	const weapon_type_t* unit_ground_weapon(const unit_t* u) const {
		if (u->unit_type->id == UnitTypes::Zerg_Lurker && !u_burrowed(u)) return nullptr;
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

	bool unit_is_carrier(const unit_t* u) const {
		return u->unit_type->id == UnitTypes::Protoss_Carrier || u->unit_type->id == UnitTypes::Hero_Gantrithor;
	}

	bool unit_is_reaver(const unit_t* u) const {
		return u->unit_type->id == UnitTypes::Protoss_Reaver || u->unit_type->id == UnitTypes::Hero_Warbringer;
	}

	bool unit_is_queen(const unit_t* u) const {
		return u->unit_type->id == UnitTypes::Zerg_Queen || u->unit_type->id == UnitTypes::Hero_Matriarch;
	}

	bool unit_is_hatchery(const unit_t* u) const {
		if (u->unit_type->id == UnitTypes::Zerg_Hatchery) return true;
		if (u->unit_type->id == UnitTypes::Zerg_Lair) return true;
		if (u->unit_type->id == UnitTypes::Zerg_Hive) return true;
		return false;
	}

	bool unit_is_ghost(const unit_t* u) const {
		if (u->unit_type->id == UnitTypes::Terran_Ghost) return true;
		if (u->unit_type->id == UnitTypes::Hero_Sarah_Kerrigan) return true;
		if (u->unit_type->id == UnitTypes::Hero_Alexei_Stukov) return true;
		if (u->unit_type->id == UnitTypes::Hero_Samir_Duran) return true;
		if (u->unit_type->id == UnitTypes::Hero_Infested_Duran) return true;
		return false;
	}

	bool unit_is_map_revealer(const unit_t* u) const {
		if (u->unit_type->id == UnitTypes::Spell_Scanner_Sweep) return true;
		if (u->unit_type->id == UnitTypes::Special_Map_Revealer) return true;
		return false;
	}

	bool unit_target_is_undetected(const unit_t* u, const unit_t* target) const {
		if (!u_cloaked(target) && !u_requires_detector(target)) return false;
		if (u->detected_flags & (1 << u->owner)) return false;
		return true;
	}

	bool unit_target_is_visible(const unit_t* u, const unit_t* target) const {
		if (target->sprite->visibility_flags & (1 << u->owner)) return true;
		return true;
	}

	bool unit_position_is_visible(const unit_t* u, xy position) const {
		return (st.tiles[tile_index(position)].visible & (1 << u->owner)) == 0;
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

	bool is_reachable(xy from, xy to) const {
		return get_region_at(from)->group_index == get_region_at(to)->group_index;
	}

	bool cc_can_be_infested(const unit_t* u) const {
		if (u->unit_type->id != UnitTypes::Terran_Command_Center) return false;
		if (!u_completed(u)) return false;
		return unit_hp_percent(u) < 50;
	}

	bool unit_can_attack_target(const unit_t* u, const unit_t* target) const {
		if (!target) return false;
		if (is_disabled(u)) return false;
		if (u_invincible(target)) return false;
		if (us_hidden(target)) return false;
		if (unit_target_is_undetected(u, target)) return false;
		if (unit_is_carrier(u)) return true;
		if (unit_is_reaver(u)) {
			if (u_flying(target)) return false;
			return is_reachable(u->sprite->position, target->sprite->position);
		}
		if (unit_is_queen(u)) {
			return cc_can_be_infested(target);
		}
		return unit_target_weapon(u, target) != nullptr;
	}

	bool unit_autoattack(unit_t* u) {
		if (!u->auto_target_unit) return false;
		if (unit_target_is_enemy(u, u->auto_target_unit)) {
			if (unit_can_attack_target(u, u->auto_target_unit)) {
				xcept("auto attack waa");
				return true;
			}
		} else {
			u->auto_target_unit = nullptr;
		}
		return false;
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
		if (u->unit_type->id == UnitTypes::Zerg_Larva) {
			if (!u_grounded_building(target)) return false;
		} else if (target->unit_type->id == UnitTypes::Zerg_Larva) {
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

	template<size_t integer_bits>
	direction_t atan(fixed_point<integer_bits, 8, true> x) const {
		bool negative = x < decltype(x)::zero();
		if (negative) x = -x;
		typename decltype(x)::raw_unsigned_type uv = x.raw_value;
		size_t r = std::lower_bound(tan_table.begin(), tan_table.end(), uv) - tan_table.begin();
		return negative ? -direction_t::from_raw((direction_t::raw_type)r) : direction_t::from_raw((direction_t::raw_type)r);
	}

	direction_t xy_direction(xy_fp8 pos) const {
		if (pos.x == fp8::zero()) return pos.y <= fp8::zero() ? direction_t::zero() : direction_t::from_raw(-128);
		direction_t r = atan(pos.y / pos.x);
		if (pos.x > fp8::zero()) r += direction_t::from_raw(64);
		else r = direction_t::from_raw(-64) + r;
		return r;
	}

	direction_t xy_direction(xy pos) const {
		if (pos.x == 0) return pos.y <= 0 ? direction_t::zero() : direction_t::from_raw(-128);
		direction_t r = atan(fp8::integer(pos.y) / pos.x);
		if (pos.x > 0) r = direction_t::from_raw(64) + r;
		else r = direction_t::from_raw(-64) + r;
		return r;
	}

	xy_fp8 direction_xy(direction_t dir, fp8 length) const {
		return global_st.direction_table[direction_index(dir)] * length;
	}

	xy_fp8 direction_xy(direction_t dir, int length) const {
		return global_st.direction_table[direction_index(dir)] * length;
	}

	xy_fp8 direction_xy(direction_t dir) const {
		return global_st.direction_table[direction_index(dir)];
	}

	size_t direction_index(direction_t dir) const {
		auto v = dir.fractional_part();
		if (v < 0) return 256 + v;
		else return v;
	}

	direction_t direction_from_index(size_t index) const {
		int v = index;
		if (v >= 128) v = -(256 - v);
		return direction_t::from_raw(v);
	}

	direction_t units_direction(const unit_t* from, const unit_t* to) const {
		xy relpos = to->sprite->position - from->sprite->position;
		return xy_direction(relpos);
	}

	bool unit_target_in_attack_angle(const unit_t* u, const unit_t* target, const weapon_type_t* weapon) const {
		auto dir = units_direction(u, target);
		if (u->unit_type->id == UnitTypes::Zerg_Lurker) {
			xcept("unit_target_in_attack_angle lurker: fixme?");
			// For some reason, this field is set here for lurkers, but I would really like u to be const.
			// todo: figure out if it is necessary.
			//u->heading = dir;
			return true;
		}
		return fp8::extend(dir - u->heading).abs() <= weapon->attack_angle;
	}

	int weapon_max_range(const unit_t* u, const weapon_type_t* w) const {
		auto has_upgrade = [&](int id) {
			return st.upgrade_levels[u->owner][id];
		};
		auto range_upgrade_bonus = [&]() {
			switch (u->unit_type->id) {
			case UnitTypes::Terran_Marine:
				return has_upgrade(UpgradeTypes::U_238_Shells) ? 32 : 0;
			case UnitTypes::Zerg_Hydralisk:
				return has_upgrade(UpgradeTypes::Grooved_Spines) ? 32 : 0;
			case UnitTypes::Protoss_Dragoon:
				return has_upgrade(UpgradeTypes::Singularity_Charge) ? 64 : 0;
			case UnitTypes::Hero_Fenix_Dragoon:
				return 64;
			case UnitTypes::Terran_Goliath:
			case UnitTypes::Terran_Goliath_Turret:
				return w->id == WeaponTypes::Hellfire_Missile_Pack && has_upgrade(UpgradeTypes::Charon_Boosters) ? 96 : 0;
			case UnitTypes::Hero_Alan_Schezar:
			case UnitTypes::Hero_Alan_Schezar_Turret:
				return w->id == WeaponTypes::Hellfire_Missile_Pack_Alan_Schezar ? 96 : 0;
			};
			return 0;
		};
		int r = 0;
		if (u_hidden(u)) r += 64;
		r += range_upgrade_bonus();
		return r;
	}

	int unit_target_movement_range(const flingy_t* u, const flingy_t* target) const {
		if (!u_movement_flag(u, 2)) return 0;
		if (u_movement_flag(target, 2)) {
			if (fp8::extend(target->next_velocity_direction - u->next_velocity_direction).abs() <= fp8::from_raw(32)) return 0;
		}
		return unit_halt_distance(u).integer_part();
	}

	bool unit_target_in_weapon_movement_range(const unit_t* u, const unit_t* target) const {
		if (!target) target = u->order_target.unit;
		if (!target) return true;
		if (!unit_target_is_visible(u, target)) return false;
		auto* w = unit_target_weapon(u, target);
		if (!w) return false;
		int d = units_distance(u, target);
		if (w->min_range && d < w->min_range) return false;
		int max_range = weapon_max_range(u, w);
		max_range += unit_target_movement_range(u, target);
		return d <= max_range;
	}

	bool some_unit_target_computer_thing(const unit_t* u, const unit_t* target) const {
		if (st.players[u->owner].controller != state::player_t::controller_computer_game) return false;
		if (u_flying(u)) return false;
		if (unit_target_in_weapon_movement_range(u, target)) return false;
		return u_status_flag(u, (unit_t::status_flags_t)0x80);
	}

	unit_t* unit_first_loaded_unit(const unit_t* u) const {
		for (size_t i = 0; i != u->unit_type->space_provided; ++i) {
			unit_t* nu = get_unit(u->loaded_units.at(i));
			if (!nu) continue;
			return nu;
		}
		return nullptr;
	}

	size_t unit_interceptor_count(const unit_t* u) const {
		if (!unit_is_carrier(u)) return 0;
		return u->carrier.inside_count + u->carrier.outside_count;
	}
	size_t unit_scarab_count(const unit_t* u) const {
		if (!unit_is_reaver(u)) return 0;
		return u->reaver.inside_count + u->reaver.outside_count;
	}

	bool unit_can_attack(const unit_t* u) const {
		if (unit_or_subunit_ground_weapon(u) || unit_or_subunit_air_weapon(u)) return true;
		if (unit_interceptor_count(u)) return true;
		if (unit_scarab_count(u)) return true;
		return false;
	}

	int unit_target_attack_priority(const unit_t* u, const unit_t* target) const {
		bool is_loaded_unit = false;
		if (target->unit_type->id == UnitTypes::Terran_Bunker) {
			const unit_t* loaded_unit = unit_first_loaded_unit(u);
			if (loaded_unit) {
				target = loaded_unit;
				is_loaded_unit = true;
			}
		}
		if (target->unit_type->id == UnitTypes::Zerg_Larva) return 5;
		if (target->unit_type->id == UnitTypes::Zerg_Egg) return 5;
		if (target->unit_type->id == UnitTypes::Zerg_Cocoon) return 5;
		if (target->unit_type->id == UnitTypes::Zerg_Lurker_Egg) return 5;
		int r = 0;
		if (ut_worker(u)) r += 2;
		else if (!unit_can_attack_target(u, target)) {
			if (unit_can_attack(target)) r += 2;
			else if (u_can_move(u)) r += 3;
			else r += 4;
		}
		if (is_loaded_unit || !u_completed(u)) ++r;
		if (r == 0 && u_cannot_attack(u)) ++r;
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
		for (unit_t* target : find_units(bounds)) {
			if (target == u) continue;
			if (!unit_target_is_enemy(u, target)) continue;
			if (!unit_target_is_visible(u, target)) continue;
			if (!unit_can_attack_target(u, target)) continue;
			int distance = units_distance(main_unit, target);
			if (distance > max_distance) continue;
			if (distance < min_distance) continue;
			if (!can_turn) {
				if (!attacking_unit->unit_type->ground_weapon) xcept("find_acquire_target: null ground weapon");
				if (!unit_target_in_attack_angle(attacking_unit, target, attacking_unit->unit_type->ground_weapon)) continue;
			}
			if (!some_unit_target_computer_thing(u, target)) {
				int prio = unit_target_attack_priority(u, target);
				if (targets[prio].size() < 0x10) targets[prio].push_back(target);
			}
		}
		return targets;
	}

	unit_t* find_acquire_target(const unit_t* u) const {
		int acq_range = unit_target_acquisition_range(u);
		if (u_hidden(u)) acq_range += 2;

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
		u->recent_order_timer = 15;
	}

	// rename to order_done?
	void idle(unit_t* u) {
		if (!u->order_queue.empty()) {
			u->user_action_flags |= 1;
			activate_next_order(u);
		} else set_unit_order(u, u->unit_type->return_to_idle);
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
		size_t top_index = tile_bb.from.y * game_st.map_tile_width + tile_bb.from.x;
		size_t bottom_index = tile_bb.to.y * game_st.map_tile_width + tile_bb.from.x;
		for (size_t i = 0; i != tile_width; ++i) {
			if (~st.tiles.at(top_index).flags & tile_t::flag_occupied) return true;
			if (~st.tiles.at(bottom_index).flags & tile_t::flag_occupied) return true;
			++top_index;
			++bottom_index;
		}
		size_t left_index = tile_bb.from.y * game_st.map_tile_width + tile_bb.from.x;
		size_t right_index = tile_bb.from.y * game_st.map_tile_width + tile_bb.to.x;
		for (size_t i = 0; i != tile_height; ++i) {
			if (~st.tiles.at(left_index).flags & tile_t::flag_occupied) return true;
			if (~st.tiles.at(right_index).flags & tile_t::flag_occupied) return true;
			left_index += game_st.map_tile_width;
			right_index += game_st.map_tile_width;
		}
		log("any_neighbor_tile_unoccupied failed\n");
		return false;
	}

	bool unit_pos_is_bordering_target(const unit_t* u, xy pos, const unit_t* target) {
		auto bb = unit_sprite_inner_bounding_box(target);
		bb.from -= target->unit_type->dimensions.to + xy(1, 1);
		bb.to += target->unit_type->dimensions.from + xy(1, 1);
		if (pos.x == bb.from.x || pos.x == bb.to.x) {
			if (pos.y >= bb.from.y && pos.y <= bb.to.y) {
				return true;
			}
		}
		if (pos.y == bb.from.y || pos.y == bb.to.y) {
			if (pos.x >= bb.from.x && pos.x <= bb.to.x) {
				return false;
			}
		}
		return false;
	}

	void move_to_target(unit_t* u, unit_t* target) {
		if (u->move_target.unit == target && u->move_target.pos == target->sprite->position) {
			u_unset_status_flag(u, unit_t::status_flag_immovable);
		} else if (unit_pos_is_bordering_target(u, u->move_target.pos, target)) {
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
			queue_order_front(u, get_order_type(Orders::HarvestGas), order_target_t(target));
		} else {
			queue_order_front(u, get_order_type(Orders::GatheringInterrupted), {});
			queue_order_front(u, get_order_type(Orders::MiningMinerals), order_target_t(target));
			idle(u);
			return true;
		}
		return false;
	}

	void wait_for_resource(unit_t* u, unit_t* target) {
		u->order_queue_timer = 0;
		u->order_state = 2;
		if (!try_gather_resource(u, target)) {
			if (!u->worker.gather_target) {
				u->worker.gather_target = target;
				target->building.resource.gather_queue.push_front(*u);
			}
			queue_order_front(u, get_order_type(Orders::GatherWaitInterrupted), {});
		}
	}

	bool is_facing_next_target_waypoint(const unit_t* u) const {
		if (u->position == u->next_target_waypoint) return true;
		if (u->heading == xy_direction(u->next_target_waypoint - u->sprite->position)) return true;
		return false;
	}

	int unit_long_path_distance(const unit_t* u, xy from, xy to) const {
		if (!u->pathing_flags & 1) return xy_length(to - from);
		if (!is_reachable(from, to)) return 0x7fff;
		pathfinder pf;
		if (!pathfinder_find_long_path(pf, u, from, to)) return 0x7ffe;
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

	void destroy_carrying_images(const unit_t* u) {
		destroy_image_from_to(u->sprite, idenums::IMAGEID_Mineral_Chunk_Shadow, idenums::IMAGEID_Psi_Emitter_Shadow_Carried);
		destroy_image_from_to(u->sprite, idenums::IMAGEID_Flag, idenums::IMAGEID_Terran_Gas_Tank_Type2);
		destroy_image_from_to(u->sprite, idenums::IMAGEID_Uraj, idenums::IMAGEID_Khalis);
	}

	void unit_gather_resources_from(unit_t* u, unit_t* resource) {
		const image_type_t* image_type = nullptr;
		bool is_minerals = false;
		switch (resource->unit_type->id) {
		case UnitTypes::Terran_Refinery:
			image_type = get_image_type(idenums::IMAGEID_Terran_Gas_Tank_Type1);
			break;
		case UnitTypes::Protoss_Assimilator:
			image_type = get_image_type(idenums::IMAGEID_Protoss_Gas_Orb_Type1);
			break;
		case UnitTypes::Zerg_Extractor:
			image_type = get_image_type(idenums::IMAGEID_Zerg_Gas_Sac_Type1);
			break;
		case UnitTypes::Resource_Mineral_Field:
		case UnitTypes::Resource_Mineral_Field_Type_2:
		case UnitTypes::Resource_Mineral_Field_Type_3:
			image_type = get_image_type(idenums::IMAGEID_Mineral_Chunk_Type1);
			is_minerals = true;
			break;
		}
		if (!image_type) return;
		int gathered = 0;
		if (resource->building.resource.resource_count < 8) {
			image_type = get_image_type(image_type->id + 1);
			if (is_minerals) {
				gathered = resource->building.resource.resource_count;
				order_SelfDestructing(resource);
			} else {
				resource->building.resource.resource_count = 0;
				gathered = 2;
			}
		} else {
			set_unit_resources(resource, resource->building.resource.resource_count - 8);
			gathered = 8;
			if (is_minerals) {
				if (resource->building.resource.resource_count == 0) {
					order_SelfDestructing(resource);
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
				if (!is_disabled(queued_unit)) {
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
		if (u->unit_type->id == UnitTypes::Zerg_Hive) return true;
		if (u->unit_type->id == UnitTypes::Zerg_Lair) return true;
		if (u->unit_type->id == UnitTypes::Zerg_Hatchery && unit_is_morphing_building(u)) return true;
		return false;
	}

	unit_t* find_nearest_active_resource_depot(const unit_t* u) const {
		if (us_hidden(u)) {
			return find_nearest_unit(u->sprite->position, map_bounds(), [&](const unit_t* target) {
				if (!unit_is_active_resource_depot(target)) return false;
				if (target->owner != u->owner) return false;
				if (~u->pathing_flags & 1 || !is_reachable(u->sprite->position, target->sprite->position)) return false;
				return true;
			});
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
			auto right_i = get(st.unit_finder_x, u->unit_finder_bounding_box.from.x);
			auto down_i = get(st.unit_finder_y, u->unit_finder_bounding_box.from.y);
			return find_nearest_unit(u->sprite->position, map_bounds(), left_i, up_i, right_i, down_i, [&](const unit_t* target) {
				if (!unit_is_active_resource_depot(target)) return false;
				if (target->owner != u->owner) return false;
				if (~u->pathing_flags & 1 || !is_reachable(u->sprite->position, target->sprite->position)) return false;
				return true;
			});
		}
	}

	void drop_carried_items(unit_t* u) {
		if ((u->carrying_flags & ~3) == 0) return;
		xcept("drop_carried_items: fixme");
	}

	void order_SelfDestructing(unit_t* u) {
		drop_carried_items(u);
		while (!u->order_queue.empty()) {
			remove_queued_order(u, &u->order_queue.front());
		}
		set_queued_order(u, true, get_order_type(Orders::Die), {});
		activate_next_order(u);
	}

	void order_Stop(unit_t* u) {
		stop_unit(u);
		set_next_target_waypoint(u, u->move_target.pos);
		iscript_run_to_idle(u);
		idle(u);
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
				if (unit_target_acquisition_range(u)) {
					unit_t* target = find_acquire_target(u);
					if (target) xcept("waa set target!");
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
				xcept("fixme move to unit");
			} else {
				set_unit_move_target(u, u->order_target.pos);
				set_next_target_waypoint(u, u->order_target.pos);
				u->order_state = 1;
			}
		} else {
			if (unit_is_at_move_target(u)) {
				idle(u);
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
			idle(u);
			return;
		}
		set_unit_gathering(u);

		auto find_mineral_field = [&](xy pos, int range, bool require_not_being_gathered) {
			rect area{pos - xy(range, range), pos + xy(range, range)};
			int ground_height = get_ground_height_at(pos);
			return find_nearest_unit(pos, area, [&](const unit_t* target) {
				if (!is_mineral_field(target->unit_type)) return false;
				if (!any_neighbor_tile_unoccupied(target)) return false;
				if (!is_reachable(u->sprite->position, target->sprite->position)) return false;
				if (get_ground_height_at(pos) != ground_height) return false;
				if (!unit_position_is_visible(u, target->sprite->position)) return false;
				if (unit_long_path_distance(target, target->sprite->position, u->sprite->position) > 2 * range) return false;
				if (require_not_being_gathered && target->building.resource.is_being_gathered) return false;
				return true;
			});
		};

		unit_t* target = u->order_target.unit;
		if (!target || !is_mineral_field(target->unit_type)) {
			xy search_pos = u->sprite->position;
			if (u->worker.target_resource_position != xy()) {
				search_pos = u->worker.target_resource_position;
				u->worker.target_resource_position = {};
			}
			target = find_mineral_field(search_pos, 32 * 12, false);
			if (!target) {
				stop_unit(u);
				set_next_target_waypoint(u, u->move_target.pos);
				idle(u);
				return;
			}
			u->order_target.pos = target->sprite->position;
			u->order_target.unit = target;
			u->order_state = 0;
		}
		if (!ut_resource(target)) xcept("MoveToMinerals: target is not a resource");
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
						} else idle(u);
					} else {
						if (u->carrying_flags & 2) {
							set_unit_order(u, get_order_type(Orders::ReturnMinerals));
						} else {
							set_next_target_waypoint(u, u->order_target.pos);
							order_target_t order_target;
							order_target.position = target->sprite->position;
							order_target.unit = target;
							queue_order_front(u, get_order_type(Orders::WaitForMinerals), order_target);
							idle(u);
						}
					}
				}
			}
		}
	}

	void order_WaitForMinerals(unit_t* u) {
		if (u->order_target.unit && is_mineral_field(u->order_target.unit->unit_type)) {
			if (u->order_state == 0) {
				wait_for_resource(u, u->order_target.unit);
			}
		} else {
			set_unit_order(u, get_order_type(Orders::MoveToMinerals));
		}
	}

	void order_MiningMinerals(unit_t* u) {
		if (!ut_worker(u)) xcept("MiningMinerals: unit is not a worker");
		unit_t* target = u->order_target.unit;
		if (target && is_mineral_field(target->unit_type)) {
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
						set_unit_order(u, get_order_type(Orders::ReturnGas));
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
			int nid = next_order->order_type->id;
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
				idle(u);
				return;
			}
		}
		u_unset_status_flag(u, unit_t::status_flag_gathering);
		u_unset_status_flag(u, unit_t::status_flag_no_collide);
		check_unit_collision(u);
		if (u->order_queue.empty()) set_queued_order(u, false, u->unit_type->return_to_idle, {});
		if (u->path) {
			free_path(u->path);
			u->path = nullptr;
		}
		u->movement_state = movement_states::UM_Init;
		if (u->sprite->elevation_level < 12) u->pathing_flags |= 1;
		else u->pathing_flags &= ~1;
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		idle(u);
	}

	void order_ReturnGas(unit_t* u) {
		order_ReturnMinerals(u);
	}

	void order_ReturnMinerals(unit_t* u) {
		if ((u->carrying_flags & 3) == 0) {
			stop_unit(u);
			set_next_target_waypoint(u, u->move_target.pos);
			idle(u);
			return;
		}
		set_unit_gathering(u);
		unit_t* target = u->order_target.unit;
		if (!target || !unit_is_active_resource_depot(target)) {
			u->order_state = 0;
		}
		if (u->order_state == 0) {
			target = find_nearest_active_resource_depot(u);
			if (target) {
				u->order_target.unit = target;
				move_to_target_reset(u, target);
				u->order_state = 1;
			} else {
				u->order_queue_timer = 75;
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
						queue_order_front(u, next_order_type, order_target_t(u->worker.target_resource_unit));
					} else {
						queue_order_front(u, next_order_type, {});
					}
					idle(u);
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
						} else if (u->sprite->position.y < bb.to.y) {
							val = &target->building.hatchery.larva_spawn_side_values[3];
						} else if (u->sprite->position.x < bb.from.x) {
							val = &target->building.hatchery.larva_spawn_side_values[2];
						} else {
							val = &target->building.hatchery.larva_spawn_side_values[0];
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
				xcept("fixme hallucination death");
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
		idle(u);
	}

	void order_GatheringInterrupted(unit_t* u) {
		if (u->worker.is_gathering) {
			gather_queue_next(u, u->worker.gather_target);
		}
		u_unset_status_flag(u, unit_t::status_flag_order_not_interruptible);
		idle(u);
	}

	void execute_main_order(unit_t* u) {
		switch (u->order_type->id) {
		case Orders::Die:
			order_Die(u);
			return;
		case Orders::IncompleteWarping:
			xcept("IncompleteWarping");
			return;
		case Orders::NukeTrack:
			xcept("NukeTrack");
			return;
		case Orders::WarpIn:
			xcept("WarpIn");
			return;
		}

		if (is_disabled(u) || (!u_can_move(u) && u_cannot_attack(u))) {
			if (u->main_order_timer == 0) u->main_order_timer = 15;
			if (is_disabled(u)) return;
		}

		switch (u->order_type->id) {
		case Orders::TurretGuard:
			order_TurretGuard(u);
			break;
		case Orders::TurretAttack:
			xcept("TurretAttack");
			break;
		case Orders::DroneBuild:
			xcept("DroneBuild");
			break;
		case Orders::PlaceBuilding:
			xcept("PlaceBuilding");
			break;
		case Orders::PlaceProtossBuilding:
			xcept("PlaceProtossBuilding");
			break;
		case Orders::ConstructingBuilding:
			xcept("ConstructingBuilding");
			break;
		case Orders::Repair:
			xcept("Repair");
			break;
		case Orders::ZergBirth:
			xcept("ZergBirth");
			break;
		case Orders::ZergUnitMorph:
			xcept("ZergUnitMorph");
			break;
		case Orders::IncompleteBuilding:
			xcept("IncompleteBuilding");
			break;
		case Orders::IncompleteMorphing:
			xcept("IncompleteMorphing");
			break;
		case Orders::ScarabAttack:
			xcept("ScarabAttack");
			break;
		case Orders::RechargeShieldsUnit:
			xcept("RechargeShieldsUnit");
			break;
		case Orders::BuildingLand:
			xcept("BuildingLand");
			break;
		case Orders::BuildingLiftOff:
			xcept("BuildingLiftOff");
			break;
		case Orders::ResearchTech:
			xcept("ResearchTech");
			break;
		case Orders::Upgrade:
			xcept("Upgrade");
			break;
		case Orders::GatheringInterrupted:
			order_GatheringInterrupted(u);
			break;
		case Orders::GatherWaitInterrupted:
			order_GatherWaitInterrupted(u);
			break;
		case Orders::Interrupted:
			xcept("Interrupted");
			break;
		case Orders::Sieging:
			xcept("Siegeing");
			break;
		case Orders::Unsieging:
			xcept("Unsiegeing");
			break;
		case Orders::ArchonWarp:
			xcept("ArchonWarp");
			break;
		case Orders::CompletingArchonSummon:
			xcept("CompletingArchonSummon");
			break;
		case Orders::NukeTrain:
			xcept("NukeTrain");
			break;
		case Orders::InitializeArbiter:
			xcept("InitializeArbiter");
			break;
		case Orders::ResetCollision:
			xcept("ResetCollision");
			break;
		case Orders::ResetHarvestCollision:
			order_ResetHarvestCollision(u);
			break;
		case Orders::CTFCOP2:
			xcept("CTFCOP2");
			break;
		case Orders::SelfDestructing:
			order_SelfDestructing(u);
			break;
		case Orders::Critter:
			xcept("Critter");
			break;
		case Orders::MedicHeal:
			xcept("MedicHeal");
			break;
		case Orders::HealMove:
			xcept("HealMove");
			break;
		case Orders::MedicHoldPosition:
			xcept("MedicHoldPosition");
			break;
		case Orders::MedicHealToIdle:
			xcept("MedicHealToIdle");
			break;
		case Orders::DarkArchonMeld:
			xcept("DarkArchonMeld");
			break;
		}
		if (u->order_queue_timer) {
			--u->order_queue_timer;
			return;
		}
		u->order_queue_timer = 8;
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
			xcept("BunkerGuard");
			break;
		case Orders::Move:
			order_Move(u);
			break;
		case Orders::Attack1:
			xcept("Attack1");
			break;
		case Orders::Attack2:
			xcept("Attack2");
			break;
		case Orders::AttackUnit:
			xcept("AttackUnit");
			break;
		case Orders::Hover:
			xcept("Hover");
			break;
		case Orders::AttackMove:
			xcept("AttackMove");
			break;
		case Orders::UnusedNothing:
			xcept("UnusedNothing");
			break;
		case Orders::UnusedPowerup:
			xcept("UnusedPowerup");
			break;
		case Orders::TowerGuard:
			xcept("TowerGuard");
			break;
		case Orders::TowerAttack:
			xcept("TowerAttack");
			break;
		case Orders::VultureMine:
			xcept("VultureMine");
			break;
		case Orders::StayInRange:
			xcept("StayInRange");
			break;
		case Orders::TurretAttack:
			xcept("TurretAttack");
			break;
		case Orders::Nothing:
			order_Nothing(u);
			break;
		case Orders::Unused_24:
			xcept("Unused_24");
			break;
		case Orders::DroneBuild:
			xcept("DroneBuild");
			break;
		case Orders::CastInfestation:
			xcept("CastInfestation");
			break;
		case Orders::MoveToInfest:
			xcept("MoveToInfest");
			break;
		case Orders::PlaceProtossBuilding:
			xcept("PlaceProtossBuilding");
			break;
		case Orders::Repair:
			xcept("Repair");
			break;
		case Orders::MoveToRepair:
			xcept("MoveToRepair");
			break;
		case Orders::ZergUnitMorph:
			xcept("ZergUnitMorph");
			break;
		case Orders::IncompleteMorphing:
			xcept("IncompleteMorphing");
			break;
		case Orders::BuildNydusExit:
			xcept("BuildNydusExit");
			break;
		case Orders::IncompleteWarping:
			xcept("IncompleteWarping");
			break;
		case Orders::Follow:
			xcept("Follow");
			break;
		case Orders::Carrier:
			xcept("Carrier");
			break;
		case Orders::ReaverCarrierMove:
			order_Move(u);
			break;
		case Orders::CarrierStop:
			xcept("CarrierStop");
			break;
		case Orders::CarrierAttack:
			xcept("CarrierAttack");
			break;
		case Orders::CarrierMoveToAttack:
			xcept("CarrierMoveToAttack");
			break;
		case Orders::CarrierIgnore2:
			xcept("CarrierIgnore2");
			break;
		case Orders::CarrierFight:
			xcept("CarrierFight");
			break;
		case Orders::CarrierHoldPosition:
			xcept("CarrierHoldPosition");
			break;
		case Orders::Reaver:
			xcept("Reaver");
			break;
		case Orders::ReaverAttack:
			xcept("ReaverAttack");
			break;
		case Orders::ReaverMoveToAttack:
			xcept("ReaverMoveToAttack");
			break;
		case Orders::ReaverFight:
			xcept("ReaverFight");
			break;
		case Orders::TrainFighter:
			xcept("TrainFighter");
			break;
		case Orders::RechargeShieldsUnit:
			xcept("RechargeShieldsUnit");
			break;
		case Orders::ShieldBattery:
			xcept("ShieldBattery");
			break;
		case Orders::InterceptorReturn:
			xcept("InterceptorReturn");
			break;
		case Orders::DroneLiftOff:
			xcept("DroneLiftOff");
			break;
		case Orders::Upgrade:
			xcept("Upgrade");
			break;
		case Orders::SpawningLarva:
			xcept("SpawningLarva");
			break;
		case Orders::Harvest1:
			xcept("Harvest1");
			break;
		case Orders::Harvest2:
			xcept("Harvest2");
			break;
		case Orders::MoveToGas:
			xcept("MoveToGas");
			break;
		case Orders::WaitForGas:
			xcept("WaitForGas");
			break;
		case Orders::HarvestGas:
			xcept("HarvestGas");
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
		case Orders::Interrupted:
			xcept("Interrupted");
			break;
		case Orders::EnterTransport:
			xcept("EnterTransport");
			break;
		case Orders::PickupIdle:
			xcept("PickupIdle");
			break;
		case Orders::PickupTransport:
			xcept("PickupTransport");
			break;
		case Orders::PickupBunker:
			xcept("PickupBunker");
			break;
		case Orders::Pickup4:
			xcept("Pickup4");
			break;
		case Orders::Unsieging:
			xcept("Unsieging");
			break;
		case Orders::WatchTarget:
			xcept("WatchTarget");
			break;
		case Orders::SpreadCreep:
			xcept("SpreadCreep");
			break;
		case Orders::CompletingArchonSummon:
			xcept("CompletingArchonSummon");
			break;
		case Orders::HoldPosition:
			xcept("HoldPosition");
			break;
		case Orders::Decloak:
			xcept("Decloak");
			break;
		case Orders::Unload:
			xcept("Unload");
			break;
		case Orders::MoveUnload:
			xcept("MoveUnload");
			break;
		case Orders::FireYamatoGun:
			xcept("FireYamatoGun");
			break;
		case Orders::MoveToFireYamatoGun:
			xcept("MoveToFireYamatoGun");
			break;
		case Orders::CastLockdown:
			xcept("CastLockdown");
			break;
		case Orders::Burrowing:
			xcept("Burrowing");
			break;
		case Orders::Burrowed:
			xcept("Burrowed");
			break;
		case Orders::Unburrowing:
			xcept("Unburrowing");
			break;
		case Orders::CastDarkSwarm:
			xcept("CastDarkSwarm");
			break;
		case Orders::CastParasite:
			xcept("CastParasite");
			break;
		case Orders::CastSpawnBroodlings:
			xcept("CastSpawnBroodlings");
			break;
		case Orders::NukeTrain:
			xcept("NukeTrain");
			break;
		case Orders::NukeLaunch:
			xcept("NukeLaunch");
			break;
		case Orders::NukePaint:
			xcept("NukePaint");
			break;
		case Orders::NukeUnit:
			xcept("NukeUnit");
			break;
		case Orders::CloakNearbyUnits:
			xcept("CloakNearbyUnits");
			break;
		case Orders::PlaceMine:
			xcept("PlaceMine");
			break;
		case Orders::RightClickAction:
			xcept("RightClickAction");
			break;
		case Orders::SuicideUnit:
			xcept("SuicideUnit");
			break;
		case Orders::SuicideLocation:
			xcept("SuicideLocation");
			break;
		case Orders::SuicideHoldPosition:
			xcept("SuicideHoldPosition");
			break;
		case Orders::Teleport:
			xcept("Teleport");
			break;
		case Orders::CastScannerSweep:
			xcept("CastScannerSweep");
			break;
		case Orders::Scanner:
			xcept("Scanner");
			break;
		case Orders::CastDefensiveMatrix:
			xcept("CastDefensiveMatrix");
			break;
		case Orders::CastPsionicStorm:
			xcept("CastPsionicStorm");
			break;
		case Orders::CastIrradiate:
			xcept("CastIrradiate");
			break;
		case Orders::CastPlague:
			xcept("CastPlague");
			break;
		case Orders::CastConsume:
			xcept("CastConsume");
			break;
		case Orders::CastEnsnare:
			xcept("CastEnsnare");
			break;
		case Orders::CastStasisField:
			xcept("CastStasisField");
			break;
		case Orders::Patrol:
			xcept("Patrol");
			break;
		case Orders::CTFCOPInit:
			xcept("CTFCOPInit");
			break;
		case Orders::CTFCOP2:
			xcept("CTFCOP2");
			break;
		case Orders::ComputerAI:
			xcept("ComputerAI");
			break;
		case Orders::AtkMoveEP:
			xcept("AtkMoveEP");
			break;
		case Orders::HarassMove:
			xcept("HarassMove");
			break;
		case Orders::AIPatrol:
			xcept("AIPatrol");
			break;
		case Orders::GuardPost:
			xcept("GuardPost");
			break;
		case Orders::RescuePassive:
			xcept("RescuePassive");
			break;
		case Orders::Neutral:
			xcept("Neutral");
			break;
		case Orders::ComputerReturn:
			xcept("ComputerReturn");
			break;
		case Orders::Critter:
			xcept("Critter");
			break;
		case Orders::HiddenGun:
			xcept("HiddenGun");
			break;
		case Orders::OpenDoor:
			xcept("OpenDoor");
			break;
		case Orders::CloseDoor:
			xcept("CloseDoor");
			break;
		case Orders::HideTrap:
			xcept("HideTrap");
			break;
		case Orders::RevealTrap:
			xcept("RevealTrap");
			break;
		case Orders::EnableDoodad:
			xcept("EnableDoodad");
			break;
		case Orders::WarpIn:
			xcept("WarpIn");
			break;
		case Orders::MedicHealToIdle:
			xcept("MedicHealToIdle");
			break;
		case Orders::CastRestoration:
			xcept("CastRestoration");
			break;
		case Orders::CastDisruptionWeb:
			xcept("CastDisruptionWeb");
			break;
		case Orders::DarkArchonMeld:
			xcept("DarkArchonMeld");
			break;
		case Orders::CastFeedback:
			xcept("CastFeedback");
			break;
		case Orders::CastOpticalFlare:
			xcept("CastOpticalFlare");
			break;
		case Orders::CastMaelstrom:
			xcept("CastMaelstrom");
			break;
		}
	}

	void execute_secondary_order(unit_t* u) {
		if (u->secondary_order_type->id == Orders::Hallucination2) {
			if (u->defensive_matrix_hp || u->stim_timer || u->ensnare_timer || u->lockdown_timer || u->irradiate_timer || u->stasis_timer || u->parasite_flags || u->storm_timer || u->plague_timer || u->is_blind || u->maelstrom_timer) {
				order_SelfDestructing(u);
			}
			return;
		}
		if (is_disabled(u)) return;
		switch (u->secondary_order_type->id) {
		case Orders::Train:
			xcept("Train");
			break;
		case Orders::BuildAddon:
			xcept("BuildAddon");
			break;
		case Orders::TrainFighter:
			xcept("TrainFighter");
			break;
		case Orders::ShieldBattery:
			xcept("ShieldBattery");
			break;
		case Orders::SpawningLarva:
			xcept("SpawningLarva");
			break;
		case Orders::SpreadCreep:
			xcept("SpreadCreep");
			break;
		case Orders::Cloak:
			xcept("Cloak");
			break;
		case Orders::Decloak:
			xcept("Decloak");
			break;
		case Orders::CloakNearbyUnits:
			xcept("CloakNearbyUnits");
			break;
		}
	}

	void update_unit(unit_t* u) {

		if (!ut_turret(u) && !us_hidden(u)) {
			update_selection_sprite(u->sprite, st.selection_circle_color[u->owner]);
		}

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

	}


	bool unit_is_at_move_target(const flingy_t* u) const {
		return u->sprite->position == u->move_target.pos;
	}

	bool unit_dead(const unit_t* u) const {
		return u->order_type->id == Orders::Die && u->order_state == 1;
	}

	bool unit_type_can_fit_at(const unit_type_t* unit_type, xy pos) const {
		if (!is_in_map_bounds(unit_type, pos)) return false;
		if (!is_walkable(pos)) return false;
		std::array<int, 4> inner;
		inner[0] = unit_type->dimensions.from.y;
		inner[1] = -unit_type->dimensions.to.x;
		inner[2] = -unit_type->dimensions.to.y;
		inner[3] = unit_type->dimensions.from.x;

		auto cmp_u = [&](int v, const regions_t::contour& c) {
			return v < c.v[0];
		};
		auto cmp_l = [&](const regions_t::contour& c, int v) {
			return c.v[0] < v;
		};

		auto& c0 = game_st.regions.contours[0];
		for (auto i = std::upper_bound(c0.begin(), c0.end(), pos.y, cmp_u); i != c0.begin();) {
			--i;
			if (inner[0] + i->v[0] < pos.y) break;
			if (inner[1] + i->v[1] <= pos.x && inner[3] + i->v[2] >= pos.x) return false;
		}
		auto& c1 = game_st.regions.contours[1];
		for (auto i = std::lower_bound(c1.begin(), c1.end(), pos.x, cmp_l); i != c1.end(); ++i) {
			if (inner[1] + i->v[0] > pos.x) break;
			if (inner[2] + i->v[1] <= pos.y && inner[0] + i->v[2] >= pos.y) return false;
		}
		auto& c2 = game_st.regions.contours[2];
		for (auto i = std::lower_bound(c2.begin(), c2.end(), pos.y, cmp_l); i != c2.end(); ++i) {
			if (inner[2] + i->v[0] > pos.y) break;
			if (inner[1] + i->v[1] <= pos.x && inner[3] + i->v[2] >= pos.x) return false;
		}
		auto& c3 = game_st.regions.contours[3];
		for (auto i = std::upper_bound(c3.begin(), c3.end(), pos.x, cmp_u); i != c3.begin();) {
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
		rect bounds = unit_bounding_box(u, pos);
		if (!is_in_map_bounds(bounds)) return std::make_pair(true, nullptr);
		unit_t* largest_unit = get_largest_blocking_unit(u, bounds);
		if (!largest_unit) return std::make_pair(!unit_type_can_fit_at(u->unit_type, pos), nullptr);
		return std::make_pair(true, largest_unit);
	}

	void set_flingy_move_target(flingy_t* u, xy target_pos, unit_t* target_unit = nullptr) {
		if (u->move_target.pos == target_pos) return;
		u->move_target.pos = target_pos;
		u->move_target.unit = target_unit;
		u->next_movement_waypoint = target_pos;
		u_unset_movement_flag(u, 4);
		u_set_movement_flag(u, 1);
	}

	xy restrict_move_target_to_valid_bounds(const unit_t* u, xy move_target) {
		return restrict_unit_pos_to_bounds(move_target, u->unit_type, map_bounds() + rect { { 0, 0 }, { 0, -32 } });
	}

	void set_unit_move_target(unit_t* u, xy move_target) {
		if (u->move_target.pos == move_target) return;
		if (u->path) u->path->state_flags |= 1;
		move_target = restrict_move_target_to_valid_bounds(u, move_target);
		set_flingy_move_target(u, move_target);
		if (u_immovable(u)) u_unset_status_flag(u, unit_t::status_flag_immovable);
		u->recent_order_timer = 15;
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
		u->recent_order_timer = 15;
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
			fp8 turn = fp8::extend(u->desired_velocity_direction - u->heading);
			if (turn > u->flingy_turn_rate) turn = u->flingy_turn_rate;
			else if (turn < -u->flingy_turn_rate) turn = -u->flingy_turn_rate;
			u->heading += direction_t::truncate(turn);
			if (u->flingy_type->id >= 0x8d && u->flingy_type->id <= 0xab) {
				u->flingy_turn_rate += fp8::from_raw(1);
			} else if (u->flingy_type->id >= 0xc9 && u->flingy_type->id <= 0xce) {
				u->flingy_turn_rate += fp8::from_raw(1);
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
				if (u->next_speed < fp8::from_raw(192)) {
					if (!u_movement_flag(u, 0x20) && !u_movement_flag(u, 0x10)) {
						u_unset_movement_flag(u, 4);
						set_current_speed(u, fp8::zero());
						return;
					}
				}
			}
		} else if (u->flingy_movement_type == 1) {
			if (unit_is_at_move_target(u)) {
				u_unset_movement_flag(u, 4);
				set_current_speed(u, fp8::zero());
				return;
			}
		} else if (u->flingy_movement_type == 2) {
			if (unit_is_at_move_target(u)) {
				u_unset_movement_flag(u, 4);
				set_current_speed(u, fp8::zero());
			} else {
				if (!u_movement_flag(u, 1)) {
					set_current_speed(u, u->next_speed);
				} else {
					auto heading_error = fp8::extend(u->heading - u->desired_velocity_direction).abs();
					if (heading_error >= fp8::from_raw(32)) {
						if (u_movement_flag(u, 2)) {
							u_unset_movement_flag(u, 2);
							u_unset_movement_flag(u, 4);
							set_current_speed(u, fp8::zero());
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

			unsigned int val = fp8::divide_multiply(diff * 2 + turn_rate - fp8::from_raw(1), turn_rate, u->next_speed).integer_part();
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
		if (speed < fp8::zero()) speed = fp8::zero();
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
		if (!u_movement_flag(u, 2) || u->current_speed == fp8::zero()) {
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
				set_current_speed(u, fp8::zero());
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
			for (auto i = std::lower_bound(c0.begin(), c0.end(), pos.y - inner[0], cmp_l); i != c0.begin();) {
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
			for (auto i = std::upper_bound(c1.begin(), c1.end(), pos.x - inner[1], cmp_u); i != c1.end(); ++i) {
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
			for (auto i = std::upper_bound(c2.begin(), c2.end(), pos.y - inner[2], cmp_u); i != c2.end(); ++i) {
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
			for (auto i = std::lower_bound(c3.begin(), c3.end(), pos.x - inner[3], cmp_l); i != c3.begin();) {
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
			return !is_in_map_bounds(unit_sprite_bounding_box(u));
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
			repulse_dir = target_dir + global_st.repulse_direction_table[direction_index(repulse_dir - target_dir)];
		}
		auto bb = unit_sprite_bounding_box(u);
		if (bb.from.x < 32) {
			if (repulse_dir < direction_t::zero()) repulse_dir = -repulse_dir;
			u->repulse_flags &= 0xf;
		} else if ((size_t)bb.to.x > game_st.map_width - 32) {
			if (repulse_dir > direction_t::zero()) repulse_dir = -repulse_dir;
			u->repulse_flags &= 0xf;
		}
		if (bb.from.y < 32) {
			if (repulse_dir + direction_t::from_raw(64) >= direction_t::zero()) repulse_dir = -repulse_dir;
			u->repulse_flags &= 0xf;
		} else if ((size_t)bb.to.y > game_st.map_height - 32) {
			if (repulse_dir + direction_t::from_raw(64) < direction_t::zero()) repulse_dir = -repulse_dir;
			u->repulse_flags &= 0xf;
		}
		u->repulse_direction = repulse_dir;
	}

	bool apply_repulse_field(unit_t* u, execute_movement_struct& ems) {
		if (!u_can_move(u)) return false;
		if (ut_building(u)) return false;
		if (u->unit_type->id == UnitTypes::Protoss_Interceptor) return false;
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
		else if (repulse.x > fp8::zero()) repulse.x += adjust.x;
		else if (repulse.x < -adjust.x * 4) repulse.x += adjust.x;
		else if (repulse.x < fp8::zero()) repulse.x -= adjust.x;
		if (repulse.y > adjust.y * 4) repulse.y -= adjust.y;
		else if (repulse.y > fp8::zero()) repulse.y += adjust.y;
		else if (repulse.y < -adjust.y * 4) repulse.y += adjust.y;
		else if (repulse.y < fp8::zero()) repulse.y -= adjust.y;
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

		a_deque<const regions_t::region*> long_path;
		size_t full_long_path_size;
		a_deque<xy> short_path;

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
		//log("find long path from %d %d to %d %d, region %d to %d\n", pf.source.x, pf.source.y, pf.destination.x, pf.destination.y, pf.source_region->index, pf.destination_region->index);
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

			//log("find from region %d to region %d\n", from_region->index, to_region->index);

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
								if (estimated_final_cost >= n->estimated_final_cost) xcept("unreachable; cost did not decrease");
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
		if (goal_node) {
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
		}
		for (auto& v : all_nodes) {
			v.region->pathfinder_node = nullptr;
		}
		//log("found long path of %d size\n", pf.full_long_path_size);
		return true;
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
		if (target->unit_type->id == UnitTypes::Special_Upper_Level_Door) return false;
		if (target->unit_type->id == UnitTypes::Special_Right_Upper_Level_Door) return false;
		if (target->unit_type->id == UnitTypes::Special_Pit_Door) return false;
		if (target->unit_type->id == UnitTypes::Special_Right_Pit_Door) return false;
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
			}
			local_edges.push_back(c);
		};

		auto pf_add_local_terrain = [&]() {
			auto cmp_l = [&](const regions_t::contour& c, int v) {
				return c.v[0] < v;
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
			for (auto i = std::lower_bound(c1.begin(), c1.end(), w.cur_pos.x - w.inner[1] + 1, cmp_l); i != c1.end(); ++i) {
				if (i->v[0] > w.cur_pos_max.x - w.inner[1] + 1) break;
				if (i->v[1] + w.inner[i->flags & 3] <= w.cur_pos_max.y) {
					if (i->v[2] + w.inner[(i->flags >> 2) & 3] >= w.cur_pos_min.y) {
						pf_push_back_local_edge(1, *i);
					}
				}
			}
			auto& c2 = game_st.regions.contours[2];
			for (auto i = std::lower_bound(c2.begin(), c2.end(), w.cur_pos.y - w.inner[2] + 1, cmp_l); i != c2.end(); ++i) {
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
			} else if (pos == target) is_goal = true;
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
			if ((size_t)pos.x + 64 >= game_st.map_width) w.cur_pos_max.x = game_st.map_width - 1;
			if ((size_t)pos.y + 64 >= game_st.map_height) w.cur_pos_max.y = game_st.map_height - 1;
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

	xy pathfinder_adjust_target_pos(const pathfinder& pf, xy target) const {
		xy pos = target;
		auto unit_bb = pf.unit_bb;
		for (size_t width = 1; width < 10; width += 2) {
			for (size_t dir = 0; dir != 4; ++dir) {
				for (size_t n = 0; n != width; ++n) {
					auto bb = translate_rect(unit_bb, pos);
					if (is_in_map_bounds(bb)) {
						bool entirely_walkable = true;
						for (size_t y = (size_t)bb.from.y / 32; entirely_walkable && y != (size_t)(bb.to.y + 31) / 32; ++y) {
							for (size_t x = (size_t)bb.from.x / 32; x != (size_t)(bb.to.x + 31) / 32; ++x) {
								size_t index = game_st.regions.tile_region_index.at(y * 256 + x);
								if (index < 0x2000) {
									auto* region = &game_st.regions.regions[index];
									if (!region->walkable()) {
										entirely_walkable = false;
										break;
									}
								}
							}
						}
						if (entirely_walkable) return pos;
					}
					pos += cardinal_direction_xy[dir] * 8;
				}
				pos -= cardinal_direction_xy[dir] * 8;
			}
			pos -= xy(8, 8);
		}
		return target;
	}

	xy pathfinder_adjust_destination(const regions_t::region* source_region, xy destination) const {
		xy target = nearest_pos_in_rect(destination, source_region->area) / 32;
		for (size_t width = 1; width < 16; width += 2) {
			for (size_t dir = 0; dir != 4; ++dir) {
				for (size_t n = 0; n != width; ++n) {
					if ((size_t)target.x < game_st.map_tile_width && (size_t)target.y < game_st.map_tile_height) {
						const regions_t::region* r = nullptr;
						size_t index = game_st.regions.tile_region_index.at((size_t)target.y * 256 + (size_t)target.x);
						if (index >= 0x2000) {
							auto* split = &game_st.regions.split_regions[index - 0x2000];
							r = split->a;
						} else r = &game_st.regions.regions[index];
						if (r == source_region) {
							return target * 32 + xy(16, 16);
						}
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
				for (unit_t* u : find_units(search_bb)) {
					if (is_intersecting(unit_bb, unit_sprite_bounding_box(u))) {
						if (pathfinder_unit_can_collide_with(pf, u)) {
							pf.target_unit = u;
							break;
						}
					}
				}
			}
			if (pf.target_unit) {
				int tid = pf.target_unit->unit_type->id;
				if (tid >= UnitTypes::Special_Zerg_Beacon && tid <= UnitTypes::Special_Protoss_Flag_Beacon) {
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
				xy adjusted_target = pathfinder_adjust_target_pos(pf, target);
				if (adjusted_target != target) {
					target = adjusted_target;
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

	bool pathfinder_find_long_path(pathfinder& pf, const unit_t* u, xy from, xy to) const {
		pf.u = u;
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
		pf.unit_bb = unit_type_bounding_box(pf.u->unit_type);
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
						fp8 halt_distance = unit_halt_distance(move_target_unit);
						xy pos = to_xy(direction_xy(move_target_unit->next_velocity_direction, halt_distance * 3));
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

	bool unit_path_to(unit_t* u, xy to, const unit_t* consider_collision_with_unit = nullptr, bool consider_collision_with_moving_units = false) {
		if (is_moving_along_path(u)) return true;
		if (!u_ground_unit(u)) {
			set_unit_move_target(u, to);
			// Not sure if this can trigger. If it can, then BroodWar has a potential null pointer
			// dereference in UM_FollowPath since from here this function returns true even if
			// path allocation fails.
			// Flying units don't do pathfinding though, so this should be unreachable.
			xcept("unit_path_to called for flying unit");
//			path_t* path = new_path();
//			if (!path) return false;
//			path->delay = 0;
//			// creation_frame is not set here :(
//			path->state_flags = 0;
//			path->long_path = {??};
//			path->full_long_path_size = 1;
//			path->current_long_path_index = 0;
//			path->short_path = {to};
//			path->current_short_path_index = 0;

//			path->source = u->sprite->position;
//			path->destination = to;
			return true;
		}
		if (!path_progress(u, to, consider_collision_with_unit, consider_collision_with_moving_units)) return false;
		if (u->next_target_waypoint != u->path->next) u->next_target_waypoint = u->path->next;
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
		set_next_speed(u, fp8::zero());
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
		slide_free_direction = direction_t::from_raw(-1);
		if (us_hidden(collision_unit)) return false;
		auto target_dir = xy_direction(u->next_movement_waypoint - u->sprite->position);
		auto target_dir_quadrant = direction_index(target_dir) / 64;
		auto dir_err = fp8::extend(target_dir - u->current_velocity_direction).abs();
		if (dir_err >= fp8::from_raw(80)) return false;
		int left_x = collision_unit->unit_finder_bounding_box.from.x - u->unit_type->dimensions.to.x - 1;
		int right_x = collision_unit->unit_finder_bounding_box.to.x + u->unit_type->dimensions.from.x + 1;
		int up_y = collision_unit->unit_finder_bounding_box.from.y - u->unit_type->dimensions.to.y - 1;
		int down_y = collision_unit->unit_finder_bounding_box.to.y + u->unit_type->dimensions.from.y + 1;
		xy target_pos;
		switch (cardinal_direction_from_to(u, collision_unit)) {
		case 0:
			target_pos.y = down_y;
			if (target_dir_quadrant == 3) slide_free_direction = direction_t::from_raw(-64);
			else if (target_dir_quadrant == 0) slide_free_direction = direction_t::from_raw(64);
			break;
		case 1:
			target_pos.x = left_x;
			if (target_dir_quadrant == 0) slide_free_direction = direction_t::from_raw(0);
			else if (target_dir_quadrant == 1) slide_free_direction = direction_t::from_raw(-128);
			break;
		case 2:
			target_pos.y = up_y;
			if (target_dir_quadrant == 1) slide_free_direction = direction_t::from_raw(64);
			else if (target_dir_quadrant == 2) slide_free_direction = direction_t::from_raw(-64);
			break;
		case 3:
			target_pos.x = right_x;
			if (target_dir_quadrant == 2) slide_free_direction = direction_t::from_raw(-128);
			else if (target_dir_quadrant == 3) slide_free_direction = direction_t::from_raw(0);
			break;
		}
		for (int i = 0; i != 2; ++i) {
			if (slide_free_direction == direction_t::from_raw(0)) {
				target_pos.y = up_y;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = direction_t::from_raw(-128);
			} else if (slide_free_direction == direction_t::from_raw(-128)) {
				target_pos.y = down_y;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = direction_t::from_raw(0);
			} else if (slide_free_direction == direction_t::from_raw(64)) {
				target_pos.x = right_x;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = direction_t::from_raw(-64);
			} else if (slide_free_direction == direction_t::from_raw(-64)) {
				target_pos.x = left_x;
				if (!is_blocked(u, target_pos).first) return true;
				slide_free_direction = direction_t::from_raw(64);
			}
		}

		return false;
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
		} else if (u_hidden(u)) {
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
				set_next_speed(u, fp8::zero());
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
			u->next_speed = fp8::zero();
			if (u->current_speed != fp8::zero()) {
				u->current_speed = fp8::zero();
				u->velocity = {};
			}
			if (u->sprite->position != u->next_target_waypoint) {
				u->next_target_waypoint = u->sprite->position;
			}
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
						direction_t dir = u->next_velocity_direction + direction_t::from_raw(16 * lcg_rand(50, -3, 3));
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
							dir = xy_direction(u->move_target.pos - u->sprite->position) + direction_t::from_raw(16 * lcg_rand(50, -1, 1));
							length = fp8::integer(8 + lcg_rand(50, 0, 2) * 4);
							if (!other_unit_is_moving && u_grounded_building(blocking_unit)) length *= 3;
							move_to = u->sprite->position + to_xy(direction_xy(dir, length));
						}
						if (!is_reachable(u->sprite->position, move_to)) {
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
					direction_t dir = xy_direction(to_xy(r->center) - u->sprite->position) + direction_t::from_raw(16 * lcg_rand(50, -2, 2));
					int length = lcg_rand(50, 2, 4) * 8;
					int center_length = xy_length(to_xy(r->center) - u->sprite->position);
					if (length > center_length - 1) length = center_length - 1;
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
						if (u->sprite->position.y - 32 < best_region->area.to.y) maybe_outside = true;
						if (maybe_outside) {
							// bug? seems like it should be best_region->center, not last_center_pos
							dir = xy_direction(last_center_pos - u->sprite->position);
						} else {
							dir = xy_direction(pathfinder_adjust_destination(best_region, u->sprite->position) - u->sprite->position);
						}

						move_to = u->sprite->position + to_xy(direction_xy(dir, 64));

					} else {
						if (!r->non_walkable_neighbors.empty()) {
							size_t random_index = lcg_rand(50, 0, r->non_walkable_neighbors.size() - 1);
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
				if (u->path) {
					free_path(u->path);
					u->path = nullptr;
				}
				if (unit_is_at_move_target(u) || u->movement_flags & 4) {
					set_unit_move_target(u, move_to);
				} else {
					set_next_target_waypoint(u, move_to);
					if (u->next_movement_waypoint != move_to) {
						u->next_movement_waypoint = move_to;
						u_set_movement_flag(u, 1);
					}
				}
				path_t* path = new_path();
				if (path) {
					path->delay = 0;
					// creation_frame is not set here :(
					path->state_flags = 0;

					// long_path[0] is uninitialized here, so we reproduce the uninitialized behavior
					if (path->short_path.size() == 1) {
						path->long_path.resize(1);
					} else {
						size_t index = path->short_path[1].x;
						if (index < game_st.regions.regions.size()) {
							path->long_path = {&game_st.regions.regions[index]};
						} else path->long_path = {nullptr};
					}

					path->full_long_path_size = 1;
					path->current_long_path_index = 0;
					path->short_path = {move_to};
					path->current_short_path_index = 0;

					path->source = u->sprite->position;
					path->destination = move_to;
					path->next = move_to;

					u->path = path;
				}

				u->terrain_no_collision_bounds = {};
				return true;
			} else {
				u->status_flags |= unit_t::status_flag_collision;
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
		if (dir_error != direction_t::from_raw(-128)) {
			if (dir_error >= direction_t::from_raw(-10) && dir_error <= direction_t::from_raw(10)) {
				u_unset_movement_flag(u, 1);
			}
		}
		if (u_status_flag(u, (unit_t::status_flags_t)0x2000000)) {
			set_movement_flags(u, ems);
		} else {
			update_current_velocity_direction_towards_waypoint(u);
			set_movement_flags(u, ems);
			update_unit_heading(u, u->next_velocity_direction);
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
		if (unit_path_to(u, u->move_target.pos)) u->movement_state = movement_states::UM_FollowPath;
		else u->movement_state = movement_states::UM_FailedPath;
		return false;
	}

	bool movement_UM_StartPath(unit_t* u, execute_movement_struct& ems) {
		if (unit_path_to(u, u->move_target.pos)) {
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
		if (!unit_path_to(u, u->move_target.pos)) {
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
			else if (u->velocity.x < fp8::from_raw(32) && u->velocity.y < fp8::from_raw(32)) fix_collision = true;
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
		const unit_t* collision_unit = nullptr;
		if (u->path) {
			collision_unit = get_unit(u->path->last_collision_unit);
			free_path(u->path);
			u->path = nullptr;
		}
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
				direction_t slide_free_direction = direction_t::from_raw(-1);
				xy movement = ems.position - u->sprite->position;
				int desired_quadrant = direction_index(u->current_velocity_direction) / 64;
				if (ems.position.x != u->sprite->position.x && (ems.position.y == u->sprite->position.y || check_unit_movement_terrain_collision(u, xy{movement.x, 0}))) {
					if (movement.x >= 0) {
						if (desired_quadrant == 0) slide_free_direction = direction_t::from_raw(0);
						if (desired_quadrant == 1) slide_free_direction = direction_t::from_raw(-128);
					} else {
						if (desired_quadrant == 3) slide_free_direction = direction_t::from_raw(0);
						if (desired_quadrant == 2) slide_free_direction = direction_t::from_raw(-128);
					}
				} else {
					if (movement.y < 0) {
						if (desired_quadrant == 0) slide_free_direction = direction_t::from_raw(64);
						if (desired_quadrant == 3) slide_free_direction = direction_t::from_raw(-64);
					} else {
						if (desired_quadrant == 1) slide_free_direction = direction_t::from_raw(64);
						if (desired_quadrant == 2) slide_free_direction = direction_t::from_raw(-64);
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
		const unit_t* collision_unit = nullptr;
		if (u->path) {
			collision_unit = get_unit(u->path->last_collision_unit);
			free_path(u->path);
			u->path = nullptr;
		}
		if (unit_is_at_move_target(u) || u->movement_flags & 4) {
			u->movement_state = movement_states::UM_AtMoveTarget;
			return true;
		}
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
		if (unit_path_to(u, u->move_target.pos)) {
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
		if (unit_is_at_move_target(u)) {
			u->movement_state = movement_states::UM_AtMoveTarget;
			return false;
		}
		set_next_speed(u, fp8::zero());
		if (u->path) {
			free_path(u->path);
			u->path = nullptr;
		}
		u->movement_state = movement_states::UM_TurnAndStart;
		return true;
	}

	bool movement_UM_FixCollision(unit_t* u, execute_movement_struct& ems) {
		if (unit_update_path_movement_state(u, false)) return true;
		if (u->pathing_collision_counter < 255) ++u->pathing_collision_counter;
		const unit_t* collision_unit = get_unit(u->path->last_collision_unit);
		if (!collision_unit || us_flag(collision_unit, sprite_t::flag_hidden) || !unit_can_collide_with(u, collision_unit)) {
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
				direction_t cmp_dir = direction_t::zero();
				auto index = direction_index(collision_unit->next_velocity_direction);
				if (index < 32) cmp_dir = direction_t::from_raw(0);
				else if (index < 96) cmp_dir = direction_t::from_raw(64);
				else if (index < 160) cmp_dir = direction_t::from_raw(-128);
				else if (index < 224) cmp_dir = direction_t::from_raw(-64);
				if (fp8::extend(cmp_dir - collision_dir).abs() > fp8::from_raw(64)) {
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
				set_next_speed(u, fp8::zero());
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
			set_next_speed(u, fp8::zero());
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
		for (fp8 speed = original_speed; speed > fp8::zero(); speed -= fp8::integer(1)) {
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
		if (u->path) {
			free_path(u->path);
			u->path = nullptr;
		}
		auto move_target = u->move_target;
		if (!unit_path_to(u, move_target.pos, nullptr, true) || u_immovable(u)) {
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

	bool execute_movement(unit_t* u) {
		execute_movement_struct ems;
		ems.refresh_vision = update_tiles;

		while (true) {
			//log("unit %d at %d %d - movement_state %d\n", u - st.units.data(), u->sprite->position.x, u->sprite->position.y, u->movement_state);
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
				xcept("fixme: movement state %d\n", u->movement_state);
			}
			if (!cont) break;
		}
		//log("post movement - unit %d at %d %d movement_state %d speed %d\n", u - st.units.data(), u->sprite->position.x, u->sprite->position.y, u->movement_state, u->current_speed.raw_value);
		return ems.refresh_vision;
	}

	bool unit_is_morphing_building(const unit_t* u) const {
		if (u_completed(u)) return false;
		unit_type_t* t = u->build_queue[u->build_queue_slot];
		if (!t) return false;
		int tt = t->id;
		return tt == UnitTypes::Zerg_Hive || tt == UnitTypes::Zerg_Lair || tt == UnitTypes::Zerg_Greater_Spire || tt == UnitTypes::Zerg_Spore_Colony || tt == UnitTypes::Zerg_Sunken_Colony;
	}


	int unit_sight_range(const unit_t* u, bool ignore_blindness = false) const {
		if (u_grounded_building(u) && !u_completed(u) && !unit_is_morphing_building(u)) return 4;
		if (!ignore_blindness && u->is_blind) return 2;
		if (u->unit_type->id == UnitTypes::Terran_Ghost && st.upgrade_levels[u->owner][UpgradeTypes::Ocular_Implants]) return 11;
		if (u->unit_type->id == UnitTypes::Zerg_Overlord && st.upgrade_levels[u->owner][UpgradeTypes::Antennae]) return 11;
		if (u->unit_type->id == UnitTypes::Protoss_Observer && st.upgrade_levels[u->owner][UpgradeTypes::Sensor_Array]) return 11;
		if (u->unit_type->id == UnitTypes::Protoss_Scout && st.upgrade_levels[u->owner][UpgradeTypes::Apial_Sensors]) return 11;
		return u->unit_type->sight_range;
	}
	int unit_sight_range_ignore_blindness(const unit_t* u) const {
		return unit_sight_range(u, true);
	}

	int unit_target_acquisition_range(const unit_t* u) const {
		if ((u_cloaked(u) || u_requires_detector(u)) && u->order_type->id != Orders::HoldPosition) {
			if (unit_is_ghost(u)) return 0;
		}
		int bonus = 0;
		if (u->unit_type->id == UnitTypes::Terran_Marine && st.upgrade_levels[u->owner][UpgradeTypes::U_238_Shells]) bonus = 1;
		if (u->unit_type->id == UnitTypes::Zerg_Hydralisk && st.upgrade_levels[u->owner][UpgradeTypes::Grooved_Spines]) bonus = 1;
		if (u->unit_type->id == UnitTypes::Protoss_Dragoon && st.upgrade_levels[u->owner][UpgradeTypes::Singularity_Charge]) bonus = 2;
		if (u->unit_type->id == UnitTypes::Hero_Fenix_Dragoon) bonus = 2;
		if (u->unit_type->id == UnitTypes::Terran_Goliath && st.upgrade_levels[u->owner][UpgradeTypes::Charon_Boosters]) bonus = 3;
		if (u->unit_type->id == UnitTypes::Terran_Goliath_Turret && st.upgrade_levels[u->owner][UpgradeTypes::Charon_Boosters]) bonus = 3;
		if (u->unit_type->id == UnitTypes::Hero_Alan_Schezar) bonus = 3;
		if (u->unit_type->id == UnitTypes::Hero_Alan_Schezar_Turret) bonus = 3;
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
			};
			return UpgradeTypes::None;
		};
		int upg = energy_upgrade();
		if (upg != UpgradeTypes::None && st.upgrade_levels[u->owner][upg]) return fp8::integer(250);
		else return fp8::integer(200);
	}


	bool visible_to_everyone(const unit_t* u) const {
		if (ut_worker(u)) {
			if (u->worker.powerup && u->worker.powerup->unit_type->id == UnitTypes::Powerup_Flag) return true;
			else return false;
		}
		if (!u->unit_type->space_provided) return false;
		if (u->unit_type->id == UnitTypes::Zerg_Overlord && !st.upgrade_levels[u->owner][UpgradeTypes::Ventral_Sacs]) return false;
		if (u_hallucination(u)) return false;
		for (auto idx : u->loaded_units) {
			unit_t* lu = get_unit(idx);
			if (!lu || !lu->sprite) continue;
			if (!ut_worker(lu)) continue;
			if (lu->worker.powerup && lu->worker.powerup->unit_type->id == UnitTypes::Powerup_Flag) return true;
		}
		return false;
	}

	size_t tile_index(xy pos) const {
		size_t ux = (size_t)pos.x / 32;
		size_t uy = (size_t)pos.y / 32;
		if (ux >= game_st.map_tile_width || uy >= game_st.map_tile_height) xcept("attempt to get tile index for invalid position %d %d", pos.x, pos.y);
		return uy * game_st.map_tile_width + ux;
	}

	int get_ground_height_at(xy pos) const {
		size_t index = tile_index(pos);
		tile_id creep_tile = st.gfx_creep_tiles.at(index);
		tile_id tile_id = creep_tile ? creep_tile : game_st.gfx_tiles.at(index);
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
		tile_t reveal_tile_mask;
		reveal_tile_mask.visible = visibility_mask;
		reveal_tile_mask.explored = visibility_mask;
		reveal_tile_mask.flags = 0xffff;
		tile_t required_tile_mask;
		required_tile_mask.visible = ~visibility_mask;
		required_tile_mask.explored = ~visibility_mask;
		required_tile_mask.flags = height_mask;
		auto& sight_vals = game_st.sight_values.at(range);
		size_t tile_x = (size_t)pos.x / 32;
		size_t tile_y = (size_t)pos.y / 32;
		tile_t* base_tile = &st.tiles[tile_x + tile_y*game_st.map_tile_width];
		if (!in_air) {
			auto* cur = sight_vals.maskdat.data();
			auto* end = cur + sight_vals.min_mask_size;
			for (; cur != end; ++cur) {
				cur->vision_propagation = 0xff;
				if (tile_x + cur->x >= game_st.map_tile_width) continue;
				if (tile_y + cur->y >= game_st.map_tile_height) continue;
				auto& tile = base_tile[cur->map_index_offset];
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
				auto& tile = base_tile[cur->map_index_offset];
				tile.raw &= reveal_tile_mask.raw;
				cur->vision_propagation = tile.raw;
			}
		} else {
			// This seems bugged; even for air units, if you only traverse ext_masked_count nodes,
			// then you will still miss out on the min_mask_size (9) last ones
			auto* cur = sight_vals.maskdat.data();
			auto* end = cur + sight_vals.ext_masked_count;
			for (; cur != end; ++cur) {
				if (tile_x + cur->x >= game_st.map_tile_width) continue;
				if (tile_y + cur->y >= game_st.map_tile_height) continue;
				base_tile[cur->map_index_offset].raw &= reveal_tile_mask.raw;
			}
		}
	}

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
		}
		reveal_sight_at(u->sprite->position, unit_sight_range(u), visible_to, u_flying(u));
	}

	void turn_turret(unit_t* tu, direction_t turn) {
		if (tu->order_target.unit) u_unset_status_flag(tu, (unit_t::status_flags_t)0x2000000);
		else {
			if (tu->heading == tu->subunit->heading) u_set_status_flag(tu, (unit_t::status_flags_t)0x2000000);
		}
		if (u_status_flag(tu, (unit_t::status_flags_t)0x2000000)) set_unit_heading(tu, tu->subunit->heading);
		else {
			tu->next_velocity_direction = (tu->next_velocity_direction + turn);
			tu->heading = tu->next_velocity_direction;
		}
		if (tu->unit_type->id == UnitTypes::Terran_Goliath_Turret || tu->unit_type->id == UnitTypes::Hero_Alan_Schezar_Turret) {
			auto diff = tu->subunit->heading - tu->heading;
			if (diff == direction_t::from_raw(-128)) {
				tu->heading = tu->subunit->heading - direction_t::from_raw(96);
			} else if (diff > direction_t::from_raw(32)) {
				tu->heading = tu->subunit->heading - direction_t::from_raw(32);
			} else if (diff < direction_t::from_raw(-32)) {
				tu->heading = tu->subunit->heading + direction_t::from_raw(32);
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
				if (u_status_flag(u->subunit, (unit_t::status_flags_t)0x1000000)) {
					u_unset_status_flag(u->subunit, (unit_t::status_flags_t)0x1000000);
					if (u_can_move(u) && !u_movement_flag(u->subunit, 8)) {
						sprite_run_anim(u->sprite, iscript_anims::WalkingToIdle);
					}
				}
			} else {
				if (!u_status_flag(u->subunit, (unit_t::status_flags_t)0x1000000)) {
					u_set_status_flag(u->subunit, (unit_t::status_flags_t)0x1000000);
					if (u_can_move(u) && !u_movement_flag(u->subunit, 8)) {
						sprite_run_anim(u->sprite, iscript_anims::Walking);
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
		bool was_visible = (u->sprite->visibility_flags & st.local_mask) != 0;
		if (update_thingy_visibility(u, u->unit_type->tile_size)) {
			bool is_visible = (u->sprite->visibility_flags & st.local_mask) != 0;
			if (u->subunit && !us_hidden(u->subunit)) {
				set_sprite_visibility(u->subunit->sprite, u->sprite->visibility_flags);
			}
			if (was_visible && !is_visible) {
				// some selection stuff
				if (u_grounded_building(u) || (u->unit_type->id >= UnitTypes::Special_Floor_Missile_Trap && u->unit_type->id <= UnitTypes::Special_Right_Wall_Flame_Trap)) {
					if (!unit_dead(u)) {
						xcept("fixme create thingy");
					}
				}
			}
		}
	}

	void execute_hidden_unit_main_order(unit_t* u) {
		switch (u->order_type->id) {
		case Orders::Die:
			xcept("hidden Die");
			return;
		case Orders::PlayerGuard:
			xcept("hidden PlayerGuard");
			return;
		case Orders::TurretGuard:
			xcept("hidden TurretGuard");
			return;
		case Orders::UnusedPowerup:
			xcept("hidden UnusedPowerup");
			return;
		case Orders::TurretAttack:
			xcept("hidden TurretAttack");
			return;
		case Orders::Nothing:
			return;
		case Orders::Unused_24:
			return;
		case Orders::InfestingCommandCenter:
			xcept("hidden InfestingCommandCenter");
			return;
		case Orders::HarvestGas:
			xcept("hidden HarvestGas");
			return;
		case Orders::PowerupIdle:
			xcept("hidden PowerupIdle");
			return;
		case Orders::EnterTransport:
			xcept("hidden EnterTransport");
			return;
		case Orders::NukeLaunch:
			xcept("hidden NukeLaunch");
			return;
		case Orders::ResetCollision:
			xcept("hidden ResetCollision");
			return;
		case Orders::ResetHarvestCollision:
			xcept("hidden ResetHarvestCollision");
			return;
		case Orders::Neutral:
			return;
		case Orders::Medic:
			return;
		case Orders::MedicHeal:
			return;
		}
		if (u->order_queue_timer) {
			--u->order_queue_timer;
			return;
		}
		u->order_queue_timer = 8;
		switch (u->order_type->id) {
		case Orders::BunkerGuard:
			xcept("hidden BunkerGuard");
			break;
		case Orders::EnterTransport:
			xcept("hidden EnterTransport");
			break;
		case Orders::ComputerAI:
			xcept("hidden ComputerAI");
			break;
		case Orders::RescuePassive:
			xcept("hidden RescuePassive");
			break;
		}
	}

	void execute_hidden_unit_secondary_order(unit_t* u) {
		switch (u->secondary_order_type->id) {
		case Orders::TrainFighter:
			xcept("hidden TrainFighter");
			break;
		case Orders::Cloak:
			xcept("hidden Cloak");
			break;
		case Orders::Decloak:
			xcept("hidden Decloak");
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
		if (is_disabled(u)) return false;
		if (u->is_blind) return false;
		return true;
	}

	uint32_t unit_calculate_detected_flags(const unit_t* u) const {
		if (u->defensive_matrix_hp || u->lockdown_timer || u->maelstrom_timer || u->irradiate_timer || u->ensnare_timer || u->stasis_timer || u->plague_timer || u->acid_spore_count) {
			return 0xff;
		} else if (visible_to_everyone(u)) {
			return 0xff;
		}

		uint32_t detected_flags = 0;

		detected_flags |= 1 << u->owner;
		detected_flags |= st.shared_vision[u->owner];
		detected_flags |= u->parasite_flags;

		for (size_t i = 0; i < st.shared_vision.size(); ++i) {
			if (u->parasite_flags & (1 << i)) detected_flags |= st.shared_vision[i];
		}

		auto test = [&](const unit_t* detector) {
			if (!unit_can_detect(detector)) return;
			if (detector == u) return;
			if (u_hallucination(detector)) return;
			if (~u->sprite->visibility_flags & (1 << detector->owner)) return;
			int range = u_grounded_building(u) ? 32 * 7 : unit_sight_range(detector);
			if (!unit_target_in_range(detector, u, range)) return;
			detected_flags |= (1 << detector->owner) | st.shared_vision[detector->owner] | detector->parasite_flags;
		};
		xy max_sight(32 * 11, 32 * 11);
		for (const unit_t* nu : find_units({ u->sprite->position - max_sight, u->sprite->position + max_sight })) {
			test(nu);
		}
		for (const unit_t* nu : ptr(st.scanner_sweep_units)) {
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
		} else if (ut_flag(u, (unit_type_t::flags_t)2)) {
			xcept("fixme: verify this flag");
			test(u->building.addon);
		}
		if (unit_is_factory(u)) {
			test(u->rally.unit);
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
		if (b->bullet_target.unit == target) {
			bullet_update_target_pos(b);
			b->bullet_target.unit = nullptr;
		}
		if (b->source_unit == target) b->source_unit = nullptr;
	}

	void update_unit_detected_flags(unit_t* u) {
		uint32_t new_flags = unit_calculate_detected_flags(u);
		if (u->detected_flags == new_flags) return;
		uint32_t old_flags = u->detected_flags;
		if (old_flags == 0x80000000) {
			old_flags = ~new_flags & 0xff;
		}
		uint32_t removed_flags = old_flags & ~new_flags;
		uint32_t changed_flags = old_flags ^ new_flags;
		u->detected_flags = new_flags;
		for (size_t i = 0; removed_flags; ++i) {
			removed_flags &= ~(1 << i);
			for (unit_t* nu : ptr(st.player_units.at(i))) {
				remove_target_references(nu, u);
			}
		}
		if (changed_flags & st.local_mask) {
			if (new_flags & st.local_mask) {
				set_sprite_cloak_visible(u->sprite);
				if (u->subunit) set_sprite_cloak_visible(u->subunit->sprite);
			} else {
				// selection stuff...
				auto hide = [&](sprite_t* sprite) {
					if (s_flag(sprite, sprite_t::flag_burrowed)) {
						set_sprite_cloak_modifier(sprite, true, true, 0, 0);
					} else {
						for (image_t* image : ptr(sprite->images)) {
							if (image->modifier >= 5 && image->modifier <= 7) {
								set_image_modifier(image, image->modifier - 3);
							}
						}
					}
				};
				hide(u->sprite);
				if (u->subunit) hide(u->subunit->sprite);
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
			xcept("update_dead_unit: fixme some creep stuff?");
		}
		if (u->sprite) {
			if (update_tiles && st.players[u->owner].controller == state::player_t::controller_occupied) {
				reveal_sight_at(u->sprite->position, 1, st.shared_vision[u->owner], u_flying(u));
			}
			update_unit_sprite(u);
		} else {
			while (!u->order_queue.empty()) {
				remove_queued_order(u, &u->order_queue.front());
			}
			st.dead_units.remove(*u);
			bw_insert_list(st.free_units, *u);
		}
	}

	void update_units() {

		// place box/target order cursor/whatever

		--st.order_timer_counter;
		if (!st.order_timer_counter) {
			st.order_timer_counter = 150;
			int v = 0;
			for (unit_t* u : ptr(st.visible_units)) {
				u->order_queue_timer = v;
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

		// some_units_loaded_and_disruption_web begin
		for (unit_t* u : ptr(st.visible_units)) {
			if (!u_flying(u) || u_status_flag(u, (unit_t::status_flags_t)0x80)) {
				u_set_status_flag(u, unit_t::status_flag_cannot_attack, false);
				if (!u_hallucination(u) && (u->unit_type->id != UnitTypes::Zerg_Overlord || st.upgrade_levels[u->owner][UpgradeTypes::Ventral_Sacs]) && u->unit_type->space_provided) {
					xcept("sub_4EB2F0 loaded unit stuff");
				} else if (u->subunit) {
					u_set_status_flag(u->subunit, unit_t::status_flag_cannot_attack, false);
				}
			}
		}
		if (st.completed_unit_counts[11][UnitTypes::Spell_Disruption_Web]) {
			xcept("disruption web stuff");
		}
		// some_units_loaded_and_disruption_web end


		for (auto i = st.dead_units.begin(); i != st.dead_units.end();) {
			unit_t* u = &*i++;
			iscript_flingy = u;
			iscript_unit = u;
			update_dead_unit(u);
		}

		for (unit_t* u : ptr(st.visible_units)) {
			iscript_flingy = u;
			iscript_unit = u;
			//log("move unit %d --\n", u - &st.units.front());
			update_unit_movement(u);
		}

		if (update_tiles) {
			for (unit_t* u : ptr(st.scanner_sweep_units)) {
				refresh_unit_vision(u);
			}
		}

		for (unit_t* u : ptr(st.visible_units)) {
			update_unit_sprite(u);
			if (u_cloaked(u) || u_requires_detector(u)) {
				u->is_cloaked = false;
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
			iscript_flingy = u;
			iscript_unit = u;
			update_hidden_unit(u);
		}
		// burrowed/cloaked units
		// update_psi()
		// some lurker stuff

		for (auto i = st.scanner_sweep_units.begin(); i != st.scanner_sweep_units.end();) {
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
				if (!iscript_execute_sprite(b->sprite)) b->sprite = nullptr;
			}
			if (!b->sprite && b->bullet_state != bullet_t::state_dying) xcept("non-dying bullet has null sprite");
			bullet_execute(b);
		}
		iscript_bullet = nullptr;
		iscript_flingy = nullptr;

	}

	void process_frame() {

		allow_random = true;

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

		allow_random = false;

	}

	void next_frame() {
		++st.current_frame;
		process_frame();
	}

	int lcg_rand(int source) {
		if (!allow_random) return 0;
		++st.random_counts[source];
		++st.total_random_counts;
		st.lcg_rand_state = st.lcg_rand_state * 22695477 + 1;
		return (st.lcg_rand_state >> 16) & 0x7fff;
	}
	int lcg_rand(int source, int from, int to) {
		return from + ((lcg_rand(source) * (to - from + 1)) >> 15);
	}

	void net_error_string(int str_index) {
		if (str_index) log(" error %d: (insert string here)\n", str_index);
		st.last_net_error = str_index;
	}

	void local_unit_status_error(unit_t* u, int err) {
		log("if local player, display unit status error %d\n", err);
	}

	size_t get_sprite_tile_line_index(int y) {
		int r = y / 32;
		if (r < 0) return 0;
		if ((size_t)r >= game_st.map_tile_height) return game_st.map_tile_height - 1;
		return (size_t)r;
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
		if ((sprite->visibility_flags & st.local_mask) != (visibility_flags & st.local_mask)) {
			for (image_t* i : ptr(sprite->images)) i->flags |= image_t::flag_redraw;
		}
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

	xy get_image_map_position(const image_t* image) const {
		xy map_pos = image->sprite->position + image->offset;
		auto& frame = image->grp->frames[image->frame_index];
		if (image->flags & image_t::flag_horizontally_flipped) {
			map_pos.x += image->grp->width / 2 - (frame.offset.x + frame.size.x);
		} else {
			map_pos.x += frame.offset.x - image->grp->width / 2;
		}
		if (image->flags & image_t::flag_y_frozen) map_pos.y = map_pos.y;
		else map_pos.y += frame.offset.y - image->grp->height / 2;
		return map_pos;
	}

	xy get_image_lo_offset(const image_t* image, int lo_index, int offset_index) const {
		int frame = image->frame_index;
		auto& lo_offsets = global_st.image_lo_offsets.at(image->image_type->id);
		if ((size_t)lo_index >= lo_offsets.size()) xcept("invalid lo index %d\n", lo_index);
		auto& frame_offsets = *lo_offsets[lo_index];
		if ((size_t)frame >= frame_offsets.size()) xcept("image %d lo_index %d does not have offsets for frame %d (frame_offsets.size() is %d)", image->image_type->id, lo_index, frame, frame_offsets.size());
		if ((size_t)offset_index >= frame_offsets[frame].size()) xcept("image %d lo_index %d frame %d does not contain an offset at index %d", image->image_type->id, lo_index, frame, offset_index);
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
			if (u->unit_type->id == UnitTypes::Protoss_Scout || u->unit_type->id == UnitTypes::Hero_Mojo || u->unit_type->id == UnitTypes::Hero_Artanis) {
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

	fp8 unit_halt_distance(const flingy_t* u) const {
		ufp8 speed = u->next_speed.as_unsigned();
		if (speed == ufp8::zero()) return fp8::zero();
		if (u->flingy_movement_type != 0) return fp8::zero();
		if (speed == u->flingy_type->top_speed.as_unsigned() && u->flingy_acceleration == u->flingy_type->acceleration) {
			return u->flingy_type->halt_distance;
		} else {
			return (ufp8::multiply_divide(speed, speed, u->flingy_acceleration.as_unsigned()) / 2u).as_signed();
		}
	}

	void iscript_set_script(image_t*image, int script_id) {
		auto i = global_st.iscript.scripts.find(script_id);
		if (i == global_st.iscript.scripts.end()) {
			xcept("script %d does not exist", script_id);
		}
		image->iscript_state.current_script = &i->second;
	}

	bool is_spell(const weapon_type_t* weapon_type) {
		int id = weapon_type->id;
		if (id == WeaponTypes::Spider_Mines) return true;
		if (id == WeaponTypes::Lockdown) return true;
		if (id == WeaponTypes::EMP_Shockwave) return true;
		if (id == WeaponTypes::Irradiate) return true;
		if (id == 50) return true;
		if (id == 51) return true;
		if (id == WeaponTypes::Suicide_Infested_Terran) return true;
		if (id == WeaponTypes::Parasite) return true;
		if (id == WeaponTypes::Spawn_Broodlings) return true;
		if (id == WeaponTypes::Ensnare) return true;
		if (id == WeaponTypes::Dark_Swarm) return true;
		if (id == WeaponTypes::Plague) return true;
		if (id == WeaponTypes::Consume) return true;
		if (id == 68) return true;
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

	void bullet_hit(bullet_t* b) {
		switch (b->weapon_type->hit_type) {
		case weapon_type_t::hit_type_none:
			break;
		default: xcept("unknown bullet hit type %d", b->weapon_type->hit_type);
		}
	}

	void bullet_update_target_pos(bullet_t* b) {
		if (!b->bullet_target.unit) return;
		b->bullet_target.pos = b->bullet_target.unit->sprite->position;
	}

	void bullet_kill(bullet_t* b) {
		b->bullet_state = bullet_t::state_dying;
		bullet_update_target_pos(b);
		sprite_run_anim(b->sprite, iscript_anims::Death);
	}

	bool bullet_state_init(bullet_t* b) {
		if (~b->order_signal & 1) return false;
		b->order_signal &= 1;
		switch (b->weapon_type->bullet_type) {
		case weapon_type_t::bullet_type_appear_at_target_unit:
		case weapon_type_t::bullet_type_persist_at_target_pos:
		case weapon_type_t::bullet_type_appear_at_source_unit:
		case weapon_type_t::bullet_type_self_destruct:
			bullet_kill(b);
			break;
		default: xcept("unknown bullet type %d", b->weapon_type->bullet_type);
		}
		return true;
	}

	bool bullet_state_dying(bullet_t* b) {
		if (b->sprite) return false;
		--st.active_bullets_size;
		st.active_bullets.remove(*b);
		bw_insert_list(st.free_bullets, *b);
		return false;
	}

	void bullet_execute(bullet_t* b) {
		while (true) {
			bool cont = false;
			switch (b->bullet_state) {
			case bullet_t::state_init:
				cont = bullet_state_init(b);
				break;
			case bullet_t::state_dying:
				cont = bullet_state_dying(b);
				break;
			default: xcept("unknown bullet state %d", b->bullet_state);
			}
			if (!cont) break;
		}
	}

	bool initialize_bullet(bullet_t* b, const weapon_type_t* weapon_type, unit_t* source_unit, xy pos, int owner, direction_t heading) {
		const flingy_type_t* flingy_type = weapon_type->flingy;
		if (!flingy_type) xcept("attempt to create bullet with null flingy");
		if (!initialize_flingy(b, flingy_type, pos, owner, heading)) return false;

		b->movement_flags |= 8;
		b->bullet_state = 0;
		b->bullet_target = {};
		b->order_signal = 0;
		b->weapon_type = weapon_type;
		b->remaining_time = weapon_type->lifetime;
		b->hit_flags = 0;
		b->remaining_bounces = 0;

		if (weapon_type->bullet_heading_offset != direction_t::zero()) {
			bool clockwise;
			if (source_unit == st.prev_bullet_source_unit) clockwise = !st.prev_bullet_heading_offset_clockwise;
			else clockwise = lcg_rand(weapon_type->id) & 1;
			direction_t heading_offset = weapon_type->bullet_heading_offset;
			if (!clockwise) heading_offset = -heading_offset;
			b->next_velocity_direction += heading_offset;
			b->heading = b->next_velocity_direction;
			st.prev_bullet_source_unit = source_unit;
			st.prev_bullet_heading_offset_clockwise = clockwise;
		}

		b->source_unit = nullptr;
		if (ut_turret(source_unit)) b->source_unit = source_unit->subunit;
		if (source_unit->unit_type->id == UnitTypes::Protoss_Scarab) b->source_unit = source_unit->fighter.parent;
		if (!b->source_unit) b->source_unit = source_unit;

		if (u_hallucination(source_unit)) b->hit_flags |= 2;
		b->prev_bounce_unit = nullptr;

		unit_t* target_unit = source_unit->order_target.unit;
		if (target_unit) {
			b->sprite->elevation_level = target_unit->sprite->elevation_level + 1;
			b->bullet_target.unit = target_unit;
			b->bullet_target.pos = target_unit->sprite->position;
		} else {
			b->sprite->elevation_level = source_unit->sprite->elevation_level + 1;
			b->bullet_target.unit = nullptr;
			b->bullet_target.pos = source_unit->sprite->position;
		}

		switch (weapon_type->bullet_type) {
		case weapon_type_t::bullet_type_appear_at_source_unit:
			break;
		default: xcept("unknown bullet_type %d", weapon_type->bullet_type);
		}

		return true;
	}

	bullet_t* new_bullet() {
		if (!st.free_bullets.empty()) {
			bullet_t* r = &st.free_bullets.front();
			st.free_bullets.pop_front();
			return r;
		}
		if (st.bullets.size() >= 100) return nullptr;
		return &*st.bullets.emplace(st.bullets.end());
	}

	void free_bullet(bullet_t* b) {
		st.free_bullets.push_front(*b);
	}

	bullet_t* create_bullet(const weapon_type_t* weapon_type, unit_t* source_unit, xy pos, int owner, direction_t heading) {
		if (weapon_type->id == WeaponTypes::Halo_Rockets && st.active_bullets_size >= 80) return nullptr;
		if (u_cannot_attack(source_unit) && !is_spell(weapon_type)) return nullptr;
		bullet_t* b = new_bullet();
		if (!b) return nullptr;
		if (!initialize_bullet(b, weapon_type, source_unit, pos, owner, heading)) {
			free_bullet(b);
			return nullptr;
		}
		++st.active_bullets_size;
		bw_insert_list(st.active_bullets, *b);
		return b;
	}

	bool iscript_execute(image_t* image, iscript_state_t& state, bool move_only = false, fp8* distance_moved = nullptr) {

		if (state.wait) {
			--state.wait;
			return true;
		}

		auto play_frame = [&](size_t frame_index) {
			if (image->frame_index_base == frame_index) return;
			image->frame_index_base = frame_index;
			update_image_frame_index(image);
		};

		auto add_image = [&](int image_id, xy offset, int order) {
			//log("add_image %d\n", image_id);
			const image_type_t* image_type = get_image_type(image_id);
			image_t* script_image = image;
			image_t* image = create_image(image_type, script_image->sprite, offset, order, script_image);
			if (!image) return (image_t*)nullptr;

			if (image->modifier == 0 && iscript_unit && u_hallucination(iscript_unit)) {
				if (game_st.is_replay || iscript_unit->owner == game_st.local_player) {
					set_image_modifier(image, image_t::modifier_hallucination);
					image->modifier_data1 = 0;
					image->modifier_data2 = 0;
				}
			}
			if (image->flags & image_t::flag_has_directional_frames) {
				int dir = script_image->flags & image_t::flag_horizontally_flipped ? 32 - script_image->frame_index_offset : script_image->frame_index_offset;
				set_image_heading_by_index(image, dir);
			}
			update_image_frame_index(image);
			if (iscript_unit && (u_burrowed(iscript_unit) || u_hidden(iscript_unit))) {
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

		auto use_weapon = [&](unit_t* u, int weapon_id) {
			const weapon_type_t* weapon_type = get_weapon_type(weapon_id);
			if (!weapon_type->flingy) return;
			create_bullet(weapon_type, u, u->order_target.pos, u->owner, u->heading);
			if (weapon_type->bullet_count == 2) {
				create_bullet(weapon_type, u, u->order_target.pos, u->owner, u->heading);
			}
		};

		const int* program_data = global_st.iscript.program_data.data();
		const int* p = program_data + state.program_counter;
		while (true) {
			using namespace iscript_opcodes;
			size_t pc = p - program_data;
			if (pc == 0) xcept("iscript: program counter is null");
			int opc = *p++ - 0x808091;
			//if (!move_only) log("iscript image %d type %d: %04x: opc %d\n", image - st.images.data(), image->image_type->id, pc, opc);
			int a, b, c;
			switch (opc) {
			case opc_playfram:
				a = *p++;
				if (move_only) break;
				play_frame(a);
				break;
			case opc_playframtile:
				a = *p++;
				if (move_only) break;
				if ((size_t)a + game_st.tileset_index < image->grp->frames.size()) play_frame(a + game_st.tileset_index);
				break;
			case opc_sethorpos:
				a = *p++;
				if (move_only) break;
				if (image->offset.x != a) {
					image->offset.x = a;
					image->flags |= image_t::flag_redraw;
				}
				break;
			case opc_setvertpos:
				a = *p++;
				if (move_only) break;
				if (!iscript_unit || (!u_completed(iscript_unit) && !u_grounded_building(iscript_unit))) {
					if (image->offset.y != a) {
						image->offset.y = a;
						image->flags |= image_t::flag_redraw;
					}
				}
				break;
			case opc_setpos:
				a = *p++;
				b = *p++;
				if (move_only) break;
				set_image_offset(image, xy(a, b));
				break;
			case opc_wait:
				state.wait = *p++ - 1;
				state.program_counter = p - program_data;
				return true;
			case opc_waitrand:
				a = *p++;
				b = *p++;
				if (move_only) break;
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
				if (move_only) break;
				add_image(a, image->offset + xy(b, c), opc == opc_imgol ? image_order_above : image_order_below);
				break;
			case opc_imgolorig:
			case opc_switchul:
				a = *p++;
				if (move_only) break;
				if (image_t* new_image = add_image(a, xy(), opc == opc_imgolorig ? image_order_above : image_order_below)) {
					if (!i_flag(new_image, image_t::flag_uses_special_offset)) {
						i_set_flag(new_image, image_t::flag_uses_special_offset);
						update_image_special_offset(image);
					}
				}
				break;
// 			case opc_imgoluselo:
// 				a = *p++;
// 				b = *p++;
// 				if (move_only) break;
//
// 				break;

			case opc_sprol:
				a = *p++;
				b = *p++;
				c = *p++;
				if (move_only) break;
				xcept("opc_sprol");
				break;

			case opc_spruluselo:
				a = *p++;
				b = *p++;
				c = *p++;
				if (move_only) break;
				xcept("opc_spruluselo");
				break;

			case opc_end:
				if (move_only) break;
				state.program_counter = 0;
				destroy_image(image);
				return false;

			case opc_playsnd:
				a = *p++;
				if (move_only) break;
				xcept("opc_playsnd");
				break;

			case opc_playsndbtwn:
				a = *p++;
				b = *p++;
				if (move_only) break;
				lcg_rand(5);
				// todo: callback for sound?
				break;
			case opc_domissiledmg:
			case opc_dogrddamage:
				if (move_only) break;
				if (iscript_bullet) bullet_hit(iscript_bullet);
				break;

			case opc_followmaingraphic:
				if (move_only) break;
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
				if (move_only) break;
				if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading - direction_t::from_raw(8 * a));
				break;
			case opc_turncwise:
				a = *p++;
				if (move_only) break;
				if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading + direction_t::from_raw(8 * a));
				break;
			case opc_turn1cwise:
				if (move_only) break;
				if (iscript_unit && !iscript_unit->order_target.unit) set_unit_heading(iscript_unit, iscript_unit->heading + direction_t::from_raw(8));
			case opc_turnrand:
				a = *p++;
				if (move_only) break;
				if (lcg_rand(6) % 4 == 1) {
					if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading - direction_t::from_raw(8 * a));
				} else {
					if (iscript_unit) set_unit_heading(iscript_unit, iscript_unit->heading + direction_t::from_raw(8 * a));
				}
				break;

			case opc_sigorder:
				a = *p++;
				if (move_only) break;
				if (iscript_flingy) iscript_flingy->order_signal |= a;
				break;

			case opc_useweapon:
				a = *p++;
				if (move_only) break;
				if (iscript_unit) use_weapon(iscript_unit, a);
				break;
			case opc_move:
				a = *p++;
				if (distance_moved) {
					if (iscript_unit) *distance_moved = get_modified_unit_speed(iscript_unit, fp8::integer(a));
				}
				if (move_only) break;
				if (iscript_unit) set_next_speed(iscript_unit, get_modified_unit_speed(iscript_unit, fp8::integer(a)));
				break;

			case opc_engframe:
				a = *p++;
				if (move_only) break;
				image->frame_index_base = a;
				set_image_frame_index_offset(image, image->sprite->main_image->frame_index_offset, i_flag(image->sprite->main_image, image_t::flag_horizontally_flipped));
				break;
			case opc_engset:
				a = *p++;
				if (move_only) break;
				image->frame_index_base = image->sprite->main_image->frame_index_base + (image->sprite->main_image->grp->frames.size() & 0x7fff) * a;
				set_image_frame_index_offset(image, image->sprite->main_image->frame_index_offset, i_flag(image->sprite->main_image, image_t::flag_horizontally_flipped));
				break;

			case opc_nobrkcodestart:
				if (move_only) break;
				if (iscript_unit) {
					u_set_status_flag(iscript_unit, unit_t::status_flag_iscript_nobrk);
					iscript_unit->sprite->flags |= sprite_t::flag_iscript_nobrk;
				}
				break;
			case opc_nobrkcodeend:
				if (move_only) break;
				if (iscript_unit) {
					u_unset_status_flag(iscript_unit, unit_t::status_flag_iscript_nobrk);
					iscript_unit->sprite->flags &= ~sprite_t::flag_iscript_nobrk;
					if (!iscript_unit->order_queue.empty() && iscript_unit->user_action_flags & 1) {
						iscript_run_to_idle(iscript_unit);
						activate_next_order(iscript_unit);
					}
				}
				break;

			case opc_tmprmgraphicstart:
				if (move_only) break;
				hide_image(image);
				break;
			case opc_tmprmgraphicend:
				if (move_only) break;
				show_image(image);
				break;

			case opc_setfldirect:
				a = *p++;
				if (move_only) break;
				if (iscript_unit) set_unit_heading(iscript_unit, direction_t::from_raw(a * 8));
				break;

			case opc_setflspeed:
				a = *p++;
				if (move_only) break;
				xcept("opc_setflspeed");
				break;

			case opc_call:
				a = *p++;
				state.return_address = p - program_data;
				p = program_data + a;
				break;
			case opc_return:
				p = program_data + state.return_address;
				break;

			case opc_pwrupcondjmp:
				a = *p++;
				if (image->sprite && image->sprite->main_image != image) {
					p = program_data + a;
				}
				break;

			case opc_imgulnextid:
				a = *p++;
				b = *p++;
				if (move_only) break;
				add_image(image->image_type->id + 1, image->offset + xy(a, b), image_order_below);
				break;

			case opc_orderdone:
				a = *p++;
				if (move_only) break;
				if (iscript_flingy) iscript_flingy->order_signal &= ~a;
				break;

			default:
				xcept("iscript: unhandled opcode %d", opc);
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
		if (!script) xcept("attempt to start animation without a script");
		auto& anims_pc = script->animation_pc;
		if ((size_t)new_anim >= anims_pc.size()) xcept("script %d does not have animation %d", script->id, new_anim);
		image->iscript_state.animation = new_anim;
		image->iscript_state.program_counter = anims_pc[new_anim];
		image->iscript_state.return_address = 0;
		image->iscript_state.wait = 0;
		//log("image %d: iscript run anim %d pc %d\n", image - st.images.data(), new_anim, anims_pc[new_anim]);
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
				image->flags |= image_t::flag_redraw;
			}
		}
	}

	void image_update_decloak(image_t* image) {
		xcept("image_update_decloak");
	}

	bool iscript_execute_sprite(sprite_t* sprite) {
		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t* image = &*i++;
			if (image->modifier == 2 || image->modifier == 5) image_update_cloak(image);
			else if (image->modifier == 4 || image->modifier == 7) image_update_decloak(image);
			else if (image->modifier == 17) xcept("iscript_execute_sprite: fixme");
			iscript_execute(image, image->iscript_state);
		}
		if (!sprite->images.empty()) return true;

		remove_sprite_from_tile_line(sprite);
		bw_insert_list(st.free_sprites, *sprite);

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
		image->grp = global_st.image_grp[image_type->id];
		int flags = 0;
		if (image_type->has_directional_frames) flags |= image_t::flag_has_directional_frames;
		if (image_type->is_clickable) flags |= 0x20;
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
		bw_insert_list(st.free_images, *image);
	}

	enum {
		image_order_top,
		image_order_bottom,
		image_order_above,
		image_order_below
	};
	image_t* create_image(int image_id, sprite_t* sprite, xy offset, int order, image_t* relimg = nullptr) {
		if ((size_t)image_id >= 999) xcept("attempt to create image with invalid id %d", image_id);
		return create_image(get_image_type(image_id), sprite, offset, order, relimg);
	}
	image_t* create_image(const image_type_t* image_type, sprite_t* sprite, xy offset, int order, image_t* relimg = nullptr) {
		if (!image_type)  xcept("attempt to create image of null type");
		//log("create image %d\n", image_type->id);

		if (st.free_images.empty()) return nullptr;
		image_t* image = &st.free_images.front();
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
		set_image_modifier(image, image->image_type->modifier);
		if (image->image_type->has_iscript_animations) image->flags |= image_t::flag_has_iscript_animations;
		else image->flags &= image_t::flag_has_iscript_animations;
		iscript_set_script(image, image->image_type->iscript_id);
		if (!iscript_run_anim(image, iscript_anims::Init)) xcept("iscript Init ended immediately (image is no longer valid, cannot continue)");
		return image;
	}

	void destroy_sprite(sprite_t* sprite) {
		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t* image = &*i++;
			destroy_image(image);
		}
		remove_sprite_from_tile_line(sprite);
		bw_insert_list(st.free_sprites, *sprite);
	}

	sprite_t* create_sprite(const sprite_type_t* sprite_type, xy pos, int owner) {
		if (!sprite_type)  xcept("attempt to create sprite of null type");
		//log("create sprite %d\n", sprite_type->id);

		if (st.free_sprites.empty()) return nullptr;
		sprite_t* sprite = &st.free_sprites.front();
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
			sprite->images.clear();
			if (!sprite_type->visible) {
				sprite->flags |= sprite_t::flag_hidden;
				set_sprite_visibility(sprite, 0);
			}
			if (!create_image(sprite_type->image, sprite, { 0,0 }, image_order_above)) return false;
			sprite->width = std::min(sprite->main_image->grp->width, (size_t)0xff);
			sprite->height = std::min(sprite->main_image->grp->width, (size_t)0xff);
			return true;
		};

		if (!initialize_sprite()) {
			bw_insert_list(st.free_sprites, *sprite);
			return nullptr;
		}
		add_sprite_to_tile_line(sprite);

		return sprite;
	}

	bool initialize_flingy(flingy_t* f, const flingy_type_t* flingy_type, xy pos, int owner, direction_t heading) {
		f->flingy_type = flingy_type;
		f->movement_flags = 0;
		f->next_speed = fp8::zero();
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
		f->hp = fp8::from_raw(1);

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
		if (cooldown != u_cooldown_upgrade(u) || speed != u_speed_upgrade(u)) {
			if (cooldown) u->status_flags |= unit_t::status_flag_cooldown_upgrade;
			if (speed) u->status_flags |= unit_t::status_flag_speed_upgrade;
			update_unit_speed(u);
		}
	}

	void update_unit_speed(unit_t* u) {

		if (u->flingy_movement_type == 2) {
			image_t* image = u->sprite->main_image;
			if (!image) xcept("null image");
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
					iscript_execute(image, st, true, &distance_moved);
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
		int supply_required = u->unit_type->supply_required;
		if (u->unit_type->group_flags & GroupFlags::Zerg) {
			const unit_type_t*ut = u->unit_type;
			if (ut->id==UnitTypes::Zerg_Egg || ut->id==UnitTypes::Zerg_Cocoon || ut->id==UnitTypes::Zerg_Lurker_Egg) {
				ut = u->build_queue[u->build_queue_slot];
				supply_required = ut->supply_required;
				if (ut_two_units_in_one_egg(u)) supply_required *= 2;
			} else {
				if (ut_flyer(u) && !u_completed(u)) supply_required *= 2;
			}
			st.supply_used[0][u->owner] += supply_required * count;
		} else if (u->unit_type->group_flags & GroupFlags::Terran) {
			st.supply_used[1][u->owner] += supply_required * count;
		} else if (u->unit_type->group_flags & GroupFlags::Protoss) {
			st.supply_used[2][u->owner] += supply_required * count;
		}
		if (u->unit_type->group_flags & GroupFlags::Factory) st.factory_counts[u->owner] += count;
		if (u->unit_type->group_flags & GroupFlags::Men) {
			st.non_building_counts[u->owner] += count;
		} else if (u->unit_type->group_flags & GroupFlags::Building) st.building_counts[u->owner] += count;
		else if (u->unit_type->id == UnitTypes::Zerg_Egg || u->unit_type->id == UnitTypes::Zerg_Cocoon || u->unit_type->id == UnitTypes::Zerg_Lurker_Egg) {
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
		reinsert(st.unit_finder_x, u->unit_finder_bounding_box.from.x, bb.from.x);
		reinsert(st.unit_finder_x, u->unit_finder_bounding_box.to.x, bb.to.x);
		reinsert(st.unit_finder_y, u->unit_finder_bounding_box.from.y, bb.from.y);
		reinsert(st.unit_finder_y, u->unit_finder_bounding_box.to.y, bb.to.y);
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
				} while (!in_bounds() || i->u->unit_finder_visited);
				i->u->unit_finder_visited = true;
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
		unit_finder_search(const state_functions& funcs, rect area, bool expand) : funcs(funcs), area(area) {
			if (funcs.unit_finder_search_active) xcept("recursive unit_finder_search is not supported");
			funcs.unit_finder_search_active = true;

			auto cmp_l = [&](auto& a, int b) {
				return a.value < b;
			};
			int begin_x = area.from.x;
			int end_x = area.to.x;
			if (expand && end_x - begin_x <= funcs.game_st.max_unit_width) {
				end_x = begin_x + funcs.game_st.max_unit_width + 1;
			}
			i_begin = std::lower_bound(funcs.st.unit_finder_x.begin(), funcs.st.unit_finder_x.end(), begin_x, cmp_l);
			i_end = std::lower_bound(funcs.st.unit_finder_x.begin(), funcs.st.unit_finder_x.end(), end_x, cmp_l);
		}
	public:
		~unit_finder_search() {
			funcs.unit_finder_search_active = false;
			for (auto i = i_begin; i != i_end; ++i) {
				i->u->unit_finder_visited = false;
			}
		}

		iterator begin() {
			auto r = iterator(this, i_begin);
			if (i_begin != i_end) {
				if (r.in_bounds()) r->unit_finder_visited = true;
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

	bool unit_is_factory(unit_t* u) const {
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
	}

	void set_unit_tiles_occupied(const unit_t* u, xy position) {
		xy size(u->unit_type->tile_size.x / 32u, u->unit_type->tile_size.y / 32u);
		xy pos(position.x / 32u, position.y / 32u);
		tiles_flags_or(pos.x - size.x / 2, pos.y - size.y / 2, size.x, size.y, tile_t::flag_occupied);
	}

	void set_unit_tiles_unoccupied(const unit_t* u, xy position) {
		xy size(u->unit_type->tile_size.x / 32u, u->unit_type->tile_size.y / 32u);
		xy pos(position.x / 32u, position.y / 32u);
		tiles_flags_and(pos.x - size.x / 2, pos.y - size.y / 2, size.x, size.y, ~tile_t::flag_occupied);
	}

	bool initialize_unit_type(unit_t* u, const unit_type_t* unit_type, xy pos, int owner) {

		auto ius = make_thingy_setter(iscript_unit, u);
		if (!initialize_flingy(u, unit_type->flingy, pos, owner, direction_t::zero())) return false;

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
			xcept("initialize_unit_type: iscript removed the sprite (if this throws, then Broodwar would crash)");
			u->sprite = nullptr;
		}
		u->last_attacking_player = 8;
		u->shield_points = fp8::integer(u->unit_type->shield_points);
		if (u->unit_type->id == UnitTypes::Protoss_Shield_Battery) u->energy = fp8::integer(100);
		else u->energy = unit_max_energy(u) / 4;

		u->sprite->elevation_level = unit_type->elevation_level;
		u_set_status_flag(u, unit_t::status_flag_grounded_building, ut_building(u));
		u_set_status_flag(u, unit_t::status_flag_flying, ut_flyer(u));
		u_set_status_flag(u, unit_t::status_flag_can_turn, ut_can_turn(u));
		u_set_status_flag(u, unit_t::status_flag_can_move, ut_can_move(u));
		u_set_status_flag(u, unit_t::status_flag_ground_unit, !ut_flyer(u));
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
		if (ut_resource(u)) {
			u->building.resource.resource_count = 0;
			u->building.resource.resource_iscript = 0;
			u->building.resource.is_being_gathered = false;
			u->building.resource.gather_queue.clear();
		} else if (is_mineral_field(unit_type)) xcept("mineral field is not a resource");
		if (unit_is_ghost(u)) {
			u->ghost.nuke_dot = nullptr;
		}

		u->path = nullptr;
		u->movement_state = movement_states::UM_Init;
		u->recent_order_timer = 0;
		u_set_status_flag(u, unit_t::status_flag_invincible, ut_invincible(u));

		u->building_overlay_state = global_st.image_lo_offsets.at(u->sprite->main_image->image_type->id).at(1)->size();

		if (u->unit_type->build_time == 0) {
			u->remaining_build_time = 1;
			u->hp_construction_rate = fp8::integer(1) / 256;
		} else {
			u->remaining_build_time = u->unit_type->build_time;
			u->hp_construction_rate = (u->unit_type->hitpoints - u->unit_type->hitpoints / 10 + fp8::integer(u->unit_type->build_time) / 256 - fp8::integer(1) / 256) / u->unit_type->build_time;
			if (u->hp_construction_rate == fp8::zero()) u->hp_construction_rate = fp8::integer(1) / 256;
		}
		if (u->unit_type->has_shield && u_grounded_building(u)) {
			fp8 max_shields = fp8::integer(u->unit_type->shield_points);
			u->shield_points = max_shields / 10;
			if (u->unit_type->build_time == 0) {
				u->shield_construction_rate = fp8::integer(1);
			} else {
				u->shield_construction_rate = (max_shields - u->shield_points) / u->unit_type->build_time;
				if (u->shield_construction_rate == fp8::zero()) u->shield_construction_rate = fp8::integer(1);
			}
		}
		update_unit_speed_upgrades(u);
		update_unit_speed(u);

		return true;
	}

	void destroy_unit_impl(unit_t* u) {
		if (unit_is_carrier(u) || unit_is_reaver(u)) {
			xcept("fixme: destroy carrier/reaver");
		} else if (unit_is_ghost(u)) {
			if (u->ghost.nuke_dot) {
				sprite_run_anim(u->ghost.nuke_dot, iscript_anims::Death);
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
				xcept("destroy_unit_impl fixme");
			case UnitTypes::Terran_Nuclear_Silo:
				xcept("destroy_unit_impl fixme");
			case UnitTypes::Terran_Nuclear_Missile:
				xcept("destroy_unit_impl fixme");
			case UnitTypes::Protoss_Pylon:
				xcept("destroy_unit_impl fixme");
			case UnitTypes::Zerg_Nydus_Canal:
				xcept("destroy_unit_impl fixme");
			}
		}
		//if (!u->loaded_units.empty()) xcept("fixme kill loaded units");
		for (unit_t* n : ptr(st.visible_units)) {
			remove_target_references(n, u);
		}
		for (unit_t* n : ptr(st.hidden_units)) {
			remove_target_references(n, u);
		}
		for (bullet_t* n : ptr(st.active_bullets)) {
			remove_target_references(n, u);
		}
		if (ut_turret(u)) {
			increment_unit_counts(u, -1);
			u->subunit = nullptr;
			st.player_units[u->owner].remove(*u);
			bw_insert_list(st.dead_units, *u);
		} else {
			int tid = u->unit_type->id;
			if (tid == UnitTypes::Terran_Refinery || tid == UnitTypes::Zerg_Extractor || tid == UnitTypes::Protoss_Assimilator) {
				xcept("fixme destroy refinery");
			} else {
				stop_unit(u);
				if (!us_hidden(u)) {
					unit_finder_remove(u);
					if (u_grounded_building(u)) set_unit_tiles_unoccupied(u, u->sprite->position);
					if (u_flying(u)) decrement_repulse_field(u);
				}
				if (!ut_building(u)) {
					if (u->is_cloaked) xcept("cloaked fixme");
				}
				if (u_grounded_building(u)) {
					if (u->secondary_order_type->id == Orders::BuildAddon) {
						if (u->current_build_unit && !u_completed(u->current_build_unit)) {
							xcept("fixme cancel addon");
						}
					}
					if (u->building.addon) xcept("fixme abandon addon");
					if (u->building.upgrade_type) xcept("fixme refund upgrade");
					if (u->building.tech_type) xcept("fixme refund tech");
					if (u->build_queue.at(u->build_queue_slot)) xcept("fixme refund build_queue");
					if (u->unit_type->group_flags & GroupFlags::Zerg) {
						xcept("fixme cancel zerg stuff");
					}
				}
				if (ut_flying_building(u) && u->building.landing_timer) {
					set_unit_tiles_unoccupied(u, u->order_target.pos);
				}
				if (u->path) {
					free_path(u->path);
					u->path = nullptr;
				}
				if (u->current_build_unit) {
					if (u->secondary_order_type->id == Orders::Train || u->secondary_order_type->id == Orders::TrainFighter) {
						order_SelfDestructing(u->current_build_unit);
					}
				}
				set_secondary_order(u, get_order_type(Orders::Nothing));
				drop_carried_items(u);
				increment_unit_counts(u, -1);
				if (u_completed(u)) add_completed_unit(u, -1, false);
				st.player_units[u->owner].remove(*u);
				if (unit_is_map_revealer(u)) st.scanner_sweep_units.remove(*u);
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
		destroy_sprite(u->sprite);
		u->sprite = nullptr;
	}

	bool initialize_unit(unit_t* u, const unit_type_t* unit_type, xy pos, int owner) {

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
		u->rank_increase = 0;
		u->kill_count = 0;

		u->remove_timer = 0;
		u->defensive_matrix_hp = 0;
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
		u->is_blind = 0;
		u->maelstrom_timer = 0;
		u->unused_0x125 = 0;
		u->acid_spore_count = 0;
		for (auto& v : u->acid_spore_time) v = 0;
		u->status_flags = 0;
		u->user_action_flags = 0;
		u->pathing_flags = 0;
		u->previous_hp = 1;

		if (!initialize_unit_type(u, unit_type, pos, owner)) return false;

		u->build_queue.fill(nullptr);
		u->unit_id_generation = (u->unit_id_generation + 1) % (1 << 5);
		if (!is_disabled(u) || u_completed(u)) {
			if (unit_is_factory(u)) u->current_button_set = UnitTypes::Factories;
			else u->current_button_set = UnitTypes::Buildings;
		}
		u->wireframe_randomizer = lcg_rand(15);
		if (ut_turret(u)) u->hp = fp8::integer(1) / 256;
		else u->hp = u->unit_type->hitpoints / 10;
		if (u_grounded_building(u)) u->order_type = u->unit_type->human_ai_idle;
		else u->order_type = get_order_type(Orders::Nothing);
		// secondary_order_id is uninitialized
		if (!u->secondary_order_type || u->secondary_order_type->id != Orders::Nothing) {
			u->secondary_order_type = get_order_type(Orders::Nothing);
			u->secondary_order_unk_a = 0;
			u->secondary_order_unk_b = 0;
			u->current_build_unit = nullptr;
			u->secondary_order_state = 0;
		}
		u->unit_finder_bounding_box = { {-1, -1}, {-1, -1} };
		st.player_units[owner].push_front(*u);
		increment_unit_counts(u, 1);

		if (u_grounded_building(u)) {
			unit_finder_insert(u);
			set_unit_tiles_occupied(u, u->sprite->position);
			check_unit_collision(u);
			if (u_flying(u)) increment_repulse_field(u); // impossible?
			set_construction_graphic(u, true);
			set_sprite_visibility(u->sprite, 0);
		} else {
			if (unit_type->id==UnitTypes::Terran_Vulture || unit_type->id==UnitTypes::Hero_Jim_Raynor_Vulture) {
				u->vulture.spider_mine_count = 0;
			}
			u->sprite->flags |= sprite_t::flag_hidden;
			set_sprite_visibility(u->sprite, 0);
		}
		u->detected_flags = 0xffffffff;
		if (ut_turret(u)) {
			u->sprite->flags |= 0x10;
		} else {
			if (!us_hidden(u)) {
				refresh_unit_vision(u);
			}
		}

		return true;
	};

	unit_t* create_unit(const unit_type_t* unit_type, xy pos, int owner) {
		if (!unit_type) xcept("attempt to create unit of null type");

		lcg_rand(14);
		auto get_new = [&](const unit_type_t* unit_type) {
			if (st.free_units.empty()) {
				net_error_string(61); // Cannot create more units
				return (unit_t*)nullptr;
			}
			if (!is_in_map_bounds(unit_type, pos)) {
				net_error_string(0);
				return (unit_t*)nullptr;
			}
			unit_t* u = &st.free_units.front();
			if (!initialize_unit(u, unit_type, pos, owner)) {
				net_error_string(62); // Unable to create unit
				return (unit_t*)nullptr;
			}
			st.free_units.pop_front();
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
			if (ut_turret(u)) xcept("unit %d has a turret but is also flagged as a turret", u->unit_type->id);
			if (!ut_turret(su)) xcept("unit %d was created as a turret but is not flagged as one", su->unit_type->id);
		} else {
			u->subunit = nullptr;
		}

		return u;
	}

	unit_t* create_unit(int unit_type_id, xy pos, int owner) {
		if ((size_t)unit_type_id >= 228) xcept("attempt to create unit with invalid id %d", unit_type_id);
		return create_unit(get_unit_type(unit_type_id), pos, owner);
	}

	void replace_sprite_images(sprite_t* sprite, const image_type_t* new_image_type, direction_t heading) {
		// selection stuff...

		for (auto i = sprite->images.begin(); i != sprite->images.end();) {
			image_t* image = &*i++;
			destroy_image(image);
		}

		create_image(new_image_type, sprite, {}, image_order_above);

		// selection stuff...

		for (image_t* img : ptr(sprite->images)) {
			set_image_heading(img, heading);
		}

	}

	void apply_unit_effects(unit_t*u) {
		if (u->defensive_matrix_timer) {
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
		if (!animated || !construction_image) construction_image = u->sprite->sprite_type->image;
		replace_sprite_images(u->sprite, construction_image, u->heading);

		if (requires_detector_or_cloaked) {
			set_sprite_cloak_modifier(u->sprite, u_requires_detector(u), u_cloaked(u), modifier_data1, modifier_data2);
			if (u->detected_flags & st.local_mask) set_sprite_cloak_visible(u->sprite);
		}

		apply_unit_effects(u);
		u->building_overlay_state = global_st.image_lo_offsets.at(u->sprite->main_image->image_type->id).at(1)->size();
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
		set_current_button_set(u, u->unit_type->id);
		if (u_grounded_building(u)) {
			u->parasite_flags = 0;
			u->is_blind = false;
			set_construction_graphic(u, false);
			sprite_run_anim(u->sprite, iscript_anims::Built);
		} else {
			if (u_can_turn(u)) {
				int dir = u->unit_type->unit_direction;
				if (dir == 32) dir = lcg_rand(36) % 32;
				set_unit_heading(u, direction_t::from_raw(dir * 8));
			}
			if (u->unit_type->id >= UnitTypes::Special_Floor_Missile_Trap && u->unit_type->id <= UnitTypes::Special_Right_Wall_Flame_Trap) {
				show_unit(u);
			}
		}

	}

	bool place_initial_unit(unit_t* u) {
		if (us_hidden(u)) {
			// implement me
		}
		return true;
	}

	void add_completed_unit(unit_t* u, int count, bool increment_score) {
		if (u_hallucination(u)) return;
		if (ut_turret(u)) return;

		st.completed_unit_counts[u->owner][u->unit_type->id] += count;
		if (u->unit_type->group_flags & GroupFlags::Zerg) {
			st.supply_available[0][u->owner] += u->unit_type->supply_provided * count;
		} else if (u->unit_type->group_flags & GroupFlags::Terran) {
			st.supply_available[1][u->owner] += u->unit_type->supply_provided * count;
		} else if (u->unit_type->group_flags & GroupFlags::Protoss) {
			st.supply_available[2][u->owner] += u->unit_type->supply_provided * count;
		}

		if (u->unit_type->group_flags & GroupFlags::Factory) {
			st.completed_factory_counts[u->owner] += count;
		}
		if (u->unit_type->group_flags & GroupFlags::Men) {
			st.completed_building_counts[u->owner] += count;
		} else if (u->unit_type->group_flags & GroupFlags::Building) {
			st.completed_building_counts[u->owner] += count;
		}
		if (increment_score) {
			if (u->owner != 11) {
				if (u->unit_type->group_flags & GroupFlags::Men) {
					bool morphed = false;
					if (u->unit_type->id == UnitTypes::Zerg_Guardian) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Devourer) morphed = true;
					if (u->unit_type->id == UnitTypes::Protoss_Dark_Archon) morphed = true;
					if (u->unit_type->id == UnitTypes::Protoss_Archon) morphed = true;
					if (u->unit_type->id == UnitTypes::Zerg_Lurker) morphed = true;
					if (!morphed) st.total_non_buildings_ever_completed[u->owner] += count;
					st.unit_score[u->owner] += u->unit_type->build_score * count;
				} else if (u->unit_type->group_flags & GroupFlags::Building) {
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

	order_t* create_order(const order_type_t* order_type, order_target_t target) {
		if (st.free_orders.empty()) return (order_t*)nullptr;
		order_t* o = &st.free_orders.front();
		st.free_orders.pop_front();
		++st.allocated_order_count;
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
			xcept("cloak fixme");
		} else {
			queue_order_back(u, order_type, target);
		}
	}

	void iscript_run_to_idle(unit_t* u) {
		u->status_flags &= ~unit_t::status_flag_iscript_nobrk;
		u->sprite->flags &= ~sprite_t::flag_iscript_nobrk;
		auto ius = make_thingy_setter(iscript_unit, u);
		auto ifs = make_thingy_setter(iscript_flingy, u);
		int anim = -1;
		int sid = u->sprite->sprite_type->id;
		switch (u->sprite->main_image->iscript_state.animation) {
		case iscript_anims::AirAttkInit:
		case iscript_anims::AirAttkRpt:
			anim = iscript_anims::AirAttkToIdle;
			break;
		case iscript_anims::AlmostBuilt:
			if (sid == idenums::SPRITEID_SCV || sid == idenums::SPRITEID_Drone || sid == idenums::SPRITEID_Probe) {
				anim = iscript_anims::GndAttkToIdle;
			}
			break;
		case iscript_anims::GndAttkInit:
		case iscript_anims::GndAttkRpt:
			anim = iscript_anims::GndAttkToIdle;
			break;
		case iscript_anims::SpecialState1:
			if (sid == idenums::SPRITEID_Medic) anim = iscript_anims::WalkingToIdle;
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
		u_unset_status_flag(u, unit_t::status_flag_holding_position);
		u_set_status_flag(u, unit_t::status_flag_order_not_interruptible, !order_type->can_be_interrupted);
		u->order_queue_timer = 0;
		u->recent_order_timer = 0;

		u->order_type = order_type;
		u->order_state = 0;

		if (target.unit) {
			if (unit_dead(target.unit) || !target.unit->sprite) xcept("attempt to activate order with dead target");
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
		for (unit_t* nu : find_units(unit_sprite_bounding_box(u))) {
			if (u_grounded_building(nu)) u->status_flags |= unit_t::status_flag_collision;
			else if (!u_flying(nu) && (!u_gathering(nu) || u_grounded_building(u))) {
				if (unit_finder_units_intersecting(u, nu)) {
					nu->status_flags |= unit_t::status_flag_collision;
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
		if (u->unit_type->id == UnitTypes::Protoss_Interceptor) return;
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
		if (u->unit_type->id == UnitTypes::Protoss_Interceptor) return;
		auto& v = st.repulse_field.at(u->repulse_index);
		if (v > 0) --v;
	}

	void show_unit(unit_t* u) {
		if (!us_hidden(u)) return;
		u->sprite->flags &= ~sprite_t::flag_hidden;
		if (u->subunit && !ut_turret(u)) u->subunit->sprite->flags &= ~sprite_t::flag_hidden;
		refresh_unit_vision(u);
		update_unit_sprite(u);
		unit_finder_insert(u);
		if (u_grounded_building(u)) {
			xcept("update tiles (mask in flag_occupied)");
		}
		check_unit_collision(u);
		if (u_flying(u)) {
			increment_repulse_field(u);
		}
		if (u->path) {
			free_path(u->path);
			u->path = nullptr;
		}

		u->movement_state = movement_states::UM_Init;
		if (u->sprite->elevation_level < 12) u->pathing_flags |= 1;
		else u->pathing_flags &= ~1;
		if (u->subunit && !ut_turret(u)) {
			if (u->subunit->path) {
				free_path(u->subunit->path);
				u->subunit->path = nullptr;
			}
			u->subunit->movement_state = movement_states::UM_Init;
			if (u->subunit->sprite->elevation_level < 12) u->subunit->pathing_flags |= 1;
			else u->subunit->pathing_flags &= ~1;
		}
		st.hidden_units.remove(*u);
		bw_insert_list(st.visible_units, *u);
	}

	void hide_unit(unit_t* u) {
		if (us_hidden(u)) return;
		xcept("hide_unit");
		for (unit_t* n : ptr(st.visible_units)) {
			remove_target_references(n, u);
		}
		for (bullet_t* n : ptr(st.active_bullets)) {
			remove_target_references(n, u);
		}
		unit_finder_remove(u);
		if (u_grounded_building(u)) set_unit_tiles_unoccupied(u, u->sprite->position);
		if (u_flying(u)) decrement_repulse_field(u);
		if (u->path) {
			free_path(u->path);
			u->path = nullptr;
		}
		u->movement_state = movement_states::UM_Init;
		if (u->sprite->elevation_level < 12) u->pathing_flags |= 1;
		else u->pathing_flags &= ~1;
		st.visible_units.remove(*u);
		bw_insert_list(st.hidden_units, *u);
		u_set_status_flag(u, unit_t::status_flag_hidden);
		set_sprite_visibility(u->sprite, 0);
		unit_t* turret = unit_turret(u);
		if (turret) {
			u_set_status_flag(turret, unit_t::status_flag_hidden);
			set_sprite_visibility(turret->sprite, 0);
		}
	}

	void set_sprite_cloak_modifier(sprite_t* sprite, bool requires_detector, bool cloaked, int data1, int data2) {
		if (requires_detector && !cloaked) {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier == 0) {
					image->modifier = 2;
					image->modifier_data1 = data1;
					image->modifier_data2 = data2;
					image->flags |= image_t::flag_redraw;
				}
			}
		} else if (!requires_detector && cloaked) {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier == 0) {
					image->modifier = 4;
					image->modifier_data1 = data1;
					image->modifier_data2 = data2;
					image->flags |= image_t::flag_redraw;
				}
			}
		} else {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier == 0 || image->modifier == 2 || image->modifier == 4) {
					image->modifier = 3;
					image->flags |= image_t::flag_redraw;
				}
			}
		}
	}

	void set_sprite_cloak_visible(sprite_t* sprite) {
		if (s_flag(sprite, sprite_t::flag_burrowed)) {
			xcept("set_sprite_cloak_visible fixme");
		} else {
			for (image_t* image : ptr(sprite->images)) {
				if (image->modifier == 0) {
					image->modifier = 6;
					image->flags |= image_t::flag_redraw;
				} else if (image->modifier >= 2 && image->modifier <= 4) {
					set_image_modifier(image, image->modifier + 3);
				}
			}
		}
	}

	void cloak_unit(unit_t* u) {
		if (u_burrowed(u)) {
			xcept("burrowed");
		} else {
			auto cloak = [&](unit_t* u) {
				u_unset_status_flag(u, unit_t::status_flag_cloaked);
				u_set_status_flag(u, unit_t::status_flag_requires_detector);
				for (image_t* image : ptr(u->sprite->images)) {
					if (image->modifier == 0 || image->modifier == 4 || image->modifier == 7) {
						image->modifier = 2;
						image->modifier_data1 = 0;
						image->modifier_data2 = 3;
						image->flags |= image_t::flag_redraw;
					}
				}
				u->detected_flags = 0x80000000;
			};
			cloak(u);
			if (u->subunit) cloak(u->subunit);
			update_unit_detected_flags(u);
			u->secondary_order_timer = 30;
		}
	}

	void complete_unit(unit_t* u) {

		if (ut_flyer(u)) {
			increment_unit_counts(u, -1);
			u->status_flags |= unit_t::status_flag_completed;
			increment_unit_counts(u, 1);
		} else {
			u->status_flags |= unit_t::status_flag_completed;
		}
		add_completed_unit(u, 1, true);
		if (ut_initially_cloaked(u)) cloak_unit(u);
		if (u->unit_type->id == UnitTypes::Spell_Scanner_Sweep || u->unit_type->id == UnitTypes::Special_Map_Revealer) {
			xcept("fixme scanner/map revealer");
		} else {
			if (us_hidden(u)) {
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
			u->detected_flags = 0x80000000;
			u->secondary_order_timer = 0;
		}
		if (st.players[u->owner].controller == state::player_t::controller_rescue_passive) {
			xcept("fixme rescue passive");
		} else {
			if (st.players[u->owner].controller == state::player_t::controller_neutral) set_unit_order(u, get_order_type(Orders::Neutral));
			else if (st.players[u->owner].controller == state::player_t::controller_computer_game) set_unit_order(u, u->unit_type->computer_ai_idle);
			else set_unit_order(u, u->unit_type->human_ai_idle);
		}
		if (ut_flag(u, (unit_type_t::flags_t)0x800)) {
			xcept("fixme unknown flag");
		}
		u->air_strength = get_unit_strength(u, false);
		u->ground_strength = get_unit_strength(u, true);
	}

	unit_t* create_initial_unit(const unit_type_t* unit_type, xy pos, int owner) {
		unit_t* u = create_unit(unit_type, pos, owner);
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

		complete_unit(u);

		return u;
	}

	void display_last_net_error_for_player(int player) {
		log("fixme: display last error (%d)\n", st.last_net_error);
	}

};

struct game_load_functions : state_functions {

	explicit game_load_functions(state& st) : state_functions(st) {}

	game_state& game_st = *st.game;

	unit_type_t* get_unit_type(int id) const {
		if ((size_t)id >= 228) xcept("invalid unit id %d", id);
		return &game_st.unit_types.vec[id];
	}
	const weapon_type_t* get_weapon_type(int id) const {
		if ((size_t)id >= 130) xcept("invalid weapon id %d", id);
		return &game_st.weapon_types.vec[id];
	}
	upgrade_type_t* get_upgrade_type(int id) const {
		if ((size_t)id >= 61) xcept("invalid upgrade id %d", id);
		return &game_st.upgrade_types.vec[id];
	}
	tech_type_t* get_tech_type(int id) const {
		if ((size_t)id >= 44) xcept("invalid tech id %d", id);
		return &game_st.tech_types.vec[id];
	}
	const flingy_type_t* get_flingy_type(int id) const {
		if ((size_t)id >= 209) xcept("invalid flingy id %d", id);
		return &global_st.flingy_types.vec[id];
	}

	void reset() {

		game_st.unit_types = data_loading::load_units_dat(global_st.units_dat);
		game_st.weapon_types = data_loading::load_weapons_dat(global_st.weapons_dat);
		game_st.upgrade_types = data_loading::load_upgrades_dat(global_st.upgrades_dat);
		game_st.tech_types = data_loading::load_techdata_dat(global_st.techdata_dat);

		auto fixup_unit_type = [&](unit_type_t*& ptr) {
			size_t index = (size_t)ptr;
			if (index == 228) ptr = nullptr;
			else ptr = get_unit_type(index);
		};
		auto fixup_weapon_type = [&](const weapon_type_t*& ptr) {
			size_t index = (size_t)ptr;
			if (index == 130) ptr = nullptr;
			else ptr = get_weapon_type(index);
		};
		auto fixup_upgrade_type = [&](upgrade_type_t*& ptr) {
			size_t index = (size_t)ptr;
			if (index == 61) ptr = nullptr;
			else ptr = get_upgrade_type(index);
		};
		auto fixup_flingy_type = [&](const flingy_type_t*& ptr) {
			size_t index = (size_t)ptr;
			ptr = get_flingy_type(index);
		};
		auto fixup_order_type = [&](const order_type_t*& ptr) {
			size_t index = (size_t)ptr;
			ptr = get_order_type(index);
		};
		auto fixup_image_type = [&](const image_type_t*& ptr) {
			size_t index = (size_t)ptr;
			if (index == 999) ptr = nullptr;
			else ptr = get_image_type(index);
		};

		for (auto& v : game_st.unit_types.vec) {
			fixup_flingy_type(v.flingy);
			fixup_unit_type(v.turret_unit_type);
			fixup_unit_type(v.subunit2);
			fixup_image_type(v.construction_animation);
			fixup_weapon_type(v.ground_weapon);
			fixup_weapon_type(v.air_weapon);
			fixup_upgrade_type(v.armor_upgrade);
			fixup_order_type(v.computer_ai_idle);
			fixup_order_type(v.human_ai_idle);
			fixup_order_type(v.return_to_idle);
			fixup_order_type(v.attack_unit);
			fixup_order_type(v.attack_move);
		}
		for (auto& v : game_st.weapon_types.vec) {
			fixup_flingy_type(v.flingy);
			fixup_upgrade_type(v.damage_upgrade);
		}

		for (auto& v : game_st.unit_type_allowed) v.fill(true);
		for (auto& v : game_st.tech_available) v.fill(true);
		st.tech_researched = {};
		for (auto& v : game_st.max_upgrade_levels) {
			for (size_t i = 0; i < game_st.max_upgrade_levels.size(); ++i) {
				v[i] = get_upgrade_type(i)->max_level;
			}
		}
		st.upgrade_levels.fill({});
		// upgrade progress?
		// UPRP stuff?

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
			for (int i = 0; i < 228; ++i) {
				unit_type_t* unit_type = get_unit_type(i);
				const unit_type_t* attacking_type = unit_type->turret_unit_type ? unit_type->turret_unit_type : unit_type;
				const weapon_type_t* ground_weapon = attacking_type->ground_weapon;
				const weapon_type_t* air_weapon = attacking_type->air_weapon;
				int acq_range = attacking_type->target_acquisition_range;
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
		for (auto& v : st.tiles) {
			v.visible = 0xff;
			v.explored = 0xff;
		}
		st.tiles_mega_tile_index.clear();
		st.tiles_mega_tile_index.resize(st.tiles.size());

		st.gfx_creep_tiles.clear();
		st.gfx_creep_tiles.resize(game_st.map_tile_width * game_st.map_tile_height);

		st.update_tiles_countdown = 1;

		st.order_timer_counter = 10;
		st.secondary_order_timer_counter = 150;
		st.current_frame = 0;

		st.visible_units.clear();
		st.hidden_units.clear();
		st.scanner_sweep_units.clear();
		st.dead_units.clear();
		for (auto& v : st.player_units) v.clear();

		auto clear_and_make_free = [&](auto& list, auto& free_list) {
			free_list.clear();
			memset(list.data(), 0, (char*)(list.data() + list.size()) - (char*)list.data());
			for (auto& v: list) {
				bw_insert_list(free_list, v);
			}
		};

		clear_and_make_free(st.units, st.free_units);
		st.active_bullets_size = 0;
		st.active_bullets.clear();
		st.bullets.clear();
		st.free_bullets.clear();
		clear_and_make_free(st.sprites, st.free_sprites);
		for (size_t i = 0; i != 2500; ++i) st.sprites[i].index = i;
		st.sprites_on_tile_line.clear();
		st.sprites_on_tile_line.resize(game_st.map_tile_height);
		clear_and_make_free(st.images, st.free_images);
		clear_and_make_free(st.orders, st.free_orders);
		st.allocated_order_count = 0;

		st.last_net_error = 0;

		game_st.is_replay = false;
		game_st.local_player = 0;

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

		st.prev_bullet_source_unit = nullptr;

		st.current_minerals = {};
		st.current_gas = {};
		st.total_minerals_gathered = {};
		st.total_gas_gathered = {};

	}

	regions_t::region* get_new_region() {
		if (game_st.regions.regions.capacity() != 5000) game_st.regions.regions.reserve(5000);
		if (game_st.regions.regions.size() >= 5000) xcept("too many regions");
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
		auto is_dir_walkable = [&](size_t walk_x, size_t walk_y, int dir) {
			return ~unwalkable_flags[walk_y * 256 * 4 + walk_x] & (1 << dir) ? true : false;
		};
		auto is_dir_unwalkable = [&](size_t walk_x, size_t walk_y, int dir) {
			return unwalkable_flags[walk_y * 256 * 4 + walk_x] & (1 << dir) ? true : false;
		};
		auto set_dir_unwalkable = [&](size_t walk_x, size_t walk_y, int dir) {
			unwalkable_flags[walk_y * 256 * 4 + walk_x] |= 1 << dir;
		};
		auto flip_dir_walkable = [&](size_t walk_x, size_t walk_y, int dir) {
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

			if (game_st.map_walk_width == 0 || game_st.map_walk_height == 0) xcept("map width/height is zero");

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
			if (flags < 5000) xcept("attempt to create region inside another region");
			r->flags = flags;
			r->tile_area = area;
			r->tile_center.x = (area.from.x + area.to.x) / 2;
			r->tile_center.y = (area.from.y + area.to.y) / 2;
			size_t tile_count = 0;
			size_t index = r->index;
			for (size_t y = area.from.y; y != area.to.y; ++y) {
				for (size_t x = area.from.x; x != area.to.x; ++x) {
					if (game_st.regions.tile_region_index[y * 256 + x] < 5000) xcept("attempt to create overlapping region");
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
						//log("found region at %d %d index %x\n", x, y, index);
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

			int prev_size = 7 * 8;

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

					int size = (area.to.x - area.from.x) * (area.to.y - area.from.y);
					if (size < prev_size) {
						auto best_area = area;
						int best_size = size;

						for (size_t n = 0; n != 25; ++n) {
							if (!find_empty_region(area.to.x, region_y)) break;
							area = find_area(region_x, region_y, region_tile_index);
							int size = (area.to.x - area.from.x) * (area.to.y - area.from.y);
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

					if (game_st.regions.regions.size() >= 5000) xcept("too many regions (nooks and crannies)");

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
					if (game_st.regions.regions.size() >= 5000) xcept("too many regions (nooks and crannies)");
					//log("created %d regions\n", game_st.regions.regions.size());
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
							if (r->area.from.x >(int)x * 32) r->area.from.x = x * 32;
							if (r->area.from.y > (int)y * 32) r->area.from.y = y * 32;
							if (r->area.to.x < ((int)x + 1) * 32) r->area.to.x = (x + 1) * 32;
							if (r->area.to.y < ((int)y + 1) * 32) r->area.to.y = (y + 1) * 32;
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
							for (size_t i = 0; i < 8; ++i) {
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
				//log("n_non_empty_regions is %d\n", n_non_empty_regions);
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
				r->index = new_index;
				game_st.regions.regions[new_index] = std::move(*r);
			}
			for (size_t y = 0; y != game_st.map_tile_height; ++y) {
				for (size_t x = 0; x != game_st.map_tile_height; ++x) {
					size_t& index = game_st.regions.tile_region_index[y * 256 + x];
					index = reindex[index];
				}
			}
			game_st.regions.regions.resize(new_region_count);

			//log("new_region_count is %d\n", new_region_count);

			refresh_regions();

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
					if (!r2) mask = r->walkable() ? 0 : 0xffff;
					else if (!r->walkable() && !r2->walkable()) mask = 0xffff;
					game_st.regions.tile_region_index[y * 256 + x] = 0x2000 + game_st.regions.split_regions.size();
					if (r->walkable()) {
						game_st.regions.split_regions.push_back({ mask, r, r2 ? r2 : r });
					} else {
						game_st.regions.split_regions.push_back({ mask, r2 ? r2 : r, r });
					}
				}
			}
			//log("created %d split regions\n", game_st.regions.split_regions.size());

			for (auto* r : ptr(game_st.regions.regions)) {
				r->priority = 0;
				static_vector<regions_t::region*, 5> nvec;
				for (auto* nr : r->non_walkable_neighbors) {
					if (nr->tile_count >= 4) {
						if (std::find(nvec.begin(), nvec.end(), nr) == nvec.end()) {
							nvec.push_back(nr);
							if (nvec.size() >= 5) break;
						}
					}
				}
				if (nvec.size() >= 2) r->priority = nvec.size();
			}

		};

		auto create_contours = [&]() {

			size_t next_x = 0;
			size_t next_y = 0;
			auto next = [&]() {
				size_t x = next_x;
				size_t y = next_y;
				if (x >= game_st.map_walk_width) {
					xcept("create_contours::next: unreachable");
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
					if (x == game_st.map_walk_width) xcept("create_contours: out of bounds");
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
				int start_cx = clut[clut_index * 2] + x * 8;
				int start_cy = clut[clut_index * 2 + 1] + y * 8;

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

					int nx = nlut[nlut_index * 2] + x * 8;
					int ny = nlut[nlut_index * 2 + 1] + y * 8;

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
				auto& t = st.tiles[y * game_st.map_tile_height + x];
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
		auto sqrt_x_times_7_58 = [&](int x) {
			if (x <= 0) return 0;
			int value = 1;
			while (true) {
				int f_eval = value * value;
				int f_derivative = 2 * value;
				int delta = (f_eval - x + f_derivative - 1) / f_derivative;
				if (delta == 0) break;
				while (std::numeric_limits<int>::max() / (value - delta) < (value - delta)) {
					delta /= 2;
				}
				value -= delta;
			}
			value = value * 758 / 100;
			size_t n = 8;
			while (n > 0) {
				int nv = value + n / 2 + 1;
				int r = (int)(((int64_t)nv * nv * 10000) / (758 * 758));
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
	};

	void calculate_unit_strengths() {

		for (int idx = 0; idx < 228; ++idx) {

			const unit_type_t* unit_type = get_unit_type(idx);
			const unit_type_t* attacking_type = unit_type;
			int air_strength = 0;
			int ground_strength = 0;
			if (attacking_type->id != UnitTypes::Zerg_Larva && attacking_type->id != UnitTypes::Zerg_Egg && attacking_type->id != UnitTypes::Zerg_Cocoon && attacking_type->id != UnitTypes::Zerg_Lurker_Egg) {
				if (attacking_type->id == UnitTypes::Protoss_Carrier || attacking_type->id == UnitTypes::Hero_Gantrithor) attacking_type = get_unit_type(UnitTypes::Protoss_Interceptor);
				else if (attacking_type->id == UnitTypes::Protoss_Reaver || attacking_type->id == UnitTypes::Hero_Warbringer) attacking_type = get_unit_type(UnitTypes::Protoss_Scarab);
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

			//log("strengths for %d is %d %d\n", idx, air_strength, ground_strength);

			game_st.unit_air_strength[idx] = air_strength;
			game_st.unit_ground_strength[idx] = ground_strength;

		}

	}

	void generate_sight_values() {
		for (size_t i = 0; i < game_st.sight_values.size(); ++i) {
			auto& v = game_st.sight_values[i];
			v.max_width = 3 + (int)i * 2;
			v.max_height = 3 + (int)i * 2;
			v.min_width = 3;
			v.min_height = 3;
			v.min_mask_size = 0;
			v.ext_masked_count = 0;
		}

		for (auto& v : game_st.sight_values) {
			struct base_mask_t {
				sight_values_t::maskdat_node_t* maskdat_node;
				bool masked;
			};
			a_vector<base_mask_t> base_mask(v.max_width*v.max_height);
			auto mask = [&](size_t index) {
				if (index >= base_mask.size()) xcept("attempt to mask invalid base mask index %d (size %d)", index, base_mask.size());
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
//			log("%d %d - masked_count is %d\n", v.max_width, v.max_height, masked_count);

			v.ext_masked_count = masked_count - v.min_mask_size;
			v.maskdat.clear();
			v.maskdat.resize(masked_count);

			auto* center = &base_mask[v.max_height / 2 * v.max_width + v.max_width / 2];
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
									auto* this_entry = &v.maskdat.at(next_entry_index++);

									int prev_x = this_x;
									int prev_y = this_y;
									if (prev_x > 0) --prev_x;
									else if (prev_x < 0) ++prev_x;
									if (prev_y > 0) --prev_y;
									else if (prev_y < 0) ++prev_y;
									if (std::abs(prev_x) == std::abs(prev_y) || (this_x == 0 && direction_x[dir]) || (this_y == 0 && direction_y[dir])) {
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

	}

	void load_tile_stuff() {

		auto set_mega_tile_flags = [&]() {
			game_st.mega_tile_flags.resize(game_st.vf4.size());
			for (size_t i = 0; i < game_st.mega_tile_flags.size(); ++i) {
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

	void load_map_file(const a_string& filename) {

		// campaign stuff? see load_map_file

		//log("load map file '%s'\n", filename);

		a_vector<uint8_t> data;
		data_loading::load_data_file(data, filename, "staredit\\scenario.chk");

		using data_loading::data_reader_le;

		a_unordered_map<tag_t, std::function<void(data_reader_le)>, tag_t> tag_funcs;

		auto tagstr = [&](tag_t tag) {
			return a_string(tag.data.data(), 4);
		};

		using tag_list_t = a_vector<std::pair<tag_t, bool>>;
		auto read_chunks = [&](const tag_list_t&tags) {
			data_reader_le r(data.data(), data.data() + data.size());
			a_unordered_map<tag_t, data_reader_le, tag_t> chunks;
			while (r.left()) {
				tag_t tag = r.get<std::array<char, 4>>();
				uint32_t len = r.get<uint32_t>();
				//log("tag '%.4s' len %d\n", (char*)&tag, len);
				const uint8_t* chunk_data = r.ptr;
				r.skip(len);
				chunks[tag] = { chunk_data, r.ptr };
			}
			for (auto& v : tags) {
				tag_t tag = std::get<0>(v);
				auto i = chunks.find(tag);
				if (i == chunks.end()) {
					if (std::get<1>(v)) xcept("map is missing required chunk '%s'", tagstr(tag));
				} else {
					if (!tag_funcs[tag]) xcept("tag '%s' is missing a function", tagstr(tag));
					//log("loading tag '%s'...\n", tagstr(tag));
					tag_funcs[tag](i->second);
				}
			}
		};

		int version = 0;
		tag_funcs["VER "] = [&](data_reader_le r) {
			version = r.get<uint16_t>();
			//log("VER: version is %d\n", version);
		};
		tag_funcs["DIM "] = [&](data_reader_le r) {
			game_st.map_tile_width = r.get<uint16_t>();
			game_st.map_tile_height = r.get<uint16_t>();
			game_st.map_width = game_st.map_tile_width * 32;
			game_st.map_height = game_st.map_tile_height * 32;
			game_st.map_walk_width = game_st.map_tile_width * 4;
			game_st.map_walk_height = game_st.map_tile_width * 4;
			//log("DIM: dimensions are %d %d\n", game_st.map_tile_width, game_st.map_tile_height);
		};
		tag_funcs["ERA "] = [&](data_reader_le r) {
			game_st.tileset_index = r.get<uint16_t>() % 8;
			//log("ERA: tileset is %d\n", game_st.tileset_index);
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
			//log("SPRP: scenario name: '%s',  description: '%s'\n", game_st.scenario_name, game_st.scenario_description);
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

		bool bVictoryCondition = false;
		bool bStartingUnits = false;
		bool bTournamentModeEnabled = false;
		bool bAlliesEnabled = true;

		tag_funcs["THG2"] = [&](data_reader_le r) {
			while (r.left()) {
				int unit_type = r.get<uint16_t>();
				int x = r.get<uint16_t>();
				int y = r.get<uint16_t>();
				(void)x; (void)y;
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
				unit_type_t* unit_type = get_unit_type(i);
				unit_type->hitpoints = fp8::from_raw(hp[i]);
				unit_type->shield_points = shield_points[i];
				unit_type->armor = armor[i];
				unit_type->build_time = build_time[i];
				unit_type->mineral_cost = mineral_cost[i];
				unit_type->gas_cost = gas_cost[i];
				unit_type->unit_map_string_index = string_index[i];
				const unit_type_t* attacking_type = unit_type->turret_unit_type ? unit_type->turret_unit_type : unit_type;
				weapon_type_t* ground_weapon = &game_st.weapon_types.vec.at(attacking_type->ground_weapon->id);
				weapon_type_t* air_weapon = &game_st.weapon_types.vec.at(attacking_type->air_weapon->id);
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

				(void)id; (void)link; (void)valid_flags; (void)units_in_hangar; (void)flags; (void)related_unit_id;

				if ((size_t)unit_type_id >= 228) xcept("UNIT: invalid unit type %d", unit_type_id);
				if ((size_t)owner >= 12) xcept("UNIT: invalid owner %d", owner);

				const unit_type_t* unit_type = get_unit_type(unit_type_id);

				//log("create unit of type %d\n", unit_type->id);

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
					if (is_mineral_field(unit_type)) return true;
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
					if (player_force[owner] && ~unit_type->group_flags & GroupFlags::Neutral) continue;
				}

				unit_t* u = create_initial_unit(unit_type, { x,y }, owner);

				if (valid_properties & 0x2) {
					using tmp_t = fixed_point<32, 8, true>;
					tmp_t tmp = tmp_t::extend(u->unit_type->hitpoints);
					tmp = std::max(tmp * hp_percent / 100, tmp_t::from_raw(1));
					set_unit_hp(u, fp8::truncate(tmp));
				}
				if (valid_properties & 0x4) set_unit_shield_points(u, fp8::integer(u->unit_type->shield_points * shield_percent / 100));
				if (valid_properties & 0x8) set_unit_energy(u, unit_max_energy(u) * energy_percent / 100);
				if (valid_properties & 0x10) set_unit_resources(u, resources);
				// more stuff...

				if (u->unit_type->id == UnitTypes::Zerg_Broodling) {
					int timer = u_hallucination(u) ? 1350 : 1800;
					if (u->remove_timer == 0 || timer < u->remove_timer) u->remove_timer = timer;
				}

				//log("created initial unit %p with id %d\n", u, u - st.units.data());

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
				a_string name = game_st.get_string(r.get<uint16_t>());
				int elevation_flags = r.get<uint16_t>();
				(void)left;
				(void)top;
				(void)right;
				(void)bottom;
				(void)name;
				(void)elevation_flags;
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
		st.local_mask = 1;

		for (int i = 0; i < 12; ++i) {
			st.alliances[i].fill(0);
			st.alliances[i][i] = 1;
		}

		for (int i = 0; i < 12; ++i) {
			st.alliances[i][11] = 1;
			st.alliances[11][i] = 1;

			if (bAlliesEnabled && !bTournamentModeEnabled) {
				for (int i2 = 0; i2 < 12; ++i2) {
					if (st.players[i].controller == state::player_t::controller_computer_game && st.players[i2].controller == state::player_t::controller_computer_game) {
						st.alliances[i][i2] = 2;
					}
				}
			}
		}

		for (int i = 0; i < 12; ++i) {
			st.shared_vision[i] = 1 << i;
			if (st.players[i].controller == state::player_t::controller_rescue_passive || st.players[i].controller == state::player_t::controller_neutral) {
				for (int i2 = 0; i2 < 12; ++i2) {
					st.alliances[i][i2] = 1;
					st.alliances[i2][i] = 1;
				}
			}
		}

		if (!bVictoryCondition && !bStartingUnits && !bTournamentModeEnabled) {

		}

		allow_random = true;

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

		allow_random = false;

	}
};

template<typename load_data_file_F>
void global_init(global_state& st, load_data_file_F&& load_data_file) {

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
			(void)signature;
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
					if ((size_t)(int)pc != pc) xcept("iscript load: 0x%x does not fit in an int", pc);
					program_data[std::get<1>(v)] = (int)pc;
				}
				return initial_pc;
			};

			auto& anim_funcs = animation_pc[id];

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
		for (auto& v : animation_pc) {
			auto& s = st.iscript.scripts[v.first];
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
		(void)file_count;

		a_vector<grp_t> grps;
		a_vector<a_vector<a_vector<xy>>> lo_offsets;

		auto load_grp = [&](data_reader_le r) {
			auto base_r = r;
			grp_t grp;
			size_t frame_count = r.get<uint16_t>();
			grp.width = r.get<uint16_t>();
			grp.height = r.get<uint16_t>();
			grp.frames.resize(frame_count);
			for (size_t i = 0; i < frame_count; ++i) {
				auto& f = grp.frames[i];
				f.offset.x = r.get<uint8_t>();
				f.offset.y = r.get<uint8_t>();
				f.size.x = r.get<uint8_t>();
				f.size.y = r.get<uint8_t>();
				size_t file_offset = r.get<uint32_t>();
				auto line_offset_r = base_r;
				line_offset_r.skip(file_offset);
				f.line_data_offset.reserve(f.size.y);
				for (size_t y = 0; y != f.size.y; ++y) {
					auto line_r = base_r;
					line_r.skip(file_offset + line_offset_r.get<uint16_t>());
					f.line_data_offset.push_back(f.data_container.size());
					for (size_t x = 0; x != f.size.x;) {
						auto v = line_r.get<uint8_t>();
						if (v & 0x80) {
							v &= 0x7f;
							if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
							f.data_container.push_back(0x80 | v);
							x += v;
						} else if (v & 0x40) {
							v &= 0x3f;
							if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
							f.data_container.push_back(0x40 | v);
							f.data_container.push_back(line_r.get<uint8_t>());
							x += v;
						} else {
							if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
							f.data_container.push_back(v);
							for (size_t i = 0; i != v; ++i) {
								f.data_container.push_back(line_r.get<uint8_t>());
							}
							x += v;
						}
					}
				}
				f.data_container.shrink_to_fit();
			}
			size_t index = grps.size();
			grps.push_back(std::move(grp));
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
				for (size_t i = 0; i < offset_count; ++i) {
					int x = r2.get<int8_t>();
					int y = r2.get<int8_t>();
					vec[i] = { x,y };
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
			load_data_file(data, format("unit\\%s", fn));
			data_reader_le data_r(data.data(), data.data() + data.size());
			size_t loaded_index = f(data_r);
			in.first->second = loaded_index;
			return loaded_index;
		};

		a_vector<size_t> image_grp_index;
		std::array<a_vector<size_t>, 6> lo_indices;

		grps.emplace_back(); // null/invalid entry
		lo_offsets.emplace_back();

		for (int i = 0; i < 999; ++i) {
			const image_type_t* image_type = get_image_type(i);
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

	load_data_file(st.units_dat, "arr/units.dat");
	load_data_file(st.weapons_dat, "arr/weapons.dat");
	load_data_file(st.upgrades_dat, "arr/upgrades.dat");
	load_data_file(st.techdata_dat, "arr/techdata.dat");

	a_vector<uint8_t> buf;
	load_data_file(buf, "arr/flingy.dat");
	st.flingy_types = data_loading::load_flingy_dat(buf);
	load_data_file(buf, "arr/sprites.dat");
	st.sprite_types = data_loading::load_sprites_dat(buf);
	load_data_file(buf, "arr/images.dat");
	st.image_types = data_loading::load_images_dat(buf);
	load_data_file(buf, "arr/orders.dat");
	st.order_types = data_loading::load_orders_dat(buf);

	auto fixup_sprite_type = [&](sprite_type_t*& ptr) {
		size_t index = (size_t)ptr;
		if (index == 517) ptr = nullptr;
		else ptr = get_sprite_type(index);
	};
	auto fixup_image_type = [&](image_type_t*& ptr) {
		size_t index = (size_t)ptr;
		if (index == 999) ptr = nullptr;
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
		load_data_file(st.tileset_vf4[i], format("Tileset\\%s.vf4", tileset_names.at(i)));
		load_data_file(st.tileset_cv5[i], format("Tileset\\%s.cv5", tileset_names.at(i)));
	}

	// This function returns (int)std::round(std::sin(PI / 128 * i) * 256) for i [0, 63]
	// using only integer arithmetic.
	auto int_sin = [&](int x) {
		int x2 = x*x;
		int x3 = x2*x;
		int x4 = x3*x;
		int x5 = x4*x;

		int64_t a0 = 26980449732;
		int64_t a1 = 1140609;
		int64_t a2 = -2785716;
		int64_t a3 = 2159;
		int64_t a4 = 58;

		return (int)((x * a0 + x2 * a1 + x3 * a2 + x4 * a3 + x5 * a4 + ((int64_t)1 << 31)) >> 32);
	};

	// The sin lookup table is hardcoded into Broodwar. We generate it here.
	for (int i = 0; i <= 64; ++i) {
		auto v = fp8::from_raw(int_sin(i));
		st.direction_table[i].x = v;
		st.direction_table[64 - i].y = -v;
		st.direction_table[64 + (64 - i)].x = v;
		st.direction_table[64 + i].y = v;
		st.direction_table[128 + i].x = -v;
		st.direction_table[128 + (64 - i)].y = v;
		st.direction_table[(192 + (64 - i)) % 256].x = -v;
		st.direction_table[(192 + i) % 256].y = -v;
	}

	auto direction_from_index = [](size_t index) {
		int v = index;
		if (v >= 128) v = -(256 - v);
		return direction_t::from_raw(v);
	};

	for (size_t i = 0; i != 256; ++i) {
		st.repulse_direction_table[i] = direction_from_index(i % 32 + (i < 128 ? 32 : -64));
	}

}

struct game_player {
	state* st = nullptr;
	std::unique_ptr<global_state> uptr_global_st;
	std::unique_ptr<game_state> uptr_game_st;
	std::unique_ptr<state> uptr_st;
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
		st = uptr_st.get();
		st->global = uptr_global_st.get();
		st->game = uptr_game_st.get();
		global_init(*uptr_global_st, std::forward<load_data_file_F>(load_data_file));
	}
	void load_map_file(const a_string& filename) {
		if (!st) xcept("game_player: not initialized");
		game_load_functions game_load_funcs(*st);
		game_load_funcs.load_map_file(filename);
		state_functions funcs(*st);
		funcs.process_frame();
		funcs.process_frame();
	}
	void next_frame() {
		if (!st) xcept("game_player: not initialized");
		state_functions funcs(*st);
		funcs.next_frame();
	}
};

}

