

namespace data_loading {
;

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
		if (ptr + sizeof(T) > end) xcept("data_reader: attempt to read past end");
		ptr += sizeof(T);
		return value_at<T, little_endian>(ptr - sizeof(T));
	}
	uint8_t*get_n(size_t n) {
		uint8_t*r = ptr;
		if (ptr + n > end || ptr + n < ptr) xcept("data_reader: attempt to read past end");
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
		if (ptr + n > end || ptr + n < ptr) xcept("data_reader: attempt to seek past end");
		ptr += n;
	}
	size_t left() {
		return end - ptr;
	}
};

using data_reader_le = data_reader<true>;
using data_reader_be = data_reader<false>;

struct data_file {
	a_vector<uint8_t> data;
	data_file(a_string fn) {
		load_data_file(data, fn);
	}
};

template<bool default_little_endian = true>
struct data_file_reader: data_reader<default_little_endian> {
	data_file_reader(data_file&f) : data_reader(f.data.data(), f.data.data() + f.data.size()) {}
};


using data_file_reader_le = data_file_reader<true>;
using data_file_reader_be = data_file_reader<false>;

template<typename to_T, typename from_T, typename std::enable_if<std::is_same<to_T,bool>::value>::type* = nullptr>
to_T data_type_cast(from_T v) {
	static_assert(std::is_integral<from_T>::value, "from_T must be integral");
	if (v != 0 && v != 1) xcept("value 0x%x is not a boolean", v);
	return !!v;
}
template<typename to_T, typename from_T, typename std::enable_if<!std::is_same<to_T, bool>::value>::type* = nullptr>
to_T data_type_cast(from_T v) {
	static_assert(std::is_integral<from_T>::value, "from_T must be integral");
	if ((from_T)(intmax_t)(to_T)v != v) xcept("value 0x%x of type %s does not fit in type %s", v, typeid(from_T).name(), typeid(to_T).name());
	return (to_T)v;
}

template<typename load_T, typename field_T, typename obj_T, typename reader_T, typename arr_T>
void read_array(reader_T&r, arr_T&arr, size_t num, size_t offset) {
	static_assert(!std::is_reference<field_T>::value, "field_T can not be a reference");
	if (offset&(alignof(field_T)-1)) xcept("offset of %d for %s is not aligned to %d bytes", typeid(field_T).name(), offset, alignof(field_T));
	for (size_t i = 0; i < num; ++i) {
		auto val = r.get<load_T>();
		*(field_T*)((uint8_t*)&arr[i] + offset) = data_type_cast<field_T>(val);
	}
}

#define rawr(load_type, name, num) read_array<load_type, decltype(arr[0].name), std::remove_reference<decltype(arr[0])>::type>(r, arr, num, offsetof(std::remove_reference<decltype(arr[0])>::type, name))

