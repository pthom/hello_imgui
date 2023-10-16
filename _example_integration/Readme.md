# How to integrate HelloImGui into your application

This directory is a **standalone** demonstration on how to easily integrate HelloImGui to your own project.
This means that you can copy-paste this folder in your project, and it will build, with no extra step!
No manual download or git clone is required.


## Explanations

The [CMakeLists.txt](CMakeLists.txt) file will download and build hello_imgui at configure time, and make the "hello_imgui_add_app" cmake function available.

*Note: hello_imgui_add_app will automaticaly link your app to hello_imgui, and will embed the assets folder (for desktop, mobile, and emscripten apps)*

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
