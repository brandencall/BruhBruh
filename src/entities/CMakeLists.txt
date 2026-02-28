add_library(entities
    player.cpp
    bullet_entity.cpp
)

target_include_directories(entities
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(entities
    PUBLIC
        components
        raylib
)
