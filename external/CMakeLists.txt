if (HELLOIMGUI_USE_QT)
    if(NOT IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/qtimgui)
        message(FATAL_ERROR "Please download qtimgui by running ${HELLOIMGUI_BASEPATH}/tools/qtimgui_download.py")
    endif()

    # qtimgui includes imgui as a submodule by default
    # we prefer to use our own version (in external/imgui)
    set(QTIMGUI_BUILD_IMGUI OFF CACHE BOOL "disable qtimgui included imgui" FORCE)
    add_subdirectory(qtimgui)
endif()

# glad OpenGL loader
if (HELLOIMGUI_USE_GLAD)
    include(${CMAKE_CURRENT_LIST_DIR}/OpenGL_Loaders/glad.cmake)
endif()

# imgui is built by default
# You can use your own :
# In this case, make sure that a target named imgui is available
# and add this code to your CMakeLists:
#
#     set(HELLOIMGUI_BUILD_IMGUI OFF CACHE BOOL "" FORCE)
#     set(HELLOIMGUI_IMGUI_SOURCE_DIR "path/to/your/imgui" CACHE STRING "" FORCE)
#
if (HELLOIMGUI_BUILD_IMGUI)
    set(imgui_dir ${CMAKE_CURRENT_LIST_DIR}/imgui)
    file(GLOB imgui_sources ${imgui_dir}/*.h ${imgui_dir}/*.cpp)
    if (HELLO_IMGUI_IMGUI_SHARED)
        add_library(imgui SHARED ${imgui_sources})
        install(TARGETS imgui DESTINATION ./lib/)
    else()
        add_library(imgui ${imgui_sources})
    endif()
    target_include_directories(imgui PUBLIC ${imgui_dir})
    if (MSVC)
        hello_imgui_msvc_target_set_folder(imgui ${HELLOIMGUI_SOLUTIONFOLDER}/external)
    endif()

    # Install
    install(FILES ${imgui_sources} DESTINATION imgui)
    install(DIRECTORY ${imgui_dir}/backends DESTINATION imgui)
    install(DIRECTORY ${imgui_dir}/misc/cpp DESTINATION imgui/misc)
    install(DIRECTORY ${imgui_dir}/misc/freetype DESTINATION imgui/misc)
endif()

if (HELLOIMGUI_USE_SDL_OPENGL3)
    # On desktop (linux, windows, osx) sdl is provided via vcpkg
    # On iOS, the build is done externally via tools/ios/sdl_compile_ios.sh
    if (ANDROID OR IOS)
        add_subdirectory(SDL)
    endif()
endif()
