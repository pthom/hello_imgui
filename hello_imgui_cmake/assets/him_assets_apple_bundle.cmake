# Bundle assets / macOS and iOS app version
function(hello_imgui_apple_bundle_add_files_from_folder app_name assets_folder location_in_bundle)
    file(GLOB_RECURSE assets ${assets_folder}/*.*)
    target_sources(${app_name} PRIVATE ${assets})
    foreach(asset ${assets})
        file(RELATIVE_PATH asset_relative ${assets_folder} ${asset})
        get_filename_component(asset_dir ${asset_relative} DIRECTORY)
        set_source_files_properties(
            ${asset}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION ${location_in_bundle}/${asset_dir}
        )
    endforeach()
endfunction()

function(hello_imgui_bundle_assets_from_folder app_name assets_folder)
    hello_imgui_apple_bundle_add_files_from_folder(${app_name} ${assets_folder} "Resources/assets")
endfunction()
