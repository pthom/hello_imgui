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

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------
// The layout helpers keep what they measured in the window's state storage, under a key made of their str_id's ID
// and a seed per helper: the bare ID would be Dear ImGui's own key for an item with that label (a tree node's open
// state), and two helpers with one str_id would share a value.
// A value never measured reads as IMGUI_KIT_UNKNOWN: the helper then draws hidden on that frame (Alpha = 0, no input)
// and keeps its overflow out of the window's content size, so that a first frame neither flashes nor adds a scrollbar.
// Their Begin/End stacks are in ImGuiKitContext, one per Dear ImGui context (imgui_kit_internal.h).

static ImGuiID StorageKey(const char* str_id, ImGuiID helper_seed)
{
    return ImHashData(&helper_seed, sizeof(helper_seed), ImGui::GetID(str_id));
}
const ImGuiID FILL_ROW_SEED = ImHashStr("##kit_fill_row");
const ImGuiID ALIGNED_GROUP_SEED = ImHashStr("##kit_aligned_group");
const ImGuiID COLUMN_SEED = ImHashStr("##kit_column");
const ImGuiID LABEL_COLUMN_SEED = ImHashStr("##kit_label_column");
const ImGuiID FLOW_SEED = ImHashStr("##kit_flow");

static float StoredFloat(ImGuiID key)
{
    return ImGui::GetStateStorage()->GetFloat(key, IMGUI_KIT_UNKNOWN);
}

static void StoreFloat(ImGuiID key, float v)
{
    ImGui::GetStateStorage()->SetFloat(key, v);
}

// A flow's item, by its rank
static ImGuiID FlowItemKey(const ImGuiKitFlow& flow, int index)
{
    return ImHashData(&index, sizeof(index), flow.Id);
}

bool IsInFillRowTail()
{
    ImGuiKitContext& kit = GetCurrentContext();
    return !kit.FillRows.empty() && kit.FillRows.back().TailStarted;
}

//-----------------------------------------------------------------------------
// [SECTION] Fill row
//-----------------------------------------------------------------------------

float BeginFillRow(const char* str_id)
{
    ImGuiKitFillRow row;
    row.Id = StorageKey(str_id, FILL_ROW_SEED);
    float tail = StoredFloat(row.Id);
    float width = ImMax(1.0f, ImGui::GetContentRegionAvail().x - (tail == IMGUI_KIT_UNKNOWN ? 0.0f : tail));
    ImGui::SetNextItemWidth(width);
    GetCurrentContext().FillRows.push_back(row);
    return width;
}

void FillRowTail()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.FillRows.empty() && "FillRowTail() needs BeginFillRow()");
    ImGuiKitFillRow& row = kit.FillRows.back();
    IM_ASSERT(!row.TailStarted && "FillRowTail() called twice");
    row.FillRight = ImGui::GetItemRectMax().x;
    row.TailStarted = true;
    if (StoredFloat(row.Id) == IMGUI_KIT_UNKNOWN)
        row.Hidden.Begin();
    ImGui::SameLine();
}

void EndFillRow()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.FillRows.empty() && "EndFillRow() without BeginFillRow()");
    ImGuiKitFillRow& row = kit.FillRows.back();
    // The tail's width: from the fill item's right edge to the tail's last item, the spacing included
    float tail = row.TailStarted ? ImMax(0.0f, ImGui::GetItemRectMax().x - row.FillRight) : 0.0f;
    StoreFloat(row.Id, tail);
    row.Hidden.End();
    kit.FillRows.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Aligned group
//-----------------------------------------------------------------------------

