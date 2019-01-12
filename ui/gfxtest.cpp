#include "gfxtest.h"
#include "SDL.h"

using namespace bwgame;

using ui::log;

FILE* log_file = nullptr;

namespace bwgame {

namespace ui {

void log_str(a_string str) {
	fwrite(str.data(), str.size(), 1, stdout);
	fflush(stdout);
	if (!log_file) log_file = fopen("log.txt", "wb");
	if (log_file) {
		fwrite(str.data(), str.size(), 1, log_file);
		fflush(log_file);
	}
}

void fatal_error_str(a_string str) {
#ifdef EMSCRIPTEN
	const char* p = str.c_str();
	EM_ASM_({js_fatal_error($0);}, p);
#endif
	log("fatal error: %s\n", str);
	std::terminate();
}

}

}

void main_t::reset() {
	saved_states.clear();
	ui.reset();
	units_matcher = unit_matcher(this);
}

void main_t::next_frame() {
	int save_interval = 10 * 1000 / 42;
	if (ui.st.current_frame == 0 || ui.st.current_frame % save_interval == 0) {
		auto i = saved_states.find(ui.st.current_frame);
		if (i == saved_states.end()) {
			auto v = std::make_unique<saved_state>();
			v->st = copy_state(ui.st);
			v->action_st = copy_state(ui.action_st, ui.st, v->st);
			v->apm = ui.apm;

			a_map<int, std::unique_ptr<saved_state>> new_saved_states;
			new_saved_states[ui.st.current_frame] = std::move(v);
			while (!saved_states.empty()) {
				auto i = saved_states.begin();
				auto v = std::move(*i);
				saved_states.erase(i);
				new_saved_states[v.first] = std::move(v.second);
			}
			std::swap(saved_states, new_saved_states);
		}
	}
	ui.replay_functions::next_frame();
	for (auto& v : ui.apm) v.update(ui.st.current_frame);
	units_matcher.on_frame();
}

void main_t::update() {
	if (!run_main_update_loop) {
		return;
	}

	auto now = clock.now();

	auto tick_speed = std::chrono::milliseconds((fp8::integer(42) / ui.game_speed).integer_part());

	if (now - last_fps >= std::chrono::seconds(1)) {
		//log("game fps: %g\n", fps_counter / std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - last_fps).count());
		last_fps = now;
		fps_counter = 0;
	}

	if (!ui.is_done() || ui.st.current_frame != ui.replay_frame) {
		if (ui.st.current_frame != ui.replay_frame) {
			if (ui.st.current_frame != ui.replay_frame) {
				auto i = saved_states.lower_bound(ui.replay_frame);
				if (i != saved_states.begin()) --i;
				auto& v = i->second;
				if (ui.st.current_frame > ui.replay_frame || v->st.current_frame > ui.st.current_frame) {
					ui.st = copy_state(v->st);
					ui.action_st = copy_state(v->action_st, v->st, ui.st);
					ui.apm = v->apm;
				}
			}
			if (ui.st.current_frame < ui.replay_frame) {
				for (size_t i = 0; i != 32 && ui.st.current_frame != ui.replay_frame; ++i) {
					for (size_t i2 = 0; i2 != 4 && ui.st.current_frame != ui.replay_frame; ++i2) {
						next_frame();
					}
					if (clock.now() - now >= std::chrono::milliseconds(50)) break;
				}
			}
			last_tick = now;
		} else {
			if (ui.is_paused) {
				last_tick = now;
			} else {
				auto tick_t = now - last_tick;
				if (tick_t >= tick_speed * 16) {
					last_tick = now - tick_speed * 16;
					tick_t = tick_speed * 16;
				}
				auto tick_n = tick_speed.count() == 0 ? 128 : tick_t / tick_speed;
				for (auto i = tick_n; i;) {
					--i;
					++fps_counter;
					last_tick += tick_speed;

					if (!ui.is_done()) next_frame();
					else break;
					if (i % 4 == 3 && clock.now() - now >= std::chrono::milliseconds(50)) break;
				}
				ui.replay_frame = ui.st.current_frame;
			}
		}
	}

	ui.update();
}

