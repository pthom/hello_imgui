// imgui_kit: the wheel session (ImGuiKit::SetupWheelSession)
// https://github.com/pthom/imgui_kit (MIT license)
// How this could fit in Dear ImGui: docs/dear_imgui_integration.md
#include "imgui_kit.h"
#include "imgui_kit_internal.h"

#include "imgui.h"
#include "imgui_internal.h"

// ImGui locks the wheel to the window it scrolls (g.WheelingWindow, kept 0.7 s after each event, so that a child
// window scrolling through does not take it). An item that takes the wheel (ImPlot's plot, ImmVision's image) owns
// ImGuiKey_MouseWheelY while hovered: ImGui's scroll then stops, and the item reads io.MouseWheel itself. A page
// scrolled with the wheel thus stops, and zooms the plot the mouse arrived on; worse, on the frame of the arrival,
// the window still owns the wheel (the lock renews it each frame) and the item zooms as well: both happen.
// This is the same lock for items: a session that started on plain content keeps the page scrolling, and the items
// see no wheel. It runs after ImGui::NewFrame() (ImGui's own scroll ran there) and before the widgets, which read
// io.MouseWheel after it.
// While ImGui keeps scrolling a window (g.WheelingWindow), NewFrame() gives the wheel's ownership back to that window,
// which erases an item's claim: with the mouse still, the page scrolls a plot under it, and the session would see no
// item. The owner the widgets left is thus read before NewFrame() (WheelSessionBeforeNewFrame).
namespace ImGuiKit
{
constexpr float SESSION_SECONDS = 0.7f;  // ImGui's WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER

// The item (not a window) that owns the wheel, as of the previous frame; 0 when none
static ImGuiID WheelOwnerItem(const ImGuiContext& g, const ImGuiKitWheelSessionState& s)
{
    // The owner noted before NewFrame(), when the hook noted it (NewFrame() may have given the wheel back to the
    // window ImGui keeps scrolling)
    ImGuiID owner = (s.OwnerFrame == g.FrameCount - 1)
        ? s.OwnerBeforeNewFrame
        : ImGui::GetKeyOwnerData(GImGui, ImGuiKey_MouseWheelY)->OwnerCurr;
    if (owner == ImGuiKeyOwner_NoOwner || owner == ImGuiKeyOwner_Any)
        return 0;
    if (g.HoveredWindow != nullptr && owner == g.HoveredWindow->ID)
        return 0;
    if (g.WheelingWindow != nullptr && owner == g.WheelingWindow->ID)
        return 0;
    return owner;
}

// ImGui's scroll of a wheel notch (UpdateMouseWheel)
static void ScrollByWheel(ImGuiWindow* window, float wheel)
{
    float max_step = window->InnerRect.GetHeight() * 0.67f;
    float step = ImTrunc(ImMin(5 * window->FontRefSize, max_step));
    ImGui::SetScrollY(window, window->Scroll.y - wheel * step);
}

// The two steps, as context hooks
static void NewFramePreHook(ImGuiContext*, ImGuiContextHook*) { WheelSessionBeforeNewFrame(); }
static void NewFramePostHook(ImGuiContext*, ImGuiContextHook*)
{
    UpdateWheelSession(GetCurrentContext().WheelSessionEnabled);
}

void WheelSessionBeforeNewFrame()
{
    ImGuiContext* g = GImGui;
    if (g == nullptr)
        return;
    ImGuiKitWheelSessionState& s = GetCurrentContext().WheelSession;
    s.OwnerBeforeNewFrame = ImGui::GetKeyOwnerData(g, ImGuiKey_MouseWheelY)->OwnerNext;
    s.OwnerFrame = g->FrameCount;
}

void UpdateWheelSession(bool enabled)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    ImGuiKitWheelSessionState& s = GetCurrentContext().WheelSession;
    if (s.Context != &g || g.FrameCount < s.FrameCount)
    {
        s = ImGuiKitWheelSessionState{};
        s.Context = &g;
    }
    s.FrameCount = g.FrameCount;
    if (!enabled)
    {
        s.Active = false;
        return;
    }

