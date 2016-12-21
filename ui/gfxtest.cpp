#include "common.h"
#include "bwgame.h"
#include "replay.h"

#include "native_window.h"
#include "native_window_drawing.h"

#include <chrono>
#include <thread>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace bwgame;

FILE* log_file = nullptr;

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

struct vr4_entry {
	using bitmap_t = std::conditional<is_native_fast_int<uint64_t>::value, uint64_t, uint32_t>::type;
	std::array<bitmap_t, 64 / sizeof(bitmap_t)> bitmap;
	std::array<bitmap_t, 64 / sizeof(bitmap_t)> inverted_bitmap;
};
struct vx4_entry {
	std::array<uint16_t, 16> images;
};

struct pcx_image {
	size_t width;
	size_t height;
	a_vector<uint8_t> data;
};

struct tileset_image_data {
	a_vector<uint8_t> wpe;
	a_vector<vr4_entry> vr4;
	a_vector<vx4_entry> vx4;
	pcx_image dark_pcx;
	std::array<pcx_image, 7> light_pcx;
	grp_t creep_grp;
};

struct image_data {
	std::array<std::array<uint8_t, 8>, 16> player_unit_colors;
	std::array<uint8_t, 16> player_minimap_colors;
	std::array<int, 0x100> creep_edge_frame_index{};
	
	a_vector<std::unique_ptr<native_window_drawing::surface>> valkyrie_test;
	a_vector<std::unique_ptr<native_window_drawing::surface>> valkyrie_test_flipped;
};

template<typename data_T>
pcx_image load_pcx_data(const data_T& data) {
	data_loading::data_reader_le r(data.data(), data.data() + data.size());
	auto base_r = r;
	auto id = r.get<uint8_t>();
	if (id != 0x0a) xcept("pcx: invalid identifier %#x", id);
	r.get<uint8_t>(); // version
	auto encoding = r.get<uint8_t>(); // encoding
	auto bpp = r.get<uint8_t>(); // bpp
	auto offset_x = r.get<uint16_t>();
	auto offset_y = r.get<uint16_t>();
	auto last_x = r.get<uint16_t>();
	auto last_y = r.get<uint16_t>();

	if (encoding != 1) xcept("pcx: invalid encoding %#x", encoding);
	if (bpp != 8) xcept("pcx: bpp is %d, expected 8", bpp);

	if (offset_x != 0 || offset_y != 0) xcept("pcx: offset %d %d, expected 0 0", offset_x, offset_y);

	r.skip(2 + 2 + 48 + 1);

	auto bit_planes = r.get<uint8_t>();
	auto bytes_per_line = r.get<uint16_t>();

	size_t width = last_x + 1;
	size_t height = last_y + 1;

	pcx_image pcx;
	pcx.width = width;
	pcx.height = height;

	pcx.data.resize(width * height);

	r = base_r;
	r.skip(128);

	auto padding = bytes_per_line * bit_planes - width;
	if (padding != 0) xcept("pcx: padding not supported");

	uint8_t* dst = pcx.data.data();
	uint8_t* dst_end = pcx.data.data() + pcx.data.size();

	while (dst != dst_end) {
		auto v = r.get<uint8_t>();
		if ((v & 0xc0) == 0xc0) {
			v &= 0x3f;
			auto c = r.get<uint8_t>();
			for (; v; --v) {
				if (dst == dst_end) xcept("pcx: failed to decode");
				*dst++ = c;
			}
		} else {
			*dst = v;
			++dst;
		}
	}

	return pcx;
}

template<typename load_data_file_F>
void load_image_data(image_data& img, load_data_file_F&& load_data_file) {
	
	std::array<int, 0x100> creep_edge_neighbors_index{};
	std::array<int, 128> creep_edge_neighbors_index_n{};
	
	for (size_t i = 0; i != 0x100; ++i) {
		int v = 0;
		if (i & 2) v |= 0x10;
		if (i & 8) v |= 0x24;
		if (i & 0x10) v |= 9;
		if (i & 0x40) v |= 2;
		if ((i & 0xc0) == 0xc0) v |= 1;
		if ((i & 0x60) == 0x60) v |= 4;
		if ((i & 3) == 3) v |= 0x20;
		if ((i & 6) == 6) v |= 8;
		if ((v & 0x21) == 0x21) v |= 0x40;
		if ((v & 0xc) == 0xc) v |= 0x40;
		creep_edge_neighbors_index[i] = v;
	}
	
	int n = 0;
	for (int i = 0; i != 128; ++i) {
		auto it = std::find(creep_edge_neighbors_index.begin(), creep_edge_neighbors_index.end(), i);
		if (it == creep_edge_neighbors_index.end()) continue;
		creep_edge_neighbors_index_n[i] = n;
		++n;
	}
	
	for (size_t i = 0; i != 0x100; ++i) {
		img.creep_edge_frame_index[i] = creep_edge_neighbors_index_n[creep_edge_neighbors_index[i]];
	}
	
	a_vector<uint8_t> tmp_data;
	auto load_pcx_file = [&](a_string filename) {
		load_data_file(tmp_data, std::move(filename));
		return load_pcx_data(tmp_data);
	};
	
	auto tunit_pcx = load_pcx_file("game/tunit.pcx");
	if (tunit_pcx.width != 128 || tunit_pcx.height != 1) xcept("tunit.pcx dimensions are %dx%d (128x1 required)", tunit_pcx.width, tunit_pcx.height);
	for (size_t i = 0; i != 16; ++i) {
		for (size_t i2 = 0; i2 != 8; ++i2) {
			img.player_unit_colors[i][i2] = tunit_pcx.data[i * 8 + i2];
		}
	}
	auto tminimap_pcx = load_pcx_file("game/tminimap.pcx");
	if (tminimap_pcx.width != 16 || tminimap_pcx.height != 1) xcept("tminimap.pcx dimensions are %dx%d (16x16 required)", tminimap_pcx.width, tminimap_pcx.height);
	for (size_t i = 0; i != 16; ++i) {
		img.player_minimap_colors[i] = tminimap_pcx.data[i];
	}
	
	auto flip = [&](auto& dst, auto& src) {
		dst.resize(src.size());
		for (size_t i = 0; i != dst.size(); ++i) {
			auto tmp = native_window_drawing::create_rgba_surface(src[i]->w, src[i]->h);
			src[i]->blit(&*tmp, 0, 0);
			void* ptr = tmp->lock();
			uint32_t* pixels = (uint32_t*)ptr;
			for (size_t y = 0; y != tmp->h; ++y) {
				for (size_t x = 0; x != tmp->w / 2; ++x) {
					std::swap(pixels[x], pixels[tmp->w - x]);
				}
				pixels += tmp->pitch / 4;
			}
			tmp->unlock();
			dst[i] = std::move(tmp);
		}
	};
	
	img.valkyrie_test.resize(17);
	for (size_t i = 0; i != img.valkyrie_test.size(); ++i) {
		auto fn = format("data/images/bw_hd_valk/%d.png", i);
		img.valkyrie_test[i] = native_window_drawing::load_image(fn.c_str());
	}
	flip(img.valkyrie_test_flipped, img.valkyrie_test);
	
}

