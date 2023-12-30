# Bundle assets / emscripten version
function(hello_imgui_bundle_assets_from_folder app_name assets_folder)
    if (IS_DIRECTORY ${assets_folder})
        target_link_options(
            ${app_name}
            PRIVATE
            "SHELL:--preload-file ${assets_folder}@/"
        )
    else()
        message(WARNING "hello_imgui_bundle_assets_from_folder: ignoring missing folder ${assets_folder}")
    endif()
endfunction()
