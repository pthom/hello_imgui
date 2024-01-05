# Build instructions

## Supported platforms and backends

**Platforms:**  Windows, Linux, OSX, iOS, Emscripten, Android (poorly supported)

**Backends:**: SDL2 + OpenGL 3 or OpenGLES3 for mobile devices, Glfw3 + OpenGL 3

## Clone the repository

```bash
git clone https://github.com/pthom/hello_imgui.git
cd hello_imgui
git submodule update --init
```

## Easy build on desktop platforms using Glfw
On Desktop platforms (Linux, MacOS, Windows), if  you do not specify any backend option, HelloImGui will automatically download Glfw and link with it.

```bash
mkdir build
cd build 
cmake ..
make -j
```


## Custom build: select your preferred backend

In order to select your own backend, use one of the cmake options below:
```bash
cmake .. -DHELLOIMGUI_USE_GLFW_OPENGL3=ON
cmake .. -DHELLOIMGUI_USE_SDL_OPENGL3=ON
```


## Build for Windows: use `int main(int, char**)`

Under windows, Hello ImGui will automatically provide a [`WinMain()` function](../../hello_imgui_cmake/HelloImGui_WinMain.cpp) that will call main, and expects its signature to be `int main(int, char**)`. You may get a linker error if your main function signature is for example `int main()`.

You can disable this via cmake by passing `-DHELLOIMGUI_WIN32_AUTO_WINMAIN=OFF` as a command line cmake option. In this case, write your own `WinMain` under windows.

Warning: if using SDL, you will need to `#define SDL_MAIN_HANDLED` before any inclusion of SDL.h (to refrain SDL from #defining  `#define main SDL_main`)

## Build instructions for iOS

"SDL + OpenGL ES3" is currently the preferred backend for iOS.

This project uses the [ios-cmake](https://github.com/leetal/ios-cmake) toolchain which is a submodule in the folder [hello_imgui_cmake/](https://github.com/pthom/hello_imgui/tree/master).

1. Launch cmake with correct team and bundle url parts:

Adapt the command below, by:
* adding your own development team Id after `-DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=` ,
* setting HELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART (for example `-DHELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART=com.org_name_or_email`)
* setting the correct platform (-DPLATFORM): see https://github.com/leetal/ios-cmake (-DPLATFORM=OS64COMBINED will build for iOS and its simulator).
````bash
mkdir build_ios_sdl
cd build_ios_sdl
cmake .. \
  -GXcode \
  -DCMAKE_TOOLCHAIN_FILE=../hello_imgui_cmake/ios-cmake/ios.toolchain.cmake \
  -DHELLOIMGUI_USE_SDL_OPENGL3=ON \
  -DPLATFORM=OS64COMBINED \
  -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=... \
  -DHELLO_IMGUI_BUNDLE_IDENTIFIER_URL_PART=... \
  ..
````

---

## Build instructions for emscripten

> [emscripten](https://emscripten.org/) is a toolchain for compiling to asm.js and WebAssembly, built using LLVM, that lets you run C and C++ on the web at near-native speed without plugins.

### Install the requirements (emsdk)

You can either install emsdk following [the instruction on the emscripten website](https://emscripten.org/docs/getting_started/downloads.html) or you can use the script [tools/emscripten/install_emscripten.sh](../../tools/emscripten/install_emscripten.sh).

````bash
../tools/emscripten/install_emscripten.sh
````

This script will download and install emscripten into `~/emsdk`

### Build for emscripten

1. Add emsdk to your shell path;

You need to **source** the script ~/emsdk/emsdk_env.sh

```bash
source ~/emsdk/emsdk_env.sh
```

2. Run cmake, using "emcmake":

````bash
mkdir build_emscripten
cd build_emscripten
emcmake cmake ..
````

3. Build

```bash
make -j 4
```

4. Test your emscripten application

You will need a web server. Python provides a basic web server that is easy to usen which you can launch like this:

````bash
cd build_emscripten
python3 -m http.server
````

Open a browser, and navigate to [http://localhost:8000](http://localhost:8000).

For example, the docking demo will be available at
[http://localhost:8000/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.html](http://localhost:8000/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.html)

### Customizing the emscripten build

Refer to the [emscripten docs](https://emscripten.org/)

By default, the application will be presented inside an empty html page.
You can adapt this page by modyfing the "shell": copy the file [hello_imgui_cmake/emscripten/shell.emscripten.html](../../hello_imgui_cmake/emscripten/shell.emscripten.html)
into your app source dir, and adapt it to your needs.



## How to build terminal executables under macOS

If you prefer to build regular terminal executables (not app bundles), use the cmake option `-DHELLOIMGUI_MACOS_NO_BUNDLE=ON`.

## Build and deploy instructions for Android

The Android version uses SDL + OpenGLES3.

_Note: The Android version is currently not actively maintained._


### Set Android required environment variables

```bash
export ANDROID_HOME=/path/to/AndroidSdk
export ANDROID_NDK_HOME=/path/to/AndroidNdk
```

For example (MacOS):

```bash
export ANDROID_HOME=/Users/Me/Library/Android/sdk
export ANDROID_NDK_HOME=/Users/Me//Library/Android/sdk/ndk/21.3.6528147
```

If `ANDROID_NDK_HOME` is unset, by default, the scripts will look for Android-ndk inside `$ANDROID_HOME/ndk-bundle`.

### Run cmake in order to create an Android studio project

The script [tools/android/cmake_arm-android.sh](../../tools/android/cmake_arm-android.sh)  will invoke cmake with the android toolchain, and also _create an Android Studio project_ which
is multiarch (arm64-v8a, armeabi-v7a, etc), via the option `-DHELLOIMGUI_CREATE_ANDROID_STUDIO_PROJECT=ON` (see [tools/android/_impl_cmake_android.sh](../../tools/android/_impl_cmake_android.sh))


Run the following commands:

```bash
mkdir build_android
cd build_android
../tools/android/cmake_arm-android.sh ..
```

Your build directory will now look like this:

```
build_android/
├── CMakeCache.txt
├── ...
├── hello-imgui-demo-classic_AndroidStudio/
├── hello_imgui_demo_minimal_AndroidStudio/
├── hello_imgui_demodocking_AndroidStudio/
├── hello_world_AndroidStudio/
├── ...
```

The folders "xxxx_AndroidStudio" contain Android Studio projects, which you can use to build and debug your app.

You can now open (for example) the project hello_imgui_demodocking_AndroidStudio with Android Studio and run it / debug it.

You can also build the project manually via gradlew like this:

```bash
export JAVA_HOME=/Library/Java/JavaVirtualMachines/adoptopenjdk-8.jdk/Contents/Home 
cd hello_imgui_demodocking_AndroidStudio
./gradlew build
```

_Note: (you need to first set JAVA_HOME to the correct java version (Android requires exactly jdk8), the path given here is for MacOS users, where adoptopenjdk provides the correct version)_

You can also install the app via command line, like this:

```bash
./gradlew installDebug
```
---


### Customize for Android

See intructions in [Hello ImGui Template](https://github.com/pthom/hello_imgui_template/).

