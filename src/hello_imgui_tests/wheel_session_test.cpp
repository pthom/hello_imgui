// The wheel session (internal/wheel_session.cpp) through Dear ImGui's null backend: a window of lines with an item
// that takes the wheel to zoom, as ImPlot's plot and ImmVision's image do (it owns ImGuiKey_MouseWheelY while
// hovered, and reads io.MouseWheel).
#define IMGUI_DEFINE_MATH_OPERATORS
#include "doctest.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_null.h"
#include "hello_imgui/internal/wheel_session.h"

namespace
{
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

    float scrollY = 0.f;   // as of the last frame's Begin()
    int zooms = 0;         // the wheel notches the zooming item saw
    ImRect zoomRect, linesRect;

    void Frame()
    {
        ImGui_ImplNull_NewFrame();
        ImGui::NewFrame();
        HelloImGui::UpdateWheelSession();
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(400.f, 300.f), ImGuiCond_Always);
        ImGui::Begin("Bench", nullptr, ImGuiWindowFlags_NoTitleBar);
        scrollY = ImGui::GetScrollY();
        for (int i = 0; i < 10; ++i)
            ImGui::Text("Line %d", i);
        ImGui::InvisibleButton("zoom", ImVec2(300.f, 100.f));  // the zooming item, as ImPlot and ImmVision do it
        zoomRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        if (ImGui::IsItemHovered())
        {
            ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
            if (ImGui::GetIO().MouseWheel != 0.f)
                zooms++;
        }
        for (int i = 10; i < 100; ++i)
            ImGui::Text("Line %d", i);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplNullRender_RenderDrawData(ImGui::GetDrawData());
    }
    void Frames(int n) { for (int i = 0; i < n; ++i) Frame(); }
    void MoveTo(ImVec2 p)
    {
        ImGui::GetIO().AddMousePosEvent(p.x, p.y);
        Frame();
    }
    void Wheel(float notches)
    {
        ImGui::GetIO().AddMouseWheelEvent(0.f, notches);
        Frame();
    }
};
}  // namespace

TEST_CASE("Wheel session: a wheel that started on the lines keeps scrolling over the zooming item, which sees none")
{
    Bench b;
    b.Frames(3);
    b.MoveTo(ImVec2(100.f, 20.f));  // on the lines
    b.Wheel(-1.f);
    float afterFirst = b.scrollY;
    CHECK(afterFirst > 0.f);
    b.MoveTo(b.zoomRect.GetCenter());  // over the item (its rectangle is on the screen, as of the last frame)
    b.Frames(2);
    CHECK(b.zoomRect.Contains(ImGui::GetIO().MousePos));  // the item is under the mouse: without the session it would zoom
    b.Wheel(-1.f);
    b.Wheel(-1.f);
    CHECK(b.scrollY > afterFirst);
    CHECK(b.zooms == 0);
}

TEST_CASE("Wheel session: a wheel that started on the zooming item zooms, and does not scroll")
{
    Bench b;
    b.Frames(3);
    b.MoveTo(b.zoomRect.GetCenter());
    b.Frames(2);  // the item owns the wheel from the frame before
    b.Wheel(-1.f);
    b.Wheel(-1.f);
    CHECK(b.zooms == 2);
    CHECK(b.scrollY == 0.f);
    b.MoveTo(ImVec2(100.f, 20.f));  // back on the lines: the page scrolls, as before
    b.Frames(2);
    b.Wheel(-1.f);
    CHECK(b.scrollY > 0.f);
}

TEST_CASE("Wheel session: a session ends after a pause, and the item takes the next wheel")
{
    Bench b;
    b.Frames(3);
    b.MoveTo(ImVec2(100.f, 20.f));
    b.Wheel(-1.f);
    b.MoveTo(b.zoomRect.GetCenter());
    b.Frames(60);  // one second: past the session's 0.7 s
    float scroll = b.scrollY;
    b.Wheel(-1.f);
    CHECK(b.zooms == 1);
    CHECK(b.scrollY == scroll);
}
