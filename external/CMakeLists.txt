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
    file(GLOB imgui_sources ${HELLOIMGUI_IMGUI_SOURCE_DIR}/*.h ${HELLOIMGUI_IMGUI_SOURCE_DIR}/*.cpp)
    set(imgui_sources ${imgui_sources} ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.h)
    if (HELLO_IMGUI_IMGUI_SHARED)
        add_library(imgui SHARED ${imgui_sources})
        install(TARGETS imgui DESTINATION ./lib/)
    else()
        add_library(imgui ${imgui_sources})
    endif()
    target_include_directories(imgui PUBLIC ${HELLOIMGUI_IMGUI_SOURCE_DIR})
    if (MSVC)
        hello_imgui_msvc_target_set_folder(imgui ${HELLOIMGUI_SOLUTIONFOLDER}/external)
    endif()

    # Install
    if(PROJECT_IS_TOP_LEVEL)
        install(FILES ${imgui_sources} DESTINATION imgui)
        install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends DESTINATION imgui)
        install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp DESTINATION imgui/misc)
        install(DIRECTORY ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype DESTINATION imgui/misc)
    endif()
endif()

if (HELLOIMGUI_USE_SDL_OPENGL3)
    # On desktop (linux, windows, osx) sdl is provided via vcpkg
    # On iOS, the build is done externally via tools/ios/sdl_compile_ios.sh
    if (ANDROID OR IOS)
        add_subdirectory(SDL)
    endif()
endif()
