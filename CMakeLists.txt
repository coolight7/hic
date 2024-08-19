cmake_minimum_required(VERSION 3.28)
project(hic VERSION 1.0.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++23")

add_subdirectory(src)
add_subdirectory(test)