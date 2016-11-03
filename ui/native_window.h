#ifndef WINDOW_H

#include <memory>

namespace native_window {
	struct window_impl;

	struct window {
		std::unique_ptr<window_impl> impl;
		window();
		~window();
		window(window&&);
		bool create(const char* title, int x, int y, int width, int height);
		void get_cursor_pos(int* x, int* y);
		bool peek_message();
		bool show_cursor(bool show);
		bool get_key_state(int vkey);
	};
}

#endif

