set(STEAM_SDK_ROOT "${CMAKE_SOURCE_DIR}/external/steamworks_sdk/sdk" CACHE PATH "")

add_library(SteamSDK INTERFACE)

target_include_directories(SteamSDK INTERFACE
    "${STEAM_SDK_ROOT}/public"
)

if(WIN32)
    target_link_libraries(SteamSDK INTERFACE
        "${STEAM_SDK_ROOT}/redistributable_bin/win64/steam_api64.lib"
    )
elseif(UNIX)
    target_link_libraries(SteamSDK INTERFACE
        "${STEAM_SDK_ROOT}/redistributable_bin/linux64/libsteam_api.so"
    )
endif()
