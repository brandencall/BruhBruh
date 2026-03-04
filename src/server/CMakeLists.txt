add_library(server
    game_server.cpp
    game_simulation.cpp
)

target_include_directories(server
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server
    PRIVATE
       shared
)
