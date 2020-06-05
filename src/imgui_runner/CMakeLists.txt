add_library(imgui_runner imgui_runner.cpp imgui_runner.h)
target_include_directories(imgui_runner INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(imgui_runner INTERFACE project_properties)
