if (EMSCRIPTEN)

function(hello_imgui_emscripten_add_shell_file app_name)
    target_link_options(
        ${app_name} 
        PRIVATE
        "SHELL:--shell-file ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/runner_emscripten_shell.html"
    )
    set_target_properties(
        ${app_name} 
        PROPERTIES
        SUFFIX .html
        )
endfunction()

# hello_imgui_emscripten_target_compile_options needs fix!!!
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
    hello_imgui_bundle_assets(${app_name} ${HELLOIMGUI_ASSETSDIR})
    hello_imgui_emscripten_add_shell_file(${app_name})
    # hello_imgui_emscripten_target_compile_options(${app_name})
endfunction()

endif (EMSCRIPTEN)
