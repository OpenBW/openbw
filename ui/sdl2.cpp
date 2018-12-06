#include "native_window.h"
#include "native_window_drawing.h"
#include "native_sound.h"

#include "common.h"
#include "SDL.h"
#ifndef OPENBW_NO_SDL_IMAGE
#include "SDL_image.h"
#endif
#ifndef OPENBW_NO_SDL_MIXER
#include "SDL_mixer.h"
#endif

#include <array>
#include <cstdlib>
#include <memory>
#include <csignal>

using bwgame::ui::log;
using bwgame::ui::fatal_error;

namespace native_window {

bool sdl_initialized = false;
void sdl_init() {
	if (!sdl_initialized) {
#ifndef OPENBW_NO_SDL_IMAGE
		IMG_Init(IMG_INIT_PNG);
#endif
		auto original_handler = signal(SIGINT, SIG_DFL);
		if (SDL_Init(SDL_INIT_VIDEO) == 0) {
			sdl_initialized = true;
		} else {
			log("SDL_Init failed: %s\n", SDL_GetError());
		}
		signal(SIGINT, original_handler);
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

	void destroy() {
		if (window) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}

	bool create(const char* title, int x, int y, int width, int height) {
		if (window) fatal_error("window already created");
		Uint32 flags = 0;
		flags |= SDL_WINDOW_RESIZABLE;
		window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
		if (!window) log("SDL_CreateWindow failed: %s\n", SDL_GetError());
		#ifdef EMSCRIPTEN
		SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
		// Keyboard events are handled by the html side
		// We need to disable these events, otherwise Emscripten captures
		//  keyboard events on the whole page, and makes HTML <input> useless
		SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
		SDL_EventState(SDL_KEYUP, SDL_DISABLE);
		SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
		#else
		if (window) {
			SDL_StartTextInput();
		}
		#endif
		return window != nullptr;
	}

	void get_cursor_pos(int* x, int* y) {
		SDL_GetMouseState(x, y);
	}

	std::array<bool, 512> key_state {};
	std::array<bool, 6> mouse_button_state {};

	bool peek_message(event_t& e) {
		SDL_Event sdl_e;
		while (SDL_PollEvent(&sdl_e)) {
			switch (sdl_e.type) {
			case SDL_MOUSEMOTION:
				e.type = event_t::type_mouse_motion;
				e.button_state = sdl_e.motion.state;
				e.mouse_x = sdl_e.motion.x;
				e.mouse_y = sdl_e.motion.y;
				e.mouse_xrel = sdl_e.motion.xrel;
				e.mouse_yrel = sdl_e.motion.yrel;
				return true;
			case SDL_MOUSEBUTTONDOWN:
				e.type = event_t::type_mouse_button_down;
				e.button = sdl_e.button.button;
				e.mouse_x = sdl_e.button.x;
				e.mouse_y = sdl_e.button.y;
				e.clicks = sdl_e.button.clicks;
				if ((size_t)e.button < mouse_button_state.size()) mouse_button_state[e.button] = true;
				return true;
			case SDL_MOUSEBUTTONUP:
				e.type = event_t::type_mouse_button_up;
				e.button = sdl_e.button.button;
				e.mouse_x = sdl_e.button.x;
				e.mouse_y = sdl_e.button.y;
				e.clicks = sdl_e.button.clicks;
				if ((size_t)e.button < mouse_button_state.size()) mouse_button_state[e.button] = false;
				return true;
			case SDL_KEYDOWN:
				e.type = event_t::type_key_down;
				e.sym = sdl_e.key.keysym.sym;
				e.scancode = sdl_e.key.keysym.scancode;
				if ((size_t)e.scancode < key_state.size()) key_state[e.scancode] = true;
				return true;
			case SDL_KEYUP:
				e.type = event_t::type_key_up;
				e.sym = sdl_e.key.keysym.sym;
				e.scancode = sdl_e.key.keysym.scancode;
				if ((size_t)e.scancode < key_state.size()) key_state[e.scancode] = false;
				return true;
			case SDL_TEXTINPUT:
				break;
			case SDL_QUIT:
				e.type = event_t::type_quit;
				return true;
			case SDL_WINDOWEVENT:
				if (sdl_e.window.windowID == SDL_GetWindowID(window)) {
					if (sdl_e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || sdl_e.window.event == SDL_WINDOWEVENT_RESIZED) {
						e.type = event_t::type_resize;
						e.width = sdl_e.window.data1;
						e.height = sdl_e.window.data2;
						return true;
					}
				}
				break;
			}
		}
		return false;
	}

	bool show_cursor(bool show) {
		return SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE) ? true : false;
	}

	bool get_key_state(int scancode) {
		return key_state.at(scancode) ? true : false;
	}

	bool get_mouse_button_state(int button) {
		return mouse_button_state.at(button) ? true : false;
	}

	void update_surface() {
		SDL_UpdateWindowSurface(window);
	}

	explicit operator bool() const {
		return window != nullptr;
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

void window::destroy() {
	impl->destroy();
}

bool window::create(const char* title, int x, int y, int width, int height) {
	return impl->create(title, x, y, width, height);
}

void window::get_cursor_pos(int* x, int* y) {
	return impl->get_cursor_pos(x, y);
}

bool window::peek_message(event_t& e) {
	return impl->peek_message(e);
}

bool window::show_cursor(bool show) {
	return impl->show_cursor(show);
}

bool window::get_key_state(int scancode) {
	return impl->get_key_state(scancode);
}

bool window::get_mouse_button_state(int button) {
	return impl->get_mouse_button_state(button);
}

void window::update_surface() {
	return impl->update_surface();
}

window::operator bool() const {
	return (bool)*impl;
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

struct sdl_surface: surface {
	SDL_Surface* surf = nullptr;
	void set(SDL_Surface* surf) {
		this->surf = surf;
		w = surf->w;
		h = surf->h;
		pitch = surf->pitch;
	}

	virtual ~sdl_surface() override {
		if (surf) SDL_FreeSurface(surf);
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
	}
	virtual void blit(surface* dst, int x, int y) override {
		auto* s = ((sdl_surface*)dst)->surf;
		if (x == 0 && y == 0) {
			SDL_BlitSurface(surf, nullptr, s, nullptr);
		} else {
			SDL_Rect r{ x, y, s->w, s->h };
			SDL_BlitSurface(surf, nullptr, s, &r);
		}
	}
	virtual void blit_scaled(surface* dst, int x, int y, int w, int h) override {
		auto* s = ((sdl_surface*)dst)->surf;
		SDL_Rect r{x, y, w, h};
		SDL_BlitScaled(surf, nullptr, s, &r);
	}
	virtual void fill(int r, int g, int b, int a) override {
		SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, r, g, b, a));
	}
	virtual void set_alpha(int a) override {
		SDL_SetSurfaceAlphaMod(surf, a);
	}
	virtual void set_blend_mode(blend_mode blend) override {
		if (blend == blend_mode::none) SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_NONE);
		else if (blend == blend_mode::alpha) SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
		else if (blend == blend_mode::add) SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_ADD);
		else if (blend == blend_mode::mod) SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_MOD);
	}
};

