if (EMSCRIPTEN)
    cmake_minimum_required(VERSION 3.13) # emscripten uses target_link_options (cmake 3.13+)
else()
    cmake_minimum_required(VERSION 3.10)
endif()

project(hello-imgui LANGUAGES C CXX VERSION "1.5.2")  # v1.5.2
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
#
# You need to select at least two backends:
#
#     - At least one (or more) rendering backend (OpenGL3, Metal, Vulkan, DirectX11, DirectX12)
#       Make your choice according to your needs and your target platforms, between:
#          -DHELLOIMGUI_HAS_OPENGL3=ON    # This is the recommended choice, especially for beginners
#          -DHELLOIMGUI_HAS_METAL=ON      # Apple only, advanced users only
#          -DHELLOIMGUI_HAS_VULKAN=ON     # Advanced users only
#          -DHELLOIMGUI_HAS_DIRECTX11=ON  # Windows only, still experimental
#          -DHELLOIMGUI_HAS_DIRECTX12=ON  # Windows only, advanced users only, still experimental
#
#     - At least one (or more) platform backend (SDL2, Glfw3):
#      Make your choice according to your needs and your target platforms, between:
#          -DHELLOIMGUI_USE_SDL2=ON
#          -DHELLOIMGUI_USE_GLFW3=ON
#
# If you make no choice, the default will be selected:
#     HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_OPENGL3
#
# Note about rendering backends:
#   OpenGL3 is the recommended choice as a starting point, especially for beginners.
#   Vulkan, Metal, and DirectX11, DirectX12 do work, but you may need to customize the rendering code inside HelloImGui:
#   see src/hello_imgui/internal/backend_impls/rendering_xxxx.[h,cpp]
#   (using those backends probably implies that you want to heavily customize the rendering code)
#
################################################################################
# Platform backends:
option(HELLOIMGUI_USE_GLFW3 "Use Glfw3 as a platform backend" OFF)
option(HELLOIMGUI_USE_SDL2 "Use Sdl2 as a platform backend" OFF)
# Rendering backends
option(HELLOIMGUI_HAS_OPENGL3 "Use OpenGL3 as a rendering backend" OFF)
option(HELLOIMGUI_HAS_METAL "Use Metal as a rendering backend" OFF)
option(HELLOIMGUI_HAS_VULKAN "Use Vulkan as a rendering backend" OFF)
option(HELLOIMGUI_HAS_DIRECTX11 "Use DirectX11 as a rendering backend" OFF)
option(HELLOIMGUI_HAS_DIRECTX12 "Use DirectX12 as a rendering backend" OFF)

# Headless mode: by default, HelloImGui's cmake tooling will always check that there is at least one
# rendering backend and one platform backend that is not a "Null" backend.
# If you set HELLOIMGUI_HEADLESS, you can disable this check, and compile HelloImGui,
# using only the Null rendering/platform backends.
option(HELLOIMGUI_HEADLESS "Allow headless mode (will use Null rendering/platform backend)" OFF)
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
# Options / Install
# By default, hello_imgui install its targets only if it is the top level project
#------------------------------------------------------------------------------
option(HELLOIMGUI_INSTALL "Install hello_imgui" ${PROJECT_IS_TOP_LEVEL})

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
option(HELLOIMGUI_STB_IMAGE_IMPLEMENTATION "Implement STB_IMAGE_IMPLEMENTATION inside hello_imgui" ON)
option(HELLOIMGUI_STB_IMAGE_WRITE_IMPLEMENTATION "Implement STB_IMAGE_WRITE_IMPLEMENTATION inside hello_imgui" ON)

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


# -----------------------------------------------------------------------------
# How to build offline with vcpkg
# -----------------------------------------------------------------------------
# By default, hello_imgui will download and build the required libraries automatically,
# if they cannot be found via cmake's find_package:
#   glfw3, lunasvg, freetype
# To build offline, you can for example pre-install the required libraries with vcpkg,
# but you can also install them via a system package manager, or add them manually as targets.
#
# see below an example with vcpkg
#
# Install requirements via vcpkg
# --------------------------------------------
#
#    git clone https://github.com/microsoft/vcpkg
#    ./vcpkg/bootstrap-vcpkg.sh
#    ./vcpkg/vcpkg install glfw3 freetype lunasvg
#    export CMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake
#    mkdir build && cd build
#    cmake ..
#
# Note: under windows, you could specify a static triplet (x64-windows-static):
#     ./vcpkg/vcpkg install freetype:x64-windows-static glfw3:x64-windows-static lunasvg:x64-windows-static
#
# -----------------------------------------------------------------------------
# How to use ImGui package from vcpkg (*not* recommended)
# -----------------------------------------------------------------------------
# HelloImGui provides its own copy of imgui, as a submodule (on the branch "docking")
# it is possible, but not recommended, to use imgui from vcpkg via the option
#     HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE
# In this case, you will need to install vcpkg, and install imgui with the correct options.
# (see below an example with vcpkg)
# (this method is not really recommended, since ImGui voluntarily chose to not provide cmake tooling)
#
#    git clone https://github.com/microsoft/vcpkg
#    ./vcpkg/bootstrap-vcpkg.sh
#    ./vcpkg/vcpkg install glfw3 freetype lunasvg
#    # install imgui from vcpkg, with the correct options
#    ./vcpkg/vcpkg install "imgui[docking-experimental,freetype-lunasvg,glfw-binding,opengl3-binding]"
#
#    export CMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake
#    mkdir build && cd build
#    cmake .. -DHELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE=ON
#
option(HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE "Use imgui from cmake package (provided by vcpkg for example) * Must be on docking branch*" OFF)

