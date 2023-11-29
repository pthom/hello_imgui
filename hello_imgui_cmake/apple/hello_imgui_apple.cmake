if (APPLE)

    # This function is common to IOS and MACOSX, and customize the Info.plist
    function(hello_imgui_add_info_plist app_name)
        if (NOT DEFINED HELLO_IMGUI_BUNDLE_IDENTIFIER)
            set(HELLO_IMGUI_BUNDLE_IDENTIFIER ${HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART}.${HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART})
        endif()
        if (MACOSX)
            set(plist_subfolder "macos")
        else()
            set(plist_subfolder "ios")
        endif()
        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${plist_subfolder}/Info.plist)
            message("hello_imgui_add_info_plist: ${app_name} found app specific Info.plist")
            set(info_plist_in ${CMAKE_CURRENT_SOURCE_DIR}/${plist_subfolder}/Info.plist)
        else()
            set(info_plist_dir_all ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/info_plist)
            set(info_plist_in ${info_plist_dir_all}/Info.plist.in)
        endif()
        set(info_plist_configured ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

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

    # Specific functions for IOS
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


        function(hello_imgui_ios_add_icons app_name)
            if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ios/icons)
                message("hello_imgui_ios_add_icons: ${app_name} found app specific icons")
                set(icons_assets_folder ${CMAKE_CURRENT_SOURCE_DIR}/ios/icons)
            else()
                set(icons_assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/apple/ios_icons)
            endif()
            hello_imgui_apple_bundle_add_files_from_folder(${app_name} ${icons_assets_folder} "")
        endfunction()

    endif(IOS)

    # Specific functions for MACOSX
    if (MACOSX)
        function(hello_imgui_macos_add_icons app_name)
            set(app_icon ${CMAKE_CURRENT_SOURCE_DIR}/macos/${HELLO_IMGUI_BUNDLE_ICON_FILE})
            if (EXISTS ${app_icon})
                message(STATUS "hello_imgui_macos_add_icons: ${app_name} found app specific icon at ${app_icon}")
                target_sources(${app_name} PRIVATE ${app_icon})
                set_source_files_properties(${app_icon}
                    PROPERTIES
                    MACOSX_PACKAGE_LOCATION "Resources"
                )
            else()
                message(STATUS "hello_imgui_macos_add_icons: ${app_name} could NOT find app specific icon")
            endif()
        endfunction()
    endif(MACOSX)


    # Definitions of hello_imgui_platform_customization for IOS and MACOSX
    if (MACOSX AND NOT HELLOIMGUI_MACOS_NO_BUNDLE)

        function(hello_imgui_platform_customization app_name)
            hello_imgui_add_info_plist(${app_name} sdl)
            hello_imgui_macos_add_icons(${app_name})
        endfunction()

    elseif(IOS)

        function(hello_imgui_platform_customization app_name)
            hello_imgui_ios_check_development_team()
            hello_imgui_ios_set_dev_team(${app_name})
            hello_imgui_add_info_plist(${app_name})
            hello_imgui_ios_add_icons(${app_name})
        endfunction()

    endif(MACOSX)
endif(APPLE)
