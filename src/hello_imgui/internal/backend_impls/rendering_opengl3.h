#pragma once
#ifdef HELLOIMGUI_HAS_OPENGL

#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"

namespace HelloImGui
{
    RenderingCallbacksPtr CreateBackendCallbacks_OpenGl3();
}

#endif