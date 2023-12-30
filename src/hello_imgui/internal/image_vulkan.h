#pragma once
#ifdef HELLOIMGUI_HAS_VULKAN

#include <vulkan/vulkan.h>
#include <memory>

namespace HelloImGui
{
    struct ImageVk
    {
        ImageVk(int width, int height, unsigned char* image_data_rgba);
        ~ImageVk();

        VkDescriptorSet DS;         // Descriptor set: this is what you'll pass to Image()
        int             Width = 0;
        int             Height = 0;
        static constexpr int Channels = 4; // We intentionally only support RGBA for now

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
