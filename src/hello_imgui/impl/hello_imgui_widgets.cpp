#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/hello_imgui_widgets.h"
#include "hello_imgui/dpi_aware.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "nlohmann/json.hpp"

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
        const char* id,
        VoidFunction widgetGuiFunction,
        float handleSizeEm,
        std::optional<VoidFunction> onItemResized,
        std::optional<VoidFunction> onItemHovered
    )
    {
        widgetGuiFunction();

        ImGuiID widget_id = ImGui::GetID(id);
        ImVec2 widget_size = ImGui::GetItemRectSize();

        if (ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
        {
            if (onItemHovered.has_value() && onItemHovered)
                onItemHovered.value()();
        }

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

        bool wasMouseJustClicked = !previousResizingState.MouseDown && resizingState->MouseDown;
        bool mouseInZoneBeforeAfter = previousResizingState.MouseInResizingZone && resizingState->MouseInResizingZone;

        ImVec2 mouseDelta = resizingState->MousePosition - previousResizingState.MousePosition;

        // Color
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Button);
        if (ImGui::IsMouseHoveringRect(zone.Min, zone.Max))
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
            color = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        }
        if (resizingState->Resizing)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
            color = ImGui::GetColorU32(ImGuiCol_ButtonActive);
        }

        ImGui::GetWindowDrawList()->AddTriangleFilled(br, bl, tr, color);

        if (mouseInZoneBeforeAfter)

        if (!resizingState->Resizing)
        {
            if (wasMouseJustClicked && mouseInZoneBeforeAfter)
            {
                if (onItemResized.has_value() && onItemResized)
                    onItemResized.value()();
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
                resizingState->Resizing = false;
            }
        }

        return widget_size;
    }


    static void setDefaultSizeIfNone(InputTextData* textInput)
    {
        if (textInput->SizeEm.x == 0 && textInput->SizeEm.y == 0)
        {
            float defaultWidth = 15.f;
            float defaultHeight = 5.f;
            if (textInput->Multiline)
                textInput->SizeEm = ImVec2(defaultWidth, defaultHeight);
            else
                textInput->SizeEm = ImVec2(defaultWidth, 0);
        }
    }

    bool InputTextResizable(const char* label, InputTextData* textInput)
    {
        setDefaultSizeIfNone(textInput);

        static std::string hash_hash = "##";

        bool labelIsHidden = (label[0] == '#' && label[1] == '#');
        std::string inputLabel = labelIsHidden ? label : hash_hash + label;
        std::string visibleLabel = labelIsHidden ? std::string(label).substr(2) : label;

        bool changed = false;

        auto gui_cb = [&]()
        {
            if (textInput->Multiline)
            {
                changed = ImGui::InputTextMultiline(inputLabel.c_str(), &textInput->Text, HelloImGui::EmToVec2(textInput->SizeEm));
            }
            else
            {
                ImGui::SetNextItemWidth(HelloImGui::EmSize(textInput->SizeEm.x));
                if (textInput->Hint.empty())
                    changed = ImGui::InputText(inputLabel.c_str(), &textInput->Text);
                else
                    changed = ImGui::InputTextWithHint(inputLabel.c_str(), textInput->Hint.c_str(), &textInput->Text);
            }
        };

        if (textInput->Resizable)
        {
            ImVec2 newSizePixels = HelloImGui::WidgetWithResizeHandle(label, gui_cb, 0.8f);
            ImVec2 newSize = HelloImGui::PixelsToEm(newSizePixels);
            if (textInput->Multiline)
                textInput->SizeEm = newSize;
            else
                textInput->SizeEm.x = newSize.x;
            if (textInput->SizeEm.x < 1.f)
                textInput->SizeEm.x = 1.f;
        }
        else
        {
            gui_cb();
        }

        if (!labelIsHidden)
        {
            ImGui::SameLine();
            ImGui::Text("%s", visibleLabel.c_str());
        }

        return changed;
    }

    // Serialization to/from dict
    DictTypeInputTextData InputTextDataToDict(const InputTextData& data)
    {
        return {
            {"Text", data.Text},
            {"Multiline", data.Multiline},
            {"SizeEm_x", data.SizeEm.x},
            {"SizeEm_y", data.SizeEm.y}
        };
    }

    InputTextData InputTextDataFromDict(const DictTypeInputTextData& dict)
    {
        InputTextData result;
        if (dict.find("Text") != dict.end())
            result.Text = std::get<std::string>(dict.at("Text"));
        if (dict.find("Multiline") != dict.end())
            result.Multiline = std::get<bool>(dict.at("Multiline"));
        if (dict.find("SizeEm_x") != dict.end())
            result.SizeEm.x = std::get<float>(dict.at("SizeEm_x"));
        if (dict.find("SizeEm_y") != dict.end())
            result.SizeEm.y = std::get<float>(dict.at("SizeEm_y"));
        return result;
    }

    // Serialization to/from string using JSON
    std::string InputTextDataToString(const InputTextData& data)
    {
        nlohmann::json j = {
            {"Text", data.Text},
            {"Multiline", data.Multiline},
            {"SizeEm_x", data.SizeEm.x},
            {"SizeEm_y", data.SizeEm.y}
        };
        return j.dump();
    }

    InputTextData InputTextDataFromString(const std::string& str)
    {
        nlohmann::json j = nlohmann::json::parse(str);
        InputTextData result;
        result.Text = j["Text"];
        result.Multiline = j["Multiline"];
        result.SizeEm.x = j["SizeEm_x"];
        result.SizeEm.y = j["SizeEm_y"];
        return result;
    }
}
