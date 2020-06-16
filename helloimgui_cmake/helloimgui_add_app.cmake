if(ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/qt-android-cmake/AddQtAndroidApk.cmake)
endif()
if (IOS)
    include(${CMAKE_CURRENT_LIST_DIR}/ios/helloimgui_ios.cmake)
endif()

#
# helloimgui_add_app is a helper function, similar to cmake's "add_executable"
#
# Usage:
# helloimgui_add_app(app_name file1.cpp file2.cpp ...)
#
# Features:
# * It will automaticaly link the exe to hello_imgui library
# * Under android, it uses [qt-android-cmake](https://github.com/LaurentGomila/qt-android-cmake.git)
#   in order to create an apk
function(helloimgui_add_app)
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
        add_qt_android_apk(${app_name}_apk ${app_name})
    endif()
    if (IOS)
        helloimgui_ios_adapt(${app_name})
    endif()

    target_link_libraries(${app_name} PRIVATE hello_imgui)

    message("helloimgui_add_app
             app_name=${app_name}
             sources=${app_sources}
            ")
endfunction()
