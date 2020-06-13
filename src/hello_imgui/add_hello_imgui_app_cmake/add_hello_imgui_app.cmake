if(ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/qt-android-cmake/AddQtAndroidApk.cmake)
endif()

#
# add_hello_imgui_app is a helper function, similar to cmake's "add_executable"
#
# Usage:
# add_hello_imgui_app(app_name file1.cpp file2.cpp ...)
#
# Features:
# * It will automaticaly link the exe to hello_imgui labrary
# * Under android, it uses [qt-android-cmake](https://github.com/LaurentGomila/qt-android-cmake.git)
#   in order to create an apk
function(add_hello_imgui_app)
    set(args ${ARGN})
    list(GET args 0 app_name)
    list(REMOVE_AT args 0)
    set(app_sources ${args})

    if (ANDROID)
        add_library(${app_name} SHARED ${app_sources})
    else()
        add_executable(${app_name} ${app_sources})
    endif()

    if(ANDROID)
        add_qt_android_apk(${app_name}_apk ${app_name})
    endif()

    target_link_libraries(${app_name} PRIVATE hello_imgui)

    message("add_hello_imgui_app
             app_name=${app_name}
             sources=${app_sources}
            ")
endfunction()