template<typename load_data_file_F>
void load_tileset_image_data(tileset_image_data& img, size_t tileset_index, load_data_file_F&& load_data_file) {
	using namespace data_loading;

	std::array<const char*, 8> tileset_names = {
		"badlands", "platform", "install", "AshWorld", "Jungle", "Desert", "Ice", "Twilight"
	};

	a_vector<uint8_t> vr4_data;
	a_vector<uint8_t> vx4_data;

	const char* tileset_name = tileset_names.at(tileset_index);

	load_data_file(vr4_data, format("Tileset/%s.vr4", tileset_name));
	load_data_file(vx4_data, format("Tileset/%s.vx4", tileset_name));
	load_data_file(img.wpe, format("Tileset/%s.wpe", tileset_name));
	
	a_vector<uint8_t> grp_data;
	load_data_file(grp_data, format("Tileset/%s.grp", tileset_name));
	img.creep_grp = read_grp(data_loading::data_reader_le(grp_data.data(), grp_data.data() + grp_data.size()));

	data_reader<true, false> vr4_r(vr4_data.data(), nullptr);
	img.vr4.resize(vr4_data.size() / 64);
	for (size_t i = 0; i != img.vr4.size(); ++i) {
		for (size_t i2 = 0; i2 != 8; ++i2) {
			auto r2 = vr4_r;
			auto v = vr4_r.get<uint64_t, true>();
			auto iv = r2.get<uint64_t, false>();
			size_t n = 8 / sizeof(vr4_entry::bitmap_t);
			for (size_t i3 = 0; i3 != n; ++i3) {
				img.vr4[i].bitmap[i2 * n + i3] = (vr4_entry::bitmap_t)v;
				img.vr4[i].inverted_bitmap[i2 * n + i3] = (vr4_entry::bitmap_t)iv;
				v >>= n == 1 ? 0 : 8 * sizeof(vr4_entry::bitmap_t);
				iv >>= n == 1 ? 0 : 8 * sizeof(vr4_entry::bitmap_t);
			}
		}
	}
	data_reader<true, false> vx4_r(vx4_data.data(), vx4_data.data() + vx4_data.size());
	img.vx4.resize(vx4_data.size() / 32);
	for (size_t i = 0; i != img.vx4.size(); ++i) {
		for (size_t i2 = 0; i2 != 16; ++i2) {
			img.vx4[i].images[i2] = vx4_r.get<uint16_t>();
		}
	}

	a_vector<uint8_t> tmp_data;
	auto load_pcx_file = [&](a_string filename) {
		load_data_file(tmp_data, std::move(filename));
		return load_pcx_data(tmp_data);
	};

	img.dark_pcx = load_pcx_file(format("Tileset/%s/dark.pcx", tileset_name));
	if (img.dark_pcx.width != 256 || img.dark_pcx.height != 32) xcept("invalid dark.pcx");
	for (size_t x = 0; x != 256; ++x) {
		img.dark_pcx.data[256 * 31 + x] = (uint8_t)x;
	}
	
	std::array<const char*, 7> light_names = {"ofire", "gfire", "bfire", "bexpl", "trans50", "red", "green"};
	for (size_t i = 0; i != 7; ++i) {
		img.light_pcx[i] = load_pcx_file(format("Tileset/%s/%s.pcx", tileset_name, light_names[i]));
	}
}

template<bool bounds_check>
void draw_tile(tileset_image_data& img, size_t megatile_index, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height) {
	auto* images = &img.vx4.at(megatile_index).images[0];
	size_t x = 0;
	size_t y = 0;
	for (size_t image_iy = 0; image_iy != 4; ++image_iy) {
		for (size_t image_ix = 0; image_ix != 4; ++image_ix) {
			auto image_index = *images;
			bool inverted = (image_index & 1) == 1;
			auto* bitmap = inverted ? &img.vr4.at(image_index / 2).inverted_bitmap[0] : &img.vr4.at(image_index / 2).bitmap[0];

			for (size_t iy = 0; iy != 8; ++iy) {
				for (size_t iv = 0; iv != 8 / sizeof(vr4_entry::bitmap_t); ++iv) {
					for (size_t b = 0; b != sizeof(vr4_entry::bitmap_t); ++b) {
						if (!bounds_check || (x >= offset_x && y >= offset_y && x < width && y < height)) {
							*dst = (uint8_t)(*bitmap >> (8 * b));
						}
						++dst;
						++x;
					}
					++bitmap;
				}
				x -= 8;
				++y;
				dst -= 8;
				dst += pitch;
			}
			x += 8;
			y -= 8;
			dst -= pitch * 8;
			dst += 8;
			++images;
		}
		x -= 32;
		y += 8;
		dst += pitch * 8;
		dst -= 32;
	}
}

