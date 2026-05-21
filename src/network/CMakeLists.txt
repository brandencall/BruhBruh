add_library(network
    server.cpp
    client.cpp
)

if (WIN32)
    target_link_libraries(network PRIVATE ws2_32 wsock32)
endif()

target_include_directories(network
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(network
    PUBLIC
        raylib
        SteamSDK
)

