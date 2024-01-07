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

# Set HELLOIMGUI_BASEPATH / HELLOIMGUI_CMAKE_PATH
include(${CMAKE_CURRENT_LIST_DIR}/hello_imgui_cmake/utils/cache_hello_imgui_paths.cmake)

###############################################################################
# hello_imgui_add_app
###############################################################################
#
# hello_imgui_add_app is a helper function, similar to cmake's "add_executable"
#
# Usage:
#     hello_imgui_add_app(app_name file1.cpp file2.cpp ...)
# Or:
#     hello_imgui_add_app(app_name file1.cpp file2.cpp ... ASSETS_LOCATION "path/to/assets")
# (By default, ASSETS_LOCATION is "assets", which means that the assets will be searched in the "assets" folder,
# relative to the location of the CMakeLists.txt file)
#
# Features:
#     * It will automatically link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
#     * It will embed the assets (for desktop, mobile, and emscripten apps)
#     * It will perform additional customization (app icon and name on mobile platforms, etc)
#
include(hello_imgui_add_app)

# HELLOIMGUI_ADD_APP_WITH_INSTALL: if ON, hello_imgui_add_app will add install() instructions for the app.
# (this works under Windows, Linux, macOS, and emscripten)
# On desktop platforms, it will install the "app exe" and the "assets" folder in the install folder.
option(HELLOIMGUI_ADD_APP_WITH_INSTALL "Add cmake install() instructions with hello_imgui_add_app" ON)

# </hello_imgui_add_app>  # do not remove this line (used by the script that generates the documentation)

###############################################################################
# HelloImGui Build options
###############################################################################

################################################################################
# <Backends> # do not remove this line (used by the script that generates the documentation)
# Available backends combinations: HelloImGui needs to use at least one of them.
# If you specify no option:
# - HELLOIMGUI_USE_GLFW_OPENGL3 will be used for Windows, Linux, and macOS
# - HELLOIMGUI_USE_SDL_OPENGL3 will be used for Android, iOS and emscripten
#
# Note:
# - Only one rendering backend can be used at a time (OpenGL3, Metal or Vulkan)!
# - You may select multiple platform backends
#   (for example, you may select both SDL and Glfw3)
# - Combinations using OpenGL3 as a renderer are easier to use,
#   and are recommended, especially for beginners.
################################################################################
# Standard backends:
# Use Glfw3 + OpenGl3
option(HELLOIMGUI_USE_GLFW_OPENGL3 "Build HelloImGui for Glfw3+OpenGL3" OFF)
# Use SDL2 + OpenGL3
option(HELLOIMGUI_USE_SDL_OPENGL3 "Build HelloImGui for SDL2+OpenGL3" OFF)

# Advanced backends, for experienced users:
#   Vulkan, Metal, and DirectX11 do work, but you may need to customize the rendering code inside HelloImGui:
#   see src/hello_imgui/internal/backend_impls/rendering_xxxx.[h,cpp]
#   (using those backends probably implies that you want to heavily customize the rendering code)
# Use Glfw3 + Metal (Apple only)
option(HELLOIMGUI_USE_GLFW_METAL "Build HelloImGui for Glfw3+Metal" OFF)
# Use SDL2 + Metal (Apple only)
option(HELLOIMGUI_USE_SDL_METAL "Build HelloImGui for SDL2+Metal" OFF)
# Use Glfw3 + Vulkan
option(HELLOIMGUI_USE_GLFW_VULKAN "Build HelloImGui for Glfw3+Vulkan" OFF)
# Use SDL2 + Vulkan
option(HELLOIMGUI_USE_SDL_VULKAN "Build HelloImGui for SDL2+Vulkan" OFF)
# Use SDL2 + DirectX 11 (Note: there is a known issue with DirectX11, see src/hello_imgui/internal/backend_impls/rendering_dx11.h)
option(HELLOIMGUI_USE_SDL_DIRECTX11 "Build HelloImGui for SDL2+DirectX11" OFF)
# Use Glfw3 + DirectX 11 (Note: there is a known issue with DirectX11, see src/hello_imgui/internal/backend_impls/rendering_dx11.h)
option(HELLOIMGUI_USE_GLFW_DIRECTX11 "Build HelloImGui for Glfw3+DirectX11" OFF)
# Experimental! DirectX12 was not tested, although it does compile!
option(HELLOIMGUI_USE_SDL_DIRECTX12 "Build HelloImGui for SDL2+DirectX12" OFF)

