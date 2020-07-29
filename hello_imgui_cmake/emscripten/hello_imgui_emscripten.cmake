if (EMSCRIPTEN)

function(hello_imgui_emscripten_add_shell_file app_name)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html)
        set(shell_template_file ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html)
    else()
        set(shell_template_file ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/shell.emscripten.html)
    endif()

    set(shell_file ${CMAKE_CURRENT_BINARY_DIR}/shell.emscripten.in.html)
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

# hello_imgui_emscripten_target_compile_options needs fix!
# For the moment, these options are global via hello_imgui_emscripten_global_options.cmake
# function(hello_imgui_emscripten_target_compile_options target_name)
#     target_compile_options(
#         ${target_name} 
#         PUBLIC
#         "SHELL:-s USE_SDL=2"
#         "SHELL:-s USE_WEBGL2=1"
#         "SHELL:-s WASM=1" 
#         "SHELL:-s FULL_ES3=1"
#         "SHELL:-s ALLOW_MEMORY_GROWTH=1"
#     )
# endfunction()


function(hello_imgui_platform_customization app_name)
    hello_imgui_emscripten_add_shell_file(${app_name})
    # hello_imgui_emscripten_target_compile_options(${app_name})
endfunction()

endif (EMSCRIPTEN)
