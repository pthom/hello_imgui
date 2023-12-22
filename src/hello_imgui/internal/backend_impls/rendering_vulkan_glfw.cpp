#ifdef HELLOIMGUI_HAS_VULKAN
#include "rendering_vulkan.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#ifdef HELLOIMGUI_USE_GLFW3
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

#include <vulkan/vulkan.h>

#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"


namespace HelloImGui
{
    //  Impl of RenderingCallbacks_Impl_SwapBuffers
    void SwapVulkanBuffers()
    {
        auto & gVkGlobals = HelloImGui::GetVulkanGlobals();
        ImGui_ImplVulkanH_Window* wd = &gVkGlobals.ImGuiMainWindowData;

        ImDrawData* main_draw_data = ImGui::GetDrawData();
        const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);

        // Present Main Platform Window
        if (!main_is_minimized)
            HelloImGui::VulkanSetup::FramePresent(wd);
    }

    // Below is implementation of RenderingCallbacks_Prepare_WithWindow_PreImGuiInit
    void PrepareGlfwForVulkan_WithWindow_PreImGuiInit(GLFWwindow* window)
    {
        auto& gVkGlobals = HelloImGui::GetVulkanGlobals();

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

    void PrepareGlfwForVulkan_PosImGuiInit()
    {
        auto & gVkGlobals = HelloImGui::GetVulkanGlobals();
        auto window = HelloImGui::GetRunnerParams()->backendPointers.glfwWindow;
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


    RenderingCallbacksPtr CreateBackendCallbacks_GlfwVulkan()
    {
        auto callbacks = std::make_shared<RenderingCallbacks>();

        callbacks->Impl_NewFrame_3D          = []
        {
            auto & gVkGlobals = HelloImGui::GetVulkanGlobals();
            auto window = HelloImGui::GetRunnerParams()->backendPointers.glfwWindow;

            // Resize swap chain?
            if (gVkGlobals.SwapChainRebuild)
            {
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                if (width > 0 && height > 0)
                {
                    ImGui_ImplVulkan_SetMinImageCount(gVkGlobals.MinImageCount);
                    ImGui_ImplVulkanH_CreateOrResizeWindow(gVkGlobals.Instance, gVkGlobals.PhysicalDevice,
                                                           gVkGlobals.Device, &gVkGlobals.ImGuiMainWindowData,
                                                           gVkGlobals.QueueFamily, gVkGlobals.Allocator, width, height,
                                                           gVkGlobals.MinImageCount);
                    gVkGlobals.ImGuiMainWindowData.FrameIndex = 0;
                    gVkGlobals.SwapChainRebuild = false;
                }
            }

            // Start the Dear ImGui frame
            ImGui_ImplVulkan_NewFrame();
            // ImGui_ImplGlfw_NewFrame();
        };

        callbacks->Impl_Frame_3D_ClearColor  = [] (ImVec4) {};

        callbacks->Impl_RenderDrawData_To_3D = []
        {
            auto & gVkGlobals = HelloImGui::GetVulkanGlobals();
            ImGui_ImplVulkanH_Window* wd = &gVkGlobals.ImGuiMainWindowData;
            ImVec4 clear_color = HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor;

            ImDrawData* main_draw_data = ImGui::GetDrawData();
            const bool main_is_minimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            if (!main_is_minimized)
                HelloImGui::VulkanSetup::FrameRender(wd, main_draw_data);
        };

        callbacks->Impl_Shutdown_3D          = []
        {
            auto & gVkGlobals = HelloImGui::GetVulkanGlobals();
            VkResult err = vkDeviceWaitIdle(gVkGlobals.Device);
            HelloImGui::VulkanSetup::check_vk_result(err);
            ImGui_ImplVulkan_Shutdown();

            //ImGui_ImplGlfw_Shutdown();
            // ImGui::DestroyContext();

            HelloImGui::VulkanSetup::CleanupVulkanWindow();
            HelloImGui::VulkanSetup::CleanupVulkan();
        };

        // callbacks->Impl_ScreenshotRgb_3D     = [] { return ImageBuffer{}; };
        // callbacks->Impl_GetFrameBufferSize;   //= [] { return ScreenSize{0, 0}; };

        return callbacks;
    }

}

#endif // HELLOIMGUI_HAS_VULKAN
