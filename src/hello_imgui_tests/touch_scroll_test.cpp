// The swipe layer (internal/touch_scroll.cpp) through Dear ImGui's null backend: the mouse and its source are
// injected, the scroll of the windows is read back. No HelloImGui::Run(): the layer is a function of the frame.
#define IMGUI_DEFINE_MATH_OPERATORS
#include "doctest.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_null.h"
#include "hello_imgui/internal/touch_scroll.h"
#include "hello_imgui/internal/touch_pinch.h"
#include "hello_imgui/hello_imgui.h"  // SetItemTakesTouchDrags

namespace
{
using HelloImGui::TouchScrollMode;

// A window at (0,0), 400x300: a button, a slider, a child of 30 lines, a wide child of one line, then 100 lines
struct Bench
{
    Bench()
    {
        ImGui::CreateContext();
        ImGui_ImplNull_Init();
    }
    ~Bench()
    {
        ImGui_ImplNull_Shutdown();
        ImGui::DestroyContext();
    }

    TouchScrollMode mode = TouchScrollMode::Always;
    bool longPressIsRightClick = true;
    bool itemPopupOpen = false, windowPopupOpen = false;  // the context menus (right clicks) of the button and the window
    bool rightDown = false;  // io.MouseDown[1], as of the last frame
    HelloImGui::TouchPinchMode pinchMode = HelloImGui::TouchPinchMode::FontScale;
    bool pinchInterrupts = false;
    bool shortContent = false;  // the window's lines fit: nothing to scroll (the child still scrolls)
    bool canvas = false;        // a canvas below the wide child: an item that sums its drags
    bool canvasTakesDrags = false;  // it calls SetItemTakesTouchDrags()
    ImGuiID canvasId = 0;
    ImVec2 canvasDrag;
    ImGuiID buttonId = 0;
    bool steal = false;  // the GUI takes the active id while the button is down (a widget that wants a long press)
    ImGuiID stealId = 0;
    int clicks = 0;
    int repeats = 0;  // the presses of a button that repeats while held
    int doubleClicks = 0;  // the frames where the widgets would see a double click (read after the layer ran)
    float slider = 0.5f;
    float scrollY = 0.f, childScrollY = 0.f, wideScrollX = 0.f;  // as of the last frame's Begin()
    float scrollMaxY = 0.f;
    float lastVtxY = 0.f;  // the y of the window's last drawn vertex (its last visible line): the overscroll moves it
    ImRect buttonRect, sliderRect, repeatRect, childRect, wideRect, canvasRect, linesRect;  // screen coordinates, as of the last frame
    ImVec2 mouse;

    void Frame()
    {
        ImGui_ImplNull_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(400.f, 300.f), ImGuiCond_Always);
        ImGui::Begin("Bench", nullptr, ImGuiWindowFlags_NoTitleBar);
        scrollY = ImGui::GetScrollY();
        scrollMaxY = ImGui::GetScrollMaxY();
        rightDown = ImGui::GetIO().MouseDown[1];
        if (steal && ImGui::IsMouseDown(0))
        {
            stealId = ImGui::GetID("steal");
            ImGui::SetActiveID(stealId, ImGui::GetCurrentWindow());
            ImGui::KeepAliveID(stealId);
        }
        if (ImGui::Button("Button"))
            clicks++;
        buttonId = ImGui::GetItemID();
        buttonRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        itemPopupOpen = false;
        if (ImGui::BeginPopupContextItem("button_menu"))
        {
            itemPopupOpen = true;
            ImGui::EndPopup();
        }
        ImGui::SliderFloat("Slider", &slider, 0.f, 1.f);
        sliderRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
        if (ImGui::Button("Repeat"))
            repeats++;
        ImGui::PopItemFlag();
        repeatRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ImGui::BeginChild("Child", ImVec2(0.f, 80.f), ImGuiChildFlags_Borders);
        childScrollY = ImGui::GetScrollY();
        for (int i = 0; i < 30; ++i)
            ImGui::Text("Child line %d", i);
        ImGui::EndChild();
        childRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ImGui::BeginChild("Wide", ImVec2(0.f, 60.f), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);  // tall enough: one line plus its scrollbar, no vertical scroll
        wideScrollX = ImGui::GetScrollX();
        ImGui::Text("A wide line, wider than the window, so that this child scrolls sideways and not vertically");
        ImGui::EndChild();
        wideRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        if (canvas)
        {
            ImGui::InvisibleButton("Canvas", ImVec2(300.f, 50.f));
            canvasId = ImGui::GetItemID();
            canvasRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            if (ImGui::IsItemActive())
                canvasDrag += ImGui::GetIO().MouseDelta;
            if (canvasTakesDrags)
                HelloImGui::SetItemTakesTouchDrags();
        }
        ImVec2 linesPos = ImGui::GetCursorScreenPos();
        for (int i = 0; i < (shortContent ? 2 : 100); ++i)
            ImGui::Text("Line %d", i);
        linesRect = ImRect(linesPos, ImVec2(linesPos.x + 300.f, 290.f));
        windowPopupOpen = false;
        if (ImGui::BeginPopupContextWindow("window_menu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            windowPopupOpen = true;
            ImGui::EndPopup();
        }
        ImGui::End();
        HelloImGui::UpdateTouchScroll(mode, longPressIsRightClick);
        HelloImGui::UpdateTouchPinch(pinchMode, pinchInterrupts);
        if (ImGui::GetIO().MouseClickedCount[ImGuiMouseButton_Left] == 2)  // the layer runs before the widgets in the runner
            doubleClicks++;
        ImGui::Render();
        HelloImGui::ApplyTouchOverscroll();
        lastVtxY = ImGui::FindWindowByName("Bench")->DrawList->VtxBuffer.back().pos.y;
        ImGui_ImplNullRender_RenderDrawData(ImGui::GetDrawData());
    }
    void Frames(int n) { for (int i = 0; i < n; ++i) Frame(); }

