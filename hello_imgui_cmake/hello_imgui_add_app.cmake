include(${CMAKE_CURRENT_LIST_DIR}/utils/cache_hello_imgui_paths.cmake)

#
# hello_imgui_add_app is a helper function, similar to cmake's "add_executable"
#
# Usage:
#     hello_imgui_add_app(app_name file1.cpp file2.cpp ...)
# Or:
#     hello_imgui_add_app(app_name file1.cpp file2.cpp ... ASSETS_LOCATION "path/to/assets")
# (By default, ASSETS_LOCATION is "assets", which means that the assets will be searched in the "assets" folder,
# relative to the location of the CMakeLists.txt file)
#
# Features:
#     * It will automatically link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
#     * It will embed the assets (for desktop, mobile, and emscripten apps)
#     * It will perform additional customization (app icon and name on mobile platforms, etc)
#
function(hello_imgui_add_app)
    #############################################################################
    # CMake argument parsing shenanigans...
    # arguments are parsed as: app_name, app_sources, assets_location
    #############################################################################
    # Define the keywords for known arguments
    set(oneValueArgs ASSETS_LOCATION)
    set(multiValueArgs "")
    set(options "")
    # Parse the arguments
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    # The application name is the first argument in ARGN
    list(GET ARGN 0 app_name)
    # After parsing, the unparsed arguments are what's left and are treated
    # as sources except the first one (which is the app name)
    # We should also remove the ASSETS_LOCATION argument if provided
    list(REMOVE_AT ARG_UNPARSED_ARGUMENTS 0)
    set(app_sources ${ARG_UNPARSED_ARGUMENTS})
    # The ASSETS_LOCATION parameter is optional
    if(NOT ARG_ASSETS_LOCATION)
        set(assets_location "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    else()
        # if assets_location is relative to the current source dir, make it absolute
        if (NOT IS_ABSOLUTE ${ARG_ASSETS_LOCATION})
            set(assets_location "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_ASSETS_LOCATION}")
        else()
            set(assets_location ${ARG_ASSETS_LOCATION})
        endif()
    endif()

    message(VERBOSE "hello_imgui_add_app
             app_name=${app_name}
             sources=${app_sources}
             assets_location=${assets_location}
    ")

    #############################################################################
    # If windows, and if the user wants to, we can make this an app without console
    # and provide a WinMain entry point
    if (WIN32)
        if (HELLOIMGUI_WIN32_NO_CONSOLE)
            # Make this an app without console
            list(PREPEND app_sources WIN32)
        endif()
        if (HELLOIMGUI_WIN32_AUTO_WINMAIN)
            # Use HelloImGui_WinMain.cpp
            list(APPEND app_sources ${HELLOIMGUI_CMAKE_PATH}/HelloImGui_WinMain.cpp)
        endif()
    endif()

    # Add the target for the application
    if (ANDROID)
        add_library(${app_name} SHARED ${app_sources})
    else()
        add_executable(${app_name} ${app_sources})
    endif()

    hello_imgui_prepare_app(${app_name} ${assets_location})
endfunction()



# Platform dependent version of hello_imgui_platform_customization
#
# By default hello_imgui_platform_customization does nothing
#
function(hello_imgui_platform_customization app_name assets_location)
endfunction()
# But it can be overriden by platform specific implementations, in the following files:
if(APPLE)
    include(${CMAKE_CURRENT_LIST_DIR}/apple/hello_imgui_apple.cmake)
elseif(ANDROID)
    include(${CMAKE_CURRENT_LIST_DIR}/android/hello_imgui_android.cmake)  # almost empty
    include(${CMAKE_CURRENT_LIST_DIR}/android/apkCMake/apkCMake.cmake)    # most of the logic is here and called by hello_imgui_prepare_app()
elseif (EMSCRIPTEN)
    include(${CMAKE_CURRENT_LIST_DIR}/emscripten/hello_imgui_emscripten.cmake)
elseif (WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/windows/hello_imgui_windows.cmake)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/assets/hello_imgui_assets.cmake)


function(set_bundle_variables_defaults app_name)
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART)
        set(HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART com.helloimgui PARENT_SCOPE)
    endif()
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART)
        set(HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART ${app_name} PARENT_SCOPE)
    endif()
    if (NOT DEFINED HELLO_IMGUI_ICON_DISPLAY_NAME)
        set(HELLO_IMGUI_ICON_DISPLAY_NAME ${app_name} PARENT_SCOPE)
    endif()
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_NAME)
        set(HELLO_IMGUI_BUNDLE_NAME ${app_name} PARENT_SCOPE)
    endif()
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_COPYRIGHT)
        set(HELLO_IMGUI_BUNDLE_COPYRIGHT "" PARENT_SCOPE)
    endif()
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_EXECUTABLE)
        set(HELLO_IMGUI_BUNDLE_EXECUTABLE ${app_name} PARENT_SCOPE)
    endif()
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_VERSION)
        set(HELLO_IMGUI_BUNDLE_VERSION 0.0.1 PARENT_SCOPE)
    endif()
    if (NOT DEFINED HELLO_IMGUI_BUNDLE_ICON_FILE)
        set(HELLO_IMGUI_BUNDLE_ICON_FILE "icon.icns" PARENT_SCOPE)
    endif()
endfunction()


# hello_imgui_prepare_app is a helper function, that will prepare an app to be used with hello_imgui
#
# Usage:
# hello_imgui_prepare_app(target_name)
#
# Features:
#     * It will automatically link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
#     * It will embed the assets (for desktop, mobile, and emscripten apps)
#     * It will perform additional customization (app icon and name on mobile platforms, etc)
function(hello_imgui_prepare_app app_name assets_location)
    set_bundle_variables_defaults(${app_name})
    hello_imgui_bundle_assets(${app_name} ${assets_location})
    hello_imgui_platform_customization(${app_name} ${assets_location})

    target_link_libraries(${app_name} PRIVATE hello-imgui::hello_imgui)

    if (ANDROID AND HELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT)
        set(apkCMake_applicationIdUrlPart ${HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART})
        set(apkCMake_applicationIdNamePart ${HELLO_IMGUI_BUNDLE_IDENTIFIER_NAME_PART})
        set(apkCMake_iconDisplayName  ${HELLO_IMGUI_ICON_DISPLAY_NAME})
        # set(apkCMake_abiFilters "'arm64-v8a', 'x86', 'x86_64'")
        apkCMake_makeAndroidStudioProject(${app_name} ${assets_location})
    endif()
endfunction()


#
# hello_imgui_get_all_subdirs_targets returns the recursive list of all targets
# under a given directory
#
function (hello_imgui_get_all_subdirs_targets out_var current_dir)
    get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)

    foreach(subdir ${subdirs})
        hello_imgui_get_all_subdirs_targets(subdir_targets ${subdir})
        list(APPEND targets ${subdir_targets})
    endforeach()

    set(${out_var} ${targets} PARENT_SCOPE)
endfunction()
