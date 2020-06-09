cmake_minimum_required(VERSION 3.15)
cmake_policy(VERSION 3.15)

# Docs options (the docs use Sphinx: see docs/index.rst)
option(HELLOIMGUI_BUILD_DOCS "Build docs" OFF)
# You can exclude the docs from "make all"
option(HELLOIMGUI_BUILD_DOCS_EXCLUDE_FROM_ALL "Build docs, but exclude them from 'build all'" OFF)

option(HELLOIMGUI_BUILD_IMGUI "Use imgui sources provided in external/imgui" ON)
option(HELLOIMGUI_USE_QT "Build ImGuiRunner for Qt" OFF)
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build ImGuiRunner for GLFW+OpenGL3" ON)
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build ImGuiRunner for SDL+OpenGL3" ON)
option(HELLOIMGUI_USE_SDL_DIRECTX11 "Build ImGuiRunner for SDL+DirectX11" ON)

option(HELLOIMGUI_USE_GLAD "Use Glad OpenGl loader" ON)

project(ImGuiRunner LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake/")

include(StandardProjectSettings)
include(CompilerWarnings)
include(StaticAnalyzers)
if (BUILD_DOCS)
    include(Doxygen)
endif()

# Link to this library to inherit the properties
add_library(project_properties INTERFACE)
target_compile_features(project_properties INTERFACE cxx_std_14)
set_project_warnings(project_properties)
# sanitizer options if supported by compiler
include(Sanitizers)
enable_sanitizers(project_properties)


add_subdirectory(external)
add_subdirectory(src)

if (HELLOIMGUI_BUILD_DOCS)
    enable_doxygen()
    add_subdirectory(docs)
endif()