void BeginAlignedGroup(const char* str_id, float align_x)
{
    ImGuiKitContext& kit = GetCurrentContext();
    ImGuiKitAlignedGroup group;
    group.Id = StorageKey(str_id, ALIGNED_GROUP_SEED);
    float width = StoredFloat(group.Id);
    // The room to align in: up to the innermost column's right edge, else up to the line's end
    float room = ImGui::GetContentRegionAvail().x;
    if (!kit.Columns.empty())
    {
        const ImGuiKitColumn& column = kit.Columns.back();
        room = (column.Width == IMGUI_KIT_UNKNOWN)
                   ? IMGUI_KIT_UNKNOWN : column.Left + column.Width - ImGui::GetCursorScreenPos().x;
    }
    if (width == IMGUI_KIT_UNKNOWN || room == IMGUI_KIT_UNKNOWN)
        group.Hidden.Begin();
    else
    {
        // A whole pixel: content that snaps to whole pixels (a child window, an image) would change the measured
        // width with the offset's fraction, and the group would move at each frame
        float offset = ImTrunc((room - width) * align_x);
        if (offset > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
    }
    kit.AlignedGroups.push_back(group);
    ImGui::BeginGroup();
}

void EndAlignedGroup()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.AlignedGroups.empty() && "EndAlignedGroup() without BeginAlignedGroup()");
    ImGui::EndGroup();
    ImGuiKitAlignedGroup& group = kit.AlignedGroups.back();
    StoreFloat(group.Id, ImGui::GetItemRectSize().x);
    group.Hidden.End();
    kit.AlignedGroups.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Column
//-----------------------------------------------------------------------------

void BeginColumn(const char* str_id)
{
    ImGuiKitColumn column;
    column.Id = StorageKey(str_id, COLUMN_SEED);
    column.Left = ImGui::GetCursorScreenPos().x;
    column.Width = StoredFloat(column.Id);
    GetCurrentContext().Columns.push_back(column);
    ImGui::BeginGroup();
}

void EndColumn()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.Columns.empty() && "EndColumn() without BeginColumn()");
    ImGui::EndGroup();
    StoreFloat(kit.Columns.back().Id, ImGui::GetItemRectSize().x);
    kit.Columns.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Label column
//-----------------------------------------------------------------------------

void BeginLabelColumn(const char* str_id, float labels_align_x)
{
    ImGuiKitContext& kit = GetCurrentContext();
    ImGuiKitLabelColumn lc;
    lc.Id = StorageKey(str_id, LABEL_COLUMN_SEED);
    lc.Left = ImGui::GetCursorScreenPos().x;
    lc.AlignX = labels_align_x;
    lc.Column = StoredFloat(lc.Id);
    kit.LabelColumns.push_back(lc);
    if (lc.Column == IMGUI_KIT_UNKNOWN)
        kit.LabelColumns.back().Hidden.Begin();
}

void Label(const char* text)
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.LabelColumns.empty() && "Label() needs BeginLabelColumn()");
    ImGuiKitLabelColumn& lc = kit.LabelColumns.back();
    const char* text_end = ImGui::FindRenderedTextEnd(text);
    float width = ImGui::CalcTextSize(text, text_end).x;
    lc.Widest = ImMax(lc.Widest, width);
    // A label wider than the last frame's widest (a text that grew) takes its own width this frame
    float column = (lc.Column == IMGUI_KIT_UNKNOWN) ? width : ImMax(lc.Column, width);

    ImGui::AlignTextToFramePadding();
    float y = ImGui::GetCursorScreenPos().y;
    ImGui::SetCursorScreenPos(ImVec2(lc.Left + (column - width) * lc.AlignX, y));
    ImGui::TextUnformatted(text, text_end);
    ImGui::SameLine();
    ImGui::SetCursorScreenPos(ImVec2(lc.Left + column + ImGui::GetStyle().ItemSpacing.x,
                                     ImGui::GetCursorScreenPos().y));
}

void EndLabelColumn()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.LabelColumns.empty() && "EndLabelColumn() without BeginLabelColumn()");
    ImGuiKitLabelColumn& lc = kit.LabelColumns.back();
    StoreFloat(lc.Id, lc.Widest);
    lc.Hidden.End();
    kit.LabelColumns.pop_back();
}

//-----------------------------------------------------------------------------
// [SECTION] Group panel
//-----------------------------------------------------------------------------

