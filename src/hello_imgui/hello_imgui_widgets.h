#pragma once
// Some additional widgets and utilities for ImGui
#include "imgui.h"
#include <optional>
#include <functional>


namespace HelloImGui
{
    using VoidFunction = std::function<void(void)>;

    void BeginGroupColumn(); // calls ImGui::BeginGroup()

    void EndGroupColumn();   // calls ImGui::EndGroup() + ImGui::SameLine()

    // WidgetWithResizeHandle: adds a resize handle to a widget
    // Example usage with ImPlot:
    //        void gui()
    //        {
    //            static ImVec2 widget_size(200, 200);
    //            auto myWidgetFunction = []()
    //            {
    //                if (ImPlot::BeginPlot("My Plot", widget_size)) {
    //                    ImPlot::PlotLine("My Line", x.data(), y.data(), 1000);
    //                    ImPlot::EndPlot();
    //                }
    //            };
    //            widget_size = widget_with_resize_handle(myWidgetFunction);
    //        }
    ImVec2 WidgetWithResizeHandle(
        VoidFunction widgetGuiFunction,
        float handleSizeEm = 1.0f,
        std::optional<VoidFunction> onItemResized = std::nullopt,
        std::optional<VoidFunction> onItemHovered = std::nullopt
        );

}