    void MoveTo(ImVec2 p)
    {
        mouse = p;
        ImGui::GetIO().AddMousePosEvent(p.x, p.y);
        Frame();
    }
    void Press(ImVec2 p)
    {
        MoveTo(p);
        ImGui::GetIO().AddMouseButtonEvent(0, true);
        Frame();
    }
    // The finger moves by `delta`, in steps of one frame
    void Drag(ImVec2 delta, int steps)
    {
        for (int i = 1; i <= steps; ++i)
            MoveTo(mouse + delta * ((float)i / (float)steps) - delta * ((float)(i - 1) / (float)steps));
    }
    // A release after a pause: the finger speed has decayed, no inertia follows
    void ReleaseStill()
    {
        Frames(15);
        Release();
    }
    void Release()
    {
        ImGui::GetIO().AddMouseButtonEvent(0, false);
        Frames(4);  // the replayed press and release, the scroll target, a frame to read
    }
    void Source(ImGuiMouseSource source) { ImGui::GetIO().AddMouseSourceEvent(source); }
};

bool Near(float a, float b, float tol = 3.f) { return ImFabs(a - b) <= tol; }
}  // namespace

TEST_CASE("Touch scroll: a swipe on the lines scrolls the window, a tap does not")
{
    Bench b;
    b.Frames(3);
    REQUIRE(b.scrollY == 0.f);
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.ReleaseStill();
    CHECK(Near(b.scrollY, 50.f));
    CHECK(ImGui::GetCurrentContext()->ActiveId == 0);

    float before = b.scrollY;
    b.Press(b.linesRect.GetCenter());
    b.ReleaseStill();
    CHECK(b.scrollY == before);
    CHECK(b.clicks == 0);
}

TEST_CASE("Touch scroll: Auto acts with a touch source only, Disabled never")
{
    Bench b;
    b.mode = TouchScrollMode::Auto;
    b.Frames(3);
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.ReleaseStill();
    CHECK(b.scrollY == 0.f);
    b.Press(b.buttonRect.GetCenter());
    CHECK(ImGui::GetCurrentContext()->ActiveId != 0);  // the button, at once
    b.Release();
    CHECK(b.clicks == 1);

    b.Source(ImGuiMouseSource_TouchScreen);
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.ReleaseStill();
    CHECK(Near(b.scrollY, 50.f));

    b.mode = TouchScrollMode::Disabled;
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.ReleaseStill();
    CHECK(Near(b.scrollY, 50.f));
}

TEST_CASE("Touch scroll: a tap on a button clicks it when the finger lifts, a swipe from it scrolls")
{
    Bench b;
    b.Frames(3);
    b.Press(b.buttonRect.GetCenter());
    b.Frames(2);
    CHECK(b.clicks == 0);  // nothing yet: the press is held back
    b.Release();
    CHECK(b.clicks == 1);  // replayed
    CHECK(b.scrollY == 0.f);

    b.Press(b.buttonRect.GetCenter());
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.ReleaseStill();
    CHECK(Near(b.scrollY, 50.f));
    CHECK(b.clicks == 1);
}

