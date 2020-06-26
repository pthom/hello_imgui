if (ANDROID)

include(android/apkCMake/apkCMake)

function (hello_imgui_platform_customization app_name)
    set(apkCMake_applicationIdUrlPart ${HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART})
    set(apkCMake_applicationIdNamePart ${HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART})
#    set(apkCMake_applicationId org.libsdl.app) # Desperate move !!!
    if (NOT HELLOIMGUI_IN_APKCMAKE_GENERATED_PROJECT)
        apkCMake_makeAndroidStudioProject(${app_name})
    endif()
endfunction()

endif(ANDROID)
