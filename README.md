![](https://github.com/pthom/hello_imgui/workflows/.github/workflows/build_ubuntu.yml/badge.svg)

# Hello, Dear ImGui

_HelloImGui_ is a library that enables to write  multiplatform Gui apps for Windows, Mac, Linux, iOS, Android, emscripten; with the simplicity of a "Hello World" app!

It is based on [Dear ImGui](https://github.com/ocornut/imgui), a  Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies.


----

__Table of contents__

* [Hello, Dear ImGui](#hello,-dear-imgui)
* [Examples](#examples)
  * [Hello, world!](#hello,-world!)
  * [Advanced example with docking support](#advanced-example-with-docking-support)
  * [ImGui "classic" demo](#imgui-"classic"-demo)
* [Features](#features)
* [Supported platforms and backends](#supported-platforms-and-backends)
  * [Platforms](#platforms)
  * [Backends](#backends)
* [Usage instructions and API](#usage-instructions-and-api)
* [Build instructions](#build-instructions)
  * [Clone the repository](#clone-the-repository)
  * [Select your backend](#select-your-backend)
    * [Install Glfw3 and Sdl2 via vcpkg ](#install-glfw3-and-sdl2-via-vcpkg-)
    * [Backend with SDL2 + OpenGL3](#backend-with-sdl2--opengl3)
    * [Backend with with Glfw3 + OpenGL3](#backend-with-with-glfw3--opengl3)
    * [Backend with Qt](#backend-with-qt)
  * [Multiplatform cmake in 2 lines](#multiplatform-cmake-in-2-lines)
  * [Android](#android)
* [Developer informations](#developer-informations)
  * [Adding backends](#adding-backends)

----

# Examples

## Hello, world!

With HelloImGui, the equivalent of the "Hello, World!" can be written with 8 C++ lines + 2 CMake lines:

<img src="docs/images/hello.png" width=200>

1. [_hello_word.main.cpp_](src/hello_imgui_demos/hello_world/hello_world.main.cpp):
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
1. [_CMakeLists.txt_](src/hello_imgui_demos/hello_world/CMakeLists.txt):
````cmake
include(${CMAKE_CURRENT_LIST_DIR}/../../hello_imgui/helloimgui_add_app_cmake/helloimgui_add_app.cmake)
helloimgui_add_app(hello_world hello_world.main.cpp)
````

> _Although this app was extremely simple to write, it will run with no additional modifications (including in the cmake code) on iOS, Android, Linux, Mac, Windows_

Source for this example: [src/hello_imgui_demos/hello_world](src/hello_imgui_demos/hello_world)


## Advanced example with docking support

This example showcases various features of _Hello ImGui_.
![demo docking](docs/images/docking.gif)

Source for this example: [src/hello_imgui_demos/hello_imgui_demodocking](src/hello_imgui_demos/hello_imgui_demodocking)



## ImGui "classic" demo

This example reproduces ImGui default example.

![demo](https://i.gyazo.com/6f12592e43590d98aa0d992aaffe685f.gif)

Source for this example: [src/hello_imgui_demos/hello_imgui_demo_classic](src/hello_imgui_demos/hello_imgui_demo_classic)


# Features

* Docking support (based on ImGui [docking branch](https://github.com/ocornut/imgui/tree/docking))
* Default docking layout + View menu with option to restore the layout
* Status bar
* Log widget
* Zoom (especialy useful for mobile devices)


# Supported platforms and backends

## Platforms
* Windows
* Linux
* OSX
* Android
* iOS should come soon
* emscripten should come soon

## Backends

* Glfw3 + OpenGL 3
* SDL2 + OpenGL 3
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

## Select your backend

Several cmake options are provided: you need to select at least one backend:
````cmake
option(HELLOIMGUI_USE_QT "Build HelloImGui for Qt" OFF)
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for GLFW+OpenGL3" ON)
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL+OpenGL3" ON)
option(HELLOIMGUI_USE_SDL_DIRECTX11 "Build HelloImGui for SDL+DirectX11" ON)
````

### Install Glfw3 and Sdl2 via vcpkg 

If you intend to use SDL of glfw, you can either use your own installation or have them installed automatically via [vcpkg](https://github.com/Microsoft/vcpkg):

Simply run this command:
````bash
./tools/vcpkg_install_third_parties.sh
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

### Backend with with Glfw3 + OpenGL3

Follow the instructiosn for SDL2, but replace HELLOIMGUI_USE_SDL_OPENGL3 by HELLOIMGUI_USE_GLFW_OPENGL3.

### Backend with Qt

Simply pass the option `-DHELLOIMGUI_USE_QT=ON` and specify the path to Qt via CMAKE_PREFIX_PATH.

For example, this line would build with Qt backend for an androïd_armv7 target:

````bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/5.12.8/android_armv7 -DHELLOIMGUI_USE_QT=ON
````

## Multiplatform cmake in 2 lines

[src/hello_imgui/helloimgui_add_app_cmake/helloimgui_add_app.cmake](src/hello_imgui/helloimgui_add_app_cmake/helloimgui_add_app.cmake) is a cmake script that provides `helloimgui_add_app`, a cmake helper function, similar to cmake's "add_executable"

**Usage**

````cmake
include(${hello_imgui_dir}/src/hello_imgui/helloimgui_add_app_cmake/helloimgui_add_app.cmake)

helloimgui_add_app(my_app main.cpp lib.cpp ...)
````

**Features**

* It will automaticaly link the exe to hello_imgui library
* Under android, it uses [qt-android-cmake](https://github.com/LaurentGomila/qt-android-cmake.git) in order to create an apk

If needed, you can copy-paste-customize this script.

## Android

... To be continued...

With Android, it is preferred to use the Qt backend.

# Developer informations

## Adding backends

Adding new backend should be easy: simply add a new derivate of [AbstractRunner](src/hello_imgui/internal/backend_impls/abstract_runner.h).
