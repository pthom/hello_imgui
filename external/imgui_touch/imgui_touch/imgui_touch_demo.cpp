// imgui_touch: the demo
// https://github.com/pthom/imgui_touch (MIT license)
//
// One collapsing header per topic. Each section is marked with IMGUI_DEMO_MARKER, as in imgui_demo.cpp: a tool can
// show the code of the section under the mouse. Plain Dear ImGui only: the demo shows what an app writes.

/*

Index of this file:
// [SECTION] Helpers
// [SECTION] Gestures
// [SECTION] Keyboard
// [SECTION] Demo window

*/

#include "imgui_touch.h"
#include "imgui_touch_internal.h"  // ImGuiTouch::RequestAnimation()
#include "imgui_internal.h"        // ImGui::DemoMarker()

#ifndef IMGUI_TOUCH_DISABLE_DEMO

#define IMGUI_DEMO_MARKER(section)  do { ImGui::DemoMarker("imgui_touch_demo.cpp", __LINE__, section); } while (0)

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------

// A (?) that shows its text in a popup, opened by a click or a tap: a touch screen has no hover for a tooltip
static void HelpMarker(const char* text)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    ImGui::PushID(text);
    if (ImGui::IsItemClicked())
        ImGui::OpenPopup("##help");
    if (ImGui::BeginPopup("##help"))
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

// A label on the left of a settings table's row; the widgets go in the second column
static void SettingRow(const char* label)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableNextColumn();
}

// Without the touch layer (an app that shows this demo without ImGuiTouch::Setup()), a note instead of the section
static bool HasTouchLayer()
{
    if (ImGuiTouch::IsInstalled())
        return true;
    ImGui::TextWrapped("This application did not set up the touch layer: ImGuiTouch::Setup(), once after "
                       "ImGui::CreateContext() (Hello ImGui does it).");
    return false;
}

//-----------------------------------------------------------------------------
// [SECTION] Gestures
//-----------------------------------------------------------------------------

// A canvas that zooms and pans, as a map or an image viewer does. With a mouse: the wheel zooms (it claims the wheel
// while hovered: ImGui::SetItemKeyOwner), a drag pans. With fingers: one finger pans at once
// (ImGuiTouch::SetItemTakesDrags), two fingers zoom and pan at once (ImGuiTouch::ItemPinch: the gesture's data)
// The canvas's view: its zoom, and where the content's origin is, from the canvas's corner, in pixels
struct CanvasView
{
    float zoom = 1.0f;
    ImVec2 offset = ImVec2(0.0f, 0.0f);
};

// Zoom by `factor` around a point of the screen (relative to the canvas's corner): the content under it stays there
static void ZoomCanvasAround(CanvasView* view, ImVec2 point, float factor)
{
    float new_zoom = ImClamp(view->zoom * factor, 0.25f, 8.0f);
    view->offset.x = point.x - (point.x - view->offset.x) * new_zoom / view->zoom;
    view->offset.y = point.y - (point.y - view->offset.y) * new_zoom / view->zoom;
    view->zoom = new_zoom;
}

