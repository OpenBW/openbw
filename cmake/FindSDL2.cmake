# This defines
# SDL2_FOUND - if SDL2 was found
# SDL2_INCLUDE_DIRS - include directories
# SDL2_LIBRARIES - libraries to link to

find_path(SDL2_INCLUDE_DIR SDL.h PATHS ENV SDL2DIR PATH_SUFFIXES SDL2)
find_library(SDL2_LIBRARY SDL2 PATHS ENV SDL2DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2 DEFAULT_MSG SDL2_LIBRARY SDL2_INCLUDE_DIR)

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY)

set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
set(SDL2_LIBRARIES ${SDL2_LIBRARY})