std::unique_ptr<surface> create_rgba_surface(int width, int height) {
	//SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGBA32);
	Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    SDL_Surface* surf = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
	if (!surf) fatal_error("SDL_CreateRGBSurfaceWithFormat failed: %s", SDL_GetError());
	auto r = std::make_unique<sdl_surface>();
	r->set(surf);
	return std::unique_ptr<surface>(r.release());
}

std::unique_ptr<surface> get_window_surface(native_window::window* wnd) {
	auto* surf = SDL_GetWindowSurface(wnd->impl->window);
	if (!surf) fatal_error("SDL_GetWindowSurface failed: %s", SDL_GetError());
	auto r = std::make_unique<sdl_surface>();
	r->set(surf);
	return std::unique_ptr<surface>(r.release());
}

std::unique_ptr<surface> convert_to_8_bit_indexed(surface* s) {
	auto* surf = SDL_ConvertSurfaceFormat(((sdl_surface*)s)->surf, SDL_PIXELFORMAT_INDEX8, 0);
	if (!surf) fatal_error("SDL_ConvertSurfaceFormat failed: %s", SDL_GetError());
	auto r = std::make_unique<sdl_surface>();
	r->set(surf);
	return std::unique_ptr<surface>(r.release());
}

palette* new_palette() {
	return new palette_impl();
}
void delete_palette(palette* pal) {
	delete pal;
}

