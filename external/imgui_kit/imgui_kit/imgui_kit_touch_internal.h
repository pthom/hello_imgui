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
// ImGuiTouch::GetCurrentContext(). The setups (ImGuiTouch::Setup(), SetKeyboardInsets(), SetupEmscripten(),
// SetupEmscriptenKeyboard()) create it; the other ImGuiTouch functions assert that a setup was called.
#pragma once
#include "imgui_kit.h"
#include "imgui_internal.h"  // ImRect, ImGuiWindow, ImGuiAxis

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
    struct PinchTaker
    {
        ImRect rect;         // the widget, on the screen
        float zoomPerNotch;  // how much the widget zooms per wheel notch (0.1: 10%)
        ImGuiMouseButton panButton;
    };
    ImVector<PinchTaker> takers, previousTakers;  // SetItemTakesPinch(): this frame's, the last frame's
    int frameCount = -1;                          // the frame UpdateTouchPinch() ran: the takers are noted then only
    bool hasTaker = false;                        // the two fingers landed on a taker: the gesture is its
    PinchTaker taker;
    int notchesSent = 0;                          // the wheel notches sent to the taker during this pinch
};

// While the keyboard is shown: where the caret was, so that its windows scroll only when it moves
struct ImGuiTouchCaretView
{
    ImGuiID window = 0;
    ImVec2 inContent = ImVec2(-1.f, -1.f);
    ImVec2 visible;  // the visible area's top and bottom
    int framesLeft = 0;
};

// In a browser (SetupEmscripten, SetupEmscriptenKeyboard): what the page reported at the last frame, and the buffers
// reused at each frame (no allocation once they have grown)
struct ImGuiTouchBrowserState
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

    // While a widget's pinch lasts, the mouse is the fingers' center: right before ImGui::NewFrame()
    void PinchBeforeNewFrame(ImGuiContext& g);

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
