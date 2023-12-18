if (EMSCRIPTEN)
    cmake_minimum_required(VERSION 3.13) # emscripten uses target_link_options (cmake 3.13+)
else()
    cmake_minimum_required(VERSION 3.10)
endif()

project(HelloImGui LANGUAGES C CXX VERSION "1.0.0")
set(CMAKE_CXX_STANDARD 17)

if (IOS)
    set(DEPLOYMENT_TARGET "13.0" CACHE STRING "" FORCE)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE CACHE BOOL "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_CURRENT_LIST_DIR}/hello_imgui_cmake/utils/cache_hello_imgui_paths.cmake)

###############################################################################
# hello_imgui_add_app
###############################################################################
# hello_imgui_add_app is a helper function, similar to cmake's "add_executable"
# Usage:
# hello_imgui_add_app(app_name file1.cpp file2.cpp ...)
#
# Features: 
#     * It will automatically link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
#     * It will embed the assets (for desktop, mobile, and emscripten apps)
#     * It will perform additional customization (app icon and name on mobile platforms, etc)
# Make cmake function `hello_imgui_add_app` available
include(hello_imgui_add_app)


###############################################################################
# HelloImGui Build options
###############################################################################

#------------------------------------------------------------------------------
# Options / Windows: provide WinMain automatically
#------------------------------------------------------------------------------
if (WIN32)
    option(HELLOIMGUI_WIN32_NO_CONSOLE "Under windows, build apps without Dos Console" ON)
    option(HELLOIMGUI_WIN32_AUTO_WINMAIN "Under windows, automatically provide a WinMain (provide `int main(int, char**)`, it will be called by WinMain())" ON)
endif()
#------------------------------------------------------------------------------
# Options / macOS: provide regular terminal executables, not app bundles
#------------------------------------------------------------------------------
if(MACOSX)
    option(HELLOIMGUI_MACOS_NO_BUNDLE "Under macOS, build regular terminal executables, not app bundles" OFF)
endif()

#------------------------------------------------------------------------------
# Glfw Backend
#------------------------------------------------------------------------------
# if HELLOIMGUI_WITH_GLFW is ON, Glfw may be downloaded and built automatically
#     (unless HELLOIMGUI_USE_GLFW_SYSTEM_LIB is ON, in which case it will be searched via find_package())
option(HELLOIMGUI_WITH_GLFW "Add GLFW backend (download if needed)" OFF)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(HELLOIMGUI_USE_GLFW_SYSTEM_LIB "Force usage of GLFW system library" ON)
else()
    option(HELLOIMGUI_USE_GLFW_SYSTEM_LIB "Force usage of GLFW system library" OFF)
endif()
# if HELLOIMGUI_USE_GLFW_OPENGL3 is ON, Glfw backend will be built,
# but Glfw need to be found via find_package()
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for GLFW+OpenGL3" OFF)

#------------------------------------------------------------------------------
# SDL Backend
#------------------------------------------------------------------------------
# if HELLOIMGUI_WITH_SDL is ON, SDL may be downloaded and built automatically
#     (unless HELLOIMGUI_USE_GLFW_SYSTEM_LIB is ON, in which case it will be searched via find_package())
option(HELLOIMGUI_WITH_SDL "Add SDL backend (download if needed)" OFF)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(HELLOIMGUI_USE_SDL_SYSTEM_LIB "Force usage of SDL system library" ON)
else()
    option(HELLOIMGUI_USE_SDL_SYSTEM_LIB "Force usage of SDL system library" OFF)
endif()
# if HELLOIMGUI_USE_SDL_OPENGL3 is ON, SDL backend will be built,
# but SDL need to be found via find_package()
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL+OpenGL3" OFF)


