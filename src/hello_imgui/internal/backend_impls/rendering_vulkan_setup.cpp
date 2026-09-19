#ifdef HELLOIMGUI_HAS_VULKAN
#include "rendering_vulkan.h"

#include "imgui_impl_vulkan.h"

#ifdef HELLOIMGUI_USE_GLFW3
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <vulkan/vulkan.h>

#include "hello_imgui/hello_imgui_logger.h"

#include <stdexcept>
#include <string>


// Validation layers: enabled by the CMake option HELLOIMGUI_VULKAN_VALIDATION (and in MSVC debug builds)
#if defined(_DEBUG) || defined(HELLOIMGUI_VULKAN_VALIDATION)
#define IMGUI_VULKAN_DEBUG_REPORT
#endif


namespace HelloImGui::VulkanSetup
{
static const char* VkResultName(VkResult err)
{
    switch (err)
    {
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        default: return "see VkResult in vulkan_core.h";
    }
}

bool log_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return true;
    fprintf(stderr, "[vulkan] Error: VkResult = %d (%s)\n", err, VkResultName(err));
    return err > 0;
}

void check_vk_result(VkResult err)
{
    // Errors (negative values) throw: AbstractRunner::Run() will tear down the app and rethrow
    // (a Python user then gets a RuntimeError instead of a dead interpreter)
    if (!log_vk_result(err))
        throw std::runtime_error(std::string("Vulkan error: ") + VkResultName(err) + " (" + std::to_string(err) + ")");
}

#ifdef IMGUI_VULKAN_DEBUG_REPORT
VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension)
{
    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;
    return false;
}

void SetupVulkan(ImVector<const char*> instance_extensions)
{
    auto& gVkGlobals = HelloImGui::GetVulkanGlobals();

    VkResult err;

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
        check_vk_result(err);

        // Enable required extensions
        if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
            instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
        {
            instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

        // Enabling validation layers
#ifdef IMGUI_VULKAN_DEBUG_REPORT
        const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
        instance_extensions.push_back("VK_EXT_debug_report");
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
        create_info.ppEnabledExtensionNames = instance_extensions.Data;
        err = vkCreateInstance(&create_info, gVkGlobals.Allocator, &gVkGlobals.Instance);
        check_vk_result(err);

        // Setup the debug report callback
#ifdef IMGUI_VULKAN_DEBUG_REPORT
        auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(gVkGlobals.Instance, "vkCreateDebugReportCallbackEXT");
        IM_ASSERT(vkCreateDebugReportCallbackEXT != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
        debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debug_report_ci.pfnCallback = debug_report;
        debug_report_ci.pUserData = nullptr;
        err = vkCreateDebugReportCallbackEXT(gVkGlobals.Instance, &debug_report_ci, gVkGlobals.Allocator, &gVkGlobals.DebugReport);
        check_vk_result(err);
#endif
    }

    // Select Physical Device (GPU)
    gVkGlobals.PhysicalDevice = ImGui_ImplVulkanH_SelectPhysicalDevice(gVkGlobals.Instance);
    IM_ASSERT(gVkGlobals.PhysicalDevice != VK_NULL_HANDLE);

    // Select graphics queue family
    gVkGlobals.QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(gVkGlobals.PhysicalDevice);
    IM_ASSERT(gVkGlobals.QueueFamily != (uint32_t)-1);

    // Create Logical Device (with 1 queue)
    {
        ImVector<const char*> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count;
        ImVector<VkExtensionProperties> properties;
        vkEnumerateDeviceExtensionProperties(gVkGlobals.PhysicalDevice, nullptr, &properties_count, nullptr);
        properties.resize(properties_count);
        vkEnumerateDeviceExtensionProperties(gVkGlobals.PhysicalDevice, nullptr, &properties_count, properties.Data);
        // Required when available (MoltenVK). Not using VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME, which is only defined by vulkan_beta.h
        if (IsExtensionAvailable(properties, "VK_KHR_portability_subset"))
            device_extensions.push_back("VK_KHR_portability_subset");

        const float queue_priority[] = { 1.0f };
        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = gVkGlobals.QueueFamily;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;
        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
        create_info.ppEnabledExtensionNames = device_extensions.Data;
        err = vkCreateDevice(gVkGlobals.PhysicalDevice, &create_info, gVkGlobals.Allocator, &gVkGlobals.Device);
        check_vk_result(err);
        vkGetDeviceQueue(gVkGlobals.Device, gVkGlobals.QueueFamily, 0, &gVkGlobals.Queue);
    }

    // Create Descriptor Pool
    // Since imgui 1.93, the Vulkan backend uses separate image views + samplers (instead of combined image samplers):
    // one VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE descriptor set per texture, plus a few VK_DESCRIPTOR_TYPE_SAMPLER ones.
    {
        VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, gVkGlobals.PoolCreateInfo_PoolSizes },
                { VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE },
            };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = gVkGlobals.PoolCreateInfo_MaxSets + IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(gVkGlobals.Device, &pool_info, gVkGlobals.Allocator, &gVkGlobals.DescriptorPool);
        check_vk_result(err);
    }
}

