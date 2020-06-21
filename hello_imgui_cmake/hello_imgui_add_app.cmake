include(${CMAKE_CURRENT_LIST_DIR}/ios/hello_imgui_ios.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/android/hello_imgui_android.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/emscripten/hello_imgui_emscripten.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/desktop/hello_imgui_desktop.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/assets/hello_imgui_assets.cmake)

function(hello_imgui_emscripten_add_local_assets app_name)
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        message("hello_imgui_emscripten_add_local_assets: ${app_name} found local assets")
        hello_imgui_bundle_assets(${app_name} ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    endif()
endfunction()

#
# hello_imgui_add_app is a helper function, similar to cmake's "add_executable"
#
# Usage:
# hello_imgui_add_app(app_name file1.cpp file2.cpp ...)
#
# Features:
# * It will automaticaly link the exe to hello_imgui library
function(hello_imgui_add_app)
    set(args ${ARGN})
    list(GET args 0 app_name)
    list(REMOVE_AT args 0)
    set(app_sources ${args})

    if (ANDROID)
        add_library(${app_name} SHARED ${app_sources})
    else()
        add_executable(${app_name} ${app_sources})
    endif()

    set(common_assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_assets)
    hello_imgui_bundle_assets(${app_name} ${common_assets_folder})
    hello_imgui_emscripten_add_local_assets(${app_name})

    hello_imgui_platform_customization(${app_name})

    target_link_libraries(${app_name} PRIVATE hello_imgui)

    message("hello_imgui_add_app
             app_name=${app_name}
             sources=${app_sources}
            ")
endfunction()
