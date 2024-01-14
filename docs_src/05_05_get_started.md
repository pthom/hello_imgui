# Get started

## Starter template

```{tip}
[The starter template](https://github.com/pthom/hello_imgui_template) will get you started in 5 minutes, and shows how to embed assets, customize the app icon (etc.), on all platforms.
It is recommended to use it as a starting point for your own project.
```

With this starter template, you do not need to clone HelloImGui, as it (optionally) can be downloaded and built automatically during CMake configure time.

See this extract from the CMakelists.txt file of the template:

```{literalinclude} ../_example_integration/CMakeLists.txt
:language: cmake
:start-at: Build hello_imgui
:end-at: hello_imgui_add_app
```


## CMake utility: hello_imgui_add_app

```{literalinclude} ../CMakeLists.txt
:language: cmake
:start-at: hello_imgui_add_app is a helper function
:end-before: include(hello_imgui_add_app)
```


## Assets and app customization

```{literalinclude} ../_example_integration/Readme.md
:language: markdown
:start-after: Assets folder structure
:end-before: Build instructions
```

## Immediate GUI mode

Hello ImGui is based on [Dear ImGui](https://github.com/ocornut/imgui), which is a library for immediate mode GUI programming.

With an Immediate mode GUI you can for example display a button and handle the click event in one line of code:

```{literalinclude} ../src/hello_imgui_demos/hello_globe/hello_globe.main.cpp
:language: cpp
```

[ImGui Manual](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html) is a great resource to learn interactively about all the widgets provided by Dear ImGui.
