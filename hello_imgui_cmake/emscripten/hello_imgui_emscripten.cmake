if (EMSCRIPTEN)

# Also see global emscripten flags in hello_imgui_cmake/emscripten/hello_imgui_emscripten_global_options.cmake


function(hello_imgui_emscripten_set_shell_file app_name)

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html)
        set(shell_template_file ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html)
    else()
        set(shell_template_file ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/shell.emscripten.html)
    endif()

    get_property(runtime_output_directory TARGET ${app_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY)
    if ("${runtime_output_directory}" STREQUAL "")
        set(real_output_directory ${CMAKE_CURRENT_BINARY_DIR})
    else()
        set(real_output_directory ${runtime_output_directory})
    endif()

    set(shell_file ${real_output_directory}/shell.emscripten.in.html)
    configure_file(
        ${shell_template_file}
        ${shell_file}
        @ONLY
    )
    target_link_options(
        ${app_name} 
        PRIVATE
        "SHELL:--shell-file ${shell_file}"
    )

    set_target_properties(
        ${app_name} 
        PROPERTIES
        SUFFIX .html
        )
endfunction()


function(hello_imgui_emscripten_target_compile_options app_name)
    target_link_options(${app_name} PRIVATE -sUSE_SDL=2 -sASSERTIONS)
    if (NOT HELLOIMGUI_EMSCRIPTEN_PTHREAD OR HELLOIMGUI_EMSCRIPTEN_PTHREAD_ALLOW_MEMORY_GROWTH)
        target_link_options(${app_name} PRIVATE -sALLOW_MEMORY_GROWTH)
    endif()
endfunction()


function(hello_imgui_set_emscripten_target_initial_memory_megabytes app_name nb_megabytes)
    math(EXPR nb_bytes "1048576 * ${nb_megabytes}")
    target_link_options(${app_name} PRIVATE -sINITIAL_MEMORY=${nb_bytes})
endfunction()


function(hello_imgui_platform_customization app_name)
    hello_imgui_emscripten_set_shell_file(${app_name})
    hello_imgui_emscripten_target_compile_options(${app_name})
endfunction()

endif (EMSCRIPTEN)
