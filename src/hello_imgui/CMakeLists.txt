set(target_name hello_imgui)
message(STATUS "Library ${target_name}")

set(imgui_examples_dir ${CMAKE_CURRENT_LIST_DIR}/../../external/imgui/examples)

file(GLOB_RECURSE sources *.h *.cpp)
add_library(${target_name} ${sources})
target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(${target_name} PUBLIC
    project_properties
    imgui
    )
target_compile_definitions(${target_name} PUBLIC -DHELLOIMGUI_FONTDIR="${CMAKE_CURRENT_LIST_DIR}/../../assets/fonts")

if (HELLOIMGUI_USE_SDL_OPENGL3)
    find_package(SDL2 CONFIG REQUIRED)
    target_compile_definitions(${target_name} PRIVATE -DHELLOIMGUI_USE_SDL_OPENGL3)
    target_link_libraries(${target_name} PUBLIC SDL2::SDL2main SDL2::SDL2-static)
    target_sources(${target_name} PRIVATE
        ${imgui_examples_dir}/imgui_impl_opengl3.h
        ${imgui_examples_dir}/imgui_impl_opengl3.cpp
        ${imgui_examples_dir}/imgui_impl_sdl.h
        ${imgui_examples_dir}/imgui_impl_sdl.cpp
        )
endif()

if (HELLOIMGUI_USE_GLFW_OPENGL3)
    find_package(glfw3 CONFIG REQUIRED)
    target_compile_definitions(${target_name} PRIVATE -DHELLOIMGUI_USE_GLFW_OPENGL3)
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

if (HELLOIMGUI_USE_GLAD)
    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_GLAD)
    target_link_libraries(${target_name} PUBLIC glad)
endif()
