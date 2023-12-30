#pragma once
#ifdef HELLOIMGUI_HAS_OPENGL

#include "hello_imgui/hello_imgui_include_opengl.h"

#include <memory>

namespace HelloImGui
{
    struct ImageOpenGl
    {
        ImageOpenGl(int width, int height, unsigned char* image_data_rgba);
        ~ImageOpenGl();

        int Width = 0;
        int Height = 0;
        GLuint TextureId = 0;
    };

    using ImageOpenGlPtr = std::shared_ptr<ImageOpenGl>;
}

#endif // #ifdef HELLOIMGUI_HAS_OPENGL
