if (EMSCRIPTEN)
    cmake_minimum_required(VERSION 3.13) # emscripten uses target_link_options (cmake 3.13+)
else()
    cmake_minimum_required(VERSION 3.10)
endif()
if (IOS)
    set(DEPLOYMENT_TARGET "13.0" CACHE STRING "" FORCE)
endif()

project(HelloImGui LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

###############################################################################
# HelloImGui Build options
###############################################################################

#------------------------------------------------------------------------------
# Options / Backend selection
#------------------------------------------------------------------------------
# Backend selection with automatic backend compilation
# (if you set any of those options, the corresponding backend will be automatically
#  downloaded and compiled as a static library at build time)
option(HELLOIMGUI_WITH_GLFW "Add GLFW+OpenGl3 support (glfw will be automatically compiled via ExternalProject_Add)" OFF)
option(HELLOIMGUI_WITH_SDL "Add SDL+OpenGl3 support (SDL will be automatically compiled via ExternalProject_Add)" OFF)

# Backend selection with manual backend compilation
# (setting those options without setting HELLOIMGUI_WITH_XXX will require that the backend can be found by find_package())
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL+OpenGL3" OFF)
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for GLFW+OpenGL3" OFF)
option(HELLOIMGUI_USE_QT "Build HelloImGui for Qt" OFF)

#------------------------------------------------------------------------------
# Build options / ImGui
#------------------------------------------------------------------------------
# if HELLOIMGUI_BUILD_IMGUI, imgui will be built as part of the build
option(HELLOIMGUI_BUILD_IMGUI "Build imgui using sources provided in external/imgui" ON)
# HELLOIMGUI_IMGUI_SOURCE_DIR: folder containing the sources for imgui (by default in the submodule external/imgui)
set(HELLOIMGUI_IMGUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/imgui" CACHE STRING "Source dir of imgui")

#------------------------------------------------------------------------------
# Build options / HelloImGui
#------------------------------------------------------------------------------
option(HELLOIMGUI_BUILD_DOCS "Build docs" OFF)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" ON)
else()
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" OFF)
endif()
option(HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT "Create Android studio projects in build dir" OFF)

# Advanced option: use imgui as a shared library: in this case, HelloImGui and ImGui possess each a different
# copy of the global GImGui, and it needs to be synchronized accross DLL boundaries
# (see comment inside imgui.cpp at the line `ImGuiContext*   GImGui = NULL`)
option(HELLO_IMGUI_IMGUI_SHARED "Use imgui as a shared library" OFF)
mark_as_advanced(HELLO_IMGUI_IMGUI_SHARED)

if (IOS OR EMSCRIPTEN)
    set(need_opengl_loader OFF)
else()
    set(need_opengl_loader ON)
endif()
option(HELLOIMGUI_USE_GLAD "Use Glad OpenGl loader" ${need_opengl_loader})


###############################################################################
# End of options
###############################################################################

if (HELLOIMGUI_WITH_GLFW)
    set(HELLOIMGUI_USE_GLFW_OPENGL3 ON CACHE BOOL "" FORCE)

    include(ExternalProject)
    ExternalProject_Add(glfw_external
        GIT_REPOSITORY    https://github.com/glfw/glfw.git
        GIT_TAG           3.3.8
        STEP_TARGETS   build
        DOWNLOAD_NO_PROGRESS OFF
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS=OFF
        )
endif()

if (HELLOIMGUI_WITH_SDL)
    set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE BOOL "" FORCE)

    include(ExternalProject)
    ExternalProject_Add(sdl_external
        GIT_REPOSITORY    https://github.com/libsdl-org/SDL.git
        GIT_TAG           release-2.24.2
        STEP_TARGETS   build
        DOWNLOAD_NO_PROGRESS OFF
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DBUILD_SHARED_LIBS=OFF
        )
endif()

if (NOT (HELLOIMGUI_USE_SDL_OPENGL3 OR HELLOIMGUI_USE_GLFW_OPENGL3 OR HELLOIMGUI_USE_QT OR HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT))
    message(FATAL_ERROR "Select at least one backend: use either
        -DHELLOIMGUI_USE_SDL_OPENGL3=ON
        -DHELLOIMGUI_USE_GLFW_OPENGL3=ON
        -DHELLOIMGUI_USE_QT=ON
        -DHELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT=ON
    ")
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

# Install
install(DIRECTORY hello_imgui_cmake DESTINATION .)
install(DIRECTORY hello_imgui_assets DESTINATION .)

if (HELLOIMGUI_BUILD_DOCS)
    add_custom_target(hello_imgui_build_doc ALL
        ${PROJECT_SOURCE_DIR}/tools/doc/process_md_docs.py
        COMMENT "Generating md doc files"
    )
endif()

if (NOT IOS AND NOT ANDROID)
    install(FILES README.md DESTINATION .)
endif()
