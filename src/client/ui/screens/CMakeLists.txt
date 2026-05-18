add_library(screens
    death_screen.cpp
    hud_screen.cpp
    scoreboard.cpp
    game_end_screen.cpp
    friends_invite_screen.cpp
    join_screen.cpp
    confirm_quit_screen.cpp
    pause_menu_screen.cpp
)

target_include_directories(screens
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(screens
    PUBLIC
        client_systems
        shared
        raylib
        SteamSDK
)
