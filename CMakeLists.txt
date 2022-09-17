if (EMSCRIPTEN)
    cmake_minimum_required(VERSION 3.13) # emscripten uses target_link_options (cmake 3.13+)
else()
    cmake_minimum_required(VERSION 3.10)
endif()

project(HelloImGui LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

#
# Available backends: you need to select at least one of them!
#
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL+OpenGL3" OFF)
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for GLFW+OpenGL3" OFF)
option(HELLOIMGUI_USE_QT "Build HelloImGui for Qt" OFF)
option(HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT "Create Android studio projects in build dir" OFF)
if (NOT (HELLOIMGUI_USE_SDL_OPENGL3 OR HELLOIMGUI_USE_GLFW_OPENGL3 OR HELLOIMGUI_USE_QT OR HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT))
    message(FATAL_ERROR "Select at least one backend: use either
        -DHELLOIMGUI_USE_SDL_OPENGL3=ON
        -DHELLOIMGUI_USE_GLFW_OPENGL3=ON
        -DHELLOIMGUI_USE_QT=ON
        -DHELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT=ON
    ")
endif()

# Advanced option: use imgui as a shared library: in this case, HelloImGui and ImGui possess each a different
# copy of the global GImGui, and it needs to be synchronized accross DLL boundaries
# (see comment inside imgui.cpp at the line `ImGuiContext*   GImGui = NULL`)
option(HELLO_IMGUI_IMGUI_SHARED "Use imgui as a shared library" OFF)
mark_as_advanced(HELLO_IMGUI_IMGUI_SHARED)


# Advanced option: use powersave mode for imgui: see PR https://github.com/ocornut/imgui/pull/4076
# This PR was adapted to imgui docking version of March 2022 here: https://github.com/pthom/imgui/tree/docking_powersave
# only use if your version of imgui has this mode!
option(HELLOIMGUI_USE_POWERSAVE "Use power save mode for imgui" OFF)
mark_as_advanced(HELLOIMGUI_USE_POWERSAVE)


if (IOS OR EMSCRIPTEN)
    set(need_opengl_loader OFF)
else()
    set(need_opengl_loader ON)
endif()
option(HELLOIMGUI_USE_GLAD "Use Glad OpenGl loader" ${need_opengl_loader})

#
# Build options
#
option(HELLOIMGUI_BUILD_IMGUI "Build imgui using sources provided in external/imgui" ON)
set(HELLOIMGUI_IMGUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/imgui" CACHE STRING "Source dir of imgui")

option(HELLOIMGUI_BUILD_DOCS "Build docs" OFF)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" ON)
else()
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" OFF)
endif()

#
# MSVC: Select the solution folder where hello_imgui should be placed
#
if (MSVC)
    if(NOT DEFINED HELLOIMGUI_SOLUTIONFOLDER)
        set(HELLOIMGUI_SOLUTIONFOLDER hello_imgui)
    endif()
endif()


set(HELLOIMGUI_BASEPATH ${CMAKE_CURRENT_LIST_DIR} CACHE STRING "Hello imgui base path" FORCE)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/")


include(StandardProjectSettings)
include(StaticAnalyzers)
include(msvc/msvc_target_group)


if (EMSCRIPTEN)
    include(${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/hello_imgui_emscripten_global_options.cmake)
endif()

add_subdirectory(external)
add_subdirectory(src)

if (HELLOIMGUI_BUILD_DOCS)
    add_custom_target(hello_imgui_build_doc ALL
        ${PROJECT_SOURCE_DIR}/tools/doc/process_md_docs.py
        COMMENT "Generating md doc files"
    )
endif()

install(FILES README.md DESTINATION .)
