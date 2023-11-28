if ((NOT IOS) AND (NOT EMSCRIPTEN) AND (NOT ANDROID))

if (APPLE AND (NOT HELLOIMGUI_MACOS_NO_BUNDLE))

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


function(hello_imgui_macos_add_info_plist app_name plist_type)
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_IDENTIFIER)
        set(HELLO_IMGUI_BUNDLE_IDENTIFIER ${HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART}.${HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART})
    endif()
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist)
        set(info_plist_in ${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist)
        message("hello_imgui_macos_add_info_plist: ${app_name} found app specific Info.plist at ${info_plist_in}")
    else()
        # if there is no app specific plist, the iOS one works fine for macOS too
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


function(hello_imgui_platform_customization app_name)
    hello_imgui_macos_add_info_plist(${app_name} sdl)
    hello_imgui_macos_add_icons(${app_name})
endfunction()

else()

function(hello_imgui_platform_customization app_name)
endfunction()

endif(APPLE AND (NOT HELLOIMGUI_MACOS_NO_BUNDLE))


endif((NOT IOS) AND (NOT EMSCRIPTEN) AND (NOT ANDROID))