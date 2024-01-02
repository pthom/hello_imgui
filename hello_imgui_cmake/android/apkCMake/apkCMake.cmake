set(apkCMake_projectTemplateFolder ${HELLOIMGUI_CMAKE_PATH}/android/apkCMake/templates/sdl CACHE STRING "" FORCE)
include(${apkCMake_projectTemplateFolder}/apkCMake_makeSymLinks.cmake)


function (apkCMake_logVar var_name)
    message(VERBOSE "    > ${var_name}=${${var_name}}")
endfunction()


macro(apkCMake_fillAndroidSdkVariables)
    if (NOT DEFINED ANDROID_HOME)
        if (DEFINED ENV{ANDROID_HOME})
            set(ANDROID_HOME $ENV{ANDROID_HOME})
        else()
            message(FATAL_ERROR "Please set env variable ANDROID_HOME to your sdk path")
        endif()
    endif()
    apkCMake_logVar(ANDROID_HOME)

    if (NOT DEFINED ANDROID_NDK_HOME)
        if (DEFINED ENV{ANDROID_NDK_HOME})
            set(ANDROID_NDK_HOME $ENV{ANDROID_NDK_HOME})
        else()
            if (IS_DIRECTORY ${ANDROID_HOME}/ndk-bundle)
                set(ANDROID_NDK_HOME ${ANDROID_HOME}/ndk-bundle)
            else()
                message(FATAL_ERROR "Please set ANDROID_NDK_HOME to your ndk path")
            endif()
        endif()
    endif()
    apkCMake_logVar(ANDROID_NDK_HOME)

    if (NOT DEFINED apkCMake_sdkDir)
        set(apkCMake_sdkDir ${ANDROID_HOME})
    endif()
    apkCMake_logVar(apkCMake_sdkDir)

    if (NOT DEFINED apkCMake_ndkVersion)
        # message(FATAL_ERROR "ANDROID_NDK_HOME=${ANDROID_NDK_HOME} but apkCMake_ndkVersion is not defined")
        # apkCMake_ndkVersion is the dirname of ANDROID_NDK_HOME
        get_filename_component(apkCMake_ndkVersion ${ANDROID_NDK_HOME} NAME)
        message(VERBOSE "apkCMake_ndkVersion=${apkCMake_ndkVersion}, inferred from NDK_HOME=${ANDROID_NDK_HOME}")
    endif()

endmacro()


macro(apkCMake_fillAndroidWantedVersions)
    if (NOT DEFINED apkCMake_compileSdkVersion)
        set(apkCMake_compileSdkVersion 26)
    endif()
    apkCMake_logVar(apkCMake_compileSdkVersion)

    if (NOT DEFINED apkCMake_minSdkVersion)
        set(apkCMake_minSdkVersion 21)
    endif()
    apkCMake_logVar(apkCMake_minSdkVersion)

    if (NOT DEFINED apkCMake_targetSdkVersion)
        set(apkCMake_targetSdkVersion 34)
    endif()
    apkCMake_logVar(apkCMake_targetSdkVersion)


    if (NOT DEFINED apkCMake_androidAppPlatform)
        set(apkCMake_androidAppPlatform "android-16")
    endif()
    apkCMake_logVar(apkCMake_androidAppPlatform)

    if (NOT DEFINED apkCMake_abiFilters)
        set(apkCMake_abiFilters "'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'")
    endif()
    apkCMake_logVar(apkCMake_abiFilters)
endmacro()


macro(apkCMake_fillVariables appTargetToEmbed)
    apkCMake_logVar(apkCMake_projectTemplateFolder)
    apkCMake_fillAndroidSdkVariables()
    apkCMake_fillAndroidWantedVersions()

    # apkCMake_appLibraryToEmbed is the name of the library that will be loaded
    # by SDLActivity (see getLibraries() in templates/sdl/apkCMake_activityClassName.java.in)
    set(apkCMake_appLibraryToEmbed ${appTargetToEmbed})
    apkCMake_logVar(apkCMake_appLibraryToEmbed)

    # apkCMake_cmakeProjectName is the name of the CMake project
    # i.e templates/sdl/gradle_template/app/jni/CMakeLists.txt.in
    # includes the line:
    # project(@apkCMake_cmakeProjectName@)
    set(apkCMake_cmakeProjectName ${apkCMake_appTargetBuildFolder}_apkCMake_GeneratedProject)
    apkCMake_logVar(apkCMake_cmakeProjectName)

    # apkCMake_outputProjectFolder define where the
    # Android Studio/Gradle project will be generated
    if (NOT DEFINED apkCMake_outputProjectFolder)
        set(apkCMake_outputProjectFolder ${CMAKE_BINARY_DIR}/${appTargetToEmbed}_AndroidStudio)
    endif()
    apkCMake_logVar(apkCMake_outputProjectFolder)

    # apkCMake_buildDir define where gradle should put its temporary build files
    if (NOT DEFINED apkCMake_buildDir)
        set(apkCMake_buildDir ${apkCMake_outputProjectFolder}/build_apkCMake)
    endif()

    # The Id of the application is separated in two part: UrlPart and NamePart
    # If you want the id com.my_company.my_app
    # then
    # set (apkCMake_applicationIdUrlPart com.my_company)
    # set(apkCMake_applicationIdNamePart my_app)
    if (NOT DEFINED apkCMake_applicationIdUrlPart)
        message(FATAL_ERROR "Please set apkCMake_applicationIdUrlPart (for example com.my_company)")
    endif()
    apkCMake_logVar(apkCMake_applicationIdUrlPart)

    if (NOT DEFINED apkCMake_applicationIdNamePart)
        message(FATAL_ERROR "Please set apkCMake_applicationIdNamePart (for example my_app)")
    endif()
    string(REGEX REPLACE     #  remove_non_alphabetical_chars
        "[^a-zA-Z]" ""
        apkCMake_applicationIdNamePart
        "${apkCMake_applicationIdNamePart}")
    apkCMake_logVar(apkCMake_applicationIdNamePart)

    if (NOT DEFINED apkCMake_iconDisplayName)
        message(FATAL_ERROR "Please set apkCMake_iconDisplayName (for example \"My App\")")
    endif()
    apkCMake_logVar(apkCMake_iconDisplayName)

    # apkCMake_activityClassName is the name of the main java activity class that will be generated
    if (NOT DEFINED apkCMake_activityClassName)
        set(apkCMake_activityClassName ${apkCMake_applicationIdNamePart})
    endif()
    apkCMake_logVar(apkCMake_activityClassName)

    # apkCMake_versionCode is used in AndroidManifest.xml.in:
    #    android:versionCode="@apkCMake_versionCode@"
    if (NOT DEFINED apkCMake_versionCode)
        set(apkCMake_versionCode 1)
    endif()
    apkCMake_logVar(apkCMake_versionCode)

    # apkCMake_versionCode is used in AndroidManifest.xml.in:
    #    android:versionName="@apkCMake_versionName@"
    if (NOT DEFINED apkCMake_versionName)
        set(apkCMake_versionName "1.0")
    endif()
    apkCMake_logVar(apkCMake_versionName)

