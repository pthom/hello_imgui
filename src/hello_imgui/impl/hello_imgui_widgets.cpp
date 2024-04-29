#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/hello_imgui_widgets.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <unordered_map>


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


    struct WidgetResizingState_
    {
        bool   Resizing = false;
        bool   MouseInResizingZone = false;
        bool   MouseDown = false;
        ImVec2 MousePosition = ImVec2();
    };

    static std::unordered_map<ImGuiID, WidgetResizingState_> gWidgetResizingStates;

    static WidgetResizingState_* GetWidgetResizingState_(ImGuiID widget_id)
    {
        if (gWidgetResizingStates.find(widget_id) == gWidgetResizingStates.end())
            gWidgetResizingStates[widget_id] = WidgetResizingState_();
        return &gWidgetResizingStates.at(widget_id);
    }


    ImVec2 WidgetWithResizeHandle(
        VoidFunction widgetGuiFunction,
        float handleSizeEm,
        std::optional<VoidFunction> onItemResized,
        std::optional<VoidFunction> onItemHovered
    )
    {
        widgetGuiFunction();

        if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
        {
            if (onItemHovered.has_value() && onItemHovered)
                onItemHovered.value()();
        }

        ImVec2 widget_size = ImGui::GetItemRectSize();
        ImGuiID widget_id = ImGui::GetItemID();

        float em = ImGui::GetFontSize(), size = em * handleSizeEm;
        ImVec2 widget_bottom_right = ImGui::GetItemRectMax();

        ImVec2 br(widget_bottom_right), bl(br.x - size, br.y), tr(br.x, br.y - size), tl(br.x - size, br.y - size);
        ImRect zone = ImRect(tl, br);

        //
        // Get and update resizing state
        //
        WidgetResizingState_* resizingState = GetWidgetResizingState_(widget_id);
        WidgetResizingState_ previousResizingState = *resizingState; // This is a copy

        resizingState->MousePosition = ImGui::GetIO().MousePos;
        resizingState->MouseInResizingZone = ImGui::IsMouseHoveringRect(zone.Min, zone.Max);
        resizingState->MouseDown = ImGui::IsMouseDown(0);

        ImVec2 mouseDelta = resizingState->MousePosition - previousResizingState.MousePosition;

        // Color
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Button);
        if (ImGui::IsMouseHoveringRect(zone.Min, zone.Max))
            color = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        if (resizingState->Resizing)
            color = ImGui::GetColorU32(ImGuiCol_ButtonActive);

        ImGui::GetWindowDrawList()->AddTriangleFilled(br, bl, tr, color);

        if (!resizingState->Resizing)
        {
            bool wasMouseJustClicked = !previousResizingState.MouseDown && resizingState->MouseDown;
            bool mouseInZoneBeforeAfter = previousResizingState.MouseInResizingZone && resizingState->MouseInResizingZone;
            if (wasMouseJustClicked && mouseInZoneBeforeAfter)
            {
                if (onItemResized.has_value() && onItemResized)
                    onItemResized.value()();
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                resizingState->Resizing = true;
            }
        }
        if (resizingState->Resizing)
        {
            if (onItemResized.has_value() && onItemResized)
                onItemResized.value()();
            if (ImGui::IsMouseDown(0))
            {
                if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
                {
                    widget_size.x += mouseDelta.x;
                    widget_size.y += mouseDelta.y;
                    ImGui::ResetMouseDragDelta(0);
                }
            }
            else
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                resizingState->Resizing = false;
            }
        }

        return widget_size;
    }

}