TEST_CASE("Touch scroll: two taps are a double click, a tap after a pause is not, one tap never is")
{
    Bench b;
    b.Frames(3);
    b.Press(b.buttonRect.GetCenter());
    b.Release();
    CHECK(b.clicks == 1);
    CHECK(b.doubleClicks == 0);  // the replayed press does not pair with the finger's own press
    b.Press(b.buttonRect.GetCenter());  // about 0.1 s after the first tap
    b.Release();
    CHECK(b.clicks == 2);
    CHECK(b.doubleClicks == 1);
    b.Frames(30);  // half a second
    b.Press(b.buttonRect.GetCenter());
    b.Release();
    CHECK(b.clicks == 3);
    CHECK(b.doubleClicks == 1);
    // A finger's two taps land apart: 12 px (ImGui's 6 px suit a mouse), and may take 0.35 s
    b.Frames(30);
    b.Source(ImGuiMouseSource_TouchScreen);
    b.Press(b.buttonRect.GetCenter());
    b.Release();
    b.Frames(14);  // about 0.35 s from the first tap's press to the second's
    b.Press(b.buttonRect.GetCenter() + ImVec2(12.f, 0.f));
    b.Release();
    CHECK(b.clicks == 5);
    CHECK(b.doubleClicks == 2);
}

TEST_CASE("Touch scroll: a hold hands the press to the widget under the finger")
{
    Bench b;
    b.Frames(3);
    b.Press(b.sliderRect.GetCenter());
    b.Frames(20);  // past the hold delay, the slider got the press
    b.Drag(ImVec2(60.f, 0.f), 5);
    b.ReleaseStill();
    CHECK(b.slider > 0.6f);
    CHECK(b.scrollY == 0.f);

    b.Press(b.linesRect.GetCenter());  // a hold on the void: nothing
    b.Frames(20);
    b.Release();
    CHECK(b.scrollY == 0.f);
    CHECK(b.clicks == 0);

    b.Press(b.buttonRect.GetCenter());  // a long tap still clicks
    b.Frames(20);
    b.Release();
    CHECK(b.clicks == 1);
}

TEST_CASE("Touch scroll: on a touch screen, a lift leaves no pointer, so nothing is hovered while the content coasts")
{
    Bench b;
    b.mode = TouchScrollMode::Auto;
    b.Frames(3);
    b.Press(b.buttonRect.GetCenter());  // a mouse: the pointer stays, the button is hovered after the click
    b.Release();
    CHECK(b.clicks == 1);
    CHECK(ImGui::IsMousePosValid());
    CHECK(ImGui::GetCurrentContext()->HoveredId != 0);

    b.Source(ImGuiMouseSource_TouchScreen);
    b.Press(b.buttonRect.GetCenter());  // a finger: the tap clicks, then no pointer, nothing hovered
    b.Release();
    CHECK(b.clicks == 2);
    CHECK(!ImGui::IsMousePosValid());
    CHECK(ImGui::GetCurrentContext()->HoveredId == 0);

    b.Press(b.linesRect.GetCenter());  // a flick: nothing hovered while the content coasts
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.Release();
    b.Frames(10);
    CHECK(b.scrollY > 50.f);
    CHECK(!ImGui::IsMousePosValid());
    CHECK(ImGui::GetCurrentContext()->HoveredId == 0);

    b.MoveTo(b.buttonRect.GetCenter());  // the next touch brings a position back
    b.Frame();
    CHECK(ImGui::IsMousePosValid());

}

TEST_CASE("Touch scroll: on a touch screen, the release of a handover is not a lift, the pointer stays")
{
    Bench b;
    b.mode = TouchScrollMode::Auto;
    b.Frames(3);
    b.Source(ImGuiMouseSource_TouchScreen);
    b.Press(b.sliderRect.GetCenter());  // a hold: the slider gets the press, with the pointer where the finger is
    b.Frames(20);
    CHECK(ImGui::IsMousePosValid());
    CHECK(ImGui::GetCurrentContext()->ActiveId != 0);
    b.Release();
    CHECK(!ImGui::IsMousePosValid());
}

