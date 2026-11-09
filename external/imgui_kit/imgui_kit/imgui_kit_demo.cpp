// imgui_kit: the demo
// https://github.com/pthom/imgui_kit (MIT license)
//
// One collapsing header per topic, in the order of imgui_kit.h. Each section is marked with IMGUI_DEMO_MARKER,
// as in imgui_demo.cpp: a tool can show the code of the section under the mouse.

/*

Index of this file:
// [SECTION] Toolbar: theme, font size, frame rounding
// [SECTION] Em units
// [SECTION] Layout
// [SECTION] Widgets
// [SECTION] Touch
// [SECTION] Demo window

*/

#include "imgui_kit.h"
#include "imgui_internal.h"     // ImGui::DemoMarker()
#include "imgui_kit_internal.h"  // ImGuiTouch::FindCurrentContext()

#ifndef IMGUI_KIT_DISABLE_DEMO

#define IMGUI_DEMO_MARKER(section)  do { ImGui::DemoMarker("imgui_kit_demo.cpp", __LINE__, section); } while (0)

namespace
{

// After a label, a widget too wide for the rest of the line goes under it: a narrow window (a phone) keeps its rows
// on screen
void NextLineIfTooWide(float widget_width)
{
    if (widget_width > ImGui::GetContentRegionAvail().x)
        ImGui::NewLine();
}

//-----------------------------------------------------------------------------
// [SECTION] Toolbar: theme, font size, frame rounding
//-----------------------------------------------------------------------------

// The theme and the frame rounding apply to the whole application; "App" restores the colors it had.
// The font size applies to the demo.
void DemoToolbar(float* font_size)
{
    IMGUI_DEMO_MARKER("Toolbar");
    static ImVec4 app_colors[ImGuiCol_COUNT];
    static float app_rounding = -1.0f;  // -1: not read yet
    ImGuiStyle& style = ImGui::GetStyle();
    if (app_rounding < 0.0f)
    {
        for (int i = 0; i < ImGuiCol_COUNT; i++)
            app_colors[i] = style.Colors[i];
        app_rounding = style.FrameRounding;
    }

    // Each row: a label, then its widgets, aligned at the widest label
    ImGuiKit::BeginLabelColumn("toolbar");
    ImGuiKit::Label("Theme");
    static int theme = 0;
    const char* themes[] = { "App", "Dark", "Light", "Classic" };
    NextLineIfTooWide(ImGuiKit::SegmentedSwitchWidth(themes, IM_ARRAYSIZE(themes)));
    if (ImGuiKit::SegmentedSwitch("##theme", &theme, themes, IM_ARRAYSIZE(themes)))
    {
        if (theme == 0)
            for (int i = 0; i < ImGuiCol_COUNT; i++)
                style.Colors[i] = app_colors[i];
        else if (theme == 1)
            ImGui::StyleColorsDark();
        else if (theme == 2)
            ImGui::StyleColorsLight();
        else
            ImGui::StyleColorsClassic();
    }

    // The sliders fill their rows (fill rows): the rows fit a phone's width
    ImGuiKit::Label("Font size");
    ImGuiKit::BeginFillRow("font_size");
    ImGui::SliderFloat("##font_size", font_size, 8.0f, 40.0f, "%.0f px");
    ImGuiKit::EndFillRow();

    ImGuiKit::Label("Frame rounding");
    ImGuiKit::BeginFillRow("rounding");
    ImGui::SliderFloat("##rounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f px");
    ImGuiKit::FillRowTail();
    ImGuiKit::HelpMarker("The style's FrameRounding: ImGui::GetStyle().FrameRounding. Buttons, sliders, input "
                         "fields and the segmented switch round their corners with it. 0 gives square frames; half "
                         "the frame's height gives a pill.");
    ImGui::SameLine();
    if (ImGui::Button("Reset"))  // the application's font size and frame rounding
    {
        *font_size = style.FontSizeBase;
        style.FrameRounding = app_rounding;
    }
    ImGuiKit::EndFillRow();
    ImGuiKit::EndLabelColumn();
}

//-----------------------------------------------------------------------------
// [SECTION] Em units
//-----------------------------------------------------------------------------

void DemoEmUnits()
{
    IMGUI_DEMO_MARKER("Em units");
    if (!ImGui::CollapsingHeader("Em units"))
        return;
    ImGui::TextWrapped("Sizes in em follow the font: the first button is 8 em wide and 2 em high, on any screen. "
                       "Change the font size at the top: it grows with the text, as a standard button does. "
                       "The last button keeps its size in pixels: at a large font, its label gets cut.");
    ImGui::Button("8 x 2 em", ImGuiKit::EmToVec2(8.f, 2.f));
    ImGui::SameLine();
    ImGui::Button("A standard button");
    ImGui::SameLine();
    ImGui::Button("100 x 30 px", ImVec2(100.f, 30.f));
    ImGui::SameLine();
    ImGuiKit::HelpMarker("EmToVec2(8, 2) gives the size in pixels, from the current font size.");
    ImGui::Text("1 em = %.1f px", ImGuiKit::EmSize());
}

//-----------------------------------------------------------------------------
// [SECTION] Layout
//-----------------------------------------------------------------------------

void DemoFillRow()
{
    IMGUI_DEMO_MARKER("Layout/Fill row");
    if (!ImGui::TreeNode("Fill row"))
        return;
    ImGui::TextWrapped("The slider fills the line; the arrows and the (i) stay on the right. Resize the window.");
    static int slice = 37;
    ImGuiKit::BeginFillRow("slice");
    ImGui::SliderInt("##slice", &slice, 0, 59, "Slice %d of 59");
    ImGuiKit::FillRowTail();
    if (ImGui::ArrowButton("##prev", ImGuiDir_Left) && slice > 0)
        slice--;
    ImGui::SameLine();
    if (ImGui::ArrowButton("##next", ImGuiDir_Right) && slice < 59)
        slice++;
    ImGui::SameLine();
    ImGuiKit::HelpMarker("BeginFillRow(), the slider, FillRowTail(), the arrows and this marker, EndFillRow().");
    ImGuiKit::EndFillRow();

    static char text[64] = "An InputText fills the line too";
    ImGuiKit::BeginFillRow("input");
    ImGui::InputText("##input", text, IM_ARRAYSIZE(text));
    ImGuiKit::FillRowTail();
    ImGui::Button("Send");
    ImGuiKit::EndFillRow();
    ImGui::TreePop();
}

void DemoAlignedGroup()
{
    IMGUI_DEMO_MARKER("Layout/Aligned group");
    if (!ImGui::TreeNode("Aligned group"))
        return;
    ImGuiKit::BeginAlignedGroup("right", 1.0f);
    ImGui::Button("Cancel");
    ImGui::SameLine();
    ImGui::Button("Apply");
    ImGuiKit::EndAlignedGroup();

    ImGuiKit::BeginAlignedGroup("center", 0.5f);
    ImGui::Text("A centered group");
    ImGuiKit::EndAlignedGroup();

    ImGuiKit::BeginAlignedGroup("center_two_lines", 0.5f);
    ImGui::Text("Two lines, centered as one box:");
    ImGui::Button("OK");
    ImGuiKit::EndAlignedGroup();
    ImGui::SameLine();
    ImGuiKit::HelpMarker("The group's box is centered, not each line: the button stays at the box's left.");
    ImGui::TreePop();
}

void DemoColumn()
{
    IMGUI_DEMO_MARKER("Layout/Column");
    if (!ImGui::TreeNode("Column"))
        return;
    ImGui::TextWrapped("Each column knows its width: the aligned groups inside it center its slider and its help "
                       "under its label, whatever the label's length.");
    static float values[] = { 0.3f, 0.6f, 0.45f };
    const char* labels[] = { "Bass", "Midrange", "Treble" };
    const char* helps[] = { "Bass: below 250 Hz.", "Midrange: from 250 Hz to 4 kHz.", "Treble: above 4 kHz." };
    for (int i = 0; i < 3; i++)
    {
        ImGui::PushID(i);
        ImGuiKit::BeginColumn("column");
        ImGui::Text("%s", labels[i]);
        ImGuiKit::BeginAlignedGroup("slider", 0.5f);
        ImGui::VSliderFloat("##v", ImGuiKit::EmToVec2(1.6f, 6.f), &values[i], 0.0f, 1.0f, "");
        ImGuiKit::EndAlignedGroup();
        ImGuiKit::BeginAlignedGroup("help", 0.5f);
        ImGuiKit::HelpMarker(helps[i]);
        ImGuiKit::EndAlignedGroup();
        ImGuiKit::EndColumn();
        ImGui::PopID();
        ImGui::SameLine(0.0f, ImGuiKit::EmSize(1.5f));
    }
    ImGui::NewLine();
    ImGui::TreePop();
}

void DemoLabelColumn()
{
    IMGUI_DEMO_MARKER("Layout/Label column");
    if (!ImGui::TreeNode("Label column"))
        return;
    ImGui::TextWrapped("Labels on the left, and the widgets after them aligned at the widest label. "
                       "After its label, a row is a normal line: SameLine() adds widgets to it.");
    static int labels_align = 0;
    static float volume = 0.5f;
    static float balance = 0.0f;
    static bool muted = false;
    ImGuiKit::BeginLabelColumn("audio", labels_align == 0 ? 0.0f : 1.0f);
    ImGuiKit::Label("Labels");
    const char* aligns[] = { "Left", "Right" };
    ImGuiKit::SegmentedSwitch("##labels_align", &labels_align, aligns, IM_ARRAYSIZE(aligns));
    ImGuiKit::Label("Volume");
    ImGui::SliderFloat("##volume", &volume, 0.0f, 1.0f);
    ImGuiKit::Label("Left/right balance");
    ImGui::SliderFloat("##balance", &balance, -1.0f, 1.0f);
    ImGuiKit::Label("Output");
    ImGui::Checkbox("Muted", &muted);
    ImGui::SameLine();
    ImGui::Button("Test sound");
    ImGuiKit::EndLabelColumn();
    ImGui::TreePop();
}

void DemoGroupPanel()
{
    IMGUI_DEMO_MARKER("Layout/Group panel");
    if (!ImGui::TreeNode("Group panel"))
        return;
    ImGui::TextWrapped("A group in a frame, its label in the frame's top border. The look of thedmd's group panel "
                       "(Dear ImGui issue #1496).");
    ImGuiKit::BeginGroupPanel("Image info");
    ImGui::Text("Size: 640 x 480");
    ImGui::Text("Channels: 3");
    ImGui::Text("Depth: 8 bits");
    ImGuiKit::EndGroupPanel();
    ImGui::SameLine();
    static float gamma = 1.0f;
    static bool flip = false;
    ImGuiKit::BeginGroupPanel("Display");
    ImGuiKit::BeginLabelColumn("display");
    ImGuiKit::Label("Gamma");
    ImGui::SetNextItemWidth(ImGuiKit::EmSize(8.f));
    ImGui::SliderFloat("##gamma", &gamma, 0.2f, 3.0f);
    ImGuiKit::Label("Flip");
    ImGui::Checkbox("##flip", &flip);
    ImGuiKit::EndLabelColumn();
    ImGuiKit::EndGroupPanel();
    ImGui::TreePop();
}

void DemoFlow()
{
    IMGUI_DEMO_MARKER("Layout/Flow");
    if (!ImGui::TreeNode("Flow"))
        return;
    ImGui::TextWrapped("The buttons stay on a line while they fit. Resize the window: they flow to the next line.");
    static int selected = 0;
    const char* presets[] = { "Coral", "Lavender", "Mint", "Sunflower yellow", "Sky", "Peach", "Forest green",
                              "Sand", "Honey", "Midnight blue", "Rose", "Turquoise", "Olive", "Salmon",
                              "Steel blue", "Plum", "Ivory", "Terracotta", "Sea green", "Mustard", "Slate gray",
                              "Cherry red", "Teal", "Custom" };  // enough to flow onto several lines in a wide window
    ImGuiKit::BeginFlow("presets");
    for (int i = 0; i < IM_ARRAYSIZE(presets); i++)
    {
        ImGuiKit::FlowNext();
        bool is_selected = (i == selected);
        if (is_selected)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        if (ImGui::Button(presets[i]))
            selected = i;
        if (is_selected)
            ImGui::PopStyleColor();
    }
    ImGuiKit::EndFlow();
    ImGui::TreePop();
}

void DemoLayout()
{
    IMGUI_DEMO_MARKER("Layout");
    if (!ImGui::CollapsingHeader("Layout"))
        return;
    ImGui::TextWrapped("These helpers measure their parts on the previous frame. The first frame draws them hidden.");
    DemoFillRow();
    DemoAlignedGroup();
    DemoColumn();
    DemoLabelColumn();
    DemoGroupPanel();
    DemoFlow();
}

//-----------------------------------------------------------------------------
// [SECTION] Widgets
//-----------------------------------------------------------------------------

void DemoHelpMarker()
{
    IMGUI_DEMO_MARKER("Widgets/Help marker");
    if (!ImGui::TreeNode("Help marker"))
        return;
    ImGui::TextWrapped("A touch screen has no hover: a tooltip never shows there. The (i) opens on a tap as well, "
                       "so its text reaches every user.");
    ImGui::BulletText("With a mouse: hover to read, click to pin.");
    ImGui::BulletText("On a touch screen: tap to open, tap elsewhere to close.");
    ImGui::Text("Try it:");
    ImGui::SameLine();
    ImGuiKit::HelpMarker("With a mouse, hovering shows this text, and a click pins it until a click elsewhere. "
                         "On a touch screen, a tap opens it.");
    ImGui::Text("Any content, with Begin/End:");
    ImGui::SameLine();
    if (ImGuiKit::BeginHelpMarker("##custom"))
    {
        ImGui::Text("A help with widgets:");
        ImGui::BulletText("a list");
        ImGui::Button("and a button");
        ImGuiKit::EndHelpMarker();
    }
    ImGui::TreePop();
}

void DemoIconButton()
{
    IMGUI_DEMO_MARKER("Widgets/Icon button");
    if (!ImGui::TreeNode("Icon button"))
        return;
    ImGui::TextWrapped("The icons are drawn with the window's draw list (lines, rectangles, circles): "
                       "no icon font to load, and they follow the font size.");
    static float zoom = 1.0f;
    if (ImGuiKit::IconButton("##minus", ImGuiKitIcon_Minus)) zoom /= 1.25f;
    ImGui::SameLine();
    if (ImGuiKit::IconButton("##plus", ImGuiKitIcon_Plus)) zoom *= 1.25f;
    ImGui::SameLine();
    if (ImGuiKit::IconButton("##zoom_in", ImGuiKitIcon_ZoomIn)) zoom *= 1.25f;
    ImGui::SameLine();
    if (ImGuiKit::IconButton("##zoom_out", ImGuiKitIcon_ZoomOut)) zoom /= 1.25f;
    ImGui::SameLine();
    if (ImGuiKit::IconButton("##zoom_1", ImGuiKitIcon_ZoomOneToOne)) zoom = 1.0f;
    ImGui::SameLine();
    if (ImGuiKit::IconButton("##zoom_fit", ImGuiKitIcon_ZoomFit)) zoom = 0.5f;
    ImGui::SameLine();
    ImGui::Text("zoom: %.2f", zoom);
    ImGui::TreePop();
}

void DemoSegmentedSwitch()
{
    IMGUI_DEMO_MARKER("Widgets/Segmented switch");
    if (!ImGui::TreeNode("Segmented switch"))
        return;
    ImGui::TextWrapped("Its corners follow the style's FrameRounding, as a button's do: change it at the top.");
    static int view = 0;
    const char* views[] = { "Demo", "Code" };
    ImGuiKit::SegmentedSwitch("##view", &view, views, IM_ARRAYSIZE(views));
    ImGui::SameLine();
    ImGui::Text("showing: %s", views[view]);
    static int unit = 1;
    const char* units[] = { "mm", "cm", "inch" };
    ImGuiKit::SegmentedSwitch("##unit", &unit, units, IM_ARRAYSIZE(units));

    ImGui::TextWrapped("One switch as a pill: push a FrameRounding of half the frame's height around it.");
    static int period = 1;
    const char* periods[] = { "Day", "Week", "Month" };
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, ImGui::GetFrameHeight() * 0.5f);
    ImGuiKit::SegmentedSwitch("##period", &period, periods, IM_ARRAYSIZE(periods));
    ImGui::PopStyleVar();
    ImGui::TreePop();
}

