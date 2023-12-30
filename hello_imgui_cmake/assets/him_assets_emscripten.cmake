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

    if (HELLOIMGUI_ADD_APP_WITH_INSTALL)
        hello_imgui_get_real_output_directory(${app_name} real_output_directory)
        install(FILES
            ${real_output_directory}/${app_name}.html
            ${real_output_directory}/${app_name}.data
            ${real_output_directory}/${app_name}.js
            ${real_output_directory}/${app_name}.wasm
            DESTINATION ${CMAKE_INSTALL_PREFIX}
        )
    endif()
endfunction()
