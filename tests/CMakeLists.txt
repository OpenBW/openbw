cmake_minimum_required(VERSION 3.1)

project(openbw_tests)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("${CMAKE_CURRENT_LIST_DIR}/..")

add_executable(replay_info_test replay_info_test.cpp)

enable_testing()
add_test(NAME replay_info_test COMMAND replay_info_test)
