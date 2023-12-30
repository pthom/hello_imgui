#pragma once
#ifdef HELLOIMGUI_HAS_VULKAN

#include <vulkan/vulkan.h>
#include <memory>

namespace HelloImGui
{
    struct ImageVk
    {
        ImageVk(const char *assetPath);
        ~ImageVk();

        VkDescriptorSet DS;         // Descriptor set: this is what you'll pass to Image()
        int             Width = 0;
        int             Height = 0;
        int             Channels = 4;

        // Need to keep track of these to properly cleanup
        VkImageView     ImageView = nullptr;
        VkImage         Image = nullptr;
        VkDeviceMemory  ImageMemory = nullptr;
        VkSampler       Sampler = nullptr;
        VkBuffer        UploadBuffer = nullptr;
        VkDeviceMemory  UploadBufferMemory = nullptr;
    };

    using ImageVkPtr = std::shared_ptr<ImageVk>;
}

#endif // #ifdef HELLOIMGUI_HAS_VULKAN
