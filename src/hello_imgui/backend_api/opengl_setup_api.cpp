#include "hello_imgui/backend_api/opengl_setup_api.h"
#include "hello_imgui/hello_imgui_include_opengl.h"

#include <string>


namespace BackendApi
{
    void IOpenGlSetup::ClearColor(ScreenSize windowSize, float r, float g, float b, float a)
    {
        glViewport(0, 0, windowSize[0], windowSize[1]);
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
} // namespace BackendApi
