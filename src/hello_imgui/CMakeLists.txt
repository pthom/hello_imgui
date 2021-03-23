set(target_name hello_imgui)
message(STATUS "Library ${target_name}")

set(imgui_backends_dir ${CMAKE_CURRENT_LIST_DIR}/../../external/imgui/backends)

if (APPLE)
    file(GLOB_RECURSE sources *.h *.cpp *.c *.mm)
else()
    file(GLOB_RECURSE sources *.h *.cpp *.c)
endif()
add_library(${target_name} ${sources})

target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(${target_name} PUBLIC imgui)


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
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLES3)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_CANNOTQUIT)
endif()

if (HELLOIMGUI_USE_SDL_OPENGL3 OR HELLOIMGUI_USE_GLFW_OPENGL3)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_HAS_OPENGL)
endif()

if (HELLOIMGUI_USE_SDL_OPENGL3)
    target_compile_definitions(${target_name} PUBLIC -DHELLOIMGUI_USE_SDL_OPENGL3)

    if (IOS)
        target_include_directories(${target_name} PUBLIC "${HELLOIMGUI_BASEPATH}/external/SDL/include/")

        string(REGEX MATCH "SIMULATOR.*" is_simulator ${PLATFORM})
        if (is_simulator)
            set(sdl_lib_dir "${HELLOIMGUI_BASEPATH}/external/SDL/Xcode-iOS/SDL/build/Release-iphonesimulator")
        else()
            set(sdl_lib_dir "${HELLOIMGUI_BASEPATH}/external/SDL/Xcode-iOS/SDL/build/Release-iphoneos")
        endif()

        target_link_libraries(${target_name} PUBLIC ${sdl_lib_dir}/libSDL2.a)
        target_link_libraries(${target_name} PUBLIC
            "-framework Foundation"
            "-framework AVFoundation"
            "-framework AudioToolbox"
            "-framework CoreBluetooth"
            "-framework CoreGraphics"
            "-framework CoreMedia"
            "-framework CoreMotion"
            "-framework CoreVideo"
            "-framework GameController"
            "-framework Metal"
            "-framework OpenGLES"
            "-framework UIKit"
            "-framework QuartzCore"
            )

        target_compile_definitions(${target_name}
            PUBLIC
            IMGUI_IMPL_OPENGL_LOADER_CUSTOM="<OpenGLES/ES3/gl.h>"
            IMGUI_IMPL_OPENGL_ES3
            HELLOIMGUI_USE_GLES3
            )
    elseif(EMSCRIPTEN)
        target_compile_options(${target_name} PUBLIC -s USE_SDL=2)
        target_link_options(${target_name} INTERFACE -s USE_SDL=2)
    elseif(ANDROID)
        target_link_libraries(${target_name} PUBLIC SDL2main SDL2)
        target_link_libraries(${target_name} PUBLIC GLESv3)
        target_compile_definitions(${target_name} PUBLIC
            IMGUI_IMPL_OPENGL_LOADER_CUSTOM="<OpenGLES/ES3/gl.h>"
            IMGUI_IMPL_OPENGL_ES3
            HELLOIMGUI_USE_GLES3
            )
        target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_CANNOTQUIT)
        target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_MOBILEDEVICE)
    else()
        if (NOT TARGET SDL2-static) # if SDL is not built as part of the whole build, find it
            find_package(SDL2 CONFIG REQUIRED)
            target_link_libraries(${target_name} PUBLIC SDL2::SDL2 SDL2::SDL2main)
        else()
            target_link_libraries(${target_name} PUBLIC SDL2-static)
        endif()
    endif()

    target_sources(${target_name} PRIVATE
        ${imgui_backends_dir}/imgui_impl_opengl3.h
        ${imgui_backends_dir}/imgui_impl_opengl3.cpp
        ${imgui_backends_dir}/imgui_impl_sdl.h
        ${imgui_backends_dir}/imgui_impl_sdl.cpp
        )
endif()

if (HELLOIMGUI_USE_GLFW_OPENGL3)
    if (NOT TARGET glfw) # if glfw is not built as part of the whole build, find it
        find_package(glfw3 CONFIG REQUIRED)
    endif()
    target_compile_definitions(${target_name} PUBLIC -DHELLOIMGUI_USE_GLFW_OPENGL3)
    target_link_libraries(${target_name} PUBLIC glfw)
    target_sources(${target_name} PRIVATE
        ${imgui_backends_dir}/imgui_impl_opengl3.h
        ${imgui_backends_dir}/imgui_impl_opengl3.cpp
        ${imgui_backends_dir}/imgui_impl_glfw.h
        ${imgui_backends_dir}/imgui_impl_glfw.cpp
        )
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

if (UNIX AND NOT EMSCRIPTEN)
    target_link_libraries(hello_imgui PUBLIC stdc++ dl)
endif()
