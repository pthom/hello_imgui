// imgui_kit: small, standalone additions to Dear ImGui
// https://github.com/pthom/imgui_kit (MIT license)

/*

Index of this file:
// [SECTION] Helpers
// [SECTION] Help marker
// [SECTION] Icon button
// [SECTION] Segmented switch
// [SECTION] Fit text
// [SECTION] Resize handle

*/

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_kit.h"
#include "imgui_kit_internal.h"
#include "imgui_internal.h"
#include <string.h>  // memcpy, strlen

namespace ImGuiKit
{

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------

// Goes to the next line when an item of this width does not fit after the previous one on the same line
// (not in a fill row's tail: the row makes the room)
static void NewLineIfNoRoom(float width)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->DC.IsSameLine && ImGui::GetContentRegionAvail().x < width && !IsInFillRowTail())
        ImGui::NewLine();
}

//-----------------------------------------------------------------------------
// [SECTION] Help marker
//-----------------------------------------------------------------------------

static void DefaultHelpMarkerTextRenderer(const char* text)
{
    ImGui::TextUnformatted(text);   // wrapped by the PushTextWrapPos() of BeginHelpMarker()
}

// As high as the line it is on: a text's height on a line of text, a frame's after a framed widget
static ImVec2 HelpMarkerSize()
{
    float base_offset = ImGui::GetCurrentWindow()->DC.CurrLineTextBaseOffset;
    return ImVec2(ImTrunc(ImGui::GetFontSize() * 1.2f), ImGui::GetTextLineHeight() + 2.0f * base_offset);
}

// The (i): a circle, a dot, a bar
static void DrawHelpIcon(ImDrawList* draw_list, const ImRect& bb, ImU32 color)
{
    ImVec2 c = bb.GetCenter();
    float r = ImGui::GetFontSize() * 0.45f;
    float thickness = ImMax(1.0f, ImGui::GetFontSize() * 0.08f);
    draw_list->AddCircle(c, r, color, 0, thickness);
    draw_list->AddCircleFilled(ImVec2(c.x, c.y - r * 0.45f), r * 0.13f, color);
    float half_w = r * 0.11f;
    draw_list->AddRectFilled(ImVec2(c.x - half_w, c.y - r * 0.12f), ImVec2(c.x + half_w, c.y + r * 0.55f), color,
                             half_w);
}

//-----------------------------------------------------------------------------
// [SECTION] Icon button
//-----------------------------------------------------------------------------

static void DrawMagnifier(ImDrawList* draw_list, ImVec2 center, float size, ImU32 color, ImGuiKitIcon icon)
{
    // The glass up and right, its handle down and left: the whole icon is centered
    float r = size * 0.24f;
    ImVec2 glass(center.x + size * 0.06f, center.y - size * 0.06f);
    draw_list->AddCircle(glass, r, color, 0, size * 0.06f);
    const float d = 0.7071f;  // the handle points down-left, at 45 degrees
    draw_list->AddLine(ImVec2(glass.x - r * 1.05f * d, glass.y + r * 1.05f * d),
                       ImVec2(glass.x - r * 1.85f * d, glass.y + r * 1.85f * d), color, size * 0.1f);
    float half = r * 0.5f, t = size * 0.05f;
    if (icon == ImGuiKitIcon_ZoomIn || icon == ImGuiKitIcon_ZoomOut)
        draw_list->AddRectFilled(ImVec2(glass.x - half, glass.y - t * 0.5f), ImVec2(glass.x + half, glass.y + t * 0.5f),
                                 color);
    if (icon == ImGuiKitIcon_ZoomIn)
        draw_list->AddRectFilled(ImVec2(glass.x - t * 0.5f, glass.y - half), ImVec2(glass.x + t * 0.5f, glass.y + half),
                                 color);
    if (icon == ImGuiKitIcon_ZoomOneToOne)
    {
        // A "1": a stroke and its serif
        float h = r * 0.45f;
        draw_list->AddRectFilled(ImVec2(glass.x - t * 0.5f, glass.y - h), ImVec2(glass.x + t * 0.5f, glass.y + h),
                                 color);
        draw_list->AddLine(ImVec2(glass.x, glass.y - h), ImVec2(glass.x - r * 0.18f, glass.y - h + r * 0.18f), color,
                           t);
    }
}

