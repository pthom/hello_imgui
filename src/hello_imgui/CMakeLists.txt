set(target_name hello_imgui)
message(STATUS "Library ${target_name}")

set(imgui_backends_dir ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends)

if (APPLE)
    file(GLOB_RECURSE sources *.h *.cpp *.c *.mm)
else()
    file(GLOB_RECURSE sources *.h *.cpp *.c)
endif()
add_library(${target_name} ${sources})


target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(${target_name} PUBLIC imgui)


if (HELLO_IMGUI_IMGUI_SHARED)
    target_compile_definitions(${target_name} PRIVATE GImGui=GImGuiFromHelloImGui)
    target_compile_definitions(${target_name} PRIVATE HELLO_IMGUI_IMGUI_SHARED)
endif()

if(APPLE AND NOT IOS)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_MACOS)
    enable_language(OBJC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -x objective-c++")
endif()

if (IOS)
    target_compile_definitions(${target_name} PUBLIC IOS)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_MOBILEDEVICE)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_CANNOTQUIT)

    # this is a hack because imgui_impl_opengl3.cpp does not include <TargetConditional.h>
    # and thus TARGET_OS_IOS is not found as it should be
    target_compile_definitions(${target_name} PRIVATE TARGET_OS_IOS)
endif()

if (EMSCRIPTEN)
    set(HELLOIMGUI_USE_SDL_OPENGL3 ON)
    set(HELLOIMGUI_USE_GLFW_OPENGL3 OFF)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLES3)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_CANNOTQUIT)
endif()


function(link_sdl target)
    if (need_fetch_make_available_sdl)
        FetchContent_MakeAvailable(sdl)
    endif()

    if(IOS)
        target_link_libraries(${target} PUBLIC SDL2-static SDL2main)
        target_link_libraries(${target} PUBLIC "-framework OpenGLES")
    elseif(EMSCRIPTEN)
        target_compile_options(${target} PUBLIC -s USE_SDL=2)
        target_link_options(${target} INTERFACE -s USE_SDL=2)
    elseif(ANDROID)
        target_link_libraries(${target} PUBLIC SDL2main SDL2)
        target_link_libraries(${target_name} PUBLIC GLESv3)
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


if (HELLOIMGUI_USE_SDL_OPENGL3)
    link_sdl(${target_name})

    if (IOS)
        target_compile_definitions(${target_name}
            PUBLIC
            IMGUI_IMPL_OPENGL_LOADER_CUSTOM="<OpenGLES/ES3/gl.h>"
            IMGUI_IMPL_OPENGL_ES3
            HELLOIMGUI_USE_GLES3
            )
    elseif(ANDROID)
        target_compile_definitions(${target_name} PUBLIC
            IMGUI_IMPL_OPENGL_LOADER_CUSTOM="<OpenGLES/ES3/gl.h>"
            IMGUI_IMPL_OPENGL_ES3
            HELLOIMGUI_USE_GLES3
            )
        target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_CANNOTQUIT)
        target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_MOBILEDEVICE)
    endif()

    target_sources(${target_name} PRIVATE
        ${imgui_backends_dir}/imgui_impl_opengl3.h
        ${imgui_backends_dir}/imgui_impl_opengl3.cpp
        ${imgui_backends_dir}/imgui_impl_sdl2.h
        ${imgui_backends_dir}/imgui_impl_sdl2.cpp
        )
endif()

if (HELLOIMGUI_USE_GLFW_OPENGL3)
    if (need_fetch_make_available_glfw)
        set(GLFW_BUILD_EXAMPLES OFF)
        set(GLFW_BUILD_TESTS OFF)
        set(GLFW_BUILD_DOCS OFF)
        set(GLFW_INSTALL OFF)
        FetchContent_MakeAvailable(glfw)
    endif()
    if (NOT TARGET glfw) # if glfw is not built as part of the whole build, find it
        find_package(glfw3 CONFIG REQUIRED)
    endif()
    target_link_libraries(${target_name} PUBLIC glfw)

    target_sources(${target_name} PRIVATE
        ${imgui_backends_dir}/imgui_impl_opengl3.h
        ${imgui_backends_dir}/imgui_impl_opengl3.cpp
        ${imgui_backends_dir}/imgui_impl_glfw.h
        ${imgui_backends_dir}/imgui_impl_glfw.cpp
        )
endif()

if (APPLE AND NOT IOS) # If mac
    target_link_libraries(${target_name} PUBLIC "-framework AppKit -framework IOKit")
endif()

if (HELLOIMGUI_USE_QT)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_QT)
    target_link_libraries(${target_name} PUBLIC qt_imgui_quick)
endif()

if (HELLOIMGUI_USE_GLAD)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLAD IMGUI_IMPL_OPENGL_LOADER_GLAD)
    target_link_libraries(${target_name} PUBLIC glad)
endif()

if (MSVC)
    hello_imgui_msvc_target_group_sources(${target_name})
    hello_imgui_msvc_target_set_folder(${target_name} ${HELLOIMGUI_SOLUTIONFOLDER})
endif()


# OS specific link options
if (UNIX AND NOT EMSCRIPTEN)
    target_link_libraries(hello_imgui PUBLIC stdc++ dl)
endif()
if (UNIX AND NOT EMSCRIPTEN AND NOT APPLE)
    target_link_libraries(hello_imgui PUBLIC X11)
endif()
if (APPLE)
    target_link_libraries(${target_name} PUBLIC "-framework Foundation")
endif()


# Compile definitions
if (HELLOIMGUI_USE_SDL_OPENGL3)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_SDL_OPENGL3)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_SDL)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_HAS_OPENGL)
endif()
if (HELLOIMGUI_USE_GLFW_OPENGL3)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLFW_OPENGL3)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLFW)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_HAS_OPENGL)
endif()


# install
if (PROJECT_IS_TOP_LEVEL AND NOT IOS AND NOT ANDROID)
    install(TARGETS ${target_name} DESTINATION ./lib/)
    file(GLOB headers *.h)
    install(FILES ${headers} DESTINATION ./include/hello_imgui/)
endif()
