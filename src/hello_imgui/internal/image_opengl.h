#pragma once
#ifdef HELLOIMGUI_HAS_OPENGL

#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/texture_gpu.h"
#include <memory>

namespace HelloImGui
{
    struct TextureGpuOpenGl: public TextureGpu
    {
        TextureGpuOpenGl() = default;
        ~TextureGpuOpenGl() override;

        ImTextureID TextureID() override;
        void _impl_StoreTexture(int width, int height, unsigned char* image_data_rgba) override;

        GLuint TextureId = 0;
    };
}

#endif // #ifdef HELLOIMGUI_HAS_OPENGL
