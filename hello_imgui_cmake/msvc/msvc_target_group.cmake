if (MSVC)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    function(hello_imgui_msvc_target_group_sources target_name)
        message(VERBOSE "hello_imgui_msvc_target_group_sources ${target_name}")
        set(parent_path ${CMAKE_CURRENT_SOURCE_DIR})
        get_target_property(target_sources ${target_name} SOURCES)
        foreach (filename ${target_sources})
            get_filename_component(filepath "${filename}" REALPATH)
            file(RELATIVE_PATH filepath ${parent_path} ${filepath})
            get_filename_component(filepath "${filepath}" DIRECTORY)
            string(REPLACE "/" "\\" filepath "${filepath}")
            source_group("${filepath}" FILES ${filename})
        endforeach()
    endfunction(hello_imgui_msvc_target_group_sources)


    function(hello_imgui_msvc_target_set_folder target_name folder_name)
        set_target_properties(${target_name} PROPERTIES FOLDER ${folder_name})
    endfunction()
endif()

