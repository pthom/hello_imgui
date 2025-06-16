add_executable(example_glfw_opengl3 example_glfw_opengl3.cpp)
find_package(OpenGL REQUIRED)
target_link_libraries(example_glfw_opengl3 PRIVATE hello_imgui OpenGL::GL)
