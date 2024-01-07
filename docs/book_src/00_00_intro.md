# Hello ImGui

Hello ImGui is a library that enables quickly write multiplatform apps with the simplicity of a "Hello World" app. It is based on [Dear ImGui](https://github.com/ocornut/imgui).


## Features

* Advanced layout handling: [Online Demo](https://traineq.org/ImGuiBundle/emscripten/bin/demo_docking.html) - [Video tutorial](https://www.youtube.com/watch?v=XKxmz__F4ow) - [Code](../../src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp)
* Window geometry utilities (autosize, restore window position)
* Save application settings: window position, layout, opened windows, theme, user defined custom settings
* Embed assets on all platforms (desktop, mobile, web)
* Customize application icon on all platforms
* Advanced font support: icons, emojis and colored fonts
* Integration with ImGui Test Engine: automate and test your apps
* Power Save mode: reduce FPS when application is idle
* [Theme tweaking](https://www.youtube.com/watch?v=Hhartw0cUjg)
* [Truly multiplatform](https://traineq.org/HelloImGui_6_Platforms.mp4) (Linux, Windows, MacOS, iOS, Android, emscripten)
* Support for movable and resizable borderless windows
* Available platform backends: SDL2, Glfw3
* Available rendering backends: OpenGL3, Metal, Vulkan, DirectX
* Set up a project in 5 minutes

## Dear ImGui Bundle - Sister library

```{image} https://raw.githubusercontent.com/pthom/imgui_bundle/main/bindings/imgui_bundle/demos_assets/images/logo_imgui_bundle_512.png
---
width: 200px
align: left
```

_Do you want an easy way to use Dear ImGui with additional widgets, and in Python?_

Dear ImGui Bundle, is a sister library, based on Hello ImGui, provides lots of additional widgets ([imgui](https://github.com/ocornut/imgui.git), [implot](https://github.com/epezent/implot), [imgui-node-editor](https://github.com/thedmd/imgui-node-editor), [ImFileDialog](https://github.com/pthom/ImFileDialog.git), [ImGuiColorTextEdit](https://github.com/BalazsJako/ImGuiColorTextEdit), [imgui_md](https://github.com/mekhontsev/imgui_md.git)), as well as complete python bindings.


See ImGuiBundle's  [C++ demos](https://github.com/pthom/imgui_bundle/tree/main/demos_cpp) and [Python demos](https://github.com/pthom/imgui_bundle/tree/main/bindings/imgui_bundle/demos).



## CI Status

```{list-table}
:header-rows: 1

* - OpenGL
  - Mobile and web
  - Alternate rendering
  - Misc

* - [![Win_Mac_Linux](https://github.com/pthom/hello_imgui/workflows/Win_Mac_Linux/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Win_Mac_Linux.yml)
    [![MinGW](https://github.com/pthom/hello_imgui/workflows/MinGW/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/MinGW.yml)

  - [![iOS](https://github.com/pthom/hello_imgui/workflows/iOS/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/iOS.yml)
[![Android](https://github.com/pthom/hello_imgui/workflows/Android/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Android.yml)
[![Emscripten](https://github.com/pthom/hello_imgui/workflows/Emscripten/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Emscripten.yml)

  - [![Metal](https://github.com/pthom/hello_imgui/workflows/Metal/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Metal.yml)
[![Vulkan](https://github.com/pthom/hello_imgui/workflows/Vulkan/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Vulkan.yml)
[![DirectX](https://github.com/pthom/hello_imgui/workflows/DirectX/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/DirectX.yml)

  - [![TestEngine](https://github.com/pthom/hello_imgui/workflows/TestEngine/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/TestEngine.yml)
[![Automate](https://github.com/pthom/hello_imgui/workflows/Automate/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/Automate.yml)
[![StarterTemplate](https://github.com/pthom/hello_imgui/workflows/StarterTemplate/badge.svg)](https://github.com/pthom/hello_imgui/actions/workflows/StarterTemplate.yml)

```

# Table of contents

```{tableofcontents}
```
