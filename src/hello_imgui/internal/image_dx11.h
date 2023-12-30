#pragma once
#ifdef HELLOIMGUI_HAS_DIRECTX11

#include "image_abstract.h"
#include <memory>
#include <d3d11.h>

namespace HelloImGui
{
    struct ImageDx11: public ImageAbstract
    {
        ImageDx11() = default;
        ~ImageDx11() override;

        ImTextureID TextureID() override;
        void _impl_StoreTexture(int width, int height, unsigned char* image_data_rgba) override;

        ID3D11ShaderResourceView* ShaderResourceView = nullptr;
    };

    using ImageDx11Ptr = std::shared_ptr<ImageDx11>;
}

#endif // #ifdef HELLOIMGUI_HAS_DIRECTX11
