#ifndef NATIVE_WINDOW_DRAWING_H
#define NATIVE_WINDOW_DRAWING_H

#include <stdint.h>
#include <memory>

namespace native_window {
	struct window;
}

namespace native_window_drawing {

	struct color {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	struct palette {
		virtual ~palette() {}
		virtual void set_colors(color colors[256]) = 0;
	};

	palette* new_palette();
	void delete_palette(palette*);

	enum struct blend_mode {
		none,
		alpha,
		add,
		mod
	};

	struct surface {
		int w;
		int h;
		int pitch;
		virtual ~surface() {}
		virtual void set_palette(palette* pal) = 0;
		virtual void* lock() = 0;
		virtual void unlock() = 0;
		virtual void blit(surface* dst, int x, int y) = 0;
		virtual void blit_scaled(surface* dst, int x, int y, int w, int h) = 0;
		virtual void fill(int r, int g, int b, int a) = 0;
		virtual void set_alpha(int a) = 0;
		virtual void set_blend_mode(blend_mode blend) = 0;
	};

	std::unique_ptr<surface> create_rgba_surface(int width, int height);
	std::unique_ptr<surface> get_window_surface(native_window::window* wnd);
	std::unique_ptr<surface> convert_to_8_bit_indexed(surface* s);

	std::unique_ptr<surface> load_image(const char* filename);
	std::unique_ptr<surface> load_image(const void* data, size_t size);
  std::unique_ptr<surface> create_text(const char* text, uint8_t color[4]);



}

#endif