static void DrawFit(ImDrawList* draw_list, ImVec2 center, float size, ImU32 color)
{
    // Four corner brackets
    float half = size * 0.34f, arm = size * 0.22f, t = size * 0.07f;
    for (int iy = -1; iy <= 1; iy += 2)
        for (int ix = -1; ix <= 1; ix += 2)
        {
            ImVec2 corner(center.x + ix * half, center.y + iy * half);
            draw_list->AddLine(corner, ImVec2(corner.x - ix * arm, corner.y), color, t);
            draw_list->AddLine(corner, ImVec2(corner.x, corner.y - iy * arm), color, t);
        }
}

static void DrawIcon(ImDrawList* draw_list, const ImRect& bb, ImGuiKitIcon icon, ImU32 color)
{
    ImVec2 c = bb.GetCenter();
    float size = ImMin(bb.GetWidth(), bb.GetHeight());
    float half = ImGui::GetFontSize() * 0.35f, t = ImMax(1.0f, ImGui::GetFontSize() * 0.09f);
    switch (icon)
    {
    case ImGuiKitIcon_Minus:
        draw_list->AddRectFilled(ImVec2(c.x - half, c.y - t), ImVec2(c.x + half, c.y + t), color);
        break;
    case ImGuiKitIcon_Plus:
        draw_list->AddRectFilled(ImVec2(c.x - half, c.y - t), ImVec2(c.x + half, c.y + t), color);
        draw_list->AddRectFilled(ImVec2(c.x - t, c.y - half), ImVec2(c.x + t, c.y + half), color);
        break;
    case ImGuiKitIcon_ZoomIn:
    case ImGuiKitIcon_ZoomOut:
    case ImGuiKitIcon_ZoomOneToOne:
        DrawMagnifier(draw_list, c, size, color, icon);
        break;
    case ImGuiKitIcon_ZoomFit:
        DrawFit(draw_list, c, size, color);
        break;
    default:
        IM_ASSERT(0); // an unknown ImGuiKitIcon
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Segmented switch
//-----------------------------------------------------------------------------

// A segment's fill, at a position in segments (between two while it slides); round only at the switch's ends
static void DrawSwitchSegment(ImDrawList* draw_list, const ImRect& bb, int labels_count, float rounding, ImVec4 col,
                              float pos)
{
    col.w = 1.0f;
    ImDrawFlags flags = ImDrawFlags_RoundNone;
    bool at_left = pos < 0.01f, at_right = pos > (float)(labels_count - 1) - 0.01f;
    if (at_left && at_right)
        flags = ImDrawFlags_RoundAll;
    else if (at_left)
        flags = ImDrawFlags_RoundLeft;
    else if (at_right)
        flags = ImDrawFlags_RoundRight;
    float segment_width = bb.GetWidth() / (float)labels_count;
    float x0 = bb.Min.x + segment_width * pos;
    draw_list->AddRectFilled(ImVec2(x0, bb.Min.y), ImVec2(x0 + segment_width, bb.Max.y), ImGui::GetColorU32(col),
                             rounding, flags);
}

//-----------------------------------------------------------------------------
// [SECTION] Help marker
//-----------------------------------------------------------------------------

constexpr float HELP_WIDTH_EM = 35.0f;      // the default width of a help's text, as Dear ImGui's demo wraps it
constexpr float HELP_MAX_HEIGHT = 0.6f;     // a help's window, at most this fraction of the screen's height

// The help window's size, before it begins. Its width: the asked one, else the text's on one line up to 35 em (the
// text is measured here, only when the window opens), else none (the content sizes it); never wider than the screen.
// Its height: at most 60% of the screen (a popup then scrolls, a tooltip shows its beginning). A content that wraps
// to its window's width (Markdown) needs a width: an auto-sized window offers it none. Returns the width set, or 0.
static float SetNextHelpWindowSize(float width, const char* text)
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float em = ImGui::GetFontSize();
    float w = 0.0f;
    if (width > 0.0f)
        w = width;
    else if (text != nullptr)
        w = ImMin(ImGui::CalcTextSize(text).x + ImGui::GetStyle().WindowPadding.x * 2.0f, em * HELP_WIDTH_EM);
    if (w > 0.0f)
        w = ImMin(w, viewport->WorkSize.x - em * 2.0f);
    ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0.0f),
                                        ImVec2(w > 0.0f ? w : FLT_MAX, viewport->WorkSize.y * HELP_MAX_HEIGHT));
    return w;
}

