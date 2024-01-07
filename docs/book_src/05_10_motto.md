# Motto

## Immediate Gui and easy app deployment


<img src="../images/hello_globe.jpg" width="150">


**hello_globe.main.cpp**
```cpp
#include "hello_imgui/hello_imgui.h"
int main(int , char *[])
{
    auto guiFunction = []() {
        ImGui::Text("Hello, ");                    // Display a simple label
        HelloImGui::ImageFromAsset("world.png");   // Display a static image
        if (ImGui::Button("Bye!"))                 // Display a button
            // and immediately handle its action if it is clicked!
            HelloImGui::GetRunnerParams()->appShallExit = true;
     };
    HelloImGui::Run(guiFunction, "Hello, globe", true);
    return 0;
}
```

Dear ImGui uses the Immediate Gui paradigm: each button, each widget returns true if the user interacted with it.


### include assets
Anything in the assets/ folder located beside the app's CMakeLists will be embedded in the app:
```
└── hello_globe.main.cpp
├── CMakeLists.txt
├── assets/
│    └── world.png
```
(even on iOS and emscripten).

## Hello, world!

Hello World in 7 lines of C++ / CMake:

[Online demo](https://traineq.org/HelloImGui/bin/hello_world.html)

Save this as `hello_world.main.cpp`
```cpp
#include "hello_imgui/hello_imgui.h"
int main(int , char *[]) {
    HelloImGui::Run(
        []{ ImGui::Text("Hello, world!"); }, // Gui code
        "Hello!",                            // Window title
        true);                               // Window size auto
}
```

Save this as `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.12)
project(helloworld_with_helloimgui)
set(CMAKE_CXX_STANDARD 17)
# Fetch hello_imgui during the build
include(FetchContent)
FetchContent_Declare(hello_imgui GIT_REPOSITORY https://github.com/pthom/hello_imgui.git GIT_TAG master)
FetchContent_MakeAvailable(hello_imgui)
# Build your app
hello_imgui_add_app(hello_world hello_world.main.cpp)
```

Now, build and run with:

```bash
# Build
mkdir build && cd build && cmake .. && cmake --build . -j 4
# Run the build hello_world app
./hello_world
```
That's it!



### handle events

