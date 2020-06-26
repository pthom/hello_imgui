#./android_project_template/app/jni/CMakeLists.txt.in
#./android_project_template/app/build.gradle.in
#./android_project_template/app/src/main/AndroidManifest.xml.in
#./android_project_template/local.properties.in
#./android_project_template/build.gradle.in

# apkCMake_projectTemplateFolder: folder where the android project template is located
# apkCMake_projectFolder: folder where the android studio project will be created
# apkCMake_cmakeProjectName: name of the project. Will be set to ${appName}_GeneratedAndroidProject
# apkCMake_buildDir : un rep a cote du projet
# apkCMake_sdkDir ANDROID_HOME
# apkCMake_ndkDir $ANDROID_HOME/ndk-bundle if exists
# apkCMake_applicationId mandatory org.libsdl.app or from helloimgui
# apkCMake_compileSdkVersion 26
# apkCMake_minSdkVersion 16
# apkCMake_targetSdkVersion 26
# apkCMake_versionCode 1
# apkCMake_versionName "1.0"
# apkCMake_androidAppPlatform android-16
# apkCMake_abiFilters 'arm64-v8a' ou 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'

set(apkCMake_defaultProjectTemplateFolder ${CMAKE_CURRENT_LIST_DIR}/android_project_template)
set(apkCMake_defaultJavaCodeTemplateFolder ${CMAKE_CURRENT_LIST_DIR}/android_javacode_template)



