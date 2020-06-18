# * Bundle id
#   set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.mycompany.\${PRODUCT_NAME:identifier}")
#   set(APP_TYPE MACOSX_BUNDLE)
# Does *NOT* workk yet
function(hello_imgui_ios_set_bundle_id app_name)
#   set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.mycompany.\${PRODUCT_NAME:identifier}" CACHE STRING "bundle id" FORCE)
#   set(APP_TYPE MACOSX_BUNDLE)

#    set_target_properties( ${app_name} PROPERTIES
#        MACOSX_BUNDLE_GUI_IDENTIFIER "${IVS_BUNDLE_IDENTIFIER}"
#        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${IVS_DEVELOPMENT_TEAM}"
#        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${IVS_CODE_SIGN_IDENTITY}"
#        MACOSX_BUNDLE_BUNDLE_VERSION "${IVS_BUNDLE_BUILD}"
#        MACOSX_BUNDLE_SHORT_VERSION_STRING "${IVS_BUNDLE_VERSION}"
#        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "2"
#        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf"
#        XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=RelWithDebInfo] "dwarf"
#        XCODE_ATTRIBUTE_ENABLE_BITCODE NO
#        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME ${IVS_ICON_FILE}
#        XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME ${IVS_BUNDLE_SPLASH}
#        XCODE_ATTRIBUTE_SKIP_INSTALL NO
#        XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
#    )
    set_target_properties( ${app_name} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.ivs.${app_name}"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}
        )
endfunction()

# Check CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM
function(hello_imgui_ios_check_development_team)
    if(NOT DEFINED CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM)
        if(DEFINED ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM})
            set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM $ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM})
        else()
            message(FATAL_ERROR STATUS "Please set CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM!")
        endif()
    endif()
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

function(hello_imgui_ios_add_info_plist app_name plist_type)
    set(info_plist_dir_all ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/ios/info_plist)
    set(info_plist ${info_plist_dir_all}/${plist_type}/Info.plist)
    set_target_properties(${app_name} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${info_plist}
        )
endfunction()


function(hello_imgui_ios_adapt app_name)
    hello_imgui_ios_check_development_team()
    hello_imgui_ios_set_bundle_id(${app_name})
    if (HELLOIMGUI_USE_SDL_OPENGL3)
        hello_imgui_ios_add_info_plist(${app_name} sdl)
    endif()
    if (IOS)
        set(assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_assets)
        hello_imgui_ios_bundle_assets(${app_name} ${assets_folder})
    endif()
endfunction()
