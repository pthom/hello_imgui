// imgui_kit: what its sources share (internal API)
// https://github.com/pthom/imgui_kit (MIT license)
//
// As Dear ImGui's imgui_internal.h: no guarantee of forward compatibility here. Code that includes it may need changes
// at a new version of the kit.
//
// The touch layer's state, ImGuiTouchContext: Dear ImGui has no slot for an extension's state, so the setups
// (ImGuiTouch::Setup(), ImGuiKit::SetupWheelSession(), ImGuiTouch::SetKeyboardInsets()) allocate one for the current Dear
// ImGui context, and store it in the UserData of a context hook they add (ImGui::AddContextHook, of type Shutdown), as
// the Test Engine stores its own. The hooks are per context: each Dear ImGui context has its own touch state, found
// again through its hooks (no global: GImGui may be thread-local). ImGui::DestroyContext() calls that hook, which frees
// the state. The setups play the role of ImPlot's CreateContext(); the kit has no DestroyContext() to call. The other
// ImGuiTouch functions assert that a setup was called.
#pragma once
#include "imgui_kit.h"
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
// The touch layer's state, one per Dear ImGui context (as Dear ImGui's own, in ImGuiContext)
//-----------------------------------------------------------------------------

constexpr int ImGuiTouchFlickSamples = 16;  // the swipe's recent positions of the finger: enough for 0.05 s at 240 fps

struct ImGuiTouchScrollSample
{
    float time;
    ImVec2 pos;
};

struct ImGuiTouchScrollState
{
    ImGuiContext* context = nullptr;  // the state belongs to one context (its windows): a new one starts afresh
    int frameCount = -1;              // (a new context may reuse the address of a destroyed one: its frames restart)
    bool owning = false;            // the sentinel holds the active id: a press, not released nor handed over yet
    bool swiping = false;           // the finger moved past the slop
    bool parked = false;            // the sentinel holds the active id for a pinch: nothing until the fingers lift
    ImGuiWindow* window = nullptr;  // the pressed window, then the one that scrolls
    ImGuiAxis axis = ImGuiAxis_None;
    ImVec2 pressPos;
    ImGuiTouchScrollSample samples[ImGuiTouchFlickSamples];  // the finger's recent positions, a ring: the lift speed comes from them
    int nbSamples = 0, nextSample = 0;
    ImVec2 inertia;                 // px/s, after the release
    float pull = 0.f;               // px: how far the finger dragged past the end of the scroll (the rubber band)
    float overscroll = 0.f;         // px: the content is drawn this far past its end, along the axis (positive: toward the start)
    float overscrollSpeed = 0.f;    // px/s, while it springs back
    int replayedPresses = 0;        // presses queued by the layer, which it must not claim
    bool watchingLongPress = false; // a touch press, still so far: a long press when it stays
    bool longPressArmed = false;    // it stayed: a right click when it lifts, unless it moves (a ring shows it)
    bool thinScrollbars = false;    // this frame: thin bars on a touch screen
    ImGuiID heldBar = 0;            // the scroll bar held by a finger, since heldBarTime: it widens
    float heldBarTime = 0.f;
    double lastReplayTime = -1e9;   // the previous replayed press: the next one pairs with it (a double tap)
    double rippleTime = -1e9;       // when the hold handed the press over: a ring around the finger, briefly
    ImVec2 ripplePos;
    ImVec2 lastReplayPos;
    int lastReplayCount = 0;
    float pressTime = 0.f;
    struct DragTaker
    {
        ImGuiID window;  // the widget's window
        ImRect rect;     // its visible part, on the screen
        bool longPressIsRightClick;
    };
    ImVector<DragTaker> takers, previousTakers;  // SetItemTakesDrags(): this frame's, the last frame's
};

enum class ImGuiTouchPinchGesture { None, Undecided, Pinch, DragStarting, Drag };

struct ImGuiTouchFinger
{
    int id;
    ImVec2 pos;
};

struct ImGuiTouchPinchState
{
    ImVector<ImGuiTouchFinger> fingers;  // the fingers down, in the order they landed
    float startDistance = 0.f;  // between the first two fingers, when the second one landed
    ImVec2 startCenter;         // their middle, then
    ImGuiTouchPinchGesture gesture = ImGuiTouchPinchGesture::None;
    float baseScale = 1.f;  // style.FontScaleMain when the pinch began
};

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

// While the keyboard is shown: where the caret was, so that its windows scroll only when it moves
struct ImGuiTouchCaretView
{
    ImGuiID window = 0;
    ImVec2 inContent = ImVec2(-1.f, -1.f);
    ImVec2 visible;  // the visible area's top and bottom
    int framesLeft = 0;
};

struct ImGuiTouchContext
{
    ImGuiTouchConfig Config;
    bool Animating = false;       // asked during the current frame
    bool WasAnimating = false;    // asked during the last complete frame: what IsAnimating() tells
    bool WheelSessionEnabled = true;
    bool SimulatedTouch = false;  // SimulateTouch, as applied at the last frame
    ImGuiTouchKeyboardInsets KeyboardInsets;
    ImGuiTouchCaretView CaretView;
    ImGuiTouchScrollState Scroll;
    ImGuiTouchPinchState Pinch;
    ImGuiKitWheelSessionState WheelSession;
};

