#include "hello_imgui/hello_imgui_widgets.h"

#include "imgui.h"

namespace HelloImGui
{
    void BeginGroupColumn()
    {
        ImGui::BeginGroup();
    }

    void EndGroupColumn()
    {
        ImGui::EndGroup();
        ImGui::SameLine();
    }
}