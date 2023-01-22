# emscripten is detected via
#  - if (EMSCRIPTEN) in cmake code
#  - #ifdef __EMSCRIPTEN__ in c++ code

set(BUILD_SHARED_LIBS OFF)

# Debug build is  -O0 -gsource-map in order to have the call stack + source map
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -gsource-map")
    # source-map-base enables to be able to "kind of debug" from the js developper console
    # Advice: always debug the same exact source code on desktop before even trying to run in the browser
    # --source-map-base http://localhost:8000/src/
endif()
