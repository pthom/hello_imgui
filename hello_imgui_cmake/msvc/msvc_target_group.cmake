if (MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    function(hello_imgui_msvc_target_group_sources target_name)
        message(VERBOSE "hello_imgui_msvc_target_group_sources ${target_name}")
        # set(parent_path ${CMAKE_CURRENT_SOURCE_DIR})
        get_target_property(target_source_dir ${target_name} SOURCE_DIR)
        get_target_property(target_sources ${target_name} SOURCES)
        foreach (filename ${target_sources})
            get_filename_component(filepath "${filename}" REALPATH)
            file(RELATIVE_PATH filepath ${target_source_dir} ${filepath})
            get_filename_component(filepath "${filepath}" DIRECTORY)
            string(REPLACE "/" "\\" filepath "${filepath}")
            source_group("${filepath}" FILES ${filename})
            # message("source_group("${filepath}" FILES ${filename})")
        endforeach()
    endfunction(hello_imgui_msvc_target_group_sources)


    function(hello_imgui_msvc_target_set_folder target_name folder_name)
        set_target_properties(${target_name} PROPERTIES FOLDER ${folder_name})
    endfunction()

    ####################################################
    # Groups MSVC target according to disk folders
    # (this should be the sensible default, but it is not with msvc)
    ####################################################
    function(hello_imgui_msvc_sort_targets_by_folder current_dir)
        hello_imgui_get_all_subdirs_targets(all_targets ${current_dir})
        foreach(target ${all_targets})
            get_target_property(target_source_dir ${target} SOURCE_DIR)
            file(RELATIVE_PATH target_relative_dir ${CMAKE_CURRENT_SOURCE_DIR} ${target_source_dir})
            # message("target ${target} in ${target_relative_dir}")
            if (NOT target_relative_dir STREQUAL "")
                set_target_properties(${target} PROPERTIES FOLDER ${target_relative_dir})
            endif()
        endforeach()
    endfunction()

else()
    # Dummy functions so we don't have to call them conditionally
    function(hello_imgui_msvc_target_group_sources target_name)
    endfunction(hello_imgui_msvc_target_group_sources)
    function(hello_imgui_msvc_target_set_folder target_name)
    endfunction(hello_imgui_msvc_target_set_folder)
endif()