void DemoFitText()
{
    IMGUI_DEMO_MARKER("Widgets/Fit text");
    if (!ImGui::TreeNode("Fit text"))
        return;
    ImGui::TextWrapped("A text cut at a word, with an ellipsis, to fit a number of lines: a card's title or summary.");
    static float width_em = 16.0f;
    static int max_lines = 2;
    ImGuiKit::BeginLabelColumn("fit_text");
    ImGuiKit::Label("Width");
    ImGui::SetNextItemWidth(ImGuiKit::EmSize(10.f));
    ImGui::SliderFloat("##width", &width_em, 4.0f, 40.0f, "%.0f em");
    ImGuiKit::Label("Lines");
    ImGui::SetNextItemWidth(ImGuiKit::EmSize(10.f));
    ImGui::SliderInt("##lines", &max_lines, 1, 5);
    ImGuiKit::EndLabelColumn();

    const char* text = "Napoleon's army crossed the Niemen in June 1812 with more than four hundred thousand men; "
                       "Minard drew its march to Moscow and its retreat as a band whose width follows the number of "
                       "soldiers, and the temperature below it.";
    float width = ImGuiKit::EmSize(width_em);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width);
    ImGui::TextUnformatted(ImGuiKit::FitText(text, width, max_lines));
    ImGui::PopTextWrapPos();
    float height = ImGui::GetTextLineHeight() * (float)max_lines;  // the room the text may take, framed
    ImGui::GetWindowDrawList()->AddRect(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(ImGuiCol_Border));
    ImGui::Dummy(ImVec2(0.0f, ImMax(0.0f, p.y + height - ImGui::GetCursorScreenPos().y)));
    ImGui::TreePop();
}

