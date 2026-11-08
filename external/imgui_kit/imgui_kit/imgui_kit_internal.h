// imgui_kit: what its sources share (internal API)
// https://github.com/pthom/imgui_kit (MIT license)
//
// As Dear ImGui's imgui_internal.h: no guarantee of forward compatibility here. Code that includes it may need changes
// at a new version of the kit.
//
// The kit's state, ImGuiKitContext: the layout helpers' Begin/End stacks, the help markers', FitText's buffer, the
// wheel session's state, and the touch layer's (imgui_kit_touch_internal.h). Dear ImGui has no slot for an extension's state, so the kit stores
// it in the UserData of a context hook it adds (ImGui::AddContextHook, of type Shutdown), as the Test Engine stores its
// own. The hooks are per context: each Dear ImGui context has its own state, found again through its hooks (no global:
// GImGui may be thread-local). ImGui::DestroyContext() calls that hook, which frees the state.
// It is created at its first use: the widgets need no setup. The touch layer's part is in use once a touch setup was called.
#pragma once
#include "imgui_kit.h"
#include "imgui_kit_touch_internal.h"
#include "imgui_internal.h"  // ImRect, ImGuiWindow, ImGuiAxis

namespace ImGuiKit
{
    // True between FillRowTail() and EndFillRow(): the fill row makes the room, a widget must not wrap
    bool IsInFillRowTail();
}

struct ImGuiKitContext;
namespace ImGuiKit
{
    // The current Dear ImGui context's kit state (see this file's top), created at its first use: the widgets need no
    // setup. Freed with the Dear ImGui context.
    ImGuiKitContext& GetCurrentContext();
}

//-----------------------------------------------------------------------------
// The wheel session (imgui_kit_wheel_session.cpp)
//-----------------------------------------------------------------------------

struct ImGuiKitWheelSessionState
{
    ImGuiContext* context = nullptr;  // the state belongs to one context: a new one starts afresh
    int frameCount = -1;              // (a new context may reuse the address of a destroyed one: its frames restart)
    bool active = false;
    bool startedOnItem = false;     // the item under the mouse owned the wheel at the first event: the item's session
    ImGuiID itemId = 0;             // that item
    ImGuiWindow* window = nullptr;  // the window the session scrolls
    bool scrolled = false;          // the page scrolled during the session (by ImGui, or by the session)
    float timer = 0.f;              // s: the session ends when it runs out; each event adds its amount's worth (ImGui's rule:
                                    // a trackpad's small momentum events extend it by little)
    ImVec2 refMousePos;             // the mouse at the last wheel event: a move past ImGui's drag threshold ends the session
    ImGuiID ownerBeforeNewFrame = 0;  // the wheel's owner as the widgets of the frame ownerFrame left it
    int ownerFrame = -1;
};

namespace ImGuiKit
{
    // The wheel session's two steps, which SetupWheelSession() calls through Dear ImGui's context hooks (the tests call
    // them directly): right before ImGui::NewFrame() (the item that claimed the wheel in the last frame, before
    // NewFrame() gives it back to a window), and right after it, before any widget
    void WheelSessionBeforeNewFrame();
    void UpdateWheelSession(bool enabled);
}

//-----------------------------------------------------------------------------
// The kit's state, one per Dear ImGui context
//-----------------------------------------------------------------------------

constexpr float ImGuiKitUnknown = -1.0f;  // a layout value never measured (the helper draws hidden on that frame)

// Hides what follows until EndHidden(), and forgets how far it reached (its overflow adds no scrollbar)
struct ImGuiKitHidden
{
    bool active = false;
    ImVec2 cursorMaxPos;

    void Begin()
    {
        active = true;
        cursorMaxPos = ImGui::GetCurrentWindow()->DC.CursorMaxPos;
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0f);
    }
    void End()
    {
        if (!active)
            return;
        ImGui::PopStyleVar();
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        // The hidden items still take their height: keep the vertical extent, drop the horizontal one
        window->DC.CursorMaxPos.x = cursorMaxPos.x;
        active = false;
    }
};

struct ImGuiKitFillRow
{
    ImGuiID id;
    float fillRight = 0.0f;  // the fill item's right edge (screen x), when the tail starts
    bool tailStarted = false;
    ImGuiKitHidden hidden;
};

struct ImGuiKitAlignedGroup
{
    ImGuiID id;
    ImGuiKitHidden hidden;
};

struct ImGuiKitColumn
{
    ImGuiID id;
    float left;   // screen x, at BeginColumn()
    float width;  // on the previous frame, or ImGuiKitUnknown
};

struct ImGuiKitLabelColumn
{
    ImGuiID id;
    float left;           // screen x, at BeginLabelColumn()
    float alignX;
    float column;         // the widest label on the previous frame, or ImGuiKitUnknown
    float widest = 0.0f;  // the widest label on this frame
    ImGuiKitHidden hidden;
};

struct ImGuiKitGroupPanel
{
    ImVec2 topLeft;      // screen position of the panel, at BeginGroupPanel()
    ImVec2 minSize;      // the size asked
    float labelLeft;     // screen x of the label, and its right end
    float labelRight;
};

struct ImGuiKitFlow
{
    ImGuiID id;
    float rightEdge;  // screen x: the line's end, fixed at BeginFlow()
    int index = 0;    // the rank of the next item
    ImGuiKitHidden hidden;
};

enum ImGuiKitHelpMarkerShown { ImGuiKitHelpMarkerShown_Tooltip, ImGuiKitHelpMarkerShown_Popup };
struct ImGuiKitHelpMarkerOpen
{
    ImGuiKitHelpMarkerShown shown;
    ImGuiStorage* storage;  // the marker's window: it keeps whether the tooltip's text was cut
    ImGuiID cut_key;
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
    ImVector<char> FitTextBuffer;                       // FitText()'s result, when the text was cut
    bool WheelSessionEnabled = true;                    // SetupWheelSession(enabled)
    ImGuiKitWheelSessionState WheelSession;
    bool TouchSetUp = false;                            // an input setup was called: Touch is in use
    ImGuiTouchContext Touch;
};
