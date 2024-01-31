cmake_minimum_required(VERSION 3.15)
project(test_vcpkg_imgui)
set(CMAKE_CXX_STANDARD 17)

find_package(hello-imgui CONFIG REQUIRED)
hello_imgui_add_app(test test.cpp)
