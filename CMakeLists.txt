cmake_minimum_required(VERSION 3.28)
project(hic VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++23")
add_definitions(-DPROGEAM_ROOT_DIR="${CMAKE_SOURCE_DIR}")

include_directories(.)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(client)
add_subdirectory(test)