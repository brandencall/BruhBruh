add_library(components
    health_component.cpp
)

target_include_directories(components
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
