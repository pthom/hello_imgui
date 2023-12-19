#pragma once
#ifdef HELLOIMGUI_HAS_METAL

#ifdef HELLOIMGUI_USE_SDL2
struct SDL_Window;
#endif

#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"

namespace HelloImGui
{
    RenderingCallbacks CreateBackendCallbacks_Metal();

#ifdef HELLOIMGUI_USE_SDL2
    void PrepareSdLForMetal_WithWindow_PreImGuiInit(SDL_Window* window);
    void PrepareSdLForMetal_PosImGuiInit();
    void SwapSdlMetalBuffers();
#endif
}

#endif // HELLOIMGUI_HAS_METAL
