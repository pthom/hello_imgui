cmake_minimum_required(VERSION 3.12)
project(helloworld_with_helloimgui)
set(CMAKE_CXX_STANDARD 17)

# Prepare hello_imgui during configure time
include(FetchContent)
FetchContent_Declare(hello_imgui GIT_REPOSITORY https://github.com/pthom/hello_imgui.git GIT_TAG master)
FetchContent_MakeAvailable(hello_imgui)

# Build your app
hello_imgui_add_app(hello_world hello_world.main.cpp)

# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .
