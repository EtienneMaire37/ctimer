cmake_minimum_required(VERSION 3.15)
project(ctimer C)

set(SRC_DIR src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_COMPILER gcc)

find_package(Curses)

file(GLOB SOURCES ${SRC_DIR}/main.c ${SRC_DIR}/parser.c ${SRC_DIR}/print.c ${SRC_DIR}/input.c ${SRC_DIR}/timer.c)

add_executable(ctimer ${SOURCES})

set_property(TARGET ctimer PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_link_libraries(ctimer ncurses panel menu)
target_include_directories(ctimer PRIVATE ${SRC_DIR})

set_target_properties(ctimer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