//-----------------------------------------------------------------------------
// The kit's state, one per Dear ImGui context: the layout helpers' Begin/End stacks, the help markers', FitText's
// buffer, and the touch layer's state
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

// In a browser (SetupEmscripten, SetupEmscriptenKeyboard): what the page reported at the last frame, and the buffers
// reused at each frame (no allocation once they have grown)
struct ImGuiKitBrowserState
{
    ImGuiMouseSource LastPointerSource = ImGuiMouseSource_Mouse;
    ImVector<int> FingersDown, FingersNow;  // the fingers' ids at the last frame, at this one
    // The keyboard: what the page was last told
    bool KeyboardWant = false;
    bool KeyboardKeysMode = false;  // the widget is not an InputText
    ImGuiID KeyboardId = 0;         // the active widget
    ImVec2 KeyboardPos;
    float KeyboardLineHeight = 0.f;
    float KeyboardDisplayW = 0.f;
    ImGuiID MirrorHash = 0;  // the text last mirrored into the page's field (its hash and length), and its caret
    int MirrorLen = -1;
    int MirrorCaret = -1;
    int FramesWithoutWant = 0;
    bool DroppedPress[ImGuiMouseButton_COUNT] = {};
    ImVector<char> Text;    // a piece of the page's answer, zero-terminated (a text typed)
    ImVector<char> Script;  // a script to send to the page
};

#ifdef __EMSCRIPTEN__
// The scripts sent to the page, built in a reused buffer: text appended, a JS string literal appended (quoted and
// escaped), then the zero that ends it
void ImGuiKitScriptAppend(ImVector<char>& script, const char* text);
void ImGuiKitScriptAppendString(ImVector<char>& script, const char* text, int len);
const char* ImGuiKitScriptEnd(ImVector<char>& script);
#endif

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
    bool TouchSetUp = false;                            // a touch setup was called: Touch is in use
    ImGuiTouchContext Touch;
    ImGuiKitBrowserState Browser;
};

// The touch layer's steps, which ImGuiTouch::Setup() calls through Dear ImGui's context hooks (the tests call them
// directly, frame by frame)
namespace ImGuiTouch
{
    // The swipe: a finger that drags the content of a window scrolls it, with inertia; a tap still clicks, a short hold
    // then a drag goes to the widget. On a touch screen, thinScrollbars makes the bars thin.
    // Right after ImGui::NewFrame(), before any widget.
    void UpdateTouchScroll(ImGuiTouchScrollMode mode, bool longPressIsRightClick, bool thinScrollbars);

    // A thin scroll bar held by a finger widens over the content (iOS): after the GUI, when this frame's scroll is final
    void DrawTouchScrollbarGrab();

    // The content dragged or flung past its end (the rubber band, the bounce): right after ImGui::Render(), before the
    // draw data is rendered
    void ApplyTouchOverscroll();

    // A second finger landed (a pinch): the layer lets go of its press (no swipe, no tap, no inertia) and keeps the
    // widgets out of it until the fingers lift. When a widget holds the press (a hold happened), it is interrupted
    // only if asked. Returns false when a widget keeps its press.
    bool TouchScrollLetGo(bool evenAWidget);

    // Two fingers that drag together (a right drag): the layer releases its press for good, nothing parked, so that
    // the widgets see the mouse. Returns false when a widget holds the press and must keep it.
    bool TouchScrollRelease(bool evenAWidget);

    // The pinch, right after ImGui::NewFrame(): two fingers that spread or close scale the font (style.FontScaleMain)
    // while they stay on the screen; two fingers that move together are a right drag. The swipe lets go of its press
    // when the second finger lands.
    void UpdateTouchPinch(ImGuiTouchPinchMode mode, bool interruptsWidgets);

    // The current Dear ImGui context's touch state (see this file's top): created by the setups, freed with the Dear
    // ImGui context. GetCurrentContext() asserts that a setup was called; FindCurrentContext() returns nullptr.
    ImGuiTouchContext& GetCurrentContext();
    ImGuiTouchContext* FindCurrentContext();
    ImGuiTouchContext& CreateCurrentContextIfNeeded();
    // The content moves on its own this frame (the inertia, the bounce, an animation): IsAnimating() is true
    void RequestAnimation();
    void SetupKeepCaretInView(ImGuiContext* ctx);  // the caret kept above the keyboard (imgui_kit_touch_keyboard.cpp)
    // For the tests, which call the steps without the hooks: whether an animation was asked since the last call
    bool ConsumeAnimationRequest();
}

namespace ImGuiKit
{
    // The wheel session's two steps: right before ImGui::NewFrame() (the item that claimed the wheel in the last frame,
    // before NewFrame() gives it back to a window), and right after it, before any widget
    void WheelSessionBeforeNewFrame();
    void UpdateWheelSession(bool enabled);
}