void DemoResizeHandle()
{
    IMGUI_DEMO_MARKER("Widgets/Resize handle");
    if (!ImGui::TreeNode("Resize handle"))
        return;
    ImGui::TextWrapped("Drag the grip at the bottom-right corner: the widget before it takes the new size. "
                       "Any widget that takes a size: a text field, a child window, a plot.");
    static char text[512] = "A text field that the reader can make larger.\nDrag its corner.";
    static ImVec2 text_size = ImGuiKit::EmToVec2(20.f, 4.f);
    ImGui::InputTextMultiline("##text", text, IM_ARRAYSIZE(text), text_size);
    ImGuiKit::ResizeHandle("##text_size", &text_size, ImGuiKit::EmToVec2(6.f, 2.f));

    static ImVec2 child_size = ImGuiKit::EmToVec2(20.f, 6.f);
    ImGui::BeginChild("##child", child_size, ImGuiChildFlags_Borders);
    ImGui::Text("A child window, %.0f x %.0f px", child_size.x, child_size.y);
    ImGui::EndChild();
    ImGuiKit::ResizeHandle("##child_size", &child_size, ImGuiKit::EmToVec2(10.f, 3.f));
    ImGui::TreePop();
}

void DemoWidgets()
{
    IMGUI_DEMO_MARKER("Widgets");
    if (!ImGui::CollapsingHeader("Widgets"))
        return;
    DemoHelpMarker();
    DemoIconButton();
    DemoSegmentedSwitch();
    DemoFitText();
    DemoResizeHandle();
}

