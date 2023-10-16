cmake_minimum_required(VERSION 3.12)
project(helloworld_with_helloimgui)
set(CMAKE_CXX_STANDARD 17)

# Build hello_imgui
# 1/  Option 1: simply fetch hello_imgui during the build
include(FetchContent)
FetchContent_Declare(hello_imgui GIT_REPOSITORY https://github.com/pthom/hello_imgui.git GIT_TAG master)
FetchContent_MakeAvailable(hello_imgui)
# 2/  Option 2: if hello_imgui is available as a submodule for example
# add_subdirectory(path/to/hello_imgui)

# Build your app
hello_imgui_add_app(hello_world hello_world.main.cpp)


# hello_imgui_add_app is a helper function, similar to cmake's "add_executable"
# Usage:
# hello_imgui_add_app(app_name file1.cpp file2.cpp ...)
#
# Features: 
# * It will automaticaly link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
# * It will embed the assets (for desktop, mobile, and emscripten apps)
# * It will perform additional customization (app icon and name on mobile platforms, etc)

# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .
