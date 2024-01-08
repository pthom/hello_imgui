# Hello ImGui

Hello ImGui is a library that enables quickly write multiplatform apps with the simplicity of a "Hello World" app. Although being extremely easy to use, it can be used to create complex and customizable applications. It is based on [Dear ImGui](https://github.com/ocornut/imgui).

## Features

**Multiplatform utilities**
* Truly multiplatform: Linux, Windows, macOS, iOS, Android, emscripten (with 4 lines of CMake code)
* Easily embed assets on all platforms (no code required)
* Customize app settings (icon and app name for mobile platforms, etc.- no code required)
* Customize application icon on all platforms (including mobile and macOS - no code required)

**Dear ImGui Tweaks**
* Power Save mode: reduce FPS when idling
* High DPI support: scale UI according to DPI, whatever the platform
* Advanced layout handling: dockable windows, multiple layouts
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

---

## Demos & real world apps


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

<img src="https://raw.githubusercontent.com/pthom/imgui_bundle/main/bindings/imgui_bundle/demos_assets/images/logo_imgui_bundle_512.png" width=200>

[online interactive demo](https://traineq.org/ImGuiBundle/emscripten/bin/demo_imgui_bundle.html) - [Source for C++ demos](https://github.com/pthom/imgui_bundle/tree/main/bindings/imgui_bundle/demos_cpp) - [Source for Python demos](https://github.com/pthom/imgui_bundle/tree/main/bindings/imgui_bundle/demos_python)



### Theme tweaking

How to use the theme tweaking utilities provided by Hello ImGui

[Online video tutorial](https://www.youtube.com/watch?v=4f_-3DDcAZk)
