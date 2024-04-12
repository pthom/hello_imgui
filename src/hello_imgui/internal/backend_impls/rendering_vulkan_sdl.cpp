#if defined(HELLOIMGUI_HAS_VULKAN) && defined(HELLOIMGUI_USE_SDL2)
#include "rendering_vulkan.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl2.h"


#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/hello_imgui.h"

#if IMGUI_VERSION_NUM  >= 19030
// The API changed with this commit
// https://github.com/ocornut/imgui/commit/f80e65a406885beedf68856057b278343d5c1407
#define IMGUI_VULKAN_RENDER_PASS_IN_STRUCTURE
#endif

namespace HelloImGui
{
    // Below is implementation of RenderingCallbacks_LinkWindowingToRenderingBackend
    void PrepareSdlForVulkan(SDL_Window* window)
    {
        auto& gVkGlobals = HelloImGui::GetVulkanGlobals();

        {
            ImVector<const char*> extensions;
            uint32_t extensions_count = 0;
            SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, nullptr);
            extensions.resize(extensions_count);
            SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, extensions.Data);
            HelloImGui::VulkanSetup::SetupVulkan(extensions);

            // Create Window Surface
            VkSurfaceKHR surface;
            //VkResult err;
            if (SDL_Vulkan_CreateSurface(window, gVkGlobals.Instance, &surface) == 0)
            {
                IM_ASSERT(0 && "Failed to create Vulkan surface");
                exit(1);
            }

            // Create Framebuffers
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            ImGui_ImplVulkanH_Window* wd = &gVkGlobals.ImGuiMainWindowData;
            HelloImGui::VulkanSetup::SetupVulkanWindow(wd, surface, w, h);
        }

        {
            ImGui_ImplVulkanH_Window* wd = &gVkGlobals.ImGuiMainWindowData;
            ImGui_ImplSDL2_InitForVulkan(window);
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

#ifdef IMGUI_VULKAN_RENDER_PASS_IN_STRUCTURE
            init_info.RenderPass = wd->RenderPass;
            ImGui_ImplVulkan_Init(&init_info);
#else
            ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
#endif
        }
    }


    RenderingCallbacksPtr CreateBackendCallbacks_SdlVulkan()
    {
        auto callbacks = PrepareBackendCallbacksCommonVulkan();

        callbacks->Impl_GetFrameBufferSize = []
        {
            auto window = (SDL_Window *)HelloImGui::GetRunnerParams()->backendPointers.sdlWindow;
            int width, height;
            SDL_GetWindowSize(window, &width, &height);
            return ScreenSize{width, height};
        };

        return callbacks;
    }

}

#endif // #if defined(HELLOIMGUI_HAS_VULKAN) && defined(HELLOIMGUI_USE_SDL2)
