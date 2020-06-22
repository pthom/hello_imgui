#pragma once
#include <string>

namespace HelloImGui
{
/**
@@md#assets
`std::string assetFileFullPath(const std::string& assetRelativeFilename)` will return the path to assets.

For example, if you have the following project structure:
````
my_app/
├── CMakeLists.txt        # Your app's CMakeLists
├── assets/               # Its assets: for mobile devices and emscripten
│   └── fonts/            # they are embeddd automatically by hello_imgui_add_app.cmake
│       └── my_font.ttf
├── my_app.main.cpp       # Its source code
````
then you can call `assetFileFullPath("fonts/my_font.ttf")`

* Under iOS it will give a path in the app bundle (/private/XXX/....)
* Under emscripten, it will be stored in the virtual filesystem at "/"
@@md
*/
std::string assetFileFullPath(const std::string& assetRelativeFilename);
} // namespace HelloImGui
