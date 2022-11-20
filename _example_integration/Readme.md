# How to use integrate HelloImGui into your application

This directory is a **standalone** demonstration on how to easily integrate HelloImGui to your own project.
This means that you can copy-paste this folder in your project, and it will build, with no extra step!
No manual download or git clone is required.


## Explanations

The [CMakeLists.txt](CMakeLists.txt) file will 
* download hello_imgui at configure time
* select the desired backend
* make the "hello_imgui_add_app" cmake function available
* Build HelloImGui and link it with your app at build time

````cmake
##########################################################
# Prepare hello_imgui during configure time
##########################################################
# Select your backend
set(HELLOIMGUI_WITH_SDL OFF CACHE BOOL "" FORCE)
set(HELLOIMGUI_WITH_GLFW ON CACHE BOOL "" FORCE)
# Download hello_imgui
include(FetchContent)
FetchContent_Declare(
    hello_imgui
    GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
    # Enter the desired git tag below
    # GIT_TAG
)
FetchContent_MakeAvailable(hello_imgui)
# Make cmake function `hello_imgui_add_app` available
list(APPEND CMAKE_MODULE_PATH ${HELLOIMGUI_CMAKE_PATH})
include(hello_imgui_add_app)
````

After this, you only need to create your exe with one line!
````cmake
hello_imgui_add_app(hello_world hello_world.main.cpp)
````
