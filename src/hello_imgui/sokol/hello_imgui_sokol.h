#pragma once
#ifdef HELLOIMGUI_USE_SOKOL
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/runner_params.h"
#include "sokol/sokol_app.h"

namespace HelloImGui
{
namespace Sokol
{

sapp_desc make_sapp_desc(HelloImGui::RunnerParams& params);

} // namespace Sokol
} // namespace HelloImGui

#endif // HELLOIMGUI_USE_SOKOL