#------------------------------------------------------------------------------
# Options / Remoting
# (Unsupported, and highly experimental. Requires specific forks of the remote rendering libraries)
#------------------------------------------------------------------------------
# Using https://github.com/sammyfreg/netImgui
option(HELLOIMGUI_WITH_NETIMGUI "Use netImgui for remote rendering" OFF)
# Using https://github.com/ggerganov/imgui-ws
option(HELLOIMGUI_WITH_IMGUIWS "Use imgui-ws for remote rendering" OFF)

###############################################################################
# End of options
###############################################################################


###############################################################################
# HelloImGui Build Actions
###############################################################################

# Avoid building hello_imgui twice
if(TARGET ${HELLOIMGUI_TARGET})
    message(STATUS "hello_imgui target (${HELLOIMGUI_TARGET}) target already defined!")
    return()
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


add_subdirectory(src)

if (HELLOIMGUI_BUILD_DOCS)
    if (NOT WIN32)
        add_custom_target(hello_imgui_build_doc ALL
            ${PROJECT_SOURCE_DIR}/tools/doc/process_md_docs.py
            COMMENT "Generating doc"
        )
    else()
        message(WARNING "HELLOIMGUI_BUILD_DOCS is not supported on Windows")
    endif()
endif()


#------------------------------------------------------------------------------
# automation tests (set cache var HELLOIMGUI_BUILD_AUTOMATION_TEST
# or HELLOIMGUI_BUILD_VCPKG_PACKAGE_TEST to enable)
# by default, they should only be built standalone
# (run cmake inside .github/ci_automation_tests or .github/ci_vcpkg_package_tests)
#------------------------------------------------------------------------------
if(HELLOIMGUI_BUILD_AUTOMATION_TEST)
    add_subdirectory(.github/ci_automation_tests)
endif()
if(HELLOIMGUI_BUILD_VCPKG_PACKAGE_TEST)
    add_subdirectory(.github/ci_vcpkg_package_tests)
endif()


#------------------------------------------------------------------------------
# CMake shenanigans so that find_package(hello-imgui CONFIG REQUIRED) works
#------------------------------------------------------------------------------
# These elements are mainly used to build a vcpkg compatible package.
# Important note:
#     - the CMake package is named hello-imgui (with a "-")
#     - the CMake target is named hello_imgui (with a "_")
# To link with hello_imgui, you need to use:
#     find_package(hello-imgui CONFIG REQUIRED)
#     target_link_libraries(your_target PRIVATE hello-imgui::hello_imgui)  # note the variation "-" vs "_"
# However, the simplest use is:
#     find_package(hello-imgui CONFIG REQUIRED)
#     hello_imgui_add_app(test test.cpp) # this will automatically link with hello_imgui
#------------------------------------------------------------------------------
if(HELLOIMGUI_INSTALL)
    # inspired from https://iamsorush.com/posts/cpp-cmake-config/
    message(STATUS "HELLOIMGUI_INSTALLABLE_DEPENDENCIES=${HELLOIMGUI_INSTALLABLE_DEPENDENCIES}")
    install(TARGETS ${HELLOIMGUI_INSTALLABLE_DEPENDENCIES}
        EXPORT hello-imgui-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

    install(EXPORT hello-imgui-targets
        FILE hello-imgui-targets.cmake
        NAMESPACE hello-imgui::
        DESTINATION lib/cmake/hello_imgui)

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "hello-imguiConfigVersion.cmake"
        VERSION ${hello_imgui_VERSION}
        COMPATIBILITY AnyNewerVersion)

    install(FILES "hello_imgui_cmake/hello-imguiConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/hello-imguiConfigVersion.cmake"
        DESTINATION lib/cmake/hello_imgui)
endif()

#------------------------------------------------------------------------------
# Log info
#------------------------------------------------------------------------------
him_log_configuration()
