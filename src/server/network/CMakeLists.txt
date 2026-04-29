add_library(server_network
    client_registry.cpp
    packet_handler.cpp
    state_broadcaster.cpp
)


target_include_directories(server_network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server_network
    PRIVATE
       shared
)
