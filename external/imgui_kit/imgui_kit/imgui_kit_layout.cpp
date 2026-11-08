// imgui_kit: small, standalone additions to Dear ImGui
// https://github.com/pthom/imgui_kit (MIT license)

/*

Index of this file:
// [SECTION] Helpers
// [SECTION] Fill row
// [SECTION] Aligned group
// [SECTION] Column
// [SECTION] Label column
// [SECTION] Group panel
// [SECTION] Flow

*/

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_kit.h"
#include "imgui_kit_internal.h"
#include "imgui_internal.h"

namespace ImGuiKit
{
namespace
{

using Hidden = ImGuiKitHidden;
using FillRow = ImGuiKitFillRow;
using AlignedGroup = ImGuiKitAlignedGroup;
using Column = ImGuiKitColumn;
using LabelColumn = ImGuiKitLabelColumn;
using GroupPanel = ImGuiKitGroupPanel;
using Flow = ImGuiKitFlow;

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------
// The layout helpers keep what they measured in the window's state storage, under the ID of their str_id.
// A value never measured reads as kUnknown: the helper then draws hidden on that frame (Alpha = 0) and keeps its
// overflow out of the window's content size, so that a first frame neither flashes nor adds a scrollbar.
// Their Begin/End stacks are in ImGuiKitContext, one per Dear ImGui context (imgui_kit_internal.h).

constexpr float kUnknown = ImGuiKitUnknown;

float StoredFloat(ImGuiID id)
{
    return ImGui::GetStateStorage()->GetFloat(id, kUnknown);
}

void StoreFloat(ImGuiID id, float v)
{
    ImGui::GetStateStorage()->SetFloat(id, v);
}


//-----------------------------------------------------------------------------
// [SECTION] Fill row
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// [SECTION] Aligned group
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// [SECTION] Column
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// [SECTION] Label column
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// [SECTION] Group panel
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// [SECTION] Flow
//-----------------------------------------------------------------------------


ImGuiID FlowItemKey(const Flow& flow, int index)
{
    return ImHashData(&index, sizeof(index), flow.id);
}

} // namespace

bool IsInFillRowTail()
{
    return !GetCurrentContext().FillRows.empty() && GetCurrentContext().FillRows.back().tailStarted;
}

//-----------------------------------------------------------------------------
// [SECTION] Fill row
//-----------------------------------------------------------------------------

float BeginFillRow(const char* str_id)
{
    FillRow row;
    row.id = ImGui::GetID(str_id);
    float tail = StoredFloat(row.id);
    float width = ImMax(1.0f, ImGui::GetContentRegionAvail().x - (tail == kUnknown ? 0.0f : tail));
    ImGui::SetNextItemWidth(width);
    GetCurrentContext().FillRows.push_back(row);
    return width;
}

void FillRowTail()
{
    IM_ASSERT(!GetCurrentContext().FillRows.empty() && "FillRowTail() needs BeginFillRow()");
    FillRow& row = GetCurrentContext().FillRows.back();
    IM_ASSERT(!row.tailStarted && "FillRowTail() called twice");
    row.fillRight = ImGui::GetItemRectMax().x;
    row.tailStarted = true;
    if (StoredFloat(row.id) == kUnknown)
        row.hidden.Begin();
    ImGui::SameLine();
}

void EndFillRow()
{
    IM_ASSERT(!GetCurrentContext().FillRows.empty() && "EndFillRow() without BeginFillRow()");
    FillRow& row = GetCurrentContext().FillRows.back();
    // The tail's width: from the fill item's right edge to the tail's last item, the spacing included
    float tail = row.tailStarted ? ImMax(0.0f, ImGui::GetItemRectMax().x - row.fillRight) : 0.0f;
    StoreFloat(row.id, tail);
    row.hidden.End();
    GetCurrentContext().FillRows.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Aligned group
//-----------------------------------------------------------------------------

void BeginAlignedGroup(const char* str_id, float align_x)
{
    AlignedGroup group;
    group.id = ImGui::GetID(str_id);
    float width = StoredFloat(group.id);
    // The room to align in: up to the innermost column's right edge, else up to the line's end
    float room = ImGui::GetContentRegionAvail().x;
    if (!GetCurrentContext().Columns.empty())
    {
        const Column& column = GetCurrentContext().Columns.back();
        room = (column.width == kUnknown) ? kUnknown : column.left + column.width - ImGui::GetCursorScreenPos().x;
    }
    if (width == kUnknown || room == kUnknown)
        group.hidden.Begin();
    else
    {
        float offset = (room - width) * align_x;
        if (offset > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
    GetCurrentContext().AlignedGroups.push_back(group);
    ImGui::BeginGroup();
}

void EndAlignedGroup()
{
    IM_ASSERT(!GetCurrentContext().AlignedGroups.empty() && "EndAlignedGroup() without BeginAlignedGroup()");
    ImGui::EndGroup();
    AlignedGroup& group = GetCurrentContext().AlignedGroups.back();
    StoreFloat(group.id, ImGui::GetItemRectSize().x);
    group.hidden.End();
    GetCurrentContext().AlignedGroups.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Column
//-----------------------------------------------------------------------------

void BeginColumn(const char* str_id)
{
    Column column;
    column.id = ImGui::GetID(str_id);
    column.left = ImGui::GetCursorScreenPos().x;
    column.width = StoredFloat(column.id);
    GetCurrentContext().Columns.push_back(column);
    ImGui::BeginGroup();
}

void EndColumn()
{
    IM_ASSERT(!GetCurrentContext().Columns.empty() && "EndColumn() without BeginColumn()");
    ImGui::EndGroup();
    StoreFloat(GetCurrentContext().Columns.back().id, ImGui::GetItemRectSize().x);
    GetCurrentContext().Columns.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Label column
//-----------------------------------------------------------------------------

void BeginLabelColumn(const char* str_id, float labels_align_x)
{
    LabelColumn lc;
    lc.id = ImGui::GetID(str_id);
    lc.left = ImGui::GetCursorScreenPos().x;
    lc.alignX = labels_align_x;
    lc.column = StoredFloat(lc.id);
    GetCurrentContext().LabelColumns.push_back(lc);
    if (lc.column == kUnknown)
        GetCurrentContext().LabelColumns.back().hidden.Begin();
}

void Label(const char* text)
{
    IM_ASSERT(!GetCurrentContext().LabelColumns.empty() && "Label() needs BeginLabelColumn()");
    LabelColumn& lc = GetCurrentContext().LabelColumns.back();
    const char* text_end = ImGui::FindRenderedTextEnd(text);
    float width = ImGui::CalcTextSize(text, text_end).x;
    lc.widest = ImMax(lc.widest, width);
    float column = (lc.column == kUnknown) ? width : lc.column;

    ImGui::AlignTextToFramePadding();
    float y = ImGui::GetCursorScreenPos().y;
    ImGui::SetCursorScreenPos(ImVec2(lc.left + (column - width) * lc.alignX, y));
    ImGui::TextUnformatted(text, text_end);
    ImGui::SameLine();
    ImGui::SetCursorScreenPos(ImVec2(lc.left + column + ImGui::GetStyle().ItemSpacing.x, ImGui::GetCursorScreenPos().y));
}

void EndLabelColumn()
{
    IM_ASSERT(!GetCurrentContext().LabelColumns.empty() && "EndLabelColumn() without BeginLabelColumn()");
    LabelColumn& lc = GetCurrentContext().LabelColumns.back();
    StoreFloat(lc.id, lc.widest);
    lc.hidden.End();
    GetCurrentContext().LabelColumns.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Group panel
//-----------------------------------------------------------------------------

// The frame is drawn at the end, around the content's group: no clip rects or item spacing to push during the content
void BeginGroupPanel(const char* label, const ImVec2& size)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    GroupPanel panel;
    panel.topLeft = ImGui::GetCursorScreenPos();
    panel.minSize = size;
    ImGui::BeginGroup();  // the whole panel

    // The label, in the frame's top border
    const char* label_end = ImGui::FindRenderedTextEnd(label);
    panel.labelLeft = panel.topLeft.x + style.FramePadding.x * 2.0f;
    panel.labelRight = panel.labelLeft + ImGui::CalcTextSize(label, label_end).x;
    ImGui::SetCursorScreenPos(ImVec2(panel.labelLeft, panel.topLeft.y));
    ImGui::TextUnformatted(label, label_end);

    // The content, inside the frame's padding
    ImGui::SetCursorScreenPos(ImVec2(panel.topLeft.x + style.FramePadding.x * 2.0f,
                                     panel.topLeft.y + ImGui::GetTextLineHeight() + style.FramePadding.y));
    ImGui::BeginGroup();
    GetCurrentContext().GroupPanels.push_back(panel);
}

void EndGroupPanel()
{
    IM_ASSERT(!GetCurrentContext().GroupPanels.empty() && "EndGroupPanel() without BeginGroupPanel()");
    const ImGuiStyle& style = ImGui::GetStyle();
    GroupPanel panel = GetCurrentContext().GroupPanels.back();
    GetCurrentContext().GroupPanels.pop_back();
    ImGui::EndGroup();  // the content
    ImVec2 content_max = ImGui::GetItemRectMax();

    float right = ImMax(content_max.x + style.FramePadding.x * 2.0f, panel.labelRight + style.FramePadding.x * 2.0f);
    right = ImMax(right, panel.topLeft.x + panel.minSize.x);
    float bottom = ImMax(content_max.y + style.FramePadding.y * 2.0f, panel.topLeft.y + panel.minSize.y);
    ImGui::SetCursorScreenPos(panel.topLeft);
    ImGui::Dummy(ImVec2(right - panel.topLeft.x, bottom - panel.topLeft.y));  // the panel's whole size
    ImGui::EndGroup();

    // The frame, from the middle of the label's line, with a gap for the label: drawn three times, clipped
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImRect frame(ImVec2(panel.topLeft.x, panel.topLeft.y + ImGui::GetTextLineHeight() * 0.5f), ImVec2(right, bottom));
    float gap = style.ItemInnerSpacing.x;
    ImRect clips[3] = {
        ImRect(ImVec2(frame.Min.x - 1.0f, frame.Min.y - 1.0f), ImVec2(panel.labelLeft - gap, frame.Max.y + 1.0f)),
        ImRect(ImVec2(panel.labelRight + gap, frame.Min.y - 1.0f), ImVec2(frame.Max.x + 1.0f, frame.Max.y + 1.0f)),
        ImRect(ImVec2(panel.labelLeft - gap, frame.Min.y + 1.0f), ImVec2(panel.labelRight + gap, frame.Max.y + 1.0f)),
    };
    for (const ImRect& clip : clips)
    {
        draw_list->PushClipRect(clip.Min, clip.Max, true);
        draw_list->AddRect(frame.Min, frame.Max, ImGui::GetColorU32(ImGuiCol_Border), style.FrameRounding);
        draw_list->PopClipRect();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Flow
//-----------------------------------------------------------------------------

void BeginFlow(const char* str_id)
{
    Flow flow;
    flow.id = ImGui::GetID(str_id);
    flow.rightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    if (StoredFloat(flow.id) == kUnknown)  // never drawn: no widths yet
        flow.hidden.Begin();
    GetCurrentContext().Flows.push_back(flow);
}

void FlowNext()
{
    IM_ASSERT(!GetCurrentContext().Flows.empty() && "FlowNext() needs BeginFlow()");
    Flow& flow = GetCurrentContext().Flows.back();
    if (flow.index > 0)
    {
        // The previous item is drawn: keep its width, and decide where the next one goes
        StoreFloat(FlowItemKey(flow, flow.index - 1), ImGui::GetItemRectSize().x);
        float nextWidth = StoredFloat(FlowItemKey(flow, flow.index));
        if (nextWidth == kUnknown)
            nextWidth = 0.0f;
        float nextLeft = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
        if (nextLeft + nextWidth <= flow.rightEdge)
            ImGui::SameLine();
    }
    ++flow.index;
}

void EndFlow()
{
    IM_ASSERT(!GetCurrentContext().Flows.empty() && "EndFlow() without BeginFlow()");
    Flow& flow = GetCurrentContext().Flows.back();
    if (flow.index > 0)
        StoreFloat(FlowItemKey(flow, flow.index - 1), ImGui::GetItemRectSize().x);
    StoreFloat(flow.id, (float)flow.index);  // the flow was drawn (its items' count)
    flow.hidden.End();
    GetCurrentContext().Flows.pop_back();
}

} // namespace ImGuiKit
