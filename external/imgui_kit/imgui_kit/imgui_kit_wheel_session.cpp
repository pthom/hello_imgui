// imgui_kit: the wheel session (ImGuiKit::SetupWheelSession)
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
namespace
{
    constexpr float kSessionSeconds = 0.7f;  // ImGui's WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER

    using State = ImGuiKitWheelSessionState;

    // The item (not a window) that owns the wheel, as of the previous frame; 0 when none
    ImGuiID WheelOwnerItem(const ImGuiContext& g, const State& s)
    {
        // The owner noted before NewFrame(), when the hook noted it (NewFrame() may have given the wheel back to the
        // window ImGui keeps scrolling)
        ImGuiID owner = (s.ownerFrame == g.FrameCount - 1)
            ? s.ownerBeforeNewFrame
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
    void ScrollByWheel(ImGuiWindow* window, float wheel)
    {
        float maxStep = window->InnerRect.GetHeight() * 0.67f;
        float step = ImTrunc(ImMin(5 * window->FontRefSize, maxStep));
        ImGui::SetScrollY(window, window->Scroll.y - wheel * step);
    }
}  // namespace

void WheelSessionBeforeNewFrame()
{
    ImGuiContext* g = GImGui;
    if (g == nullptr)
        return;
    ImGuiTouch::GetCurrentContext().WheelSession.ownerBeforeNewFrame = ImGui::GetKeyOwnerData(g, ImGuiKey_MouseWheelY)->OwnerNext;
    ImGuiTouch::GetCurrentContext().WheelSession.ownerFrame = g->FrameCount;
}

void UpdateWheelSession(bool enabled)
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    State& s = ImGuiTouch::GetCurrentContext().WheelSession;
    if (s.context != &g || g.FrameCount < s.frameCount)
    {
        s = State{};
        s.context = &g;
    }
    s.frameCount = g.FrameCount;
    if (!enabled)
    {
        s.active = false;
        return;
    }

    if (s.active)
    {
        s.timer -= io.DeltaTime;
        // A mouse that moves ends the session, as it ends ImGui's lock of the scrolled window: the wheel goes to what
        // is under the mouse from then on
        const float dx = io.MousePos.x - s.refMousePos.x, dy = io.MousePos.y - s.refMousePos.y;
        const bool moved = ImGui::IsMousePosValid() && dx * dx + dy * dy > io.MouseDragThreshold * io.MouseDragThreshold;
        if (s.timer <= 0.f || moved)
            s.active = false;
    }
    if (io.MouseWheel == 0.f)
        return;
    if (!s.active)
    {
        s.active = true;
        s.itemId = WheelOwnerItem(g, s);
        s.startedOnItem = (s.itemId != 0);
        s.scrolled = false;
        s.window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;  // ImGui locked the one it scrolled
    }
    s.timer = ImMin(s.timer + ImAbs(io.MouseWheel) * kSessionSeconds, kSessionSeconds);
    s.refMousePos = io.MousePos;
    if (s.window == nullptr)
        return;
    const bool scrolledByImGui = (g.WheelingWindowScrolledFrame == g.FrameCount);
    if (s.startedOnItem)
    {
        // The item's session: the wheel is its own. ImGui scrolls when the item's ownership lapses for a frame (the
        // owner is set for the next frame, from a hovered item): that scroll is cancelled while the mouse is on the
        // item. Once the mouse left it, the page scrolls as usual
        if (scrolledByImGui && g.HoveredIdPreviousFrame == s.itemId)
            ImGui::SetScrollY(s.window, s.window->Scroll.y);
        return;
    }
    // The page's session: ImGui scrolled, or an item took the wheel over (ImGui then did not scroll, and the page
    // scrolls here). In both cases the widgets see no wheel. When neither happened (a window that cannot scroll, a
    // widget that reads the wheel without owning it: a node editor's zoom), the wheel is left as it is
    const ImGuiID ownerItem = WheelOwnerItem(g, s);
    if (scrolledByImGui)
        s.scrolled = true;
    if (!scrolledByImGui && ownerItem == 0)
        return;
    if (ownerItem != 0 && !s.scrolled)
    {
        // An item claimed the wheel while nothing has scrolled so far: the session was its own from the start, with
        // its ownership one frame late (set from a hovered item, for the next frame)
        s.startedOnItem = true;
        s.itemId = ownerItem;
        return;
    }
    if (!scrolledByImGui)
    {
        ScrollByWheel(s.window, io.MouseWheel);
        s.scrolled = true;
    }
    io.MouseWheel = 0.f;
}
}  // namespace ImGuiKit
