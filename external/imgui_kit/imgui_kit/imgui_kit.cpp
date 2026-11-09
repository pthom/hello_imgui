// imgui_kit: small, standalone additions to Dear ImGui
// https://github.com/pthom/imgui_kit (MIT license)

/*

Index of this file:
// [SECTION] Em units

*/

#include "imgui_kit.h"
#include "imgui_kit_internal.h"

namespace ImGuiKit
{

//-----------------------------------------------------------------------------
// [SECTION] Em units
//-----------------------------------------------------------------------------

float EmSize()
{
    IM_ASSERT(ImGui::GetCurrentContext() != nullptr && "EmSize() needs a Dear ImGui context");
    return ImGui::GetFontSize();
}

float EmSize(float lines)
{
    return EmSize() * lines;
}

ImVec2 EmToVec2(float x, float y)
{
    float k = EmSize();
    return ImVec2(k * x, k * y);
}

ImVec2 EmToVec2(ImVec2 v)
{
    return EmToVec2(v.x, v.y);
}

ImVec2 PixelsToEm(ImVec2 pixels)
{
    float k = EmSize();
    return ImVec2(pixels.x / k, pixels.y / k);
}

float PixelSizeToEm(float pixel_size)
{
    return pixel_size / EmSize();
}

} // namespace ImGuiKit

//-----------------------------------------------------------------------------
// The kit's state, one per Dear ImGui context
//-----------------------------------------------------------------------------
// It hangs on a Shutdown hook of the Dear ImGui context (its UserData), as the Test Engine's: the hook frees it with
// the context (see imgui_kit_internal.h's top). The hooks are per context: no global (GImGui may be thread-local).

// The hook frees the state with its context
static void ShutdownHook(ImGuiContext*, ImGuiContextHook* h)
{
    IM_DELETE((ImGuiKitContext*)h->UserData);
    h->UserData = nullptr;
}

namespace ImGuiKit
{
ImGuiKitContext& GetCurrentContext()
{
    ImGuiContext* ctx = GImGui;
    IM_ASSERT(ctx != nullptr && "imgui_kit needs a Dear ImGui context: call ImGui::CreateContext() first");
    const ImGuiID owner = 0x4B49544B;  // "KITK": the owner of the kit's hook (any ID no one else uses)
    for (ImGuiContextHook& hook : ctx->Hooks)
        if (hook.Owner == owner && hook.Type == ImGuiContextHookType_Shutdown)
            return *(ImGuiKitContext*)hook.UserData;
    ImGuiKitContext* state = IM_NEW(ImGuiKitContext)();
    ImGuiContextHook hook;
    hook.Type = ImGuiContextHookType_Shutdown;
    hook.Owner = owner;
    hook.UserData = state;
    hook.Callback = ShutdownHook;
    ImGui::AddContextHook(ctx, &hook);
    return *state;
}

// A request at this frame or the last one: an app asks after rendering, and one more frame lets the animation land
bool IsAnimating()
{
    const ImGuiKitContext& kit = GetCurrentContext();
    const bool widgets = kit.AnimationFrame >= 0 && GImGui->FrameCount - kit.AnimationFrame <= 1;
    return widgets || ImGuiTouch::IsAnimating();
}

void RequestAnimation() { GetCurrentContext().AnimationFrame = GImGui->FrameCount; }
} // namespace ImGuiKit
