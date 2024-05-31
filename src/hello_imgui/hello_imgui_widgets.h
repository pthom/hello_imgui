#pragma once
// Some additional widgets and utilities for ImGui
#include "imgui.h"
#include <optional>
#include <functional>
#include <map>
#include <string>
#include <variant>

namespace HelloImGui
{
    using VoidFunction = std::function<void(void)>;

    void BeginGroupColumn(); // calls ImGui::BeginGroup()

    void EndGroupColumn();   // calls ImGui::EndGroup() + ImGui::SameLine()

    // @@md#WidgetWithResizeHandle

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
    //            widget_size = widget_with_resize_handle("plot", myWidgetFunction);
    //        }
    ImVec2 WidgetWithResizeHandle(
        const char* id,
        VoidFunction widgetGuiFunction,
        float handleSizeEm = 1.0f,
        std::optional<VoidFunction> onItemResized = std::nullopt,
        std::optional<VoidFunction> onItemHovered = std::nullopt
        );

    // @@md


    // --------------------------------------------------------------------------------------------

    // @@md#InputTextResizable

    // `InputTextResizable`: displays a resizable text input widget
    //
    // The `InputTextResizable` widget allows you to create a text input field that can be resized by the user.
    // It supports both single-line and multi-line text input.
    // Note: the size of the widget is expressed in em units.
    // **Usage example:**
    //    C++:
    //       ```cpp
    //       // Somewhere in the application state
    //       (static) InputTextData textInput("My text", true, ImVec2(10, 3));
    //       // In the GUI function
    //       bool changed = InputTextResizable("Label", &textInput);
    //       ```
    //    Python:
    //       ```python
    //       # Somewhere in the application state
    //       text_input = hello_imgui.InputTextData("My text", multiline=True, size_em=ImVec2(10, 3))
    //       # In the GUI function
    //       changed = hello_imgui.input_text_resizable("Label", text_input)
    //       ```
    struct InputTextData
    {
        // The text edited in the input field
        std::string Text;

        // An optional hint displayed when the input field is empty
        // (only works for single-line text input)
        std::string Hint;

        // If true, the input field is multi-line
        bool Multiline = false;

        // If true, the input field is resizable
        bool Resizable = true;

        // The size of the input field in em units
        ImVec2 SizeEm = ImVec2(0, 0);

        InputTextData(const std::string& text = "", bool multiline = false, ImVec2 size_em = ImVec2(0, 0)) : Text(text), Multiline(multiline), SizeEm(size_em) {}
    };
    bool InputTextResizable(const char* label, InputTextData* textInput);

    // Serialization for InputTextData
    // -------------------------------
    // to/from dict
    using DictTypeInputTextData = std::map<std::string, std::variant<std::string, bool, float>>;
    DictTypeInputTextData InputTextDataToDict(const InputTextData& data);
    InputTextData InputTextDataFromDict(const DictTypeInputTextData& dict);
    // to/from string
    std::string InputTextDataToString(const InputTextData& data);
    InputTextData InputTextDataFromString(const std::string& str);

    // @@md
}
