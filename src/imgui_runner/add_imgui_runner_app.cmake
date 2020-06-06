if(ANDROID)
    set(qtimgui_dir ${CMAKE_CURRENT_LIST_DIR}/../../external/qtimgui)
    include(${qtimgui_dir}/tools/qt-android-cmake/AddQtAndroidApk.cmake)
endif()

function(add_imguirunner_app)
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

    target_link_libraries(${app_name} PRIVATE imgui_runner)

    if (IMGUIRUNNER_QT)
        set(imgui_runner_impl_libs qt_imgui_quick imgui_runner_qt)
    endif()

    target_link_libraries(${app_name} PRIVATE ${imgui_runner_impl_libs})

    message("add_imgui_app
             app_name=${app_name}
             sources=${app_sources}
             linked with ${imgui_runner_impl_libs}")
endfunction()