    if (s.Active)
    {
        s.Timer -= io.DeltaTime;
        // A mouse that moves ends the session, as it ends ImGui's lock of the scrolled window: the wheel goes to what
        // is under the mouse from then on
        const float dx = io.MousePos.x - s.RefMousePos.x, dy = io.MousePos.y - s.RefMousePos.y;
        const bool moved = ImGui::IsMousePosValid()
                           && dx * dx + dy * dy > io.MouseDragThreshold * io.MouseDragThreshold;
        if (s.Timer <= 0.0f || moved)
            s.Active = false;
    }
    if (io.MouseWheel == 0.0f)
        return;
    if (!s.Active)
    {
        s.Active = true;
        s.ItemId = WheelOwnerItem(g, s);
        s.StartedOnItem = (s.ItemId != 0);
        s.Scrolled = false;
        s.Window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;  // ImGui locked the one it scrolled
    }
    s.Timer = ImMin(s.Timer + ImAbs(io.MouseWheel) * SESSION_SECONDS, SESSION_SECONDS);
    s.RefMousePos = io.MousePos;
    if (s.Window == nullptr)
        return;
    const bool scrolled_by_im_gui = (g.WheelingWindowScrolledFrame == g.FrameCount);
    if (s.StartedOnItem)
    {
        // The item's session: the wheel is its own. ImGui scrolls when the item's ownership lapses for a frame (the
        // owner is set for the next frame, from a hovered item): that scroll is cancelled while the mouse is on the
        // item. Once the mouse left it, the page scrolls as usual
        if (scrolled_by_im_gui && g.HoveredIdPreviousFrame == s.ItemId)
            ImGui::SetScrollY(s.Window, s.Window->Scroll.y);
        return;
    }
    // The page's session: ImGui scrolled, or an item took the wheel over (ImGui then did not scroll, and the page
    // scrolls here). In both cases the widgets see no wheel. When neither happened (a window that cannot scroll, a
    // widget that reads the wheel without owning it: a node editor's zoom), the wheel is left as it is
    const ImGuiID owner_item = WheelOwnerItem(g, s);
    if (scrolled_by_im_gui)
        s.Scrolled = true;
    if (!scrolled_by_im_gui && owner_item == 0)
        return;
    if (owner_item != 0 && !s.Scrolled)
    {
        // An item claimed the wheel while nothing has scrolled so far: the session was its own from the start, with
        // its ownership one frame late (set from a hovered item, for the next frame)
        s.StartedOnItem = true;
        s.ItemId = owner_item;
        return;
    }
    if (!scrolled_by_im_gui)
    {
        ScrollByWheel(s.Window, io.MouseWheel);
        s.Scrolled = true;
    }
    io.MouseWheel = 0.0f;
}

// Installs the two steps once, in the current context (their owner tells); a later call changes `enabled`
void SetupWheelSession(bool enabled)
{
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "ImGuiKit::SetupWheelSession() needs a context: call ImGui::CreateContext() first");
    GetCurrentContext().WheelSessionEnabled = enabled;
    const ImGuiID owner = ImHashStr("ImGuiKit_WheelSession");
    for (const ImGuiContextHook& hook : ctx->Hooks)
        if (hook.Owner == owner && hook.Type != ImGuiContextHookType_PendingRemoval_)
            return;
    ImGuiContextHook hook;
    hook.Owner = owner;
    hook.Type = ImGuiContextHookType_NewFramePre;
    hook.Callback = NewFramePreHook;
    ImGui::AddContextHook(ctx, &hook);
    hook.Type = ImGuiContextHookType_NewFramePost;
    hook.Callback = NewFramePostHook;
    ImGui::AddContextHook(ctx, &hook);
}

}  // namespace ImGuiKit
