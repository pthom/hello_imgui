#pragma once
#ifdef HELLOIMGUI_HAS_METAL

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <memory>

namespace HelloImGui
{
    struct ImageMetal
    {
        ImageMetal(int width, int height, unsigned char* image_data_rgba);
        ~ImageMetal();

        id<MTLTexture> Texture;
        int Width;
        int Height;
    };

    using ImageMetalPtr = std::shared_ptr<ImageMetal>;
}

#endif // #ifdef HELLOIMGUI_HAS_METAL
