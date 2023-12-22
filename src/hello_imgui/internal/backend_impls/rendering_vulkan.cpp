#ifdef HELLOIMGUI_HAS_VULKAN
#include "rendering_vulkan.h"

namespace HelloImGui
{
    VulkanGlobals& GetVulkanGlobals()
    {
        static VulkanGlobals sVulkanGlobals;
        return sVulkanGlobals;
    }
}

#endif // HELLOIMGUI_HAS_VULKAN
