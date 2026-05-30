add_library(client
    game_client.cpp
    session_manager.cpp
    game.cpp
)

add_subdirectory(scenes)
add_subdirectory(systems)
add_subdirectory(renderers)
add_subdirectory(ui)

target_include_directories(client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client
    PUBLIC
        scenes
        client_systems
        renderers
        ui
        raylib
        SteamSDK
        shared
)
