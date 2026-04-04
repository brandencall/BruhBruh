add_library(scenes
    game_scene.cpp
)

target_include_directories(scenes
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(scenes
    PUBLIC
        raylib
)
