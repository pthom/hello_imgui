###################################################################################################
#
# This file contains the main CMake logic that is used to build HelloImGui
#
# It is included by src/hello_imgui/CMakeLists.txt, which will call
#    him_main_add_hello_imgui_library()
# which is defined a the bottom of this file
#
###################################################################################################

# allows the linking of a target in a different directory
# Needed to add freetype to the imgui target, if it is build externally
cmake_policy(SET CMP0079 NEW)


###################################################################################################
# Store installable dependencies
###################################################################################################
function(him_add_installable_dependency dependency_name)
    set(HELLOIMGUI_INSTALLABLE_DEPENDENCIES ${HELLOIMGUI_INSTALLABLE_DEPENDENCIES} ${dependency_name} CACHE INTERNAL "" FORCE)
    message(STATUS "Added installable dependency ${dependency_name}, HELLOIMGUI_INSTALLABLE_DEPENDENCIES=${HELLOIMGUI_INSTALLABLE_DEPENDENCIES}")
endfunction()

function(him_reset_installable_dependencies)
    set(HELLOIMGUI_INSTALLABLE_DEPENDENCIES "" CACHE INTERNAL "" FORCE)
endfunction()


###################################################################################################
# Backend combinations handling: prefix him_back
###################################################################################################

function(him_back_available_platform_backends out_var)
    set(${out_var}
        HELLOIMGUI_USE_SDL2
        HELLOIMGUI_USE_GLFW3
        PARENT_SCOPE)
endfunction()

function(him_back_available_rendering_backends out_var)
    set(${out_var}
        HELLOIMGUI_HAS_OPENGL3
        HELLOIMGUI_HAS_METAL
        HELLOIMGUI_HAS_VULKAN
        HELLOIMGUI_HAS_DIRECTX11
        HELLOIMGUI_HAS_DIRECTX12
        PARENT_SCOPE)
endfunction()

