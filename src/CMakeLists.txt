add_subdirectory(hello_imgui)

if (HELLOIMGUI_BUILD_DEMOS)
    add_subdirectory(hello_imgui_demos)
endif()
if(HELLOIMGUI_BUILD_TESTS)
    add_subdirectory(hello_imgui_tests)
endif()
add_subdirectory(hello_imgui_remote)
