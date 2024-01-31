include(CMakeFindDependencyMacro)
find_dependency(imgui CONFIG REQUIRED)

include(${HELLOIMGUI_CMAKE_PATH}/hello_imgui_add_app.cmake)
include(${HELLOIMGUI_CMAKE_PATH}/hello-imgui-targets.cmake)
