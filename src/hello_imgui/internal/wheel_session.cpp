#include "hello_imgui/internal/wheel_session.h"

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
namespace HelloImGui
{
namespace
{
    constexpr float kSessionSeconds = 0.7f;  // ImGui's WINDOWS_MOUSE_WHEEL_SCROLL_LOCK_TIMER

    struct State
    {
        ImGuiContext* context = nullptr;  // the state belongs to one context: a new one starts afresh
        int frameCount = -1;              // (a new context may reuse the address of a destroyed one: its frames restart)
        bool active = false;
        bool startedOnItem = false;     // the item under the mouse owned the wheel at the first event: the item's session
        ImGuiWindow* window = nullptr;  // the window the session scrolls
        double lastEventTime = -1e9;
    };
    State gState;

    // Whether an item (not a window) owns the wheel, as of the previous frame
    bool WheelOwnedByItem(const ImGuiContext& g)
    {
        ImGuiID owner = ImGui::GetKeyOwnerData(GImGui, ImGuiKey_MouseWheelY)->OwnerCurr;
        if (owner == ImGuiKeyOwner_NoOwner || owner == ImGuiKeyOwner_Any)
            return false;
        if (g.HoveredWindow != nullptr && owner == g.HoveredWindow->ID)
            return false;
        if (g.WheelingWindow != nullptr && owner == g.WheelingWindow->ID)
            return false;
        return true;
    }

    // ImGui's scroll of a wheel notch (UpdateMouseWheel)
    void ScrollByWheel(ImGuiWindow* window, float wheel)
    {
        float maxStep = window->InnerRect.GetHeight() * 0.67f;
        float step = ImTrunc(ImMin(5 * window->FontRefSize, maxStep));
        ImGui::SetScrollY(window, window->Scroll.y - wheel * step);
    }
}  // namespace

void UpdateWheelSession()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    State& s = gState;
    if (s.context != &g || g.FrameCount < s.frameCount)
    {
        s = State{};
        s.context = &g;
    }
    s.frameCount = g.FrameCount;

    if (io.MouseWheel == 0.f)
    {
        if (g.Time - s.lastEventTime > kSessionSeconds)
            s.active = false;
        return;
    }
    s.lastEventTime = g.Time;
    if (!s.active)
    {
        s.active = true;
        s.startedOnItem = WheelOwnedByItem(g);
        s.window = g.WheelingWindow ? g.WheelingWindow : g.HoveredWindow;  // ImGui locked the one it scrolled
    }
    if (s.startedOnItem || s.window == nullptr)
        return;
    // The page's session: ImGui scrolled unless an item took the wheel over; then the page scrolls here. Either way
    // the widgets see no wheel
    if (g.WheelingWindowScrolledFrame != g.FrameCount)
        ScrollByWheel(s.window, io.MouseWheel);
    io.MouseWheel = 0.f;
}
}  // namespace HelloImGui
