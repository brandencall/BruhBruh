cmake_minimum_required(VERSION 3.20)
project(BoysGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(src/core)
add_subdirectory(src/entities)
add_subdirectory(src/systems)
add_subdirectory(external/raylib)

# Main executable
add_executable(BruhBruh src/main.cpp)

# Link internal modules
target_link_libraries(BruhBruh
    PRIVATE
        core
        entities
        systems
        raylib
)
