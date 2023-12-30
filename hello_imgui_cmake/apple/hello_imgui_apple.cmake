if(IOS)
    include(${CMAKE_CURRENT_LIST_DIR}/hello_imgui_ios.cmake)
endif()
if(MACOSX)
    include(${CMAKE_CURRENT_LIST_DIR}/hello_imgui_macos.cmake)
endif()


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
# Definitions of hello_imgui_platform_customization
# for IOS and MACOSX
########################################################
function(hello_imgui_platform_customization app_name assets_location)
    if (MACOSX AND NOT HELLOIMGUI_MACOS_NO_BUNDLE)
        hello_imgui_add_info_plist(${app_name} ${assets_location})
        hello_imgui_macos_add_icons(${app_name} ${assets_location})
    elseif(IOS)
        hello_imgui_ios_check_development_team()
        hello_imgui_ios_set_dev_team(${app_name})
        hello_imgui_add_info_plist(${app_name} ${assets_location})
        hello_imgui_ios_copy_resources(${app_name} ${assets_location})
        hello_imgui_ios_add_icons(${app_name} ${assets_location})
    endif()

    # Install app to install directory
    if (MACOSX AND HELLOIMGUI_ADD_APP_WITH_INSTALL)
        install(TARGETS ${app_name} DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
endfunction()
