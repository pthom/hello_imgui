if (IOS)
    include(${CMAKE_CURRENT_LIST_DIR}/ios/hello_imgui_ios.cmake)
endif()
if (ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/android/hello_imgui_android.cmake)
endif()
if (EMSCRIPTEN)
    include(${CMAKE_CURRENT_LIST_DIR}/emscripten/hello_imgui_emscripten.cmake)
endif()

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

    if (ANDROID)
        hello_imgui_android_adapt(${app_name})
    endif()
    if (IOS)
        hello_imgui_ios_adapt(${app_name})
    endif()
    if (EMSCRIPTEN)
        hello_imgui_emscripten_adapt(${app_name})
    endif()

    target_link_libraries(${app_name} PRIVATE hello_imgui)

    message("hello_imgui_add_app
             app_name=${app_name}
             sources=${app_sources}
            ")
endfunction()
