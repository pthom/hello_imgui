// imgui_kit: the touch layer's internals (ImGuiTouch: the swipe, the pinch, the keyboard, the browser)
// https://github.com/pthom/imgui_kit (MIT license)
// How this could fit in Dear ImGui: docs/dear_imgui_integration.md
//
// As Dear ImGui's imgui_internal.h: no guarantee of forward compatibility here. Code that includes it may need changes
// at a new version of the kit.
//
// The layer's state, ImGuiTouchContext: one per Dear ImGui context, as Dear ImGui's own state is in ImGuiContext. Dear
// ImGui has no slot for an extension's state: the kit keeps it in its own per-context state (ImGuiKitContext, see
// imgui_kit_internal.h). Only imgui_kit_touch.cpp knows that: the layer's other sources reach the state through
// ImGuiTouch::GetTouchContext(). The setups (ImGuiTouch::Setup(), SetKeyboardInsets(), SetupEmscripten(),
// SetupEmscriptenKeyboard()) create it; the other ImGuiTouch functions assert that a setup was called.
#pragma once
#include "imgui_kit.h"
#include "imgui_internal.h"  // ImRect, ImGuiWindow, ImGuiAxis

#define IMGUI_TOUCH_FLICK_SAMPLES 16  // the swipe's recent positions of the finger: enough for 0.05 s at 240 fps

struct ImGuiTouchScrollSample
{
    float Time;
    ImVec2 Pos;
};

struct ImGuiTouchScrollState
{
    ImGuiContext* Context = nullptr;  // the state belongs to one context (its windows): a new one starts afresh
    int FrameCount = -1;            // (a new context may reuse the address of a destroyed one: its frames restart)
    bool Owning = false;            // the sentinel holds the active id: a press, not released nor handed over yet
    bool Swiping = false;           // the finger moved past the slop
    bool Parked = false;            // the sentinel holds the active id for a pinch: nothing until the fingers lift
    ImGuiWindow* Window = nullptr;  // the pressed window, then the one that scrolls
    ImGuiAxis Axis = ImGuiAxis_None;
    ImVec2 PressPos;
    ImGuiTouchScrollSample Samples[IMGUI_TOUCH_FLICK_SAMPLES];  // the finger's recent positions, a ring: the lift speed
                                    // comes from them
    int NbSamples = 0, NextSample = 0;
    ImVec2 Inertia;                 // px/s, after the release
    float Pull = 0.0f;              // px: how far the finger dragged past the end of the scroll (the rubber band)
    float Overscroll = 0.0f;        // px: the content is drawn this far past its end, along the axis (positive:
                                    // toward the start)
    float OverscrollSpeed = 0.0f;   // px/s, while it springs back
    int ReplayedPresses = 0;        // presses queued by the layer, which it must not claim
    bool WatchingLongPress = false; // a touch press, still so far: a long press when it stays
    bool LongPressArmed = false;    // it stayed: a right click when it lifts, unless it moves (a ring shows it)
    bool ThinScrollbars = false;    // this frame: thin bars on a touch screen
    bool MouseInUse = false;        // the last pointer that moved or pressed was a mouse (a touch laptop's): the
                                    // theme's bars
    // While the bars are thin: the style's own values, put back when they stop, and the values written last
    bool BarsWritten = false;
    float ThemeBarSize = 0.0f, ThemeBarRounding = 0.0f;
    float WrittenBarSize = 0.0f, WrittenBarRounding = 0.0f;
    ImGuiID HeldBar = 0;            // the scroll bar held by a finger, since HeldBarTime: it widens
    float HeldBarTime = 0.0f;
    double LastReplayTime = -1e9;   // the previous replayed press: the next one pairs with it (a double tap)
    double RippleTime = -1e9;       // when the hold handed the press over: a ring around the finger, briefly
    ImVec2 RipplePos;
    ImVec2 LastReplayPos;
    int LastReplayCount = 0;
    float PressTime = 0.0f;
    struct DragTaker
    {
        ImGuiID Window;             // the widget's window
        ImRect Rect;                // its visible part, on the screen
        bool LongPressIsRightClick;
        ImGuiAxis Axis;             // SetItemTakesDrags(X or Y): the drags along it only; ImGuiAxis_None: all, at once
    };
    ImVector<DragTaker> Takers, PreviousTakers;  // SetItemTakesDrags(): this frame's, the last frame's
    ImGuiAxis TakerAxis = ImGuiAxis_None;  // the press landed on a widget that takes the drags along this axis
};

