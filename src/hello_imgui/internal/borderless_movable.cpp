#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/internal/borderless_movable.h"
#include "hello_imgui/hello_imgui.h"
#include "imgui_internal.h"


namespace HelloImGui
{
    bool HandleBorderlessMovable(
        std::function<ScreenBounds()> fnGetWindowBounds,
        std::function<void(ScreenBounds)> fnSetWindowBounds,
        const HelloImGui::RunnerParams& runnerParams
        )
    {
        // - If using viewports, the mouse position is absolute, and we can confidently move the window
        //   knowing that the mouse position will not be affected.
        // - If not using viewports, the mouse position is relative to the window, and the window may
        //   jump from position to position when dragged: MouseDelta is not reliable in this case
        // => in this case, we do not move the window
        bool isMousePositionAbsolute = ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable;

        bool shouldClose = false;

        bool borderlessMovable = runnerParams.appWindowParams.borderlessMovable;
        bool borderlessResizable = runnerParams.appWindowParams.borderlessResizable;
        bool borderlessClosable = runnerParams.appWindowParams.borderlessClosable;
        ImVec4 borderlessHighlightColor = runnerParams.appWindowParams.borderlessHighlightColor;

        ImU32 highlightColorU32 = ImGui::GetColorU32(borderlessHighlightColor);
        if (borderlessHighlightColor.w == 0.f)
            highlightColorU32 = ImGui::GetColorU32(ImGuiCol_TitleBg, 0.6f);

        // Drag zone at the top of the window
        if (borderlessMovable)
        {
            ImVec2 dragSize(ImGui::GetMainViewport()->Size.x, ImGui::GetFontSize() * 1.5f);
            ImRect dragArea(ImGui::GetMainViewport()->Pos, ImGui::GetMainViewport()->Pos + dragSize);

            // Update dragging state
            ImVec2 mousePos = ImGui::GetMousePos();
            static bool isDragging = false;

            static ImVec2 mouseDragLastPos; // used to store DragDelta when isMousePositionAbsolute==false

            if (dragArea.Contains(mousePos) && !isDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                isDragging = true;
                mouseDragLastPos = mousePos;
            }
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && isDragging)
                isDragging = false;

            // Draw drag zone
            if (dragArea.Contains(mousePos) || isDragging)
            {
                ImGui::GetForegroundDrawList()->AddRectFilled(
                    dragArea.Min,
                    dragArea.Max,
                    highlightColorU32
                );
            }

            // Move window if dragging
            if (isDragging)
            {
                ImVec2 dragDelta;
                if (isMousePositionAbsolute)
                {
                    dragDelta = ImGui::GetMouseDragDelta(0);
                    ImGui::ResetMouseDragDelta(0);
                    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                }
                else
                {
                    dragDelta = mousePos - mouseDragLastPos;
                    // take the window movement into account, since our future mouse position
                    // will be relative to the moved window!
                    mouseDragLastPos = mousePos - dragDelta;
                }

                auto windowBounds = fnGetWindowBounds();
                windowBounds.position[0] += (int)(dragDelta.x);
                windowBounds.position[1] += (int)(dragDelta.y);
                fnSetWindowBounds(windowBounds);
            }

            // Set mouse cursor: probably not visible for moving (the cursor will be the classic arrow)
            if (dragArea.Contains(mousePos) || isDragging)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

            // Close button at the top-right of the window
            if (borderlessClosable && dragArea.Contains(mousePos))
            {
                ImVec2 margin(ImGui::GetFontSize() * 0.2f, ImGui::GetFontSize() * 0.2f);
                ImVec2 topRight(
                    ImGui::GetMainViewport()->Pos.x + ImGui::GetMainViewport()->Size.x - margin.x,
                    ImGui::GetMainViewport()->Pos.y + margin.y
                    );

                ImVec2 btnSize(ImGui::GetFontSize() * 1.f, ImGui::GetFontSize() * 1.f);
                ImVec2 btnPos(topRight.x - btnSize.x, topRight.y);
                ImRect btnArea(btnPos, btnPos + btnSize);

                //auto colorButton = ImGui::GetColorU32(ImGuiCol_Button, 0.95f);
                ImU32 colorButton = 0xFF0000BB;
                ImGui::GetForegroundDrawList()->AddCircleFilled(
                    btnArea.GetCenter(),
                    btnSize.x * 0.5f,
                    colorButton
                );

                bool mouseHoverButton = btnArea.Contains(mousePos);

                if (true)
                {
                    // Draw a cross in the circle
                    ImVec2 crossSize(btnSize.x * 0.4f, btnSize.y * 0.4f);
                    ImVec2 crossPos(btnArea.GetCenter().x - crossSize.x * 0.55f, btnArea.GetCenter().y - crossSize.y * 0.55f);
                    ImRect crossArea(crossPos, crossPos + crossSize);
                    ImGui::GetForegroundDrawList()->AddLine(
                        crossArea.Min,
                        crossArea.Max,
                        ImGui::GetColorU32(ImGuiCol_Text),
                        1.5f
                    );
                    ImGui::GetForegroundDrawList()->AddLine(
                        ImVec2(crossArea.Min.x, crossArea.Max.y),
                        ImVec2(crossArea.Max.x, crossArea.Min.y),
                        ImGui::GetColorU32(ImGuiCol_Text),
                        1.5f
                    );
                }

                // Handle click: if the mouse was pressed down on the button and released on the button, then close the window
                static bool clickedDownOnButton = false;
                if (mouseHoverButton && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    clickedDownOnButton = true;
                }
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && mouseHoverButton && clickedDownOnButton)
                {
                    clickedDownOnButton = false;
                    shouldClose = true;
                }
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    clickedDownOnButton = false;
            }
        }

        // Drag zone at the bottom-right of the window
        if (borderlessResizable)
        {
            ImVec2 bottomRightCorner = ImGui::GetMainViewport()->Pos + ImGui::GetMainViewport()->Size;
            ImVec2 topLeftCorner = bottomRightCorner - HelloImGui::EmToVec2(1.5f, 1.5f);
            ImRect dragArea = ImRect(topLeftCorner, bottomRightCorner);

            // Update dragging state
            static bool isDragging = false;
            ImVec2 mousePos = ImGui::GetIO().MousePos;
            if (dragArea.Contains(mousePos) && !isDragging && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                isDragging = true;
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                isDragging = false;

            // Draw drag zone
            if (dragArea.Contains(mousePos) || isDragging)
            {
                // Small triangle in the bottom-right corner
                ImVec2 t1 = bottomRightCorner;
                ImVec2 t2 = bottomRightCorner - HelloImGui::EmToVec2(1.5f, 0.f);
                ImVec2 t3 = bottomRightCorner - HelloImGui::EmToVec2(0.f, 1.5f);
                ImGui::GetForegroundDrawList()->AddTriangleFilled(
                    t1, t2, t3,
                    highlightColorU32
                );
            }

            // Resize window if dragging
            int minAcceptableWindowSize = 60;
            if (isDragging)
            {
                ImVec2 dragDelta = ImGui::GetMouseDragDelta(0);
                ImGui::ResetMouseDragDelta(0);
                auto windowBounds = fnGetWindowBounds();
                windowBounds.size[0] += (int)dragDelta.x;
                windowBounds.size[1] += (int)dragDelta.y;
                if (windowBounds.size[0] < minAcceptableWindowSize)
                {
                    windowBounds.size[0] = minAcceptableWindowSize;
                    isDragging = false;
                }
                if (windowBounds.size[1] < minAcceptableWindowSize)
                {
                    windowBounds.size[1] = minAcceptableWindowSize;
                    isDragging = false;
                }
                fnSetWindowBounds(windowBounds);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
            }

            // With borderless windows, SDL is able to let window resize themselves to 0,
            // and they are then impossible to resize! So we force a minimum size here:
            {
                bool tooSmall = false;
                auto windowBounds = fnGetWindowBounds();
                if (windowBounds.size[0] < minAcceptableWindowSize)
                {
                    windowBounds.size[0] = minAcceptableWindowSize;
                    tooSmall = true;
                }
                if (windowBounds.size[1] < minAcceptableWindowSize)
                {
                    windowBounds.size[1] = minAcceptableWindowSize;
                    tooSmall = true;
                }
                if (tooSmall)
                    fnSetWindowBounds(windowBounds);
            }

            // Set mouse cursor: (not visible under glfw < 3.4, which does not implement this cursor)
            if (dragArea.Contains(mousePos) || isDragging)
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        }
        return shouldClose;
    }
} // namespace HelloImGui


