// A test bench for the swipe (RunnerParams::touchScrollMode): a long window with widgets, a child window, a wide
// child, and, for a desktop without a touch screen, a checkbox that makes ImGui believe the mouse is a finger.
#include "hello_imgui/hello_imgui.h"

#include <cstdio>

static const char* kParagraphs[] = {
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore "
    "magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
    "consequat.",
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur "
    "sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.",
    "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem "
    "aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo.",
    "Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores "
    "eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui dolorem ipsum quia dolor sit amet, "
    "consectetur, adipisci velit, sed quia non numquam eius modi tempora incidunt ut labore et dolore magnam aliquam "
    "quaerat voluptatem.",
    "Ut enim ad minima veniam, quis nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid ex ea "
    "commodi consequatur? Quis autem vel eum iure reprehenderit qui in ea voluptate velit esse quam nihil molestiae "
    "consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla pariatur?",
    "At vero eos et accusamus et iusto odio dignissimos ducimus qui blanditiis praesentium voluptatum deleniti atque "
    "corrupti quos dolores et quas molestias excepturi sint occaecati cupiditate non provident, similique sunt in "
    "culpa qui officia deserunt mollitia animi, id est laborum et dolorum fuga.",
    "Et harum quidem rerum facilis est et expedita distinctio. Nam libero tempore, cum soluta nobis est eligendi "
    "optio cumque nihil impedit quo minus id quod maxime placeat facere possimus, omnis voluptas assumenda est, "
    "omnis dolor repellendus.",
    "Temporibus autem quibusdam et aut officiis debitis aut rerum necessitatibus saepe eveniet ut et voluptates "
    "repudiandae sint et molestiae non recusandae. Itaque earum rerum hic tenetur a sapiente delectus, ut aut "
    "reiciendis voluptatibus maiores alias consequatur aut perferendis doloribus asperiores repellat.",
};
static const int kNbParagraphs = sizeof(kParagraphs) / sizeof(kParagraphs[0]);

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
        // The source applies to the events queued after this call, i.e. the next frame's. A desktop backend sets no
        // source (except on Windows); in the browser, HelloImGui reports the real one, which this overrides while
        // checked (so that a mouse can play the finger)
        if (simulateTouch)
            io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen);
        if (ImGui::Checkbox("Simulate a touch source", &simulateTouch) && !simulateTouch)
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
        const char* sourceNames[] = {"Mouse", "TouchScreen", "Pen"};
        ImGui::Text("io.MouseSource: %s", sourceNames[io.MouseSource]);
        int mode = (int)params.touchScrollMode;
        ImGui::Combo("touchScrollMode", &mode, "Auto\0Always\0Disabled\0");
        params.touchScrollMode = (HelloImGui::TouchScrollMode)mode;
        int pinchMode = (int)params.touchPinchMode;
        ImGui::Combo("touchPinchMode", &pinchMode, "FontScale\0Disabled\0");
        params.touchPinchMode = (HelloImGui::TouchPinchMode)pinchMode;
        ImGui::Checkbox("touchPinchInterruptsWidgets", &params.touchPinchInterruptsWidgets);
        ImGui::SameLine();
        ImGui::Text("FontScaleMain: %.2f", ImGui::GetStyle().FontScaleMain);
        ImGui::Separator();
        ImGui::Text("Clicks: %d   Slider: %.2f", nbClicks, slider);
        ImGui::Separator();

        // A vertical slider and a big button
        {
            ImGui::Text("The slider and the button below are usable on a touch screen, but they trigger after a small delay; to enable scroll detection.");
            static float v = 1.0;
            ImGui::VSliderFloat("##v", HelloImGui::EmToVec2(1.5f, 12.f), &v, 0.0f, 1.0f);
            ImGui::SameLine();
            ImGui::Button("Click me##v", HelloImGui::EmToVec2(12.f, 12.f));
        }

        // Paragraphs to swipe on, with widgets and child windows between them
        for (int i = 0; i < 2 * kNbParagraphs; ++i)
        {
            ImGui::TextWrapped("%s", kParagraphs[i % kNbParagraphs]);
            if (i % 3 == 1)
            {
                char label[32];
                snprintf(label, sizeof(label), "Click me##%d", i);
                if (ImGui::Button(label))
                    ++nbClicks;
                ImGui::SameLine();
                snprintf(label, sizeof(label), "##slider%d", i);
                ImGui::SliderFloat(label, &slider, 0.f, 1.f);
            }
            if (i == 2)
            {
                ImGui::BeginChild("child", HelloImGui::EmToVec2(0.f, 9.f), ImGuiChildFlags_Borders);
                for (int j = 0; j < 30; ++j)
                    ImGui::Text("Child line %2d: a swipe scrolls the child", j);
                ImGui::EndChild();
            }
            if (i == 5)
            {
                ImGui::BeginChild("wide", HelloImGui::EmToVec2(0.f, 6.f), ImGuiChildFlags_Borders,
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
