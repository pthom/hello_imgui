if (ANDROID)

include(android/apkCMake/apkCMake)

function (hello_imgui_platform_customization app_name)
    set(apkCMake_applicationId ${HELLO_IMGUI_BUNDLE_IDENTIFIER})
#    set(apkCMake_applicationId org.libsdl.app) # Desperate move !!!
    if (NOT HELLOIMGUI_IN_APKCMAKE_GENERATED_PROJECT)
        apkCMake_makeAndroidStudioProject(${app_name})
    endif()
endfunction()

endif(ANDROID)
