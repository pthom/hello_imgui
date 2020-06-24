cmake_minimum_required(VERSION 3.10)
# cmake_policy(VERSION 3.15)

project(HelloImGui LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

#
# Available backends: you need to select at least one of them!
#
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL+OpenGL3" OFF)
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for GLFW+OpenGL3" OFF)
option(HELLOIMGUI_USE_QT "Build HelloImGui for Qt" OFF)
option(HELLOIMGUI_USE_SDL_DIRECTX11 "Build HelloImGui for SDL+DirectX11" OFF)

if (IOS OR EMSCRIPTEN)
    set(need_opengl_loader OFF)
else()
    set(need_opengl_loader ON)
endif()
option(HELLOIMGUI_USE_GLAD "Use Glad OpenGl loader" ${need_opengl_loader})

#
# Build options
#
option(HELLOIMGUI_BUILD_DOCS "Build docs" OFF)
option(HELLOIMGUI_BUILD_IMGUI "Use imgui sources provided in external/imgui" ON)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" ON)
else()
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" OFF)
endif()

set(HELLOIMGUI_BASEPATH ${CMAKE_CURRENT_LIST_DIR} CACHE STRING "Hello imgui base path" FORCE)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/")


include(StandardProjectSettings)
include(CompilerWarnings)
include(StaticAnalyzers)

# Link to this library to inherit the properties
add_library(project_properties INTERFACE)

set_project_warnings(project_properties)
# sanitizer options if supported by compiler
if(NOT ANDROID)
    include(Sanitizers) # due to cmake 3.10 limitation
    enable_sanitizers(project_properties)
endif()

if (EMSCRIPTEN)
    include(${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/hello_imgui_emscripten_global_options.cmake)
endif()

add_subdirectory(external)
add_subdirectory(src)

if (HELLOIMGUI_BUILD_DOCS)
    add_custom_target(hello_imgui_build_doc ALL
        ${CMAKE_SOURCE_DIR}/tools/doc/process_md_docs.py
        COMMENT "Generating md doc files"
    )
endif()
