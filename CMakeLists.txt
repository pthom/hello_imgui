cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)

project(HelloImGui LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)

#
# Available backends: you need to select at least on of them!
#
option(HELLOIMGUI_USE_QT "Build HelloImGui for Qt" OFF)
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for GLFW+OpenGL3" OFF)
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL+OpenGL3" OFF)
option(HELLOIMGUI_USE_SDL_DIRECTX11 "Build HelloImGui for SDL+DirectX11" OFF)

option(HELLOIMGUI_USE_GLAD "Use Glad OpenGl loader" ON)

#
# Build options
#
option(HELLOIMGUI_BUILD_DOCS "Build docs" OFF)
option(HELLOIMGUI_BUILD_IMGUI "Use imgui sources provided in external/imgui" ON)
option(HELLOIMGUI_BUILD_DEMOS "Build demos" ON)


set(HELLOIMGUI_BASEPATH ${CMAKE_CURRENT_LIST_DIR} CACHE STRING "Hello imgui base path" FORCE)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake/")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${HELLOIMGUI_BASEPATH}/helloimgui_cmake/")


include(StandardProjectSettings)
include(CompilerWarnings)
include(StaticAnalyzers)

# Link to this library to inherit the properties
add_library(project_properties INTERFACE)

set_project_warnings(project_properties)
# sanitizer options if supported by compiler
include(Sanitizers)
enable_sanitizers(project_properties)

add_subdirectory(external)
add_subdirectory(src)

if (HELLOIMGUI_BUILD_DOCS)
    add_custom_target(helloimgui_build_doc ALL 
        ${CMAKE_SOURCE_DIR}/tools/process_md_docs.py
        COMMENT "Generating md doc files"
    )
endif()
