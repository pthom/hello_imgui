set(target_name hello_imgui)
message(STATUS "Library ${target_name}")

set(imgui_examples_dir ${CMAKE_CURRENT_LIST_DIR}/../../external/imgui/examples)

file(GLOB_RECURSE sources *.h *.cpp *.mm)
add_library(${target_name} ${sources})

target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(${target_name} PUBLIC
    project_properties
    imgui
    )

set(HELLOIMGUI_ASSETSDIR ${CMAKE_CURRENT_LIST_DIR}/../../helloimgui_assets)
target_compile_definitions(${target_name} PUBLIC -DHELLOIMGUI_ASSETSDIR="${HELLOIMGUI_ASSETSDIR}")

if (IOS)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLES3)
    target_compile_definitions(${target_name} PUBLIC -DIOS)
endif()


if (HELLOIMGUI_USE_SDL_OPENGL3)
#    find_package(SDL2 CONFIG REQUIRED)
    target_compile_definitions(${target_name} PUBLIC -DHELLOIMGUI_USE_SDL_OPENGL3)

    if (IOS)
        target_include_directories(${target_name} PUBLIC "/Users/pascal/dvp/OpenSource/ImGuiWork/hello_imgui/external/SDL/include/")
        target_link_directories(${target_name} PUBLIC "/Users/pascal/dvp/OpenSource/ImGuiWork/hello_imgui/external/SDL/Xcode-iOS/SDL/output/iOS/debug/")
        target_link_libraries(${target_name} PUBLIC SDL2)
        target_link_libraries(${target_name} PUBLIC
            "-framework Foundation"
            "-framework AVFoundation"
            "-framework AudioToolbox"
            "-framework CoreMedia"
            "-framework CoreMotion"
            "-framework CoreVideo"
            "-framework CoreGraphics"
            "-framework GameController"
            "-framework Metal"
            "-framework OpenGLES"
            "-framework UIKit"
            "-framework QuartzCore"
            )

        target_compile_definitions(${target_name} PUBLIC IMGUI_IMPL_OPENGL_LOADER_CUSTOM="<OpenGLES/ES3/gl.h>")
    else()
    #    target_link_libraries(${target_name} PUBLIC SDL2::SDL2main SDL2::SDL2-static)
        target_link_libraries(${target_name} PUBLIC SDL2main SDL2-static)
    endif()

    target_sources(${target_name} PRIVATE
        ${imgui_examples_dir}/imgui_impl_opengl3.h
        ${imgui_examples_dir}/imgui_impl_opengl3.cpp
        ${imgui_examples_dir}/imgui_impl_sdl.h
        ${imgui_examples_dir}/imgui_impl_sdl.cpp
        )
endif()

if (HELLOIMGUI_USE_GLFW_OPENGL3)
    find_package(glfw3 CONFIG REQUIRED)
    target_compile_definitions(${target_name} PUBLIC -DHELLOIMGUI_USE_GLFW_OPENGL3)
    target_link_libraries(${target_name} PUBLIC glfw)
    target_sources(${target_name} PRIVATE
        ${imgui_examples_dir}/imgui_impl_opengl3.h
        ${imgui_examples_dir}/imgui_impl_opengl3.cpp
        ${imgui_examples_dir}/imgui_impl_glfw.h
        ${imgui_examples_dir}/imgui_impl_glfw.cpp
        )
endif()

if (HELLOIMGUI_USE_QT)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_QT)
    target_link_libraries(${target_name} PUBLIC qt_imgui_quick)
endif()

if (HELLOIMGUI_USE_GLAD AND NOT IOS)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLAD)
    target_link_libraries(${target_name} PUBLIC glad)
endif()