unit_types_t load_units_dat(a_string fn) {
	static const size_t total_count = 228;
	static const size_t units_count = 106;
	static const size_t buildings_count = 96;

	unit_types_t unit_types;
	unit_types.vec.resize(total_count);
	for (size_t i = 0; i < total_count; ++i) {
		auto&v = unit_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = unit_types.vec;

	rawr(uint8_t, flingy, total_count);
	rawr(uint16_t, turret_unit_type, total_count);
	rawr(uint16_t, subunit2, total_count);
	rawr(uint16_t, infestation, buildings_count);
	rawr(uint32_t, construction_animation, total_count);
	rawr(uint8_t, unit_direction, total_count);
	rawr(uint8_t, has_shield, total_count);
	rawr(uint16_t, shield_points, total_count);
	rawr(int32_t, hitpoints, total_count);
	rawr(uint8_t, elevation_level, total_count);
	rawr(uint8_t, unknown1, total_count);
	rawr(uint8_t, sublabel, total_count);
	rawr(uint8_t, computer_ai_idle, total_count);
	rawr(uint8_t, human_ai_idle, total_count);
	rawr(uint8_t, return_to_idle, total_count);
	rawr(uint8_t, attack_unit, total_count);
	rawr(uint8_t, attack_move, total_count);
	rawr(uint8_t, ground_weapon, total_count);
	rawr(uint8_t, max_ground_hits, total_count);
	rawr(uint8_t, air_weapon, total_count);
	rawr(uint8_t, max_air_hits, total_count);
	rawr(uint8_t, ai_internal, total_count);
	rawr(uint32_t, flags, total_count);
	rawr(uint8_t, target_acquisition_range, total_count);
	rawr(uint8_t, sight_range, total_count);
	rawr(uint8_t, armor_upgrade, total_count);
	rawr(uint8_t, unit_size, total_count);
	rawr(uint8_t, armor, total_count);
	rawr(uint8_t, right_click_action, total_count);
	rawr(uint16_t, ready_sound, units_count);
	rawr(uint16_t, first_what_sound, total_count);
	rawr(uint16_t, last_what_sound, total_count);
	rawr(uint16_t, first_pissed_sound, units_count);
	rawr(uint16_t, last_pissed_sound, units_count);
	rawr(uint16_t, first_yes_sound, units_count);
	rawr(uint16_t, last_yes_sound, units_count);
	rawr(uint16_t, staredit_placement_box_width, total_count);
	rawr(uint16_t, staredit_placement_box_height, total_count);
	rawr(uint16_t, addon_horizontal, buildings_count);
	rawr(uint16_t, addon_vertical, buildings_count);
	rawr(uint16_t, dimensions.from.x, total_count);
	rawr(uint16_t, dimensions.from.y, total_count);
	rawr(uint16_t, dimensions.to.x, total_count);
	rawr(uint16_t, dimensions.to.y, total_count);
	rawr(uint16_t, portrait, total_count);
	rawr(uint16_t, mineral_cost, total_count);
	rawr(uint16_t, gas_cost, total_count);
	rawr(uint16_t, build_time, total_count);
	rawr(uint16_t, unknown2, total_count);
	rawr(uint8_t, staredit_group_flags, total_count);
	rawr(uint8_t, supply_provided, total_count);
	rawr(uint8_t, supply_required, total_count);
	rawr(uint8_t, space_required, total_count);
	rawr(uint8_t, space_provided, total_count);
	rawr(uint16_t, build_score, total_count);
	rawr(uint16_t, destroy_score, total_count);
	rawr(uint16_t, unit_map_string_index, total_count);
	rawr(uint8_t, is_broodwar, total_count);
	rawr(uint16_t, staredit_availability_flags, total_count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return unit_types;
}


weapon_types_t load_weapons_dat(a_string fn) {
	static const size_t count = 130;

	weapon_types_t weapon_types;
	weapon_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto&v = weapon_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = weapon_types.vec;

	rawr(uint16_t, label, count);
	rawr(uint32_t, flingy, count);
	rawr(uint8_t, unused, count);
	rawr(uint16_t, target_flags, count);
	rawr(uint32_t, min_range, count);
	rawr(uint32_t, max_range, count);
	rawr(uint8_t, damage_upgrade, count);
	rawr(uint8_t, weapon_type, count);
	rawr(uint8_t, weapon_behavior, count);
	rawr(uint8_t, remove_after, count);
	rawr(uint8_t, explosion_type, count);
	rawr(uint16_t, inner_splash_radius, count);
	rawr(uint16_t, medium_splash_radius, count);
	rawr(uint16_t, outer_splash_radius, count);
	rawr(uint16_t, damage_amount, count);
	rawr(uint16_t, damage_bonus, count);
	rawr(uint8_t, cooldown, count);
	rawr(uint8_t, damage_factor, count);
	rawr(uint8_t, attack_angle, count);
	rawr(uint8_t, launch_spin, count);
	rawr(int8_t, forward_offset, count);
	rawr(int8_t, upward_offset, count);
	rawr(uint16_t, target_error_message, count);
	rawr(uint16_t, icon, count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return weapon_types;
}

upgrade_types_t load_upgrades_dat(a_string fn) {
	static const size_t count = 61;

	upgrade_types_t upgrade_types;
	upgrade_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto&v = upgrade_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = upgrade_types.vec;

	rawr(uint16_t, mineral_cost_base, count);
	rawr(uint16_t, mineral_cost_factor, count);
	rawr(uint16_t, gas_cost_base, count);
	rawr(uint16_t, gas_cost_factor, count);
	rawr(uint16_t, research_time_base, count);
	rawr(uint16_t, research_time_factor, count);
	rawr(uint16_t, unknown, count);
	rawr(uint16_t, icon, count);
	rawr(uint16_t, label, count);
	rawr(uint8_t, race, count);
	rawr(uint8_t, max_repeats, count);
	rawr(uint8_t, is_broodwar, count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return upgrade_types;
}

tech_types_t load_techdata_dat(a_string fn) {
	static const size_t count = 44;

	tech_types_t tech_types;
	tech_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto&v = tech_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = tech_types.vec;

	rawr(uint16_t, mineral_cost, count);
	rawr(uint16_t, gas_cost, count);
	rawr(uint16_t, research_time, count);
	rawr(uint16_t, energy_cost, count);
	rawr(uint32_t, unknown, count);
	rawr(uint16_t, icon, count);
	rawr(uint16_t, label, count);
	rawr(uint8_t, race, count);
	rawr(uint16_t, flags, count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return tech_types;
}

flingy_types_t load_flingy_dat(a_string fn) {
	static const size_t count = 209;

	flingy_types_t flingy_types;
	flingy_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto&v = flingy_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = flingy_types.vec;

	rawr(uint16_t, sprite, count);
	rawr(uint32_t, top_speed, count);
	rawr(uint16_t, acceleration, count);
	rawr(uint32_t, halt_distance, count);
	rawr(uint8_t, turn_rate, count);
	rawr(uint8_t, unused, count);
	rawr(uint8_t, movement_type, count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return flingy_types;
}

sprite_types_t load_sprites_dat(a_string fn) {
	static const size_t count = 517;
	static const size_t selectable_count = 387;

	sprite_types_t sprite_types;
	sprite_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto&v = sprite_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = sprite_types.vec;

	rawr(uint16_t, image, count);
	rawr(uint8_t, health_bar_size, selectable_count);
	rawr(uint16_t, flags, count);
	rawr(uint8_t, selection_circle, selectable_count);
	rawr(uint8_t, selection_circle_vpos, selectable_count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return sprite_types;
}

image_types_t load_images_dat(a_string fn) {
	static const size_t count = 999;

	image_types_t image_types;
	image_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto&v = image_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = image_types.vec;

	rawr(uint32_t, grp_filename_index, count);
	rawr(uint8_t, has_directional_frames, count);
	rawr(uint8_t, is_clickable, count);
	rawr(uint8_t, has_iscript_animations, count);
	rawr(uint8_t, draw_if_cloaked, count);
	rawr(uint8_t, palette_type, count);
	rawr(uint8_t, color_shift, count);
	rawr(uint32_t, iscript_id, count);
	rawr(uint32_t, shield_filename_index, count);
	rawr(uint32_t, attack_filename_index, count);
	rawr(uint32_t, damage_filename_index, count);
	rawr(uint32_t, special_filename_index, count);
	rawr(uint32_t, landing_dust_filename_index, count);
	rawr(uint32_t, lift_off_filename_index, count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return image_types;
}

order_types_t load_orders_dat(a_string fn) {
	static const size_t count = 189;

	order_types_t order_types;
	order_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto&v = order_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = i;
	}

	data_file f(fn);
	data_file_reader_le r(f);

	auto&arr = order_types.vec;

	rawr(uint8_t, use_weapon_targeting, count);
	rawr(uint8_t, background, count);
	rawr(uint8_t, unused3, count);
	rawr(uint8_t, valid_for_turret, count);
	rawr(uint8_t, can_be_interrupted, count);
	rawr(uint8_t, unk7, count);
	rawr(uint8_t, can_be_queued, count);
	rawr(uint8_t, unk9, count);
	rawr(uint8_t, can_be_obstructed, count);
	rawr(uint8_t, unk11, count);
	rawr(uint8_t, unused12, count);
	rawr(uint8_t, weapon, count);
	rawr(uint8_t, tech_type, count);
	rawr(uint8_t, seq, count);
	rawr(int16_t, highlight, count);
	rawr(uint16_t, dep_index, count);
	rawr(uint8_t, obscured, count);

	if (r.left()) log(" WARNING: %s: %d bytes left\n", fn, r.left());

	return order_types;
}

#undef rawr

}

