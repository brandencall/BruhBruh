cmake_minimum_required(VERSION 3.20)
project(BoysGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENV{ASAN_OPTIONS} "suppressions=${CMAKE_SOURCE_DIR}/asan.supp:detect_leaks=1")

add_compile_definitions(SUPPORT_WINMM_HIGHRES_TIMER=0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(SteamSDK)

# Add subdirectories
add_subdirectory(external/raylib)
add_subdirectory(src/server)
add_subdirectory(src/shared)
add_subdirectory(src/network)
add_subdirectory(src/client)


add_executable(BruhBruh src/main.cpp)

if(WIN32)
    add_custom_command(TARGET BruhBruh POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${STEAM_SDK_ROOT}/redistributable_bin/win64/steam_api64.dll"
        $<TARGET_FILE_DIR:BruhBruh>
    )
elseif(UNIX)
    add_custom_command(TARGET BruhBruh POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${STEAM_SDK_ROOT}/redistributable_bin/linux64/libsteam_api.so"
        $<TARGET_FILE_DIR:BruhBruh>
    )
endif()

# Link internal modules
target_link_libraries(BruhBruh
    PRIVATE
        client
        server
        shared
        network
        raylib
        SteamSDK
)

add_custom_command(TARGET BruhBruh POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/steam_appid.txt"
    $<TARGET_FILE_DIR:BruhBruh>
)

if(MSVC)
    target_link_options(BruhBruh PRIVATE
        /NODEFAULTLIB:winmm.lib
        /FORCE:MULTIPLE
    )
endif()
