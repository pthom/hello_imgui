set(helloimgui_target hello_imgui)
message(STATUS "Library ${helloimgui_target}")
set(imgui_backends_dir ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends)


###################################################################################################
# Add library and sources: API = add_hello_imgui_target
# (enable objective-c++ for mac)
###################################################################################################
function(add_hello_imgui_target)
    if (APPLE)
        file(GLOB_RECURSE sources *.h *.cpp *.c *.mm)
    else()
        file(GLOB_RECURSE sources *.h *.cpp *.c)
    endif()
    add_library(${helloimgui_target} ${sources})
    if(APPLE AND NOT IOS)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_MACOS)
        enable_language(OBJC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++")
    endif()
    target_include_directories(${helloimgui_target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(${helloimgui_target} PUBLIC imgui)
endfunction()


###################################################################################################
# Link Os dependent frameworks: API = link_os_dependent_libs
###################################################################################################
function(link_os_dependent_libs)
    if (IOS)
        target_link_libraries(${helloimgui_target} PUBLIC "-framework UIKit")
    endif ()
    if (APPLE AND NOT IOS) # If mac
        target_link_libraries(${helloimgui_target} PUBLIC "-framework AppKit -framework IOKit")
    endif()
    if (APPLE)
        target_link_libraries(${helloimgui_target} PUBLIC "-framework Foundation")
    endif()

    # OS specific link options
    if (UNIX AND NOT EMSCRIPTEN)
        target_link_libraries(${helloimgui_target} PUBLIC stdc++ dl)
    endif()
    if (UNIX AND NOT EMSCRIPTEN AND NOT APPLE AND NOT ANDROID)
        target_link_libraries(${helloimgui_target} PUBLIC X11)
    endif()
endfunction()



###################################################################################################
# OpenGL backend: API = use_opengl3_backend
###################################################################################################
function(use_opengl3_backend target)
    target_sources(${target} PRIVATE
        ${imgui_backends_dir}/imgui_impl_opengl3.h
        ${imgui_backends_dir}/imgui_impl_opengl3.cpp
    )
    target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_HAS_OPENGL)

    if(ANDROID OR IOS)
        _link_opengles_sdl(${helloimgui_target})
    endif()
    if (HELLOIMGUI_USE_GLAD)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_GLAD IMGUI_IMPL_OPENGL_LOADER_GLAD)
        target_link_libraries(${helloimgui_target} PUBLIC glad)
    endif()
endfunction()

function(_link_opengles_sdl target)
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
# SDL backend: API = use_sdl2_backend
###################################################################################################
function (use_sdl2_backend target)
    target_sources(${target} PRIVATE
        ${imgui_backends_dir}/imgui_impl_sdl2.h
        ${imgui_backends_dir}/imgui_impl_sdl2.cpp
    )
    _link_sdl(${helloimgui_target})
    target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_SDL)
endfunction()

function(_link_sdl target)
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
# Glfw backend: API = use_glfw_backend
###################################################################################################
function(use_glfw_backend target)
    if (need_fetch_make_available_glfw)
        _fetch_make_available_glfw()
    endif()
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

function(_fetch_make_available_glfw)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_INSTALL OFF)
    FetchContent_MakeAvailable(glfw)
endfunction()


###################################################################################################
# Compile definitions: API = himgui_add_compile_definitions
###################################################################################################
function(himgui_add_compile_definitions)
    _add_mobile_compile_definitions()
    _add_ios_compile_definitions()
    _add_apple_compile_definitions()
    add_imgui_shared_compile_definitions()
endfunction()

function(_add_mobile_compile_definitions)
    if(ANDROID OR IOS)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_CANNOTQUIT)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_MOBILEDEVICE)
    endif()
endfunction()

function(_add_ios_compile_definitions)
    if (IOS)
        target_compile_definitions(${helloimgui_target} PUBLIC IOS)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_IOS)
        # this is a hack because imgui_impl_opengl3.cpp does not include <TargetConditional.h>
        # and thus TARGET_OS_IOS is not found as it should be
        target_compile_definitions(${helloimgui_target} PRIVATE TARGET_OS_IOS)
    endif()
endfunction()

function(_add_apple_compile_definitions)
    if (IOS OR (MACOSX AND NOT HELLOIMGUI_MACOS_NO_BUNDLE))
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_INSIDE_APPLE_BUNDLE)
    endif()
endfunction()

function(add_imgui_shared_compile_definitions)
    # Compile definitions
    if (HELLO_IMGUI_IMGUI_SHARED)
        target_compile_definitions(${helloimgui_target} PRIVATE GImGui=GImGuiFromHelloImGui)
        target_compile_definitions(${helloimgui_target} PRIVATE HELLO_IMGUI_IMGUI_SHARED)
    endif()
endfunction()

    
###################################################################################################
# Main: API = himgui_main
###################################################################################################
function(himgui_main)
    add_hello_imgui_target()
    link_os_dependent_libs()

    if (EMSCRIPTEN)
        set(HELLOIMGUI_USE_SDL_OPENGL3 ON PARENT_SCOPE)
        set(HELLOIMGUI_USE_GLFW_OPENGL3 OFF PARENT_SCOPE)
    endif()


    himgui_add_compile_definitions()

    if (HELLOIMGUI_USE_SDL_OPENGL3)
        use_opengl3_backend(${helloimgui_target})
        use_sdl2_backend(${helloimgui_target})
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_SDL_OPENGL3)
    endif()

    if (HELLOIMGUI_USE_GLFW_OPENGL3)
        use_opengl3_backend(${helloimgui_target})
        use_glfw_backend(${helloimgui_target})
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_GLFW_OPENGL3)
    endif()

    if (MSVC)
        hello_imgui_msvc_target_group_sources(${helloimgui_target})
        hello_imgui_msvc_target_set_folder(${helloimgui_target} ${HELLOIMGUI_SOLUTIONFOLDER})
    endif()



    if (EMSCRIPTEN)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_USE_GLES3)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_CANNOTQUIT)
    endif()


    if (HELLOIMGUI_WIN32_AUTO_WINMAIN)
        target_compile_definitions(${helloimgui_target} PUBLIC HELLOIMGUI_WIN32_AUTO_WINMAIN)
    endif()

endfunction()


###################################################################################################
# Install: API = himgui_install
###################################################################################################
function(himgui_install)
if (PROJECT_IS_TOP_LEVEL AND NOT IOS AND NOT ANDROID)
    install(TARGETS ${helloimgui_target} DESTINATION ./lib/)
    file(GLOB headers *.h)
    install(FILES ${headers} DESTINATION ./include/hello_imgui/)
endif()
endfunction()


###################################################################################################
# Call main and install
# This is the only part where we are allow to make function calls
# *above we only define functions!*
###################################################################################################
himgui_main()
himgui_install()
