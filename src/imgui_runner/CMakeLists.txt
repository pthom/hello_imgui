add_library(imgui_runner imgui_runner.cpp imgui_runner.h)
target_include_directories(imgui_runner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui_runner PUBLIC
        project_properties
        imgui
        )