TEST_CASE("Touch pinch: two fingers scale the font, and take the press from the swipe layer")
{
    Bench b;
    b.Frames(3);
    ImGui::GetStyle().FontScaleMain = 1.f;
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, -30.f), 3);
    HelloImGui::SetTouchPointers(2, 1.f);  // the second finger lands
    b.Frame();
    HelloImGui::SetTouchPointers(2, 1.5f);  // the fingers spread
    b.Frames(2);
    CHECK(ImGui::GetStyle().FontScaleMain == 1.5f);
    b.Drag(ImVec2(0.f, -30.f), 3);  // the first finger's motion scrolls no more
    CHECK(Near(b.scrollY, 30.f));
    HelloImGui::SetTouchPointers(2, 6.f);  // clamped
    b.Frame();
    CHECK(ImGui::GetStyle().FontScaleMain == 5.f);
    HelloImGui::SetTouchPointers(0, 1.f);  // the fingers lift: the scale stays, no tap, no inertia
    b.Release();
    b.Frames(10);
    CHECK(ImGui::GetStyle().FontScaleMain == 5.f);
    CHECK(Near(b.scrollY, 30.f));
    CHECK(b.clicks == 0);
    ImGui::GetStyle().FontScaleMain = 1.f;
}

TEST_CASE("Touch pinch: two fingers that move together are a right drag, the left press is released")
{
    Bench b;
    b.Frames(3);
    ImGui::GetStyle().FontScaleMain = 1.f;
    b.Press(b.linesRect.GetCenter());
    HelloImGui::SetTouchPointers(2, 1.f, ImVec2(0.f, 0.f));  // the second finger lands
    b.Frames(2);
    CHECK(!b.rightDown);
    HelloImGui::SetTouchPointers(2, 1.02f, ImVec2(0.f, -30.f));  // the two move together
    b.Frames(4);
    CHECK(b.rightDown);
    CHECK(!ImGui::GetIO().MouseDown[0]);
    CHECK(ImGui::GetStyle().FontScaleMain == 1.f);
    b.Drag(ImVec2(0.f, -30.f), 3);  // the first finger moves: no scroll (it is a right drag now)
    CHECK(b.scrollY == 0.f);
    HelloImGui::SetTouchPointers(0, 1.f, ImVec2(0.f, 0.f));
    b.Release();
    CHECK(!b.rightDown);
    CHECK(b.clicks == 0);
}

TEST_CASE("Touch pinch: a widget that holds the press keeps it, unless the pinch interrupts widgets")
{
    Bench b;
    b.Frames(3);
    ImGui::GetStyle().FontScaleMain = 1.f;
    b.Press(b.sliderRect.GetCenter());
    b.Frames(20);  // the hold: the slider got the press
    HelloImGui::SetTouchPointers(2, 1.5f);
    b.Frames(2);
    CHECK(ImGui::GetStyle().FontScaleMain == 1.f);  // the slider keeps its drag
    b.Drag(ImVec2(60.f, 0.f), 5);
    CHECK(b.slider > 0.6f);
    HelloImGui::SetTouchPointers(0, 1.f);
    b.Release();

    b.pinchInterrupts = true;
    b.slider = 0.5f;
    b.Press(b.sliderRect.GetCenter());
    b.Frames(20);
    float held = b.slider;  // the press moved the grab to the finger
    HelloImGui::SetTouchPointers(2, 1.5f);
    b.Frames(2);
    CHECK(ImGui::GetStyle().FontScaleMain == 1.5f);  // the pinch took over
    b.Drag(ImVec2(60.f, 0.f), 5);
    CHECK(b.slider == held);  // the slider lost the drag
    HelloImGui::SetTouchPointers(0, 1.f);
    b.Release();
    ImGui::GetStyle().FontScaleMain = 1.f;
}

TEST_CASE("Touch scroll: a press is not held back when nothing can scroll, nor a drag along an axis nothing scrolls")
{
    {
        Bench b;
        b.shortContent = true;
        b.Frames(3);
        b.Press(b.buttonRect.GetCenter());  // the window fits: the button gets the press at once
        CHECK(ImGui::GetCurrentContext()->ActiveId == b.buttonId);
        b.Release();
        CHECK(b.clicks == 1);
        b.Press(b.childRect.GetCenter());  // the child scrolls: a swipe there still works
        b.Drag(ImVec2(0.f, -30.f), 3);
        b.ReleaseStill();
        CHECK(Near(b.childScrollY, 30.f));
    }  // one ImGui context at a time
    Bench c;
    c.Frames(3);
    c.Press(c.sliderRect.GetCenter());  // the window scrolls vertically only: a horizontal drag goes to the slider
    c.Drag(ImVec2(60.f, 0.f), 5);       // at once, no hold
    c.ReleaseStill();
    CHECK(c.slider > 0.6f);
    CHECK(c.scrollY == 0.f);
}

