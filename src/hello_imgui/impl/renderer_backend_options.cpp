#include "hello_imgui/renderer_backend_options.h"

namespace HelloImGui
{

// currently, only the metal backend has support for this
#ifndef HELLOIMGUI_HAS_METAL
bool hasEdrSupport() { return false; }
#endif

}  // namespace HelloImGui