VkPresentModeKHR SelectPresentMode(ImGui_ImplVulkanH_Window* wd)
{
    // - vsync: FIFO (always available)
    // - no vsync: MAILBOX or IMMEDIATE, when the device provides them (otherwise, FIFO, i.e. vsync remains)
    auto& gVkGlobals = HelloImGui::GetVulkanGlobals();
    if (gVkGlobals.VsyncToMonitor)
    {
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
        return ImGui_ImplVulkanH_SelectPresentMode(gVkGlobals.PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    }
    else
    {
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
        return ImGui_ImplVulkanH_SelectPresentMode(gVkGlobals.PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
    }
}

// All the ImGui_ImplVulkanH_XXX structures/functions are optional helpers used by the demo.
// Your real engine/app may not use them.
void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height)
{
    auto& gVkGlobals = HelloImGui::GetVulkanGlobals();

    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;
    vkGetPhysicalDeviceSurfaceSupportKHR(gVkGlobals.PhysicalDevice, gVkGlobals.QueueFamily, wd->Surface, &res);
    if (res != VK_TRUE)
    {
        fprintf(stderr, "Error no WSI support on physical device 0\n");
        exit(-1);
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
    const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(gVkGlobals.PhysicalDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

    // Select Present Mode
    wd->PresentMode = SelectPresentMode(wd);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    IM_ASSERT(gVkGlobals.MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(
        gVkGlobals.Instance, gVkGlobals.PhysicalDevice, gVkGlobals.Device, wd,
        gVkGlobals.QueueFamily, gVkGlobals.Allocator,
        width, height,
        gVkGlobals.MinImageCount,
        0  // image_usage
        );
}

void CleanupVulkan()
{
    auto& gVkGlobals = HelloImGui::GetVulkanGlobals();

    vkDestroyDescriptorPool(gVkGlobals.Device, gVkGlobals.DescriptorPool, gVkGlobals.Allocator);

#ifdef IMGUI_VULKAN_DEBUG_REPORT
    // Remove the debug report callback
    auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(gVkGlobals.Instance, "vkDestroyDebugReportCallbackEXT");
    vkDestroyDebugReportCallbackEXT(gVkGlobals.Instance, gVkGlobals.DebugReport, gVkGlobals.Allocator);
#endif // IMGUI_VULKAN_DEBUG_REPORT

    vkDestroyDevice(gVkGlobals.Device, gVkGlobals.Allocator);
    vkDestroyInstance(gVkGlobals.Instance, gVkGlobals.Allocator);
}

void CleanupVulkanWindow()
{
    auto& gVkGlobals = HelloImGui::GetVulkanGlobals();
    ImGui_ImplVulkanH_DestroyWindow(
        gVkGlobals.Instance,
        gVkGlobals.Device,
        &gVkGlobals.ImGuiMainWindowData,
        gVkGlobals.Allocator);
    // Since imgui 1.92.6, ImGui_ImplVulkanH_DestroyWindow does not destroy the surface (it is user provided)
    vkDestroySurfaceKHR(gVkGlobals.Instance, gVkGlobals.ImGuiMainWindowData.Surface, gVkGlobals.Allocator);
}

void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data)
{
    auto& gVkGlobals = HelloImGui::GetVulkanGlobals();
    VkResult err;

    VkSemaphore image_acquired_semaphore  = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    err = vkAcquireNextImageKHR(gVkGlobals.Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        gVkGlobals.SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)  // when suboptimal, the image was acquired: render it
        check_vk_result(err);

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        err = vkWaitForFences(gVkGlobals.Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);    // wait indefinitely instead of periodically checking
        check_vk_result(err);

        err = vkResetFences(gVkGlobals.Device, 1, &fd->Fence);
        check_vk_result(err);
    }
    {
        err = vkResetCommandPool(gVkGlobals.Device, fd->CommandPool, 0);
        check_vk_result(err);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
        check_vk_result(err);
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        err = vkEndCommandBuffer(fd->CommandBuffer);
        check_vk_result(err);
        err = vkQueueSubmit(gVkGlobals.Queue, 1, &info, fd->Fence);
        check_vk_result(err);
    }
}

void FramePresent(ImGui_ImplVulkanH_Window* wd)
{
    auto& gVkGlobals = HelloImGui::GetVulkanGlobals();
    if (gVkGlobals.SwapChainRebuild)
        return;
    VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;
    VkResult err = vkQueuePresentKHR(gVkGlobals.Queue, &info);
    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        gVkGlobals.SwapChainRebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return;
    if (err != VK_SUBOPTIMAL_KHR)
        check_vk_result(err);
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount; // Now we can use the next set of semaphores
}

} // namespace HelloImGui::VulkanSetup


#endif // HELLOIMGUI_HAS_VULKAN
