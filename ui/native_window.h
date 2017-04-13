#ifndef NATIVE_WINDOW_H
#define NATIVE_WINDOW_H

#include <memory>

namespace native_window {
	struct window_impl;
	
	struct event_t {
		enum {
			type_none,
			type_quit,
			type_key_down,
			type_key_up,
			type_resize,
			type_mouse_button_down,
			type_mouse_button_up,
			type_mouse_motion
		};
		int type = type_none;
		int sym = -1;
		int scancode = -1;
		int width = -1;
		int height = -1;
		int button = -1;
		int mouse_x = -1;
		int mouse_y = -1;
		int button_state = 0;
		int mouse_xrel = -1;
		int mouse_yrel = -1;
		int clicks = -1;
	};

	struct window {
		std::unique_ptr<window_impl> impl;
		window();
		~window();
		window(window&&);
		void destroy();
		bool create(const char* title, int x, int y, int width, int height);
		void get_cursor_pos(int* x, int* y);
		bool peek_message(event_t& e);
		bool show_cursor(bool show);
		bool get_key_state(int scancode);
		bool get_mouse_button_state(int button);
		void update_surface();
		explicit operator bool() const;
	};
}

#endif

