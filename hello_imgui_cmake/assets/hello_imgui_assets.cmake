function(hello_imgui_file_glob_recurse_relative out_file_list folder)
    FILE(GLOB_RECURSE files_fullpath ${folder}/*)
    set(files_relativepath "")
    foreach(file_fullpath ${files_fullpath})
        string(REPLACE ${folder}/ "" file_relativepath ${file_fullpath})
        list(APPEND files_relative_path ${file_relativepath})
    endforeach()
    set(${out_file_list} ${files_relative_path} PARENT_SCOPE)
endfunction()


if (EMSCRIPTEN)

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

elseif(IOS)

    # Bundle assets / iOS version
    function(hello_imgui_bundle_assets_from_folder app_name assets_folder)
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

    function(hello_imgui_bundle_assets_from_folder app_name assets_folder)
        message(VERBOSE "hello_imgui_bundle_assets_from_folder ${app_name} ${assets_folder}")
        FILE(GLOB children ${assets_folder}/*)
        if (DEFINED HELLO_IMGUI_ANDROID_ASSETS_FOLDER)
            foreach(child ${children})
                message(VERBOSE "    Copying ${child}")
                FILE(COPY ${child} DESTINATION ${HELLO_IMGUI_ANDROID_ASSETS_FOLDER})
            endforeach()
        endif()
    endfunction()


else()
	function(hello_imgui_get_real_output_directory app_name r)
        # Warning: RUNTIME_OUTPUT_DIRECTORY is stable, but RUNTIME_OUTPUT_DIRECTORY_CONFIG can vary between Debug/Release configs
        # cf https://cmake.org/cmake/help/latest/prop_tgt/RUNTIME_OUTPUT_DIRECTORY_CONFIG.html
        get_property(runtime_output_directory TARGET ${app_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY)
        if ("${runtime_output_directory}" STREQUAL "")
            set(${r} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
        else()
			set(${r} ${runtime_output_directory} PARENT_SCOPE)
        endif()
	endfunction()

    function(_do_copy_asset app_name src dst)
		hello_imgui_get_real_output_directory(${app_name} real_output_directory)
		
        FILE(COPY "${src}" DESTINATION "${real_output_directory}/${dst}")
        message(VERBOSE "_do_copy_asset=> FILE(COPY ${src} DESTINATION ${real_output_directory}/${dst})")

        if (IS_DIRECTORY ${src})
            install(DIRECTORY "${src}" DESTINATION "${CMAKE_INSTALL_PREFIX}/${dst}")
            message(VERBOSE "_do_copy_asset=> install(DIRECTORY ${src} DESTINATION ${CMAKE_INSTALL_PREFIX}/${dst}  )")
        else()
            install(FILES "${src}" DESTINATION "${CMAKE_INSTALL_PREFIX}/${dst}")
            message(VERBOSE "_do_copy_asset=> install(FILES ${src} DESTINATION ${CMAKE_INSTALL_PREFIX}/${dst}  )")
        endif()
    endfunction()

    # Bundle assets
    function(hello_imgui_bundle_assets_from_folder app_name assets_folder)
        message(VERBOSE "hello_imgui_bundle_assets_from_folder ${app_name} ${assets_folder}")
        FILE(GLOB children ${assets_folder}/*)
        foreach(child ${children})
            _do_copy_asset(${app_name} ${child} assets/)
        endforeach()
    endfunction()

endif()


function(hello_imgui_copy_folder1_files_missing_from_folder2 folder_src_1 folder_src_2 folder_dst)
    hello_imgui_file_glob_recurse_relative(files1 ${folder_src_1})
    hello_imgui_file_glob_recurse_relative(files2 ${folder_src_2})

    foreach(file1 ${files1})
        list(FIND files2 ${file1} index)
        if (${index} EQUAL -1)
            get_filename_component(file1_dir ${file1} DIRECTORY)
            FILE(COPY ${folder_src_1}/${file1} DESTINATION ${folder_dst}/${file1_dir})
        endif()
    endforeach()
endfunction()


function(hello_imgui_bundle_assets app_name)
    set(common_assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_assets)
    set(local_assets_folder ${CMAKE_CURRENT_SOURCE_DIR}/assets)

    set(common_assets_folder_copy ${CMAKE_CURRENT_BINARY_DIR}/tmp/common_assets)
    file(REMOVE_RECURSE ${common_assets_folder_copy})
    hello_imgui_copy_folder1_files_missing_from_folder2(
        ${common_assets_folder} ${local_assets_folder} ${common_assets_folder_copy})

    hello_imgui_bundle_assets_from_folder(${app_name} ${common_assets_folder_copy})
    
    if (IS_DIRECTORY ${local_assets_folder})
        message(VERBOSE "hello_imgui_bundle_assets: ${app_name} found local assets")
        hello_imgui_bundle_assets_from_folder(${app_name} ${local_assets_folder})
    endif()

    if (WIN32)
		# Fix msvc quirk: set the debugger working dir to the exe dir!
		hello_imgui_get_real_output_directory(${app_name} app_output_dir)
		set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_output_dir})
	endif()

endfunction()
