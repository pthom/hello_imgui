if (APPLE)

    # hello_imgui_select_plist: select best plist file for the current platform
    function(hello_imgui_select_plist assets_location output_plist_path)
        # List of possible paths, in order of priority (first is highest priority)
        if (MACOSX)
            set(possible_paths
                ${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist
                ${assets_location}/app_settings/apple/Info.macos.plist
                ${assets_location}/app_settings/apple/Info.plist
                ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/info_plist/Info.plist.in
            )
        endif()
        if (IOS)
            set(possible_paths
                ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist
                ${assets_location}/app_settings/apple/Info.ios.plist
                ${assets_location}/app_settings/apple/Info.plist
                ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/info_plist/Info.plist.in
            )
        endif()

        # Find the first existing plist
        foreach(possible_path ${possible_paths})
            if (EXISTS ${possible_path})
                set(found_plist ${possible_path})
                break()
            endif()
        endforeach()

        # if found_plist is not defined, we have a problem
        if (NOT DEFINED found_plist)
            message(FATAL_ERROR "hello_imgui_select_plist: no plist found in ${possible_paths}")
        endif()

        set(${output_plist_path} ${found_plist} PARENT_SCOPE)
    endfunction()


    # This function is common to IOS and MACOSX, and customize the Info.plist
    function(hello_imgui_add_info_plist app_name assets_location)
        hello_imgui_select_plist( ${assets_location} info_plist_in)
        message(VERBOSE "hello_imgui_add_info_plist: info_plist_in=${info_plist_in} for app_name=${app_name}")
        set(info_plist_configured ${CMAKE_CURRENT_BINARY_DIR}/Info${app_name}.plist)

        if (NOT DEFINED HELLO_IMGUI_BUNDLE_IDENTIFIER)
            set(HELLO_IMGUI_BUNDLE_IDENTIFIER ${HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART}.${HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART})
        endif()

        # Clean CFBundleIdentifier (remove unwanted characters)
        # See https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html#//apple_ref/doc/uid/TP40009249-SW1
        # The string should be in reverse DNS format using only the Roman alphabet
        # in upper and lower case (A–Z, a–z), the dot (“.”), and the hyphen (“-”)
        string(REGEX REPLACE
            "[^a-zA-Z.-]" "-"
            HELLO_IMGUI_BUNDLE_IDENTIFIER
            "${HELLO_IMGUI_BUNDLE_IDENTIFIER}"
        )
        string(TOLOWER "${HELLO_IMGUI_BUNDLE_IDENTIFIER}" HELLO_IMGUI_BUNDLE_IDENTIFIER)
        message(VERBOSE "HELLO_IMGUI_BUNDLE_IDENTIFIER=${HELLO_IMGUI_BUNDLE_IDENTIFIER}")

        configure_file(${info_plist_in} ${info_plist_configured})
        set_target_properties(${app_name} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${info_plist_configured}
        )
    endfunction()

    ########################################################
    # Specific functions for IOS
    ########################################################
    if (IOS)
        string(REGEX MATCH "SIMULATOR.*" IOS_IS_SIMULATOR ${PLATFORM})

        function(hello_imgui_ios_set_dev_team app_name)
            if (IOS_IS_SIMULATOR)
                return()
            endif()
            set_target_properties( ${app_name} PROPERTIES
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}
                )
        endfunction()


        # Check CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM
        function(hello_imgui_ios_check_development_team)
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
        function(hello_imgui_ios_add_icons app_name)
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
                find_program(PYTHON_EXECUTABLE NAMES python3 python)
                if (NOT PYTHON_EXECUTABLE)
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
                    COMMAND ${PYTHON_EXECUTABLE} ${script_png_icon_to_icon_folder} ${custom_app_png_icon} ${custom_icons_assets_folder}
                    RESULT_VARIABLE script_png_icon_to_icon_folder_result
                )
                if (NOT ${script_png_icon_to_icon_folder_result} EQUAL 0)
                    message(WARNING "
                        ${app_name}: failed to create iOS icon from ${custom_app_icon}
                        Tried to run:
                            ${PYTHON_EXECUTABLE} ${script_png_icon_to_icon_folder} ${custom_app_png_icon} ${custom_icons_assets_folder}
                        This is not a fatal error, but the app will not have a custom icon.
                    ")
                    return()
                endif()

                set(icons_assets_folder ${custom_icons_assets_folder})
                set(found_custom_icon ON)
            endif()

            hello_imgui_apple_bundle_add_files_from_folder(${app_name} ${icons_assets_folder} "")
        endfunction()

    endif(IOS)



    ########################################################
    # Specific functions for MACOSX
    ########################################################
    if (MACOSX)

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
                find_program(PYTHON_EXECUTABLE NAMES python3 python)
                if (NOT PYTHON_EXECUTABLE)
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
                    COMMAND ${PYTHON_EXECUTABLE} ${script_png_to_icns} ${custom_app_png_icon} ${custom_app_icon}
                    RESULT_VARIABLE script_png_to_icns_result
                )
                if (NOT ${script_png_to_icns_result} EQUAL 0)
                    message(WARNING "
                        ${app_name}: failed to create macOS icon from ${custom_app_icon}
                            Tried to run:
                                ${PYTHON_EXECUTABLE} ${script_png_to_icns} ${custom_app_png_icon} ${custom_app_icon}
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
    endif(MACOSX)


    ########################################################
    # Definitions of hello_imgui_platform_customization
    # for IOS and MACOSX
    ########################################################
    if (MACOSX AND NOT HELLOIMGUI_MACOS_NO_BUNDLE)

        function(hello_imgui_platform_customization app_name assets_location)
            hello_imgui_add_info_plist(${app_name} ${assets_location})
            hello_imgui_macos_add_icons(${app_name} ${assets_location})
        endfunction()

    elseif(IOS)

        function(hello_imgui_platform_customization app_name assets_location)
            hello_imgui_ios_check_development_team()
            hello_imgui_ios_set_dev_team(${app_name})
            hello_imgui_add_info_plist(${app_name} ${assets_location})
            hello_imgui_ios_add_icons(${app_name})
        endfunction()

    endif()
endif(APPLE)
