#pragma once
#ifdef HELLOIMGUI_HAS_DIRECTX11

#include "hello_imgui/texture_gpu.h"
#include <memory>
#include <d3d11.h>

namespace HelloImGui
{
    struct TextureGpuDx11: public TextureGpu
    {
        TextureGpuDx11() = default;
        ~TextureGpuDx11() override;

        ImTextureID TextureID() override;
        void _impl_StoreTexture(int width, int height, unsigned char* image_data_rgba) override;

        ID3D11ShaderResourceView* ShaderResourceView = nullptr;
    };

    using TextureGpuDx11Ptr = std::shared_ptr<TextureGpuDx11>;
}

#endif // #ifdef HELLOIMGUI_HAS_DIRECTX11
