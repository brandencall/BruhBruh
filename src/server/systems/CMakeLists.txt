add_library(server_systems
    spawn_system.cpp
)

target_include_directories(server_systems
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(server_systems
    PRIVATE
       shared
)
