
function(hello_imgui_emscripten_adapt app_name)
    target_link_options(
        ${app_name} 
        PRIVATE
        ${EMSCRIPTEN_LINK_OPTIONS}
        --preload-file ${HELLOIMGUI_ASSETSDIR}@/
    )
    set_target_properties(
        ${app_name} 
        PROPERTIES
        SUFFIX .html
        )
endfunction()