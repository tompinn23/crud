cmake_minimum_required(VERSION 3.20)
project(unit)

add_executable(unitd src/unitd.c src/log.c)
target_compile_definitions(unitd PRIVATE UN_LOG_SRC_DIR="${CMAKE_SOURCE_DIR}/")
add_executable(unitc src/unitc.c src/log.c)
target_compile_definitions(unitc PRIVATE UN_LOG_SRC_DIR="${CMAKE_SOURCE_DIR}/")
