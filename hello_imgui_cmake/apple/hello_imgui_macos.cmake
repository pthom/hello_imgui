########################################################
# Specific functions for MACOSX
########################################################
# Handle icons (and conversion if necessary)
function(hello_imgui_macos_add_icons app_name assets_location)
    # Default HelloImGui icon
    set(app_icon ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/mac_icons/icon.icns)
    set(found_custom_icon OFF)

    # First possible custom icon in macos/icon.icns
    set(custom_app_icon ${CMAKE_CURRENT_SOURCE_DIR}/macos/${HELLO_IMGUI_BUNDLE_ICON_FILE})
    if ((NOT found_custom_icon) AND (EXISTS ${custom_app_icon}))
        set(app_icon ${custom_app_icon})
        set(found_custom_icon ON)
        message(STATUS "hello_imgui_macos_add_icons: found app specific icon at ${custom_app_icon} for app ${app_name} ")
    endif()

    # Second possible custom icon, in assets/app_settings/icon.png (which we need to convert to icon.icns)
    set(custom_app_png_icon ${assets_location}/app_settings/icon.png)
    if ((NOT found_custom_icon) AND (EXISTS ${custom_app_png_icon}))
        # find python program
        find_program(Python_EXECUTABLE NAMES python3 python)
        if (NOT Python_EXECUTABLE)
            message(WARNING "
            ${app_name}: can't create a macOS icons from ${custom_app_png_icon}
                (did not find python)
                This is not a fatal error, but the app will not have a custom icon.
            ")
            return()
        endif()

        # We need to convert icon.png to icon.icns
        set(script_png_to_icns "${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/macos_png_icon_to_icns.py")
        message(VERBOSE "hello_imgui_macos_add_icons: converting ${custom_app_png_icon} to icns for app ${app_name}")
        set(custom_app_icon ${CMAKE_CURRENT_BINARY_DIR}/icon.icns)
        execute_process(
            COMMAND ${Python_EXECUTABLE} ${script_png_to_icns} ${custom_app_png_icon} ${custom_app_icon}
            RESULT_VARIABLE script_png_to_icns_result
        )
        if (NOT ${script_png_to_icns_result} EQUAL 0)
            message(WARNING "
                ${app_name}: failed to create macOS icon from ${custom_app_icon}
                    Tried to run:
                        ${Python_EXECUTABLE} ${script_png_to_icns} ${custom_app_png_icon} ${custom_app_icon}
                    This is not a fatal error, but the app will not have a custom icon.
                ")
            return()
        endif()

        set(app_icon ${custom_app_icon})
        set(found_custom_icon ON)
        message(VERBOSE "found custom icon at ${custom_app_icon} for app ${app_name} ")
    endif()

    # Add icon to target
    target_sources(${app_name} PRIVATE ${app_icon})
    set_source_files_properties(${app_icon}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endfunction()
