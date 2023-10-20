set(BUILD_SHARED_LIBS OFF)

if (HELLOIMGUI_EMSCRIPTEN_PTHREAD)
    add_compile_options(-pthread)
    #    add_link_options(-pthread)  # Must be done per target
endif()
