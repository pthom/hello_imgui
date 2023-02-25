# How to integrate HelloImGui into your application

This directory is a **standalone** demonstration on how to easily integrate HelloImGui to your own project.
This means that you can copy-paste this folder in your project, and it will build, with no extra step!
No manual download or git clone is required.


## Explanations

The [CMakeLists.txt](CMakeLists.txt) file will 
* download hello_imgui at configure time
* select the desired backend (edit the first lines if you want to use sdl instead of glfw)
* make the "hello_imgui_add_app" cmake function available
* Build HelloImGui and link it with your app at build time

```cmake
##########################################################
# Prepare hello_imgui during configure time
##########################################################
# Download hello_imgui
include(FetchContent)
FetchContent_Declare(
    hello_imgui
    GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
    # Enter the desired git tag below
    # GIT_TAG
)
FetchContent_MakeAvailable(hello_imgui)
# Make cmake function `hello_imgui_add_app` available
list(APPEND CMAKE_MODULE_PATH ${HELLOIMGUI_CMAKE_PATH})
include(hello_imgui_add_app)
```

After this, you only need to create your exe with one line!
```cmake
hello_imgui_add_app(hello_world hello_world.main.cpp)
```

### Standard usage

```bash
mkdir build
cd build
cmake ..
make -j 4
./hello_world
```

### Usage with emscripten

#### Install emscripten
You can either install emsdk following [the instruction on the emscripten website](https://emscripten.org/docs/getting_started/downloads.html) or you can use the script [../tools/emscripten/install_emscripten.sh](../tools/emscripten/install_emscripten.sh).

#### Compile with emscripten

```bash
# Add emscripten tools to your path
source ~/emsdk/emsdk_env.sh

# cmake and build
mkdir build_emscripten
cd build_emscripten
emcmake cmake ..
make -j 4

# launch a webserver
python3 -m http.server
```

Open a browser, and navigate to [http://localhost:8000](http://localhost:8000).