typedef int ImGuiTouchPinchGesture;  // -> enum ImGuiTouchPinchGesture_
enum ImGuiTouchPinchGesture_
{
    ImGuiTouchPinchGesture_None,
    ImGuiTouchPinchGesture_Undecided,
    ImGuiTouchPinchGesture_Pinch,
    ImGuiTouchPinchGesture_DragStarting,  // decided; the button goes down at the next frame
    ImGuiTouchPinchGesture_Drag,
};

struct ImGuiTouchFinger
{
    int Id;
    ImVec2 Pos;
};

struct ImGuiTouchPinchState
{
    ImVector<ImGuiTouchFinger> Fingers;  // the fingers down, in the order they landed
    bool Cancelled = false;            // a finger was cancelled since the last frame: its press ends with no click
    float StartDistance = 0.0f;        // between the first two fingers, when the second one landed (or the pair
                                       // changed)
    ImVec2 StartCenter;                // their middle, then
    float ScaleBase = 1.0f;            // the scale reached when the pair last changed: the scale is
                                       // ScaleBase * distance / StartDistance
    ImGuiTouchPinchGesture Gesture = ImGuiTouchPinchGesture_None;
    float BaseScale = 1.0f;            // style.FontScaleMain when the pinch began
    // A widget that claims two fingers (ItemPinch, the adapters): the claims of one item, or of one rectangle, merge
    struct PinchTaker
    {
        ImGuiID Id = 0;                // the item's (ItemPinch, RectPinch); 0 for the adapters on a rectangle
        ImRect Rect;                   // its visible part, on the screen
        bool Data = false;             // ItemPinch(): the widget reads the gesture
        bool Wheel = false;            // SetItemPinchAsWheel()
        float WheelZoomFactor = 1.1f;  // the widget's zoom for one wheel unit
        ImGuiTouchPinchDelivery Delivery = ImGuiTouchPinchDelivery_WheelContinuous;
        bool Pan = false;              // SetItemTwoFingerPanAsMouseDrag()
        ImGuiMouseButton PanButton = ImGuiMouseButton_Right;
    };
    ImVector<PinchTaker> Takers, PreviousTakers;  // the claims: this frame's, the last frame's
    int FrameCount = -1;               // the frame UpdateTouchPinch() ran: the takers are noted then only
    bool HasTaker = false;             // the two fingers landed on a taker: the gesture is its
    PinchTaker Taker;
    float NotchesSent = 0.0f;          // the wheel notches sent to the taker during this pinch
    // The gesture's data for its taker (ItemPinch), at this frame; ended: the taker's id, for the frame after the lift
    ImGuiTouchPinch Data;
    float LastDistance = 0.0f;
    ImVec2 LastCenter;
    ImGuiID EndedId = 0;
    int EndedFrame = -1;
};

// While the keyboard is shown: where the caret was, so that its windows scroll only when it moves
struct ImGuiTouchCaretView
{
    ImGuiID Window = 0;
    ImVec2 InContent = ImVec2(-1.0f, -1.0f);
    ImVec2 Visible;  // the visible area's top and bottom
    int FramesLeft = 0;
};

// In a browser (SetupEmscripten, SetupEmscriptenKeyboard): what the page reported at the last frame, and the buffers
// reused at each frame (no allocation once they have grown)
struct ImGuiTouchUrlZone
{
    ImRect Rect;    // the link's visible part, in Dear ImGui's coordinates
    int UrlOffset;  // its url, in ImGuiTouchBrowserState::UrlZoneUrls
};

