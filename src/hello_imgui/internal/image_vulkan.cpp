#ifdef HELLOIMGUI_HAS_VULKAN
#include "image_vulkan.h"

#include "imgui.h"
#include "hello_imgui/internal/backend_impls/rendering_vulkan.h"

// Inspired from https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#example-for-vulkan-users
// WARNING: THIS IS ONE WAY TO DO THIS AMONG MANY, and provided for informational purpose.
// Unfortunately due to the nature of Vulkan, it is not really possible
// to provide a function that will work in all setups and codebases.

namespace HelloImGui
{

    // Helper function to find Vulkan memory type bits. See ImGui_ImplVulkan_MemoryType() in imgui_impl_vulkan.cpp
    uint32_t findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        auto& g_PhysicalDevice = GetVulkanGlobals().PhysicalDevice;

        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(g_PhysicalDevice, &mem_properties);

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;

        return 0xFFFFFFFF; // Unable to find memoryType
    }

    void ImageVulkan::_impl_StoreTexture(int width, int height, unsigned char* image_data_rgba)
    {
        VulkanGlobals& vkGlobals = GetVulkanGlobals();
        
        auto &self = *this;

        // Calculate allocation size (in number of bytes)
        size_t image_size = width * height * self.Channels;

        VkResult err;

        // Create the Vulkan image.
        {
            VkImageCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            info.imageType = VK_IMAGE_TYPE_2D;
            info.format = VK_FORMAT_R8G8B8A8_UNORM;
            info.extent.width = width;
            info.extent.height = height;
            info.extent.depth = 1;
            info.mipLevels = 1;
            info.arrayLayers = 1;
            info.samples = VK_SAMPLE_COUNT_1_BIT;
            info.tiling = VK_IMAGE_TILING_OPTIMAL;
            info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
            info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            err = vkCreateImage(vkGlobals.Device, &info, vkGlobals.Allocator, &self.Image);
            VulkanSetup::check_vk_result(err);
            VkMemoryRequirements req;
            vkGetImageMemoryRequirements(vkGlobals.Device, self.Image, &req);
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            err = vkAllocateMemory(vkGlobals.Device, &alloc_info, vkGlobals.Allocator, &self.ImageMemory);
            VulkanSetup::check_vk_result(err);
            err = vkBindImageMemory(vkGlobals.Device, self.Image, self.ImageMemory, 0);
            VulkanSetup::check_vk_result(err);
        }

        // Create the Image View
        {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = self.Image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = VK_FORMAT_R8G8B8A8_UNORM;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.layerCount = 1;
            err = vkCreateImageView(vkGlobals.Device, &info, vkGlobals.Allocator, &self.ImageView);
            VulkanSetup::check_vk_result(err);
        }

        // Create Sampler
        {
            VkSamplerCreateInfo sampler_info{};
            sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_info.magFilter = VK_FILTER_LINEAR;
            sampler_info.minFilter = VK_FILTER_LINEAR;
            sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // outside image bounds just use border color
            sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.minLod = -1000;
            sampler_info.maxLod = 1000;
            sampler_info.maxAnisotropy = 1.0f;
            err = vkCreateSampler(vkGlobals.Device, &sampler_info, vkGlobals.Allocator, &self.Sampler);
            VulkanSetup::check_vk_result(err);
        }

        // Create Descriptor Set using ImGUI's implementation
        self.DS = ImGui_ImplVulkan_AddTexture(self.Sampler, self.ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Create Upload Buffer
        {
            VkBufferCreateInfo buffer_info = {};
            buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_info.size = image_size;
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            err = vkCreateBuffer(vkGlobals.Device, &buffer_info, vkGlobals.Allocator, &self.UploadBuffer);
            VulkanSetup::check_vk_result(err);
            VkMemoryRequirements req;
            vkGetBufferMemoryRequirements(vkGlobals.Device, self.UploadBuffer, &req);
            VkMemoryAllocateInfo alloc_info = {};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = req.size;
            alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            err = vkAllocateMemory(vkGlobals.Device, &alloc_info, vkGlobals.Allocator, &self.UploadBufferMemory);
            VulkanSetup::check_vk_result(err);
            err = vkBindBufferMemory(vkGlobals.Device, self.UploadBuffer, self.UploadBufferMemory, 0);
            VulkanSetup::check_vk_result(err);
        }

        // Upload to Buffer:
        {
            void* map = NULL;
            err = vkMapMemory(vkGlobals.Device, self.UploadBufferMemory, 0, image_size, 0, &map);
            VulkanSetup::check_vk_result(err);
            memcpy(map, image_data_rgba, image_size);
            VkMappedMemoryRange range[1] = {};
            range[0].sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            range[0].memory = self.UploadBufferMemory;
            range[0].size = image_size;
            err = vkFlushMappedMemoryRanges(vkGlobals.Device, 1, range);
            VulkanSetup::check_vk_result(err);
            vkUnmapMemory(vkGlobals.Device, self.UploadBufferMemory);
        }

        // Create a command buffer that will perform following steps when hit in the command queue.
        // TODO: this works in the example, but may need input if this is an acceptable way to access the pool/create the command buffer.
        VkCommandPool command_pool = vkGlobals.ImGuiMainWindowData.Frames[vkGlobals.ImGuiMainWindowData.FrameIndex].CommandPool;
        VkCommandBuffer command_buffer;
        {
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandPool = command_pool;
            alloc_info.commandBufferCount = 1;

            err = vkAllocateCommandBuffers(vkGlobals.Device, &alloc_info, &command_buffer);
            VulkanSetup::check_vk_result(err);

            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(command_buffer, &begin_info);
            VulkanSetup::check_vk_result(err);
        }

        // Copy to Image
        {
            VkImageMemoryBarrier copy_barrier[1] = {};
            copy_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            copy_barrier[0].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            copy_barrier[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            copy_barrier[0].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            copy_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            copy_barrier[0].image = self.Image;
            copy_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copy_barrier[0].subresourceRange.levelCount = 1;
            copy_barrier[0].subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, copy_barrier);

            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.layerCount = 1;
            region.imageExtent.width = self.Width;
            region.imageExtent.height = self.Height;
            region.imageExtent.depth = 1;
            vkCmdCopyBufferToImage(command_buffer, self.UploadBuffer, self.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            VkImageMemoryBarrier use_barrier[1] = {};
            use_barrier[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            use_barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            use_barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            use_barrier[0].oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            use_barrier[0].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            use_barrier[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            use_barrier[0].image = self.Image;
            use_barrier[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            use_barrier[0].subresourceRange.levelCount = 1;
            use_barrier[0].subresourceRange.layerCount = 1;
            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, use_barrier);
        }

        // End command buffer
        {
            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            err = vkEndCommandBuffer(command_buffer);
            VulkanSetup::check_vk_result(err);
            err = vkQueueSubmit(vkGlobals.Queue, 1, &end_info, VK_NULL_HANDLE);
            VulkanSetup::check_vk_result(err);
            err = vkDeviceWaitIdle(vkGlobals.Device);
            VulkanSetup::check_vk_result(err);
        }


        //this->imTextureId = (ImTextureID)(intptr_t)vkImageView;
    }

    // Destructor to clean up Vulkan resources
    ImageVulkan::~ImageVulkan()
    {
        VulkanGlobals& vkGlobals = GetVulkanGlobals();
        auto& self = *this;

        vkFreeMemory(vkGlobals.Device, self.UploadBufferMemory, nullptr);
        vkDestroyBuffer(vkGlobals.Device, self.UploadBuffer, nullptr);
        vkDestroySampler(vkGlobals.Device, self.Sampler, nullptr);
        vkDestroyImageView(vkGlobals.Device, self.ImageView, nullptr);
        vkDestroyImage(vkGlobals.Device, self.Image, nullptr);
        vkFreeMemory(vkGlobals.Device, self.ImageMemory, nullptr);
        ImGui_ImplVulkan_RemoveTexture(self.DS);
    }

    ImTextureID ImageVulkan::TextureID()
    {
        return (ImTextureID)DS;
        //return (ImTextureID)(intptr_t)ImageView;
    }


}

#endif // #ifdef HELLOIMGUI_HAS_VULKAN