static void ZoomableCanvas(const char* str_id, ImVec2 size)
{
    static CanvasView view;
    float& zoom = view.zoom;
    ImVec2& offset = view.offset;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1(p0.x + size.x, p0.y + size.y);

    ImGui::InvisibleButton(str_id, size);
    ImGuiTouch::SetItemTakesDrags();
    ImGuiTouchPinch pinch = ImGuiTouch::ItemPinch();
    if (pinch.Active)
    {
        ZoomCanvasAround(&view, ImVec2(pinch.Center.x - p0.x, pinch.Center.y - p0.y), pinch.ScaleDelta);
        offset.x += pinch.CenterDelta.x;
        offset.y += pinch.CenterDelta.y;
    }
    else if (ImGui::IsItemActive())
    {
        offset.x += io.MouseDelta.x;
        offset.y += io.MouseDelta.y;
    }
    if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY) && io.MouseWheel != 0.0f)
        ZoomCanvasAround(&view, ImVec2(io.MousePos.x - p0.x, io.MousePos.y - p0.y), ImPow(1.1f, io.MouseWheel));

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
        ImGuiTouch::RequestAnimation();  // an app that idles still draws the fade
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float rounding = ImGui::GetStyle().FrameRounding;
    float em = ImGui::GetFontSize();
    draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_FrameBg), rounding);
    draw_list->PushClipRect(p0, p1, true);
    float step = em * zoom;  // a grid of 1 em at zoom 1
    ImU32 grid_color = ImGui::GetColorU32(ImGuiCol_Border);
    for (float x = p0.x + ImFmod(offset.x, step); x < p1.x; x += step)
        draw_list->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), grid_color);
    for (float y = p0.y + ImFmod(offset.y, step); y < p1.y; y += step)
        draw_list->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), grid_color);
    ImVec2 center(p0.x + offset.x + 4.0f * step, p0.y + offset.y + 3.0f * step);
    draw_list->AddCircleFilled(center, 1.5f * step, ImGui::GetColorU32(ImGuiCol_SliderGrabActive));
    draw_list->AddRectFilled(ImVec2(center.x + 3.0f * step, center.y - step),
                             ImVec2(center.x + 6.0f * step, center.y + step),
                             ImGui::GetColorU32(ImGuiCol_ButtonActive));
    if (gesture_alpha > 0.0f)
    {
        ImU32 color = ImGui::GetColorU32(ImGuiCol_Text, gesture_alpha);
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

static const char* TOUCH_SETUP_CODE = R"(ImGui::CreateContext();
ImGuiTouch::Setup();            // the touch layer: swipe, long press, pinch
#ifdef __EMSCRIPTEN__
ImGuiTouch::SetupEmscripten();  // in a browser: which pointer is a finger, the fingers
#endif

// The render loop: nothing to add, the layer runs inside ImGui::NewFrame() and ImGui::Render().
// - An app that sleeps when idle keeps drawing while ImGuiTouch::IsAnimating().
// - A widget that a finger drags: ImGuiTouch::SetItemTakesDrags() right after it.
// - A widget that zooms: ImGuiTouch::ItemPinch() right after it (SetItemPinchAsWheel() if it zooms by the wheel).
// Hello ImGui does the setup itself (RunnerParams::touch_config).)";

// The settings change the application's touch layer, live. "Simulate touch" lets the mouse play a finger.
static void DemoGestures()
{
    IMGUI_DEMO_MARKER("Gestures");
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    if (!ImGui::CollapsingHeader("Gestures"))
        return;
    if (!HasTouchLayer())
        return;
    ImGuiIO& io = ImGui::GetIO();
    const float em = ImGui::GetFontSize();
    ImGui::TextWrapped("The gestures of a phone: a finger drags the content (with inertia, and a bounce at the end), "
                       "a long press is a right click, two fingers pinch the font.");
    if (ImGui::TreeNode("The setup code"))
    {
        ImGui::TextUnformatted(TOUCH_SETUP_CODE);
        ImGui::TreePop();
    }

    ImGuiTouchConfig& config = ImGuiTouch::GetConfig();
    if (ImGui::BeginTable("##touch_settings", 2, ImGuiTableFlags_SizingFixedFit))
    {
        if (!(io.ConfigFlags & ImGuiConfigFlags_IsTouchScreen))
        {
            SettingRow("On a desktop");
            ImGui::Checkbox("Simulate touch", &config.SimulateTouch);
            HelpMarker("The mouse plays a finger (ImGuiTouchConfig::SimulateTouch): every gesture works in the zone "
                       "below, as on a phone.\n\n"
                       "- Drag up or down to scroll the content. Start moving as soon as you press, as with a "
                       "finger.\n"
                       "- Press and hold, then release: a long press, which opens the zone's context menu (a right "
                       "click).");
        }
        SettingRow("Swipe");
        const char* scroll_modes[] = { "Touch", "Always", "Off" };  // the order of ImGuiTouchScrollMode_
        ImGui::SetNextItemWidth(8.0f * em);
        ImGui::Combo("##scroll_mode", &config.ScrollMode, scroll_modes, IM_ARRAYSIZE(scroll_modes));
        HelpMarker("Touch: the swipe follows a finger only. Always: the mouse's left button drags the content too, "
                   "while the mouse stays a mouse.");
        SettingRow("Pinch");
        ImGui::Checkbox("scales the font", &config.PinchScalesFont);
        SettingRow("Hold");
        ImGui::SetNextItemWidth(8.0f * em);
        ImGui::SliderFloat("##hold", &config.HoldSeconds, 0.0f, 1.0f, "%.2f s");
        HelpMarker("A finger still for this long hands its press to the widget under it, which then gets the drag (a "
                   "slider, a text selection). At zero, the widgets get every press at once.");
        SettingRow("Long press");
        ImGui::Checkbox("is a right click", &config.LongPressIsRightClick);
        SettingRow("Its delay");
        ImGui::SetNextItemWidth(8.0f * em);
        ImGui::SliderFloat("##long_press", &config.LongPressSeconds, 0.3f, 2.0f, "%.2f s");
        SettingRow("Scroll bars");
        ImGui::Checkbox("thin", &config.ThinScrollbars);
        ImGui::EndTable();
    }

    ImGui::BeginChild("##touch_zone", ImVec2(0.0f, 16.0f * em), ImGuiChildFlags_Borders);
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
    ImGui::InputTextMultiline("##select", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, 4.0f * em),
                              ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_WordWrap);

    ZoomableCanvas("##canvas", ImVec2(16.0f * em, 8.0f * em));
    HelpMarker("A canvas that zooms and pans, as a plot or an image viewer does:\n\n"
               "- It calls ImGuiTouch::SetItemTakesDrags() right after it: a drag that starts on it pans it at once. "
               "So the zone cannot be scrolled from inside the canvas.\n"
               "- It claims the wheel while hovered: if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY)).\n"
               "- It reads two fingers' gesture with ImGuiTouch::ItemPinch(): their spread zooms it and their move "
               "pans it, both at once, as on a map. The font keeps its size.");
    if (ImGui::BeginPopupContextWindow("##zone_menu"))
    {
        ImGui::TextUnformatted("The zone's context menu: a right click, or a long press");
        ImGui::EndPopup();
    }

    // A child that scrolls sideways: a swipe inside scrolls it, not the zone. Its buttons count their taps. Its slider
    // takes the sideways drags (ImGuiTouch::SetItemTakesDrags(ImGuiTouchAxis_X)): a sideways drag on it moves it at
    // once
    ImGui::TextWrapped("A sideways swipe scrolls this row; a tap clicks a button; a sideways drag moves the slider:");
    ImGui::BeginChild("##sideways", ImVec2(0.0f, 5.0f * em), ImGuiChildFlags_Borders,
                      ImGuiWindowFlags_HorizontalScrollbar);
    static int taps[8] = {};
    // Three per screen: the row scrolls
    float card_width = ImMax(6.0f * em, ImGui::GetContentRegionAvail().x * 0.3f);
    static float row_value = 0.5f;
    ImGui::SetNextItemWidth(card_width);
    ImGui::SliderFloat("##row_slider", &row_value, 0.0f, 1.0f);
    ImGuiTouch::SetItemTakesDrags(ImGuiTouchAxis_X);
    for (int i = 0; i < IM_ARRAYSIZE(taps); i++)
    {
        ImGui::SameLine();
        char label[32];
        ImFormatString(label, IM_ARRAYSIZE(label), "Card %d\n%d taps###card%d", i + 1, taps[i], i);
        if (ImGui::Button(label, ImVec2(card_width, 3.0f * em)))
            taps[i]++;
    }
    ImGui::EndChild();
    for (int i = 1; i <= 8; i++)
        ImGui::Text("More content, to scroll (%d)", i);
    ImGui::EndChild();
    // IsAnimating(): an application that sleeps when nothing happens keeps drawing frames meanwhile
    ImGui::Text("Moving on its own: %s", ImGuiTouch::IsAnimating() ? "yes" : "no");
}

