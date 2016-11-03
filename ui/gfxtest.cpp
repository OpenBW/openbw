#include "common.h"
#include "bwgame.h"

#include "native_window.h"
#include "native_window_drawing.h"

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
	log("fatal error: %s\n", str);
	std::terminate();
}

native_window_drawing::color palette_colors[256] = {
	{0, 0, 0, 0},
	{39, 39, 59, 0},
	{47, 47, 71, 0},
	{55, 59, 83, 0},
	{47, 51, 75, 0},
	{43, 43, 67, 0},
	{39, 39, 59, 0},
	{83, 87, 119, 0},
	{75, 79, 111, 0},
	{71, 75, 103, 0},
	{79, 83, 115, 0},
	{87, 95, 131, 0},
	{99, 107, 147, 0},
	{107, 119, 163, 0},
	{58, 0, 58, 0},
	{25, 0, 25, 0},
	{44, 36, 24, 0},
	{72, 36, 20, 0},
	{92, 44, 20, 0},
	{112, 48, 20, 0},
	{104, 60, 36, 0},
	{124, 64, 24, 0},
	{120, 76, 44, 0},
	{168, 8, 8, 0},
	{140, 84, 48, 0},
	{132, 96, 68, 0},
	{160, 84, 28, 0},
	{196, 76, 24, 0},
	{188, 104, 36, 0},
	{180, 112, 60, 0},
	{208, 100, 32, 0},
	{220, 148, 52, 0},
	{224, 148, 84, 0},
	{236, 196, 84, 0},
	{52, 68, 40, 0},
	{64, 108, 60, 0},
	{72, 108, 80, 0},
	{76, 128, 80, 0},
	{80, 140, 92, 0},
	{92, 160, 120, 0},
	{0, 0, 24, 0},
	{0, 16, 52, 0},
	{0, 8, 80, 0},
	{36, 52, 72, 0},
	{48, 64, 84, 0},
	{20, 52, 124, 0},
	{52, 76, 108, 0},
	{64, 88, 116, 0},
	{72, 104, 140, 0},
	{0, 112, 156, 0},
	{88, 128, 164, 0},
	{64, 104, 212, 0},
	{24, 172, 184, 0},
	{36, 36, 252, 0},
	{100, 148, 188, 0},
	{112, 168, 204, 0},
	{140, 192, 216, 0},
	{148, 220, 244, 0},
	{172, 220, 232, 0},
	{172, 252, 252, 0},
	{204, 248, 248, 0},
	{252, 252, 0, 0},
	{244, 228, 144, 0},
	{252, 252, 192, 0},
	{12, 12, 12, 0},
	{24, 20, 16, 0},
	{28, 28, 32, 0},
	{40, 40, 48, 0},
	{56, 48, 36, 0},
	{56, 60, 68, 0},
	{76, 64, 48, 0},
	{76, 76, 76, 0},
	{92, 80, 64, 0},
	{88, 88, 88, 0},
	{104, 104, 104, 0},
	{120, 132, 108, 0},
	{104, 148, 108, 0},
	{116, 164, 124, 0},
	{152, 148, 140, 0},
	{144, 184, 148, 0},
	{152, 196, 168, 0},
	{176, 176, 176, 0},
	{172, 204, 176, 0},
	{196, 192, 188, 0},
	{204, 224, 208, 0},
	{240, 240, 240, 0},
	{28, 16, 8, 0},
	{40, 24, 12, 0},
	{52, 16, 8, 0},
	{52, 32, 12, 0},
	{56, 16, 32, 0},
	{52, 40, 32, 0},
	{68, 52, 8, 0},
	{72, 48, 24, 0},
	{96, 0, 0, 0},
	{84, 40, 32, 0},
	{80, 64, 20, 0},
	{92, 84, 20, 0},
	{132, 4, 4, 0},
	{104, 76, 52, 0},
	{124, 56, 48, 0},
	{112, 100, 32, 0},
	{124, 80, 80, 0},
	{164, 52, 28, 0},
	{148, 108, 0, 0},
	{152, 92, 64, 0},
	{140, 128, 52, 0},
	{152, 116, 84, 0},
	{184, 84, 68, 0},
	{176, 144, 24, 0},
	{176, 116, 92, 0},
	{244, 4, 4, 0},
	{200, 120, 84, 0},
	{252, 104, 84, 0},
	{224, 164, 132, 0},
	{252, 148, 104, 0},
	{252, 204, 44, 0},
	{16, 252, 24, 0},
	{12, 0, 32, 0},
	{28, 28, 44, 0},
	{36, 36, 76, 0},
	{40, 44, 104, 0},
	{44, 48, 132, 0},
	{32, 24, 184, 0},
	{52, 60, 172, 0},
	{104, 104, 148, 0},
	{100, 144, 252, 0},
	{124, 172, 252, 0},
	{0, 228, 252, 0},
	{156, 144, 64, 0},
	{168, 148, 84, 0},
	{188, 164, 92, 0},
	{204, 184, 96, 0},
	{232, 216, 128, 0},
	{236, 196, 176, 0},
	{252, 252, 56, 0},
	{252, 252, 124, 0},
	{252, 252, 164, 0},
	{8, 8, 8, 0},
	{16, 16, 16, 0},
	{24, 24, 24, 0},
	{40, 40, 40, 0},
	{52, 52, 52, 0},
	{76, 60, 56, 0},
	{68, 68, 68, 0},
	{72, 72, 88, 0},
	{88, 88, 104, 0},
	{116, 104, 56, 0},
	{120, 100, 92, 0},
	{96, 96, 124, 0},
	{132, 116, 116, 0},
	{132, 132, 156, 0},
	{172, 140, 124, 0},
	{172, 152, 148, 0},
	{144, 144, 184, 0},
	{184, 184, 232, 0},
	{248, 140, 20, 0},
	{16, 84, 60, 0},
	{32, 144, 112, 0},
	{44, 180, 148, 0},
	{4, 32, 100, 0},
	{72, 28, 80, 0},
	{8, 52, 152, 0},
	{104, 48, 120, 0},
	{136, 64, 156, 0},
	{12, 72, 204, 0},
	{188, 184, 52, 0},
	{220, 220, 60, 0},
	{16, 0, 0, 0},
	{36, 0, 0, 0},
	{52, 0, 0, 0},
	{72, 0, 0, 0},
	{96, 24, 4, 0},
	{140, 40, 8, 0},
	{200, 24, 24, 0},
	{224, 44, 44, 0},
	{232, 32, 32, 0},
	{232, 80, 20, 0},
	{252, 32, 32, 0},
	{232, 120, 36, 0},
	{248, 172, 60, 0},
	{0, 20, 0, 0},
	{0, 40, 0, 0},
	{0, 68, 0, 0},
	{0, 100, 0, 0},
	{8, 128, 8, 0},
	{36, 152, 36, 0},
	{60, 156, 60, 0},
	{88, 176, 88, 0},
	{104, 184, 104, 0},
	{128, 196, 128, 0},
	{148, 212, 148, 0},
	{12, 20, 36, 0},
	{36, 60, 100, 0},
	{48, 80, 132, 0},
	{56, 92, 148, 0},
	{72, 116, 180, 0},
	{84, 132, 196, 0},
	{96, 148, 212, 0},
	{120, 180, 236, 0},
	{20, 16, 8, 0},
	{24, 20, 12, 0},
	{40, 48, 12, 0},
	{16, 16, 24, 0},
	{20, 20, 32, 0},
	{44, 44, 64, 0},
	{68, 76, 104, 0},
	{4, 4, 4, 0},
	{28, 24, 16, 0},
	{32, 28, 20, 0},
	{36, 32, 28, 0},
	{48, 40, 28, 0},
	{64, 56, 44, 0},
	{84, 72, 52, 0},
	{104, 92, 76, 0},
	{144, 124, 100, 0},
	{40, 32, 16, 0},
	{44, 36, 20, 0},
	{52, 44, 24, 0},
	{56, 44, 28, 0},
	{60, 48, 28, 0},
	{64, 52, 32, 0},
	{68, 56, 36, 0},
	{80, 68, 36, 0},
	{88, 76, 40, 0},
	{100, 88, 44, 0},
	{12, 16, 4, 0},
	{20, 24, 4, 0},
	{28, 32, 8, 0},
	{32, 40, 12, 0},
	{52, 60, 16, 0},
	{64, 72, 16, 0},
	{32, 32, 48, 0},
	{20, 20, 20, 0},
	{32, 24, 28, 0},
	{32, 32, 32, 0},
	{40, 32, 24, 0},
	{40, 36, 36, 0},
	{48, 44, 44, 0},
	{60, 48, 56, 0},
	{60, 56, 60, 0},
	{72, 60, 48, 0},
	{68, 52, 64, 0},
	{84, 64, 72, 0},
	{92, 100, 100, 0},
	{108, 116, 120, 0},
	{88, 78, 47, 0},
	{77, 67, 44, 0},
	{71, 59, 43, 0},
	{75, 63, 47, 0},
	{83, 67, 51, 0},
	{67, 75, 103, 0},
	{75, 83, 111, 0},
	{83, 91, 123, 0},
	{91, 99, 135, 0},
	{255, 255, 255, 0},
};

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

