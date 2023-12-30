#pragma once
#ifdef HELLOIMGUI_HAS_METAL

#include "image_abstract.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>
#include <memory>

namespace HelloImGui
{
    struct ImageMetal: public ImageAbstract
    {
        ImageMetal() = default;
        ~ImageMetal() override;

        ImTextureID TextureID() override;
        void _impl_StoreTexture(int width, int height, unsigned char* image_data_rgba) override;

        id<MTLTexture> Texture;
    };
}

#endif // #ifdef HELLOIMGUI_HAS_METAL
