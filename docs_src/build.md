# Build instructions

## Build Hello ImGui and its demos

On almost all platforms, HelloImGui can be compiled with simple commands:

```bash
```bash
git clone https://github.com/pthom/hello_imgui.git
cd hello_imgui
mkdir build && cd build
cmake .. 
make -j
```

This will compile the HelloImGui library, and the demos (which will be located in the build/bin/ folder).

## Build your application using Hello ImGui

To build an application that uses HelloImGui, you can either place HelloImGui inside your project (for example as a submodule), or it can be downloaded and built automatically by cmake.

In any case, follow the build instructions given in the [HelloImGui Starter Template](https://github.com/pthom/hello_imgui_template)

## Available backends

See documentation below (extract from [CMakeLists.txt](https://github.com/pthom/hello_imgui/blob/master/CMakeLists.txt)): 
```{literalinclude} ../CMakeLists.txt
:language: cmake
:start-after: <Backends>
:end-before: </Backends>
```


In order to select your own backend, use one of the afore mentioned backend combinations, for example:
```bash
cmake .. -DHELLOIMGUI_USE_SDL2=ON -DHELLOIMGUI_HAS_OPENGL3=ON
```


## Hello ImGui dependencies

HelloImGui depends on the following libraries:

* **Glfw3 or SDL2**, depending on the platform backend you selected
* **Freetype**, for font rendering, if HELLOIMGUI_USE_FREETYPE is ON, which is the default

Those dependencies may be downloaded and built automatically by cmake, or you can provide your own version of those libraries.

See documentation below (extract from [CMakeLists.txt](https://github.com/pthom/hello_imgui/blob/master/CMakeLists.txt)):

```{literalinclude} ../CMakeLists.txt
:language: cmake
:start-after: <Dependencies>
:end-before: </Dependencies>
```

## Get dependencies via vcpkg

You can install almost all required dependencies with [vcpkg](https://github.com/microsoft/vcpkg).

Note: this will not support ImGui Test Engine, as it is not available in vcpkg yet.

### Manually

The file [vcpkg.json](https://github.com/pthom/hello_imgui/blob/master/vcpkg.json) defines the dependencies required by HelloImGui.

For example:
```bash
# Clone hello_imgui 
git clone https://github.com/pthom/hello_imgui.git
cd hello_imgui
# Clone vcpkg -& bootstrap
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=$(pwd)/vcpkg     # You *need* to set this environment variable
                                   # to tell cmake where to find vcpkg

# Install dependencies required by hello_imgui
# (they will be read from the vcpkg.json file)
./vcpkg/vcpkg install

# Build hello_imgui
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . -j 4 
```

### Using CMake Presets:


The file [CMakePresets.json](https://github.com/pthom/hello_imgui/blob/master/CMakePresets.json) defines several presets which will use vcpkg to grab the required dependencies.

Thus, you can build HelloImGui with vcpkg dependencies, using a CMake preset, like this:

```bash
# Clone hello_imgui 
git clone https://github.com/pthom/hello_imgui.git
cd hello_imgui
# Clone vcpkg -& bootstrap
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=$(pwd)/vcpkg     # You *need* to set this environment variable
                                   # to tell cmake where to find vcpkg

# Use the CMake preset "build_vcpkg_default" 
# This will grab all dependencies from vcpkg, 
# and use vcpkg toolchain file
cmake --preset build_vcpkg_default
cmake --build . -j 4 
```


## Hello ImGui CMake options

The [CMakelists.txt](https://github.com/pthom/hello_imgui/blob/master/CMakeLists.txt) file is heavily documented.

Below are the most important options:

**Freetype** (default: ON, except for Android and Mingw)
```cmake
option(HELLOIMGUI_USE_FREETYPE "Use freetype for text rendering" ${freetype_default})
```

**ImGui Test Engine** (default: OFF)
```cmake
option(HELLOIMGUI_WITH_TEST_ENGINE "Provide ImGui Test engine" OFF)
```


## OS specific instructions

### Windows instructions

Under windows, Hello ImGui will automatically provide a [`WinMain()` function](../hello_imgui_cmake/HelloImGui_WinMain.cpp) that will call main, and expects its signature to be `int main(int, char**)`. You may get a linker error if your main function signature is for example `int main()`.

You can disable this via cmake by passing `-DHELLOIMGUI_WIN32_AUTO_WINMAIN=OFF` as a command line cmake option. In this case, write your own `WinMain` under windows.

Warning: if using SDL, you will need to `#define SDL_MAIN_HANDLED` before any inclusion of SDL.h (to refrain SDL from #defining  `#define main SDL_main`)

### iOS instructions

"SDL + OpenGL ES3" is currently the preferred backend for iOS.

This project uses the [ios-cmake](https://github.com/leetal/ios-cmake) toolchain which is a submodule in the folder [hello_imgui_cmake/](https://github.com/pthom/hello_imgui/tree/master/hello_imgui_cmake).

See compilation instructions in the [HelloImGui Starter Template](https://github.com/pthom/hello_imgui_template)


### Emscripten instructions

> [emscripten](https://emscripten.org/) is a toolchain for compiling to asm.js and WebAssembly, built using LLVM, that lets you run C and C++ on the web at near-native speed without plugins.

See compilation instruction in the [HelloImGui Starter Template](https://github.com/pthom/hello_imgui_template)


To test your emscripten application, you will need a web server. Python provides a basic web server that is easy to usen which you can launch like this:

````bash
cd build_emscripten
python3 -m http.server
````

Open a browser, and navigate to [http://localhost:8000](http://localhost:8000).


### macOS instructions

If you prefer to build regular terminal executables (not app bundles), use the cmake option `-DHELLOIMGUI_MACOS_NO_BUNDLE=ON`.

### Android instructions

The Android version uses SDL + OpenGLES3.

See compilation instructions in the [HelloImGui Starter Template](https://github.com/pthom/hello_imgui_template)