void draw_tile(tileset_image_data& img, size_t megatile_index, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height) {
	if (offset_x == 0 && offset_y == 0 && width == 32 && height == 32) {
		draw_tile<false>(img, megatile_index, dst, pitch, offset_x, offset_y, width, height);
	} else {
		draw_tile<true>(img, megatile_index, dst, pitch, offset_x, offset_y, width, height);
	}
}

template<bool bounds_check, bool flipped, typename remap_F>
void draw_frame(const grp_t::frame_t& frame, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height, remap_F&& remap_f) {
	for (size_t y = 0; y != offset_y; ++y) {
		dst += pitch;
	}

	for (size_t y = offset_y; y != height; ++y) {

		if (flipped) dst += frame.size.x - 1;

		const uint8_t* d = frame.data_container.data() + frame.line_data_offset.at(y);
		for (size_t x = flipped ? frame.size.x - 1 : 0; x != (flipped ? (size_t)0 - 1 : frame.size.x);) {
			int v = *d++;
			if (v & 0x80) {
				v &= 0x7f;
				x += flipped ? -v : v;
				dst += flipped ? -v : v;
			} else if (v & 0x40) {
				v &= 0x3f;
				int c = *d++;
				for (;v; --v) {
					if (!bounds_check || (x >= offset_x && x < width)) {
						*dst = remap_f(c, *dst);
					}
					dst += flipped ? -1 : 1;
					x += flipped ? -1 : 1;
				}
			} else {
				for (;v; --v) {
					int c = *d++;
					if (!bounds_check || (x >= offset_x && x < width)) {
						*dst = remap_f(c, *dst);
					}
					dst += flipped ? -1 : 1;
					x += flipped ? -1 : 1;
				}
			}
		}

		if (!flipped) dst -= frame.size.x;
		else ++dst;
		dst += pitch;

	}
}

struct no_remap {
	uint8_t operator()(uint8_t new_value, uint8_t old_value) const {
		return new_value;
	}
};