//-----------------------------------------------------------------------------
// [SECTION] Touch
//-----------------------------------------------------------------------------

// A canvas that zooms and pans, as a map or an image viewer does. With a mouse: the wheel zooms (it claims the wheel
// while hovered: ImGui::SetItemKeyOwner), a drag pans. With fingers: one finger pans at once
// (ImGuiTouch::SetItemTakesDrags), two fingers zoom and pan at once (ImGuiTouch::ItemPinch: the gesture's data)
void ZoomableCanvas(const char* str_id, ImVec2 size)
{
    static float zoom = 1.0f;
    static ImVec2 offset(0.0f, 0.0f);  // where the content's origin is, from the canvas's corner, in pixels
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1(p0.x + size.x, p0.y + size.y);
    // Zoom by `factor` around a point of the screen: the content under it stays there
    auto zoom_around = [&](ImVec2 point, float factor) {
        float new_zoom = ImClamp(zoom * factor, 0.25f, 8.0f);
        float px = point.x - p0.x, py = point.y - p0.y;
        offset.x = px - (px - offset.x) * new_zoom / zoom;
        offset.y = py - (py - offset.y) * new_zoom / zoom;
        zoom = new_zoom;
    };

    ImGui::InvisibleButton(str_id, size);
    ImGuiTouch::SetItemTakesDrags();
    ImGuiTouchPinch pinch = ImGuiTouch::ItemPinch();
    if (pinch.Active)
    {
        zoom_around(pinch.Center, pinch.ScaleDelta);
        offset.x += pinch.CenterDelta.x;
        offset.y += pinch.CenterDelta.y;
    }
    else if (ImGui::IsItemActive())
    {
        offset.x += io.MouseDelta.x;
        offset.y += io.MouseDelta.y;
    }
    if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY) && io.MouseWheel != 0.0f)
        zoom_around(io.MousePos, ImPow(1.1f, io.MouseWheel));

    // The gesture, as the canvas sees it: a dot under one finger (or the mouse); for two fingers, a ring at their
    // middle that follows their spread, and the layer's decision (pinch or pan). It fades once they lift.
    static float gesture_alpha = 0.0f;
    static bool gesture_two = false;
    static ImVec2 gesture_pos;
    static float gesture_scale = 1.0f;
    static ImGuiTouchPinchKind gesture_kind = ImGuiTouchPinchKind_Undecided;
    if (pinch.Active)
    {
        gesture_two = true;
        gesture_pos = pinch.Center;
        gesture_scale = pinch.Scale;
        gesture_kind = pinch.Kind;
        gesture_alpha = 1.0f;
    }
    else if (ImGui::IsItemActive())
    {
        gesture_two = false;
        gesture_pos = io.MousePos;
        gesture_alpha = 1.0f;
    }
    else if (gesture_alpha > 0.0f)
    {
        gesture_alpha = ImMax(0.0f, gesture_alpha - io.DeltaTime * 3.0f);
        ImGuiKit::RequestAnimation();  // an app that idles still draws the fade
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float rounding = ImGui::GetStyle().FrameRounding;
    draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_FrameBg), rounding);
    draw_list->PushClipRect(p0, p1, true);
    float step = ImGuiKit::EmSize() * zoom;  // a grid of 1 em at zoom 1
    ImU32 grid_color = ImGui::GetColorU32(ImGuiCol_Border);
    for (float x = p0.x + ImFmod(offset.x, step); x < p1.x; x += step)
        draw_list->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), grid_color);
    for (float y = p0.y + ImFmod(offset.y, step); y < p1.y; y += step)
        draw_list->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), grid_color);
    ImVec2 center(p0.x + offset.x + 4.0f * step, p0.y + offset.y + 3.0f * step);
    draw_list->AddCircleFilled(center, 1.5f * step, ImGui::GetColorU32(ImGuiCol_SliderGrabActive));
    draw_list->AddRectFilled(ImVec2(center.x + 3.0f * step, center.y - step), ImVec2(center.x + 6.0f * step, center.y + step),
                             ImGui::GetColorU32(ImGuiCol_ButtonActive));
    if (gesture_alpha > 0.0f)
    {
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text, gesture_alpha);
        float em = ImGuiKit::EmSize();
        if (!gesture_two)
            draw_list->AddCircleFilled(gesture_pos, 0.4f * em, color);
        else
        {
            float radius = 1.5f * em * gesture_scale;
            draw_list->AddCircle(gesture_pos, radius, color, 0, 2.0f);
            draw_list->AddCircleFilled(gesture_pos, 0.15f * em, color);
            const char* kind = (gesture_kind == ImGuiTouchPinchKind_Pinch) ? "pinch"
                             : (gesture_kind == ImGuiTouchPinchKind_Pan) ? "pan" : "two fingers";
            draw_list->AddText(ImVec2(gesture_pos.x + radius + 0.3f * em, gesture_pos.y - 0.5f * em), color, kind);
        }
    }
    draw_list->PopClipRect();
    draw_list->AddRect(p0, p1, ImGui::GetColorU32(ImGuiCol_Border), rounding);
    char label[32];
    ImFormatString(label, IM_ARRAYSIZE(label), "zoom x%.2f", zoom);
    draw_list->AddText(ImVec2(p0.x + ImGui::GetStyle().FramePadding.x, p1.y - ImGui::GetTextLineHeightWithSpacing()),
                       ImGui::GetColorU32(ImGuiCol_Text), label);
}

