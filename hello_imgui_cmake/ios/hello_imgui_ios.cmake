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


# Bundle assets
function(hello_imgui_ios_bundle_assets app_name assets_folder)
    FILE(GLOB_RECURSE assets ${assets_folder}/*.*)
    target_sources(${app_name} PRIVATE ${assets})
    foreach(asset ${assets})
        string(REPLACE ${assets_folder}/ "" asset_relative ${asset})
        get_filename_component(asset_dir ${asset_relative} DIRECTORY)
        SET_SOURCE_FILES_PROPERTIES (
            ${app_name}
            ${asset}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources/${asset_dir}
        )
    endforeach()
endfunction()


function(hello_imgui_ios_add_icons app_name)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ios/icons)
        message("hello_imgui_ios_add_icons: ${app_name} found app specific icons")
        set(icons_assets_folder ${CMAKE_CURRENT_SOURCE_DIR}/ios/icons)
    else()
        set(icons_assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/ios/icons)
    endif()
    hello_imgui_ios_bundle_assets(${app_name} ${icons_assets_folder})
endfunction()


function(hello_imgui_ios_add_info_plist app_name plist_type)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist)
        message("hello_imgui_ios_add_info_plist: ${app_name} found app specific Info.plist")
        set(info_plist ${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist)
    else()
        set(info_plist_dir_all ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/ios/info_plist)
        set(info_plist ${info_plist_dir_all}/${plist_type}/Info.plist)
    endif()
    set_target_properties(${app_name} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${info_plist}
        )
endfunction()


function(hello_imgui_ios_adapt app_name)
    hello_imgui_ios_check_development_team()
    hello_imgui_ios_set_dev_team(${app_name})
    if (HELLOIMGUI_USE_SDL_OPENGL3)
        hello_imgui_ios_add_info_plist(${app_name} sdl)
    endif()
    if (IOS)
        set(assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_assets)
        hello_imgui_ios_bundle_assets(${app_name} ${assets_folder})
        hello_imgui_ios_add_icons(${app_name})
    endif()
endfunction()
