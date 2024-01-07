# Get started

## Starter template
[The starter template](https://github.com/pthom/hello_imgui_template) will get you started in 5 minutes, and shows how to embed assets, customize the app icon (etc.), on all platforms.


```{note}
[hello_imgui_add_app](../../hello_imgui_cmake/hello_imgui_add_app.cmake) is a CMake helper function, similar to cmake's "add_executable" which will:

* automaticaly link the target to the required libraries (hello_imgui, OpenGl, glad, etc)
* embed the assets
* perform additional customization (app icon and name on mobile platforms, etc)

Usage: `hello_imgui_add_app(app_name file1.cpp file2.cpp ...)`
```