macro(apkCMake_fillVariables)
    if (NOT DEFINED ANDROID_HOME)
        if (DEFINED ENV{ANDROID_HOME})
            set(ANDROID_HOME $ENV{ANDROID_HOME})
        else()
            message(FATAL_ERROR "Please set env variable ANDROID_HOME to your sdk path")
        endif()
    endif()
    message(STATUS "    > ANDROID_HOME=${ANDROID_HOME}")

    set(apkCMake_appTargetToEmbedCMakeListsDir ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(apkCMake_appTargetBuildFolder ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    if (NOT DEFINED apkCMake_cmakeProjectName)
        set(apkCMake_cmakeProjectName ${apkCMake_appTargetBuildFolder}_apkCMake_GeneratedProject)
    endif()
    message(STATUS "    > apkCMake_cmakeProjectName=${apkCMake_cmakeProjectName}")

    if (NOT DEFINED apkCMake_projectTemplateFolder)
        set(apkCMake_projectTemplateFolder ${apkCMake_defaultProjectTemplateFolder})
    endif()
    message(STATUS "    > apkCMake_projectTemplateFolder=${apkCMake_projectTemplateFolder}")

    if (NOT DEFINED apkCMake_javaCodeTemplateFolder)
        set(apkCMake_javaCodeTemplateFolder ${apkCMake_defaultJavaCodeTemplateFolder})
    endif()
    message(STATUS "    > apkCMake_javaCodeTemplateFolder=${apkCMake_projectTemplateFolder}")

    if (NOT DEFINED apkCMake_projectFolder)
        set(apkCMake_projectFolder ${CMAKE_BINARY_DIR}/${appTargetToEmbed}_AndroidStudio)
    endif()
    message(STATUS "    > apkCMake_projectFolder=${apkCMake_projectFolder}")

    if (NOT DEFINED apkCMake_buildDir)
        set(apkCMake_buildDir ${apkCMake_projectFolder}/build_apkCMake)
    endif()
    message(STATUS "    > apkCMake_buildDir=${apkCMake_buildDir}")

    if (NOT DEFINED apkCMake_sdkDir)
        set(apkCMake_sdkDir ${ANDROID_HOME})
    endif()
    message(STATUS "    > apkCMake_sdkDir=${apkCMake_sdkDir}")
    if (NOT DEFINED apkCMake_ndkDir)
        if (IS_DIRECTORY ${apkCMake_sdkDir}/ndk-bundle)
            set(apkCMake_ndkDir ${apkCMake_sdkDir}/ndk-bundle)
        else()
            message(FATAL_ERROR "Please set apkCMake_ndkDir")
        endif()
    endif()
    message(STATUS "    > apkCMake_ndkDir=${apkCMake_ndkDir}")


    if (NOT DEFINED apkCMake_applicationIdUrlPart)
        message(FATAL_ERROR "Please set apkCMake_applicationIdUrlPart (for example com.my_company)")
    endif()
    message(STATUS "    > apkCMake_applicationIdUrlPart=${apkCMake_applicationIdUrlPart}")

    if (NOT DEFINED apkCMake_applicationIdNamePart)
        message(FATAL_ERROR "Please set apkCMake_applicationIdNamePart (for example my_app)")
    endif()
#    remove_non_alphabetical_chars(apkCMake_applicationIdNamePart)
    string(REGEX REPLACE
        "[^a-zA-Z]" ""
        tmp
        ${apkCMake_applicationIdNamePart})
    set(apkCMake_applicationIdNamePart ${tmp})
    message(STATUS "    > apkCMake_applicationIdNamePart=${apkCMake_applicationIdNamePart}")

    if (NOT DEFINED apkCMake_className)
        set(apkCMake_className ${apkCMake_applicationIdNamePart})
    endif()
    message(STATUS "    > apkCMake_className=${apkCMake_className}")

    # apkCMake_compileSdkVersion 26
    # apkCMake_minSdkVersion 16
    # apkCMake_versionCode 1
    # apkCMake_versionName "1.0"
    # apkCMake_androidAppPlatform android-16
    # apkCMake_abiFilters 'arm64-v8a' ou 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
    if (NOT DEFINED apkCMake_compileSdkVersion)
        set(apkCMake_compileSdkVersion 26)
    endif()
    message(STATUS "    > apkCMake_compileSdkVersion=${apkCMake_compileSdkVersion}")

    if (NOT DEFINED apkCMake_minSdkVersion)
        set(apkCMake_minSdkVersion 21)
    endif()
    message(STATUS "    > apkCMake_minSdkVersion=${apkCMake_minSdkVersion}")

    if (NOT DEFINED apkCMake_targetSdkVersion)
        set(apkCMake_targetSdkVersion 26)
    endif()
    message(STATUS "    > apkCMake_targetSdkVersion=${apkCMake_targetSdkVersion}")

    if (NOT DEFINED apkCMake_versionCode)
        set(apkCMake_versionCode 1)
    endif()
    message(STATUS "    > apkCMake_versionCode=${apkCMake_versionCode}")

    if (NOT DEFINED apkCMake_versionName)
        set(apkCMake_versionName "1.0")
    endif()
    message(STATUS "    > apkCMake_versionName=${apkCMake_versionName}")

    if (NOT DEFINED apkCMake_androidAppPlatform)
        set(apkCMake_androidAppPlatform "android-16")
    endif()
    message(STATUS "    > apkCMake_androidAppPlatform=${apkCMake_androidAppPlatform}")

    if (NOT DEFINED apkCMake_abiFilters)
#        set(apkCMake_abiFilters "'arm64-v8a'")
#        set(apkCMake_abiFilters "'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'")
        set(apkCMake_abiFilters "'arm64-v8a', 'x86', 'x86_64'")

    endif()
    message(STATUS "    > apkCMake_abiFilters=${apkCMake_abiFilters}")
endmacro()


function(apkCMake_copyDirectoryContent src dst)
    file(GLOB_RECURSE all_files RELATIVE ${src} ${src}/*)
    foreach(one_file ${all_files})
        get_filename_component(dirname ${one_file} DIRECTORY)
        get_filename_component(basename ${one_file} NAME)
#        message("dirname=${dirname} basename=${basename}")
        file(COPY ${src}/${dirname}/${basename} DESTINATION ${dst}/${dirname})
    endforeach()
endfunction()


function (apkCMake_configureFile_InPlace filename)
    configure_file(${filename}.in ${filename})
    file(RENAME ${filename}.in ${filename}.in.done)
endfunction()

function (log_var var_name)
    message("${var_name}=${${var_name}}")
endfunction()

function (apkCmake_process_applicationId_javaCode)
    string(REPLACE "." "/" apkCMake_javaCodeDestination ${apkCMake_applicationIdUrlPart}.${apkCMake_applicationIdNamePart})
    set(apkCMake_javaCodeDestination ${apkCMake_projectFolder}/app/src/main/java/${apkCMake_javaCodeDestination})

    set(apkCMake_javaCodeSource ${apkCMake_javaCodeTemplateFolder})
    log_var(apkCMake_javaCodeDestination)
    log_var(apkCMake_javaCodeSource)
    configure_file(
        ${apkCMake_javaCodeSource}/apkCMake_className.java.in
        ${apkCMake_javaCodeDestination}/${apkCMake_className}.java
    )

    #    file(GLOB java_sources RELATIVE ${apkCMake_javaCodeSource} ${apkCMake_javaCodeSource}/*)
#    foreach(java_source ${java_sources})
#        message("java_source=${java_source}")
#        configure_file(${apkCMake_javaCodeSource}/${java_source} ${apkCMake_javaCodeDestination}/${java_source})
#    endforeach()
endfunction()


function(apkCMake_makeAndroidStudioProject appTargetToEmbed)
    message(STATUS "apkCMake_makeAndroidStudioProject ${appTargetToEmbed}")

    apkCMake_fillVariables()
    apkCMake_copyDirectoryContent(${apkCMake_projectTemplateFolder} ${apkCMake_projectFolder})

    apkCMake_configureFile_InPlace(${apkCMake_projectFolder}/app/jni/CMakeLists.txt)
    apkCMake_configureFile_InPlace(${apkCMake_projectFolder}/app/build.gradle)
    apkCMake_configureFile_InPlace(${apkCMake_projectFolder}/app/src/main/AndroidManifest.xml)
    apkCMake_configureFile_InPlace(${apkCMake_projectFolder}/local.properties)
    apkCMake_configureFile_InPlace(${apkCMake_projectFolder}/build.gradle)

    apkCmake_process_applicationId_javaCode()
    message(STATUS "    ---> Success: please open the project ${apkCMake_projectFolder} with Android Studio!")
endfunction()
