# hello_imgui_bundle_assets_from_folder for Desktop platforms (Windows, Linux, macOS when not using app bundles)
#
# In this case, we will:
#     - copy the assets folder to the output directory (so that we can run the app without install)
#     - install the assets folder and the app exe to the install directory
#     - set the debugger working directory to the output directory (for Win32)


function(_do_install_asset app_name src dst)
    hello_imgui_get_real_output_directory(${app_name} real_output_directory)

#    # Copy at configure time
#    FILE(COPY "${src}" DESTINATION "${real_output_directory}/${dst}")
#    message(VERBOSE "_do_copy_asset=> FILE(COPY ${src} DESTINATION ${real_output_directory}/${dst})")

    # Copy at install time
    if (HELLOIMGUI_ADD_APP_WITH_INSTALL)
        if (IS_DIRECTORY ${src})
            install(DIRECTORY "${src}" DESTINATION "${CMAKE_INSTALL_PREFIX}/${dst}")
            message(VERBOSE "_do_copy_asset=> install(DIRECTORY ${src} DESTINATION ${CMAKE_INSTALL_PREFIX}/${dst}  )")
        else()
            install(FILES "${src}" DESTINATION "${CMAKE_INSTALL_PREFIX}/${dst}")
            message(VERBOSE "_do_copy_asset=> install(FILES ${src} DESTINATION ${CMAKE_INSTALL_PREFIX}/${dst}  )")
        endif()
    endif()
endfunction()


# Bundle assets
function(hello_imgui_bundle_assets_from_folder app_name assets_folder)
    message(VERBOSE "hello_imgui_bundle_assets_from_folder ${app_name} ${assets_folder}")
    FILE(GLOB children ${assets_folder}/*)
    foreach(child ${children})
        # Will copy install time
        _do_install_asset(${app_name} ${child} assets/)
    endforeach()

    # Copy at build time
    if (true)
        FILE(GLOB_RECURSE children_files_only LIST_DIRECTORIES false RELATIVE ${assets_folder} ${assets_folder}/*)
        hello_imgui_get_real_output_directory(${app_name} real_output_directory)
        set(dst_folder "${real_output_directory}/assets")
        foreach(child ${children_files_only})
            add_custom_command(TARGET ${app_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${assets_folder}/${child}"
                    "${dst_folder}/${child}")
        endforeach ()
    endif()

    if (WIN32)
        # Fix msvc quirk: set the debugger working dir to the exe dir!
        hello_imgui_get_real_output_directory(${app_name} app_output_dir)
        set_target_properties(${app_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${app_output_dir})
    endif()

    # Install app exe to install directory
    if (HELLOIMGUI_ADD_APP_WITH_INSTALL)
        install(TARGETS ${app_name} DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
endfunction()
