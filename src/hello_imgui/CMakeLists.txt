###################################################################################################
# Global variables and + objective-c++ for apple
###################################################################################################
include(FetchContent)
include(${HELLOIMGUI_BASEPATH}/external/OpenGL_Loaders/glad.cmake)
include(${HELLOIMGUI_BASEPATH}/src/hello_imgui_test_engine_integration/hello_imgui_test_engine_cmake.cmake)


# Set target name
set(helloimgui_target hello_imgui)
message(STATUS "Library ${helloimgui_target}")


# Enable objective-c++ for apple. This is global and cannot (yet) be done in a function.
if (APPLE)
    enable_language(OBJC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++")
endif()

# Global variable for this file
set(imgui_backends_dir ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends)


###################################################################################################
# Add library and sources: API = him_add_hello_imgui
###################################################################################################
function(him_add_hello_imgui)
    if (APPLE)
        file(GLOB_RECURSE sources *.h *.cpp *.c *.mm)
    else()
        file(GLOB_RECURSE sources *.h *.cpp *.c)
    endif()
    add_library(${helloimgui_target} ${sources})
    if(APPLE AND NOT IOS)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_MACOS)
    endif()
    target_include_directories(${helloimgui_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(${helloimgui_target} PUBLIC imgui)
endfunction()


###################################################################################################
# Build imgui: API = him_build_imgui
###################################################################################################
function(him_build_imgui)
    _him_checkout_imgui_submodule_if_needed()
    _him_do_build_imgui()
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
    if (HELLOIMGUI_BUILD_IMGUI)
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

        # Install
        if(PROJECT_IS_TOP_LEVEL)
            install(FILES ${imgui_sources} DESTINATION imgui)
            install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends DESTINATION imgui)
            install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp DESTINATION imgui/misc)
            install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype DESTINATION imgui/misc)
        endif()
    endif()
endfunction()



###################################################################################################
# Sanity checks: API = him_sanity_checks
###################################################################################################
function(him_sanity_checks)
    _him_try_select_glfw_if_no_backend_selected()

    if (HELLOIMGUI_WITH_GLFW)
        set(HELLOIMGUI_USE_GLFW_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()
    if (HELLOIMGUI_WITH_SDL)
        set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()

    _him_shout_if_no_backend()
endfunction()

function(_him_try_select_glfw_if_no_backend_selected)
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
            set(HELLOIMGUI_WITH_GLFW ON CACHE BOOL "" FORCE)
            message(STATUS "
                HelloImGui: using Glfw as default default backend (it will be downloaded and built inside {CMAKE_CURRENT_BINARY_DIR}/_deps/glfw-*)
                ${backend_message}
            ")
        else()
            # Check if Glfw can be found
            find_package(glfw3 QUIET)
            if (glfw3_FOUND)
                set(HELLOIMGUI_USE_GLFW_OPENGL3 ON CACHE BOOL "" FORCE)
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
endfunction()

function(_him_shout_if_no_backend)
    if (NOT (HELLOIMGUI_USE_SDL_OPENGL3 OR HELLOIMGUI_USE_GLFW_OPENGL3 OR HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT))
        message(FATAL_ERROR "Select at least one backend: use either
        -DHELLOIMGUI_USE_SDL_OPENGL3=ON
        -DHELLOIMGUI_USE_GLFW_OPENGL3=ON
        -DHELLOIMGUI_USE_QT=ON
        -DHELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT=ON
    ")
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
        target_link_libraries(${helloimgui_target} PUBLIC "-framework UIKit")
    endif ()
    if (APPLE AND NOT IOS) # If mac
        target_link_libraries(${helloimgui_target} PUBLIC "-framework AppKit -framework IOKit")
    endif()
    if (APPLE)
        target_link_libraries(${helloimgui_target} PUBLIC "-framework Foundation")
    endif()
endfunction()

function(_him_add_ios_compile_definitions)
    if (IOS)
        target_compile_definitions(${helloimgui_target} PUBLIC IOS)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_IOS)
        # this is a hack because imgui_impl_opengl3.cpp does not include <TargetConditional.h>
        # and thus TARGET_OS_IOS is not found as it should be
        target_compile_definitions(${helloimgui_target} PRIVATE TARGET_OS_IOS)
    endif()
endfunction()

function(_him_add_app_bundle_compile_definitions)
    if (IOS OR (MACOSX AND NOT HELLOIMGUI_MACOS_NO_BUNDLE))
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_INSIDE_APPLE_BUNDLE)
    endif()
endfunction()


###################################################################################################
# Linux related options: API = him_add_linux_options
###################################################################################################
function(him_add_linux_options)
    if (UNIX AND NOT EMSCRIPTEN  AND NOT APPLE AND NOT ANDROID)
        target_link_libraries(${helloimgui_target} PUBLIC stdc++ dl)
        target_link_libraries(${helloimgui_target} PUBLIC X11)
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
        hello_imgui_msvc_target_group_sources(${helloimgui_target})
        hello_imgui_msvc_target_set_folder(${helloimgui_target} ${HELLOIMGUI_SOLUTIONFOLDER})
    endif()
endfunction()

function(_him_win_add_auto_win_main)
    if (HELLOIMGUI_WIN32_AUTO_WINMAIN)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_WIN32_AUTO_WINMAIN)
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
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_CANNOTQUIT)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_MOBILEDEVICE)
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
    #------------------------------------------------------------------------------
    # use SDL for emscripten
    #------------------------------------------------------------------------------
    if (EMSCRIPTEN AND NOT HELLOIMGUI_USE_SDL_OPENGL3 AND NOT HELLOIMGUI_USE_GLFW_OPENGL3)
        set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_USE_GLFW_OPENGL3 OFF CACHE BOOL "" FORCE)
    endif()

    if (EMSCRIPTEN)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_GLES3)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_CANNOTQUIT)
    endif()

