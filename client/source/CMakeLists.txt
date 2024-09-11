cmake_minimum_required(VERSION 3.28)
project(hic-cli-source VERSION 1.0.0)

aux_source_directory(. DIR_hic_cli_source)

add_library(${PROJECT_NAME} STATIC ${DIR_hic_cli_source})

target_link_libraries(${PROJECT_NAME}
	hic
)