#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/internal/borderless_resizable.h"
#include "hello_imgui/hello_imgui.h"
#include "imgui_internal.h"


namespace HelloImGui
{
    void HandleBorderlessResizable(
        BackendApi::WindowPointer window,
        BackendApi::IBackendWindowHelper * backendWindowHelper)
    {
        ImVec2 winSize = ImGui::GetWindowSize();
        auto highlightColor = ImGui::GetColorU32(ImGuiCol_FrameBgHovered, 0.8f);

        // Drag zone at the top of the window
        {
            ImVec2 topLeftCorner = ImGui::GetMainViewport()->Pos;
            ImVec2 bottomRightCorner = topLeftCorner + ImVec2(winSize.x, HelloImGui::EmSize(1.5f));
            ImRect dragRect = ImRect(topLeftCorner, bottomRightCorner);

            ImVec2 mousePos = ImGui::GetIO().MousePos;
            bool isMouseInDragZone = dragRect.Contains(mousePos);

            // highlight drag zone if hovering
            if (isMouseInDragZone)
            {
                ImGui::GetForegroundDrawList()->AddRectFilled(
                    dragRect.Min,
                    dragRect.Max,
                    highlightColor
                );
            }

            static bool isDragging = false;
            if (ImGui::IsMouseDragging(0) && isMouseInDragZone)
                isDragging = true;
            if (ImGui::IsMouseReleased(0))
                isDragging = false;

            if (isDragging)
            {
                ImVec2 dragDelta = ImGui::GetMouseDragDelta(0);
                ImGui::ResetMouseDragDelta(0);

                // move window
                {
                    auto windowBounds = backendWindowHelper->GetWindowBounds(window);
                    windowBounds.position[0] += (int)dragDelta.x;
                    windowBounds.position[1] += (int)dragDelta.y;
                    backendWindowHelper->SetWindowBounds(window, windowBounds);
                }
            }
        }

        // Drag zone at the bottom-right of the window
        // This is still a draft, which needs to be completed
        // and probably refactored: there is too much repetition between the two blocks of code
        {
            ImVec2 bottomRightCorner = ImGui::GetMainViewport()->Pos + ImGui::GetMainViewport()->Size;
            ImVec2 topLeftCorner = bottomRightCorner - HelloImGui::EmToVec2(1.5f, 1.5f);

            // Small triangle in the bottom-right corner
            ImVec2 t1 = bottomRightCorner;
            ImVec2 t2 = bottomRightCorner - HelloImGui::EmToVec2(1.5f, 0.f);
            ImVec2 t3 = bottomRightCorner - HelloImGui::EmToVec2(0.f, 1.5f);

            ImRect dragRect = ImRect(topLeftCorner, bottomRightCorner);

            ImVec2 mousePos = ImGui::GetIO().MousePos;
            bool isMouseInDragZone = dragRect.Contains(mousePos);

            // highlight drag zone if hovering
            if (isMouseInDragZone)
            {
                ImGui::GetForegroundDrawList()->AddTriangleFilled(
                    t1, t2, t3,
                    highlightColor
                );
            }

            // To be completed...

        }
    }
}


