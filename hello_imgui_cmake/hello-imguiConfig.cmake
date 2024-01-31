include(CMakeFindDependencyMacro)
find_dependency(imgui CONFIG REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/hello_imgui_add_app.cmake)

include(${CMAKE_CURRENT_LIST_DIR}/hello-imgui-targets.cmake)
