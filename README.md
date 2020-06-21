![Ubuntu](https://github.com/pthom/hello_imgui/workflows/Ubuntu/badge.svg)
![Windows](https://github.com/pthom/hello_imgui/workflows/Windows/badge.svg)
![MacOS](https://github.com/pthom/hello_imgui/workflows/MacOS/badge.svg)
![iOS](https://github.com/pthom/hello_imgui/workflows/iOS/badge.svg)
![Android](https://github.com/pthom/hello_imgui/workflows/Android/badge.svg)
![Emscripten](https://github.com/pthom/hello_imgui/workflows/Emscripten/badge.svg)

# Hello, Dear ImGui

_HelloImGui_ is a library that enables to write  multiplatform Gui apps for Windows, Mac, Linux, iOS, Android, emscripten; with the simplicity of a "Hello World" app!

It is based on [Dear ImGui](https://github.com/ocornut/imgui), a Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies.

__Online Emscripten/Wasm demos:__


| Hello, World | Advanced Docking | Classic ImGui Demo
| --- | --- | --- |
| [![Hello, World](docs/images/wasm_demos/hello.jpg)][hello-world]  | [![Advanced Docking demo](docs/images/wasm_demos/docking.jpg)][docking]  | [![ImGui classic demo](docs/images/wasm_demos/classic.jpg)][classic]

[hello-world]: https://traineq.org/HelloImGui/hello_imgui_demos/hello_world/hello_world.html  "Hello world"
[docking]: https://traineq.org/HelloImGui/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.html  "Advanced docking demo"
[classic]: https://traineq.org/HelloImGui/hello_imgui_demos/hello_imgui_demo_classic/hello-imgui-demo-classic.html  "ImGui classic demo"

----

__Table of contents__

<span id="TOC"/></span>

* [Hello, Dear ImGui](#hello-dear-imgui)
* [Examples](#examples)
  * [Hello, world!](#hello-world)
  * [Advanced example with docking support](#advanced-example-with-docking-support)
  * [ImGui "classic" demo](#imgui-classic-demo)
  * [Use HelloImGui as a submodule in your own application](#use-helloimgui-as-a-submodule-in-your-own-application)
* [Features](#features)
* [Supported platforms and backends](#supported-platforms-and-backends)
  * [Platforms](#platforms)
  * [Backends](#backends)
* [Usage instructions and API](#usage-instructions-and-api)
* [Build instructions](#build-instructions)
  * [Clone the repository](#clone-the-repository)
  * [Build instructions for desktop platforms (Linux, MacOS, Windows)](#build-instructions-for-desktop-platforms-linux-macos-windows)
    * [Select your backend](#select-your-backend)
    * [Install Glfw3 and Sdl2 via vcpkg](#install-glfw3-and-sdl2-via-vcpkg)
    * [Backend with SDL2 + OpenGL3](#backend-with-sdl2--opengl3)
      * [Warning: main() signature with SDL](#warning-main-signature-with-sdl)
    * [Backend with with Glfw3 + OpenGL3](#backend-with-with-glfw3--opengl3)
    * [Backend with Qt](#backend-with-qt)
  * [Build instructions for iOS](#build-instructions-for-ios)
    * [Install requirements](#install-requirements)
    * [Build for iOS](#build-for-ios)
    * [Customizing the iOS build](#customizing-the-ios-build)
    * [Embed more files with your iOS application](#embed-more-files-with-your-ios-application)
  * [Build instructions for emscripten](#build-instructions-for-emscripten)
    * [Install the requirements (emsdk)](#install-the-requirements-emsdk)
    * [Build for emscripten](#build-for-emscripten)
    * [Customizing the emscripten build](#customizing-the-emscripten-build)
    * [Embed more files with your emscripten application](#embed-more-files-with-your-emscripten-application)
  * [Build instructions for Android](#build-instructions-for-android)
    * [Set Android required environment variables](#set-android-required-environment-variables)
    * [Install SDL2 for Android via vcpkg](#install-sdl2-for-android-via-vcpkg)
    * [Run cmake with the android toolchain](#run-cmake-with-the-android-toolchain)
    * [Build for Android](#build-for-android)
    * [Android build status](#android-build-status)
* [Developer informations](#developer-informations)
  * [Adding backends](#adding-backends)
  * [Multiplatform cmake in 2 lines](#multiplatform-cmake-in-2-lines)

----

# Examples

## Hello, world!

With HelloImGui, the equivalent of the "Hello, World!" can be written with 8 C++ lines + 2 CMake lines:

<img src="docs/images/hello.png" width=200> [Online demo][hello-world]

[__hello_word.main.cpp__](src/hello_imgui_demos/hello_world/hello_world.main.cpp)
`````cpp
#include "hello_imgui/hello_imgui.h"
int main()
{
    HelloImGui::Run(
        []{ ImGui::Text("Hello, world!"); }, // Gui code
        { 200.f, 50.f },                     // Window Size
        "Hello!" );                          // Window title
}
`````

[__CMakeLists.txt__](src/hello_imgui_demos/hello_world/CMakeLists.txt):
````cmake
include(${CMAKE_CURRENT_LIST_DIR}/../../hello_imgui/helloimgui_add_app_cmake/helloimgui_add_app.cmake)
helloimgui_add_app(hello_world hello_world.main.cpp)
````

_Although this app was extremely simple to write, it will run with no additional modifications (including in the cmake code) on iOS, Android, Linux, Mac, Windows and Emscripten_

Source for this example: [src/hello_imgui_demos/hello_world](src/hello_imgui_demos/hello_world)


## Advanced example with docking support

This example showcases various features of _Hello ImGui_.
![demo docking](docs/images/docking.gif) [Online demo][docking]

Source for this example: [src/hello_imgui_demos/hello_imgui_demodocking](src/hello_imgui_demos/hello_imgui_demodocking)


## ImGui "classic" demo

This example reproduces ImGui default example.

<img src=https://i.gyazo.com/6f12592e43590d98aa0d992aaffe685f.gif height=100> [Online demo][classic]

Source for this example: [src/hello_imgui_demos/hello_imgui_demo_classic](src/hello_imgui_demos/hello_imgui_demo_classic)

## Use HelloImGui as a submodule in your own application

[hello_imgui_my_app](https://github.com/pthom/hello_imgui_my_app) is a separate repo that gives a working example on how to use the library as a submodule in a separate application. 


# Features

* Docking support (based on ImGui [docking branch](https://github.com/ocornut/imgui/tree/docking))
* Default docking layout + View menu with option to restore the layout
* Status bar
* Log widget
* Zoom (especialy useful for mobile devices)
* Mobile apps specific callbacks (OnPause, OnResume, OnLowMemory)
* Mobile apps customization (icon, embedded files, etc)


# Supported platforms and backends

## Platforms
* Windows
* Linux
* OSX
* iOS
* emscripten
* Android : being developed

## Backends 
* SDL2 + OpenGL 3 or OpenGLES3 for mobile devices
* Glfw3 + OpenGL 3
* Qt

# Usage instructions and API

_RunnerParams_ contains all the settings and callbacks in order to run an application. 

> _[These settings are explained in details in the API Doc](src/hello_imgui/hello_imgui_api.md)_

![a](src/hello_imgui/doc_src/hello_imgui_diagram.png)



# Build instructions

## Clone the repository
````bash
git clone https://github.com/pthom/hello_imgui.git
cd hello_imgui
git submodule update --init
````

## Build instructions for desktop platforms (Linux, MacOS, Windows)

### Select your backend

Several cmake options are provided: you need to select at least one backend:
````cmake
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL+OpenGL3" OFF)
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for GLFW+OpenGL3" OFF)
option(HELLOIMGUI_USE_QT "Build HelloImGui for Qt" OFF)
option(HELLOIMGUI_USE_SDL_DIRECTX11 "Build HelloImGui for SDL+DirectX11" OFF)
````

_"HELLOIMGUI_USE_SDL_OPENGL3" is the preferred backend, since it works under all platforms (windows, linux, osx, android, emscripten, iOS). On Mobile platforms, it will use OpenGLES3._

### Install Glfw3 and Sdl2 via vcpkg 

If you intend to use SDL of glfw, you can either use your own installation or have them installed automatically via [vcpkg](https://github.com/Microsoft/vcpkg):

Simply run this command:
````bash
./tools/vcpkg_install_third_parties.py
````

This script will download and build vcpkg, then install sdl2 and Glfw3 into `hello_imgui/vcpkg/`

### Backend with SDL2 + OpenGL3

If you intend to use SDL provided by vcpkg use the following instructions:
````bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake  -DHELLOIMGUI_USE_SDL_OPENGL3=ON ..
make -j4
````

If you intend to use your own SDL installation, simply remove the argument "-DCMAKE_TOOLCHAIN_FILE".

#### Warning: main() signature with SDL

Warning for SDL apps under iOS:

SDL uses a dirty hack in order to _replace your main() function by its own main() function_,
which will then call your own main !

Please make sure that the signature of your main() function is *exactly*
    `int main(int argc, char **argv)`
and that your main() function returns an int.

### Backend with with Glfw3 + OpenGL3

Follow the instructiosn for SDL2, but replace HELLOIMGUI_USE_SDL_OPENGL3 by HELLOIMGUI_USE_GLFW_OPENGL3.

### Backend with Qt

Simply pass the option `-DHELLOIMGUI_USE_QT=ON` and specify the path to Qt via CMAKE_PREFIX_PATH.

For example, this line would build with Qt backend for an androïd_armv7 target:

````bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/5.12.8/clang_64 -DHELLOIMGUI_USE_QT=ON
````

----

## Build instructions for iOS

"SDL + OpenGL ES3" is currently the preferred backend for iOS.

This project uses the [ios-cmake](https://github.com/leetal/ios-cmake) toolchain which is a submodule in the folder [hello_imgui_cmake/ios-cmake](hello_imgui_cmake/ios-cmake).

### Install requirements

1. First, you need to download and compile SDL

Launch [tools/ios/sdl_compile_ios.sh](tools/ios/sdl_compile_ios.sh), which will download and compile SDL for iOS and the simulator, into the folder "external/SDL"

````bash
./tools/ios/sdl_compile_ios.sh
````


2. Set your development team Id inside [tools/ios/set_dev_team.source](tools/ios/set_dev_team.source)

Edit the file and replace the id with your own team id.
````bash
export CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM="0123456789"
````

### Build for iOS

1. **Source** tools/ios/set_dev_team.source in order to add the CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM environment variable

````bash
source tools/ios/set_dev_team.source
````

2. Launch cmake using [./tools/ios/cmake_ios_sdl.sh](tools/ios/cmake_ios_sdl.sh):

````bash
./tools/ios/cmake_ios_sdl.sh
````

This will create a build directory named "build_ios_sdl/" and then open the project "HelloImGui.xcodeproj".

If you want to run cmake by yourself, here are the required commands:
`````bash
mkdir build_ios_sdl
cd build_ios_sdl
export CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=YourTeamIdHere
cmake .. \
  -GXcode \
  -DCMAKE_TOOLCHAIN_FILE=../hello_imgui_cmake/ios-cmake/ios.toolchain.cmake \
  -DHELLOIMGUI_USE_SDL_OPENGL3=ON \
  -DPLATFORM=OS64 \
  -DENABLE_BITCODE=OFF \
  .. \
`````

### Customizing the iOS build

Simply create a folder named "ios" beside the application 'CMakeLists.txt'

Example of customization:
````
YourAppSourceFolder/
├── CMakeLists.txt                              # The app's CMakeLists
├── hello_imgui_demodocking.main.cpp            # its source code
|
└── ios/                                        # ios/ is where you customize the iOS App
    |
    ├── Info.plist                              # If present, this Info.plist will be applied 
    |                                           # (if not, a default is provided)
    |                                           # You can there customize the App icon name, etc.
    |
    └── icons/                                  # Icons and Launch images placed inside icons/ 
        ├── Default-375w-812h@3x.disabled.png   # will be placed in the application bundle 
        ├── Default-568h@2x.png                 # and thus used by the app
        ├── Default.png
        ├── Icon.png
        └── Readme.md
````

The example [src/hello_imgui_demos/hello_imgui_demodocking](src/hello_imgui_demos/hello_imgui_demodocking) shows some customization.


### Embed more files with your iOS application

By default, the iPhone apps will embed the fonts provided in the [hello_imgui_assets](hello_imgui_assets) folder.

[hello_imgui_cmake/ios/hello_imgui_ios.cmake](hello_imgui_cmake/ios/hello_imgui_ios.cmake) provides a function named `hello_imgui_bundle_assets` which helps embedding more assets.
 
----

## Build instructions for emscripten

> [emscripten](https://emscripten.org/) is a toolchain for compiling to asm.js and WebAssembly, built using LLVM, that lets you run C and C++ on the web at near-native speed without plugins.

### Install the requirements (emsdk)

You can either install emsdk following [the instruction on the emscripten website](https://emscripten.org/docs/getting_started/downloads.html) or you can use the script [tools/emscripten/cmake_emscripten.sh](tools/emscripten/cmake_emscripten.sh).

`````bash
./tools/emscripten/cmake_emscripten.sh
`````

This script will download and install emscripten into `~/emsdk`

### Build for emscripten

1. Add emsdk to your shell path;

You need to **source** the script ~/emsdk/emsdk_env.sh

````bash
source ~/emsdk/emsdk_env.sh
````

2. Run cmake, using "emcmake":

`````bash
mkdir build_emscripten
cd build_emscripten
emcmake cmake .. -DHELLOIMGUI_USE_SDL_OPENGL3=ON ..
`````

Note: the script [tools/emscripten/cmake_emscripten.sh](tools/emscripten/cmake_emscripten.sh) does exactly this.

3. Build

````bash
make -j 4
````

4. Test your emscripten application

You will need a web server. Python provides a basic web server that is easy to usen which you can launch like this:

`````bash
cd build_emscripten
python3 -m http.server
`````

Open a browser, and navigate to [http://localhost:8000](http://localhost:8000).

For example, the docking demo will be available at 
[http://localhost:8000/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.html](http://localhost:8000/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.html)

### Customizing the emscripten build

Refer to the [emscripten docs](https://emscripten.org/)

By default, the application will be presented in an empty html page. You can adapt [hello_imgui_cmake/emscripten/runner_emscripten_shell.html](hello_imgui_cmake/emscripten/runner_emscripten_shell.html) if you want.

### Embed more files with your emscripten application

By default, the emscripten apps will embed the fonts provided in the [hello_imgui_assets](hello_imgui_assets) folder, using emscripten `--preload-file` option.

----

## Build instructions for Android

Android support is being developed : as I do not own an Android device, help would be appreciated.
The Android version uses SDL + OpenGLES3.

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

The Android build is not complete: the application is correctly compiled as a static library, and linked with SDL. However, no apk is produced at the moment. 

Help would be appreciated!

The principle would be to fill [hello_imgui_cmake/android/hello_imgui_android.cmake](hello_imgui_cmake/android/hello_imgui_android.cmake), with inspiration from [hello_imgui_cmake/ios/hello_imgui_ios.cmake](hello_imgui_cmake/ios/hello_imgui_ios.cmake).

----

# Developer informations

## Adding backends

Adding new backend should be easy: simply add a new derivate of [AbstractRunner](src/hello_imgui/internal/backend_impls/abstract_runner.h).


## Multiplatform cmake in 2 lines

[hello_imgui_cmake/hello_imgui_add_app.cmake](hello_imgui_cmake/hello_imgui_add_app.cmake) is a cmake script that provides `helloimgui_add_app`, a cmake helper function, similar to cmake's "add_executable"

**Usage**

````cmake
include(hello_imgui_add_app)
hello_imgui_add_app(my_app my_app.main.cpp)
````

**Features**

* It will automaticaly link the exe to hello_imgui library
* Under iOS it will set the Development Team, add an Info.plist, add icon and LaunchImages
* It will bundle the assets in the app (fonts)
