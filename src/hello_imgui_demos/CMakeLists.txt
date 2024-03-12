set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(subdirs
    hello_imgui_demo_minimal
    hello_imgui_demo_classic
    hello_imgui_demodocking
    hello_world
    hello_globe
    hello_idbfs
    hello_imgui_demo_test_engine
    hello_custom_background
)
if(HELLOIMGUI_HAS_METAL)
    # Demo of EDR (Extended Dynamic Range) support, only for Metal
    list(APPEND subdirs hello_edr)
endif()

foreach(target_name ${subdirs})
    add_subdirectory(${target_name})
    if (MSVC)
        hello_imgui_msvc_target_set_folder(${target_name} ${HELLOIMGUI_SOLUTIONFOLDER}/hello_imgui_demos)
    endif()
endforeach()
