cmake_minimum_required(VERSION 3.8)

project(cgse)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CURSES
find_library(CURSES ncursesw HINTS /usr/local/lib)
include_directories(/usr/local/include)

add_executable(cgse main.c)
target_link_libraries(cgse ${CURSES})

# Add more include directories if needed
target_include_directories(cgse PUBLIC "${CMAKE_SOURCE_DIR}")

