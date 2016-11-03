#include "native_window.h"
#include "native_window_drawing.h"

#include "common.h"
#include "SDL.h"
#include <mutex>
#include <array>
#include <cstdlib>

namespace native_window {

std::mutex init_mut;
bool sdl_initialized = false;
void sdl_init() {
	std::lock_guard<std::mutex> l(init_mut);
	if (!sdl_initialized) {
		if (SDL_Init(SDL_INIT_VIDEO) == 0) {
			sdl_initialized = true;
		} else {
			log("SDL_Init failed: %s\n", SDL_GetError());
		}
	}
}


struct window_impl {

	SDL_Window* window = nullptr;

	window_impl() {
		sdl_init();
	}
	~window_impl() {
		if (window) SDL_DestroyWindow(window);
	}

	bool create(const char* title, int x, int y, int width, int height) {
		Uint32 flags = 0;
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		if (!window) log("SDL_CreateWindow failed: %s\n", SDL_GetError());
		if (window) {
			SDL_StartTextInput();
		}
		return window != nullptr;
	}

	void get_cursor_pos(int* x, int* y) {
		SDL_GetMouseState(x, y);
	}

	bool peek_message() {
		SDL_Event e;
		if (!SDL_PollEvent(&e)) return false;
		switch (e.type) {
		case SDL_MOUSEMOTION:
			return true;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			return true;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			return true;
		case SDL_TEXTINPUT:
			return true;
		case SDL_QUIT:
			exit(0);
			return true;
		}
		return false;
	}

	bool show_cursor(bool show) {
		return SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE) ? true : false;
	}

};

window::window() {
	impl = std::make_unique<window_impl>();
}

window::~window() {
}

window::window(window&& n) {
	impl = std::move(n.impl);
}

bool window::create(const char* title, int x, int y, int width, int height) {
	return impl->create(title, x, y, width, height);
}

void window::get_cursor_pos(int* x, int* y) {
	return impl->get_cursor_pos(x, y);
}

bool window::peek_message() {
	return impl->peek_message();
}

bool window::show_cursor(bool show) {
	return impl->show_cursor(show);
}

}

namespace native_window_drawing {

struct palette_impl : palette {
	SDL_Palette* pal = nullptr;
	palette_impl() {
		pal = SDL_AllocPalette(256);
	}
	virtual ~palette_impl() override {
		SDL_FreePalette(pal);
	}
	virtual void set_colors(color colors[256]) override {
		std::array<SDL_Color, 256> col{};
		for (size_t i = 0; i < 256; ++i) {
			col[i].r = colors[i].r;
			col[i].g = colors[i].g;
			col[i].b = colors[i].b;
			//col[i].a = colors[i].a;
		}
		if (SDL_SetPaletteColors(pal, col.data(), 0, 256)) fatal_error("SDL_SetPaletteColors failed: %s", SDL_GetError());
	}
};

struct surface_impl : surface {
	SDL_Surface* window_s = nullptr;
	SDL_Surface* surf = nullptr;
	SDL_Window* window = nullptr;
	virtual void create(native_window::window* wnd) override {
		window = wnd->impl->window;
		window_s = SDL_GetWindowSurface(window);
		if (!window_s) fatal_error("SDL_GetWindowSurface failed: %s", SDL_GetError());

		surf = SDL_ConvertSurfaceFormat(window_s, SDL_PIXELFORMAT_INDEX8, 0);
		if (!surf) fatal_error("SDL_ConvertSurfaceFormat failed: %s", SDL_GetError());
	}
	virtual void set_palette(palette* pal) override {
		if (SDL_SetSurfacePalette(surf, ((palette_impl*)pal)->pal)) fatal_error("SDL_SetSurfacePalette failed: %s", SDL_GetError());
	}
	virtual void* lock() override {
		if (SDL_LockSurface(surf)) fatal_error("SDL_LockSurface failed: %s", SDL_GetError());
		return surf->pixels;
	}
	virtual void unlock() override {
		SDL_UnlockSurface(surf);
		SDL_BlitSurface(surf, nullptr, window_s, nullptr);
		SDL_UpdateWindowSurface(window);
	}
};

palette* new_palette() {
	return new palette_impl();
}
void delete_palette(palette* pal) {
	delete pal;
}
surface* new_surface() {
	return new surface_impl();
}
void delete_surface(surface* surf) {
	delete surf;
}

}
