// imgui_kit: what its sources share (internal API)
// https://github.com/pthom/imgui_kit (MIT license)
//
// As Dear ImGui's imgui_internal.h: no guarantee of forward compatibility here. Code that includes it may need changes
// at a new version of the kit.
//
// The kit's state, ImGuiKitContext: the layout helpers' Begin/End stacks, the help markers', FitText's buffer, the
// wheel session's state, and the touch layer's (imgui_kit_touch_internal.h). Dear ImGui has no slot for an extension's
// state, so the kit stores it in the UserData of a context hook it adds (ImGui::AddContextHook, of type Shutdown), as
// the Test Engine stores its own. The hooks are per context: each Dear ImGui context has its own state, found again
// through its hooks (no global: GImGui may be thread-local). ImGui::DestroyContext() calls that hook, which frees the
// state.
// It is created at its first use: the widgets need no setup. The touch layer's part is in use once a touch setup was
// called.
#pragma once
#include "imgui_kit.h"
#include "imgui_kit_touch_internal.h"
#include "imgui_internal.h"  // ImRect, ImGuiWindow, ImGuiAxis

struct ImGuiKitContext;
namespace ImGuiKit
{
    // The current Dear ImGui context's kit state (see this file's top), created at its first use: the widgets need no
    // setup. Freed with the Dear ImGui context.
    ImGuiKitContext& GetCurrentContext();
    // True between FillRowTail() and EndFillRow(): the fill row makes the room, a widget must not wrap
    bool IsInFillRowTail();
    // The wheel session's two steps, which SetupWheelSession() calls through Dear ImGui's context hooks (the tests call
    // them directly): right before ImGui::NewFrame() (the item that claimed the wheel in the last frame, before
    // NewFrame() gives it back to a window), and right after it, before any widget
    void WheelSessionBeforeNewFrame();
    void UpdateWheelSession(bool enabled);
}

//-----------------------------------------------------------------------------
// The wheel session (imgui_kit_wheel_session.cpp)
//-----------------------------------------------------------------------------

struct ImGuiKitWheelSessionState
{
    ImGuiContext* Context = nullptr;  // the state belongs to one context: a new one starts afresh
    int FrameCount = -1;            // (a new context may reuse the address of a destroyed one: its frames restart)
    bool Active = false;
    bool StartedOnItem = false;     // the item under the mouse owned the wheel at the first event: the item's session
    ImGuiID ItemId = 0;             // that item
    ImGuiWindow* Window = nullptr;  // the window the session scrolls
    bool Scrolled = false;          // the page scrolled during the session (by ImGui, or by the session)
    float Timer = 0.0f;             // s: the session ends when it runs out; each event adds its amount's worth
                                    // (ImGui's rule: a trackpad's small momentum events extend it by little)
    ImVec2 RefMousePos;             // the mouse at the last wheel event: a move past ImGui's drag threshold ends the
                                    // session
    ImGuiID OwnerBeforeNewFrame = 0;  // the wheel's owner as the widgets of the frame OwnerFrame left it
    int OwnerFrame = -1;
};

//-----------------------------------------------------------------------------
// The kit's state, one per Dear ImGui context
//-----------------------------------------------------------------------------

#define IMGUI_KIT_UNKNOWN (-1.0f)  // a layout value never measured (the helper draws hidden on that frame)

// Hides what follows until End(): invisible, and taking no input (a hidden button is not clicked, a hidden marker
// shows no tooltip); it forgets how far they reached (their overflow adds no scrollbar)
struct ImGuiKitHidden
{
    bool Active = false;
    ImVec2 CursorMaxPos;

    void Begin()
    {
        Active = true;
        CursorMaxPos = ImGui::GetCurrentWindow()->DC.CursorMaxPos;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    }
    void End()
    {
        if (!Active)
            return;
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
        ImGuiWindow* Window = ImGui::GetCurrentWindow();
        // The hidden items still take their height: keep the vertical extent, drop the horizontal one
        Window->DC.CursorMaxPos.x = CursorMaxPos.x;
        Active = false;
    }
};

struct ImGuiKitFillRow
{
    ImGuiID Id;
    float FillRight = 0.0f;  // the fill item's right edge (screen x), when the tail starts
    bool TailStarted = false;
    ImGuiKitHidden Hidden;
};

struct ImGuiKitAlignedGroup
{
    ImGuiID Id;
    ImGuiKitHidden Hidden;
};

struct ImGuiKitColumn
{
    ImGuiID Id;
    float Left;   // screen x, at BeginColumn()
    float Width;  // on the previous frame, or IMGUI_KIT_UNKNOWN
};

struct ImGuiKitLabelColumn
{
    ImGuiID Id;
    float Left;           // screen x, at BeginLabelColumn()
    float AlignX;
    float Column;         // the widest label on the previous frame, or IMGUI_KIT_UNKNOWN
    float Widest = 0.0f;  // the widest label on this frame
    ImGuiKitHidden Hidden;
};

struct ImGuiKitGroupPanel
{
    ImVec2 TopLeft;      // screen position of the panel, at BeginGroupPanel()
    ImVec2 MinSize;      // the size asked
    float LabelLeft;     // screen x of the label, and its right end
    float LabelRight;
};

struct ImGuiKitFlow
{
    ImGuiID Id;
    float RightEdge;  // screen x: the line's end, fixed at BeginFlow()
    int Index = 0;    // the rank of the next item
    ImGuiKitHidden Hidden;
};

enum ImGuiKitHelpMarkerShown { ImGuiKitHelpMarkerShown_Tooltip, ImGuiKitHelpMarkerShown_Popup };
struct ImGuiKitHelpMarkerOpen
{
    ImGuiKitHelpMarkerShown Shown;
    ImGuiStorage* Storage;  // the marker's window: it keeps whether the tooltip's text was cut
    ImGuiID Cut_key;
};

// A result of FitText(): the cut text, under the key of (text, wrap width, max lines, font size)
struct ImGuiKitFitTextEntry
{
    ImGuiID Key = 0;
    ImVector<char> Text;
};

struct ImGuiKitContext
{
    ImVector<ImGuiKitFillRow> FillRows;
    ImVector<ImGuiKitAlignedGroup> AlignedGroups;
    ImVector<ImGuiKitColumn> Columns;
    ImVector<ImGuiKitLabelColumn> LabelColumns;
    ImVector<ImGuiKitGroupPanel> GroupPanels;
    ImVector<ImGuiKitFlow> Flows;
    ImVector<ImGuiKitHelpMarkerOpen> HelpMarkersShown;  // what each open BeginHelpMarker() shows, for EndHelpMarker()
    ImGuiKitFitTextEntry FitTextCache[8];               // FitText()'s last results (a text that does not fit is cut
    int FitTextNext = 0;                                // once, not at each frame); the next entry to reuse
    bool WheelSessionEnabled = true;                    // SetupWheelSession(enabled)
    ImGuiKitWheelSessionState WheelSession;
    int AnimationFrame = -1;                            // the last frame a widget asked to animate (RequestAnimation)
    ImGuiKit::HelpMarkerTextRenderer HelpMarkerRenderer = nullptr;  // SetHelpMarkerTextRenderer(); null: plain text
    bool TouchSetUp = false;                            // an input setup was called: Touch is in use
    ImGuiTouchContext Touch;
};
