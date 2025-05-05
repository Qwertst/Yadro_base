cmake_minimum_required(VERSION 3.16)
project(club)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -g")
include_directories(src)

add_executable(club
    src/main.cpp
    src/club_manager.cpp
)
