# emscripten is detected via
#  - if (EMSCRIPTEN) in cmake code
#  - #ifdef __EMSCRIPTEN__ in c++ code

set(BUILD_SHARED_LIBS OFF)

# Standard flags for emscripten
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-s USE_SDL=2 -s USE_WEBGL2=1 -s WASM=1 -s FULL_ES3=1 -s ALLOW_MEMORY_GROWTH=1")

# Debug build is  -O0 -g4 in order to have the call stack + source map
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g4 -s DISABLE_EXCEPTION_CATCHING=0")

    # source-map-base enables to be able to "kind of debug" from the js developper console
    # Advice: always debug the same exact source code on desktop before even trying to run in the browser
    set(link_options_debug -O0 --source-map-base http://localhost:8000/src/)
endif()
