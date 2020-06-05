cmake_minimum_required(VERSION 3.10)

option(IMGUIRUNNER_QT "Build ImGuiRunner for Qt" OFF)
option(IMGUIRUNNER_GLFW "Build ImGuiRunner for GLFW" ON)

project(ImGuiRunner LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake/")

include(StandardProjectSettings)
include(CompilerWarnings)
include(Doxygen)
include(StaticAnalyzers)

# Link to this library to inherit the properties
add_library(project_properties INTERFACE)
target_compile_features(project_properties INTERFACE cxx_std_14)
set_project_warnings(project_properties)
# sanitizer options if supported by compiler
include(Sanitizers)
enable_sanitizers(project_properties)


add_subdirectory(external)
add_subdirectory(src)

# Docs options (the docs use Sphinx: see docs/index.rst)
option(BUILD_DOCS "Build docs" OFF)
# You can exclude the docs from "make all"
option(BUILD_DOCS_EXCLUDE_FROM_ALL "Build docs, but exclude them from 'build all'" OFF)
if (BUILD_DOCS)
    enable_doxygen()
    add_subdirectory(docs)
endif()
