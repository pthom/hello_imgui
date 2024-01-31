<a href="://pthom.github.io/hello_imgui/"><img src="logo/logo_hello_imgui.png" width="120"></a>

_See the [HelloImGui documentation website](https://pthom.github.io/hello_imgui/) for API, usage and build instructions._


**OpenGL**<br/>
[![Win_Mac_Linux](https://github.com/pthom/hello_imgui/workflows/Win_Mac_Linux/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Win_Mac_Linux.yml)
[![MinGW](https://github.com/pthom/hello_imgui/workflows/MinGW/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/MinGW.yml)

**Mobile and web**<br/>
[![iOS](https://github.com/pthom/hello_imgui/workflows/iOS/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/iOS.yml)
[![Android](https://github.com/pthom/hello_imgui/workflows/Android/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Android.yml)
[![Emscripten](https://github.com/pthom/hello_imgui/workflows/Emscripten/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Emscripten.yml)

**Alternate rendering backends**<br/>
[![Metal](https://github.com/pthom/hello_imgui/workflows/Metal/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Metal.yml)
[![Vulkan](https://github.com/pthom/hello_imgui/workflows/Vulkan/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Vulkan.yml)
[![DirectX](https://github.com/pthom/hello_imgui/workflows/DirectX/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/DirectX.yml)

**Misc**<br/>
[![TestEngine](https://github.com/pthom/hello_imgui/workflows/TestEngine/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/TestEngine.yml)
[![Automate](https://github.com/pthom/hello_imgui/workflows/Automate/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Automate.yml)
[![StarterTemplate](https://github.com/pthom/hello_imgui/workflows/StarterTemplate/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/StarterTemplate.yml)

**Vcpkg**<br/>
[![VcpkgDeps](https://github.com/pthom/hello_imgui/workflows/VcpkgDeps/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/VcpkgDeps.yml)
[![VcpkgPackage](https://github.com/pthom/hello_imgui/workflows/VcpkgPackage/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/VcpkgPackage.yml)

# Hello ImGui


Hello ImGui is a library designed to make multiplatform app development as simple as writing a "Hello World" program, built on top of [Dear ImGui](https://github.com/ocornut/imgui).

Its flexibility makes it suited for complex applications, as well as for simple prototypes; allowing you to focus on the creative aspects of your projects.

## Features

**Multiplatform utilities**
* Seamlessly multiplatform: supports Linux, Windows, macOS, iOS, Android, and Emscripten with minimal setup (1 line of CMake code)
* Asset embedding across all platforms  (no code required)
* Effortless app customization, including app icons and names for mobile platforms (no code required)
* Universal application icon customization, extending to mobile and macOS (no code required)

**Dear ImGui Tweaks**
* Power Save mode: optimize performance by reducing FPS when idle
* High DPI support: automatically scales UI to match screen DPI on any platform.
* Enhanced layout handling: dockable windows and multiple layouts for complex UIs
* Window geometry utilities: autosize application window, restore app window position
* Theme tweaking: extensive list of additional themes
* Support for movable and resizable borderless windows
* Advanced font support: icons, emojis and colored fonts
* Integration with ImGui Test Engine: automate and test your apps
* Save user settings: window position, layout, opened windows, theme, user defined custom settings
* Easily add a custom 3D background to your app 

**Backends**
* Available platform backends: SDL2, Glfw3
* Available rendering backends: OpenGL3, Metal, Vulkan, DirectX


## Demos & real world apps

### Motto

The minimum code to start developing a GUI app should be... minimal. Here is a multiplatform Hello World in 7 lines.

*6 lines of C++*
```cpp
#include "hello_imgui/hello_imgui.h"
int main(int , char *[]) {
    HelloImGui::Run(
        []{ ImGui::Text("Hello, world!"); }, // Gui code
        "Hello!", true);                     // Window title + Window size auto
}
```

*1 line of CMake*
```cmake
hello_imgui_add_app(hello_world hello_world.cpp)
```


### Advanced layout

The docking demo show how to handle complex layouts, use themes, store user settings, reduce FPS and CPU usage when idling, load fonts and icons, and much more

![](https://github.com/pthom/hello_imgui/blob/master/docs_src/images/docking.jpg?raw=true)

[Online demo](https://traineq.org/ImGuiBundle/emscripten/bin/demo_docking.html) - [Source](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp) - Video tutorial: [how to handle multiple complex layouts](https://www.youtube.com/watch?v=XKxmz__F4ow)

### Custom 3D background

How to use a custom 3D background in your app

![](https://github.com/pthom/hello_imgui/blob/master/docs_src/images/custom_background.jpg?raw=true≥)

[Online demo](https://traineq.org/ImGuiBundle/emscripten/bin/demo_custom_background.html) - [Source](https://github.com/pthom/hello_imgui/blob/master/src/hello_imgui_demos/hello_custom_background/hello_custom_background.main.cpp)


### ImGui Manual

[ImGui Manual](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html) is an interactive Manual for Dear ImGui, built with Hello ImGui.

[![](https://github.com/pthom/hello_imgui/blob/master/docs_src/images/manual.jpg?raw=true)](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html)

[Online manual](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html) -
[Source](https://github.com/pthom/imgui_manual)


### RPN Calculator

[RPN Calculator](https://pthom.github.io/imgui_rpn_calculator/) is a simple RPN calculator, built, built to demonstrate how easy a multiplatform app can be built with Hello ImGui.

![](https://github.com/pthom/rpn_calculator/blob/main/screenshot.png?raw=true)

[Online demo](https://traineq.org/HelloImGui/rpn/rpn_calculator.html) - [Source](https://github.com/pthom/rpn_calculator/) - [Video tutorial](https://www.youtube.com/watch?v=dArP4lBnOr8)

### Starter template
[The starter template](https://github.com/pthom/hello_imgui_template) will get you started in 5 minutes, and shows how to embed assets, customize the app icon (etc.), on all platforms.

### Dear ImGui Bundle

[Dear ImGui Bundle](https://pthom.github.io/imgui_bundle/), a full-fledged library, built on top of Hello ImGui, provides lots of additional widgets ([imgui](https://github.com/ocornut/imgui.git), [implot](https://github.com/epezent/implot), [imgui-node-editor](https://github.com/thedmd/imgui-node-editor), [ImFileDialog](https://github.com/pthom/ImFileDialog.git), [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit), [imgui_md](https://github.com/mekhontsev/imgui_md.git)), as well as complete python bindings.

<img src="https://raw.githubusercontent.com/pthom/imgui_bundle/main/bindings/imgui_bundle/demos_assets/images/logo_imgui_bundle_512.png" width=100>

[online interactive demo](https://traineq.org/ImGuiBundle/emscripten/bin/demo_imgui_bundle.html) - [Source for C++ demos](https://github.com/pthom/imgui_bundle/tree/main/bindings/imgui_bundle/demos_cpp) - [Source for Python demos](https://github.com/pthom/imgui_bundle/tree/main/bindings/imgui_bundle/demos_python)


### Theme tweaking

How to use the theme tweaking utilities provided by Hello ImGui

[Online video tutorial](https://www.youtube.com/watch?v=4f_-3DDcAZk)

---

## About

### Credits

Hello ImGui is based on [Dear ImGui](https://github.com/ocornut/imgui) (MIT License), by Omar Cornut. Portions use [ImGui Test Engine](https://github.com/ocornut/imgui_test_engine), which is governed by a [specific license](https://github.com/ocornut/imgui_test_engine/blob/main/imgui_test_engine/LICENSE.txt)

Portions of this software use the [FreeType Project](https://www.freetype.org), [lunasvg](https://github.com/sammycage/lunasvg) (MIT License), [GLFW](https://www.glfw.org/) (zlib/libpng license), [SDL](https://www.libsdl.org/) (zlib license).

### License

Hello ImGui is licensed under the MIT License, see [LICENSE](https://github.com/pthom/hello_imgui/blob/master/LICENSE)

### Contribute

Quality contributions are welcome!

### Support the project

Hello ImGui is a free and open source project, and its development and maintenance require considerable efforts.

If you find it valuable for your work – especially in a commercial enterprise or a research setting – please consider supporting its development by [making a donation](https://www.paypal.com/donate/?hosted_button_id=SHJ68RVDKURZA). Thank you!

<!--
[![](https://raw.githubusercontent.com/pthom/imgui_bundle/main/bindings/imgui_bundle/doc/doc_images/btn_donate.gif)](https://www.paypal.com/donate/?hosted_button_id=SHJ68RVDKURZA)
-->


# More info

_See the [HelloImGui documentation website](https://pthom.github.io/hello_imgui/) for API, usage and build instructions._