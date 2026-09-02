# Legacy compatibility shim: the CMake package was renamed from "hello-imgui" to "hello_imgui".
# This file keeps find_package(hello-imgui) and the hello-imgui:: namespace working.
# It will be removed in a future release. Please use:
#     find_package(hello_imgui CONFIG REQUIRED)
#     target_link_libraries(your_target PRIVATE hello_imgui::hello_imgui)
message(DEPRECATION "find_package(hello-imgui) is deprecated: use find_package(hello_imgui) and the hello_imgui:: namespace")
find_package(hello_imgui CONFIG REQUIRED HINTS "${CMAKE_CURRENT_LIST_DIR}/../hello_imgui")
if(NOT TARGET hello-imgui::hello_imgui)
    add_library(hello-imgui::hello_imgui INTERFACE IMPORTED)
    target_link_libraries(hello-imgui::hello_imgui INTERFACE hello_imgui::hello_imgui)
endif()
