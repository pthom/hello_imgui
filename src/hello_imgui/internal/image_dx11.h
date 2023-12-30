#pragma once
#ifdef HELLOIMGUI_HAS_DIRECTX11

#include <memory>
#include <d3d11.h>

namespace HelloImGui
{
    struct ImageDx11
    {
        ImageDx11(int width, int height, unsigned char* image_data_rgba);
        ~ImageDx11();

        int Width = 0;
        int Height = 0;
        ID3D11ShaderResourceView* ShaderResourceView = nullptr;
    };

    using ImageDx11Ptr = std::shared_ptr<ImageDx11>;
}

#endif // #ifdef HELLOIMGUI_HAS_DIRECTX11