// The frame is drawn at the end, around the content's group: no clip rects or item spacing to push during the content.
// The label is pushed as an ID: two panels may hold items with the same labels.
void BeginGroupPanel(const char* label, const ImVec2& size)
{
    const ImGuiStyle& style = ImGui::GetStyle();
    ImGuiKitGroupPanel panel;
    panel.TopLeft = ImGui::GetCursorScreenPos();
    panel.MinSize = size;
    ImGui::PushID(label);
    ImGui::BeginGroup();  // the whole panel

    // The label, in the frame's top border
    const char* label_end = ImGui::FindRenderedTextEnd(label);
    panel.LabelLeft = panel.TopLeft.x + style.FramePadding.x * 2.0f;
    panel.LabelRight = panel.LabelLeft + ImGui::CalcTextSize(label, label_end).x;
    ImGui::SetCursorScreenPos(ImVec2(panel.LabelLeft, panel.TopLeft.y));
    ImGui::TextUnformatted(label, label_end);

    // The content, inside the frame's padding
    ImGui::SetCursorScreenPos(ImVec2(panel.TopLeft.x + style.FramePadding.x * 2.0f,
                                     panel.TopLeft.y + ImGui::GetTextLineHeight() + style.FramePadding.y));
    ImGui::BeginGroup();
    GetCurrentContext().GroupPanels.push_back(panel);
}

void EndGroupPanel()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.GroupPanels.empty() && "EndGroupPanel() without BeginGroupPanel()");
    const ImGuiStyle& style = ImGui::GetStyle();
    ImGuiKitGroupPanel panel = kit.GroupPanels.back();
    kit.GroupPanels.pop_back();
    ImGui::EndGroup();  // the content
    ImVec2 content_max = ImGui::GetItemRectMax();

    float right = ImMax(content_max.x + style.FramePadding.x * 2.0f, panel.LabelRight + style.FramePadding.x * 2.0f);
    right = ImMax(right, panel.TopLeft.x + panel.MinSize.x);
    float bottom = ImMax(content_max.y + style.FramePadding.y * 2.0f, panel.TopLeft.y + panel.MinSize.y);
    ImGui::SetCursorScreenPos(panel.TopLeft);
    ImGui::Dummy(ImVec2(right - panel.TopLeft.x, bottom - panel.TopLeft.y));  // the panel's whole size
    ImGui::EndGroup();
    ImGui::PopID();

    // The frame, from the middle of the label's line, with a gap for the label: drawn three times, clipped
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImRect frame(ImVec2(panel.TopLeft.x, panel.TopLeft.y + ImGui::GetTextLineHeight() * 0.5f), ImVec2(right, bottom));
    float gap = style.ItemInnerSpacing.x;
    ImRect clips[3] = {
        ImRect(ImVec2(frame.Min.x - 1.0f, frame.Min.y - 1.0f), ImVec2(panel.LabelLeft - gap, frame.Max.y + 1.0f)),
        ImRect(ImVec2(panel.LabelRight + gap, frame.Min.y - 1.0f), ImVec2(frame.Max.x + 1.0f, frame.Max.y + 1.0f)),
        ImRect(ImVec2(panel.LabelLeft - gap, frame.Min.y + 1.0f), ImVec2(panel.LabelRight + gap, frame.Max.y + 1.0f)),
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
    ImGuiKitFlow flow;
    flow.Id = StorageKey(str_id, FLOW_SEED);
    flow.RightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    if (StoredFloat(flow.Id) == IMGUI_KIT_UNKNOWN)  // never drawn: no widths yet
        flow.Hidden.Begin();
    GetCurrentContext().Flows.push_back(flow);
}

void FlowNext()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.Flows.empty() && "FlowNext() needs BeginFlow()");
    ImGuiKitFlow& flow = kit.Flows.back();
    if (flow.Index > 0)
    {
        // The previous item is drawn: keep its width, and decide where the next one goes
        StoreFloat(FlowItemKey(flow, flow.Index - 1), ImGui::GetItemRectSize().x);
        // An item never measured (new since the last frame) starts a line: for one frame, rather than overflow
        float next_width = StoredFloat(FlowItemKey(flow, flow.Index));
        float next_left = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
        if (next_width != IMGUI_KIT_UNKNOWN && next_left + next_width <= flow.RightEdge)
            ImGui::SameLine();
    }
    ++flow.Index;
}

void EndFlow()
{
    ImGuiKitContext& kit = GetCurrentContext();
    IM_ASSERT(!kit.Flows.empty() && "EndFlow() without BeginFlow()");
    ImGuiKitFlow& flow = kit.Flows.back();
    if (flow.Index > 0)
        StoreFloat(FlowItemKey(flow, flow.Index - 1), ImGui::GetItemRectSize().x);
    StoreFloat(flow.Id, (float)flow.Index);  // the flow was drawn (its items' count)
    flow.Hidden.End();
    kit.Flows.pop_back();
}

} // namespace ImGuiKit
