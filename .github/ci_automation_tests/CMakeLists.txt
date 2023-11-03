cmake_minimum_required(VERSION 3.10)
project(HelloImGuiCiAutomationTests LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)


if(NOT TARGET hello_imgui)
    set(HELLOIMGUI_WITH_TEST_ENGINE ON CACHE BOOL "" FORCE)
    add_subdirectory(../../ hello_imgui)
endif()

hello_imgui_add_app(ci_automation_test_app ci_automation_test_app.cpp)