function(him_back_parse_legacy_combinations)
    if(HELLOIMGUI_USE_SDL_OPENGL3)
        message(WARNING "
        HELLOIMGUI_USE_SDL_OPENGL3 is deprecated,
        use HELLOIMGUI_USE_SDL2 + HELLOIMGUI_HAS_OPENGL3 instead")
        set(HELLOIMGUI_USE_SDL2 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()
    if(HELLOIMGUI_USE_GLFW_OPENGL3)
        message(WARNING "
        HELLOIMGUI_USE_GLFW_OPENGL3 is deprecated,
        use HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_OPENGL3 instead")
        set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()

    if(HELLOIMGUI_USE_SDL_METAL)
        message(WARNING "
        HELLOIMGUI_USE_SDL_METAL is deprecated,
        use HELLOIMGUI_USE_SDL2 + HELLOIMGUI_HAS_METAL instead")
        set(HELLOIMGUI_USE_SDL2 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_METAL ON CACHE BOOL "" FORCE)
    endif()
    if(HELLOIMGUI_USE_GLFW_METAL)
        message(WARNING "
        HELLOIMGUI_USE_GLFW_METAL is deprecated,
        use HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_METAL instead")
        set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_METAL ON CACHE BOOL "" FORCE)
    endif()

    if(HELLOIMGUI_USE_GLFW_VULKAN)
        message(WARNING "
        HELLOIMGUI_USE_GLFW_VULKAN is deprecated,
        use HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_VULKAN instead")
        set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_VULKAN ON CACHE BOOL "" FORCE)
    endif()
    if(HELLOIMGUI_USE_SDL_VULKAN)
        message(WARNING "
        HELLOIMGUI_USE_SDL_VULKAN is deprecated,
        use HELLOIMGUI_USE_SDL2 + HELLOIMGUI_HAS_VULKAN instead")
        set(HELLOIMGUI_USE_SDL2 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_VULKAN ON CACHE BOOL "" FORCE)
    endif()

    if(HELLOIMGUI_USE_SDL_DIRECTX11)
        message(WARNING "
        HELLOIMGUI_USE_SDL_DIRECTX11 is deprecated,
        use HELLOIMGUI_USE_SDL2 + HELLOIMGUI_HAS_DIRECTX11 instead")
        set(HELLOIMGUI_USE_SDL2 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_DIRECTX11 ON CACHE BOOL "" FORCE)
    endif()
    if(HELLOIMGUI_USE_GLFW_DIRECTX11)
        message(WARNING "
        HELLOIMGUI_USE_GLFW_DIRECTX11 is deprecated,
        use HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_DIRECTX11 instead")
        set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_DIRECTX11 ON CACHE BOOL "" FORCE)
    endif()

    if(HELLOIMGUI_USE_SDL_DIRECTX12)
        message(WARNING "
        HELLOIMGUI_USE_SDL_DIRECTX12 is deprecated,
        use HELLOIMGUI_USE_SDL2 + HELLOIMGUI_HAS_DIRECTX12 instead")
        set(HELLOIMGUI_USE_SDL2 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_DIRECTX12 ON CACHE BOOL "" FORCE)
    endif()

    # Legacy options: HELLOIMGUI_WITH_SDL and HELLOIMGUI_WITH_GLFW
    if (HELLOIMGUI_WITH_SDL)
        message(WARNING "
        HELLOIMGUI_WITH_SDL is deprecated,
        use HELLOIMGUI_USE_SDL2 + HELLOIMGUI_HAS_OPENGL3 instead")
        set(HELLOIMGUI_USE_SDL2 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()
    if (HELLOIMGUI_WITH_GLFW)
        message(WARNING "
        HELLOIMGUI_WITH_GLFW is deprecated,
        use HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_OPENGL3 instead")
        set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
    endif()

endfunction()

function(him_back_check_if_no_platform_backend_selected out_var) # will set out_result to ON if no platform backend selected
    set(result ON)
    him_back_available_platform_backends(available_backends)
    foreach(backend ${available_backends})
        if(${backend})
            set(result OFF)
        endif()
    endforeach()
    set(${out_var} ${result} PARENT_SCOPE)
endfunction()

function(him_back_check_if_no_rendering_backend_selected out_var) # will set out_result to ON if no rendering backend selected
    set(result ON)
    him_back_available_rendering_backends(available_backends)
    foreach(backend ${available_backends})
        if(${backend})
            set(result OFF)
        endif()
    endforeach()
    set(${out_var} ${result} PARENT_SCOPE)
endfunction()

function(him_back_check_if_no_backend_option_chosen out_var)
    if (HELLOIMGUI_HEADLESS)
        set(${out_var} OFF PARENT_SCOPE)
        return()
    endif()
    him_back_check_if_no_platform_backend_selected(no_platform_selected)
    him_back_check_if_no_rendering_backend_selected(no_rendering_selected)
    if(no_platform_selected AND no_rendering_selected)
        set(${out_var} ON PARENT_SCOPE)
    else()
        set(${out_var} OFF PARENT_SCOPE)
    endif()
endfunction()

function(him_back_check_at_least_one_rendering_one_platform_backend out_var)
    if (HELLOIMGUI_HEADLESS)
        set(${out_var} ON PARENT_SCOPE)
        return()
    endif()
    him_back_check_if_no_platform_backend_selected(no_platform_selected)
    him_back_check_if_no_rendering_backend_selected(no_rendering_selected)
    if(no_platform_selected OR no_rendering_selected)
        set(${out_var} OFF PARENT_SCOPE)
    else()
        set(${out_var} ON PARENT_SCOPE)
    endif()
endfunction()

function(him_back_describe_active_rendering_backends out_description)
    set(result "")
    him_back_available_rendering_backends(available_backends)
    foreach(backend ${available_backends})
        if (${backend})
            set(result "${result} ${backend}")
        endif()
    endforeach()

    if (HELLOIMGUI_HEADLESS)
        set(result "${result} null-headless")
    endif()

    set(${out_description} ${result} PARENT_SCOPE)
endfunction()

function(him_back_describe_active_platform_backends out_description)
    set(result "")
    him_back_available_platform_backends(available_backends)
    foreach(backend ${available_backends})
        if (${backend})
            set(result "${result} ${backend}")
        endif()
    endforeach()

    if (HELLOIMGUI_HEADLESS)
        set(result "${result} null-headless")
    endif()

    set(${out_description} ${result} PARENT_SCOPE)
endfunction()


###################################################################################################
# Add library and sources: API = him_add_hello_imgui
###################################################################################################
function(him_add_hello_imgui)
    file(GLOB_RECURSE sources
        ${CMAKE_CURRENT_LIST_DIR}/*.h
        ${CMAKE_CURRENT_LIST_DIR}/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/*.c)
    if (APPLE)
        file(GLOB_RECURSE sources_mm ${CMAKE_CURRENT_LIST_DIR}/*.mm)
        set(sources ${sources} ${sources_mm})
    endif()
    add_library(${HELLOIMGUI_TARGET} ${sources})
    if(APPLE AND NOT IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_MACOS)
    endif()
    if(APPLE)
        target_compile_options(${HELLOIMGUI_TARGET} PRIVATE "-x" "objective-c++")
    endif()

    target_include_directories(${HELLOIMGUI_TARGET} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)

    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC stb_hello_imgui)
    if (HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        find_package(imgui CONFIG REQUIRED)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC imgui::imgui)
    else()
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC imgui)
    endif()

    add_library(hello-imgui::hello_imgui ALIAS hello_imgui)
    him_add_installable_dependency(${HELLOIMGUI_TARGET})
endfunction()


###################################################################################################
# Build imgui: API = him_build_imgui + him_install_imgui (to be called at the end)
###################################################################################################
function(him_build_imgui)
    # check that HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE is on if a package is found
    find_package(imgui CONFIG QUIET)
    if(imgui_FOUND AND NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        message(FATAL_ERROR "
            imgui is found via find_package(imgui), but HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE is OFF.
            You should either
                - set -DHELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE=ON (this will use the imgui CMake package)
                - or uninstall the imgui package (e.g. vcpkg remove imgui)
        ")
    endif()

    message(STATUS "HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE is ${HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE}")
    if (HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        set(HELLOIMGUI_BUILD_IMGUI OFF CACHE BOOL "" FORCE)
        find_package(imgui CONFIG REQUIRED)
    else()
        if (HELLOIMGUI_BUILD_IMGUI)
            _him_checkout_imgui_submodule_if_needed()
            _him_do_build_imgui()
        endif()
        if (HELLOIMGUI_USE_FREETYPE)
            _him_add_freetype_to_imgui()
        endif()
    endif()
endfunction()

function(him_install_imgui)
    if(HELLOIMGUI_INSTALL)
        if(NOT TARGET imgui)
            return()
        endif()

        install(TARGETS imgui DESTINATION ./lib/)
        file(GLOB imgui_headers
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/*.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp/*.h
        )
        install(FILES ${imgui_headers} DESTINATION include)

        if(HELLOIMGUI_HAS_OPENGL3)
            install(FILES ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.h DESTINATION include)
        endif()
        if(HELLOIMGUI_HAS_METAL)
            install(FILES ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_metal.h DESTINATION include)
        endif()
        if(HELLOIMGUI_HAS_VULKAN)
            install(FILES ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.h DESTINATION include)
        endif()
        if(HELLOIMGUI_HAS_DIRECTX11)
            install(FILES ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx11.h DESTINATION include)
        endif()

        if(HELLOIMGUI_USE_SDL2)
            install(FILES ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.h DESTINATION include)
        endif()
        if(HELLOIMGUI_USE_GLFW3)
            install(FILES ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.h DESTINATION include)
        endif()

        if(HELLOIMGUI_USE_FREETYPE)
            install(FILES ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype/imgui_freetype.h DESTINATION include)
        endif()
    endif()
endfunction()


function(_him_checkout_imgui_submodule_if_needed)
    if (HELLOIMGUI_BUILD_IMGUI)
        # if HELLOIMGUI_IMGUI_SOURCE_DIR is  CMAKE_CURRENT_LIST_DIR/imgui
        # and the submodule is not present, update submodules
        if (HELLOIMGUI_IMGUI_SOURCE_DIR STREQUAL ${HELLOIMGUI_BASEPATH}/external/imgui)
            if (NOT EXISTS ${HELLOIMGUI_IMGUI_SOURCE_DIR}/imgui.h)
                # Run git submodule update --init --recursive
                message(WARNING "Updating imgui submodule")
                execute_process(
                    COMMAND git submodule update --init --recursive
                    WORKING_DIRECTORY ${HELLOIMGUI_BASEPATH})
            endif()
        endif()
    endif()
endfunction()

function(_him_do_build_imgui)
    file(GLOB imgui_sources ${HELLOIMGUI_IMGUI_SOURCE_DIR}/*.h ${HELLOIMGUI_IMGUI_SOURCE_DIR}/*.cpp)
    set(imgui_sources ${imgui_sources}
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp/imgui_stdlib.h)
    if (HELLO_IMGUI_IMGUI_SHARED)
        add_library(imgui SHARED ${imgui_sources})
    else()
        add_library(imgui ${imgui_sources})
    endif()
    target_include_directories(imgui PUBLIC
        $<BUILD_INTERFACE:${HELLOIMGUI_IMGUI_SOURCE_DIR}>
        $<BUILD_INTERFACE:${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends>
        $<BUILD_INTERFACE:${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/cpp>
        $<BUILD_INTERFACE:${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype>
    )
    him_add_installable_dependency(imgui)
    hello_imgui_msvc_target_set_folder(imgui ${HELLOIMGUI_SOLUTIONFOLDER}/external)
endfunction()

function(_him_add_freetype_to_imgui)
    # Add freetype + lunasvg to imgui
    # This is especially useful to support emojis (possibly colored) in imgui
    # See doc:
    #     https://github.com/ocornut/imgui/blob/master/docs/FONTS.md#using-colorful-glyphsemojis
    # We have to
    # - add imgui_freetype.cpp and imgui_freetype.h to imgui
    # - enable freetype in imgui via IMGUI_ENABLE_FREETYPE
    # - enable lunasvg in imgui via IMGUI_ENABLE_FREETYPE_LUNASVG
    # - add lunasvg to imgui
    # - define IMGUI_USE_WCHAR32 in imgui

    # Note: also change add_imgui.cmake in bundle!

    #
    # 1. Build or find freetype (if downloaded, make sure it is static)
    #
    if(TARGET freetype)
        message(STATUS "HelloImGui: using freetype target")
        set(freetype_linked_library freetype)
    else()
        set(download_freetype OFF)
        if (HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED AND NOT HELLOIMGUI_FETCH_FORBIDDEN)
            find_package(Freetype 2.12 QUIET)
            if (NOT Freetype_FOUND)
                set(download_freetype ON)
            else()
                message(STATUS "Found system lib Freetype")
            endif()
        endif()
        if(HELLOIMGUI_FREETYPE_STATIC AND NOT HELLOIMGUI_FETCH_FORBIDDEN)
            message("Forcing download of Freetype because HELLOIMGUI_FREETYPE_STATIC is ON")
            set(download_freetype ON)
        endif()

        if (download_freetype)
            message(STATUS "HelloImGui: downloading and building freetype")

            set(backup_shared_lib ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

            if (NOT HELLOIMGUI_INSTALL)
                set(SKIP_INSTALL_ALL ON CACHE INTERNAL "" FORCE) # disable Freetype install
            endif()

            include(FetchContent)
            if(IOS)
                set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
                set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
            endif()
            FetchContent_Declare(
                freetype
                GIT_REPOSITORY https://github.com/freetype/freetype.git
                GIT_TAG        VER-2-13-2
                GIT_PROGRESS TRUE
            )
            FetchContent_MakeAvailable(freetype)
            set(freetype_linked_library freetype)
            hello_imgui_msvc_target_set_folder(freetype ${HELLOIMGUI_SOLUTIONFOLDER}/external)

            set(BUILD_SHARED_LIBS ${backup_shared_lib} CACHE BOOL "" FORCE)
        else()
            find_package(Freetype 2.12 QUIET)
            if(NOT Freetype_FOUND AND NOT HELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED)
                message(STATUS "
                    HelloImGui: freetype not found. You may set
                        -DHELLOIMGUI_DOWNLOAD_FREETYPE_IF_NEEDED=ON
                    to download and build freetype automatically
                ")
            endif()
            find_package(Freetype 2.12 REQUIRED)
            set(freetype_linked_library Freetype::Freetype)
        endif()
    endif()

    target_link_libraries(imgui PUBLIC ${freetype_linked_library})

    if(TARGET freetype)
        set(HELLOIMGUI_FREETYPE_SELECTED_INFO "Use target freetype" CACHE INTERNAL "" FORCE)
    elseif(download_freetype)
        set(HELLOIMGUI_FREETYPE_SELECTED_INFO "Downloaded VER-2-13-2" CACHE INTERNAL "" FORCE)
    else()
        set(HELLOIMGUI_FREETYPE_SELECTED_INFO "Use system Library" CACHE INTERNAL "" FORCE)
    endif()

    #
    # 2. Build lunasvg (static)
    #
    # Fetch and build lunasvg
    if(NOT TARGET lunasvg)
        # Try using lunasvg unofficial package from vcpkg
        find_package(unofficial-lunasvg CONFIG QUIET)
        if(unofficial-lunasvg_FOUND)
            target_link_libraries(imgui PRIVATE unofficial::lunasvg::lunasvg)
        elseif(NOT HELLOIMGUI_FETCH_FORBIDDEN)
            set(backup_shared_lib ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
            include(FetchContent)
            FetchContent_Declare(lunasvg
                GIT_REPOSITORY https://github.com/sammycage/lunasvg
                GIT_TAG        v2.3.9
                GIT_PROGRESS TRUE
            )
            FetchContent_MakeAvailable(lunasvg)
            set(BUILD_SHARED_LIBS ${backup_shared_lib} CACHE BOOL "" FORCE)
            target_link_libraries(imgui PUBLIC lunasvg)
            get_target_property(lunasvg_include_dirs lunasvg INTERFACE_INCLUDE_DIRECTORIES)
            # Patch lunasvg include dir, for installable version (CMake install shenanigans)
            set_target_properties(lunasvg PROPERTIES INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/_deps/lunasvg-src/include>)
            get_target_property(lunasvg_include_dirs lunasvg INTERFACE_INCLUDE_DIRECTORIES)

            him_add_installable_dependency(lunasvg)
            hello_imgui_msvc_target_set_folder(lunasvg ${HELLOIMGUI_SOLUTIONFOLDER}/external)
        endif()
    endif()

    #
    # 3. Add freetype and LunaSvg support to imgui
    #    with support for wchar32 (for emojis, and other unicode characters)
    target_sources(imgui PRIVATE
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
        ${HELLOIMGUI_IMGUI_SOURCE_DIR}/misc/freetype/imgui_freetype.h)
    target_compile_definitions(imgui PUBLIC IMGUI_ENABLE_FREETYPE IMGUI_ENABLE_FREETYPE_LUNASVG)
    target_compile_definitions(imgui PUBLIC IMGUI_USE_WCHAR32)
endfunction()


###################################################################################################
# Sanity checks: API = him_sanity_checks + him_get_active_backends (displayed after configure)
###################################################################################################
function(him_sanity_checks)
    him_back_check_if_no_backend_option_chosen(no_backend_option_chosen)
    _him_check_fetch_forbidden() # may set HELLOIMGUI_FETCH_FORBIDDEN

    if (NOT WIN32)
        set(HELLOIMGUI_HAS_DIRECTX11 OFF CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_DIRECTX12 OFF CACHE BOOL "" FORCE)
    endif()
    if (NOT APPLE)
        set(HELLOIMGUI_HAS_METAL OFF CACHE BOOL "" FORCE)
    endif()

    if (no_backend_option_chosen AND NOT HELLOIMGUI_USING_VCPKG_TOOLCHAIN)
        # use SDL for emscripten and iOS, GLFW for the rest
        if (EMSCRIPTEN OR IOS)
            set(HELLOIMGUI_USE_SDL2 ON CACHE BOOL "" FORCE)
            set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
        else()
            _him_try_select_glfw_opengl3_if_no_backend_selected()
        endif()
    endif()

    him_back_check_at_least_one_rendering_one_platform_backend(ok_backend)
    if (NOT ok_backend AND NOT HELLOIMGUI_USING_VCPKG_TOOLCHAIN)
        him_back_available_platform_backends(platform_backends)
        him_back_available_rendering_backends(rendering_backends)
        message(FATAL_ERROR "
            HelloImGui: no rendering or platform backend selected!
            Please select at least one of the following:
                - rendering backend: ${rendering_backends}
                - platform backend: ${platform_backends}
        ")
    endif()
endfunction()

function(_him_check_fetch_forbidden)
    set(HELLOIMGUI_FETCH_FORBIDDEN OFF CACHE INTERNAL "" FORCE)
    if(CMAKE_TOOLCHAIN_FILE)
        if("${CMAKE_TOOLCHAIN_FILE}" MATCHES "vcpkg.cmake")
            set(HELLOIMGUI_FETCH_FORBIDDEN ON CACHE INTERNAL "" FORCE)
            set(HELLOIMGUI_DEFAULT_BACKEND_FORBIDDEN ON CACHE INTERNAL "" FORCE)
            set(HELLOIMGUI_USING_VCPKG_TOOLCHAIN ON CACHE INTERNAL "" FORCE)
        endif()
    endif()
    if(HELLOIMGUI_FETCH_FORBIDDEN)
        message(STATUS "HelloImGui: Fetching is forbidden")
    endif()
endfunction()


function(_him_try_select_glfw_opengl3_if_no_backend_selected)
    #------------------------------------------------------------------------------
    # Backend check: If no backend option is selected,
    # either select Glfw automatically if possible, or fail
    #------------------------------------------------------------------------------
    #
    if (HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED)
        set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
        set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
        message(STATUS "HelloImGui: using HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_OPENGL3 as default backend combination")
    else()
        # Check if Glfw can be found
        find_package(glfw3 QUIET)
        if (glfw3_FOUND)
            set(HELLOIMGUI_USE_GLFW3 ON CACHE BOOL "" FORCE)
            set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)
            message(STATUS
                "HelloImGui: using HELLOIMGUI_USE_GLFW3 + HELLOIMGUI_HAS_OPENGL3 as default backend combination (glfw was found via find_package(glfw3))
                ")
        else()
            set(glfw_help_msg "
            you can install glfw via your package manager (apt, pacman, etc).
            For example, on Ubuntu, you can run:
                sudo apt install libglfw3-dev
            on macOS you can run:
                brew install glfw3
        ")
            message(FATAL_ERROR "
                    HelloImGui: no backend selected, and could not find Glfw via find_package(glfw3).
                    ${backend_message} ${glfw_help_msg}
            ")
        endif()
    endif()
endfunction()


###################################################################################################
# stb_image: API = him_add_stb_image
###################################################################################################
function(him_add_stb_image)
    find_package(Stb QUIET)

    # Add stb for HelloImGui
    set(stb_dir ${HELLOIMGUI_BASEPATH}/external/stb_hello_imgui)
    add_library(stb_hello_imgui STATIC ${stb_dir}/stb_impl_hello_imgui.cpp)
    hello_imgui_msvc_target_set_folder(stb_hello_imgui ${HELLOIMGUI_SOLUTIONFOLDER}/external)

    if(Stb_FOUND)
        message(STATUS "HelloImGui: using stb from find_package(Stb)")
        target_include_directories(stb_hello_imgui PUBLIC $<BUILD_INTERFACE:${Stb_INCLUDE_DIR}>)
    else()
        target_include_directories(stb_hello_imgui PUBLIC $<BUILD_INTERFACE:${HELLOIMGUI_BASEPATH}/external/stb_hello_imgui>)
    endif()

    if(HELLOIMGUI_STB_IMAGE_IMPLEMENTATION)
        target_compile_definitions(stb_hello_imgui PRIVATE STB_IMAGE_IMPLEMENTATION)
    endif()
    if(HELLOIMGUI_STB_IMAGE_WRITE_IMPLEMENTATION)
        target_compile_definitions(stb_hello_imgui PRIVATE STB_IMAGE_WRITE_IMPLEMENTATION)
    endif()

    if(HELLOIMGUI_INSTALL AND NOT Stb_FOUND)
        file(GLOB stb_headers ${stb_dir}/*.h)
        install(FILES ${stb_headers} DESTINATION include)
    endif()

    # This is always installed, since it might use STB_IMAGE_IMPLEMENTATION or STB_IMAGE_WRITE_IMPLEMENTATION
    him_add_installable_dependency(stb_hello_imgui)
endfunction()

###################################################################################################
# Apple related options: API = him_add_apple_options
###################################################################################################
function(him_add_apple_options)
    _him_link_apple_frameworks()
    _him_add_ios_compile_definitions()
    _him_add_app_bundle_compile_definitions()
endfunction()

function(_him_link_apple_frameworks)
    if (IOS)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC "-framework UIKit")
    endif ()
    if (APPLE AND NOT IOS) # If mac
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC "-framework AppKit -framework IOKit")
    endif()
    if (APPLE)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC "-framework Foundation")
    endif()
endfunction()

function(_him_add_ios_compile_definitions)
    if (IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_IOS)
        # this is a hack because imgui_impl_opengl3.cpp does not include <TargetConditional.h>
        # and thus TARGET_OS_IOS is not found as it should be
        target_compile_definitions(${HELLOIMGUI_TARGET} PRIVATE TARGET_OS_IOS)
    endif()
endfunction()

function(_him_add_app_bundle_compile_definitions)
    if (IOS OR (MACOSX AND NOT HELLOIMGUI_MACOS_NO_BUNDLE))
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_INSIDE_APPLE_BUNDLE)
    endif()
endfunction()


###################################################################################################
# Linux related options: API = him_add_linux_options
###################################################################################################
function(him_add_linux_options)
    if (UNIX AND NOT EMSCRIPTEN  AND NOT APPLE AND NOT ANDROID)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC stdc++ dl)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC X11)
    endif()
endfunction()


###################################################################################################
# Windows related options: API = him_add_windows_options
###################################################################################################
function(him_add_windows_options)
    _him_msvc_group_sources()
    _him_win_add_auto_win_main()
endfunction()

function(_him_msvc_group_sources)
    hello_imgui_msvc_target_group_sources(${HELLOIMGUI_TARGET})
    hello_imgui_msvc_target_set_folder(${HELLOIMGUI_TARGET} ${HELLOIMGUI_SOLUTIONFOLDER})
endfunction()

function(_him_win_add_auto_win_main)
    if (HELLOIMGUI_WIN32_AUTO_WINMAIN)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_WIN32_AUTO_WINMAIN)
    endif()
endfunction()


###################################################################################################
# Mobile related options (i.e. iOS & Android) : API = him_add_mobile_options
###################################################################################################
function(him_add_mobile_options)
    _him_add_mobile_compile_definitions()
endfunction()

function(_him_add_mobile_compile_definitions)
    if(ANDROID OR IOS)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_CANNOTQUIT)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_MOBILEDEVICE)
    endif()
endfunction()


###################################################################################################
# Android related options: API = him_add_android_options
###################################################################################################
function(him_add_android_options)
    if (ANDROID)
        # Empty for now (we still call add_mobile_options in main)
    endif()
endfunction()


###################################################################################################
# Emscripten related options: API = him_add_emscripten_options
###################################################################################################
function(him_add_emscripten_options)
    if (EMSCRIPTEN)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLES3)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_CANNOTQUIT)
        target_link_options(${HELLOIMGUI_TARGET} PUBLIC -sMAX_WEBGL_VERSION=2)
    endif()
endfunction()


###################################################################################################
# OpenGL Rendering backend: API = him_has_opengl3_backend
###################################################################################################
function(him_has_opengl3 target)
    # vcpkg will have added those files to imgui
    if (NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        target_sources(${target} PRIVATE
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
    endif()
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_OPENGL)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_OPENGL3)
    set(HELLOIMGUI_HAS_OPENGL ON CACHE BOOL "" FORCE)
    set(HELLOIMGUI_HAS_OPENGL3 ON CACHE BOOL "" FORCE)

    if(ANDROID OR IOS)
        _him_link_opengl_es_sdl(${HELLOIMGUI_TARGET})
    endif()

    if (IOS OR EMSCRIPTEN)
        set(HELLOIMGUI_USE_GLAD OFF CACHE INTERNAL "" FORCE)
    else()
        set(HELLOIMGUI_USE_GLAD ON CACHE INTERNAL "" FORCE)
    endif()

    if (HELLOIMGUI_USE_GLAD)
        target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLAD IMGUI_IMPL_OPENGL_LOADER_GLAD)
        _him_add_glad()
    endif()
endfunction()

function(_him_link_opengl_es_sdl target)
    if(IOS)
        target_link_libraries(${target} PUBLIC "-framework OpenGLES")
    elseif(ANDROID)
        target_link_libraries(${target} PUBLIC GLESv3)
    endif()

    target_compile_definitions(${target}
        PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_CUSTOM="<OpenGLES/ES3/gl.h>"
        IMGUI_IMPL_OPENGL_ES3
        HELLOIMGUI_USE_GLES3
    )
endfunction()

function(_him_add_glad)
    if(TARGET glad)
        return()
    endif()

    find_package(glad CONFIG QUIET)
    if(glad_FOUND)
        message(STATUS "HelloImGui: using glad from find_package(glad)")
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC glad::glad)
        return()
    endif()

    set(glad_dir ${HELLOIMGUI_BASEPATH}/external/OpenGL_Loaders/glad)
    set(glad_files
        ${glad_dir}/src/glad.c
        ${glad_dir}/include/glad/glad.h
        ${glad_dir}/include/KHR/khrplatform.h)
    add_library(glad ${glad_files})
    target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${glad_dir}/include>)

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

    hello_imgui_msvc_target_set_folder(glad ${HELLOIMGUI_SOLUTIONFOLDER}/external/OpenGL_Loaders)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC glad)

    him_add_installable_dependency(glad)
    if(HELLOIMGUI_INSTALL)
        install(TARGETS glad DESTINATION ./lib/)
        install(FILES ${glad_dir}/include/glad/glad.h DESTINATION include/glad)
        install(FILES ${glad_dir}/include/KHR/khrplatform.h DESTINATION include/KHR)
    endif()
endfunction()



###################################################################################################
# Metal Rendering backend: API = him_has_metal_backend
###################################################################################################
function(him_has_metal target)
    if (NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        target_sources(${target} PRIVATE
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_metal.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_metal.mm
        )
    endif()
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_METAL)
    set(HELLOIMGUI_HAS_METAL ON CACHE BOOL "" FORCE)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC
        "-framework Metal -framework MetalKit -framework QuartzCore")
endfunction()


###################################################################################################
# Vulkan Rendering backend: API = him_has_vulkan
###################################################################################################
function(him_has_vulkan target)
    if (NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        target_sources(${target} PRIVATE
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        )
    endif()
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_VULKAN)
    set(HELLOIMGUI_HAS_VULKAN ON CACHE BOOL "" FORCE)
    find_package(Vulkan REQUIRED)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC Vulkan::Vulkan)
endfunction()


###################################################################################################
# DirectX11 Rendering backend: API = him_has_directx11
###################################################################################################
function(him_has_directx11 target)
    if (NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        target_sources(${target} PRIVATE
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx11.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
        )
    endif()
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_DIRECTX11)
    set(HELLOIMGUI_HAS_DIRECTX11 ON CACHE BOOL "" FORCE)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC d3d11.lib)
endfunction()


###################################################################################################
# DirectX12 Rendering backend: API = him_has_directx12
###################################################################################################
function(him_has_directx12 target)
    if (NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        target_sources(${target} PRIVATE
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx12.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
        )
    endif()
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_HAS_DIRECTX12)
    set(HELLOIMGUI_HAS_DIRECTX12 ON CACHE BOOL "" FORCE)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC d3d12.lib dxgi.lib)
endfunction()


###################################################################################################
# SDL platform backend: API = him_use_sdl2_backend
###################################################################################################
function (him_use_sdl2_backend target)
    _him_fetch_sdl_if_needed()
    #    _him_fail_if_sdl_not_found()
    _him_link_sdl(${HELLOIMGUI_TARGET})

    # vcpkg will have added those files to imgui
    if (NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        target_sources(${target} PRIVATE
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
        )
    endif()
    set(HELLOIMGUI_USE_SDL2 ON CACHE INTERNAL "" FORCE)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL2)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_SDL)  # legacy!
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC _THREAD_SAFE) # flag outputted by sdl2-config --cflags
endfunction()

function(_him_fetch_sdl_if_needed)
    set(shall_fetch_sdl OFF)

    # Always fetch SDL for iOS and Android
    if (HELLOIMGUI_USE_SDL2 AND (IOS OR ANDROID))
        set(shall_fetch_sdl ON)
    endif()

    # Fetch SDL if:
    # option HELLOIMGUI_DOWNLOAD_SDL_IF_NEEDED was passed
    # and SDL not available by find_package
    # and HELLOIMGUI_USE_SDL_SYSTEM_LIB is OFF
    if (HELLOIMGUI_DOWNLOAD_SDL_IF_NEEDED AND NOT TARGET sdl AND NOT EMSCRIPTEN AND NOT HELLOIMGUI_FETCH_FORBIDDEN)
        find_package(SDL2 QUIET)
        if (NOT SDL2_FOUND)
            set(shall_fetch_sdl ON)
        else()
            message(STATUS "Found system lib SDL2")
        endif()
    endif()

    if (shall_fetch_sdl)
        set(SDL2_DISABLE_INSTALL OFF CACHE BOOL "" FORCE)
        _him_fetch_declare_sdl()
        if(ANDROID)
            FetchContent_Populate(sdl)
            _him_prepare_android_sdl_symlink()
        else()
            FetchContent_MakeAvailable(sdl)
        endif()
    else()
        set(HELLOIMGUI_SDL_SELECTED_INFO "Use system Library" CACHE INTERNAL "" FORCE)
    endif()
endfunction()

function(_him_fetch_declare_sdl)
    # iOS and Android were tested with SDL 2.28.5
    # other platforms, not yet
    if (IOS OR ANDROID)
        set(sdl_version 2.28.5)
    else()
        set(sdl_version 2.24.2)
    endif()

    message(STATUS "Fetching SDL version ${sdl_version}")
    include(FetchContent)
    # Set(FETCHCONTENT_QUIET FALSE)
    FetchContent_Declare(sdl
        GIT_REPOSITORY    https://github.com/libsdl-org/SDL.git
        GIT_TAG           release-${sdl_version}
        GIT_PROGRESS TRUE
    )
    set(HELLOIMGUI_SDL_SELECTED_INFO "Downloaded ${sdl_version}" CACHE INTERNAL "" FORCE)
endfunction()

function(_him_prepare_android_sdl_symlink)
    # We now have SDL in _deps/sdl-src
    set(sdl_location ${CMAKE_BINARY_DIR}/_deps/sdl-src)
    # We need to communicate this location to the function apkCMake_makeSymLinks()
    # so that it can create the symlinks in the right place:
    # it will use the variable apkCMake_sdl_symlink_target
    set(apkCMake_sdl_symlink_target ${CMAKE_BINARY_DIR}/_deps/sdl-src CACHE STRING "" FORCE)
endfunction()

function(_him_link_sdl target)
    if (need_fetch_make_available_sdl)
        FetchContent_MakeAvailable(sdl)
    endif()

    if(IOS)
        target_link_libraries(${target} PUBLIC SDL2-static SDL2main)
    elseif(EMSCRIPTEN)
        target_compile_options(${target} PUBLIC -s USE_SDL=2)
        target_link_options(${target} INTERFACE -s USE_SDL=2)
    elseif(ANDROID)
        target_link_libraries(${target} PUBLIC SDL2main SDL2)
    elseif(TARGET SDL2-static)
        target_link_libraries(${target} PUBLIC SDL2-static)
    else()
        find_package(SDL2 REQUIRED)
        target_link_libraries(${target} PUBLIC SDL2::SDL2 SDL2::SDL2main)
    endif()

    if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
        target_link_libraries(${target} PUBLIC Xext X11)
    endif()
endfunction()


###################################################################################################
# Glfw platform backend: API = him_use_glfw_backend
###################################################################################################
function(him_use_glfw3_backend target)
    _him_fetch_glfw_if_needed()
    if (NOT TARGET glfw) # if glfw is not built as part of the whole build, find it
        find_package(glfw3 CONFIG REQUIRED)
    endif()
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC glfw)

    # vcpkg will have added those files to imgui
    if (NOT HELLOIMGUI_USE_IMGUI_CMAKE_PACKAGE)
        target_sources(${target} PRIVATE
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.h
            ${HELLOIMGUI_IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        )
    endif()
    set(HELLOIMGUI_USE_GLFW3 ON CACHE INTERNAL "" FORCE)
    set(HELLOIMGUI_USE_GLFW3 ON CACHE INTERNAL "" FORCE)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW3)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_USE_GLFW) # legacy!
endfunction()

function(_him_fetch_glfw_if_needed)
    set(shall_fetch_glfw OFF)
    if (HELLOIMGUI_DOWNLOAD_GLFW_IF_NEEDED AND NOT TARGET glfw AND NOT HELLOIMGUI_FETCH_FORBIDDEN)
        find_package(glfw3 QUIET)
        if (NOT glfw3_FOUND)
            set(shall_fetch_glfw ON)
        else()
            message(STATUS "Found system lib glfw3")
        endif()
    endif()

    if (shall_fetch_glfw)
        message(STATUS "HelloImGui: downloading and building glfw")
        include(FetchContent)
        # Set(FETCHCONTENT_QUIET FALSE)
        FetchContent_Declare(glfw
            GIT_REPOSITORY    https://github.com/glfw/glfw.git
            GIT_TAG           3.3.8
            GIT_PROGRESS TRUE
        )

        set(GLFW_BUILD_EXAMPLES OFF)
        set(GLFW_BUILD_TESTS OFF)
        set(GLFW_BUILD_DOCS OFF)
        set(GLFW_INSTALL ON)
        FetchContent_MakeAvailable(glfw)
        set(HELLOIMGUI_GLFW_SELECTED_INFO "Downloaded 3.3.8" CACHE INTERNAL "" FORCE)
    else()
        set(HELLOIMGUI_GLFW_SELECTED_INFO "Use system Library" CACHE INTERNAL "" FORCE)
    endif()

endfunction()


###################################################################################################
# Remoting with https://github.com/sammyfreg/netImgui: API = him_with_netimgui
###################################################################################################
function(him_with_netimgui)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_WITH_NETIMGUI)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_WITH_REMOTE_DISPLAY)

#    message(STATUS "HelloImGui: downloading and building netImgui")
#    include(FetchContent)
#    # Set(FETCHCONTENT_QUIET FALSE)
#    FetchContent_Declare(net_imgui
#        GIT_REPOSITORY    https://github.com/pthom/netImgui.git
#        GIT_TAG           cmake_multiplatform
#        GIT_PROGRESS TRUE
#    )
#    FetchContent_MakeAvailable(net_imgui)

    # Add netImgui to the project
    set(NETIMGUI_DIR ${HELLOIMGUI_BASEPATH}/external/netImgui CACHE STRING "" FORCE)
    set(NETIMGUI_BUILD_IMGUI OFF CACHE BOOL "" FORCE)
    set(NETIMGUI_BUILD_CLIENT ON CACHE BOOL "" FORCE)
    set(NETIMGUI_BUILD_SERVER_LIB ON CACHE BOOL "" FORCE)

    #set(NETIMGUI_BUILD_SERVER_APP_SOKOL ON CACHE BOOL "" FORCE)
    set(NETIMGUI_BUILD_SERVER_APP_SOKOL OFF CACHE BOOL "" FORCE)
    set(NETIMGUI_SERVER_APP_BACKEND_GLFW_GL3 ON CACHE BOOL "" FORCE)

    set(NETIMGUI_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${NETIMGUI_DIR}  netimgui)

    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC net_imgui_client)
    him_add_installable_dependency(net_imgui_client)
endfunction()


###################################################################################################
# Remoting with https://github.com/ggerganov/imgui-ws: API = him_with_imguiws
###################################################################################################
function(him_with_imguiws)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_WITH_IMGUIWS)
    target_compile_definitions(${HELLOIMGUI_TARGET} PUBLIC HELLOIMGUI_WITH_REMOTE_DISPLAY)

    # Add imgui-ws to the project
    set(IMGUIWS_DIR ${HELLOIMGUI_BASEPATH}/external/imgui-ws CACHE STRING "" FORCE)
    add_subdirectory(${IMGUIWS_DIR} imgui-ws)
    target_include_directories(imgui-ws PRIVATE $<BUILD_INTERFACE:${HELLOIMGUI_IMGUI_SOURCE_DIR}/..>)
    target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC imgui-ws)
    target_compile_definitions(imgui PUBLIC "ImDrawIdx=unsigned int")
    set(HELLOIMGUI_INSTALL OFF CACHE BOOL "" FORCE)  # imgui-ws is not installable
endfunction()

###################################################################################################
# Miscellaneous: API = him_add_misc_options
###################################################################################################
function(him_add_misc_options)
    _him_add_imgui_shared_compile_definitions()
endfunction()

function(_him_add_imgui_shared_compile_definitions)
    if (HELLO_IMGUI_IMGUI_SHARED)
        target_compile_definitions(${HELLOIMGUI_TARGET} PRIVATE GImGui=GImGuiFromHelloImGui)
        target_compile_definitions(${HELLOIMGUI_TARGET} PRIVATE HELLO_IMGUI_IMGUI_SHARED)
    endif()
endfunction()


###################################################################################################
# Install: API = him_install
###################################################################################################
function(him_install)
    if (HELLOIMGUI_INSTALL AND NOT IOS AND NOT ANDROID)
        install(TARGETS ${HELLOIMGUI_TARGET} DESTINATION lib/)
        file(GLOB headers *.h)
        install(FILES ${headers} DESTINATION include/hello_imgui/)
        file(GLOB internal_headers internal/*.h)
        install(FILES ${internal_headers} DESTINATION include/hello_imgui/internal)

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            install(DIRECTORY ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake DESTINATION share/${PROJECT_NAME})
            install(DIRECTORY ${HELLOIMGUI_BASEPATH}/hello_imgui_assets DESTINATION share/${PROJECT_NAME})
            if (NOT IOS AND NOT ANDROID)
                install(FILES ${HELLOIMGUI_BASEPATH}/README.md DESTINATION share/${PROJECT_NAME})
            endif()
        endif()
    endif()

endfunction()

###################################################################################################
# Add nlohmann_json: API = him_add_nlohmann_json
###################################################################################################
function(him_add_nlohmann_json)
    find_package(nlohmann_json CONFIG QUIET)
    if(nlohmann_json_FOUND)
        message(STATUS "HelloImGui: using nlohmann_json from find_package(nlohmann_json)")
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC nlohmann_json::nlohmann_json)
        set(HELLOIMGUI_NLOHMANN_JSON_SELECTED_INFO "Found via find_package(nlohmann_json)" CACHE INTERNAL "" FORCE)
    else()
        message(STATUS "HelloImGui: using nlohmann_json from external/nlohmann_json")
        set(nlohmann_json_dir ${HELLOIMGUI_BASEPATH}/external/nlohmann_json)
        add_library(nlohmann_json INTERFACE)
        target_include_directories(nlohmann_json INTERFACE $<BUILD_INTERFACE:${nlohmann_json_dir}>)
        # target_compile_definitions(nlohmann_json INTERFACE NLOHMANN_JSON_NOEXCEPTION)
        target_link_libraries(${HELLOIMGUI_TARGET} PUBLIC nlohmann_json)
        set(HELLOIMGUI_NLOHMANN_JSON_SELECTED_INFO "Using external/nlohmann_json" CACHE INTERNAL "" FORCE)

        him_add_installable_dependency(nlohmann_json)
        if(HELLOIMGUI_INSTALL)
            install(FILES ${nlohmann_json_dir}/nlohmann/json.hpp DESTINATION include/nlohmann/json.hpp)
            install(FILES ${nlohmann_json_dir}/nlohmann/json_fwd.hpp DESTINATION include/nlohmann/json_fwd.hpp)
        endif()
    endif()
endfunction()


###################################################################################################
# Log Configuration at the end of the configuration: API = him_log_configuration
###################################################################################################
function(him_log_configuration)
    him_back_describe_active_platform_backends(active_platform_backends)
    him_back_describe_active_rendering_backends(active_rendering_backends)

    # set imgui_source_dir to the relative path of HELLOIMGUI_IMGUI_SOURCE_DIR versus this project
    file(RELATIVE_PATH imgui_source_dir ${HELLOIMGUI_BASEPATH} ${HELLOIMGUI_IMGUI_SOURCE_DIR})

    # Use netImGui:                     ${HELLOIMGUI_WITH_NETIMGUI}

    set(msg "
    ===========================================================================
        Hello ImGui build options:
    ===========================================================================
      Platform Backend(s):             ${active_platform_backends}
      Rendering Backend(s):            ${active_rendering_backends}
    ---------------------------------------------------------------------------
      Options:
        HELLOIMGUI_USE_FREETYPE:        ${HELLOIMGUI_USE_FREETYPE}  (${HELLOIMGUI_FREETYPE_SELECTED_INFO})
        HELLOIMGUI_WITH_TEST_ENGINE:    ${HELLOIMGUI_WITH_TEST_ENGINE}
        BUILD_DEMOS - TESTS - DOCS:     ${HELLOIMGUI_BUILD_DEMOS} - ${HELLOIMGUI_BUILD_TESTS} - ${HELLOIMGUI_BUILD_DOCS}
    ---------------------------------------------------------------------------
      ImGui:
        Build ImGui:                    ${HELLOIMGUI_BUILD_IMGUI}
        ImGui source dir:               ${imgui_source_dir}
    ---------------------------------------------------------------------------
      nlohmann_json:                    ${HELLOIMGUI_NLOHMANN_JSON_SELECTED_INFO}")

    if(EMSCRIPTEN)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      Emscripten options
        HELLOIMGUI_EMSCRIPTEN_PTHREAD:  ${HELLOIMGUI_EMSCRIPTEN_PTHREAD}
        HELLOIMGUI_EMSCRIPTEN_PTHREAD_ALLOW_MEMORY_GROWTH: ${HELLOIMGUI_EMSCRIPTEN_PTHREAD_ALLOW_MEMORY_GROWTH}")
    endif ()

    if(HELLOIMGUI_HAS_OPENGL)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      OpenGL - use glad loader          ${HELLOIMGUI_USE_GLAD}")
    endif()

    set(msg "${msg}
    ---------------------------------------------------------------------------
      Platform Backend(s):")
    if(HELLOIMGUI_USE_SDL2)
        set(msg "${msg}
        SDL:                            ${HELLOIMGUI_SDL_SELECTED_INFO}")
    endif()
    if(HELLOIMGUI_USE_GLFW3)
        set(msg "${msg}
        Glfw:                           ${HELLOIMGUI_GLFW_SELECTED_INFO}")
    endif()

    if(WIN32)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      Windows:
        HELLOIMGUI_WIN32_NO_CONSOLE:    ${HELLOIMGUI_WIN32_NO_CONSOLE}
        HELLOIMGUI_WIN32_AUTO_WINMAIN:  ${HELLOIMGUI_WIN32_AUTO_WINMAIN}")
    endif()

    if(MACOSX)
        set(msg "${msg}
    ---------------------------------------------------------------------------
      macOS:
        HELLOIMGUI_MACOS_NO_BUNDLE:     ${HELLOIMGUI_MACOS_NO_BUNDLE}")
    endif()

    set(msg "${msg}
    ===========================================================================")

    message(STATUS "${msg}")
endfunction()

###################################################################################################
# Main: API = him_main_add_hello_imgui_library
###################################################################################################
function(him_main_add_hello_imgui_library)
    him_reset_installable_dependencies()
    him_back_parse_legacy_combinations()
    him_sanity_checks()
    him_add_stb_image()
    him_build_imgui()
    him_add_hello_imgui()
    him_add_nlohmann_json()
    if (HELLOIMGUI_WITH_TEST_ENGINE)
        add_imgui_test_engine()
    endif()

    if (HELLOIMGUI_USE_SDL2)
        him_use_sdl2_backend(${HELLOIMGUI_TARGET})
    endif()
    if (HELLOIMGUI_USE_GLFW3)
        him_use_glfw3_backend(${HELLOIMGUI_TARGET})
    endif()

    if (HELLOIMGUI_HAS_OPENGL3)
        him_has_opengl3(${HELLOIMGUI_TARGET})
    endif()
    if (HELLOIMGUI_HAS_METAL)
        him_has_metal(${HELLOIMGUI_TARGET})
    endif()
    if (HELLOIMGUI_HAS_VULKAN)
        him_has_vulkan(${HELLOIMGUI_TARGET})
    endif()
    if (HELLOIMGUI_HAS_DIRECTX11)
        him_has_directx11(${HELLOIMGUI_TARGET})
    endif()
    if (HELLOIMGUI_HAS_DIRECTX12)
        him_has_directx12(${HELLOIMGUI_TARGET})
    endif()

    if (HELLOIMGUI_WITH_NETIMGUI)
        him_with_netimgui()
    endif()
    if (HELLOIMGUI_WITH_IMGUIWS)
        him_with_imguiws()
    endif()

    him_add_apple_options()
    him_add_linux_options()
    him_add_windows_options()
    him_add_mobile_options()
    him_add_android_options()
    him_add_emscripten_options()
    him_add_misc_options()
    him_install()
    him_install_imgui()
endfunction()