// The text wraps at the window's edge when its width is set, else at 35 em
static void PushHelpTextWrapPos(float window_width)
{
    ImGui::PushTextWrapPos(window_width > 0.0f ? 0.0f : ImGui::GetFontSize() * HELP_WIDTH_EM);
}

// text: HelpMarker()'s, which sizes the window when it opens; null for BeginHelpMarker()
static bool BeginHelpMarkerEx(const char* str_id, float width, const char* text)
{
    ImGuiKitContext& kit = GetCurrentContext();
    ImVec2 size = HelpMarkerSize();
    NewLineIfNoRoom(size.x);
    ImGui::PushID(str_id);
    bool clicked = ImGui::InvisibleButton("##help_marker", size);
    bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip);
    bool popup_open = ImGui::IsPopupOpen("##help_popup");
    ImU32 color = ImGui::GetColorU32((ImGui::IsItemHovered() || popup_open) ? ImGuiCol_Text : ImGuiCol_TextDisabled);
    DrawHelpIcon(ImGui::GetWindowDrawList(), ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), color);

    if (clicked)
        ImGui::OpenPopup("##help_popup");
    if (ImGui::IsPopupOpen("##help_popup"))
    {
        float window_width = SetNextHelpWindowSize(width, text);
        if (ImGui::BeginPopup("##help_popup"))
        {
            ImGuiKitHelpMarkerOpen open = { ImGuiKitHelpMarkerShown_Popup, nullptr, 0 };
            kit.HelpMarkersShown.push_back(open);
            PushHelpTextWrapPos(window_width);
            return true;
        }
    }
    // A finger has no hover: on a touch screen, only the tap shows the text
    bool touch = ImGui::GetIO().MouseSource == ImGuiMouseSource_TouchScreen;
    if (hovered && !touch)
    {
        // A tooltip cannot scroll (the mouse cannot reach it): no scrollbar, and when its text was cut on the
        // previous frame, its first line says how to read the rest
        ImGuiKitHelpMarkerOpen open{ImGuiKitHelpMarkerShown_Tooltip, ImGui::GetStateStorage(),
                            ImGui::GetID("##help_tooltip_cut")};
        bool was_cut = open.Storage->GetBool(open.Cut_key, false);
        float window_width = SetNextHelpWindowSize(width, text);
        if (ImGui::BeginTooltipEx(ImGuiTooltipFlags_None, ImGuiWindowFlags_NoScrollbar))
        {
            kit.HelpMarkersShown.push_back(open);
            if (was_cut)
            {
                ImGui::TextDisabled("Click the (i) to read all");
                ImGui::Separator();
            }
            PushHelpTextWrapPos(window_width);
            return true;
        }
    }
    ImGui::PopID();
    return false;
}

bool BeginHelpMarker(const char* str_id, float width_em)
{
    return BeginHelpMarkerEx(str_id, width_em * ImGui::GetFontSize(), nullptr);
}

void EndHelpMarker()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.HelpMarkersShown.empty()
              && "EndHelpMarker() without a BeginHelpMarker() that returned true");
    ImGui::PopTextWrapPos();
    const ImGuiKitHelpMarkerOpen& open = kit.HelpMarkersShown.back();
    if (open.Shown == ImGuiKitHelpMarkerShown_Popup)
        ImGui::EndPopup();
    else
    {
        open.Storage->SetBool(open.Cut_key, ImGui::GetCurrentWindow()->ScrollMax.y > 0.0f);
        ImGui::EndTooltip();
    }
    kit.HelpMarkersShown.pop_back();
    ImGui::PopID();
}

