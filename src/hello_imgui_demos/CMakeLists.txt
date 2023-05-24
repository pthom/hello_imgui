set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(subdirs
    hello_imgui_demo_minimal
    hello_imgui_demo_classic
    hello_imgui_demodocking
    hello_world
    hello_globe
    hello_idbfs
)
foreach(target_name ${subdirs})
    add_subdirectory(${target_name})
    if (MSVC)
        hello_imgui_msvc_target_set_folder(${target_name} ${HELLOIMGUI_SOLUTIONFOLDER}/hello_imgui_demos)
    endif()
endforeach()