endfunction()


###################################################################################################
# OpenGL backend: API = him_use_opengl3_backend
###################################################################################################
function(him_use_opengl3_backend target)
    target_sources(${target} PRIVATE
        ${imgui_backends_dir}/imgui_impl_opengl3.h
        ${imgui_backends_dir}/imgui_impl_opengl3.cpp
    )
    target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_HAS_OPENGL)

    if(ANDROID OR IOS)
        _him_link_opengles_sdl(${helloimgui_target})
    endif()
    if (HELLOIMGUI_USE_GLAD)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_GLAD IMGUI_IMPL_OPENGL_LOADER_GLAD)
        target_link_libraries(${helloimgui_target} PUBLIC glad)
    endif()
    if (HELLOIMGUI_USE_GLAD)
        him_install_glad()
    endif()
endfunction()

function(_him_link_opengles_sdl target)
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
# SDL backend: API = him_use_sdl2_backend
###################################################################################################
function (him_use_sdl2_backend target)
    _him_fetch_sdl_if_needed()
    _him_link_sdl(${helloimgui_target})

    target_sources(${target} PRIVATE
        ${imgui_backends_dir}/imgui_impl_sdl2.h
        ${imgui_backends_dir}/imgui_impl_sdl2.cpp
    )
    target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_SDL)
endfunction()


function(_him_fetch_sdl_if_needed)
    set(shall_fetch_sdl OFF)

    # Always fetch SDL for iOS and Android
    if (HELLOIMGUI_USE_SDL_OPENGL3 AND (IOS OR ANDROID))
        set(shall_fetch_sdl ON)
    endif()

    # Fetch SDL if:
    # option HELLOIMGUI_WITH_SDL was passed
    # and SDL not available by find_package
    # and HELLOIMGUI_USE_SDL_SYSTEM_LIB is OFF
    if (HELLOIMGUI_WITH_SDL AND NOT HELLOIMGUI_USE_SDL_SYSTEM_LIB
        AND NOT TARGET sdl AND NOT EMSCRIPTEN)
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
    endif()
endfunction()

