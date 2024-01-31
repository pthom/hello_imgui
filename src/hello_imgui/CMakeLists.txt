###################################################################################################
# Build hello_imgui
###################################################################################################
# Global variables and + objective-c++ for apple
include(FetchContent)
include(${HELLOIMGUI_BASEPATH}/src/hello_imgui_test_engine_integration/hello_imgui_test_engine_cmake.cmake)


# Enable objective-c for apple. This is global and cannot (yet) be done in a function.
if (APPLE)
    enable_language(OBJC)
endif()

# Global variables for this file
message(STATUS "Library ${HELLOIMGUI_TARGET}")

# Call him_main_add_hello_imgui_library()
# This is the only part where we are allow to make function calls
# *above we only define functions (and sadly some global variables)*
include(${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/hello_imgui_build_lib.cmake)
him_main_add_hello_imgui_library()
