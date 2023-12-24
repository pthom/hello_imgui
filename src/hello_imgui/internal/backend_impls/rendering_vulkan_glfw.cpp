#if defined(HELLOIMGUI_HAS_VULKAN) && defined(HELLOIMGUI_USE_GLFW3)
#include "rendering_vulkan.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"


namespace HelloImGui
{
    // Below is implementation of RenderingCallbacks_LinkWindowingToRenderingBackend
    void PrepareGlfwForVulkan(GLFWwindow* window)
    {
        auto& gVkGlobals = HelloImGui::GetVulkanGlobals();

        {
            if (!glfwVulkanSupported())
            {
                IM_ASSERT(0 && "GLFW: Vulkan Not Supported");
                exit(1);
            }

            ImVector<const char*> extensions;
            uint32_t extensions_count = 0;
            const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
            for (uint32_t i = 0; i < extensions_count; i++)
                extensions.push_back(glfw_extensions[i]);
            HelloImGui::VulkanSetup::SetupVulkan(extensions);

            // Create Window Surface
            VkSurfaceKHR surface;
            VkResult err = glfwCreateWindowSurface(gVkGlobals.Instance, window, gVkGlobals.Allocator, &surface);
            HelloImGui::VulkanSetup::check_vk_result(err);

            // Create Framebuffers
            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            ImGui_ImplVulkanH_Window* wd = &gVkGlobals.ImGuiMainWindowData;
            HelloImGui::VulkanSetup::SetupVulkanWindow(wd, surface, w, h);
        }

        {
            ImGui_ImplVulkanH_Window* wd = &gVkGlobals.ImGuiMainWindowData;

            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForVulkan(window, true);
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = gVkGlobals.Instance;
            init_info.PhysicalDevice = gVkGlobals.PhysicalDevice;
            init_info.Device = gVkGlobals.Device;
            init_info.QueueFamily = gVkGlobals.QueueFamily;
            init_info.Queue = gVkGlobals.Queue;
            init_info.PipelineCache = gVkGlobals.PipelineCache;
            init_info.DescriptorPool = gVkGlobals.DescriptorPool;
            init_info.Subpass = 0;
            init_info.MinImageCount = gVkGlobals.MinImageCount;
            init_info.ImageCount = wd->ImageCount;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = gVkGlobals.Allocator;
            init_info.CheckVkResultFn = HelloImGui::VulkanSetup::check_vk_result;
            ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
        }
    }


    RenderingCallbacksPtr CreateBackendCallbacks_GlfwVulkan()
    {
        auto callbacks = PrepareBackendCallbacksCommonVulkan();

        callbacks->Impl_GetFrameBufferSize = []
        {
            auto window = (GLFWwindow *) HelloImGui::GetRunnerParams()->backendPointers.glfwWindow;
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            return ScreenSize{width, height};
        };

        return callbacks;
    }

}

#endif // #if defined(HELLOIMGUI_HAS_VULKAN) && defined(HELLOIMGUI_USE_GLFW3)