template<typename remap_F = no_remap>
void draw_frame(const grp_t::frame_t& frame, bool flipped, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height, remap_F&& remap_f = remap_F()) {

	if (offset_x == 0 && offset_y == 0 && width == frame.size.x && height == frame.size.y) {
		if (flipped) draw_frame<false, true>(frame, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
		else draw_frame<false, false>(frame, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	} else {
		if (flipped) draw_frame<true, true>(frame, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
		else draw_frame<true, false>(frame, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	}

}

struct apm_t {
	a_deque<int> history;
	int current_apm = 0;
	int last_frame_div = 0;
	static const int resolution = 1;
	void add_action(int frame) {
		if (!history.empty() && frame / resolution == last_frame_div) {
			++history.back();
		} else {
			if (history.size() >= 10 * 1000 / 42 / resolution) history.pop_front();
			history.push_back(1);
			last_frame_div = frame / 12;
		}
	}
	void update(int frame) {
		if (history.empty() || frame / resolution != last_frame_div) {
			if (history.size() >= 10 * 1000 / 42 / resolution) history.pop_front();
			history.push_back(0);
			last_frame_div = frame / resolution;
		}
		if (frame % resolution) return;
		//log("history.size() is %d\n", history.size());
		if (history.size() == 0) {
			current_apm = 0;
			return;
		}
		int sum = 0;
		for (auto& v : history) sum += v;
		current_apm = (int)(sum * ((int64_t)256 * 60 * 1000 / 42 / resolution) / history.size() / 256);
	}
};

struct ui_functions: replay_functions {
	image_data img;
	tileset_image_data tileset_img;
	native_window::window wnd;

	xy screen_pos;

	size_t screen_width;
	size_t screen_height;
	
	game_player player;
	replay_state current_replay_state;
	action_state current_action_state;
	std::array<apm_t, 12> apm;
	ui_functions(game_player player) : replay_functions(player.st(), current_action_state, current_replay_state), player(std::move(player)) {
		init();
	}
	
	a_vector<const image_t*> image_draw_queue;
	
	bool use_new_images = false;
	
	bool is_new_image(const image_t* image) {
		if (!use_new_images) return false;
		switch (image->image_type->id) {
		case ImageTypes::IMAGEID_Valkyrie: return true;
		default:
			return false;
		}
	}
	
	a_vector<uint8_t> creep_random_tile_indices = a_vector<uint8_t>(256 * 256);
	void init() {
		uint32_t rand_state = (uint32_t)clock.now().time_since_epoch().count();
		auto rand = [&]() {
			rand_state = rand_state * 22695477 + 1;
			return (rand_state >> 16) & 0x7fff;
		};
		for (auto& v : creep_random_tile_indices) {
			if (rand() % 100 < 4) v = 6 + rand() % 7;
			else v = rand() % 6;
		}
	}
	
	virtual void on_action(int owner, int action) override {
		apm.at(owner).add_action(st.current_frame);
	}

	rect_t<xy_t<size_t>> screen_tile_bounds() {
		size_t from_tile_y = screen_pos.y / 32u;
		if (from_tile_y >= game_st.map_tile_height) from_tile_y = 0;
		size_t to_tile_y = (screen_pos.y + screen_height + 31) / 32u;
		if (to_tile_y > game_st.map_tile_height) to_tile_y = game_st.map_tile_height;
		size_t from_tile_x = screen_pos.x / 32u;
		if (from_tile_x >= game_st.map_tile_width) from_tile_y = 0;
		size_t to_tile_x = (screen_pos.x + screen_width + 31) / 32u;
		if (to_tile_x > game_st.map_tile_width) to_tile_x = game_st.map_tile_width;

		return {{from_tile_x, from_tile_y}, {to_tile_x, to_tile_y}};
	}

	void draw_tiles(uint8_t* data, size_t data_pitch) {

		auto screen_tile = screen_tile_bounds();

		size_t tile_index = screen_tile.from.y * game_st.map_tile_width + screen_tile.from.x;
		auto* megatile_index = &st.tiles_mega_tile_index[tile_index];
		auto* tile = &st.tiles[tile_index];
		size_t width = screen_tile.to.x - screen_tile.from.x;
		
		xy dirs[9] = {{1, 1}, {0, 1}, {-1, 1}, {1, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}, {0, 0}};

		for (size_t tile_y = screen_tile.from.y; tile_y != screen_tile.to.y; ++tile_y) {
			for (size_t tile_x = screen_tile.from.x; tile_x != screen_tile.to.x; ++tile_x) {

				int screen_x = tile_x * 32 - screen_pos.x;
				int screen_y = tile_y * 32 - screen_pos.y;

				size_t offset_x = 0;
				size_t offset_y = 0;
				if (screen_x < 0) {
					offset_x = -screen_x;
				}
				if (screen_y < 0) {
					offset_y = -screen_y;
				}

				uint8_t* dst = data + screen_y * data_pitch + screen_x;

				size_t width = 32;
				size_t height = 32;

				width = std::min(width, screen_width - screen_x);
				height = std::min(height, screen_height - screen_y);

				size_t index = *megatile_index;
				if (tile->flags & tile_t::flag_has_creep) {
					index = game_st.cv5.at(1).mega_tile_index[creep_random_tile_indices[tile_x + tile_y * game_st.map_tile_width]];
				}
				draw_tile(tileset_img, index, dst, data_pitch, offset_x, offset_y, width, height);
				
				if (~tile->flags & tile_t::flag_has_creep) {
					size_t creep_index = 0;
					for (size_t i = 0; i != 9; ++i) {
						int add_x = dirs[i].x;
						int add_y = dirs[i].y;
						if (tile_x + add_x >= game_st.map_tile_width) continue;
						if (tile_y + add_y >= game_st.map_tile_height) continue;
						if (st.tiles[tile_x + add_x + (tile_y + add_y) * game_st.map_tile_width].flags & tile_t::flag_has_creep) creep_index |= 1 << i;
					}
					size_t creep_frame = img.creep_edge_frame_index[creep_index];
					
					if (creep_frame) {
						
						auto& frame = tileset_img.creep_grp.frames.at(creep_frame - 1);
						
						screen_x += frame.offset.x;
						screen_y += frame.offset.y;
						
						size_t width = frame.size.x;
						size_t height = frame.size.y;
				
						if (screen_x < (int)screen_width && screen_y < (int)screen_height) {
							if (screen_x + (int)width > 0 && screen_y + (int)height > 0) {
								
								size_t offset_x = 0;
								size_t offset_y = 0;
								if (screen_x < 0) {
									offset_x = -screen_x;
								}
								if (screen_y < 0) {
									offset_y = -screen_y;
								}
								
								uint8_t* dst = data + screen_y * data_pitch + screen_x;
					
								width = std::min(width, screen_width - screen_x);
								height = std::min(height, screen_height - screen_y);
								
								draw_frame(frame, false, dst, data_pitch, offset_x, offset_y, width, height);
							}
						}
					}
				}

				++megatile_index;
				++tile;
			}
			megatile_index -= width;
			megatile_index += game_st.map_tile_width;
			tile -= width;
			tile += game_st.map_tile_width;
		}
	}

	void draw_image(const image_t* image, uint8_t* data, size_t data_pitch, size_t color_index) {
		
		if (is_new_image(image)) {
			image_draw_queue.push_back(image);
			return;
		}
		
		xy map_pos = get_image_map_position(image);

		int screen_x = map_pos.x - screen_pos.x;
		int screen_y = map_pos.y - screen_pos.y;

		if (screen_x >= (int)screen_width || screen_y >= (int)screen_height) return;

		auto& frame = image->grp->frames.at(image->frame_index);

		size_t width = frame.size.x;
		size_t height = frame.size.y;

		if (screen_x + (int)width <= 0 || screen_y + (int)height <= 0) return;

		size_t offset_x = 0;
		size_t offset_y = 0;
		if (screen_x < 0) {
			offset_x = -screen_x;
		}
		if (screen_y < 0) {
			offset_y = -screen_y;
		}

		uint8_t* dst = data + screen_y * data_pitch + screen_x;

		width = std::min(width, screen_width - screen_x);
		height = std::min(height, screen_height - screen_y);

		if (image->modifier == 0 || image->modifier == 1 || image->modifier == 2 || image->modifier == 3 || image->modifier == 4) {
			uint8_t* ptr = img.player_unit_colors.at(color_index).data();
			auto player_color = [ptr](uint8_t new_value, uint8_t) {
				if (new_value >= 8 && new_value < 16) return ptr[new_value - 8];
				return new_value;
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, player_color);
		} else if (image->modifier == 10) {
			uint8_t* ptr = &tileset_img.dark_pcx.data[256 * 18];
			auto shadow = [ptr](uint8_t, uint8_t old_value) {
				return ptr[old_value];
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, shadow);
		} else if (image->modifier == 9) {
			size_t index = image->image_type->color_shift;
			auto& data = tileset_img.light_pcx.at(index - 1).data;
			uint8_t* ptr = data.data();
			size_t size = data.size() / 256;
			auto glow = [ptr, size](uint8_t new_value, uint8_t old_value) {
				if (new_value >= size) return (uint8_t)0;
				return ptr[256u * new_value + old_value];
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, glow);
		} else xcept("don't know how to draw image modifier %d", image->modifier);

	}

	void draw_sprite(const sprite_t* sprite, uint8_t* data, size_t data_pitch) {
		for (auto* image : ptr(reverse(sprite->images))) {
			if (i_flag(image, image_t::flag_hidden)) continue;
			draw_image(image, data, data_pitch, st.players[sprite->owner].color);
		}
	}

	void draw_sprites(uint8_t* data, size_t data_pitch) {
		
		image_draw_queue.clear();

		a_vector<std::pair<uint_fast32_t, const sprite_t*>> sorted_sprites;
		
		auto screen_tile = screen_tile_bounds();

		size_t from_y = screen_tile.from.y;
		if (from_y < 4) from_y = 0;
		else from_y -= 4;
		size_t to_y = screen_tile.to.y;
		if (to_y >= game_st.map_tile_height - 4) to_y = game_st.map_tile_height - 1;
		else to_y += 4;
		for (size_t y = from_y; y != to_y; ++y) {
			for (auto* sprite : ptr(st.sprites_on_tile_line.at(y))) {
				if (s_hidden(sprite)) continue;
				uint_fast32_t score = 0;
				score |= sprite->elevation_level;
				score <<= 13;
				score |= sprite->elevation_level <= 4 ? sprite->position.y : 0;
				score <<= 1;
				score |= s_flag(sprite, sprite_t::flag_turret) ? 1 : 0;
				sorted_sprites.emplace_back(score, sprite);
			}
		}

		std::sort(sorted_sprites.begin(), sorted_sprites.end());

		for (auto& v : sorted_sprites) {
			draw_sprite(v.second, data, data_pitch);
		}

	}
	
	void fill_rectangle(uint8_t* data, size_t data_pitch, rect area, uint8_t index) {
		if (area.from.x < 0) area.from.x = 0;
		if (area.from.y < 0) area.from.y = 0;
		if (area.to.x > (int)screen_width) area.to.x = screen_width;
		if (area.to.y > (int)screen_height) area.to.y = screen_height;
		if (area.from.x >= area.to.x || area.from.y >= area.to.y) return;
		size_t width = area.to.x - area.from.x;
		size_t pitch = data_pitch;
		size_t from_y = area.from.y;
		size_t to_y = area.to.y;
		uint8_t* ptr = data + screen_width * from_y + area.from.x;
		for (size_t i = from_y; i != to_y; ++i) {
			memset(ptr, index, width);
			ptr += pitch;
		}
	}
	
	void line_rectangle(uint8_t* data, size_t data_pitch, rect area, uint8_t index) {
		size_t width = area.to.x - area.from.x;
		size_t height = area.to.y - area.from.y;
		uint8_t* p = data + screen_width * (size_t)area.from.y + (size_t)area.from.x;
		memset(p, index, width);
		memset(p + data_pitch * height, index, width);
		for (size_t y = 0; y != height; ++y) {
			p[data_pitch * y] = index;
			p[data_pitch * y + width - 1] = index;
		}
	}
	
	bool unit_visble_on_minimap(unit_t* u) {
		if (ut_turret(u)) return false;
		if (unit_is_trap(u)) return false;
		if (unit_is(u, UnitTypes::Spell_Dark_Swarm)) return false;
		if (unit_is(u, UnitTypes::Spell_Disruption_Web)) return false;
		return true;
	}
	
	rect get_minimap_area() {
		size_t minimap_width = std::max(game_st.map_tile_width, game_st.map_tile_height);
		size_t minimap_height = std::max(game_st.map_tile_width, game_st.map_tile_height);
		if (game_st.map_width < game_st.map_height) {
			minimap_width = minimap_width * minimap_width * game_st.map_tile_width / (minimap_height * game_st.map_tile_height);
		} else if (game_st.map_height < game_st.map_width) {
			minimap_height = minimap_height * minimap_height * game_st.map_tile_height / (minimap_width* game_st.map_tile_width);
		}
		if (screen_width < minimap_width || screen_height < minimap_height) return {};
		int map_screen_x = 4;
		int map_screen_y = screen_height - 4 - minimap_height;
		rect area;
		area.from = {map_screen_x, map_screen_y};
		area.to = area.from + xy{(int)minimap_width, (int)minimap_height};
		return area;
	}

	void draw_minimap(uint8_t* data, size_t data_pitch) {
		auto area = get_minimap_area();
		size_t minimap_width = area.to.x - area.from.x;
		size_t minimap_height = area.to.y - area.from.y;
		if (minimap_width != game_st.map_tile_width) return;
		if (minimap_height != game_st.map_tile_height) return;
		fill_rectangle(data, data_pitch, area, 0);
		line_rectangle(data, data_pitch, {area.from - xy(1, 1), area.to + xy(1, 1)}, 0);
		
		uint8_t* p = data + screen_width * (size_t)area.from.y + (size_t)area.from.x;
		
		size_t pitch = screen_width - game_st.map_tile_width;
		for (size_t y = 0; y != game_st.map_tile_height; ++y) {
			for (size_t x = 0; x != game_st.map_tile_width; ++x) {
				auto* images = &tileset_img.vx4.at(st.tiles_mega_tile_index[y * game_st.map_tile_width + x]).images[0];
				auto* bitmap = &tileset_img.vr4.at(*images / 2).bitmap[0];
				auto val = bitmap[55 / sizeof(vr4_entry::bitmap_t)];
				size_t shift = 8 * (55 % sizeof(vr4_entry::bitmap_t));
				val >>= shift;
				*p++ = (uint8_t)val;
			}
			p += pitch;
		}
		
		for (size_t i = 12; i != 0;) {
			--i;
			for (unit_t* u : ptr(st.player_units[i])) {
				if (!unit_visble_on_minimap(u)) continue;
				int color = img.player_minimap_colors.at(st.players[u->owner].color);
				size_t w = u->unit_type->placement_size.x / 32u;
				size_t h = u->unit_type->placement_size.y / 32u;
				if (ut_building(u)) {
					if (w > 2) w = 4;
					if (h > 2) h = 4;
				} else {
					if (w < 2) w = 2;
					if (h < 2) h = 2;
				}
				rect unit_area;
				unit_area.from = area.from + (u->sprite->position - u->unit_type->placement_size / 2) / 32u;
				unit_area.to = unit_area.from + xy(w, h);
				fill_rectangle(data, data_pitch, unit_area, color);
			}
		}
		
		rect screen_rect;
		screen_rect.from = area.from + xy(screen_pos.x / 32u, screen_pos.y / 32u);
		screen_rect.to = screen_rect.from + xy((screen_width + 31) / 32u, (screen_height + 31) / 32u);
		line_rectangle(data, data_pitch, screen_rect, 255);
		
	}
	
	int replay_frame = 0;
	
	rect get_replay_slider_area() {
#ifdef EMSCRIPTEN
		return {};
#endif
		rect r;
		int width = 192;
		int height = 32;
		r.from.x = (int)screen_width - 8 - width;
		r.from.y = (int)screen_height - 8 - 128 - height;
		r.to.x = r.from.x + width;
		r.to.y = r.from.y + height;
		if (r.from.x < 0 || r.from.y < 0) return {};
		return r;
	}
	
	void draw_ui(uint8_t* data, size_t data_pitch) {
		auto area = get_replay_slider_area();
		if (area == rect{}) return;
		fill_rectangle(data, data_pitch, area, 1);
		line_rectangle(data, data_pitch, area, 12);
		
		int button_w = 16;
		int button_h = 32;
		int ow = (area.to.x - area.from.x) - button_w;
		int ox = replay_frame * ow / replay_st.end_frame;
		
		if (st.current_frame != replay_frame) {
			int cox = st.current_frame * ow / replay_st.end_frame;
			line_rectangle(data, data_pitch, rect{area.from + xy(cox + button_w / 2, 0), area.from + xy(cox + button_w / 2 + 1, button_h)}, 50);
		}
		
		fill_rectangle(data, data_pitch, rect{area.from + xy(ox, 0), area.from + xy(ox, 0) + xy(button_w, button_h)}, 10);
		line_rectangle(data, data_pitch, rect{area.from + xy(ox, 0), area.from + xy(ox, 0) + xy(button_w, button_h)}, 51);
		
	}
	
	void draw_new_image(const image_t* image) {
		size_t frame_index = image->frame_index;
		
		xy map_pos = get_image_map_position(image);

		int screen_x = map_pos.x - screen_pos.x;
		int screen_y = map_pos.y - screen_pos.y;
		
		int w = 54;
		int h = 45;
//		int w = 65;
//		int h = 54;
		
		auto& frame = image->grp->frames.at(image->frame_index);

		int width = (int)frame.size.x;
		int height = (int)frame.size.y;
		
		screen_x -= (w - width) / 2;
		screen_y -= (h - height) / 2;
		
		if (screen_x >= (int)screen_width || screen_y >= (int)screen_height) return;
		if (screen_x + (int)w <= 0 || screen_y + (int)h <= 0) return;
		
		if (i_flag(image, image_t::flag_horizontally_flipped)) {
			img.valkyrie_test_flipped[frame_index]->blit_scaled(&*window_surface, screen_x, screen_y, w, h);
		} else {
			img.valkyrie_test[frame_index]->blit_scaled(&*window_surface, screen_x, screen_y, w, h);
		}
	}
	
	void draw_image_queue() {
		
		for (auto* image : image_draw_queue) {
			draw_new_image(image);
		}
//		//img.valkyrie_test[0]->blit(&*window_surface, 0, 0);
//		img.valkyrie_test[8]->blit_scaled(&*window_surface, 100, 100, 54, 45);
		
//		auto* g = global_st.image_grp[(int)ImageTypes::IMAGEID_Valkyrie];
//		log("valk is %d %d\n", g->width, g->height);
	}
	
	fp8 game_speed = fp8::integer(1);

	std::unique_ptr<native_window_drawing::surface> window_surface;
	std::unique_ptr<native_window_drawing::surface> indexed_surface;
	native_window_drawing::palette* palette = nullptr;
	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point last_draw;
	std::chrono::high_resolution_clock::time_point last_input_poll;
	std::chrono::high_resolution_clock::time_point last_fps;
	int fps_counter = 0;
	size_t scroll_speed_n = 0;
	
	void resize(int width, int height) {
		screen_width = width;
		screen_height = height;
		window_surface.reset();
		indexed_surface.reset();
	}
	
	bool is_moving_minimap = false;
	bool is_moving_replay_slider = false;
	bool is_paused = false;
	
	void update() {
		auto now = clock.now();
		
		if (now - last_fps >= std::chrono::seconds(1)) {
			//log("draw fps: %g\n", fps_counter / std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - last_fps).count());
			last_fps = now;
			fps_counter = 0;
		}
		++fps_counter;
		
		auto minimap_area = get_minimap_area();
		auto replay_slider_area = get_replay_slider_area();
		
		auto move_minimap = [&](int mouse_x, int mouse_y) {
			if (mouse_x < minimap_area.from.x) mouse_x = minimap_area.from.x;
			else if (mouse_x >= minimap_area.to.x) mouse_x = minimap_area.to.x - 1;
			if (mouse_y < minimap_area.from.y) mouse_y = minimap_area.from.y;
			else if (mouse_y >= minimap_area.to.y) mouse_y = minimap_area.to.y - 1;
			int x = mouse_x - minimap_area.from.x;
			int y = mouse_y - minimap_area.from.y;
			x = x * game_st.map_tile_width / (minimap_area.to.x - minimap_area.from.x);
			y = y * game_st.map_tile_height / (minimap_area.to.y - minimap_area.from.y);
			screen_pos = xy(32 * x - screen_width / 2, 32 * y - screen_height / 2);
		};
		
		auto check_move_minimap = [&](auto& e) {
			if (e.mouse_x >= minimap_area.from.x && e.mouse_x < minimap_area.to.x) {
				if (e.mouse_y >= minimap_area.from.y && e.mouse_y < minimap_area.to.y) {
					is_moving_minimap = true;
					move_minimap(e.mouse_x, e.mouse_y);
				}
			}
		};
		
		auto move_replay_slider = [&](int mouse_x, int mouse_y) {
			(void)mouse_y;
			int x = mouse_x - replay_slider_area.from.x;
			int button_w = 16;
			x -= button_w / 2;
			int ow = (replay_slider_area.to.x - replay_slider_area.from.x) - button_w;
			if (x < 0) x = 0;
			if (x >= ow) x = ow - 1;
			replay_frame = x * replay_st.end_frame / ow;
			
		};
		
		auto check_move_replay_slider = [&](auto& e) {
			if (e.mouse_x >= replay_slider_area.from.x && e.mouse_x < replay_slider_area.to.x) {
				if (e.mouse_y >= replay_slider_area.from.y && e.mouse_y < replay_slider_area.to.y) {
					is_moving_replay_slider = true;
					move_replay_slider(e.mouse_x, e.mouse_y);
				}
			}
		};
	
		native_window::event_t e;
		while (wnd.peek_message(e)) {
			switch (e.type) {
			case native_window::event_t::type_quit:
				exit(0);
				break;
			case native_window::event_t::type_resize:
				resize(e.width, e.height);
				break;
			case native_window::event_t::type_mouse_button_down:
				if (e.button == 1) {
					check_move_minimap(e);
					check_move_replay_slider(e);
				}
				break;
			case native_window::event_t::type_mouse_motion:
				if (e.button_state & 1) {
					check_move_minimap(e);
					check_move_replay_slider(e);
				} else if (e.button_state & 4) {
					screen_pos -= xy(e.mouse_xrel, e.mouse_yrel);
				}
				break;
			case native_window::event_t::type_mouse_button_up:
				if (e.button == 1) {
					if (is_moving_minimap) is_moving_minimap = false;
					if (is_moving_replay_slider) is_moving_replay_slider = false;
				}
				break;
			case native_window::event_t::type_key_down:
				if (e.sym == 'q') {
					use_new_images = !use_new_images;
				}
#ifndef EMSCRIPTEN
				if (e.sym == ' ' || e.sym == 'p') {
					is_paused = !is_paused;
				}
				if (e.sym == 'a' || e.sym == 'u') {
					if (game_speed < fp8::integer(128)) game_speed *= 2;
				}
				if (e.sym == 'z' || e.sym == 'd') {
					if (game_speed > 2_fp8) game_speed /= 2;
				}
				if (e.sym == '\b') {
					int t = 5 * 42 / 1000;
					if (replay_frame < t) replay_frame = 0;
					else replay_frame -= t;
				}
#endif
				break;
			}
		}
		
		if (!window_surface) {
			window_surface = native_window_drawing::get_window_surface(&wnd);
			indexed_surface = native_window_drawing::convert_to_8_bit_indexed(&*window_surface);
			indexed_surface->set_palette(palette);
			
			screen_width = indexed_surface->pitch;
		}
		
		auto input_poll_speed = std::chrono::milliseconds(12);
		
		auto input_poll_t = now - last_input_poll;
		if (input_poll_t >= input_poll_speed) {
			last_input_poll = now;
			if (input_poll_t >= input_poll_speed * 2) last_input_poll = now - input_poll_speed;
			else last_input_poll += input_poll_speed;
			std::array<int, 6> scroll_speeds = {4, 4, 8, 12, 12, 16};
			
			fp8 mult = fp8::integer(std::chrono::duration_cast<std::chrono::milliseconds>(input_poll_t).count()) / fp8::integer(std::chrono::milliseconds(24).count());
			for (auto& v : scroll_speeds) v = (fp8::integer(v) * mult).integer_part();
			int scroll_speed = scroll_speeds[scroll_speed_n];
			auto prev_screen_pos = screen_pos;
			if (wnd.get_key_state(81)) screen_pos.y += scroll_speed;
			else if (wnd.get_key_state(82)) screen_pos.y -= scroll_speed;
			if (wnd.get_key_state(79)) screen_pos.x += scroll_speed;
			else if (wnd.get_key_state(80)) screen_pos.x -= scroll_speed;
			if (screen_pos != prev_screen_pos) {
				if (scroll_speed_n != scroll_speeds.size() - 1) ++scroll_speed_n;
			} else scroll_speed_n = 0;
			
			if (is_moving_minimap) {
				int x = -1;
				int y = -1;
				wnd.get_cursor_pos(&x, &y);
				if (x != -1) move_minimap(x, y);
			}
			if (is_moving_replay_slider) {
				int x = -1;
				int y = -1;
				wnd.get_cursor_pos(&x, &y);
				if (x != -1) move_replay_slider(x, y);
			}
		}
		if (screen_pos.y + screen_height > game_st.map_height) screen_pos.y = game_st.map_height - screen_height;
		else if (screen_pos.y < 0) screen_pos.y = 0;
		if (screen_pos.x + screen_width > game_st.map_width) screen_pos.x = game_st.map_width - screen_width;
		else if (screen_pos.x < 0) screen_pos.x = 0;

		uint8_t* data = (uint8_t*)indexed_surface->lock();
		draw_tiles(data, indexed_surface->pitch);
		draw_sprites(data, indexed_surface->pitch);
		draw_minimap(data, indexed_surface->pitch);
		draw_ui(data, indexed_surface->pitch);
		indexed_surface->unlock();
		
		indexed_surface->blit(&*window_surface, 0, 0);
		
		draw_image_queue();
		
		wnd.update_surface();
	}
};

struct saved_state {
	state st;
	action_state action_st;
	std::array<apm_t, 12> apm;
};

struct main_t {
	ui_functions ui;
	
	main_t(game_player player) : ui(std::move(player)) {}
	
	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point last_tick;
	
	std::chrono::high_resolution_clock::time_point last_fps;
	int fps_counter = 0;
	
	a_map<int, std::unique_ptr<saved_state>> saved_states;
	
	std::array<tileset_image_data, 8> tileset_img;
	
	template<typename load_data_file_F>
	void load_all_image_data(load_data_file_F&& load_data_file) {
		load_image_data(ui.img, load_data_file);
		for (size_t i = 0; i != 8; ++i) {
			load_tileset_image_data(tileset_img[i], i, load_data_file);
		}
	}
	
	void set_image_data() {
		ui.tileset_img = tileset_img.at(ui.game_st.tileset_index);
		
		native_window_drawing::color palette_colors[256];
		if (ui.tileset_img.wpe.size() != 256 * 4) xcept("wpe size invalid (%d)", ui.tileset_img.wpe.size());
		for (size_t i = 0; i != 256; ++i) {
			palette_colors[i].r = ui.tileset_img.wpe[4 * i + 0];
			palette_colors[i].g = ui.tileset_img.wpe[4 * i + 1];
			palette_colors[i].b = ui.tileset_img.wpe[4 * i + 2];
			palette_colors[i].a = ui.tileset_img.wpe[4 * i + 3];
		}
		ui.palette->set_colors(palette_colors);
	}
	
	void reset() {
		saved_states.clear();
		ui.apm = {};
		ui.replay_frame = 0;
		auto& game = *ui.st.game;
		ui.st = state();
		game = game_state();
		ui.replay_st = replay_state();
		ui.action_st = action_state();
		
		ui.st.global = &ui.global_st;
		ui.st.game = &game;
	}
	
	void update() {
		auto now = clock.now();
		
		auto tick_speed = std::chrono::milliseconds((fp8::integer(42) / ui.game_speed).integer_part());
		
		if (now - last_fps >= std::chrono::seconds(1)) {
			//log("game fps: %g\n", fps_counter / std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - last_fps).count());
			last_fps = now;
			fps_counter = 0;
		}
		
		auto next = [&]() {
			int save_interval = ui.replay_st.end_frame / 20;
			if (ui.st.current_frame == 0 || ui.st.current_frame % save_interval == 0) {
				auto i = saved_states.find(ui.st.current_frame);
				if (i == saved_states.end()) {
					auto v = std::make_unique<saved_state>();
					v->st = copy_state(ui.st);
					v->action_st = copy_state(ui.action_st, ui.st, v->st);
					v->apm = ui.apm;
					saved_states[ui.st.current_frame] = std::move(v);
				}
			}
			ui.replay_functions::next_frame();
			for (auto& v : ui.apm) v.update(ui.st.current_frame);
		};
		
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
					for (size_t i = 0; i != 128 && ui.st.current_frame != ui.replay_frame; ++i) {
						next();
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
						
						if (!ui.is_done()) next();
						else break;
					}
					ui.replay_frame = ui.st.current_frame;
				}
			}
		}
		
		ui.update();
	}
};

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
		else xcept("js_file_reader: unknown filename '%s'", filename);
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

#include <bind.h>
#include <val.h>
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

EMSCRIPTEN_BINDINGS(openbw) {
	register_vector<js_unit>("vector_js_unit");
	class_<util_functions>("util_functions")
		.function("worker_supply", &util_functions::worker_supply)
		.function("army_supply", &util_functions::army_supply)
		.function("get_all_incomplete_units", &util_functions::get_all_incomplete_units, allow_raw_pointers())
		.function("get_all_completed_units", &util_functions::get_all_completed_units, allow_raw_pointers())
		.function("get_all_units", &util_functions::get_all_units, allow_raw_pointers())
		;
	function("get_util_funcs", &get_util_funcs);
	
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

bool any_replay_loaded = false;

extern "C" void load_replay(const uint8_t* data, size_t len) {
	m->reset();
	m->ui.load_replay_data(data, len);
	m->set_image_data();
	any_replay_loaded = true;
}

#endif

int main() {

	using namespace bwgame;
	
	log("v10\n");

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
	
	m.load_all_image_data(load_data_file);

#ifndef EMSCRIPTEN
	ui.load_replay_file("maps/zzvalk.rep");
//	game_load_functions funcs(m.ui.st);
//	funcs.reset();
#endif

	auto& wnd = ui.wnd;
	wnd.create("test", 0, 0, screen_width, screen_height);

	auto* palette = native_window_drawing::new_palette();

	ui.palette = palette;
	ui.screen_width = screen_width;
	ui.screen_height = screen_height;
	ui.screen_pos = {(int)ui.game_st.map_width / 2 - (int)screen_width / 2, (int)ui.game_st.map_height / 2 - (int)screen_height / 2};
	
	m.set_image_data();
	
	log("loaded in %dms\n", std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - start).count());

#ifdef EMSCRIPTEN
	::m = &m;
	EM_ASM({js_load_done();});
	emscripten_set_main_loop_arg([](void* ptr) {
		if (!any_replay_loaded) return;
		EM_ASM({js_pre_main_loop();});
		((main_t*)ptr)->update();
		EM_ASM({js_post_main_loop();});
	}, &m, 0, 1);
#else
	while (true) {
		m.update();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
#endif

	return 0;
}

