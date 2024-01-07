###################################################################################################
#
# This file contains the main CMake logic that is used to build HelloImGui
#
# It is included by src/hello_imgui/CMakeLists.txt, which will call
#    him_main_add_hello_imgui_library()
# which is defined a the bottom of this file
#
###################################################################################################

# allows the linking of a target in a different directory
# Needed to add freetype to the imgui target, if it is build externally
cmake_policy(SET CMP0079 NEW)

###################################################################################################
# Add library and sources: API = him_add_hello_imgui
###################################################################################################
function(him_add_hello_imgui)
    if (APPLE)
        file(GLOB_RECURSE sources *.h *.cpp *.c *.mm)
    else()
        file(GLOB_RECURSE sources *.h *.cpp *.c)
    endif()
    add_library(${HELLOIMGUI_TARGET} ${sources})
    if(APPLE AND NOT IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_MACOS)
    endif()
    target_include_directories(${HELLOIMGUI_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC imgui)
endfunction()


###################################################################################################
# Build imgui: API = him_build_imgui
###################################################################################################
function(him_build_imgui)
    _him_checkout_imgui_submodule_if_needed()
    if (HELLOIMGUI_BUILD_IMGUI)
        _him_do_build_imgui()
        _him_install_imgui()
    endif()
    if (HELLOIMGUI_USE_FREETYPE)
        _him_add_freetype_to_imgui()
    endif()
endfunction()

function(_him_checkout_imgui_submodule_if_needed)
    if (HELLOIMGUI_BUILD_IMGUI)
        # if HELLOIMGUI_IMGUI_SOURCE_DIR is  CMAKE_CURRENT_LIST_DIR/imgui
        # and the submodule is not present, update submodules
        if (HELLOIMGUI_IMGUI_SOURCE_DIR STREQUAL ${HELLOIMGUI_BASEPATH}/external/imgui)
            if (NOT EXISTS ${HELLOIMGUI_IMGUI_SOURCE_DIR}/imgui.h)
                # Run git submodule update --init --recursive
                message(WARNING "Updating imgui submodule")
                execute_process(
                    COMMAND git submodule update --init --recursive
                    WORKING_DIRECTORY ${HELLOIMGUI_BASEPATH})
            endif()
        endif()
    endif()
endfunction()

function(_him_do_build_imgui)
    file(GLOB imgui_sources ${HELLOIMGUI_IMGUI_SOURCE_DIR}/*.h ${HELLOIMGUI_IMGUI_SOURCE_DIR}/*.cpp)
    set(imgui_sources ${imgui_sources}
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.h)
    if (HELLO_IMGUI_IMGUI_SHARED)
        add_library(imgui SHARED ${imgui_sources})
        install(TARGETS imgui DESTINATION ./lib/)
    else()
        add_library(imgui ${imgui_sources})
    endif()
    target_include_directories(imgui PUBLIC ${HELLOIMGUI_IMGUI_SOURCE_DIR})

    if (MSVC)
        hello_imgui_msvc_target_set_folder(imgui ${HELLOIMGUI_SOLUTIONFOLDER}/external)
    endif()
endfunction()

function(_him_add_freetype_to_imgui)
    # Add freetype + lunasvg to imgui
    # This is especially useful to support emojis (possibly colored) in imgui
    # See doc:
    #     https://github.com/ocornut/imgui/blob/master/docs/FONTS.md#using-colorful-glyphsemojis
    # We have to
    # - add imgui_freetype.cpp and imgui_freetype.h to imgui
    # - enable freetype in imgui via IMGUI_ENABLE_FREETYPE
    # - enable lunasvg in imgui via IMGUI_ENABLE_FREETYPE_LUNASVG
    # - add lunasvg to imgui
    # - define IMGUI_USE_WCHAR32 in imgui

    # Note: also change add_imgui.cmake in bundle!

    #
    # 1. Build or find freetype (if downloaded, make sure it is static)
    #
    if(TARGET freetype)
        message(STATUS "HelloImGui: using freetype target")
        set(freetype_linked_library freetype)
    else()
        set(download_freetype OFF)
        if (HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED)
            find_package(Freetype 2.12 QUIET)
            if (NOT Freetype_FOUND)
                set(download_freetype ON)
            endif()
        endif()
        if(HELLOIMGUI_FREETYPE_STATIC)
            set(download_freetype ON)
        endif()

        if (download_freetype)
            message(STATUS "HelloImGui: downloading and building freetype")

            set(backup_shared_lib ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

            include(FetchContent)
            if(IOS)
                set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
                set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
            endif()
            FetchContent_Declare(
                freetype
                GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
                GIT_TAG        VER-2-13-2
                GIT_PROGRESS TRUE
            )
            FetchContent_MakeAvailable(freetype)
            set(freetype_linked_library freetype)

            set(BUILD_SHARED_LIBS ${backup_shared_lib} CACHE BOOL "" FORCE)
        else()
            find_package(Freetype 2.12 QUIET)
            if(NOT Freetype_FOUND AND NOT HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED)
                message(STATUS "
                    HelloImGui: freetype not found. You may set
                        -DHELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED=ON
                    to download and build freetype automatically
                ")
            endif()
            find_package(Freetype 2.12 REQUIRED)
            set(freetype_linked_library Freetype::Freetype)
        endif()
    endif()

    target_link_libraries(imgui PUBLIC ${freetype_linked_library})

    if(TARGET freetype)
        set(HELLOIMGUI_FREETYPE_SELECTED_INFO "Use target freetype" CACHE INTERNAL "" FORCE)
    elseif(download_freetype)
        set(HELLOIMGUI_FREETYPE_SELECTED_INFO "Downloaded VER-2-13-2" CACHE INTERNAL "" FORCE)
    else()
        set(HELLOIMGUI_FREETYPE_SELECTED_INFO "Use system Library" CACHE INTERNAL "" FORCE)
    endif()

    #
    # 2. Build lunasvg (static)
    #
    # Fetch and build lunasvg
    if(NOT TARGET lunasvg)
        set(backup_shared_lib ${BUILD_SHARED_LIBS})
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        include(FetchContent)
        FetchContent_Declare(lunasvg
            GIT_REPOSITORY https://github.com/sammycage/lunasvg
            GIT_TAG        v2.3.9
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(lunasvg)
        set(BUILD_SHARED_LIBS ${backup_shared_lib} CACHE BOOL "" FORCE)
    endif()
    target_link_libraries(imgui PUBLIC lunasvg)

    #
    # 3. Add freetype and LunaSvg support to imgui
    #    with support for wchar32 (for emojis, and other unicode characters)
    target_sources(imgui PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype/imgui_freetype.h)
    target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE IMGUI_ENABLE_FREETYPE_LUNASVG)
    target_compile_definitions(imgui PUBLIC IMGUI_USE_WCHAR32)
endfunction()

function(_him_install_imgui)
    if(PROJECT_IS_TOP_LEVEL)
        install(FILES ${imgui_sources} DESTINATION imgui)
        install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends DESTINATION imgui)
        install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp DESTINATION imgui/misc)
        install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype DESTINATION imgui/misc)
    endif()
endfunction()


###################################################################################################
# Sanity checks: API = him_sanity_checks + him_get_active_backends (displayed after configure)
###################################################################################################
function(him_sanity_checks)
    # use SDL for emscripten
    if (EMSCRIPTEN AND NOT HELLOIMGUI_USE_SDL_OPENGL3 AND NOT HELLOIMGUI_USE_GLFW_OPENGL3)
        set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()
    if (IOS AND NOT HELLOIMGUI_USE_SDL_OPENGL3 AND NOT HELLOIMGUI_USE_GLFW_OPENGL3 AND NOT HELLOIMGUI_USE_SDL_METAL)
        set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()

    # Legacy options: HELLOIMGUI_WITH_SDL and HELLOIMGUI_WITH_GLFW
    if (HELLOIMGUI_WITH_SDL)
        message(WARNING "HELLOIMGUI_WITH_SDL is deprecated, use HELLOIMGUI_USE_SDL_OPENGL3 instead")
        set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()
    if (HELLOIMGUI_WITH_GLFW)
        message(WARNING "HELLOIMGUI_WITH_GLFW is deprecated, use DHELLOIMGUI_USE_GLFW_OPENGL3 instead")
        set(HELLOIMGUI_USE_GLFW_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()

    _him_check_if_no_backend_selected(no_backend_selected)

    if (no_backend_selected)
        set(backend_message "
                HelloImGui: no backend selected!
                    In order to select your own backend, use one of the cmake options below:")
        him_get_available_backends(available_backends)
        foreach(backend ${available_backends})
            set(backend_message "${backend_message}
                        -D${backend}=ON")
        endforeach()

        message(STATUS "${backend_message}")

        _him_try_select_glfw_opengl3_if_no_backend_selected()
    endif()

    _him_check_if_no_backend_selected(no_backend_selected)
    if (no_backend_selected)
        message(FATAL_ERROR "HelloImGui: no backend selected, and could not auto-select one!")
    endif()
endfunction()

function(him_get_available_backends out_var)
    set(${out_var}
        HELLOIMGUI_USE_SDL_OPENGL3
        HELLOIMGUI_USE_GLFW_OPENGL3
        HELLOIMGUI_USE_SDL_METAL
        HELLOIMGUI_USE_GLFW_METAL
        HELLOIMGUI_USE_GLFW_VULKAN
        HELLOIMGUI_USE_SDL_VULKAN
        HELLOIMGUI_USE_SDL_DIRECTX11
        HELLOIMGUI_USE_GLFW_DIRECTX11
        HELLOIMGUI_USE_SDL_DIRECTX12

        PARENT_SCOPE
    )
endfunction()

function(_him_check_if_no_backend_selected out_var) # will set out_result to ON if no backend selected
    set(result ON)

    him_get_available_backends(available_backends)
    foreach(backend ${available_backends})
        if(${backend})
            set(result OFF)
        endif()
    endforeach()
    set(${out_var} ${result} PARENT_SCOPE)
    message(STATUS "_him_check_if_no_backend_selected return ${result}")
endfunction()

function(him_get_active_backends out_selected_backends)
    set(selected_backends "")
    him_get_available_backends(available_backends)
    foreach(backend ${available_backends})
        if (${backend})
            set(selected_backends "${selected_backends} ${backend}")
        endif()
    endforeach()
    set(${out_selected_backends} ${selected_backends} PARENT_SCOPE)
endfunction()

function(_him_try_select_glfw_opengl3_if_no_backend_selected)
    #------------------------------------------------------------------------------
    # Backend check: If no backend option is selected,
    # either select Glfw automatically if possible, or fail
    #------------------------------------------------------------------------------
    #
    if (HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED)
        set(HELLOIMGUI_USE_GLFW_OPENGL3 ON CACHE BOOL "" FORCE)
        message(STATUS "HelloImGui: using HELLOIMGUI_USE_GLFW_OPENGL3 as default default backend")
    else()
        # Check if Glfw can be found
        find_package(glfw3 QUIET)
        if (glfw3_FOUND)
            set(HELLOIMGUI_USE_GLFW_OPENGL3 ON CACHE BOOL "" FORCE)
            message(STATUS
                "HelloImGui: using HELLOIMGUI_USE_GLFW_OPENGL3 as default default backend (glfw was found via find_package(glfw3))
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
endfunction()


###################################################################################################
# Apple related options: API = him_add_apple_options
###################################################################################################
function(him_add_apple_options)
    _him_link_apple_frameworks()
    _him_add_ios_compile_definitions()
    _him_add_app_bundle_compile_definitions()
endfunction()

function(_him_link_apple_frameworks)
    if (IOS)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC "-framework UIKit")
    endif ()
    if (APPLE AND NOT IOS) # If mac
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC "-framework AppKit -framework IOKit")
    endif()
    if (APPLE)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC "-framework Foundation")
    endif()
endfunction()

function(_him_add_ios_compile_definitions)
    if (IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_IOS)
        # this is a hack because imgui_impl_opengl3.cpp does not include <TargetConditional.h>
        # and thus TARGET_OS_IOS is not found as it should be
        target_compile_definitions(${HELLOIMGUI_TARGET} PRIVATE TARGET_OS_IOS)
    endif()
endfunction()

function(_him_add_app_bundle_compile_definitions)
    if (IOS OR (MACOSX AND NOT HELLOIMGUI_MACOS_NO_BUNDLE))
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_INSIDE_APPLE_BUNDLE)
    endif()
endfunction()


###################################################################################################
# Linux related options: API = him_add_linux_options
###################################################################################################
function(him_add_linux_options)
    if (UNIX AND NOT EMSCRIPTEN  AND NOT APPLE AND NOT ANDROID)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC stdc++ dl)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC X11)
    endif()
endfunction()


###################################################################################################
# Windows related options: API = him_add_windows_options
###################################################################################################
function(him_add_windows_options)
    _him_msvc_group_sources()
    _him_win_add_auto_win_main()
endfunction()

function(_him_msvc_group_sources)
    if (MSVC)
        hello_imgui_msvc_target_group_sources(${HELLOIMGUI_TARGET})
        hello_imgui_msvc_target_set_folder(${HELLOIMGUI_TARGET} ${HELLOIMGUI_SOLUTIONFOLDER})
    endif()
endfunction()

function(_him_win_add_auto_win_main)
    if (HELLOIMGUI_WIN32_AUTO_WINMAIN)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_WIN32_AUTO_WINMAIN)
    endif()
endfunction()


###################################################################################################
# Mobile related options (i.e. iOS & Android) : API = him_add_mobile_options
###################################################################################################
function(him_add_mobile_options)
    _him_add_mobile_compile_definitions()
endfunction()

function(_him_add_mobile_compile_definitions)
    if(ANDROID OR IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_CANNOTQUIT)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_MOBILEDEVICE)
    endif()
endfunction()


###################################################################################################
# Android related options: API = him_add_android_options
###################################################################################################
function(him_add_android_options)
    if (ANDROID)
        # Empty for now (we still call add_mobile_options in main)
    endif()
endfunction()


###################################################################################################
# Emscripten related options: API = him_add_emscripten_options
###################################################################################################
function(him_add_emscripten_options)
    if (EMSCRIPTEN)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLES3)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_CANNOTQUIT)
    endif()
endfunction()


###################################################################################################
# OpenGL Rendering backend: API = him_has_opengl3_backend
###################################################################################################
function(him_has_opengl3 target)
    target_sources(${target} PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.h
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_OPENGL)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_OPENGL3)
    set(HELLOIMGUI_HAS_OPENGL ON CACHE BOOL "" FORCE)

    if(ANDROID OR IOS)
        _him_link_opengl_es_sdl(${HELLOIMGUI_TARGET})
    endif()

    if (IOS OR EMSCRIPTEN)
        set(HELLOIMGUI_USE_GLAD OFF CACHE INTERNAL "" FORCE)
    else()
        set(HELLOIMGUI_USE_GLAD ON CACHE INTERNAL "" FORCE)
    endif()

    if (HELLOIMGUI_USE_GLAD)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLAD IMGUI_IMPL_OPENGL_LOADER_GLAD)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC glad)
        him_install_glad()
    endif()
endfunction()

function(_him_link_opengl_es_sdl target)
    if(IOS)
        target_link_libraries(${target} PUBLIC "-framework OpenGLES")
    elseif(ANDROID)
        target_link_libraries(${target} PUBLIC GLESv3)
    endif()

    target_compile_definitions(${target}
        PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_CUSTOM="<OpenGLES/ES3/gl.h>"
        IMGUI_IMPL_OPENGL_ES3
        HELLOIMGUI_USE_GLES3
    )
endfunction()


###################################################################################################
# Metal Rendering backend: API = him_has_metal_backend
###################################################################################################
function(him_has_metal target)
    target_sources(${target} PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_metal.h
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_metal.mm
    )
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_METAL)
    set(HELLOIMGUI_HAS_METAL ON CACHE BOOL "" FORCE)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC
        "-framework Metal -framework MetalKit -framework QuartzCore")
endfunction()


###################################################################################################
# Vulkan Rendering backend: API = him_has_vulkan
###################################################################################################
function(him_has_vulkan target)
    target_sources(${target} PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.h
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    )
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_VULKAN)
    set(HELLOIMGUI_HAS_VULKAN ON CACHE BOOL "" FORCE)
    find_package(Vulkan REQUIRED)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC Vulkan::Vulkan)
endfunction()


###################################################################################################
# DirectX11 Rendering backend: API = him_has_directx11
###################################################################################################
function(him_has_directx11 target)
    target_sources(${target} PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx11.h
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
    )
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_DIRECTX11)
    set(HELLOIMGUI_HAS_DIRECTX11 ON CACHE BOOL "" FORCE)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC d3d11.lib)
endfunction()


###################################################################################################
# DirectX12 Rendering backend: API = him_has_directx12
###################################################################################################
function(him_has_directx12 target)
    target_sources(${target} PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx12.h
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
    )
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_DIRECTX12)
    set(HELLOIMGUI_HAS_DIRECTX12 ON CACHE BOOL "" FORCE)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC d3d12.lib dxgi.lib)
endfunction()


###################################################################################################
# Check only one rendering backend is selected: API = him_check_only_one_backend_selected
###################################################################################################
function(him_check_only_one_rendering_backend_selected)
    # Only one of HELLOIMGUI_HAS_OPENGL, HELLOIMGUI_HAS_METAL, HELLOIMGUI_HAS_VULKAN can be ON
    # count selected rendering backends
    set(selected_backends 0)
    if (HELLOIMGUI_HAS_OPENGL)
        math(EXPR selected_backends "${selected_backends} + 1")
    endif()
    if (HELLOIMGUI_HAS_METAL)
        math(EXPR selected_backends "${selected_backends} + 1")
    endif()
    if (HELLOIMGUI_HAS_VULKAN)
        math(EXPR selected_backends "${selected_backends} + 1")
    endif()
    if (HELLOIMGUI_HAS_DIRECTX11)
        math(EXPR selected_backends "${selected_backends} + 1")
    endif()
    if (HELLOIMGUI_HAS_DIRECTX12)
        math(EXPR selected_backends "${selected_backends} + 1")
    endif()
    # selected_backends should be 1
    if (selected_backends EQUAL 0)
        message(FATAL_ERROR "HelloImGui: no rendering backend selected")
    endif()
    if (NOT selected_backends EQUAL 1)
        message(FATAL_ERROR "HelloImGui: only one of HELLOIMGUI_HAS_OPENGL, HELLOIMGUI_HAS_METAL, HELLOIMGUI_HAS_VULKAN, HELLOIMGUI_HAS_DIRECTX12 can be ON")
    endif()
endfunction()



###################################################################################################
# SDL platform backend: API = him_use_sdl2_backend
###################################################################################################
function (him_use_sdl2_backend target)
    _him_fetch_sdl_if_needed()
    #    _him_fail_if_sdl_not_found()
    _him_link_sdl(${HELLOIMGUI_TARGET})

    target_sources(${target} PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.h
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    )
    set(HELLOIMGUI_USE_SDL2 ON CACHE INTERNAL "" FORCE)
    set(HELLOIMGUI_USE_SDL ON CACHE INTERNAL "" FORCE)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL2)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC _THREAD_SAFE) # flag outputted by sdl2-config --cflags
endfunction()

function(_him_fetch_sdl_if_needed)
    set(shall_fetch_sdl OFF)

    # Always fetch SDL for iOS and Android
    if (HELLOIMGUI_USE_SDL_OPENGL3 AND (IOS OR ANDROID))
        set(shall_fetch_sdl ON)
    endif()

    # Fetch SDL if:
    # option HELLOIMGUI_DOWNLOAD_SDL_IF_NEEDED was passed
    # and SDL not available by find_package
    # and HELLOIMGUI_USE_SDL_SYSTEM_LIB is OFF
    if (HELLOIMGUI_DOWNLOAD_SDL_IF_NEEDED AND NOT TARGET sdl AND NOT EMSCRIPTEN)
        find_package(SDL2 QUIET)
        if (NOT SDL2_FOUND)
            set(shall_fetch_sdl ON)
        endif()
    endif()

    if (shall_fetch_sdl)
        _him_fetch_declare_sdl()
        if(ANDROID)
            FetchContent_Populate(sdl)
            _him_prepare_android_sdl_symlink()
        else()
            FetchContent_MakeAvailable(sdl)
        endif()
    else()
        set(HELLOIMGUI_SDL_SELECTED_INFO "Use system Library" CACHE INTERNAL "" FORCE)
    endif()
endfunction()

function(_him_fetch_declare_sdl)
    # iOS and Android were tested with SDL 2.28.5
    # other platforms, not yet
    if (IOS OR ANDROID)
        set(sdl_version 2.28.5)
    else()
        set(sdl_version 2.24.2)
    endif()

    message(STATUS "Fetching SDL version ${sdl_version}")
    include(FetchContent)
    # Set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(sdl
        GIT_REPOSITORY    https://github.com/libsdl-org/SDL.git
        GIT_TAG           release-${sdl_version}
        GIT_PROGRESS TRUE
    )
    set(HELLOIMGUI_SDL_SELECTED_INFO "Downloaded ${sdl_version}" CACHE INTERNAL "" FORCE)
endfunction()

function(_him_prepare_android_sdl_symlink)
    # We now have SDL in _deps/sdl-src
    set(sdl_location ${CMAKE_BINARY_DIR}/_deps/sdl-src)
    # We need to communicate this location to the function apkCMake_makeSymLinks()
    # so that it can create the symlinks in the right place:
    # it will use the variable apkCMake_sdl_symlink_target
    set(apkCMake_sdl_symlink_target ${CMAKE_BINARY_DIR}/_deps/sdl-src CACHE STRING "" FORCE)
endfunction()

function(_him_link_sdl target)
    if (need_fetch_make_available_sdl)
        FetchContent_MakeAvailable(sdl)
    endif()

    if(IOS)
        target_link_libraries(${target} PUBLIC SDL2-static SDL2main)
    elseif(EMSCRIPTEN)
        target_compile_options(${target} PUBLIC -s USE_SDL=2)
        target_link_options(${target} INTERFACE -s USE_SDL=2)
    elseif(ANDROID)
        target_link_libraries(${target} PUBLIC SDL2main SDL2)
    elseif(TARGET SDL2-static)
        target_link_libraries(${target} PUBLIC SDL2-static)
    else()
        find_package(SDL2 REQUIRED)
        target_link_libraries(${target} PUBLIC SDL2::SDL2 SDL2::SDL2main)
    endif()

    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        target_link_libraries(${target} PUBLIC Xext X11)
    endif()
endfunction()


###################################################################################################
# Glfw platform backend: API = him_use_glfw_backend
###################################################################################################
function(him_use_glfw_backend target)
    _him_fetch_glfw_if_needed()
    if (NOT TARGET glfw) # if glfw is not built as part of the whole build, find it
        find_package(glfw3 CONFIG REQUIRED)
    endif()
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC glfw)

    target_sources(${target} PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.h
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    )
    set(HELLOIMGUI_USE_GLFW ON CACHE INTERNAL "" FORCE)
    set(HELLOIMGUI_USE_GLFW3 ON CACHE INTERNAL "" FORCE)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW3)
endfunction()

function(_him_fetch_glfw_if_needed)
    set(shall_fetch_glfw OFF)
    if (HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED AND NOT TARGET glfw)
        find_package(glfw3 QUIET)
        if (NOT glfw3_FOUND)
            set(shall_fetch_glfw ON)
        endif()
    endif()

    if (shall_fetch_glfw)
        message(STATUS "HelloImGui: downloading and building glfw")
        include(FetchContent)
        # Set(FETCHCONTENT_QUIET FALSE)
        FetchContent_Declare(glfw
            GIT_REPOSITORY    https://github.com/glfw/glfw.git
            GIT_TAG           3.3.8
            GIT_PROGRESS TRUE
        )

        set(GLFW_BUILD_EXAMPLES OFF)
        set(GLFW_BUILD_TESTS OFF)
        set(GLFW_BUILD_DOCS OFF)
        set(GLFW_INSTALL OFF)
        FetchContent_MakeAvailable(glfw)
        set(HELLOIMGUI_GLFW_SELECTED_INFO "Downloaded 3.3.8" CACHE INTERNAL "" FORCE)
    else()
        set(HELLOIMGUI_GLFW_SELECTED_INFO "Use system Library" CACHE INTERNAL "" FORCE)
    endif()

endfunction()


###################################################################################################
# Miscellaneous: API = him_add_misc_options
###################################################################################################
function(him_add_misc_options)
    _him_add_imgui_shared_compile_definitions()
endfunction()

function(_him_add_imgui_shared_compile_definitions)
    if (HELLO_IMGUI_IMGUI_SHARED)
        target_compile_definitions(${HELLOIMGUI_TARGET} PRIVATE GImGui=GImGuiFromHelloImGui)
        target_compile_definitions(${HELLOIMGUI_TARGET} PRIVATE HELLO_IMGUI_IMGUI_SHARED)
    endif()
endfunction()


###################################################################################################
# Install: API = him_install
###################################################################################################
function(him_install)
    if (PROJECT_IS_TOP_LEVEL AND NOT IOS AND NOT ANDROID)
        install(TARGETS ${HELLOIMGUI_TARGET} DESTINATION ./lib/)
        file(GLOB headers *.h)
        install(FILES ${headers} DESTINATION ./include/hello_imgui/)
    endif()
endfunction()


###################################################################################################
# Log Configuration at the end of the configuration: API = him_log_configuration
###################################################################################################
function(him_log_configuration)
    him_get_active_backends(selected_backends)

    # set imgui_source_dir to the relative path of HELLOIMGUI_IMGUI_SOURCE_DIR versus this project
    file(RELATIVE_PATH imgui_source_dir ${HELLOIMGUI_BASEPATH} ${HELLOIMGUI_IMGUI_SOURCE_DIR})

    set(msg "
    ===========================================================================
        Hello ImGui build options:
    ===========================================================================
      Backends: ${selected_backends}
    ---------------------------------------------------------------------------
      Options:
        HELLOIMGUI_USE_FREETYPE:        ${HELLOIMGUI_USE_FREETYPE}  (${HELLOIMGUI_FREETYPE_SELECTED_INFO})
        HELLOIMGUI_WITH_TEST_ENGINE:    ${HELLOIMGUI_WITH_TEST_ENGINE}
        BUILD_DEMOS - TESTS - DOCS:     ${HELLOIMGUI_BUILD_DEMOS} - ${HELLOIMGUI_BUILD_TESTS} - ${HELLOIMGUI_BUILD_DOCS}
    ---------------------------------------------------------------------------
      ImGui:
        Build ImGui:                    ${HELLOIMGUI_BUILD_IMGUI}
        ImGui source dir:               ${imgui_source_dir}")

    if(EMSCRIPTEN)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      Emscripten options
        HELLOIMGUI_EMSCRIPTEN_PTHREAD:  ${HELLOIMGUI_EMSCRIPTEN_PTHREAD}
        HELLOIMGUI_EMSCRIPTEN_PTHREAD_ALLOW_MEMORY_GROWTH: ${HELLOIMGUI_EMSCRIPTEN_PTHREAD_ALLOW_MEMORY_GROWTH}")
    endif ()

    if(HELLOIMGUI_HAS_OPENGL)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      OpenGL - use glad loader          ${HELLOIMGUI_USE_GLAD}")
    endif()

    set(msg "${msg}
    ---------------------------------------------------------------------------
      Platform Backend(s):")
    if(HELLOIMGUI_USE_SDL)
        set(msg "${msg}
        SDL:                            ${HELLOIMGUI_SDL_SELECTED_INFO}")
    endif()
    if(HELLOIMGUI_USE_GLFW)
        set(msg "${msg}
        Glfw:                           ${HELLOIMGUI_GLFW_SELECTED_INFO}")
    endif()

    if(WIN32)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      Windows:
        HELLOIMGUI_WIN32_NO_CONSOLE:    ${HELLOIMGUI_WIN32_NO_CONSOLE}
        HELLOIMGUI_WIN32_AUTO_WINMAIN:  ${HELLOIMGUI_WIN32_AUTO_WINMAIN}")
    endif()

    if(MACOSX)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      macOS:
        HELLOIMGUI_MACOS_NO_BUNDLE:     ${HELLOIMGUI_MACOS_NO_BUNDLE}")
    endif()

    set(msg "${msg}
    ===========================================================================")

    message(STATUS "${msg}")
endfunction()

###################################################################################################
# Main: API = him_main_add_hello_imgui_library
###################################################################################################
function(him_main_add_hello_imgui_library)
    him_sanity_checks()
    him_build_imgui()
    him_add_hello_imgui()
    if (HELLOIMGUI_WITH_TEST_ENGINE)
        add_imgui_test_engine()
    endif()

    if (HELLOIMGUI_USE_SDL_OPENGL3)
        him_has_opengl3(${HELLOIMGUI_TARGET})
        him_use_sdl2_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL_OPENGL3)
    endif()

    if (HELLOIMGUI_USE_GLFW_OPENGL3)
        him_has_opengl3(${HELLOIMGUI_TARGET})
        him_use_glfw_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW_OPENGL3)
    endif()

    if(HELLOIMGUI_USE_SDL_METAL)
        him_has_metal(${HELLOIMGUI_TARGET})
        him_use_sdl2_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL_METAL)
    endif()

    if(HELLOIMGUI_USE_GLFW_METAL)
        him_has_metal(${HELLOIMGUI_TARGET})
        him_use_glfw_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW_METAL)
    endif()

    if(HELLOIMGUI_USE_GLFW_VULKAN)
        him_has_vulkan(${HELLOIMGUI_TARGET})
        him_use_glfw_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW_METAL)
    endif()

    if(HELLOIMGUI_USE_SDL_VULKAN)
        him_has_vulkan(${HELLOIMGUI_TARGET})
        him_use_sdl2_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL_VULKAN)
    endif()

    if(HELLOIMGUI_USE_SDL_DIRECTX11)
        him_has_directx11(${HELLOIMGUI_TARGET})
        him_use_sdl2_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL_DIRECTX11)
    endif ()

    if(HELLOIMGUI_USE_GLFW_DIRECTX11)
        him_has_directx11(${HELLOIMGUI_TARGET})
        him_use_glfw_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW_DIRECTX11)
    endif ()

    if(HELLOIMGUI_USE_SDL_DIRECTX12)
        him_has_directx12(${HELLOIMGUI_TARGET})
        him_use_sdl2_backend(${HELLOIMGUI_TARGET})
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL_DIRECTX12)
    endif ()

    him_check_only_one_rendering_backend_selected()
    him_add_apple_options()
    him_add_linux_options()
    him_add_windows_options()
    him_add_mobile_options()
    him_add_android_options()
    him_add_emscripten_options()
    him_add_misc_options()
    him_install()

    him_get_active_backends(selected_backends)
    message(STATUS "HelloImGui backends: ${selected_backends}")
endfunction()
