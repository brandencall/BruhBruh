cmake_minimum_required(VERSION 3.20)
project(BoysGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(external/raylib)
add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/shared)
add_subdirectory(src/network)

# Main executable
add_executable(BruhBruh src/main.cpp)

# Link internal modules
target_link_libraries(BruhBruh
    PRIVATE
        client
        server
        shared
        network
        raylib
)
