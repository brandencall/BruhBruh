add_library(network
    server.cpp
    client.cpp
)

target_include_directories(network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(network
    PUBLIC
        raylib
)

