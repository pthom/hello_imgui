// RequestRefresh() and SetItemIsLive(): what they ask of the idling, through Dear ImGui's null backend. The runner reads
// the request once per frame (ConsumeRefreshRequest(), internal/refresh_request.h), and does not idle when there is one.
#include "doctest.h"
#include "imgui.h"
#include "imgui_impl_null.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/refresh_request.h"

#include <thread>

namespace
{
struct Bench
{
    Bench()
    {
        ImGui::CreateContext();
        ImGui_ImplNull_Init();
        HelloImGui::ConsumeRefreshRequest();  // none left by another test
    }
    ~Bench()
    {
        ImGui_ImplNull_Shutdown();
        ImGui::DestroyContext();
    }

    bool live = true;
    bool liveItemOutOfView = false;  // the live item below many lines, past the window's bottom

    void Frame()
    {
        ImGui_ImplNull_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(400.f, 300.f), ImGuiCond_Always);
        ImGui::Begin("Bench");
        for (int i = 0; i < (liveItemOutOfView ? 100 : 1); ++i)
            ImGui::Text("Line %d", i);
        ImGui::Button("A live plot");
        HelloImGui::SetItemIsLive(live);
        ImGui::End();
        ImGui::Render();
        ImGui_ImplNullRender_RenderDrawData(ImGui::GetDrawData());
    }
    void Frames(int n) { for (int i = 0; i < n; ++i) Frame(); }
};
}  // namespace

TEST_CASE("SetItemIsLive: a live item in view asks for one refresh at each frame")
{
    Bench b;
    b.Frames(2);
    CHECK(HelloImGui::ConsumeRefreshRequest());
    CHECK(!HelloImGui::ConsumeRefreshRequest());  // consumed: the runner reads it once per frame
    b.Frame();
    CHECK(HelloImGui::ConsumeRefreshRequest());
}

TEST_CASE("SetItemIsLive: an item that is not live, or out of view, asks for nothing")
{
    Bench b;
    b.live = false;
    b.Frames(2);
    CHECK(!HelloImGui::ConsumeRefreshRequest());
    b.live = true;
    b.liveItemOutOfView = true;
    b.Frames(2);
    CHECK(!HelloImGui::ConsumeRefreshRequest());
}

TEST_CASE("RequestRefresh: from another thread (data that arrives: a camera, a socket)")
{
    HelloImGui::ConsumeRefreshRequest();
    std::thread t([] { HelloImGui::RequestRefresh(); });
    t.join();
    CHECK(HelloImGui::ConsumeRefreshRequest());
    CHECK(!HelloImGui::ConsumeRefreshRequest());
}
