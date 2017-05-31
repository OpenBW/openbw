cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")

find_package(SDL2 REQUIRED)

include_directories(
	..
	${SDL2_INCLUDE_DIRS}
)

add_definitions(-DOPENBW_NO_SDL_IMAGE)
add_definitions(-DOPENBW_NO_SDL_MIXER)

add_definitions(-DOPENBW_ENABLE_UI)

add_library(openbw_ui ./sdl2.cpp)

target_link_libraries(openbw_ui ${SDL2_LIBRARIES})

set_property(TARGET openbw_ui PROPERTY POSITION_INDEPENDENT_CODE ON)
