cmake_minimum_required(VERSION 3.28)
project(test VERSION 1.0.0)

aux_source_directory(. DIR_test)

add_executable(test ${DIR_test})