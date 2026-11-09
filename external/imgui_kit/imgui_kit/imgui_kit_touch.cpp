// imgui_kit: the setup of the touch layer (ImGuiTouch), through Dear ImGui's context hooks
// https://github.com/pthom/imgui_kit (MIT license)
#include "imgui_kit.h"
#include "imgui_kit_internal.h"
#include "imgui_internal.h"  // AddContextHook

// Each step runs at a moment Dear ImGui names:
//   NewFramePre:  SimulateTouch retags the mouse's events; during a widget's pinch, the mouse goes to the fingers'
//                 center
//   NewFramePost: before any widget, the swipe, then the pinch
//   EndFramePre:  after the GUI, the widened scrollbar thumb (the frame's scroll is final)
//   RenderPost:   the draw data exists: the overscroll moves the content past its end
// A hook belongs to its context: Setup() installs the hooks of the current context, once (their owner tells).

static const ImGuiID TOUCH_HOOKS_OWNER = ImHashStr("ImGuiTouch");

// SimulateTouch: the frame's mouse events (queued by the backend, which may tag each of them as a mouse's) become a
// touch screen's, and so does the source of what follows. When it is turned off, the source goes back to the mouse.
static void ApplySimulatedTouch(ImGuiContext& g, ImGuiTouchContext& tc)
{
    if (tc.Config.SimulateTouch)
    {
        for (ImGuiInputEvent& e : g.InputEventsQueue)
        {
            if (e.AddedByTestEngine)  // the touch layer's own events (replayed, with the source they need)
                continue;
            if (e.Type == ImGuiInputEventType_MousePos)
                e.MousePos.MouseSource = ImGuiMouseSource_TouchScreen;
            else if (e.Type == ImGuiInputEventType_MouseButton)
                e.MouseButton.MouseSource = ImGuiMouseSource_TouchScreen;
            else if (e.Type == ImGuiInputEventType_MouseWheel)
                e.MouseWheel.MouseSource = ImGuiMouseSource_TouchScreen;
        }
        g.IO.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
    }
    else if (tc.SimulatedTouch)
        g.IO.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
    tc.SimulatedTouch = tc.Config.SimulateTouch;
}

static bool HasHooks(ImGuiContext* ctx, ImGuiID owner)
{
    for (const ImGuiContextHook& hook : ctx->Hooks)
        if (hook.Owner == owner && hook.Type != ImGuiContextHookType_PendingRemoval_)
            return true;
    return false;
}

static void AddHook(ImGuiContext* ctx, ImGuiID owner, ImGuiContextHookType type, ImGuiContextHookCallback callback)
{
    ImGuiContextHook hook;
    hook.Type = type;
    hook.Owner = owner;
    hook.Callback = callback;
    ImGui::AddContextHook(ctx, &hook);
}

// The steps, at the moments Dear ImGui names (see the top of this file)
static void NewFramePreHook(ImGuiContext* g, ImGuiContextHook*)
{
    ApplySimulatedTouch(*g, ImGuiTouch::GetTouchContext());
    ImGuiTouch::PinchBeforeNewFrame();
}
static void NewFramePostHook(ImGuiContext*, ImGuiContextHook*)
{
    ImGuiTouchContext& tc = ImGuiTouch::GetTouchContext();
    tc.WasAnimating = tc.Animating;
    tc.Animating = false;
    ImGuiTouch::ApplyFingerCancel();
    ImGuiTouch::UpdateTouchScroll(tc.Config.ScrollMode, tc.Config.LongPressIsRightClick, tc.Config.ThinScrollbars);
    ImGuiTouch::UpdateTouchPinch(tc.Config.PinchScalesFont);
}
static void EndFramePreHook(ImGuiContext*, ImGuiContextHook*) { ImGuiTouch::DrawTouchScrollbarGrab(); }
static void RenderPostHook(ImGuiContext*, ImGuiContextHook*) { ImGuiTouch::ApplyTouchOverscroll(); }
#ifdef IMGUI_HAS_ITEM_DRAG_AXIS_CALLBACK
static void ItemDragAxisCallback(ImGuiContext*, ImGuiID, const ImRect& bb, ImGuiAxis axis)
{
    ImGuiTouch::AddDragTaker(bb, true, axis);
}
#endif

namespace ImGuiTouch
{
// The touch state is a part of the kit's (ImGuiKitContext), in use once a setup was called: the only place that knows
ImGuiTouchContext* FindTouchContext()
{
    if (GImGui == nullptr)
        return nullptr;
    ImGuiKitContext& kit = ImGuiKit::GetCurrentContext();
    return kit.TouchSetUp ? &kit.Touch : nullptr;
}

ImGuiTouchContext& GetTouchContext()
{
    ImGuiTouchContext* state = FindTouchContext();
    IM_ASSERT(state != nullptr
              && "imgui_kit's touch layer: call ImGuiTouch::Setup() first (or another ImGuiTouch setup)");
    return *state;
}

ImGuiTouchContext& CreateCurrentContextIfNeeded()
{
    ImGuiKitContext& kit = ImGuiKit::GetCurrentContext();
    kit.TouchSetUp = true;
    return kit.Touch;
}
} // namespace ImGuiTouch

namespace ImGuiTouch
{
void Setup(const ImGuiTouchConfig& config)
{
    CreateCurrentContextIfNeeded().Config = config;
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "ImGuiTouch::Setup() needs a context: call ImGui::CreateContext() first");
#ifdef IMGUI_HAS_ITEM_DRAG_AXIS_CALLBACK
    // A Dear ImGui that tells its draggable widgets (sliders, drags, the color picker): they take a finger's drag along
    // their axis, as with SetItemTakesDrags(ImGuiTouchAxis_X or _Y)
    ctx->ItemDragAxisCallback = ItemDragAxisCallback;
#endif
    if (HasHooks(ctx, TOUCH_HOOKS_OWNER))
        return;
    AddHook(ctx, TOUCH_HOOKS_OWNER, ImGuiContextHookType_NewFramePre, NewFramePreHook);
    AddHook(ctx, TOUCH_HOOKS_OWNER, ImGuiContextHookType_NewFramePost, NewFramePostHook);
    AddHook(ctx, TOUCH_HOOKS_OWNER, ImGuiContextHookType_EndFramePre, EndFramePreHook);
    AddHook(ctx, TOUCH_HOOKS_OWNER, ImGuiContextHookType_RenderPost, RenderPostHook);
}

bool IsInstalled() { return FindTouchContext() != nullptr; }
ImGuiTouchConfig& GetConfig() { return GetTouchContext().Config; }

// Without the touch layer, nothing records the animations: the kit's widgets animate all the same (an app that draws
// every frame), and IsAnimating() says false
bool IsAnimating()
{
    ImGuiTouchContext* state = FindTouchContext();
    return state != nullptr && (state->Animating || state->WasAnimating);
}

void RequestAnimation()
{
    if (ImGuiTouchContext* state = FindTouchContext())
        state->Animating = true;
}

bool ConsumeAnimationRequest()
{
    ImGuiTouchContext& tc = GetTouchContext();
    bool asked = tc.Animating;
    tc.Animating = false;
    return asked;
}
} // namespace ImGuiTouch
