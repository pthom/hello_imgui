if (HELLOIMGUI_USE_SOKOL)

    target_compile_definitions(${target_name} PUBLIC HELLOIMGUI_USE_SOKOL)
    target_link_libraries(${target_name} PUBLIC sokol)

    if (APPLE)
        option(HELLOIMGUI_SOKOL_METAL "Use Metal backend for sokol" ON)
    endif()
    if (EMSCRIPTEN)
#        set(HELLOIMGUI_SOKOL_METAL OFF CACHE BOOL "" FORCE)
        set(HELLOIMGUI_SOKOL_GLES3 ON)
        set(HELLOIMGUI_HAS_OPENGL ON)
        target_compile_definitions(hello_imgui PUBLIC HELLOIMGUI_HAS_OPENGL)
    endif()

    if (HELLOIMGUI_SOKOL_METAL)
        target_compile_options(${target_name} PRIVATE
            -fobjc-arc
            -DSOKOL_METAL
            )
        target_link_libraries(${target_name} PUBLIC
            "-framework Metal"
            "-framework Cocoa"
            "-framework MetalKit"
            "-framework Quartz"
            "-framework AudioToolbox"
            )
     endif()

    if (HELLOIMGUI_SOKOL_GLES3)
        target_compile_options(${target_name} PRIVATE
            -DSOKOL_GLES3
            )
    endif()
endif(HELLOIMGUI_USE_SOKOL)