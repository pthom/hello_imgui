
function(hello_imgui_ios_set_dev_team app_name)
    string(REGEX MATCH "SIMULATOR.*" IOS_IS_SIMULATOR ${PLATFORM})
    if (IOS_IS_SIMULATOR)
        return()
    endif()
    set_target_properties( ${app_name} PROPERTIES
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}
        )
endfunction()


# Check CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM
function(hello_imgui_ios_check_development_team)
    string(REGEX MATCH "SIMULATOR.*" IOS_IS_SIMULATOR ${PLATFORM})
    if (IOS_IS_SIMULATOR)
        return()
    endif()
    if(NOT DEFINED CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM OR ${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM} STREQUAL "123456789A")
        if(DEFINED ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM})
            set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM $ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM} CACHE STRING "iOS Devel Team id" FORCE)
        else()
            message(FATAL_ERROR "Please set CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM")
        endif()
    endif()
    message(VERBOSE "CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}")
endfunction()

# Handle icons (and conversion if necessary)
function(hello_imgui_ios_add_icons app_name assets_location)
    # Default HelloImGui icons
    set(icons_assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/ios_icons)
    set(found_custom_icon OFF)

    # First possible custom icons folder, in ios/icons
    set(custom_icons_assets_folder ${CMAKE_CURRENT_SOURCE_DIR}/ios/icons)
    if ((NOT found_custom_icon) AND (IS_DIRECTORY ${custom_icons_assets_folder}))
        message(STATUS "hello_imgui_ios_add_icons: found ios/icons specific icons folder for app ${app_name} ")
        set(icons_assets_folder ${custom_icons_assets_folder})
        set(found_custom_icon ON)
    endif()

    # Second possible custom icon, in assets/app_settings/icon.png
    set(custom_app_png_icon ${assets_location}/app_settings/icon.png)
    if ((NOT found_custom_icon) AND (EXISTS ${custom_app_png_icon}))
        # find python program
        find_program(Python_EXECUTABLE NAMES python3 python)
        if (NOT Python_EXECUTABLE)
            message(WARNING "
                ${app_name}: can't create iOS icon from ${custom_app_png_icon}
                (did not find python)
                This is not a fatal error, but the iOS app will not have the correct icon.
            ")
            return()
        endif()

        # We need to convert icon.png to an icon folder
        set(script_png_icon_to_icon_folder "${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/ios_png_icon_to_icon_folder.py")
        message(STATUS "hello_imgui_ios_add_icons: converting ${custom_app_png_icon} to icon folder for app ${app_name} ")
        set(custom_icons_assets_folder ${CMAKE_CURRENT_BINARY_DIR}/custom_icons)
        execute_process(
            COMMAND ${Python_EXECUTABLE} ${script_png_icon_to_icon_folder} ${custom_app_png_icon} ${custom_icons_assets_folder}
            RESULT_VARIABLE script_png_icon_to_icon_folder_result
        )
        if (NOT ${script_png_icon_to_icon_folder_result} EQUAL 0)
            message(WARNING "
                ${app_name}: failed to create iOS icon from ${custom_app_icon}
                Tried to run:
                    ${Python_EXECUTABLE} ${script_png_icon_to_icon_folder} ${custom_app_png_icon} ${custom_icons_assets_folder}
                This is not a fatal error, but the app will not have a custom icon.
            ")
            return()
        endif()

        set(icons_assets_folder ${custom_icons_assets_folder})
        set(found_custom_icon ON)
    endif()

    hello_imgui_apple_bundle_add_files_from_folder(${app_name} ${icons_assets_folder} "")
endfunction()


function(hello_imgui_ios_copy_resources app_name assets_location)
    # if assets_location/app_settings/apple/Resources/ios exists, copy it to the bundle Resources folder
    set(custom_resources_folder ${assets_location}/app_settings/apple/Resources/ios)
    if (EXISTS ${custom_resources_folder})
        message(STATUS "hello_imgui_ios_copy_resources: found custom iOS resources folder ${custom_resources_folder} for app ${app_name} ")
        hello_imgui_apple_bundle_add_files_from_folder(${app_name} ${custom_resources_folder} "Resources")
    endif()
endfunction()
