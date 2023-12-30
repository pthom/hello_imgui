# Platform dependent definition of hello_imgui_bundle_assets_from_folder
if (EMSCRIPTEN)
    include(${CMAKE_CURRENT_LIST_DIR}/him_assets_emscripten.cmake)
elseif(IOS OR (MACOSX AND (NOT HELLOIMGUI_MACOS_NO_BUNDLE)))
    include(${CMAKE_CURRENT_LIST_DIR}/him_assets_apple_bundle.cmake)
elseif(ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/him_assets_android.cmake)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/him_assets_desktop.cmake)
endif()


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


function(hello_imgui_file_glob_recurse_relative out_file_list folder)
    FILE(GLOB_RECURSE files_fullpath ${folder}/*)
    set(files_relativepath "")
    foreach(file_fullpath ${files_fullpath})
        string(REPLACE ${folder}/ "" file_relativepath ${file_fullpath})
        list(APPEND files_relative_path ${file_relativepath})
    endforeach()
    set(${out_file_list} ${files_relative_path} PARENT_SCOPE)
endfunction()


function(hello_imgui_copy_folder1_files_missing_from_folder2 folder_src_1 folder_src_2 folder_dst)
    if (NOT EXISTS ${folder_dst})
        file(MAKE_DIRECTORY ${folder_dst})
    endif()
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


function(hello_imgui_bundle_assets app_name assets_location)
    set(common_assets_folder ${HELLOIMGUI_BASEPATH}/hello_imgui_assets)
    set(local_assets_folder ${assets_location})

    set(common_assets_folder_copy ${CMAKE_CURRENT_BINARY_DIR}/tmp/common_assets)
    file(REMOVE_RECURSE ${common_assets_folder_copy})
    hello_imgui_copy_folder1_files_missing_from_folder2(
        ${common_assets_folder} ${local_assets_folder} ${common_assets_folder_copy})

    hello_imgui_bundle_assets_from_folder(${app_name} ${common_assets_folder_copy})
    
    if (IS_DIRECTORY ${local_assets_folder})
        message(VERBOSE "hello_imgui_bundle_assets: ${app_name} found local assets")
        hello_imgui_bundle_assets_from_folder(${app_name} ${local_assets_folder})
    endif()
endfunction()
