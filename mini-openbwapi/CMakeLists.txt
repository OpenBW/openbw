cmake_minimum_required(VERSION 3.1)

option(OPENBW_ENABLE_UI "Enables support for the graphical user interface, requires SDL2")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

if (OPENBW_ENABLE_UI)
	find_package(SDL2 REQUIRED)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${SDL2_INCLUDE_DIR}
)

add_definitions(-DOPENBW_NO_SDL_IMAGE)
add_definitions(-DOPENBW_NO_SDL_MIXER)

set(mini-openbwapi_srcs openbwapi.cpp)

if (OPENBW_ENABLE_UI)
	add_definitions(-DOPENBW_ENABLE_UI)
	list(APPEND mini-openbwapi_srcs ../ui/sdl2.cpp)
endif()

add_library(mini-openbwapi ${mini-openbwapi_srcs})

target_link_libraries(mini-openbwapi
	${SDL2_LIBRARY}
)
