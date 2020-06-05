set(imgui_dir ${CMAKE_CURRENT_LIST_DIR}/../../external/imgui)
add_library(imgui_runner_glfw
    imgui_runner_glfw.cpp
    ${imgui_dir}/examples/imgui_impl_glfw.h
    ${imgui_dir}/examples/imgui_impl_glfw.cpp
    )
target_link_libraries(imgui_runner_glfw PUBLIC imgui_runner glfw)
