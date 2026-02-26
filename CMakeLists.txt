cmake_minimum_required(VERSION 3.20)
project(BruhBruh)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/raylib)

add_executable(BruhBruh 
    src/main.cpp
    src/game.cpp
    src/game.hpp
    src/player.cpp
    src/player.hpp
)

target_link_libraries(BruhBruh raylib)
