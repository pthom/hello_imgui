#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/internal/borderless_resizable.h"
#include "hello_imgui/hello_imgui.h"
#include "imgui_internal.h"

struct dragArea
{
    ImRect dragArea;
    ImDrawCornerFlags corner_type;
    bool isDragging = false;
};

void BorderlessResizableHandler(struct dragArea *dragAreaData, const int numOfDrag, HelloImGui::BackendApi::WindowPointer window, HelloImGui::BackendApi::IBackendWindowHelper * backendWindowHelper, ImU32 highlightColor)
{
    ImVec2 mousePos = ImGui::GetIO().MousePos;
    // prevent multiple dragging event concurrent
    static bool Dragging = false;

    for (int i = 0; i < numOfDrag; i++)
    {
        const ImRect dragArea = dragAreaData[i].dragArea;
        bool *isDragging = &(dragAreaData[i].isDragging);
        const ImGuiMouseCursor corner_type = dragAreaData[i].corner_type;

        bool isMouseInDragZone = dragArea.Contains(mousePos);

        // highlight drag zone if hovering or dragging
        if (isMouseInDragZone || *isDragging)
        {
            ImGui::GetForegroundDrawList()->AddRectFilled(
                dragArea.Min,
                dragArea.Max,
                highlightColor
            );
        }

        if (ImGui::IsMouseDragging(0) && isMouseInDragZone && Dragging == false)
        {
            *isDragging = true;
            Dragging = true;
        }
        if (ImGui::IsMouseReleased(0))
        {
            *isDragging = false;
            Dragging = false;
        }

        if (*isDragging)
        {
            ImVec2 MouseDelta = ImGui::GetIO().MouseDelta;
            auto windowBounds = backendWindowHelper->GetWindowBounds(window);
            switch (corner_type)
            {
            case ImDrawCornerFlags_Top:
                // move window
                {
                    windowBounds.position[0] += (int)MouseDelta.x;
                    windowBounds.position[1] += (int)MouseDelta.y;
                }
                break;
            case ImDrawCornerFlags_TopRight:
                // resize window
                {
                    windowBounds.position[1] += (int)MouseDelta.y;
                    windowBounds.size[0] += (int)MouseDelta.x;
                    windowBounds.size[1] -= (int)MouseDelta.y;
                }
                break;
            case ImDrawCornerFlags_TopLeft:
                // resize window
                {
                    windowBounds.position[0] += (int)MouseDelta.x;
                    windowBounds.position[1] += (int)MouseDelta.y;
                    windowBounds.size[0] -= (int)MouseDelta.x;
                    windowBounds.size[1] -= (int)MouseDelta.y;
                }
                break;
            case ImDrawCornerFlags_BotRight:
                // resize window
                {
                    windowBounds.size[0] += (int)MouseDelta.x;
                    windowBounds.size[1] += (int)MouseDelta.y;
                }
                break;
            case ImDrawCornerFlags_BotLeft:
                // resize window
                {
                    windowBounds.size[0] -= (int)MouseDelta.x;
                    windowBounds.size[1] += (int)MouseDelta.y;
                    windowBounds.position[0] += (int)MouseDelta.x;
                }
                break;
            default:
                break;
            }
            backendWindowHelper->SetWindowBounds(window, windowBounds);
        }
    }
}

namespace HelloImGui
{
    void HandleBorderlessResizable(
        BackendApi::WindowPointer window,
        BackendApi::IBackendWindowHelper * backendWindowHelper)
    {
        ImVec2 winPos = ImGui::GetMainViewport()->Pos;
        ImVec2 winSize = ImGui::GetMainViewport()->Size;
        auto highlightColor = ImGui::GetColorU32(ImGuiCol_FrameBgHovered, 0.8f);
        ImVec2 dragPadding = HelloImGui::EmToVec2(1.5f, 1.5f);

        static struct dragArea dragAreaData[4];
        unsigned int dragAreaIndex = 0;

        // Top pan area
        {
            ImVec2 bottomRightCorner = winPos + ImVec2(winSize.x - dragPadding.x, dragPadding.y);
            ImVec2 topLeftCorner = winPos + ImVec2(dragPadding.x, 0.f);
            ImRect dragRect = ImRect(topLeftCorner, bottomRightCorner);
            dragAreaData[dragAreaIndex].dragArea = dragRect;
            dragAreaData[dragAreaIndex].corner_type = ImDrawCornerFlags_Top;
        }
        dragAreaIndex++;

        // top-left resize area
        {
            ImVec2 bottomRightCorner = winPos + dragPadding;
            ImVec2 topLeftCorner = winPos;

            ImRect dragRect = ImRect(topLeftCorner, bottomRightCorner);
            dragAreaData[dragAreaIndex].dragArea = dragRect;
            dragAreaData[dragAreaIndex].corner_type = ImDrawCornerFlags_TopLeft;
        }
        dragAreaIndex++;

        // top-right resize area
        {
            ImVec2 bottomRightCorner = winPos + ImVec2(winSize.x, dragPadding.y);
            ImVec2 topLeftCorner = bottomRightCorner - dragPadding;

            ImRect dragRect = ImRect(topLeftCorner, bottomRightCorner);
            dragAreaData[dragAreaIndex].dragArea = dragRect;
            dragAreaData[dragAreaIndex].corner_type = ImDrawCornerFlags_TopRight;
        }
        dragAreaIndex++;

        // bottom-right resize area
        {
            ImVec2 bottomRightCorner = winPos + winSize;
            ImVec2 topLeftCorner = bottomRightCorner - dragPadding;

            // Small triangle in the bottom-right corner
            ImVec2 t1 = bottomRightCorner;
            ImVec2 t2 = bottomRightCorner - HelloImGui::EmToVec2(dragPadding.x, 0.f);
            ImVec2 t3 = bottomRightCorner - HelloImGui::EmToVec2(0.f, dragPadding.y);

            ImRect dragRect = ImRect(topLeftCorner, bottomRightCorner);
            dragAreaData[dragAreaIndex].dragArea = dragRect;
            dragAreaData[dragAreaIndex].corner_type = ImDrawCornerFlags_BotRight;
        }
        dragAreaIndex++;

        // bottom-left resize area
        {
            ImVec2 bottomRightCorner = winPos + ImVec2(dragPadding.x, winSize.y);
            ImVec2 topLeftCorner = bottomRightCorner - dragPadding;

            ImRect dragRect = ImRect(topLeftCorner, bottomRightCorner);
            dragAreaData[dragAreaIndex].dragArea = dragRect;
            dragAreaData[dragAreaIndex].corner_type = ImDrawCornerFlags_BotLeft;
        }
        dragAreaIndex++;

        BorderlessResizableHandler(dragAreaData, dragAreaIndex, window, backendWindowHelper, highlightColor);
    }
}


