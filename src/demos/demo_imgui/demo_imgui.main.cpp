#include "imgui_runner.h"

struct AppState
{
    bool show_test_window = true;
    bool show_another_window = false;
};

void GuiLoop(AppState &appState)
{
    // 1. Show a simple window
    // Tip: ImGuiRunner provide a default background that occupies the whole app window
    // (you can add other windows on top of it)
    {
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        if (ImGui::Button("Test Window")) appState.show_test_window ^= 1;
        if (ImGui::Button("Another Window")) appState.show_another_window ^= 1;
        ImGui::Text("Application average %.3lf ms/frame (%.1lf FPS)",
                    static_cast<double>(1000.f / ImGui::GetIO().Framerate),
                    static_cast<double>(ImGui::GetIO().Framerate));
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (appState.show_another_window)
    {
        ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Another Window", &appState.show_another_window);
        ImGui::Text("Hello");
        ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (appState.show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();
    }
}

int main(int, char **)
{
    AppState appState;
    auto guiLoopWithCapture = [&]() {
        GuiLoop(appState);
    };
    ImGuiRunner::Run(guiLoopWithCapture);
}
