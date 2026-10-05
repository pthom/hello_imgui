add_executable(hello_imgui_tests
    hello_imgui_ini_settings_test.cpp imgui_theme_test.cpp touch_scroll_test.cpp hello_imgui_tests_main.cpp
    # the null backend, for the tests that run frames without a window
    ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_null.cpp)
target_include_directories(hello_imgui_tests PRIVATE ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends)
target_link_libraries(hello_imgui_tests PRIVATE hello_imgui)
