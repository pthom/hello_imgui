# * Bundle id
#   set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.mycompany.\${PRODUCT_NAME:identifier}")
#   set(APP_TYPE MACOSX_BUNDLE)
# Does *NOT* workk yet
function(helloimgui_ios_set_bundle_id app_name)
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
function(helloimgui_ios_check_development_team)
    if(NOT DEFINED CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM)
        if(DEFINED ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM})
            set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM $ENV{CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM})
        else()
            message(FATAL_ERROR STATUS "Please set CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM!")
        endif()
    endif()
endfunction()

# Bundle assets
function(helloimgui_ios_bundle_assets app_name)
    FILE(GLOB_RECURSE helloimgui_assets ${HELLOIMGUI_BASEPATH}/helloimgui_assets/*.*)
    target_sources(${app_name} PRIVATE ${helloimgui_assets})
    SET_SOURCE_FILES_PROPERTIES (
        ${app_name}
        ${helloimgui_assets}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endfunction()


## Font dir for desktop
#set(HELLOIMGUI_ASSETSDIR ${CMAKE_CURRENT_LIST_DIR}/../../helloimgui_assets)
#target_compile_definitions(${target_name} PUBLIC -DHELLOIMGUI_ASSETSDIR="${HELLOIMGUI_ASSETSDIR}")
## Place fonts in bundle for IOS
#if (IOS)
#    set(bundle_fonts ${fonts_dir}/DroidSans.ttf ${fonts_dir}/fontawesome-webfont.ttf)
#    target_sources(${target_name} PRIVATE ${bundle_fonts})
#
#    SET_SOURCE_FILES_PROPERTIES (
#        ${PROJECT_NAME}
#        ${bundle_fonts}
#        PROPERTIES
#        MACOSX_PACKAGE_LOCATION Resources
#    )
#endif()


# * Link SDL ?

# * Qt Link



function(helloimgui_ios_adapt app_name)
    helloimgui_ios_check_development_team()
    helloimgui_ios_set_bundle_id(${app_name})
    helloimgui_ios_bundle_assets(${app_name})
endfunction()
