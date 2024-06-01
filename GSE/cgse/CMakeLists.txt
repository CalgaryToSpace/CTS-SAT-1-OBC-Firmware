cmake_minimum_required(VERSION 3.8)

project(cgse)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Debugging enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -std=gnu99 ")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Optimization enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=gnu99 ")
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

# CURSES
find_library(CURSES ncursesw HINTS /usr/local/lib)
include_directories(/usr/local/include)

add_executable(cgse main.c command_history.c telecommand_definitions.c)
target_link_libraries(cgse ${CURSES})

# Add more include directories if needed
target_include_directories(cgse PUBLIC "." "include")

install(TARGETS cgse DESTINATION "$ENV{HOME}/bin")