const char* TOUCH_SETUP_CODE = R"(ImGui::CreateContext();
ImGuiTouch::Setup();            // the touch layer: swipe, long press, pinch
ImGuiKit::SetupWheelSession();  // the wheel stays with the window it scrolls
#ifdef __EMSCRIPTEN__
ImGuiTouch::SetupEmscripten();  // in a browser: which pointer is a finger, the fingers
#endif

// The render loop: nothing to add, the kit runs inside ImGui::NewFrame() and ImGui::Render().
// - An app that sleeps when idle keeps drawing while ImGuiKit::IsAnimating().
// - A widget that a finger drags: ImGuiTouch::SetItemTakesDrags() right after it.
// - A widget that zooms: ImGuiTouch::ItemPinch() right after it (SetItemPinchAsWheel() if it zooms by the wheel).
// - A widget that reads the wheel claims it: if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY)) ...
// Hello ImGui does the setup itself (RunnerParams::touchConfig).)";

// The settings change the application's touch layer, live. "Simulate a touch source" lets the mouse play a finger.
// Without the touch layer (an app that shows this demo without ImGuiTouch::Setup()), a note instead of the section
bool HasTouchLayer()
{
    if (ImGuiTouch::FindCurrentContext() != nullptr)
        return true;
    ImGui::TextWrapped("This application did not set up the touch layer: ImGuiTouch::Setup(), once after "
                       "ImGui::CreateContext() (Hello ImGui does it).");
    return false;
}