void HelpMarker(const char* text, float width_em)
{
    if (BeginHelpMarkerEx(text, width_em * ImGui::GetFontSize(), text))
    {
        HelpMarkerTextRenderer renderer = GetCurrentContext().HelpMarkerRenderer;
        (renderer ? renderer : DefaultHelpMarkerTextRenderer)(text);
        EndHelpMarker();
    }
}

float HelpMarkerWidth()
{
    return HelpMarkerSize().x + ImGui::GetStyle().ItemSpacing.x;
}

void SetHelpMarkerTextRenderer(HelpMarkerTextRenderer renderer)
{
    GetCurrentContext().HelpMarkerRenderer = renderer;
}

//-----------------------------------------------------------------------------
// [SECTION] Icon button
//-----------------------------------------------------------------------------

bool IconButton(const char* str_id, ImGuiKitIcon icon, const ImVec2& size)
{
    float h = ImGui::GetFrameHeight();
    ImVec2 button_size(size.x > 0.0f ? size.x : h, size.y > 0.0f ? size.y : h);
    ImGui::PushID(str_id);
    bool pressed = ImGui::Button("##icon_button", button_size);
    ImGui::PopID();
    DrawIcon(ImGui::GetWindowDrawList(), ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()), icon,
             ImGui::GetColorU32(ImGuiCol_Text));
    return pressed;
}

//-----------------------------------------------------------------------------
// [SECTION] Segmented switch
//-----------------------------------------------------------------------------

float SegmentedSwitchWidth(const char* const labels[], int labels_count)
{
    // Equal segments, each wide enough for the longest label
    float label_width = 0.0f;
    for (int i = 0; i < labels_count; i++)
        label_width = ImMax(label_width, ImGui::CalcTextSize(labels[i], nullptr, true).x);
    return (label_width + ImGui::GetStyle().FramePadding.x * 4.0f) * (float)labels_count;
}

bool SegmentedSwitch(const char* str_id, int* current, const char* const labels[], int labels_count)
{
    IM_ASSERT(labels_count > 0);
    const ImGuiStyle& style = ImGui::GetStyle();
    ImVec2 size(SegmentedSwitchWidth(labels, labels_count), ImGui::GetFrameHeight());
    float segment_width = size.x / (float)labels_count;
    ImVec2 p0 = ImGui::GetCursorScreenPos();

    // As a framed widget: the text that follows on its line is aligned with its labels
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    const ImGuiID id = window->GetID(str_id);
    const ImRect bb(p0, p0 + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    // The press, rather than the release: a touch screen may not report the latter where it started. A nav activation
    // (Space, Enter, a gamepad's button) makes it active, as a slider: Left and Right then move the choice, and the
    // same key leaves it.
    ImGuiContext& g = *GImGui;
    const bool hovered = ImGui::ItemHoverable(bb, id, g.LastItemData.ItemFlags);
    const bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left, ImGuiInputFlags_None, id);
    if (clicked || g.NavActivateId == id)
    {
        if (clicked)
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }
    int picked = *current;
    if (clicked)
        picked = ImClamp((int)((ImGui::GetIO().MouseClickedPos[0].x - p0.x) / segment_width), 0, labels_count - 1);
    if (g.ActiveId == id)
    {
        if (g.ActiveIdSource == ImGuiInputSource_Mouse)
        {
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                ImGui::ClearActiveID();
        }
        else
        {
            const float tweak = ImGui::GetNavTweakPressedAmount(ImGuiAxis_X);
            if (tweak != 0.0f)
                picked = ImClamp(*current + ((tweak > 0.0f) ? 1 : -1), 0, labels_count - 1);
            if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
                ImGui::ClearActiveID();
        }
    }
    const bool changed = (picked != *current);
    *current = picked;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p1 = p0 + size;
    float rounding = ImMin(style.FrameRounding, size.y * 0.5f);
    // The active segment's color is mixed from the frame and the text colors: it stands out in any theme, and its
    // label stays readable.
    ImVec4 window_bg = style.Colors[ImGuiCol_WindowBg], frame = style.Colors[ImGuiCol_FrameBg];
    ImVec4 track = ImLerp(window_bg, frame, frame.w);
    track.w = 1.0f;
    ImVec4 text = style.Colors[ImGuiCol_Text];
    ImGui::RenderNavCursor(bb, id);
    draw_list->AddRectFilled(p0, p1, ImGui::GetColorU32(ImGuiCol_FrameBg), rounding);
    if (ImGui::IsItemHovered())
    {
        int over = ImClamp((int)((ImGui::GetIO().MousePos.x - p0.x) / segment_width), 0, labels_count - 1);
        if (over != *current)
            DrawSwitchSegment(draw_list, bb, labels_count, rounding, ImLerp(track, text, 0.08f), (float)over);
    }
    // The fill slides to the current segment, in about a tenth of a second
    if (*current >= 0 && *current < labels_count)
    {
        ImGuiStorage* storage = ImGui::GetStateStorage();
        static const ImGuiID fill_pos_seed = ImHashStr("##fill_pos");
        const ImGuiID pos_id = ImHashData(&fill_pos_seed, sizeof(fill_pos_seed), id);
        float target = (float)*current;
        float pos = storage->GetFloat(pos_id, target);
        float step = ImMin(1.0f, ImGui::GetIO().DeltaTime * 18.0f);
        pos = (ImFabs(target - pos) < 0.01f) ? target : pos + (target - pos) * step;
        if (pos != target)
            ImGuiKit::RequestAnimation();  // an app that idles keeps drawing meanwhile (IsAnimating())
        storage->SetFloat(pos_id, pos);
        DrawSwitchSegment(draw_list, bb, labels_count, rounding, ImLerp(track, text, 0.22f), pos);
    }
    for (int i = 1; i < labels_count; i++)
    {
        float x = p0.x + segment_width * (float)i;
        draw_list->AddLine(ImVec2(x, p0.y), ImVec2(x, p1.y), ImGui::GetColorU32(ImGuiCol_Border));
    }
    draw_list->AddRect(p0, p1, ImGui::GetColorU32(ImGuiCol_Border), rounding);
    for (int i = 0; i < labels_count; i++)
    {
        ImVec2 text_size = ImGui::CalcTextSize(labels[i], nullptr, true);
        ImVec2 text_pos(p0.x + segment_width * (float)i + (segment_width - text_size.x) * 0.5f,
                        p0.y + (size.y - text_size.y) * 0.5f);
        draw_list->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), labels[i],
                           ImGui::FindRenderedTextEnd(labels[i]));
    }
    return changed;
}

