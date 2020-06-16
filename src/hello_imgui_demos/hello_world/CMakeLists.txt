set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/../../hello_imgui_cmake)
include(helloimgui_add_app)
helloimgui_add_app(hello_world hello_world.main.cpp)
