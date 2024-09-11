cmake_minimum_required(VERSION 3.28)
project(hic-cli VERSION 1.0.0)

aux_source_directory(. DIR_hic_cli)

add_subdirectory(source)

add_executable(${PROJECT_NAME} ${DIR_hic_cli})

target_link_libraries(${PROJECT_NAME}
	hic-cli-source
)