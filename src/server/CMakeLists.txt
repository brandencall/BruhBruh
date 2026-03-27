add_library(server
    game_server.cpp
    game_simulation.cpp
)

add_subdirectory(network)

target_include_directories(server
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server
    PUBLIC 
       server_network
       shared
)
