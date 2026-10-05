// A test bench for the swipe (RunnerParams::touchScrollMode): a long window with widgets, a child window, a wide
// child, and, for a desktop without a touch screen, a checkbox that makes ImGui believe the mouse is a finger.
#include "hello_imgui/hello_imgui.h"

#include <cstdio>

int main(int, char*[])
{
    HelloImGui::RunnerParams params;
    params.appWindowParams.windowTitle = "Touch scroll";
    params.appWindowParams.windowGeometry.size = {500, 800};

    bool simulateTouch = false;
    int nbClicks = 0;
    float slider = 0.5f;
    params.callbacks.ShowGui = [&]()
    {
        ImGuiIO& io = ImGui::GetIO();
        // The source applies to the events queued after this call, i.e. the next frame's (a desktop backend sets no
        // source, except on Windows: there, this overrides it)
        io.AddMouseSourceEvent(simulateTouch ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);

        ImGui::Checkbox("Simulate a touch source", &simulateTouch);
        const char* sourceNames[] = {"Mouse", "TouchScreen", "Pen"};
        ImGui::Text("io.MouseSource: %s", sourceNames[io.MouseSource]);
        int mode = (int)params.touchScrollMode;
        ImGui::Combo("touchScrollMode", &mode, "Auto\0Always\0Disabled\0");
        params.touchScrollMode = (HelloImGui::TouchScrollMode)mode;
        ImGui::Separator();
        ImGui::Text("Clicks: %d   Slider: %.2f", nbClicks, slider);
        ImGui::Separator();

        for (int i = 0; i < 80; ++i)
        {
            ImGui::Text("Line %2d: drag here to scroll, tap the buttons, drag the sliders", i);
            if (i % 10 == 5)
            {
                char label[32];
                snprintf(label, sizeof(label), "Click me##%d", i);
                if (ImGui::Button(label))
                    ++nbClicks;
                ImGui::SameLine();
                snprintf(label, sizeof(label), "##slider%d", i);
                ImGui::SliderFloat(label, &slider, 0.f, 1.f);
            }
            if (i == 20)
            {
                ImGui::BeginChild("child", ImVec2(0.f, 150.f), ImGuiChildFlags_Borders);
                for (int j = 0; j < 30; ++j)
                    ImGui::Text("Child line %2d: a swipe scrolls the child, then its parent", j);
                ImGui::EndChild();
            }
            if (i == 40)
            {
                ImGui::BeginChild("wide", ImVec2(0.f, 100.f), ImGuiChildFlags_Borders,
                                  ImGuiWindowFlags_HorizontalScrollbar);
                for (int j = 0; j < 5; ++j)
                    ImGui::Text("Wide line %d: a horizontal swipe scrolls this child sideways, "
                                "a vertical one scrolls the parent window", j);
                ImGui::EndChild();
            }
        }
    };
    HelloImGui::Run(params);
    return 0;
}