void DemoTouch()
{
    IMGUI_DEMO_MARKER("Touch");
    if (!ImGui::CollapsingHeader("Touch"))
        return;
    if (!HasTouchLayer())
        return;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::TextWrapped("The gestures of a phone: a finger drags the content (with inertia, and a bounce at the end), "
                       "a long press is a right click, two fingers pinch the font.");
    if (ImGui::TreeNode("The setup code"))
    {
        ImGui::TextUnformatted(TOUCH_SETUP_CODE);
        ImGui::TreePop();
    }

    ImGuiTouchConfig& config = ImGuiTouch::GetConfig();
    ImGuiKit::BeginLabelColumn("touch_settings");
    if (!(io.ConfigFlags & ImGuiConfigFlags_IsTouchScreen))
    {
        ImGuiKit::Label("On a desktop");
        ImGui::Checkbox("Simulate a touch source", &config.SimulateTouch);
        ImGui::SameLine();
        ImGuiKit::HelpMarker("The mouse plays a finger (ImGuiTouchConfig::SimulateTouch): every gesture works in the "
                             "zone below, as on a phone:\n\n"
                             "- Drag up or down to scroll the content. Start moving as soon as you press, as with a "
                             "finger.\n"
                             "- Press and hold, then release: a long press, which opens the zone's context menu (a "
                             "right click).");
    }
    ImGuiKit::Label("Swipe");
    const char* scroll_modes[] = { "Touch", "Always", "Off" };
    NextLineIfTooWide(ImGuiKit::SegmentedSwitchWidth(scroll_modes, IM_ARRAYSIZE(scroll_modes)));
    ImGuiKit::SegmentedSwitch("##scroll_mode", &config.ScrollMode, scroll_modes, IM_ARRAYSIZE(scroll_modes));
    ImGui::SameLine();
    ImGuiKit::HelpMarker("Touch: the swipe follows a finger only. Always: the mouse's left button drags the "
                         "content too, while the mouse stays a mouse.");
    ImGuiKit::Label("Pinch");
    ImGui::Checkbox("scales the font", &config.PinchScalesFont);
    ImGuiKit::Label("Long press");
    ImGui::Checkbox("is a right click", &config.LongPressIsRightClick);
    ImGuiKit::Label("Scroll bars");
    ImGui::Checkbox("thin", &config.ThinScrollbars);
    ImGuiKit::EndLabelColumn();

    ImGui::BeginChild("##touch_zone", ImVec2(0.0f, ImGuiKit::EmSize(16.f)), ImGuiChildFlags_Borders);
    ImGui::Bullet();
    ImGui::TextWrapped("Swipe: a finger that drags this zone scrolls it, with inertia, and a bounce at the end.");
    ImGui::Bullet();
    ImGui::TextWrapped("Tap: a click, when the finger lifts.");
    ImGui::Bullet();
    ImGui::TextWrapped("Hold, then drag: the widget under the finger takes it (the text below: it selects).");
    ImGui::Bullet();
    ImGui::TextWrapped("Long press, then lift: a right click (this zone's context menu).");
    ImGui::Bullet();
    ImGui::TextWrapped("Two fingers: a pinch scales the font (on the canvas, it zooms the canvas); moved together, a "
                       "right drag.");
    // A text to select: a drag that starts on it scrolls the zone; a short hold, then a drag, selects (as on a
    // phone). A ring shows when the hold took effect.
    static char text[] = "Napoleon's army crossed the Niemen in June 1812, with more than four hundred thousand men. "
                         "Six months later, fewer than ten thousand came back.";
    ImGui::TextWrapped("Select a few words: a short hold, then a drag.");
    ImGui::InputTextMultiline("##select", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGuiKit::EmSize(4.f)),
                              ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_WordWrap);

    ZoomableCanvas("##canvas", ImGuiKit::EmToVec2(16.f, 8.f));
    ImGui::SameLine();
    ImGuiKit::HelpMarker("A canvas that zooms and pans, as a plot or an image viewer does:\n\n"
                         "- It calls ImGuiTouch::SetItemTakesDrags() right after it: a drag that starts on it pans it "
                         "at once. So the zone cannot be scrolled from inside the canvas.\n"
                         "- It claims the wheel while hovered: if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY)). With "
                         "the wheel session, a wheel that scrolls the zone keeps scrolling it when the canvas passes "
                         "under the mouse; a wheel that starts on the canvas zooms it.\n"
                         "- It reads two fingers' gesture with ImGuiTouch::ItemPinch(): their spread zooms it and their "
                         "move pans it, both at once, as on a map. The font keeps its size.");
    if (ImGui::BeginPopupContextWindow("##zone_menu"))
    {
        ImGui::TextUnformatted("The zone's context menu: a right click, or a long press");
        ImGui::EndPopup();
    }

    // A child that scrolls sideways: a swipe inside scrolls it, not the zone. Its buttons count their taps. Its slider
    // takes the sideways drags (ImGuiTouch::SetItemTakesDrags(ImGuiTouchAxis_X)): a sideways drag on it moves it at once
    ImGui::TextWrapped("A sideways swipe scrolls this row; a tap clicks a button; a sideways drag moves the slider:");
    ImGui::BeginChild("##sideways", ImVec2(0.0f, ImGuiKit::EmSize(5.f)), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_HorizontalScrollbar);
    static int taps[8] = {};
    float card_width = ImMax(ImGuiKit::EmSize(6.f), ImGui::GetContentRegionAvail().x * 0.3f);  // 3 per screen: it scrolls
    static float row_value = 0.5f;
    ImGui::SetNextItemWidth(card_width);
    ImGui::SliderFloat("##row_slider", &row_value, 0.0f, 1.0f);
    ImGuiTouch::SetItemTakesDrags(ImGuiTouchAxis_X);
    for (int i = 0; i < IM_ARRAYSIZE(taps); i++)
    {
        ImGui::SameLine();
        char label[32];
        ImFormatString(label, IM_ARRAYSIZE(label), "Card %d\n%d taps###card%d", i + 1, taps[i], i);
        if (ImGui::Button(label, ImVec2(card_width, ImGuiKit::EmSize(3.f))))
            taps[i]++;
    }
    ImGui::EndChild();
    for (int i = 1; i <= 8; i++)
        ImGui::Text("More content, to scroll (%d)", i);
    ImGui::EndChild();
    // IsAnimating(): an application that sleeps when nothing happens keeps drawing frames meanwhile.
    // ImGuiKit::IsAnimating() adds the kit's widgets (a segmented switch that slides) to the touch layer's.
    ImGui::Text("Moving on its own: touch %s, kit %s", ImGuiTouch::IsAnimating() ? "yes" : "no",
                ImGuiKit::IsAnimating() ? "yes" : "no");
}