//-----------------------------------------------------------------------------
// [SECTION] Keyboard
//-----------------------------------------------------------------------------

// The phone's keyboard, in a browser: the fields of all kinds, one of them in a child that scrolls
static void DemoKeyboard()
{
    IMGUI_DEMO_MARKER("Keyboard");
    if (!ImGui::CollapsingHeader("Keyboard"))
        return;
    if (!HasTouchLayer())
        return;
    const float em = ImGui::GetFontSize();
    ImGui::TextWrapped("In a browser on a phone: tap a field, then the keyboard button that shows next to it (or the "
                       "field again). The phone's keyboard opens, and a full-screen window keeps out of it "
                       "(ImGuiTouch::GetKeyboardInsets()). The application calls ImGuiTouch::SetupEmscriptenKeyboard() "
                       "once.");
    static char line[128] = "";
    ImGui::InputTextWithHint("text", "type here", line, IM_ARRAYSIZE(line));
    static char lines[1024] = "Several lines:\nmove the caret from the keyboard's space bar.";
    ImGui::InputTextMultiline("lines", lines, IM_ARRAYSIZE(lines), ImVec2(0.0f, 5.0f * em));

    ImGui::TextWrapped("A field in a child that scrolls: it stays above the keyboard.");
    ImGui::BeginChild("##keyboard_child", ImVec2(0.0f, 10.0f * em), ImGuiChildFlags_Borders);
    for (int i = 1; i <= 12; i++)
        ImGui::Text("Content above the field (%d)", i);
    static char deep[128] = "";
    ImGui::InputTextWithHint("field", "type here", deep, IM_ARRAYSIZE(deep));
    for (int i = 1; i <= 12; i++)
        ImGui::Text("Content below the field (%d)", i);
    ImGui::EndChild();
}

//-----------------------------------------------------------------------------
// [SECTION] Demo window
//-----------------------------------------------------------------------------

void ImGuiTouch::ShowDemoWindow(bool* p_open)
{
    const float em = ImGui::GetFontSize();
    ImGui::SetNextWindowSize(ImVec2(36.0f * em, 30.0f * em), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("ImGuiTouch Demo", p_open))
    {
        ImGui::End();
        return;
    }
    ImGuiTouch::ShowDemoContents();
    ImGui::End();
}

void ImGuiTouch::ShowDemoContents()
{
    ImGui::TextWrapped("ImGuiTouch %s: a touch screen for Dear ImGui, as on a phone", IMGUI_TOUCH_VERSION);
    // The theme applies to the whole application: a check of both themes on a phone
    static int theme = -1;  // -1: the application's
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Theme");
    ImGui::SameLine();
    if (ImGui::RadioButton("Dark", &theme, 0))
        ImGui::StyleColorsDark();
    ImGui::SameLine();
    if (ImGui::RadioButton("Light", &theme, 1))
        ImGui::StyleColorsLight();
    ImGui::Separator();
    DemoGestures();
    DemoKeyboard();
}

#else

void ImGuiTouch::ShowDemoWindow(bool*) {}
void ImGuiTouch::ShowDemoContents() {}

#endif // #ifndef IMGUI_TOUCH_DISABLE_DEMO
