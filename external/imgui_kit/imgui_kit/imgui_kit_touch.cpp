// imgui_kit: the setup of the touch layer (ImGuiTouch) and of the wheel session, through Dear ImGui's context hooks
// https://github.com/pthom/imgui_kit (MIT license)
#include "imgui_kit.h"
#include "imgui_kit_internal.h"
#include "imgui_internal.h"  // AddContextHook

// Each step runs at a moment Dear ImGui names:
//   NewFramePre:  the wheel session notes the item that claimed the wheel (NewFrame() gives it back to a window)
//   NewFramePost: before any widget, the swipe, then the pinch; the wheel session's step, in the order of the setups
//   EndFramePre:  after the GUI, the widened scrollbar thumb (the frame's scroll is final)
//   RenderPost:   the draw data exists: the overscroll moves the content past its end
// A hook belongs to its context: Setup() installs the hooks of the current context, once (their owner tells).

namespace
{
    const ImGuiID kTouchHooksOwner = ImHashStr("ImGuiTouch");
    const ImGuiID kWheelHooksOwner = ImHashStr("ImGuiKit_WheelSession");

    ImGuiTouchConfig gTouchConfig;
    bool gAnimating = false;      // asked during the current frame
    bool gWasAnimating = false;   // asked during the last complete frame: what IsAnimating() tells
    bool gWheelSessionEnabled = true;

    bool HasHooks(ImGuiContext* ctx, ImGuiID owner)
    {
        for (const ImGuiContextHook& hook : ctx->Hooks)
            if (hook.Owner == owner && hook.Type != ImGuiContextHookType_PendingRemoval_)
                return true;
        return false;
    }

    void AddHook(ImGuiContext* ctx, ImGuiID owner, ImGuiContextHookType type, ImGuiContextHookCallback callback)
    {
        ImGuiContextHook hook;
        hook.Type = type;
        hook.Owner = owner;
        hook.Callback = callback;
        ImGui::AddContextHook(ctx, &hook);
    }
}

namespace ImGuiTouch
{
void Setup(const ImGuiTouchConfig& config)
{
    gTouchConfig = config;
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "ImGuiTouch::Setup() needs a context: call ImGui::CreateContext() first");
    if (HasHooks(ctx, kTouchHooksOwner))
        return;
    AddHook(ctx, kTouchHooksOwner, ImGuiContextHookType_NewFramePost, [](ImGuiContext*, ImGuiContextHook*) {
        gWasAnimating = gAnimating;
        gAnimating = false;
        UpdateTouchScroll(gTouchConfig.ScrollMode, gTouchConfig.LongPressIsRightClick, gTouchConfig.ThinScrollbars);
        UpdateTouchPinch(gTouchConfig.PinchMode, gTouchConfig.PinchInterruptsWidgets);
    });
    AddHook(ctx, kTouchHooksOwner, ImGuiContextHookType_EndFramePre,
            [](ImGuiContext*, ImGuiContextHook*) { DrawTouchScrollbarGrab(); });
    AddHook(ctx, kTouchHooksOwner, ImGuiContextHookType_RenderPost,
            [](ImGuiContext*, ImGuiContextHook*) { ApplyTouchOverscroll(); });
}

ImGuiTouchConfig& GetConfig() { return gTouchConfig; }

bool IsAnimating() { return gAnimating || gWasAnimating; }

namespace Internal
{
void RequestAnimation() { gAnimating = true; }

bool ConsumeAnimationRequest()
{
    bool asked = gAnimating;
    gAnimating = false;
    return asked;
}
}
} // namespace ImGuiTouch

namespace ImGuiKit
{
void SetupWheelSession(bool enabled)
{
    gWheelSessionEnabled = enabled;
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "ImGuiKit::SetupWheelSession() needs a context: call ImGui::CreateContext() first");
    if (HasHooks(ctx, kWheelHooksOwner))
        return;
    AddHook(ctx, kWheelHooksOwner, ImGuiContextHookType_NewFramePre,
            [](ImGuiContext*, ImGuiContextHook*) { WheelSessionBeforeNewFrame(); });
    AddHook(ctx, kWheelHooksOwner, ImGuiContextHookType_NewFramePost,
            [](ImGuiContext*, ImGuiContextHook*) { UpdateWheelSession(gWheelSessionEnabled); });
}
} // namespace ImGuiKit
