#pragma once
#ifdef HELLOIMGUI_HAS_VULKAN

#include "imgui_impl_vulkan.h"
#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"

#include <vulkan/vulkan.h>

// For more info, see "Anatomy of an ImGui app lifecycle (cf ImGui examples)", in rendering_callbacks.h

struct GLFWwindow;
struct SDL_Window;


namespace HelloImGui
{
    // Functions from imgui examples/example_glfw_vulkan/main.cpp
    namespace VulkanSetup
    {
        void check_vk_result(VkResult err);
        VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
        bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension);
        VkPhysicalDevice SetupVulkan_SelectPhysicalDevice();
        void SetupVulkan(ImVector<const char*> instance_extensions);
        void SetupVulkanWindow(ImGui_ImplVulkanH_Window* wd, VkSurfaceKHR surface, int width, int height);
        void CleanupVulkan();
        void CleanupVulkanWindow();
        void FrameRender(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data);
        void FramePresent(ImGui_ImplVulkanH_Window* wd);
    }

    // Data
    struct VulkanGlobals
    {
        //gVkGlobals
        VkAllocationCallbacks*   Allocator = nullptr;
        VkInstance               Instance = VK_NULL_HANDLE;
        VkPhysicalDevice         PhysicalDevice = VK_NULL_HANDLE;
        VkDevice                 Device = VK_NULL_HANDLE;
        uint32_t                 QueueFamily = (uint32_t)-1;
        VkQueue                  Queue = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT DebugReport = VK_NULL_HANDLE;
        VkPipelineCache          PipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool         DescriptorPool = VK_NULL_HANDLE;

        ImGui_ImplVulkanH_Window ImGuiMainWindowData;
        int                      MinImageCount = 2;
        bool                     SwapChainRebuild = false;

        // The maximum number of image sampler descriptor and descriptor set is set at startup
        // Yoy may need to increase these values if you use a lot of images in your application.
        uint32_t                 PoolCreateInfo_PoolSizes = 100;
        uint32_t                 PoolCreateInfo_MaxSets = 100;
    };
    VulkanGlobals& GetVulkanGlobals();

    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapVulkanBuffers();

    RenderingCallbacksPtr PrepareBackendCallbacksCommonVulkan();

#ifdef HELLOIMGUI_USE_GLFW3
    RenderingCallbacksPtr CreateBackendCallbacks_GlfwVulkan();
    void PrepareGlfwForVulkan(GLFWwindow* window);
#endif
#ifdef HELLOIMGUI_USE_SDL2
    RenderingCallbacksPtr CreateBackendCallbacks_SdlVulkan();
    void PrepareSdlForVulkan(SDL_Window* window);
#endif

}

#endif // HELLOIMGUI_HAS_VULKAN
