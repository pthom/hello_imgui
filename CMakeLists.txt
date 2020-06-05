cmake_minimum_required(VERSION 3.10)

project(PROJECTNAME LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake/")

include(StandardProjectSettings)
include(CompilerWarnings)
include(Doxygen)
include(StaticAnalyzers)

# Link to this library to inherit the properties
add_library(project_properties INTERFACE)
target_compile_features(project_properties INTERFACE cxx_std_14)
# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

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
