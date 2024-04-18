set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(HELLOIMGUI_WITH_NETIMGUI AND HELLOIMGUI_HAS_OPENGL3)
    add_subdirectory(netimgui_remote_display)
endif()
