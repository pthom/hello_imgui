vcpkg_check_linkage(ONLY_STATIC_LIBRARY) # this mirrors ImGui's portfile behavior

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO pthom/hello_imgui
    REF b95830dc7f4d6da9c7d278d21c9af03999b90f14
    SHA512 a7b83a3735975da9e117a775f722f183763fb8a706c43bb44190a74191a4618cdb5c6852973c1d429330354cc40438ff51602fc73eaa89a6d5a1ac5a392701eb
    HEAD_REF master
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
    "opengl3-binding" HELLOIMGUI_HAS_OPENGL3
    "metal-binding" HELLOIMGUI_HAS_METAL
    "experimental-vulkan-binding" HELLOIMGUI_HAS_VULKAN
    "experimental-dx11-binding" HELLOIMGUI_HAS_DIRECTX11
    "experimental-dx12-binding" HELLOIMGUI_HAS_DIRECTX12
    "glfw-binding" HELLOIMGUI_USE_GLFW3
    "sdl2-binding" HELLOIMGUI_USE_SDL2
    "freetype-lunasvg" HELLOIMGUI_USE_FREETYPE # When hello_imgui is built with freetype, it will also build with lunasvg
)


set(platform_options "")
if(WIN32)
    # Standard win32 options (these are the defaults for HelloImGui)
    # we could add a vcpkg feature for this, but it would have to be platform specific
    list(APPEND platform_options
        -DHELLOIMGUI_WIN32_NO_CONSOLE=ON
        -DHELLOIMGUI_WIN32_AUTO_WINMAIN=ON
    )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Standard macOS options (these are the defaults for HelloImGui)
    # we could add a vcpkg feature for this, but it would have to be platform specific
    list(APPEND platform_options
        -DHELLOIMGUI_MACOS_NO_BUNDLE=OFF
    )
endif()


vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DHELLOIMGUI_BUILD_DEMOS=OFF
        -DHELLOIMGUI_BUILD_DOCS=OFF
        -DHELLOIMGUI_BUILD_TESTS=OFF

        # vcpkg does not support ImGui Test Engine, so we cannot enable it
        -DHELLOIMGUI_WITH_TEST_ENGINE=OFF

        -DHELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE=ON
        -DHELLO_IMGUI_IMGUI_SHARED=OFF
        -DHELLOIMGUI_BUILD_IMGUI=OFF

        ${platform_options}

        # Rendering backends
        -DHELLOIMGUI_HAS_OPENGL3=${HELLOIMGUI_HAS_OPENGL3}
        -DHELLOIMGUI_HAS_METAL=${HELLOIMGUI_HAS_METAL}
        -DHELLOIMGUI_HAS_VULKAN=${HELLOIMGUI_HAS_VULKAN}
        -DHELLOIMGUI_HAS_DIRECTX11=${HELLOIMGUI_HAS_DIRECTX11}
        -DHELLOIMGUI_HAS_DIRECTX12=${HELLOIMGUI_HAS_DIRECTX12}

        # Platform backends
        -DHELLOIMGUI_USE_GLFW3=${HELLOIMGUI_USE_GLFW3}
        -DHELLOIMGUI_USE_SDL2=${HELLOIMGUI_USE_SDL2}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/hello_imgui PACKAGE_NAME "hello-imgui")  # should be active once himgui produces a config

file(REMOVE_RECURSE
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share"
    "${CURRENT_PACKAGES_DIR}/share/hello-imgui/hello_imgui_cmake/ios-cmake"
)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
