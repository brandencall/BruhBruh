add_library(ui
    ui_manager.cpp
)

add_subdirectory(screens)

target_include_directories(ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ui
    PUBLIC
        screens 
)
