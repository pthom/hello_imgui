vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

#vcpkg_from_github(
#    OUT_SOURCE_PATH SOURCE_PATH
#    REPO pthom/hello_imgui
#    REF v1.3.0
#    SHA512 b601828303515a7385d9bbe9e959e0c62f1efa32a99d7e28b1213bbf0e90d51a9d6f8dc5fdda2f077a9fe3ffe9f8f7b9917385848f57c9ec641c3af7432bd03d
#    HEAD_REF master
#)

set(VCPKG_USE_HEAD_VERSION ON CACHE BOOL "" FORCE)
vcpkg_from_git(
    OUT_SOURCE_PATH SOURCE_PATH
    URL file:///Users/pascal/dvp/OpenSource/ImGuiWork/_Bundle/hello_imgui_vcpkg
    HEAD_REF vcpkg_packaging
#    REF c95e2bfdc26477929354daa2213394a972016a1b
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    "opengl3-binding" FEATURE_OPENGL3_BINDING
    "glfw-binding" FEATURE_GLFW_BINDING
    "sdl2-binding" FEATURE_SDL2_BINDING
)

message(STATUS "
    FEATURE_OPTIONS: ${FEATURE_OPTIONS}
    FEATURE_OPENGL3_BINDING: ${FEATURE_OPENGL3_BINDING}
    FEATURE_GLFW_BINDING: ${FEATURE_GLFW_BINDING}
    FEATURE_SDL2_BINDING: ${FEATURE_SDL2_BINDING}
")

if(FEATURE_OPENGL3_BINDING AND FEATURE_GLFW_BINDING)
    set(HELLOIMGUI_USE_GLFW_OPENGL3 ON)
endif()
if(FEATURE_OPENGL3_BINDING AND FEATURE_SDL2_BINDING)
    set(HELLOIMGUI_USE_SDL_OPENGL3 ON)
endif()


vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DHELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE=ON
        -DHELLOIMGUI_BUILD_DEMOS=OFF

        -DHELLOIMGUI_USE_GLFW_OPENGL3=${HELLOIMGUI_USE_GLFW_OPENGL3}
        -DHELLOIMGUI_USE_SDL_OPENGL3=${HELLOIMGUI_USE_SDL_OPENGL3}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/hello_imgui PACKAGE_NAME "hello-imgui")  # should be active once himgui produces a config

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/share/hello-imgui/hello_imgui_cmake/ios-cmake"
    "${CURRENT_PACKAGES_DIR}/debug/share/hello-imgui/hello_imgui_cmake/ios-cmake")
