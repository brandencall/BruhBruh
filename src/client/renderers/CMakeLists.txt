add_library(renderers
    ability_pickup_renderer.cpp
    character_renderer.cpp
    wall_renderer.cpp
)

target_include_directories(renderers
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(renderers
    PUBLIC
        raylib
)