struct image_data {
	a_vector<vr4_entry> vr4;
	a_vector<vx4_entry> vx4;
	pcx_image dark_pcx;
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
void load_image_data(image_data& img, game_state& game_st, load_data_file_F&& load_data_file) {
	using namespace data_loading;

	std::array<const char*, 8> tileset_names = {
		"badlands", "platform", "install", "AshWorld", "Jungle", "Desert", "Ice", "Twilight"
	};

	a_vector<uint8_t> vr4_data;
	a_vector<uint8_t> vx4_data;

	const char* tileset_name = tileset_names.at(game_st.tileset_index);

	load_data_file(vr4_data, format("Tileset/%s.vr4", tileset_name));
	load_data_file(vx4_data, format("Tileset/%s.vx4", tileset_name));

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

}

template<bool bounds_check>
void draw_tile(image_data& img, size_t megatile_index, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height) {
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

void draw_tile(image_data& img, size_t megatile_index, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height) {
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

#include <chrono>
#include <thread>

struct main_t {
	image_data img;
	native_window::window wnd;

	xy screen_pos;

	size_t screen_width;
	size_t screen_height;

	rect_t<xy_t<size_t>> screen_tile;

	game_player player;
	state& st = *player.st;
	game_state& game_st = *st.game;
	state_functions funcs{st};

	main_t(native_window::window wnd, game_player player) : wnd(std::move(wnd)), player(std::move(player)) {}

	void draw_tiles(uint8_t* data) {

		size_t from_tile_y = screen_pos.y / 32u;
		if (from_tile_y >= game_st.map_tile_height) from_tile_y = 0;
		size_t to_tile_y = (screen_pos.y + screen_height + 31) / 32u;
		if (to_tile_y > game_st.map_tile_height) to_tile_y = game_st.map_tile_height;
		size_t from_tile_x = screen_pos.x / 32u;
		if (from_tile_x >= game_st.map_tile_width) from_tile_y = 0;
		size_t to_tile_x = (screen_pos.x + screen_width + 31) / 32u;
		if (to_tile_x > game_st.map_tile_width) to_tile_x = game_st.map_tile_width;

		screen_tile = {{from_tile_x, from_tile_y}, {to_tile_x, to_tile_y}};

		size_t tile_index = from_tile_y * game_st.map_tile_width + from_tile_x;
		auto* megatile_index = &st.tiles_mega_tile_index[tile_index];
		size_t width = to_tile_x - from_tile_x;

		for (size_t tile_y = from_tile_y; tile_y != to_tile_y; ++tile_y) {
			for (size_t tile_x = from_tile_x; tile_x != to_tile_x; ++tile_x) {

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

				uint8_t* dst = data + screen_y * screen_width + screen_x;

				size_t width = 32;
				size_t height = 32;

				width = std::min(width, screen_width - screen_x);
				height = std::min(height, screen_height - screen_y);

				draw_tile(img, *megatile_index, dst, screen_width, offset_x, offset_y, width, height);

				++megatile_index;
			}
			megatile_index -= width;
			megatile_index += game_st.map_tile_width;
		}
	}

	void draw_image(const image_t* image, uint8_t* data) {
		xy map_pos = funcs.get_image_map_position(image);

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

		uint8_t* dst = data + screen_y * screen_width + screen_x;

		width = std::min(width, screen_width - screen_x);
		height = std::min(height, screen_height - screen_y);

		if (image->modifier == 0) {

			draw_frame(frame, funcs.i_flag(image, image_t::flag_horizontally_flipped), dst, screen_width, offset_x, offset_y, width, height);

		} else if (image->modifier == 10) {

			uint8_t* ptr = &img.dark_pcx.data[256 * 18];
			auto shadow = [ptr](uint8_t, uint8_t old_value) {
				return ptr[old_value];
			};

			draw_frame(frame, funcs.i_flag(image, image_t::flag_horizontally_flipped), dst, screen_width, offset_x, offset_y, width, height, shadow);

		} else xcept("don't know how to draw image modifier %d", image->modifier);

	}

	void draw_sprite(const sprite_t* sprite, uint8_t* data) {
		for (auto* image : ptr(reverse(sprite->images))) {
			if (!funcs.i_flag(image, image_t::flag_hidden)) {
				draw_image(image, data);
			}
		}
	}

	void draw_sprites(uint8_t* data) {

		a_vector<std::pair<uint_fast32_t, const sprite_t*>> sorted_sprites;

		for (size_t y = screen_tile.from.y; y != screen_tile.to.y; ++y) {
			for (auto* sprite : ptr(st.sprites_on_tile_line.at(y))) {
				if (sprite->visibility_flags != 0) {
					uint_fast32_t score = 0;
					score |= sprite->elevation_level;
					score <<= 13;
					score |= sprite->elevation_level <= 4 ? sprite->position.y : 0;
					score <<= 5;
					score |= funcs.s_flag(sprite, (sprite_t::flags_t)0x10) ? 1 : 0;
					//score = sprite->position.y;
					sorted_sprites.emplace_back(score, sprite);
				}
			}
		}

		std::sort(sorted_sprites.begin(), sorted_sprites.end());

		for (auto& v : sorted_sprites) {
			draw_sprite(v.second, data);
		}

	}

	uint32_t random_state = 42;

	auto random() {
		random_state = random_state * 22695477 + 1;
		return (random_state >> 16) & 0x7fff;
	}


	void order_move(int unit_id, int x, int y) {
		funcs.set_unit_order(&st.units[unit_id], funcs.get_order_type(Orders::Move), xy(x, y));
	}

	std::chrono::high_resolution_clock clock;
	template<typename T>
	auto elapsed(T since) {
		return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1000>>>(clock.now() - since).count();
	}

	native_window_drawing::surface* surface = nullptr;
	std::chrono::high_resolution_clock::time_point last_tick;
	void tick() {
		wnd.peek_message();

		auto frame_start = clock.now();
		for (unit_t* u : ptr(st.visible_units)) {
			if (random() % 30 == 0) order_move(u - &st.units.front(), u->position.x - 256 + random() % 512, u->position.y - 256 + random() % 512);
		}
		player.next_frame();
		log("next_frame took %gms\n", elapsed(frame_start));


		auto draw_start = clock.now();
		uint8_t* data = (uint8_t*)surface->lock();
		draw_tiles(data);
		draw_sprites(data);
		log("drawing took %gms\n", elapsed(draw_start));
		surface->unlock();

		auto now = clock.now();
		double t = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1000>>>(now - last_tick).count();
		if (t < 42.0) {
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(42 - t)));
		}
		last_tick = clock.now();
	}
};

int main() {

	using namespace bwgame;

	std::chrono::high_resolution_clock clock;
	auto load_start = clock.now();

	size_t screen_width = 1280;
	size_t screen_height = 800;

	native_window::window wnd;
	wnd.create("test", 0, 0, screen_width, screen_height);

	auto* surface = native_window_drawing::new_surface();
	auto* palette = native_window_drawing::new_palette();
	palette->set_colors(palette_colors);

	surface->create(&wnd);
	surface->set_palette(palette);

	auto load_data_file = data_loading::data_files_directory(".");

	main_t m(std::move(wnd), game_player(load_data_file));
	m.surface = surface;
	m.screen_width = screen_width;
	m.screen_height = screen_height;

	m.player.load_map_file("maps/testone.scm");

	m.screen_pos = {32 * 46 + 16, 32 * 41 + 3};

	load_image_data(m.img, m.game_st, load_data_file);

	auto elapsed = [&](auto since) {
		return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1000>>>(clock.now() - since).count();
	};

	log("loaded in %gms\n", elapsed(load_start));

#ifdef EMSCRIPTEN
	main_t* nm = new main_t(std::move(m));
	emscripten_set_main_loop_arg([](void* ptr) {
		((main_t*)ptr)->tick();
	}, nm, 0, 0);
#else
	while (true) {
		m.tick();
	}
#endif

	return 0;
}

