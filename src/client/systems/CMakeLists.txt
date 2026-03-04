add_library(client_systems
    client_bullet_system.cpp
)

target_include_directories(client_systems
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(client_systems
    PUBLIC
        raylib
)
