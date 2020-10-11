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
| [Code](src/hello_imgui_demos/hello_world/hello_world.main.cpp)|[Code](src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp)|[Code](src/hello_imgui_demos/hello_imgui_demo_classic/hello_imgui_demo_classic.main.cpp)|

[hello-world]: https://traineq.org/HelloImGui/hello_imgui_demos/hello_world/hello_world.html  "Hello world"
[docking]: https://traineq.org/HelloImGui/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.html  "Advanced docking demo"
[classic]: https://traineq.org/HelloImGui/hello_imgui_demos/hello_imgui_demo_classic/hello-imgui-demo-classic.html  "ImGui classic demo"

Below, a movie showing Hello ImGui running on 6 platforms:

[![Running on 6 platforms](docs/HelloImGui_6_Platforms.png) ](https://traineq.org/HelloImGui_6_Platforms.mp4)

__Applications:__

[ImGui Manual](https://github.com/pthom/imgui_manual) uses Hello ImGui.

Just click on the image below to open it:

[![ImGui Manual](https://raw.githubusercontent.com/pthom/imgui_manual/master/doc/images/link_manual.png)](https://traineq.org/imgui_manual/src/imgui_manual.html)

The video below gives a quick (2 minutes) introduction:

[![video on youtube](https://raw.githubusercontent.com/pthom/imgui_manual/master/doc/images/link_youtube_demo.png)](https://www.youtube.com/watch?v=MJ5jgoPPjH4)

---

__Instant develop__

You can test Hello ImGui 3 minutes, *without even installing anything*! No IDE, no text editor, no library, no compiler is required.

[Test ImGui application instant develop](https://github.com/pthom/hello_imgui_my_app/tree/gitpod#instant-develop)

---

__Table of contents__

[TOC]

---

# Examples

## Hello, world!

With HelloImGui, the equivalent of the "Hello, World!" can be written with 8 C++ lines + 2 CMake lines:

<img src="docs/images/hello.png" width=200> [Online demo][hello-world]

[__hello_word.main.cpp__](src/hello_imgui_demos/hello_world/hello_world.main.cpp)
`````cpp
#include "hello_imgui/hello_imgui.h"
int main(int, char **)
{
    HelloImGui::Run(
        []{ ImGui::Text("Hello, world!"); }, // Gui code
        { 200.f, 50.f },                     // Window Size
        "Hello!" );                          // Window title
    return 0;
}
`````

[__CMakeLists.txt__](src/hello_imgui_demos/hello_world/CMakeLists.txt):
````cmake
include(hello_imgui_add_app)
hello_imgui_add_app(hello_world hello_world.main.cpp)
````

_Although this app was extremely simple to write, it will run with no additional modifications (including in the cmake code) on iOS, Android, Linux, Mac, Windows and Emscripten_

Source for this example: [src/hello_imgui_demos/hello_world](src/hello_imgui_demos/hello_world)

## Advanced example with docking support

This example showcases various features of _Hello ImGui_.
![demo docking](docs/images/docking.gif) [Online demo][docking]

Source for this example: [src/hello_imgui_demos/hello_imgui_demodocking](src/hello_imgui_demos/hello_imgui_demodocking)

## Example of an app using HelloImGui as a submodule

[hello_imgui_my_app](https://github.com/pthom/hello_imgui_my_app) is a separate repo that gives a working example on how to use the library as a submodule. 

## ImGui "classic" demo

This example reproduces ImGui default example.

<img src=https://i.gyazo.com/6f12592e43590d98aa0d992aaffe685f.gif height=100> [Online demo][classic]

Source for this example: [src/hello_imgui_demos/hello_imgui_demo_classic](src/hello_imgui_demos/hello_imgui_demo_classic)

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
* Android

## Backends

* SDL2 + OpenGL 3 or OpenGLES3 for mobile devices
* Glfw3 + OpenGL 3
* Qt

Adding new backends should be easy: simply add a new derivate of [AbstractRunner](src/hello_imgui/internal/backend_impls/abstract_runner.h).

# Usage instructions and API

_RunnerParams_ contains all the settings and callbacks in order to run an application. 

> _[These settings are explained in details in the API Doc](src/hello_imgui/hello_imgui_api.md)_

![a](src/hello_imgui/doc_src/hello_imgui_diagram.png)

# Build instructions

> Note: If you want to use HelloImGui in your own application, you may also want to look at [hello_imgui_my_app](https://github.com/pthom/hello_imgui_my_app), which is a separate repo that gives a working example on how to use the library as a submodule.
  
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

"HELLOIMGUI_USE_SDL_OPENGL3" is the preferred backend, since it works under all platforms (windows, linux, osx, android, emscripten, iOS). On Mobile platforms, it will use OpenGLES3. Use it with `cmake .. -DHELLOIMGUI_USE_SDL_OPENGL3=ON`

### Plug your backend

#### Option 1: plug your backend manually

You can install your backend by any mean (global install, Conan, submodule, etc). 

Before adding the hello_imgui directory (`add_subdirectory(hello_imgui)`), just make 
sure that your backend is available, and select it via one of the variables HELLOIMGUI_USE_SDL_OPENGL3, 
HELLOIMGUI_USE_GLFW_OPENGL3, or HELLOIMGUI_USE_QT).

For example, the cmake script below works for the GLFW backend:

````cmake
  # Here, glfw was added as a submodule into a folder "glfw"
  add_subdirectory(glfw) 
  # We instruct HelloImgui to use glfw
  set(HELLOIMGUI_USE_GLFW_OPENGL3 ON CACHE BOOL "" FORCE)
  # And add HelloImGui
  add_subdirectory(hello_imgui)
````

#### Option 2: plug SDL or Glfw3 via vcpkg

[Vcpkg](https://github.com/Microsoft/vcpkg) is a C++ Library Manager for Windows, Linux, and MacOS (support for iOS and Android is coming).

If you intend to use SDL of glfw, you can have them installed automatically via Vcpkg: simply run this command:

````bash
./tools/vcpkg_install_third_parties.py
````

This script will download and build vcpkg, then install sdl2 and Glfw3 into `hello_imgui/vcpkg/`

You can then build HelloImgui, using the following instructions:

````bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DHELLOIMGUI_USE_SDL_OPENGL3=ON ..
make -j4
````
(Use `-DHELLOIMGUI_USE_GLFW_OPENGL3=ON` for glfw)


### SDL Backend Warning for main() signature

@import "src/hello_imgui/hello_imgui.h" {md_id=SDLMain}

### Backend with Qt

Requirements:

* You need to have Qt >= 5.10 installed
* The Qt backend uses [qtimgui](https://github.com/seanchas116/qtimgui) , which you need to download into external/qutimgui.
  You can use the script [tools/qtimgui_download.py](tools/qtimgui_download.py) in order to download it
  in one step.

Usage: simply pass the option `-DHELLOIMGUI_USE_QT=ON` and specify the path to Qt via CMAKE_PREFIX_PATH.

For example, this line would build with Qt backend for an androïd_armv7 target:

````bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/5.12.8/clang_64 -DHELLOIMGUI_USE_QT=ON
````

---

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
mkdir build_ios && cd build_ios_sdl
../tools/ios/cmake_ios_sdl.sh
````

This will invoke cmake and then open the project "HelloImGui.xcodeproj".

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

See [Embed assets and customize apps](#embed-assets-and-customize-apps)
 
---

## Build instructions for emscripten

> [emscripten](https://emscripten.org/) is a toolchain for compiling to asm.js and WebAssembly, built using LLVM, that lets you run C and C++ on the web at near-native speed without plugins.

### Install the requirements (emsdk)

You can either install emsdk following [the instruction on the emscripten website](https://emscripten.org/docs/getting_started/downloads.html) or you can use the script [tools/emscripten/cmake_emscripten.sh](tools/emscripten/cmake_emscripten.sh).

`````bash
../tools/emscripten/install_emscripten.sh
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

Note: the script [tools/emscripten/cmake_emscripten.sh](tools/emscripten/cmake_emscripten.sh) does the cmake part of this.

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

By default, the application will be presented inside an empty html page. 
You can adapt this page by modyfing the "shell": copy the file [hello_imgui_cmake/emscripten/shell.emscripten.html](hello_imgui_cmake/emscripten/shell.emscripten.html)
into your app source dir, and adapt it to your needs. 
 
---

## Build and deploy instructions for Android

The Android version uses SDL + OpenGLES3.

### Download SDL 

You need to download SDL manually for Android, like this:

````bash
./tools/sdl_download.sh
````

### Set Android required environment variables

````bash
export ANDROID_HOME=/path/to/AndroidSdk
````

For example (MacOS):

````bash
export ANDROID_HOME=/Users/Me/Library/Android/sdk
````

By default, the scripts will look for Android-ndk inside $ANDROID_HOME/ndk-bundle.

### Run cmake in order to create an Android studio project

The script [tools/android/cmake_arm-android.sh](tools/android/cmake_arm-android.sh)  will invoke cmake with the android toolchain, and also _create an Android Studio project_ which
is multiarch (arm64-v8a, armeabi-v7a, etc), via the option `-DHELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT=ON` (see [tools/android/_impl_cmake_android.sh](tools/android/_impl_cmake_android.sh))


Run the following commands:

````bash
mkdir build_android
cd build_android
../tools/android/cmake_arm-android.sh
````

Your build directory will now look like this:

````
build_android/
├── CMakeCache.txt
├── ...
├── hello-imgui-demo-classic_AndroidStudio/
├── hello_imgui_demo_minimal_AndroidStudio/
├── hello_imgui_demodocking_AndroidStudio/
├── hello_world_AndroidStudio/
├── ...
````

The folders "xxxx_AndroidStudio" contain Android Studio projects, which you can use to build and debug your app.

You can now open (for example) the project hello_imgui_demodocking_AndroidStudio with Android Studio and run it / debug it.

You can also build the project manually via gradlew like this:

````bash
export JAVA_HOME=/Library/Java/JavaVirtualMachines/adoptopenjdk-8.jdk/Contents/Home 
cd hello_imgui_demodocking_AndroidStudio
./gradlew build
````

_Note: (you need to first set JAVA_HOME to the correct java version (Android requires exactly jdk8), the path given here is for MacOS users, where adoptopenjdk provides the correct version)_

You can also install the app via command line, like this:

````bash
./gradlew installDebug
````

---

# Embed assets and customize apps

## Embed assets

Anything in the assets/ folder located beside the app's CMakeLists will be embedded
on mobile devices and emscripten, i.e they will be bundled together with the app; and you can access them via `assetFileFullPath(const std::string& assetRelativeFilename)`.

## Customize per platform 

### iOS

For iOS, simply create a folder named "ios" beside the application 'CMakeLists.txt'. There, you can add a custom Info.plist, as well as app icons and launch screens.

### Android 

For Android, simply create a folder named "android" beside the application 'CMakeLists.txt'. There, you can add a custom "res/" folder, containing your icons and application settings inside "res/values/".


## Example of customization:

````
hello_imgui_democking/
├── CMakeLists.txt                              # The app's CMakeLists
├── hello_imgui_demodocking.main.cpp            # its source code
│
│
├── assets/                                     # Anything in the assets/ folder located
│   └── fonts/                                  # beside the app's CMakeLists will be embedded
│       └── Akronim-Regular.ttf                 # on mobile devices and emscripten             
│
│
├── android/                                    # android/ is where you customize the Android App
│   ├── mipmap-source/
│   │   ├── Readme.md
│   │   └── ic_launcher.png                     # an icon that helps creating the different sizes
│   └── res/                                    # anything in the res/ folder will be embedded as a resource
│       ├── mipmap-hdpi/
│       │   └── ic_launcher.png                 # icons with different sizes
│       ├── mipmap-mdpi/
│       │   └── ic_launcher.png
│       ├── mipmap-xhdpi/
│       │   └── ic_launcher.png
│       ├── mipmap-xxhdpi/
│       │   └── ic_launcher.png
│       ├── mipmap-xxxhdpi/
│       │   └── ic_launcher.png
│       └── values/
│           ├── colors.xml
│           ├── strings.xml                    # Customize the application icon label here
│           └── styles.xml
│
│
└── ios/                                        # ios/ is where you customize the iOS App
    │
    ├── Info.plist                              # If present, this Info.plist will be applied 
    │                                           # (if not, a default is provided)
    │                                           # You can there customize the App icon name, etc.
    │
    └── icons/                                  # Icons and Launch images placed inside icons/ 
        ├── Default-375w-812h@3x.disabled.png   # will be placed in the application bundle 
        ├── Default-568h@2x.png                 # and thus used by the app
        ├── Default.png
        ├── Icon.png
        └── Readme.md

````

### Resizing icons for Android

You can use the script [tools/android/resize_icons.py](tools/android/resize_icons.py) in order 
to quickly create the icons with all the required sizes.

@import "tools/android/resize_icons.py" {md_id=resize_icons}

---

# Alternatives

[OpenFrameworks](https://openframeworks.cc/) and [Cinder](https://libcinder.org/) are alternatives in order to quickly start a C++ application under many platforms.

Being oriented for creative coding, they are much more feature rich, offers some level of native hardware access (camera, accelerometer), but they are also less lightweight than ImGui + HelloImGui.

[sokol_app](https://github.com/floooh/sokol#sokol_apph) is a minimal cross-platform application-wrapper library.