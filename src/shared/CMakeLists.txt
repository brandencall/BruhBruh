
add_library(shared INTERFACE)

add_subdirectory(systems)

target_link_libraries(shared
    INTERFACE
        systems
        raylib
)