std::unique_ptr<surface> load_image(const char* filename) {
#ifndef OPENBW_NO_SDL_IMAGE
	auto* surf = IMG_Load(filename);
	if (!surf) fatal_error("IMG_Load(%s) failed: %s", filename, IMG_GetError());
	auto r = std::make_unique<sdl_surface>();
	r->set(surf);
	return std::unique_ptr<surface>(r.release());
#else
	return nullptr;
#endif
}

std::unique_ptr<surface> load_image(const void* data, size_t size) {
#ifndef OPENBW_NO_SDL_IMAGE
	auto* surf = IMG_Load_RW(SDL_RWFromConstMem(data, (int)size), 1);
	if (!surf) fatal_error("IMG_Load_RW(mem) failed: %s", IMG_GetError());
	auto r = std::make_unique<sdl_surface>();
	r->set(surf);
	return std::unique_ptr<surface>(r.release());
#else
	return nullptr;
#endif
}
}

namespace native_sound {

bool initialized = false;

int frequency = 0;
int channels = 64;

#ifndef OPENBW_NO_SDL_MIXER

void init() {
	if (initialized) return;
	initialized = true;
	Mix_Init(0);
	int freq = frequency;
	if (freq == 0) {
//#ifdef EMSCRIPTEN
#if 0
		freq = EM_ASM_INT_V({
			var context;
			try {
				context = new AudioContext();
			} catch (e) {
				context = new webkitAudioContext();
			}
			return context.sampleRate;
		});
#else
		freq = MIX_DEFAULT_FREQUENCY;
#endif
	}
	Mix_OpenAudio(freq, MIX_DEFAULT_FORMAT, 2, 1024);
	Mix_AllocateChannels(channels);
}

struct sdl_sound: sound {
	Mix_Chunk* c = nullptr;
	virtual ~sdl_sound() override {
		if (c) Mix_FreeChunk(c);
	}
};

void play(int channel, sound* arg_s, int volume, int pan) {
	if (!initialized) init();
	sdl_sound* s = (sdl_sound*)arg_s;
	if (!s || !s->c) return;
	int c = Mix_PlayChannel(channel, s->c, 0);
	if (c != -1) {
		Mix_Volume(c, volume);
		//int left = 254;
		//int right = 254;
		//if (pan < 0) right += pan;
		//else left -= pan;
		//Mix_SetPanning(c, left, right);
	}
}

bool is_playing(int channel) {
	return Mix_Playing(channel) != 0;
}

void stop(int channel) {
	Mix_HaltChannel(channel);
}

void set_volume(int channel, int volume) {
	Mix_Volume(channel, volume);
}

std::unique_ptr<sound> load_wav(const void* data, size_t size) {
	if (!initialized) init();
	Mix_Chunk* c = Mix_LoadWAV_RW(SDL_RWFromConstMem(data, (int)size), 1);
	if (!c) return {};
	auto r = std::make_unique<sdl_sound>();
	r->c = c;
	return std::unique_ptr<sound>(r.release());
}

#else

void init() {
}

struct sdl_sound: sound {
	virtual ~sdl_sound() override {}
};

void play(int channel, sound* arg_s, int volume, int pan) {
}

bool is_playing(int channel) {
	return false;
}

void stop(int channel) {
}

void set_volume(int channel, int volume) {
}

std::unique_ptr<sound> load_wav(const void* data, size_t size) {
	return nullptr;
}

#endif

}
