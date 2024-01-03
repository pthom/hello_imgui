#pragma once
// Some additional widgets and utilities for ImGui


namespace HelloImGui
{
    void BeginGroupColumn(); // calls ImGui::BeginGroup()
    void EndGroupColumn();   // calls ImGui::EndGroup() + ImGui::SameLine()
}