endmacro()


function (apkCMake_configureFile_InPlace filename)
    configure_file(${filename}.in ${filename} @ONLY)
    file(REMOVE ${filename}.in)
endfunction()


function(apkCMake_copyDirectoryContent src dst)
    file(GLOB_RECURSE all_files RELATIVE ${src} ${src}/*)
    foreach(one_file ${all_files})
        get_filename_component(dirname ${one_file} DIRECTORY)
        get_filename_component(basename ${one_file} NAME)
        file(COPY ${src}/${dirname}/${basename} DESTINATION ${dst}/${dirname})
    endforeach()
endfunction()


function(apkCMake_copyAndConfigureDirectoryContent src dst)
    file(GLOB_RECURSE all_files RELATIVE ${src} ${src}/*)
    foreach(one_file ${all_files})
        get_filename_component(dirname ${one_file} DIRECTORY)
        get_filename_component(basename ${one_file} NAME)
        file(COPY ${src}/${dirname}/${basename} DESTINATION ${dst}/${dirname})
        if (basename MATCHES ".*\.in$") # if basename endWith(".in")
            string(REGEX REPLACE ".in$" "" basename_no_in ${basename})
            message(VERBOSE "Should configure ${basename}")
            apkCMake_configureFile_InPlace(${dst}/${dirname}/${basename_no_in})
        endif()
    endforeach()
endfunction()


function(apkCMake_addTemplateResFolder)
    set(apkCMake_resTemplateFolder ${HELLOIMGUI_CMAKE_PATH}/android/res)
    set(resOutputFolder ${apkCMake_outputProjectFolder}/app/src/main/res)
    apkCMake_copyDirectoryContent(${apkCMake_resTemplateFolder} ${resOutputFolder})
endfunction()


function(apkCMake_addAppSettingsAndroidFolder assets_location)
    set(local_settings_location ${assets_location}/app_settings/android)
    if (IS_DIRECTORY ${local_settings_location})
        message(VERBOSE "apkCMake_addAppSettingsAndroidFolder: ${app_name} found local settings in ${local_settings_location}")
        set(settingsOutputFolder ${apkCMake_outputProjectFolder}/app/src/main/)
        apkCMake_copyDirectoryContent(${local_settings_location} ${settingsOutputFolder})
    endif()
endfunction()


function (apkCmake_processActivityClass)
    string(REPLACE "." "/" javaCodeFolder ${apkCMake_applicationIdUrlPart}.${apkCMake_applicationIdNamePart})
    set(javaCodeFolder ${apkCMake_outputProjectFolder}/app/src/main/java/${javaCodeFolder})
    apkCMake_logVar(javaCodeFolder)
    if (EXISTS ${apkCMake_projectTemplateFolder}/apkCMake_activityClassName.java.in)
        configure_file(
            ${apkCMake_projectTemplateFolder}/apkCMake_activityClassName.java.in
            ${javaCodeFolder}/${apkCMake_activityClassName}.java
            @ONLY
        )
    else()
        message(FATAL_ERROR "cannot find ${apkCMake_projectTemplateFolder}/apkCMake_activityClassName.java.in")
    endif()
endfunction()


function(apkCMake_makeAndroidStudioProject appTargetToEmbed assets_location)
    message(VERBOSE "apkCMake_makeAndroidStudioProject ${appTargetToEmbed}")
    apkCMake_fillVariables(${appTargetToEmbed})
    apkCMake_copyAndConfigureDirectoryContent(${apkCMake_projectTemplateFolder}/gradle_template ${apkCMake_outputProjectFolder})
    apkCmake_processActivityClass()
    apkCMake_makeSymLinks()

    apkCMake_addTemplateResFolder()
    apkCMake_addAppSettingsAndroidFolder(${assets_location})

    message(STATUS "    ---> Success: please open the project ${apkCMake_outputProjectFolder} with Android Studio!")
endfunction()
