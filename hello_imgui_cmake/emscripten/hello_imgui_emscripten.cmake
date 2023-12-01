if (EMSCRIPTEN)

include(${CMAKE_CURRENT_LIST_DIR}/hello_imgui_emscripten_global_options.cmake)


function(_him_get_real_output_directory app_name output_dir)
    get_property(runtime_output_directory TARGET ${app_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY)
    if ("${runtime_output_directory}" STREQUAL "")
        set(${output_dir} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
    else()
        set(${output_dir} ${runtime_output_directory} PARENT_SCOPE)
    endif()
endfunction()


function(_hello_imgui_emscripten_set_shell_file app_name)

    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html)
        set(shell_file ${CMAKE_CURRENT_SOURCE_DIR}/shell.emscripten.html)
    else()
        set(shell_template_file ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/shell.emscripten.html)
        _him_get_real_output_directory(${app_name} real_output_directory)
        file(MAKE_DIRECTORY ${real_output_directory}) # make dir real_output_directory if needed
        set(shell_file ${CMAKE_CURRENT_BINARY_DIR}/shell.emscripten_${app_name}.html)
        set(HELLO_IMGUI_FAVICON "${app_name}_favicon.ico")
        configure_file(
            ${shell_template_file}
            ${shell_file}
            @ONLY
        )
    endif()

    target_link_options(
        ${app_name} 
        PRIVATE
        "SHELL:--shell-file ${shell_file}"
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
    set(custom_app_icon ${real_output_directory}/${app_name}_favicon.ico)

    # if the user did not provide a custom icon, we copy the standard hello_imgui icon
    set(custom_app_png_icon ${assets_location}/app_settings/icon.png)
    if (NOT EXISTS ${custom_app_png_icon})
        set(standard_favicon ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/hello_imgui_favicon.ico)
        message(VERBOSE "_hello_imgui_create_ico: copying ${standard_favicon} to ${custom_app_icon} for app ${app_name}")
        # copy and rename the standard_favicon to custom_app_icon
        # CMake forces us to use configure_file if we want to rename the file... Whatever
        configure_file(${standard_favicon} ${custom_app_icon} COPYONLY)
        return()
    endif()

    # find python program
    find_program(PYTHON_EXECUTABLE NAMES python3 python)
    # if python is not found, we can't create the ico file
    if (NOT PYTHON_EXECUTABLE)
        message(WARNING "
        ${app_name}: can't create a favicon.ico file from ${custom_app_png_icon}
                (did not find python)
                This is not a fatal error, but the web app will not have a favicon.
                ")
        return()
    endif()

    # We need to convert icon.png to a favicon.ico file
    set(script_png_to_ico "${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/emscripten_png_icon_to_ico.py")
    message(VERBOSE "_hello_imgui_create_ico: converting ${custom_app_png_icon} to favicon.ico for app ${app_name}")
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} ${script_png_to_ico} ${custom_app_png_icon} ${custom_app_icon}
        RESULT_VARIABLE script_png_to_ico_result
    )
    if (NOT ${script_png_to_ico_result} EQUAL 0)
        message(WARNING "
        ${app_name}: failed to create a favicon.ico file from ${custom_app_icon}
                Tried to run:
                    ${PYTHON_EXECUTABLE} ${script_png_to_ico} ${custom_app_png_icon} ${custom_app_icon}

                This is not a fatal error, but the web app will not have a favicon.
                In order to have a custom icon, you need to have python with the Pillow package:
                    pip install Pillow
                ")
        return()
    endif()
endfunction()



function(hello_imgui_platform_customization app_name assets_location)
    _hello_imgui_create_emscripten_ico(${app_name} ${assets_location})
    _hello_imgui_emscripten_set_shell_file(${app_name})
    _hello_imgui_emscripten_target_compile_options(${app_name})
endfunction()

endif (EMSCRIPTEN)
