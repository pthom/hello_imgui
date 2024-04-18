if (NOT HELLOIMGUI_WITH_NETIMGUI)
    message(FATAL_ERROR "HELLOIMGUI_WITH_NETIMGUI is not enabled")
endif()
if (NOT HELLOIMGUI_HAS_OPENGL3)
    message(FATAL_ERROR "HELLOIMGUI_HAS_OPENGL3 is not enabled, and required for hello_imgui_remote_display")
endif()


set(hal_gl3_cpp_file ${NETIMGUI_DIR}/Code/ServerApp/Source/GlfwGL3/NetImguiServer_HAL_GL3.cpp)
set(sources ${hal_gl3_cpp_file} netimgui_remote_display.cpp NetImguiServer_HAL_Glfw.cpp)
hello_imgui_add_app(netimgui_remote_display ${sources})
target_compile_definitions(net_imgui_server_lib PUBLIC NETIMGUI_SERVER_APP_BACKEND_GLFW_GL3=1)
target_link_libraries(netimgui_remote_display PRIVATE net_imgui_server_lib)
if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    target_link_libraries(netimgui_remote_display PRIVATE OpenGL::GL)
endif()

if (EMSCRIPTEN)
    # emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -DHELLOIMGUI_WITH_NETIMGUI=ON -DHELLOIMGUI_EMSCRIPTEN_PTHREAD=ON
    target_link_options(netimgui_remote_display PRIVATE -lwebsocket.js -sPROXY_POSIX_SOCKETS -pthread -sPTHREAD_POOL_SIZE=4 )
endif()
