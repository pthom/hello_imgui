## Android Status

Android support is being developed.

Currently, the code correctly compiles and link for Android; however no apk is generated.

The Android version uses SDL + OpenGLES3.

## Build instructions for Android

### Set Android required environment variables

````bash
export ANDROID_NDK_HOME=/path/to/AndroidNdk
export JAVA_HOME=/path/to/jdk8
````

For example (MacOS):
````bash
export ANDROID_NDK_HOME=/Users/Me/Library/Android/sdk/ndk-bundle
export JAVA_HOME=/Library/Java/JavaVirtualMachines/adoptopenjdk-8.jdk/Contents/Home
````

### Install SDL2 for Android via vcpkg

[tools/android/vcpkg_install_sdl.sh](tools/android/vcpkg_install_sdl.sh) will install sdl for both arm-android
and arm64-android.

Note: see the [vcpkg doc about android](https://github.com/microsoft/vcpkg/blob/master/docs/users/android.md
)

`````bash
./tools/android/vcpkg_install_sdl.sh
`````

### Run cmake with the android toolchain

[tools/android/cmake_arm64-android.sh](tools/android/cmake_arm64-android.sh) will create a folder "build_arm64-android" and run cmake with the correct toolchains for vcpkg and android (see also [tools/android/_impl_cmake_android.sh](tools/android/_impl_cmake_android.sh)).

````bash
./tools/android/cmake_arm64-android.sh
````

You can also create a build for arm-android with:
````bash
./tools/android/cmake_arm-android.sh
````


### Build for Android

````
cd build_arm-android
make -j 4
````

### Android build status

The Android build is not complete: the application is correctly compiled as a static library, and linked with SDL. 

However, no apk is produced at the moment. 

---

## Next steps

The philosophy of Hello ImGui is to make as easy as possible to create a new app, so the users
should not worry too much about editing manifests, Android.mk, etc (reasonable default should be provided
by a cmake scripts, and the important one should be configurable via cmake).

Also, adding assets should be as easy as adding files or symlinks into the application local assets/ folder.

The principle would be to fill [hello_imgui_android.cmake](hello_imgui_cmake/android/hello_imgui_android.cmake), with inspiration from [hello_imgui_ios.cmake](hello_imgui_cmake/ios/hello_imgui_ios.cmake).

Some possible sources for inspiration:

* [Android Apk](https://github.com/hunter-packages/android-apk) by the former maintainer of [Hunter](https://github.com/cpp-pm/hunter), provides a script that seems promising
* The [SDL readme about android](https://hg.libsdl.org/SDL/file/default/docs/README-android.md) provides hints about the java code required to run with SDL backend
* [qt-android-cmake](https://github.com/LaurentGomila/qt-android-cmake) can produce an apk via cmake, but it also require Qt 

 