function(_him_prepare_android_sdl_symlink)
    # We now have SDL in _deps/sdl-src
    set(sdl_location ${CMAKE_BINARY_DIR}/_deps/sdl-src)
    # We need to communicate this location to the function apkCMake_makeSymLinks()
    # so that it can create the symlinks in the right place:
    # it will use the variable apkCMake_sdl_symlink_target
    set(apkCMake_sdl_symlink_target ${CMAKE_BINARY_DIR}/_deps/sdl-src CACHE STRING "" FORCE)
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
    Set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(sdl
        GIT_REPOSITORY    https://github.com/libsdl-org/SDL.git
        GIT_TAG           release-${sdl_version}
        GIT_PROGRESS TRUE
    )
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
        find_package(SDL2)
        if (SDL2_FOUND AND (TARGET SDL2::SDL2))
            target_link_libraries(${target} PUBLIC SDL2::SDL2 SDL2::SDL2main)
        else()
            if (NOT DEFINED SDL2_INCLUDE_DIRS)
                set(SDL2_INCLUDE_DIRS /usr/include/SDL2)
            endif()
            target_include_directories(${target}  PUBLIC ${SDL2_INCLUDE_DIRS})
            if (DEFINED SDL2_LIBDIR)
                target_link_directories(${target} PUBLIC ${SDL2_LIBDIR})
            endif()
            target_link_libraries(${target} PUBLIC SDL2 SDL2main)
        endif()
    endif()

    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        target_link_libraries(${target} PUBLIC Xext X11)
    endif()
endfunction()


###################################################################################################
# Glfw backend: API = him_use_glfw_backend
###################################################################################################
function(him_use_glfw_backend target)
    _him_fetch_glfw_if_needed()
    if (NOT TARGET glfw) # if glfw is not built as part of the whole build, find it
        find_package(glfw3 CONFIG REQUIRED)
    endif()
    target_link_libraries(${helloimgui_target} PUBLIC glfw)

    target_sources(${target} PRIVATE
        ${imgui_backends_dir}/imgui_impl_glfw.h
        ${imgui_backends_dir}/imgui_impl_glfw.cpp
    )
    target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_GLFW)
endfunction()

function(_him_fetch_glfw_if_needed)
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

    if (shall_fetch_glfw)
        message(STATUS "HelloImGui: downloading and building glfw")
        include(FetchContent)
        Set(FETCHCONTENT_QUIET FALSE)
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
    endif()

endfunction()


###################################################################################################
# Miscellanous: API = him_add_misc_options
###################################################################################################
function(him_add_misc_options)
    _him_add_imgui_shared_compile_definitions()
endfunction()

function(_him_add_imgui_shared_compile_definitions)
    if (HELLO_IMGUI_IMGUI_SHARED)
        target_compile_definitions(${helloimgui_target} PRIVATE GImGui=GImGuiFromHelloImGui)
        target_compile_definitions(${helloimgui_target} PRIVATE HELLO_IMGUI_IMGUI_SHARED)
    endif()
endfunction()


###################################################################################################
# Install: API = him_install
###################################################################################################
function(him_install)
    if (PROJECT_IS_TOP_LEVEL AND NOT IOS AND NOT ANDROID)
        install(TARGETS ${helloimgui_target} DESTINATION ./lib/)
        file(GLOB headers *.h)
        install(FILES ${headers} DESTINATION ./include/hello_imgui/)
    endif()
endfunction()

    
###################################################################################################
# Main: API = him_main
###################################################################################################
function(him_main)
    him_sanity_checks()
    him_build_imgui()
    him_add_hello_imgui()
    if (HELLOIMGUI_WITH_TEST_ENGINE)
        add_imgui_test_engine()
    endif()

    if (HELLOIMGUI_USE_SDL_OPENGL3)
        him_use_opengl3_backend(${helloimgui_target})
        him_use_sdl2_backend(${helloimgui_target})
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_SDL_OPENGL3)
    endif()

    if (HELLOIMGUI_USE_GLFW_OPENGL3)
        him_use_opengl3_backend(${helloimgui_target})
        him_use_glfw_backend(${helloimgui_target})
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_GLFW_OPENGL3)
    endif()

    him_add_apple_options()
    him_add_linux_options()
    him_add_windows_options()
    him_add_mobile_options()
    him_add_android_options()
    him_add_emscripten_options()
    him_add_misc_options()
    him_install()
endfunction()


###################################################################################################
# Call him_main()
# This is the only part where we are allow to make function calls
# *above we only define functions (and sadly some global variables)*
###################################################################################################
him_main()