// The phone's keyboard, in a browser: the fields of all kinds, one of them in a child that scrolls
void DemoTouchKeyboard()
{
    IMGUI_DEMO_MARKER("Touch keyboard");
    if (!ImGui::CollapsingHeader("Touch keyboard"))
        return;
    if (!HasTouchLayer())
        return;
    ImGui::TextWrapped("In a browser on a phone: tap a field, then the keyboard button that shows next to it (or the "
                       "field again). The phone's keyboard opens, and a full-screen window keeps out of it "
                       "(ImGuiTouch::GetKeyboardInsets()). The application calls ImGuiTouch::SetupEmscriptenKeyboard() "
                       "once.");
    static char line[128] = "";
    ImGui::InputTextWithHint("text", "type here", line, IM_ARRAYSIZE(line));
    static char lines[1024] = "Several lines:\nmove the caret from the keyboard's space bar.";
    ImGui::InputTextMultiline("lines", lines, IM_ARRAYSIZE(lines), ImVec2(0.0f, ImGuiKit::EmSize(5.f)));

    ImGui::TextWrapped("A field in a child that scrolls: it stays above the keyboard.");
    ImGui::BeginChild("##keyboard_child", ImVec2(0.0f, ImGuiKit::EmSize(10.f)), ImGuiChildFlags_Borders);
    for (int i = 1; i <= 12; i++)
        ImGui::Text("Content above the field (%d)", i);
    static char deep[128] = "";
    ImGui::InputTextWithHint("field", "type here", deep, IM_ARRAYSIZE(deep));
    for (int i = 1; i <= 12; i++)
        ImGui::Text("Content below the field (%d)", i);
    ImGui::EndChild();
}

} // namespace

