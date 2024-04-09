#pragma once
#ifdef HELLOIMGUI_HAS_NULL

#include "hello_imgui/internal/backend_impls/rendering_callbacks.h"

namespace HelloImGui
{
    RenderingCallbacksPtr CreateBackendCallbacks_Null();
}

#endif