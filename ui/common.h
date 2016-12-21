#ifndef COMMON_H
#define COMMON_H

#include "containers.h"
#include "strf.h"

#include <mutex>

void log_str(bwgame::a_string str);
void fatal_error_str(bwgame::a_string str);

template<typename...T>
bwgame::a_string format(const char*fmt, T&&...args) {
	bwgame::a_string str;
	bwgame::strf::format(str, fmt, std::forward<T>(args)...);
	return str;
}

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

#endif
