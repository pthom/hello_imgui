# Main

## Doc
* [X] Move Android doc

## Publish

## Features
* [ ] Docking / External windows ?


# Platforms

## CMake / hello_imgui_add_app:
     
## iOS

## Emscripten


## Windows

## OSX
* [ ] Make app package / embed assets ?

## Android
* [ ] Implement hello_imgui_platform_customization for android
    (See [Android_status.md](Android_status.md))

rename folders org.libsdl.app => apkCMake_applicationId

Voir APP_ABI dans hello_imgui_cmake/android/apkCMake/android_project_template/app/jni/Application.mk

`````bash
export android-template=external/SDL/android-project
export android-project=hello_imgui_cmake/android/build_android_example/android-project
cp -a $android-template $android-project

# configure $android-project/app/build.gradle:
# cmake +
##arguments "-DANDROID_APP_PLATFORM=android-16", "-DANDROID_STL=c++_static"
##// abiFilters 'armeabi-v7a', 'arm64-v8a', 'x86', 'x86_64'
##abiFilters 'arm64-v8a'

# edit $android-project/app/jni/CMakeLists.txt
#add_subdirectory(${HELLOIMGUI_BASEPATH})
#add_subdirectory(${HELLOIMGUI_BASEPATH}/src/hello_imgui_demos/hello_imgui_demodocking)
#add_subdirectory(${HELLOIMGUI_BASEPATH}/external/SDL)

edit android-project/local.properties and change paths

export ANDROID_HOME=/Users/pascal/Library/Android/sdk/
./gradlew build 
./gradlew installRelease

icons in android_project_template/app/src/main/res
`````

## Misc
* [ ] https://github.com/pthom/hello_imgui_my_app: more advanced demo?
* [ ] Add issue / qtimgui on Windows (blank window)
   