# </Backends>  # do not remove this line (used by the script that generates the documentation)


#------------------------------------------------------------------------------
# Options / Freetype
#------------------------------------------------------------------------------
set(freetype_default ON)
if(ANDROID OR MINGW)
    # Freetype is not available on Android. The mix SDL + Freetype cause issues (cannot find SDL.h)
    # Freetype currently fails to build on MinGW
    set(freetype_default OFF)
endif()
# Note: to reduce wasm size, you may want to disable freetype on emscripten.
option(HELLOIMGUI_USE_FREETYPE "Use freetype for text rendering" ${freetype_default})

#------------------------------------------------------------------------------
# <Dependencies> # do not remove this line (used by the script that generates the documentation)
# Automatic download of Glfw3, SDL2, and Freetype (provided as a convenience)
#
# (this is disabled by default on Linux, which prefers to use the system libraries,
# enabled by default on other platforms)
#
# Note:
#
# SDL and Glfw3 will be downloaded only if:
# -----------------------------------------
#   - HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED or HELLOIMGUI_DOWNLOAD_SDL_IF_NEEDED is ON
#   - HELLOIMGUI_USE_SDL_XXXX or HELLOIMGUI_USE_GLFW_XXXX is ON
#   - SDL and/or Glfw3 were not added as CMake target
#     (add_subdirectory(external/SDL) or add_subdirectory(external/glfw) for example)
#   - find_package(glfw3 or SDL2) fails. If a system library is found,
#     or a user library provided in a path added to CMAKE_PREFIX_PATH,
#     it will be used instead
#
# - Freetype will be downloaded only if:
# --------------------------------------
#   - HELLOIMGUI_DOWNLOAD_FREETYPE is ON, HELLOIMGUI_USE_FREETYPE is ON
#   - Freetype was not added as a CMake target
#         (add_subdirectory(external/freetype) for example)
#   - find_package(freetype) fails
#   (it will also be forcibly downloaded if HELLOIMGUI_FREETYPE_STATIC is ON)
#
#
# Automatic download of SDL2, Glfw3, and Freetype is disabled by default on Linux,
# because it is recommended to use the system libraries instead:
# On ubuntu, you can install them with:
#    sudo apt install libglfw3-dev libsdl2-dev libfreetype-dev
#
#------------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(autodownload_default OFF)
else()
    set(autodownload_default ON)
endif()
option(HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED "Download and build GLFW if needed" ${autodownload_default})
option(HELLOIMGUI_DOWNLOAD_SDL_IF_NEEDED "Download and build GLFW if needed" ${autodownload_default})
option(HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED "Download and build Freetype if needed" ${autodownload_default})
option(HELLOIMGUI_FREETYPE_STATIC "Force static linking of freetype (only used for python bindings)" OFF)

# </Dependencies>  # do not remove this line (used by the script that generates the documentation)

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
# Instructions: how to enable Emscripten multithreading support
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
set(HELLOIMGUI_TARGET hello_imgui CACHE STRING "Name of the hello_imgui target")

option(HELLOIMGUI_BUILD_DOCS "Build docs" OFF)
mark_as_advanced(HELLOIMGUI_BUILD_DOCS)

option(HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT "Create Android studio projects in build dir" OFF)
mark_as_advanced(HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT)

# Advanced option: use imgui as a shared library: in this case, HelloImGui and ImGui possess each a different
# copy of the global GImGui, and it needs to be synchronized accross DLL boundaries
# (see comment inside imgui.cpp at the line `ImGuiContext*   GImGui = NULL`)
option(HELLO_IMGUI_IMGUI_SHARED "Use imgui as a shared library" OFF)
mark_as_advanced(HELLO_IMGUI_IMGUI_SHARED)

###############################################################################
# End of options
###############################################################################




###############################################################################
# HelloImGui Build Actions
###############################################################################

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

add_subdirectory(src)

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
        COMMENT "Generating doc"
    )
endif()

if (PROJECT_IS_TOP_LEVEL AND NOT IOS AND NOT ANDROID)
    install(FILES README.md DESTINATION .)
endif()

#------------------------------------------------------------------------------
# Log info
#------------------------------------------------------------------------------
him_log_configuration()
