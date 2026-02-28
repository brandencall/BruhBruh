add_library(systems
    bullet_system.cpp
    collision_system.cpp
    damage_system.cpp
)

target_include_directories(systems
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(systems
    PUBLIC
        entities
        raylib
)
