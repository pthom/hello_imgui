if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW) # target_link_libraries() allows use with targets in other directories.
endif()

include(FetchContent)


# By default we will fetch imgui_test_engine from https://github.com/pthom/imgui_test_engine/ (branch imgui_bundle)
# but you can override this (set HELLOIMGUI_FETCH_IMGUI_TEST_ENGINE=OFF and set HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR)
option(HELLOIMGUI_FETCH_IMGUI_TEST_ENGINE "Fetch imgui_test_engine from https://github.com/pthom/imgui_test_engine/ (branch imgui_bundle)" ON)


# fetch imgui_test_engine from https://github.com/pthom/imgui_test_engine/ (branch imgui_bundle) if needed
function(_fetch_imgui_test_engine_if_needed)
    if(NOT HELLOIMGUI_FETCH_IMGUI_TEST_ENGINE AND NOT HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR)
        message(FATAL_ERROR "Either set HELLOIMGUI_FETCH_IMGUI_TEST_ENGINE=ON or set HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR")
    endif()
    if(HELLOIMGUI_FETCH_IMGUI_TEST_ENGINE)
        Set(FETCHCONTENT_QUIET FALSE)
        FetchContent_Declare(
            imgui_test_engine
            GIT_REPOSITORY https://github.com/pthom/imgui_test_engine.git
            GIT_PROGRESS TRUE
            GIT_TAG imgui_bundle
        )
        FetchContent_MakeAvailable(imgui_test_engine)
        set(HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps/imgui_test_engine-src CACHE STRING "" FORCE)
    endif()
endfunction()


# Add imgui_test_engine lib with sources in imgui_test_engine/imgui_test_engine
function(_add_imgui_test_engine_lib)
    set(te_source_folder ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR}/imgui_test_engine)
    file(GLOB_RECURSE sources ${te_source_folder}/*.h ${te_source_folder}/*.cpp)
    add_library(imgui_test_engine ${sources})
    target_include_directories(imgui_test_engine PUBLIC $<BUILD_INTERFACE:${te_source_folder}/..>)

    # Link imgui_test_engine with imgui
    target_link_libraries(imgui_test_engine PUBLIC imgui)
    # BUT ALSO any app built with ImGui should now also link with imgui_test_engine
    target_link_libraries(imgui PUBLIC imgui_test_engine)

    if (HELLOIMGUI_STB_IMAGE_WRITE_IMPLEMENTATION)
        target_compile_definitions(imgui_test_engine PRIVATE IMGUI_DISABLE_STB_IMAGE_WRITE_IMPLEMENTATION)
    endif()
    target_compile_definitions(imgui_test_engine PRIVATE
        IMGUI_STB_IMAGE_WRITE_FILENAME="${HELLOIMGUI_BASEPATH}/external/stb_hello_imgui/stb_image_write.h"
    )

    # install test_engine headers
    if(HELLOIMGUI_INSTALL)
        file(GLOB te_headers ${te_source_folder}/*.h)
        install(FILES ${te_headers} DESTINATION include/imgui_test_engine)
        install(DIRECTORY ${te_source_folder}/thirdparty DESTINATION include)
    endif()
    him_add_installable_dependency(imgui_test_engine)
endfunction()


# ImGui uses imconfig from imconfig_with_test_engine.h (with options for imgui_test_engine)
function(_configure_imgui_with_test_engine)
    target_compile_definitions(imgui
        PUBLIC
        IMGUI_ENABLE_TEST_ENGINE
        IMGUI_TEST_ENGINE_ENABLE_CAPTURE=1
        IMGUI_TEST_ENGINE_ENABLE_STD_FUNCTION=1
        IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1
        )
    #        IMGUI_TEST_ENGINE_ENABLE_IMPLOT=0
endfunction()


# Add integration into HelloImGui
function(_add_hello_imgui_test_engine_integration)
    target_compile_definitions(hello_imgui PUBLIC HELLOIMGUI_WITH_TEST_ENGINE)
    target_sources(hello_imgui PRIVATE
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/test_engine_integration.cpp
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/test_engine_integration.h
        )
    target_include_directories(hello_imgui PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_FUNCTION_LIST_DIR}/..>)
endfunction()


# Configure imgui_test_engine for use with python / pybind11: move the GIL between threads
function(configure_imgui_test_engine_with_python_gil)
    # 1. imgui_test_engine should move the GIL between threads
    target_compile_definitions(imgui_test_engine PUBLIC IMGUI_TEST_ENGINE_WITH_PYTHON_GIL)
    # 2. and for this it needs to link with pybind
    if(SKBUILD)
        # if building wheel, only add include path to pybind11 and python
        target_link_libraries(imgui_test_engine PUBLIC pybind11::pybind11)
    else()
        # if building an app, link the python interpreter
        target_link_libraries(imgui_test_engine PUBLIC pybind11::embed)
    endif()
endfunction()


# Unused: add the original app_minimal example from imgui_test_engine
function(_add_imgui_test_engine_app_minimal_example)
    # This does not compile at the moment since app_minimal_main.cpp require implot
    # (but commenting out the related lines works)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(imgui_base_path ${PROJECT_SOURCE_DIR}/external/imgui)
    add_executable(test_app_minimal
        ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR}/app_minimal/app_minimal_main.cpp
        ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR}/app_minimal/app_minimal_tests.cpp
        ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR}/app_minimal/app_minimal_imconfig.h
        ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR}/shared/imgui_app.cpp
        ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR}/shared/imgui_app.h
        )
    target_compile_definitions(test_app_minimal PUBLIC IMGUI_APP_GLFW_GL3)
    find_package(Python3 COMPONENTS Development)
    target_link_libraries(test_app_minimal PRIVATE imgui glfw Python3::Python)
    target_include_directories(test_app_minimal PUBLIC ${imgui_base_path}/backends)
endfunction()


# Public API for this module
function(add_imgui_test_engine)
    _fetch_imgui_test_engine_if_needed()
    _add_imgui_test_engine_lib()
    _configure_imgui_with_test_engine()
    _add_hello_imgui_test_engine_integration()
    # _add_imgui_test_engine_app_minimal_example()
endfunction()