#ifdef IMGUI_KIT_USE_STL
bool SegmentedSwitch(const char* str_id, int* current, const std::vector<std::string>& labels)
{
    const char* stack_labels[16];  // no allocation for the usual counts
    ImVector<const char*> heap_labels;
    const char** c_labels = stack_labels;
    if (labels.size() > IM_ARRAYSIZE(stack_labels))
    {
        heap_labels.resize((int)labels.size());
        c_labels = heap_labels.Data;
    }
    for (int i = 0; i < (int)labels.size(); i++)
        c_labels[i] = labels[(size_t)i].c_str();
    return SegmentedSwitch(str_id, current, c_labels, (int)labels.size());
}
#endif

//-----------------------------------------------------------------------------
// [SECTION] Fit text
//-----------------------------------------------------------------------------

// The cut is computed once per text (the key: the text, the width, the lines, the font size), and kept in a small
// cache: a list of cards with cut summaries measures nothing at the frames that follow
const char* FitText(const char* text, float wrap_width, int max_lines)
{
    const int len = (int)strlen(text);
    ImGuiKitContext& kit = GetCurrentContext();
    struct { float wrap_width; int max_lines; float font_size; } key_data;
    key_data.wrap_width = wrap_width;
    key_data.max_lines = max_lines;
    key_data.font_size = ImGui::GetFontSize();
    const ImGuiID key = ImHashData(&key_data, sizeof(key_data), ImHashData(text, (size_t)len));
    for (ImGuiKitFitTextEntry& entry : kit.FitTextCache)
        if (entry.Key == key && entry.Key != 0)
            return entry.Text.empty() ? text : entry.Text.Data;  // an empty entry: the text fits, as it is

    const float max_height = (float)max_lines * ImGui::GetTextLineHeight() + 1.0f;
    ImGuiKitFitTextEntry& entry = kit.FitTextCache[kit.FitTextNext];
    kit.FitTextNext = (kit.FitTextNext + 1) % IM_ARRAYSIZE(kit.FitTextCache);
    entry.Key = key;
    ImVector<char>& buffer = entry.Text;
    if (ImGui::CalcTextSize(text, nullptr, false, wrap_width).y <= max_height)
    {
        buffer.resize(0);  // it fits: the text itself, and the next frames skip the measure
        return text;
    }
    // The result is never longer than the text and the ellipsis: the buffer grows to that, and never truncates
    buffer.resize(len + 4);
    const char* ellipsis = "\xE2\x80\xA6";  // U+2026, 3 bytes
    // Words leave from the end, until the rest and the ellipsis fit; a word that is alone and too long (a URL, a
    // path) loses characters instead
    int end = len;
    while (true)
    {
        int word_start = end;
        while (word_start > 0 && text[word_start - 1] != ' ')  // back to the start of the last word
            word_start--;
        while (word_start > 0 && text[word_start - 1] == ' ')  // and before the spaces that precede it
            word_start--;
        if (word_start > 0)
            end = word_start;
        else
            for (end--; end > 0 && (text[end] & 0xC0) == 0x80; end--) {}  // one character: back to its lead byte
        memcpy(buffer.Data, text, (size_t)end);
        memcpy(buffer.Data + end, ellipsis, 4);  // with its terminating zero
        if (end == 0 || ImGui::CalcTextSize(buffer.Data, nullptr, false, wrap_width).y <= max_height)
            return buffer.Data;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Resize handle
//-----------------------------------------------------------------------------

// The grip lies over the item it resizes, which was submitted first and may own the hover (a plot, a text field): the
// grip takes the active state at the press itself, so that the drag goes to it.
bool ResizeHandle(const char* str_id, ImVec2* size, const ImVec2& min_size)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(str_id);
    const ImVec2 item_min = ImGui::GetItemRectMin(), item_max = ImGui::GetItemRectMax();
    const float grip = ImGui::GetFontSize();
    const ImRect grip_bb(ImVec2(item_max.x - grip, item_max.y - grip), item_max);
    ImGui::ItemAdd(grip_bb, id, nullptr, ImGuiItemFlags_NoNav);

    ImGuiStorage* storage = ImGui::GetStateStorage();
    const ImGuiID offset_x_key = ImHashStr("offset_x", 0, id), offset_y_key = ImHashStr("offset_y", 0, id);
    // The item may be a child window (a multiline text field, a child): it, not this window, is then the hovered one
    const ImGuiHoveredFlags hover_flags =
        ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_ChildWindows;
    const bool hovered = ImGui::IsWindowHovered(hover_flags) && ImGui::IsMouseHoveringRect(grip_bb.Min, grip_bb.Max);
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        ImGui::SetActiveID(id, window);
        ImGui::FocusWindow(window);
        // Where the grip was taken, from the item's corner: the corner stays at that distance from the mouse
        storage->SetFloat(offset_x_key, item_max.x - g.IO.MousePos.x);
        storage->SetFloat(offset_y_key, item_max.y - g.IO.MousePos.y);
    }

    bool changed = false;
    const bool active = (g.ActiveId == id);
    if (active)
    {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            ImVec2 corner(g.IO.MousePos.x + storage->GetFloat(offset_x_key),
                          g.IO.MousePos.y + storage->GetFloat(offset_y_key));
            ImVec2 new_size(ImMax(corner.x - item_min.x, min_size.x), ImMax(corner.y - item_min.y, min_size.y));
            if (new_size.x != size->x || new_size.y != size->y)
            {
                *size = new_size;
                changed = true;
            }
        }
        else
            ImGui::ClearActiveID();
    }
    if (hovered || active)
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);

    // A window's resize grip: its colors, a triangle in the corner
    ImGuiCol col = active ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip;
    window->DrawList->AddTriangleFilled(grip_bb.Max, ImVec2(grip_bb.Min.x, grip_bb.Max.y),
                                        ImVec2(grip_bb.Max.x, grip_bb.Min.y), ImGui::GetColorU32(col));
    return changed;
}

} // namespace ImGuiKit
