add_library(screens
    death_screen.cpp
)

target_include_directories(screens
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(screens
    PUBLIC
        shared
        raylib
)
