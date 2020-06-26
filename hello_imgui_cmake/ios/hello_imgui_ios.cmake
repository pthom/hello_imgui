if (IOS)

function(hello_imgui_ios_set_dev_team app_name)
    set_target_properties( ${app_name} PROPERTIES
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}
        )
endfunction()


# Check CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM
function(hello_imgui_ios_check_development_team)
    string(REGEX MATCH "SIMULATOR.*" is_simulator ${PLATFORM})
    if (is_simulator)
        return()
     endif()
    if(NOT DEFINED CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM OR ${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM} STREQUAL "123456789A")
        if(DEFINED ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM})
            set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM $ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM} CACHE STRING "iOS Devel Team id" FORCE)
        else()
            message(FATAL_ERROR "Please set CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM")
        endif()
    endif()
    message("CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}")
endfunction()


function(hello_imgui_ios_add_icons app_name)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ios/icons)
        message("hello_imgui_ios_add_icons: ${app_name} found app specific icons")
        set(icons_assets_folder ${CMAKE_CURRENT_SOURCE_DIR}/ios/icons)
    else()
        set(icons_assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/ios/icons)
    endif()
    hello_imgui_bundle_assets(${app_name} ${icons_assets_folder})
endfunction()


function(hello_imgui_ios_add_info_plist app_name plist_type)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist)
        message("hello_imgui_ios_add_info_plist: ${app_name} found app specific Info.plist")
        set(info_plist_in ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist)
    else()
        set(HELLO_IMGUI_CFBundleIdentifier ${HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART}.${HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART})
        set(info_plist_dir_all ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/ios/info_plist)
        set(info_plist_in ${info_plist_dir_all}/${plist_type}/Info.plist.in)
    endif()
    set(info_plist_configured ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

    # Clean CFBundleIdentifier (remove unwanted characters)
    # See https://developer.apple.com/library/archive/documentation/General/Reference/InfoPlistKeyReference/Articles/CoreFoundationKeys.html#//apple_ref/doc/uid/TP40009249-SW1
    # The string should be in reverse DNS format using only the Roman alphabet
    # in upper and lower case (A–Z, a–z), the dot (“.”), and the hyphen (“-”)
    string(REGEX REPLACE
        "[^a-zA-Z.-]" "-"
        HELLO_IMGUI_CFBundleIdentifier
        "${HELLO_IMGUI_CFBundleIdentifier}"
        )
    message("HELLO_IMGUI_CFBundleIdentifier=${HELLO_IMGUI_CFBundleIdentifier}")

    configure_file(${info_plist_in} ${info_plist_configured})
    set_target_properties(${app_name} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${info_plist_configured}
        )
endfunction()


function(hello_imgui_platform_customization app_name)
    hello_imgui_ios_check_development_team()
    hello_imgui_ios_set_dev_team(${app_name})
    #if (HELLOIMGUI_USE_SDL_OPENGL3)
        hello_imgui_ios_add_info_plist(${app_name} sdl)
    #endif()
    hello_imgui_ios_add_icons(${app_name})
endfunction()

endif(IOS)
