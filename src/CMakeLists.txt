cmake_minimum_required(VERSION 3.28)
project(hic VERSION 1.0.0)

aux_source_directory(. DIR_hic)
aux_source_directory(./magic DIR_magic)
aux_source_directory(./analyse DIR_analyse)
aux_source_directory(./vm DIR_vm)

add_library(${PROJECT_NAME} STATIC ${DIR_hic} ${DIR_magic} ${DIR_analyse} ${DIR_vm})