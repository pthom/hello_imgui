if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW) # target_link_libraries() allows use with targets in other directories.
endif()


# Add imgui_test_engine lib with sources in imgui_test_engine/imgui_test_engine
function(_add_imgui_test_engine_lib)
    set(source_folder ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR}/imgui_test_engine)
    file(GLOB_RECURSE sources ${source_folder}/*.h ${source_folder}/*.cpp)
    add_library(imgui_test_engine ${sources})
    target_include_directories(imgui_test_engine PUBLIC ${source_folder}/..)
    target_link_libraries(imgui_test_engine PUBLIC imgui)

    if(HELLOIMGUI_BUILD_PYTHON)
        # if building python bindings, build a duplicate imgui_test_engine_py lib...
        add_library(imgui_test_engine_py ${sources})
        target_include_directories(imgui_test_engine_py PUBLIC ${source_folder}/..)
        target_link_libraries(imgui_test_engine_py PUBLIC imgui)
        # ... that will move the GIL between threads
        target_compile_definitions(imgui_test_engine_py PUBLIC IMGUI_TEST_ENGINE_WITH_PYTHON_GIL)
        # ... and needs to link with pybind
        target_link_libraries(imgui_test_engine_py PUBLIC pybind11::pybind11 pybind11::module)
    endif()
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
    target_include_directories(hello_imgui PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/..)
    target_include_directories(hello_imgui PRIVATE ${HELLOIMGUI_IMGUI_TEST_ENGINE_SOURCE_DIR})
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
    _add_imgui_test_engine_lib()
    _configure_imgui_with_test_engine()
    _add_hello_imgui_test_engine_integration()
    # _add_imgui_test_engine_app_minimal_example()
endfunction()
