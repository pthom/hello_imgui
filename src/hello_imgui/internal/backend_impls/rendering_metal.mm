#ifdef HELLOIMGUI_HAS_METAL
#include "rendering_metal.h"

namespace HelloImGui
{
    MetalGlobals& GetMetalGlobals()
    {
        static MetalGlobals sMetalGlobals;
        return sMetalGlobals;
    }
}

#endif // HELLOIMGUI_HAS_METAL