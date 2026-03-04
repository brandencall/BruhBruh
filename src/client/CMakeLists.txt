add_library(client
    game_client.cpp
)

add_subdirectory(entities)
add_subdirectory(systems)

target_include_directories(client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client
    PUBLIC
        entities
        client_systems
        raylib
)

target_link_libraries(client
    PRIVATE
       shared
)