TEST_CASE("Touch scroll: a widget that takes the drags gets the press at once, also along the axis the window scrolls")
{
    {
        Bench b;
        b.canvas = true;
        b.Frames(3);
        b.Press(b.canvasRect.GetCenter());  // a plain item: the press is held back, a vertical drag scrolls the window
        CHECK(ImGui::GetCurrentContext()->ActiveId != b.canvasId);
        b.Drag(ImVec2(0.f, -40.f), 4);
        b.ReleaseStill();
        CHECK(b.scrollY > 0.f);
        CHECK(b.canvasDrag.y == 0.f);
    }  // one ImGui context at a time
    Bench c;
    c.canvas = c.canvasTakesDrags = true;
    c.Frames(3);
    c.Press(c.canvasRect.GetCenter());  // SetItemTakesTouchDrags(): the canvas has the press at once, and the drag
    CHECK(ImGui::GetCurrentContext()->ActiveId == c.canvasId);
    c.Drag(ImVec2(0.f, -40.f), 4);
    c.ReleaseStill();
    CHECK(c.scrollY == 0.f);
    CHECK(Near(c.canvasDrag.y, -40.f));
    c.Press(c.linesRect.GetCenter());  // elsewhere, the window still scrolls
    c.Drag(ImVec2(0.f, -40.f), 4);
    c.ReleaseStill();
    CHECK(c.scrollY > 0.f);
}

TEST_CASE("Touch scroll: a long press is a right click, a shorter one or a moving one is not")
{
    Bench b;
    b.Frames(3);
    b.Press(b.buttonRect.GetCenter());
    b.Frames(40);  // 0.66 s still: the hold gave the button the press, the long press takes it away and right clicks
    CHECK(b.itemPopupOpen);
    CHECK(b.clicks == 0);
    CHECK(!ImGui::GetIO().MouseDown[0]);
    b.Release();
    CHECK(b.clicks == 0);  // the real lift: nothing more
    b.Press(b.linesRect.GetCenter());  // away from the menu: it closes
    b.Release();
    CHECK(!b.itemPopupOpen);

    b.Press(b.linesRect.GetCenter());  // on the void
    b.Frames(40);
    CHECK(b.windowPopupOpen);
    b.Release();
    b.Press(ImVec2(b.linesRect.Min.x + 10.f, b.linesRect.Min.y + 10.f));  // away from the menu: it closes
    b.Release();
    CHECK(!b.windowPopupOpen);

    b.Press(b.buttonRect.GetCenter());  // a shorter hold: a click when the finger lifts
    b.Frames(15);
    b.Release();
    CHECK(b.clicks == 1);
    CHECK(!b.itemPopupOpen);

    b.Press(b.linesRect.GetCenter());  // a finger that moves before the delay: a swipe, no menu
    b.Frames(10);
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.Frames(40);
    CHECK(!b.windowPopupOpen);
    b.ReleaseStill();

    b.longPressIsRightClick = false;
    b.Press(b.buttonRect.GetCenter());
    b.Frames(40);
    CHECK(!b.itemPopupOpen);
    b.Release();
    CHECK(b.clicks == 2);
}

TEST_CASE("Touch scroll: a button that repeats keeps the finger past the long press, and repeats")
{
    Bench b;
    b.Frames(3);
    b.Source(ImGuiMouseSource_TouchScreen);
    b.Press(b.repeatRect.GetCenter());
    b.Frames(60);  // one second: the hold hands the press over, the repeats start, the long press would fire at 0.5 s
    CHECK(b.repeats > 5);
    CHECK(b.windowPopupOpen == false);
    CHECK(b.itemPopupOpen == false);
    int repeatsSoFar = b.repeats;
    b.Frames(12);
    CHECK(b.repeats > repeatsSoFar);  // still repeating: the press was not taken away
    b.Release();
}