#------------------------------------------------------------------------------
# Build options / ImGui
#------------------------------------------------------------------------------
# HELLOIMGUI_IMGUI_SOURCE_DIR: folder containing the sources for imgui (by default in the submodule external/imgui)
if (NOT DEFINED HELLOIMGUI_IMGUI_SOURCE_DIR)
    set(HELLOIMGUI_IMGUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/imgui" CACHE STRING "Source dir of ImGui")
endif()
# if HELLOIMGUI_BUILD_IMGUI, imgui will be built as part of the build of HelloImGui
if (NOT DEFINED HELLOIMGUI_BUILD_IMGUI)
    option(HELLOIMGUI_BUILD_IMGUI "Build ImGui as part of HelloImGui" ON)
endif()

#------------------------------------------------------------------------------
# Common build options / HelloImGui
#------------------------------------------------------------------------------
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" ON)
else()
    option(HELLOIMGUI_BUILD_DEMOS "Build demos" OFF)
endif()
option(HELLOIMGUI_BUILD_TESTS "Build tests" OFF)

#------------------------------------------------------------------------------
# Options / ImGui Test Engine
#------------------------------------------------------------------------------
option(HELLOIMGUI_WITH_TEST_ENGINE "Provide ImGui Test engine" OFF)
message(STATUS "HELLOIMGUI_WITH_TEST_ENGINE=${HELLOIMGUI_WITH_TEST_ENGINE}")

#------------------------------------------------------------------------------
# Instructions: how to enable Esmcripten multihtread support
#------------------------------------------------------------------------------
if (EMSCRIPTEN)
    # 1. set HELLOIMGUI_EMSCRIPTEN_PTHREAD to ON to enable support for multithreading
    option(HELLOIMGUI_EMSCRIPTEN_PTHREAD "Build emscripten with multithreading support" OFF)

    # 2. With multithreading support, automatic memory growth can be slow with emscripten, and is disabled by default
    # In that case, you can call
    #    hello_imgui_set_emscripten_target_initial_memory_megabytes(your_app_name nb_megabytes)
    # to set the initial memory for a given target
    option(HELLOIMGUI_EMSCRIPTEN_PTHREAD_ALLOW_MEMORY_GROWTH "Allow memory growth with emscripten even if multithreading support is on" OFF)

    # 3. Enable multithreading per target:
    #        if(EMSCRIPTEN)
    #            target_link_options(your_app PRIVATE -pthread)
    #            target_link_options(your_app PRIVATE -sPTHREAD_POOL_SIZE=3)  # Specify the needed number of threads!
    #        endif()

    # 4. Warning: a specific http server may be needed!
    #    You will need a server that sends specific http headers (Cross Origin Opener Policy (COOP) and Cross Origin Embedder Policy (COEP))
    #    HelloImGui provides a demo web server which provides that sends those headers. You can run it like this:
    #        python tools/emscripten/webserver_multithread_policy.py
endif()


#------------------------------------------------------------------------------
# Advanced build options / HelloImGui
#------------------------------------------------------------------------------
option(HELLOIMGUI_BUILD_DOCS "Build docs" OFF)
mark_as_advanced(HELLOIMGUI_BUILD_DOCS)

option(HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT "Create Android studio projects in build dir" OFF)
mark_as_advanced(HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT)

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


#------------------------------------------------------------------------------
# Backend check: If no backend option is selected, 
# either select Glfw automatically if possible, or fail
#------------------------------------------------------------------------------
# 
if (NOT HELLOIMGUI_WITH_GLFW
    AND NOT HELLOIMGUI_WITH_SDL
    AND NOT HELLOIMGUI_USE_SDL_OPENGL3
    AND NOT HELLOIMGUI_USE_GLFW_OPENGL3
    AND NOT EMSCRIPTEN
    )
    set(backend_message "
                    In order to select your own backend, use one of the cmake options below:
                    -DHELLOIMGUI_WITH_GLFW=ON             # To download and build glfw automatically
                    -DHELLOIMGUI_WITH_SDL=ON              # To download and build SDL automatically
                    -DHELLOIMGUI_USE_GLFW_OPENGL3=ON      # To use your own version of GLFW (it should be findable via find_package(glfw3))
                    -DHELLOIMGUI_USE_SDL_OPENGL3=ON       # To use your own version of SDL (it should be findable via find_package(SDL2))

                    (Note: under Linux, it is advised to use system-wide libraries, and not to use
                    -DHELLOIMGUI_WITH_GLFW=ON or -DHELLOIMGUI_WITH_SDL=ON)
    ")
    if (NOT HELLOIMGUI_USE_GLFW_SYSTEM_LIB)
        set(HELLOIMGUI_WITH_GLFW ON)
        message(STATUS "
                HelloImGui: using Glfw as default default backend (it will be downloaded and built inside {CMAKE_CURRENT_BINARY_DIR}/_deps/glfw-*)
                ${backend_message}
                ")
    else()
        # Check if Glfw can be found
        find_package(glfw3 QUIET)
        if (glfw3_FOUND)
            set(HELLOIMGUI_USE_GLFW_OPENGL3 ON)
            message(STATUS
                    "HelloImGui: using Glfw as default default backend (it was found via find_package(glfw3))
                    ${backend_message}
                    ")
        else()
            set(glfw_help_msg "
                you can install glfw via your package manager (apt, pacman, etc).
                For example, on Ubuntu, you can run:
                    sudo apt install libglfw3-dev
                on macOS you can run:
                    brew install glfw3
            ")
            message(FATAL_ERROR "
                        HelloImGui: no backend selected, and could not find Glfw via find_package(glfw3).                    
                        ${backend_message} ${glfw_help_msg}
                        ")
        endif()
    endif()
endif()


###############################################################################
# HelloImGui Build Actions
###############################################################################

#------------------------------------------------------------------------------
# use SDL for emscripten
#------------------------------------------------------------------------------
if (EMSCRIPTEN AND NOT HELLOIMGUI_USE_SDL_OPENGL3 AND NOT HELLOIMGUI_USE_GLFW_OPENGL3)
    set(HELLOIMGUI_USE_SDL_OPENGL3 ON)
endif()


#------------------------------------------------------------------------------
# Check options logic
#------------------------------------------------------------------------------
if (HELLOIMGUI_WITH_GLFW)
    set(HELLOIMGUI_USE_GLFW_OPENGL3 ON)
endif()
if (HELLOIMGUI_WITH_SDL)
    set(HELLOIMGUI_USE_SDL_OPENGL3 ON)
endif()

# Shall we fetch glfw? (fill shall_fetch_glfw)
set(shall_fetch_glfw OFF)
if (HELLOIMGUI_WITH_GLFW
    AND NOT HELLOIMGUI_USE_GLFW_SYSTEM_LIB
    AND NOT TARGET glfw
    AND NOT EMSCRIPTEN
    )
    find_package(glfw3 QUIET)
    if (NOT glfw3_FOUND)
        set(shall_fetch_glfw ON)
    endif()
endif()

# Shall we fetch sdl? (fill shall_fetch_sdl)
set(shall_fetch_sdl OFF)
if (HELLOIMGUI_WITH_SDL
    AND NOT HELLOIMGUI_USE_SDL_SYSTEM_LIB
    AND NOT TARGET sdl
    AND NOT EMSCRIPTEN
    )
    find_package(SDL2 QUIET)
    if (NOT SDL2_FOUND)
        set(shall_fetch_sdl ON)
    endif()
endif()


#------------------------------------------------------------------------------
# Download backend glfw or sdl if required
#------------------------------------------------------------------------------
if (shall_fetch_glfw)
    message(STATUS "HelloImGui: downloading and building glfw")
    include(FetchContent)
    Set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(glfw
        GIT_REPOSITORY    https://github.com/glfw/glfw.git
        GIT_TAG           3.3.8
        GIT_PROGRESS TRUE
        )
    set(need_fetch_make_available_glfw ON)
endif()

if (shall_fetch_sdl)
    message(STATUS "HelloImGui: downloading and building SDL")
    include(FetchContent)
    Set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(sdl
        GIT_REPOSITORY    https://github.com/libsdl-org/SDL.git
        GIT_TAG           release-2.24.2
        GIT_PROGRESS TRUE
        )
    set(need_fetch_make_available_sdl ON)
endif()

if (NOT (HELLOIMGUI_USE_SDL_OPENGL3 OR HELLOIMGUI_USE_GLFW_OPENGL3 OR HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT))
    message(FATAL_ERROR "Select at least one backend: use either
        -DHELLOIMGUI_USE_SDL_OPENGL3=ON
        -DHELLOIMGUI_USE_GLFW_OPENGL3=ON
        -DHELLOIMGUI_USE_QT=ON
        -DHELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT=ON
    ")
endif()

#------------------------------------------------------------------------------
# MSVC: Select the solution folder where hello_imgui should be placed
#------------------------------------------------------------------------------
if (MSVC)
    if(NOT DEFINED HELLOIMGUI_SOLUTIONFOLDER)
        set(HELLOIMGUI_SOLUTIONFOLDER hello_imgui)
    endif()
endif()


#------------------------------------------------------------------------------
# Main build actions
#------------------------------------------------------------------------------
include(cmake/StandardProjectSettings.cmake)
include(cmake/StaticAnalyzers.cmake)
include(msvc/msvc_target_group)


if (EMSCRIPTEN)
    include(${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/hello_imgui_emscripten_global_options.cmake)
endif()


add_subdirectory(external)
add_subdirectory(src)

#------------------------------------------------------------------------------
# imgui_test_engine integration
#------------------------------------------------------------------------------
if (HELLOIMGUI_WITH_TEST_ENGINE)
    include(${CMAKE_CURRENT_LIST_DIR}/src/hello_imgui_test_engine_integration/hello_imgui_test_engine_cmake.cmake)
    add_imgui_test_engine()
endif()

#------------------------------------------------------------------------------
# automation tests (set cache var HELLOIMGUI_BUILD_AUTOMATION_TEST to enable)
# by default, they should only be built standalone, (run cmake inside .github/ci_automation_tests)
#------------------------------------------------------------------------------
if(HELLOIMGUI_BUILD_AUTOMATION_TEST)
    add_subdirectory(.github/ci_automation_tests)
endif()

#------------------------------------------------------------------------------
# install
#------------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
    install(DIRECTORY hello_imgui_cmake DESTINATION .)
    install(DIRECTORY hello_imgui_assets DESTINATION .)
endif()

if (HELLOIMGUI_BUILD_DOCS)
    add_custom_target(hello_imgui_build_doc ALL
        ${PROJECT_SOURCE_DIR}/tools/doc/process_md_docs.py
        COMMENT "Generating md doc files"
    )
endif()

if (NOT IOS AND NOT ANDROID)
    install(FILES README.md DESTINATION .)
endif()
