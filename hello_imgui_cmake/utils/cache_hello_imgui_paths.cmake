set(helloimgui_basepath ${CMAKE_CURRENT_LIST_DIR}/../..)
# change helloimgui_basepath to absolute path
get_filename_component(helloimgui_basepath ${helloimgui_basepath} ABSOLUTE)

set(HELLOIMGUI_BASEPATH ${helloimgui_basepath} CACHE STRING "Hello imgui base path" FORCE)
set(HELLOIMGUI_CMAKE_PATH ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake CACHE STRING "Hello imgui cmake path" FORCE)
list(APPEND CMAKE_MODULE_PATH ${HELLOIMGUI_CMAKE_PATH})
