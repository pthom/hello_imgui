#ifdef HELLOIMGUI_HAS_DIRECTX11
#include "image_dx11.h"

#include "imgui.h"
#include "hello_imgui/internal/backend_impls/rendering_dx11.h"


namespace HelloImGui
{
    void ImageDx11::_impl_StoreTexture(int width, int height, unsigned char* image_data_rgba)
    {
        auto& gDx11Globals =  GetDx11Globals();

        // Create texture
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D *pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = image_data_rgba;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        gDx11Globals.pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        gDx11Globals.pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &ShaderResourceView);
        pTexture->Release();
    }

    ImageDx11::~ImageDx11()
    {
        if (ShaderResourceView)
            ShaderResourceView->Release();
    }

    ImTextureID ImageDx11::TextureID()
    {
        return (ImTextureID)(uintptr_t)(ShaderResourceView);
    }

}

#endif // #ifdef HELLOIMGUI_HAS_DIRECTX11