TEST_CASE("Touch scroll: a long press on a slider, with a touch source, does not move it")
{
    Bench b;
    b.mode = TouchScrollMode::Auto;
    b.Frames(3);
    b.Source(ImGuiMouseSource_TouchScreen);
    b.Press(b.sliderRect.GetCenter());
    b.Frames(15);  // the hold gave the slider the press, at the finger
    float held = b.slider;
    CHECK(held > 0.3f);
    b.Frames(30);  // the long press takes it away: no frame with the button down at no position
    CHECK(b.slider == held);
    CHECK(!ImGui::GetIO().MouseDown[0]);
    b.Release();
    CHECK(b.slider == held);
}

TEST_CASE("Touch scroll: a widget that takes the active id after the press ends the swipe")
{
    Bench b;
    b.Frames(3);
    b.Press(b.linesRect.GetCenter());
    b.steal = true;
    b.Frame();
    CHECK(ImGui::GetCurrentContext()->ActiveId == b.stealId);
    b.Drag(ImVec2(0.f, -50.f), 5);
    b.ReleaseStill();
    CHECK(b.scrollY == 0.f);
}

TEST_CASE("Touch scroll: a child scrolls itself, and its parent when it cannot scroll that way")
{
    Bench b;
    b.Frames(3);
    b.Press(b.childRect.GetCenter());
    b.Drag(ImVec2(0.f, -30.f), 3);
    b.ReleaseStill();
    CHECK(Near(b.childScrollY, 30.f));
    CHECK(b.scrollY == 0.f);

    b.Press(b.wideRect.GetCenter());
    b.Drag(ImVec2(-30.f, 0.f), 3);
    b.ReleaseStill();
    CHECK(Near(b.wideScrollX, 30.f));
    CHECK(b.scrollY == 0.f);

    b.Press(b.wideRect.GetCenter());
    b.Drag(ImVec2(0.f, -30.f), 3);
    b.ReleaseStill();
    CHECK(Near(b.scrollY, 30.f));
    CHECK(Near(b.wideScrollX, 30.f));
}

TEST_CASE("Touch scroll: a flick keeps scrolling, then stops")
{
    Bench b;
    b.Frames(3);
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, -50.f), 5);  // 10 px per frame: 600 px/s
    b.Release();
    float atRelease = b.scrollY;
    b.Frames(10);
    CHECK(b.scrollY > atRelease + 20.f);
    b.Frames(120);
    float stopped = b.scrollY;
    b.Frames(10);
    CHECK(b.scrollY == stopped);
}

TEST_CASE("Touch scroll: a drag past the start pulls the content, which springs back at the lift")
{
    Bench b;
    b.Frames(3);
    float rest = b.lastVtxY;
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, 100.f), 5);  // down, while the window is at its start: nothing scrolls, the content follows
    CHECK(b.scrollY == 0.f);
    float pulled = b.lastVtxY;
    CHECK(pulled > rest + 20.f);
    CHECK(pulled < rest + 100.f);  // less than the finger: the rubber band
    b.Frames(15);
    CHECK(b.lastVtxY == pulled);  // held there by the finger
    b.Drag(ImVec2(0.f, -40.f), 2);  // the finger comes back: the content follows it, still nothing scrolls
    CHECK(b.lastVtxY < pulled);
    CHECK(b.lastVtxY > rest);
    CHECK(b.scrollY == 0.f);
    b.Release();
    b.Frames(60);
    CHECK(b.lastVtxY == rest);
    CHECK(b.scrollY == 0.f);
}

TEST_CASE("Touch scroll: a flick that reaches the end overshoots, then springs back to the end")
{
    Bench b;
    b.Frames(3);
    b.Press(b.linesRect.GetCenter());
    b.Drag(ImVec2(0.f, -600.f), 6);  // 100 px per frame: thousands of px/s, enough to reach the end
    b.Release();
    bool overshot = false;
    for (int i = 0; i < 120 && !overshot; ++i)
    {
        b.Frame();
        if (b.scrollY == b.scrollMaxY)
        {
            float atEnd = b.lastVtxY;
            b.Frames(2);
            overshot = b.lastVtxY < atEnd;  // the content keeps going up past its end
        }
    }
    CHECK(overshot);
    b.Frames(60);
    CHECK(b.scrollY == b.scrollMaxY);
    float settled = b.lastVtxY;
    b.Frames(5);
    CHECK(b.lastVtxY == settled);
}
