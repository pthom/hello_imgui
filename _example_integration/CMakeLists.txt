cmake_minimum_required(VERSION 3.12)
project(helloworld_with_helloimgui)
set(CMAKE_CXX_STANDARD 17)

# Enable imgui_test_engine integration if desired
# set(HELLOIMGUI_WITH_TEST_ENGINE ON CACHE BOOL "" FORCE)

##########################################################
# Prepare hello_imgui during configure time
##########################################################
# Download hello_imgui
include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    hello_imgui
    GIT_REPOSITORY https://github.com/pthom/hello_imgui.git
    GIT_PROGRESS TRUE
    # Enter the desired git tag or branch below
    GIT_TAG with_imgui_test_engine
)
FetchContent_MakeAvailable(hello_imgui)


##########################################################
# Build your app
##########################################################
# Call hello_imgui_add_app
hello_imgui_add_app(hello_world hello_world.main.cpp)

# Now you can build your app with
#     mkdir build && cd build && cmake .. && cmake --build .
