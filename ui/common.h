#ifndef COMMON_H
#define COMMON_H

#include "../containers.h"
#include "../strf.h"
#include "../util.h"

#include <mutex>

namespace bwgame {

namespace ui {

void log_str(a_string str);
void fatal_error_str(a_string str);

template<typename...T>
void log(const char* fmt, T&&... args) {
	log_str(format(fmt, std::forward<T>(args)...));
}


template<typename... T>
void fatal_error(const char* fmt, T&&... args) {
	fatal_error_str(format(fmt, std::forward<T>(args)...));
}

template<typename... T>
void xcept(const char* fmt, T&&... args) {
	fatal_error_str(format(fmt, std::forward<T>(args)...));
}

}

}

#endif