struct ImGuiTouchBrowserState
{
    bool Installed = false;  // SetupEmscripten() was called: the page has the probe (and takes the url zones)
    ImGuiMouseSource LastPointerSource = ImGuiMouseSource_Mouse;
    // The links the page opens itself (SetItemOpensUrl): this frame's, pushed to the page before the next frame when
    // they differ from the last frame's (kept in ...Pushed)
    ImVector<ImGuiTouchUrlZone> UrlZones, UrlZonesPushed;
    ImVector<char> UrlZoneUrls, UrlZoneUrlsPushed;  // their urls, each zero-terminated
    float UrlZonesDisplayW = 0.0f;                  // the display width the zones were pushed with
    ImVector<char> UrlZonesScript;                  // the script that tells the page, reused
    ImVector<int> FingersDown, FingersNow;  // the fingers' ids at the last frame, at this one
    ImVector<int> FingersCancelled;         // the fingers the browser cancelled since the last frame
    // The keyboard: what the page was last told
    bool KeyboardWant = false;
    bool KeyboardKeysMode = false;  // the widget is not an InputText
    ImGuiID KeyboardId = 0;         // the active widget
    ImVec2 KeyboardPos;
    float KeyboardLineHeight = 0.0f;
    float KeyboardDisplayW = 0.0f;
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
void ImGuiTouchScriptAppend(ImVector<char>& script, const char* text);
void ImGuiTouchScriptAppendString(ImVector<char>& script, const char* text, int len);
const char* ImGuiTouchScriptEnd(ImVector<char>& script);
#endif

struct ImGuiTouchContext
{
    ImGuiTouchConfig Config;
    bool Animating = false;       // asked during the current frame
    bool WasAnimating = false;    // asked during the last complete frame: what IsAnimating() tells
    bool SimulatedTouch = false;  // SimulateTouch, as applied at the last frame
    ImGuiTouchKeyboardInsets KeyboardInsets;
    ImGuiTouchCaretView CaretView;
    ImGuiTouchScrollState Scroll;
    ImGuiTouchPinchState Pinch;
    ImGuiTouchBrowserState Browser;  // SetupEmscripten(), SetupEmscriptenKeyboard()
};

// The touch layer's steps, which the setups call through Dear ImGui's context hooks (the tests call them
// directly, frame by frame)
namespace ImGuiTouch
{
    // The swipe: a finger that drags the content of a window scrolls it, with inertia; a tap still clicks, a short hold
    // then a drag goes to the widget. On a touch screen, thin_scrollbars makes the bars thin.
    // Right after ImGui::NewFrame(), before any widget.
    void UpdateTouchScroll(ImGuiTouchScrollMode mode, bool long_press_is_right_click, bool thin_scrollbars);

    // A thin scroll bar held by a finger widens over the content (iOS): after the GUI, when this frame's scroll is
    // final
    void DrawTouchScrollbarGrab();

    // The content dragged or flung past its end (the rubber band, the bounce): right after ImGui::Render(), before the
    // draw data is rendered
    void ApplyTouchOverscroll();

    // A second finger landed (a pinch): the layer lets go of its press (no swipe, no tap, no inertia) and keeps the
    // widgets out of it until the fingers lift. even_a_widget: a widget that holds the press (after a hold) loses it
    // too; else it keeps it, and this returns false.
    bool TouchScrollLetGo(bool even_a_widget);

    // Two fingers that drag together, or a cancel: the layer releases its press for good, nothing parked, so that
    // the widgets see the mouse. even_a_widget: as above (the layer's own gestures take a widget's press; a widget that
    // reads the pinch's data keeps its own). Returns false when a widget keeps its press.
    bool TouchScrollRelease(bool even_a_widget);

    // The pinch, right after ImGui::NewFrame(): two fingers that spread or close scale the font (style.FontScaleMain)
    // while they stay on the screen; two fingers that move together are a right drag. The swipe lets go of its press
    // when the second finger lands.
    void UpdateTouchPinch(bool scales_font);
    // A finger cancelled since the last frame (ImGuiTouchPhase_Cancelled), right after ImGui::NewFrame() and before the
    // swipe: the gesture ends, and the press ends with no click (the widget's and the swipe's).
    void ApplyFingerCancel();

    // While a widget's pinch lasts, the mouse is the fingers' center: right before ImGui::NewFrame()
    void PinchBeforeNewFrame();

    // The current Dear ImGui context's touch state (see this file's top): created by the setups, freed with the Dear
    // ImGui context. GetTouchContext() asserts that a setup was called; FindTouchContext() returns nullptr.
    ImGuiTouchContext& GetTouchContext();
    ImGuiTouchContext* FindTouchContext();
    ImGuiTouchContext& CreateCurrentContextIfNeeded();
    // A widget that takes a finger's drags (SetItemTakesDrags, or Dear ImGui's draggable widgets
    // through ImGuiItemDragAxisCallback when the build has it); ImGuiAxis_None: all, at once
    void AddDragTaker(const ImRect& item_rect, bool long_press_is_right_click, ImGuiAxis axis);
    // The content moves on its own this frame (the inertia, the bounce, an animation): IsAnimating() is true
    void RequestAnimation();
    void SetupKeepCaretInView(ImGuiContext* ctx);  // the caret kept above the keyboard (imgui_kit_touch_keyboard.cpp)
    // For the tests, which call the steps without the hooks: whether an animation was asked since the last call
    bool ConsumeAnimationRequest();
}
