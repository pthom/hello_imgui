# imgui is built by default
# You can use your own (but make sure that a target named imgui is available
option(IMGUICV_BUILD_IMGUI "Link imgui provided as a subdirectory by imguicv" ON)
if (IMGUICV_BUILD_IMGUI)
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

