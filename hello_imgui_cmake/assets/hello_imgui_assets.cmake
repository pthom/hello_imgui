
if (EMSCRIPTEN)

    # Bundle assets / emscripten version
    function(hello_imgui_bundle_assets app_name assets_folder)
        target_link_options(
            ${app_name}
            PRIVATE
            "SHELL:--preload-file ${assets_folder}@/"
        )
    endfunction()

elseif(IOS)

    # Bundle assets / iOS version
    function(hello_imgui_bundle_assets app_name assets_folder)
        FILE(GLOB_RECURSE assets ${assets_folder}/*.*)
        target_sources(${app_name} PRIVATE ${assets})
        foreach(asset ${assets})
            string(REPLACE ${assets_folder}/ "" asset_relative ${asset})
            get_filename_component(asset_dir ${asset_relative} DIRECTORY)
            SET_SOURCE_FILES_PROPERTIES (
                ${app_name}
                ${asset}
                PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources/${asset_dir}
            )
        endforeach()
    endfunction()

elseif(ANDROID)

    function(hello_imgui_bundle_assets app_name assets_folder)
        message("hello_imgui_bundle_assets ${app_name} ${assets_folder}")
        FILE(GLOB children ${assets_folder}/*)
        # message(FATAL_ERROR "HELLO_IMGUI_ANDROID_ASSETS_FOLDER=${HELLO_IMGUI_ANDROID_ASSETS_FOLDER}")
        if (DEFINED HELLO_IMGUI_ANDROID_ASSETS_FOLDER)
            foreach(child ${children})
                message(VERBOSE "    Copying ${child}")
                FILE(COPY ${child} DESTINATION ${HELLO_IMGUI_ANDROID_ASSETS_FOLDER})
            endforeach()
        endif()
    endfunction()


else()

    # Bundle assets
    function(hello_imgui_bundle_assets app_name assets_folder)
        message(VERBOSE "hello_imgui_bundle_assets ${app_name} ${assets_folder}")
        FILE(GLOB children ${assets_folder}/*)
        foreach(child ${children})
            message(VERBOSE "    Copying ${child}")
            FILE(COPY ${child} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets/)
        endforeach()
    endfunction()

endif()


function(hello_imgui_add_local_assets app_name)
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets)
        message(VERBOSE "hello_imgui_emscripten_add_local_assets: ${app_name} found local assets")
        hello_imgui_bundle_assets(${app_name} ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    endif()
    set(HELLOIMGUI_ASSETSDIR ${CMAKE_CURRENT_LIST_DIR}/hello_imgui_assets CACHE STRING "Hello imgui assets path" FORCE)
endfunction()

