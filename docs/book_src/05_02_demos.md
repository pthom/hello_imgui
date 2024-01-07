# Demos


## Complete demo - advanced layout, FPS, theme, etc:

The C++ demo file [hello_imgui_demodocking.main.cpp](../../src/hello_imgui_demos/hello_imgui_demodocking/hello_imgui_demodocking.main.cpp) demonstrates:

* How to handle complex layouts: you can define several layouts and switch between them:  each layout which will remember the user modifications and the list of opened windows
* How to use theming
* How to store you own user settings in the app ini file
* How to add a status bar and a log window
* How to set an adaptative FPS for your application (to reduce CPU usage)

You can try this demo online via an [emscripten web demo](https://traineq.org/HelloImGui/bin/hello_imgui_demodocking.html). It is also available [in python](https://github.com/pthom/imgui_bundle/blob/main/bindings/imgui_bundle/demos_python/demos_immapp/demo_docking.py), inside [Dear ImGui Bundle](https://github.com/pthom/imgui_bundle)

Also, see this video that give more explanations on [how to handle multiple complex layouts](https://www.youtube.com/watch?v=XKxmz__F4ow)

## Demo - custom background

```{image} ../images/custom_background.jpg
---
width: 200px
align: left
```

The C++ demo file [hello_custom_background.main.cpp](../../src/hello_imgui_demos/hello_custom_background/hello_custom_background.main.cpp) demonstrates how to use a custom 3D background.

