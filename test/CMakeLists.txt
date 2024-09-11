cmake_minimum_required(VERSION 3.28)
project(test VERSION 1.0.0)

aux_source_directory(. DIR_test)

add_executable(${PROJECT_NAME} ${DIR_test})

target_link_libraries(${PROJECT_NAME}
	hic
	hic-cli-source
)