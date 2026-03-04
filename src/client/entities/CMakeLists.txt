add_library(entities
    render_player.cpp
)

target_include_directories(entities
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(entities
    PUBLIC
        raylib
)
