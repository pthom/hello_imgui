set(GLAD_DIR ${CMAKE_CURRENT_LIST_DIR}/glad CACHE INTERNAL "Directory of glad")
set(GLAD_FILES
    ${GLAD_DIR}/src/glad.c
    ${GLAD_DIR}/include/glad/glad.h
    ${GLAD_DIR}/include/KHR/khrplatform.h)
add_library(glad ${GLAD_FILES})
target_include_directories(glad PUBLIC SYSTEM ${GLAD_DIR}/include)
if(WIN32)
    target_link_libraries(glad PUBLIC opengl32.lib)
else()
    target_link_libraries(glad PUBLIC ${OPENGL_LIBRARIES})
endif()
get_target_property(library_type glad TYPE)
target_compile_definitions(glad PUBLIC HELLOIMGUI_USE_GLAD)
if (library_type STREQUAL SHARED_LIBRARY)
    # If glad is a shared lobrary, define the macro GLAD_API_EXPORT on the command line.
    target_compile_definitions(glad PRIVATE GLAD_GLAPI_EXPORT)
    target_compile_definitions(glad PUBLIC GLAD_GLAPI_EXPORT PRIVATE GLAD_GLAPI_EXPORT_BUILD)
endif()

if (MSVC)
    hello_imgui_msvc_target_set_folder(glad ${HELLOIMGUI_SOLUTIONFOLDER}/external/OpenGL_Loaders)
endif()

if(PROJECT_IS_TOP_LEVEL)
    install(TARGETS glad DESTINATION ./lib/)
endif()
