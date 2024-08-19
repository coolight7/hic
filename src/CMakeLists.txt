cmake_minimum_required(VERSION 3.28)
project(hic VERSION 1.0.0)

aux_source_directory(. DIR_hic)

add_executable(hic ${DIR_hic})