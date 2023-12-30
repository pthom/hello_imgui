include(${CMAKE_CURRENT_LIST_DIR}/hello_imgui_emscripten_global_options.cmake)


function(_him_get_real_output_directory app_name output_dir)
    get_property(runtime_output_directory TARGET ${app_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY)
    if ("${runtime_output_directory}" STREQUAL "")
        set(${output_dir} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
    else()
        set(${output_dir} ${runtime_output_directory} PARENT_SCOPE)
    endif()
endfunction()


function(_hello_imgui_emscripten_add_js_css_resources app_name assets_location)
    set(resources_dir ${assets_location}/app_settings/emscripten)
    _him_get_real_output_directory(${app_name} real_output_directory)

    # find all files inside resources_dir
    file(GLOB resources_files ${resources_dir}/*)
    # remove the shell.emscripten.html file
    list(FILTER resources_files EXCLUDE REGEX "shell.emscripten.html$")
    list(FILTER resources_files EXCLUDE REGEX "shell.emscripten.html.in$")

    # copy all files to the output directory (no configure_file here)
    foreach(resource_file ${resources_files})
        file(COPY ${resource_file} DESTINATION ${real_output_directory})
        if(HELLOIMGUI_ADD_APP_WITH_INSTALL)
            install(FILES ${resource_file} DESTINATION ${CMAKE_INSTALL_PREFIX})
        endif()
    endforeach()
endfunction()


function(_hello_imgui_emscripten_set_shell_file app_name assets_location)
    set(possible_shell_files
        ${assets_location}/app_settings/emscripten/shell.emscripten.html
        ${assets_location}/app_settings/emscripten/shell.emscripten.html.in
        ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html
        ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html.in
        ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/shell.emscripten.html
    )
    set(shell_template_file "")
    foreach(shell_file ${possible_shell_files})
        if (EXISTS ${shell_file})
            set(shell_template_file ${shell_file})
            break()
        endif()
    endforeach()

    if (NOT EXISTS ${shell_template_file})
        message(FATAL_ERROR "
        ${app_name}: could not find a shell.emscripten.html file in the following locations:
            ${possible_shell_files}
        ")
    endif()

    _him_get_real_output_directory(${app_name} real_output_directory)
    file(MAKE_DIRECTORY ${real_output_directory}) # make dir real_output_directory if needed
    set(shell_file_configured ${CMAKE_CURRENT_BINARY_DIR}/tmp/shell.emscripten_${app_name}.html)
    set(HELLO_IMGUI_FAVICON "${app_name}_favicon.png")
    configure_file(
        ${shell_template_file}
        ${shell_file_configured}
        @ONLY
    )

    target_link_options(
        ${app_name} 
        PRIVATE
        "SHELL:--shell-file ${shell_file_configured}"
    )

    set_target_properties(
        ${app_name} 
        PROPERTIES
        SUFFIX .html
        )
endfunction()


function(_hello_imgui_emscripten_target_compile_options app_name)
    target_link_options(${app_name} PRIVATE -sUSE_SDL=2 -sASSERTIONS)
    if (NOT HELLOIMGUI_EMSCRIPTEN_PTHREAD OR HELLOIMGUI_EMSCRIPTEN_PTHREAD_ALLOW_MEMORY_GROWTH)
        target_link_options(${app_name} PRIVATE -sALLOW_MEMORY_GROWTH)
    endif()
    if (HELLOIMGUI_EMSCRIPTEN_PTHREAD)
        # if using multithreading, *everything* needs to be compiled with -pthread
        target_compile_options(imgui PUBLIC -pthread)
        target_link_options(imgui PUBLIC -pthread)
        target_compile_definitions(hello_imgui PUBLIC HELLOIMGUI_EMSCRIPTEN_PTHREAD)
    endif()
endfunction()


function(hello_imgui_set_emscripten_target_initial_memory_megabytes app_name nb_megabytes)
    math(EXPR nb_bytes "1048576 * ${nb_megabytes}")
    target_link_options(${app_name} PRIVATE -sINITIAL_MEMORY=${nb_bytes})
endfunction()


function(_hello_imgui_create_emscripten_ico app_name assets_location)
    _him_get_real_output_directory(${app_name} real_output_directory)
    file(MAKE_DIRECTORY ${real_output_directory}) # make dir real_output_directory if needed
    set(final_icon ${real_output_directory}/${app_name}_favicon.png)

    # if the user provided a custom icon, we use it
    set(user_provided_app_png_icon ${assets_location}/app_settings/icon.png)

    if (NOT EXISTS ${user_provided_app_png_icon})
        # if the user did not provide a custom icon, we copy the standard hello_imgui icon
        set(standard_favicon ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/hello_imgui_favicon.png)
        message(VERBOSE "_hello_imgui_create_ico: copying ${standard_favicon} to ${custom_app_icon} for app ${app_name}")
        # copy and rename the standard_favicon to custom_app_icon
        # CMake forces us to use configure_file if we want to rename the file... Whatever
        configure_file(${standard_favicon} ${final_icon} COPYONLY)
    else()
        message(VERBOSE "_hello_imgui_create_ico: copying ${standard_favicon} to ${custom_app_icon} for app ${app_name}")
        configure_file(${user_provided_app_png_icon} ${final_icon} COPYONLY)
    endif()

    if (HELLOIMGUI_ADD_APP_WITH_INSTALL)
        install(FILES ${final_icon} DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()

endfunction()



function(hello_imgui_platform_customization app_name assets_location)
    _hello_imgui_create_emscripten_ico(${app_name} ${assets_location})
    _hello_imgui_emscripten_set_shell_file(${app_name} ${assets_location})
    _hello_imgui_emscripten_target_compile_options(${app_name})
    _hello_imgui_emscripten_add_js_css_resources(${app_name} ${assets_location})
endfunction()
