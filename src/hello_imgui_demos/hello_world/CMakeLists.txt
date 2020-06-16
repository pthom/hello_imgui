set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/../../hello_imgui_cmake)
include(add_hello_imgui_app)
add_hello_imgui_app(hello_world hello_world.main.cpp)
