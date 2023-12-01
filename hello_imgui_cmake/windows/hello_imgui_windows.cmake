# if we are under windows
if (WIN32)

    function(_hello_imgui_create_windows_ico assets_location)
        set(custom_app_png_icon ${assets_location}/app_settings/icon.png)
        if (NOT EXISTS ${custom_app_png_icon})
            return()
        endif()

        # We need to convert icon.png to a windows ico file
        set(script_png_to_ico "${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/windows/windows_png_icon_to_ico.py")
        message(STATUS "_hello_imgui_create_ico: converting ${custom_app_png_icon} to windows ico for app ${app_name}")
        set(custom_app_icon ${CMAKE_CURRENT_BINARY_DIR}/icon.ico)
        execute_process(
            COMMAND python3 ${script_png_to_ico} ${custom_app_png_icon} ${custom_app_icon}
            RESULT_VARIABLE script_png_to_ico_result
        )
        if (NOT ${script_png_to_ico_result} EQUAL 0)
            message(WARNING "
                ${app_name}: failed to convert app icon ${custom_app_icon}
                to a Windows ico file.

                This is not a fatal error, but the app will not have a custom icon.

                In order to have a custom icon, you need to have python3 with the python3 Pillow package:

                    pip install Pillow

                ")
            return()
        endif()

        set(app_icon ${custom_app_icon})
        set(found_custom_icon ON)


    endfunction()

    function(_hello_imgui_add_windows_icon app_name assets_location)
        _hello_imgui_create_windows_ico(${assets_location})
        set(custom_app_icon ${CMAKE_CURRENT_BINARY_DIR}/icon.ico)
        if (NOT EXISTS ${custom_app_icon})
            return()
        endif()

        # Create rc file
        set(icon_rc_file "${CMAKE_CURRENT_BINARY_DIR}/app.rc")
        set(icon_rc_file_contents "IDI_ICON1 ICON DISCARDABLE \"${custom_app_icon}\"")

        # Add the rc file to the executable
        target_sources(${app_name} PRIVATE ${icon_rc_file})
    endfunction()


    function(hello_imgui_platform_customization app_name assets_location)
        _hello_imgui_add_windows_icon(${app_name} ${assets_location})
    endfunction()
endif(WIN32)



