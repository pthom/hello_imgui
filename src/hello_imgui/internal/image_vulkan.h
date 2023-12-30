#pragma once
#ifdef HELLOIMGUI_HAS_VULKAN

#include "image_abstract.h"
#include <vulkan/vulkan.h>
#include <memory>

namespace HelloImGui
{
    struct ImageVulkan: public ImageAbstract
    {
        ImageVulkan() = default;
        ~ImageVulkan() override;

        ImTextureID TextureID() override;
        void _impl_StoreTexture(int width, int height, unsigned char* image_data_rgba) override;

        // Specific to Vulkan
        VkDescriptorSet DS;
        static constexpr int Channels = 4; // We intentionally only support RGBA for now
        VkImageView     ImageView = nullptr;
        VkImage         Image = nullptr;
        VkDeviceMemory  ImageMemory = nullptr;
        VkSampler       Sampler = nullptr;
        VkBuffer        UploadBuffer = nullptr;
        VkDeviceMemory  UploadBufferMemory = nullptr;
    };
}

#endif // #ifdef HELLOIMGUI_HAS_VULKAN