main_t* g_m = nullptr;

uint32_t freemem_rand_state = (uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
auto freemem_rand() {
	freemem_rand_state = freemem_rand_state * 22695477 + 1;
	return (freemem_rand_state >> 16) & 0x7fff;
}

void out_of_memory() {
	printf("out of memory :(\n");
#ifdef EMSCRIPTEN
	const char* p = "out of memory :(";
	EM_ASM_({js_fatal_error($0);}, p);
#endif
	throw std::bad_alloc();
}

size_t bytes_allocated = 0;

void free_memory() {
	if (!g_m) out_of_memory();
	size_t n_states = g_m->saved_states.size();
	printf("n_states is %zu\n", n_states);
	if (n_states <= 2) out_of_memory();
	size_t n;
	if (n_states >= 300) n = 1 + freemem_rand() % (n_states - 2);
	else {
		auto begin = std::next(g_m->saved_states.begin());
		auto end = std::prev(g_m->saved_states.end());
		n = 1;
		int best_score = std::numeric_limits<int>::max();
		size_t i_n = 1;
		for (auto i = begin; i != end; ++i, ++i_n) {
			int score = 0;
			for (auto i2 = begin; i2 != end; ++i2) {
				if (i2 != i) {
					int d = i2->first - i->first;
					score += d*d;
				}
			}
			if (score < best_score) {
				best_score = score;
				n = i_n;
			}
		}
	}
	g_m->saved_states.erase(std::next(g_m->saved_states.begin(), n));
}

//extern "C" void set_malloc_fail_handler(bool(*)());

//bool malloc_fail_handler() {
//	free_memory();
//	return true;
//}

struct dlmalloc_chunk {
	size_t prev_foot;
	size_t head;
	dlmalloc_chunk* fd;
	dlmalloc_chunk* bk;
};

size_t alloc_size(void* ptr) {
	dlmalloc_chunk* c = (dlmalloc_chunk*)((char*)ptr - sizeof(size_t) * 2);
	return c->head & ~7;
}

extern "C" void* dlmalloc(size_t);
extern "C" void dlfree(void*);

size_t max_bytes_allocated = 160 * 1024 * 1024;

extern "C" void* malloc(size_t n) {
	void* r = dlmalloc(n);
	while (!r) {
		printf("failed to allocate %zu bytes\n", n);
		free_memory();
		r = dlmalloc(n);
	}
	bytes_allocated += alloc_size(r);
	while (bytes_allocated > max_bytes_allocated) free_memory();
	return r;
}

extern "C" void free(void* ptr) {
	if (!ptr) return;
	bytes_allocated -= alloc_size(ptr);
	dlfree(ptr);
}

#ifdef EMSCRIPTEN

namespace bwgame {
namespace data_loading {

template<bool default_little_endian = true>
struct js_file_reader {
	a_string filename;
	size_t index = ~(size_t)0;
	size_t file_pointer = 0;
	js_file_reader() = default;
	explicit js_file_reader(a_string filename) {
		open(std::move(filename));
	}
	void open(a_string filename) {
		if (filename == "StarDat.mpq") index = 0;
		else if (filename == "BrooDat.mpq") index = 1;
		else if (filename == "Patch_rt.mpq") index = 2;
		else ui::xcept("js_file_reader: unknown filename '%s'", filename);
		this->filename = std::move(filename);
	}

	void get_bytes(uint8_t* dst, size_t n) {
		EM_ASM_({js_read_data($0, $1, $2, $3);}, index, dst, file_pointer, n);
		file_pointer += n;
	}

	void seek(size_t offset) {
		file_pointer = offset;
	}
	size_t tell() const {
		file_pointer;
	}

	size_t size() {
		return EM_ASM_INT({return js_file_size($0);}, index);
	}

};

}
}

main_t* m;

int current_width = -1;
int current_height = -1;

extern "C" void ui_resize(int width, int height) {
	if (width == current_width && height == current_height) return;
	if (width <= 0 || height <= 0) return;
	current_width = width;
	current_height = height;
	if (!m) return;
	m->ui.window_surface.reset();
	m->ui.indexed_surface.reset();
	m->ui.rgba_surface.reset();
	m->ui.wnd.destroy();
	m->ui.wnd.create("test", 0, 0, width, height);
	m->ui.resize(width, height);
}

extern "C" double replay_get_value(int index) {
	switch (index) {
	case 0:
		return m->ui.game_speed.raw_value / 256.0;
	case 1:
		return m->ui.is_paused ? 1 : 0;
	case 2:
		return (double)m->ui.st.current_frame;
	case 3:
		return (double)m->ui.replay_frame;
	case 4:
		return (double)m->ui.replay_st.end_frame;
	case 5:
		return (double)(uintptr_t)m->ui.replay_st.map_name.data();
	case 6:
		return (double)m->ui.replay_frame / m->ui.replay_st.end_frame;
	default:
		return 0;
	}
}

extern "C" void replay_set_value(int index, double value) {
	switch (index) {
	case 0:
		m->ui.game_speed.raw_value = (int)(value * 256.0);
		if (m->ui.game_speed < 1_fp8) m->ui.game_speed = 1_fp8;
		break;
	case 1:
		m->ui.is_paused = value != 0.0;
		break;
	case 3:
		m->ui.replay_frame = (int)value;
		if (m->ui.replay_frame < 0) m->ui.replay_frame = 0;
		if (m->ui.replay_frame > m->ui.replay_st.end_frame) m->ui.replay_frame = m->ui.replay_st.end_frame;
		break;
	case 6:
		m->ui.replay_frame = (int)(m->ui.replay_st.end_frame * value);
		if (m->ui.replay_frame < 0) m->ui.replay_frame = 0;
		if (m->ui.replay_frame > m->ui.replay_st.end_frame) m->ui.replay_frame = m->ui.replay_st.end_frame;
		break;
	}
}

#include <emscripten/bind.h>
#include <emscripten/val.h>
using namespace emscripten;

struct js_unit_type {
	const unit_type_t* ut = nullptr;
	js_unit_type() {}
	js_unit_type(const unit_type_t* ut) : ut(ut) {}
	auto id() const {return ut ? (int)ut->id : 228;}
	auto build_time() const {return ut->build_time;}
};

struct js_unit {
	unit_t* u = nullptr;
	js_unit() {}
	js_unit(unit_t* u) : u(u) {}
	auto owner() const {return u->owner;}
	auto remaining_build_time() const {return u->remaining_build_time;}
	auto unit_type() const {return u->unit_type;}
	auto build_type() const {return u->build_queue.empty() ? nullptr : u->build_queue.front();}
};


struct util_functions: state_functions {
	util_functions(state& st) : state_functions(st) {}

	double worker_supply(int owner) {
		double r = 0.0;
		for (const unit_t* u : ptr(st.player_units.at(owner))) {
			if (!ut_worker(u)) continue;
			if (!u_completed(u)) continue;
			r += u->unit_type->supply_required.raw_value / 2.0;
		}
		return r;
	}

	double army_supply(int owner) {
		double r = 0.0;
		for (const unit_t* u : ptr(st.player_units.at(owner))) {
			if (ut_worker(u)) continue;
			if (!u_completed(u)) continue;
			r += u->unit_type->supply_required.raw_value / 2.0;
		}
		return r;
	}

	auto get_all_incomplete_units() {
		val r = val::array();
		size_t i = 0;
		for (unit_t* u : ptr(st.visible_units)) {
			if (u_completed(u)) continue;
			r.set(i++, u);
		}
		for (unit_t* u : ptr(st.hidden_units)) {
			if (u_completed(u)) continue;
			r.set(i++, u);
		}
		return r;
	}

	auto get_all_completed_units() {
		val r = val::array();
		size_t i = 0;
		for (unit_t* u : ptr(st.visible_units)) {
			if (!u_completed(u)) continue;
			r.set(i++, u);
		}
		for (unit_t* u : ptr(st.hidden_units)) {
			if (!u_completed(u)) continue;
			r.set(i++, u);
		}
		return r;
	}

	auto get_all_units() {
		val r = val::array();
		size_t i = 0;
		for (unit_t* u : ptr(st.visible_units)) {
			r.set(i++, u);
		}
		for (unit_t* u : ptr(st.hidden_units)) {
			r.set(i++, u);
		}
		for (unit_t* u : ptr(st.map_revealer_units)) {
			r.set(i++, u);
		}
		return r;
	}

	auto get_completed_upgrades(int owner) {
		val r = val::array();
		size_t n = 0;
		for (size_t i = 0; i != 61; ++i) {
			int level = player_upgrade_level(owner, (UpgradeTypes)i);
			if (level == 0) continue;
			val o = val::object();
			o.set("id", val((int)i));
			o.set("icon", val(get_upgrade_type((UpgradeTypes)i)->icon));
			o.set("level", val(level));
			r.set(n++, o);
		}
		return r;
	}

	auto get_completed_research(int owner) {
		val r = val::array();
		size_t n = 0;
		for (size_t i = 0; i != 44; ++i) {
			if (!player_has_researched(owner, (TechTypes)i)) continue;
			val o = val::object();
			o.set("id", val((int)i));
			o.set("icon", val(get_tech_type((TechTypes)i)->icon));
			r.set(n++, o);
		}
		return r;
	}

	auto get_incomplete_upgrades(int owner) {
		val r = val::array();
		size_t i = 0;
		for (unit_t* u : ptr(st.player_units[owner])) {
			if (u->order_type->id == Orders::Upgrade && u->building.upgrading_type) {
				val o = val::object();
				o.set("id", val((int)u->building.upgrading_type->id));
				o.set("icon", val((int)u->building.upgrading_type->icon));
				o.set("level", val(u->building.upgrading_level));
				o.set("remaining_time", val(u->building.upgrade_research_time));
				o.set("total_time", val(upgrade_time_cost(owner, u->building.upgrading_type)));
				r.set(i++, o);
			}
		}
		return r;
	}

	auto get_incomplete_research(int owner) {
		val r = val::array();
		size_t i = 0;
		for (unit_t* u : ptr(st.player_units[owner])) {
			if (u->order_type->id == Orders::ResearchTech && u->building.researching_type) {
				val o = val::object();
				o.set("id", val((int)u->building.researching_type->id));
				o.set("icon", val((int)u->building.researching_type->icon));
				o.set("remaining_time", val(u->building.upgrade_research_time));
				o.set("total_time", val(u->building.researching_type->research_time));
				r.set(i++, o);
			}
		}
		return r;
	}

	auto get_all_selected_units() {
		val r = val::array();
		size_t i = 0;
		for (unit_t* u : ptr(st.visible_units)) {
			if (u_completed(u)) continue;
			r.set(i++, u);
		}
		for (unit_t* u : ptr(st.hidden_units)) {
			if (u_completed(u)) continue;
			r.set(i++, u);
		}
		return r;
	}
};

optional<util_functions> m_util_funcs;

util_functions& get_util_funcs() {
	m_util_funcs.emplace(m->ui.st);
	return *m_util_funcs;
}

const unit_type_t* unit_t_unit_type(const unit_t* u) {
	return u->unit_type;
}
const unit_type_t* unit_t_build_type(const unit_t* u) {
	if (u->build_queue.empty()) return nullptr;
	return u->build_queue.front();
}

int unit_type_t_id(const unit_type_t& ut) {
	return (int)ut.id;
}

void set_volume(double percent) {
	m->ui.set_volume((int)(percent * 100));
}

double get_volume() {
	return m->ui.global_volume / 100.0;
}

class Dump {
public:
	#define STR(a) #a
	#define DUMP_VAL(name) o.set(STR(name), to_emscripten(dumping->name))
	#define DUMP_POS(field) o.set(STR(field), dump_pos(dumping->field))

	template <typename T>
	static val to_emscripten(T& v) {
		return val(v);
	}
	static val to_emscripten(fp8& v) {
		double as_double = v.raw_value;
		as_double /= (1 << v.fractional_bits);
		return val(as_double);
	}
	template<typename T>
	static val to_emscripten(id_type_for<T>* v) {
		if (!v) {
			return val::null();
		}
		return val((int)v->id);
	}
	static val to_emscripten(unit_t* unit) {
		util_functions f(m->ui.st);
		if (!unit) {
			return val::null();
		}
		return val(f.get_unit_id(unit).raw_value);
	}

	template <typename T>
	static val dump_pos(T& pos) {
		val o = val::object();
		o.set("x", to_emscripten(pos.x));
		o.set("y", to_emscripten(pos.y));
		return o;
	}

	static val dump_sprite(sprite_t* dumping) {
		val o = val::object();
		DUMP_VAL(index);
		DUMP_VAL(owner);
		DUMP_VAL(selection_index);
		DUMP_VAL(visibility_flags);
		DUMP_VAL(elevation_level);
		DUMP_VAL(flags);
		DUMP_VAL(selection_timer);
		DUMP_VAL(width);
		DUMP_VAL(height);
		DUMP_POS(position);
		return o;
	}

	static val dump_thingy(thingy_t* dumping) {
		val o = val::object();
		DUMP_VAL(hp);
		o.set("sprite", dump_sprite(dumping->sprite));
		return o;
	}

	static val dump_target(target_t* dumping) {
		val o = val::object();
		DUMP_POS(pos);
		DUMP_VAL(unit);
		return o;
	}

	static val dump_flingy(flingy_t* dumping) {
		val o = val::object();
		DUMP_VAL(index);
		o.set("move_target", dump_target(&dumping->move_target));
		DUMP_POS(next_movement_waypoint);
		DUMP_POS(next_target_waypoint);
		DUMP_VAL(movement_flags);
		DUMP_POS(position);
		DUMP_POS(exact_position);
		DUMP_VAL(flingy_top_speed);
		DUMP_VAL(current_speed);
		DUMP_VAL(next_speed);
		DUMP_POS(velocity);
		DUMP_VAL(flingy_acceleration);
		o.set("sprite", dump_sprite(dumping->sprite));
		o.set("_thingy_t", dump_thingy(dumping));
		return o;
	}

	static val dump_unit(unit_t* dumping) {
		val o = val::object();
		DUMP_VAL(owner);
		DUMP_VAL(order_state);
		DUMP_VAL(main_order_timer);
		DUMP_VAL(ground_weapon_cooldown);
		DUMP_VAL(air_weapon_cooldown);
		DUMP_VAL(spell_cooldown);
		o.set("order_target", dump_target(&dumping->order_target));

		DUMP_VAL(shield_points);
		DUMP_VAL(unit_type);

		DUMP_VAL(subunit);
		DUMP_VAL(auto_target_unit);
		DUMP_VAL(connected_unit);
		DUMP_VAL(order_queue_count);
		DUMP_VAL(order_process_timer);
		DUMP_VAL(unknown_0x086);
		DUMP_VAL(attack_notify_timer);
		DUMP_VAL(previous_unit_type);
		DUMP_VAL(last_event_timer);
		DUMP_VAL(last_event_color);
		DUMP_VAL(rank_increase);
		DUMP_VAL(kill_count);
		DUMP_VAL(last_attacking_player);
		DUMP_VAL(secondary_order_timer);
		DUMP_VAL(user_action_flags);
		DUMP_VAL(cloak_counter);
		DUMP_VAL(movement_state);
		DUMP_VAL(energy);
		DUMP_VAL(unit_id_generation);
		DUMP_VAL(damage_overlay_state);
		DUMP_VAL(hp_construction_rate);
		DUMP_VAL(shield_construction_rate);
		DUMP_VAL(remaining_build_time);
		DUMP_VAL(previous_hp);

		val loaded = val::object();
		for (int i = 0; i < dumping->loaded_units.size(); ++i) {
			loaded.set(i, dumping->loaded_units[i].raw_value);
		}
		o.set("loaded_units", loaded);
		o.set("_flingy_t", dump_flingy(dumping));
		return o;
	}
};

val lookup_unit_extended(int32_t index) {
	util_functions f(m->ui.st);
	unit_t* u = f.get_unit(unit_id(index));
	if (!u) {
		return val::null();
	}
	return Dump::dump_unit(u);
}

val lookup_unit(int32_t index) {
	util_functions f(m->ui.st);
	unit_t* u = f.get_unit(unit_id(index));
	if (!u) {
		return val::null();
	}
	val o = val::object();
	o.set("bw_id", f.get_unit_id(u).raw_value);
	o.set("player_id", u->owner);
	o.set("x", u->position.x);
	o.set("y", u->position.y);
	o.set("type", (int)u->unit_type->id);
	o.set("hp", Dump::to_emscripten(u->hp));
	o.set("ground_weapon_cooldown", u->ground_weapon_cooldown);
	o.set("air_weapon_cooldown", u->air_weapon_cooldown);
	return o;
}

auto get_selected_units() {
	util_functions f(m->ui.st);
	auto& st = m->ui.st;
	val r = val::array();
	size_t i = 0;
	for (unit_t* u : ptr(st.visible_units)) {
		if (m->ui.current_selection_is_selected(u)) {
			val o = val::object();
			o.set("bw_id", f.get_unit_id(u).raw_value);
			o.set("player_id", u->owner);
			o.set("id", - (int)f.get_unit_id(u).raw_value);
			o.set("x", u->position.x);
			o.set("y", u->position.y);
			o.set("type", (int)u->unit_type->id);
			r.set(i++, o);
		}
	}
	return r;
}

void select_unit_by_bw_id(size_t index) {
	util_functions f(m->ui.st);
	m->ui.current_selection_add(f.get_unit(unit_id(index)));
}

void set_screen_center_position(int32_t x, int32_t y) {
	ui_functions& ui = m->ui;
	ui.screen_pos.x = x - ui.view_width / 2;
	ui.screen_pos.y = y - ui.view_height / 2;
}

void clear_selection() {
	m->ui.current_selection.clear();
}

unit_matcher* get_units_matcher() {
	return &g_m->units_matcher;
}

void enable_main_update_loop() {
	m->run_main_update_loop = true;
}

bool any_replay_loaded = false;
bool has_replay_loaded() {
	return any_replay_loaded;
}

bool js_add_draw_command(val command) {
	auto cmd = std::make_unique<draw_command_t>(command);
	if (!cmd->is_valid()) {
		return false;
	}
	m->ui.add_draw_command(std::move(cmd));
	return true;
}

void js_clear_draw_commands() {
	m->ui.clear_draw_commands();
}

extern "C" void js_add_screen_overlay(
		float* values,
		size_t x_dim,
		size_t y_dim,
		int x_topleft,
		int y_topleft,
		float x_scaling,
		float y_scaling,
		float original_ratio,
		float game_saturation,
		float mean,
		float stddev,
		bool render_fast) {
	m->ui.overlay = ui_functions::overlay_t{
		std::vector<float>(values, values + (x_dim * y_dim)),
		{x_dim, y_dim},
		{x_topleft, y_topleft},
		{x_scaling, y_scaling},
		original_ratio,
		game_saturation,
		render_fast
	};
	for (auto& v: m->ui.overlay->values) {
		v -= mean;
		v /= stddev;
	}
}

void js_set_highlighted_units(val unit_ids) {
	auto ids = emscripten::vecFromJSArray<int32_t>(unit_ids);
	m->ui.highlighted_units_ids.clear();
	for (auto id: ids) {
		m->ui.highlighted_units_ids.insert(id);
	}
}

void js_set_highlighted_rects(val rectangles_) {
	auto rectangles = emscripten::vecFromJSArray<val>(rectangles_);
	m->ui.highlighted_rectangles.clear();
	for (auto rect: rectangles) {
		auto from = emscripten::vecFromJSArray<size_t>(rect["from"]);
		auto to = emscripten::vecFromJSArray<size_t>(rect["to"]);
		m->ui.highlighted_rectangles.push_back({
			{from[0], from[1]},
			{to[0], to[1]},
		});
	}
}

void js_remove_screen_overlay() {
	m->ui.overlay.reset();
}

val get_screen_info() {
	val v = val::object();
	v.set("screen_x", m->ui.screen_pos.x);
	v.set("screen_y", m->ui.screen_pos.y);
	v.set("screen_width", m->ui.screen_width);
	v.set("screen_height", m->ui.screen_height);
	v.set("cursor_x", m->ui.cursor_pos.x);
	v.set("cursor_y", m->ui.cursor_pos.y);
	return v;
}

void reset_replay() {
	m->reset();
	m->ui.reset();
	m->ui.set_image_data();
	m->run_main_update_loop = true;
	any_replay_loaded = false;
}

EMSCRIPTEN_BINDINGS(openbw) {
	register_vector<js_unit>("vector_js_unit");
	class_<util_functions>("util_functions")
		.function("worker_supply", &util_functions::worker_supply)
		.function("army_supply", &util_functions::army_supply)
		.function("get_all_incomplete_units", &util_functions::get_all_incomplete_units, allow_raw_pointers())
		.function("get_all_completed_units", &util_functions::get_all_completed_units, allow_raw_pointers())
		.function("get_all_units", &util_functions::get_all_units, allow_raw_pointers())
		.function("get_completed_upgrades", &util_functions::get_completed_upgrades)
		.function("get_completed_research", &util_functions::get_completed_research)
		.function("get_incomplete_upgrades", &util_functions::get_incomplete_upgrades)
		.function("get_incomplete_research", &util_functions::get_incomplete_research)
		;
	function("get_util_funcs", &get_util_funcs);

	function("set_volume", &set_volume);
	function("get_volume", &get_volume);

	class_<unit_type_t>("unit_type_t")
		.property("id", &unit_type_t_id)
		.property("build_time", &unit_type_t::build_time)
		;

	class_<unit_t>("unit_t")
		.property("owner", &unit_t::owner)
		.property("remaining_build_time", &unit_t::remaining_build_time)
		.function("unit_type", &unit_t_unit_type, allow_raw_pointers())
		.function("build_type", &unit_t_build_type, allow_raw_pointers())
		;

	function("get_selected_units", &get_selected_units);
	function("select_unit_by_bw_id", &select_unit_by_bw_id);
	function("set_screen_center_position", &set_screen_center_position);
	function("clear_selection", &clear_selection);
	function("enable_main_update_loop", &enable_main_update_loop);
	function("lookup_unit", &lookup_unit);
	function("lookup_unit_extended", &lookup_unit_extended);
	function("reset_replay", &reset_replay);

	// Draw commands
	function("add_draw_command", &js_add_draw_command);
	function("clear_draw_commands", &js_clear_draw_commands);
	function("remove_screen_overlay", &js_remove_screen_overlay);
	function("set_highlighted_units", &js_set_highlighted_units);
	function("set_highlighted_rects", &js_set_highlighted_rects);
	function("get_screen_info", &get_screen_info);

	function("get_units_matcher", &get_units_matcher, allow_raw_pointers());
	function("has_replay_loaded", &has_replay_loaded);
	class_<unit_matcher>("unit_matcher")
		.function("add_unit", &unit_matcher::add_unit)
		.function("do_matching", &unit_matcher::do_matching)
		.function("get_matching", &unit_matcher::get_matching);
}

extern "C" double player_get_value(int player, int index) {
	if (player < 0 || player >= 12) return 0;
	switch (index) {
	case 0:
		return m->ui.st.players.at(player).controller == player_t::controller_occupied ? 1 : 0;
	case 1:
		return (double)m->ui.st.players.at(player).color;
	case 2:
		return (double)(uintptr_t)m->ui.replay_st.player_name.at(player).data();
	case 3:
		return m->ui.st.supply_used.at(player)[0].raw_value / 2.0;
	case 4:
		return m->ui.st.supply_used.at(player)[1].raw_value / 2.0;
	case 5:
		return m->ui.st.supply_used.at(player)[2].raw_value / 2.0;
	case 6:
		return std::min(m->ui.st.supply_available.at(player)[0].raw_value / 2.0, 200.0);
	case 7:
		return std::min(m->ui.st.supply_available.at(player)[1].raw_value / 2.0, 200.0);
	case 8:
		return std::min(m->ui.st.supply_available.at(player)[2].raw_value / 2.0, 200.0);
	case 9:
		return (double)m->ui.st.current_minerals.at(player);
	case 10:
		return (double)m->ui.st.current_gas.at(player);
	case 11:
		return util_functions(m->ui.st).worker_supply(player);
	case 12:
		return util_functions(m->ui.st).army_supply(player);
	case 13:
		return (double)(int)m->ui.st.players.at(player).race;
	case 14:
		return (double)m->ui.apm.at(player).current_apm;
	default:
		return 0;
	}
}

extern "C" void load_replay(const uint8_t* data, size_t len) {
	m->reset();
	m->ui.load_replay_data(data, len);
	m->ui.set_image_data();
	m->run_main_update_loop = false;
	any_replay_loaded = true;
}

#endif

int main() {

	using namespace bwgame;

	log("v29\n");

	size_t screen_width = 1280;
	size_t screen_height = 800;

	std::chrono::high_resolution_clock clock;
	auto start = clock.now();

#ifdef EMSCRIPTEN
	if (current_width != -1) {
		screen_width = current_width;
		screen_height = current_height;
	}
	auto load_data_file = data_loading::data_files_directory<data_loading::data_files_loader<data_loading::mpq_file<data_loading::js_file_reader<>>>>("");
#else
	auto load_data_file = data_loading::data_files_directory("");
#endif

	game_player player(load_data_file);

	main_t m(std::move(player));
	auto& ui = m.ui;

	m.ui.load_all_image_data(load_data_file);

	ui.load_data_file = [&](a_vector<uint8_t>& data, a_string filename) {
		load_data_file(data, std::move(filename));
	};

	ui.init();

#ifndef EMSCRIPTEN
	ui.load_replay_file("maps/p49.rep");
#endif
	auto& wnd = ui.wnd;
	wnd.create("test", 0, 0, screen_width, screen_height);

	ui.resize(screen_width, screen_height);
	ui.screen_pos = {(int)ui.game_st.map_width / 2 - (int)screen_width / 2, (int)ui.game_st.map_height / 2 - (int)screen_height / 2};

	ui.set_image_data();

	log("loaded in %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - start).count());

	//set_malloc_fail_handler(malloc_fail_handler);
#ifdef EMSCRIPTEN
	::m = &m;
	::g_m = &m;
	//EM_ASM({js_load_done();});
	emscripten_set_main_loop_arg([](void* ptr) {
		if (!any_replay_loaded) return;
		EM_ASM({js_pre_main_loop();});
		((main_t*)ptr)->update();
		EM_ASM({js_post_main_loop();});
	}, &m, 0, 1);
#else
	::g_m = &m;
	while (true) {
		m.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
#endif
	::g_m = nullptr;

	return 0;
}
