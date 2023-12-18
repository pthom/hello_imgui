function(apkCMake_makeSymLinks)
    set(links_folder ${apkCMake_outputProjectFolder}/app/jni)
    file(CREATE_LINK ${HELLOIMGUI_BASEPATH} ${links_folder}/hello_imgui_subrepo SYMBOLIC)
    file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR} ${links_folder}/appDir SYMBOLIC)

    # apkCMake_sdl_symlink_target was filled by _him_prepare_android_sdl_symlink() is src/hello_imgui/CMakeLists.txt
    file(CREATE_LINK ${apkCMake_sdl_symlink_target} ${links_folder}/SDL SYMBOLIC)
endfunction()
