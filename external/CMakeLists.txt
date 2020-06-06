if (HELLOIMGUI_USE_QT)
    # qtimgui build imgui as a submodule by default
    # we prefer to use our own version (in external/imgui)
    set(QTIMGUI_BUILD_IMGUI OFF CACHE BOOL "disable qtimgui included imgui" FORCE)
    add_subdirectory(qtimgui)
endif()

# GLFW (Free, open source, portable framework for OpenGL application development).
if (HELLOIMGUI_USE_GLFW)
    SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
    SET(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
    SET(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
    set(GLFW_BUILD_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
    add_subdirectory(glfw)
endif()

if (HELLOIMGUI_USE_SDL AND NOT EMSCRIPTEN) # SDL is already build by emscripten
    add_subdirectory(SDL)
endif()

# glad OpenGL loader
if (HELLOIMGUI_USE_GLAD)
    include(${CMAKE_CURRENT_LIST_DIR}/OpenGL_Loaders/glad.cmake)
endif()

# imgui is built by default
# You can use your own (but make sure that a target named imgui is available
if (HELLOIMGUI_BUILD_IMGUI)
    set(imgui_dir ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
    add_library(imgui
            ${imgui_dir}/imgui_demo.cpp
            ${imgui_dir}/imgui_draw.cpp
            ${imgui_dir}/imgui_internal.h
            ${imgui_dir}/imgui_widgets.cpp
            ${imgui_dir}/imgui.cpp
            ${imgui_dir}/imgui.h
            )
    target_include_directories(imgui PUBLIC ${imgui_dir})
endif()