//-----------------------------------------------------------------------------
// [SECTION] Demo window
//-----------------------------------------------------------------------------

void ImGuiKit::ShowDemoWindow(bool* p_open)
{
    ImGui::SetNextWindowSize(ImGuiKit::EmToVec2(36.f, 30.f), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ImGuiKit Demo", p_open))
    {
        ImGui::End();
        return;
    }
    ImGuiKit::ShowDemoContents();
    ImGui::End();
}

void ImGuiKit::ShowDemoContents()
{
    ImGui::TextWrapped("ImGuiKit %s: small, standalone additions to Dear ImGui", IMGUI_KIT_VERSION);
    static float font_size = 0.0f;  // 0: the application's size, read at the first frame
    if (font_size == 0.0f)
        font_size = ImGui::GetStyle().FontSizeBase;
    DemoToolbar(&font_size);
    ImGui::Separator();
    ImGui::PushFont(nullptr, font_size);
    DemoEmUnits();
    DemoLayout();
    DemoWidgets();
    DemoTouch();
    DemoTouchKeyboard();
    ImGui::PopFont();
}

#else

void ImGuiKit::ShowDemoWindow(bool*) {}
void ImGuiKit::ShowDemoContents() {}

#endif // #ifndef IMGUI_KIT_DISABLE_DEMO
