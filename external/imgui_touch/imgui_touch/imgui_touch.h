// imgui_touch: a touch screen for Dear ImGui, as on a phone (the swipe, the long press, the pinch, the keyboard)
// https://github.com/pthom/imgui_touch (MIT license)

/*

Index of this file:
// [SECTION] Header mess
// [SECTION] Touch (ImGuiTouch)
// [SECTION] Demo

*/

#pragma once

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#define IMGUI_TOUCH_VERSION       "0.1.0"
#define IMGUI_TOUCH_VERSION_NUM   100     // major * 10000 + minor * 100 + patch

#include "imgui.h"

typedef int ImGuiTouchScrollMode;    // -> enum ImGuiTouchScrollMode_
typedef int ImGuiTouchPinchDelivery; // -> enum ImGuiTouchPinchDelivery_
typedef int ImGuiTouchAxis;          // -> enum ImGuiTouchAxis_
typedef int ImGuiTouchPhase;         // -> enum ImGuiTouchPhase_
typedef int ImGuiTouchPinchKind;     // -> enum ImGuiTouchPinchKind_

//-----------------------------------------------------------------------------
// [SECTION] Touch (ImGuiTouch)
//-----------------------------------------------------------------------------
// A touch screen, as on a phone:
//   - the swipe: a finger that drags a window's content scrolls it, with inertia and a bounce at the end. A tap still
//     clicks; a short hold, then a drag, goes to the widget (a slider, a text selection); a long press is a right
//     click (a context menu);
//   - the pinch: two fingers that spread or close scale the font; two fingers that move together are a right drag
//     (the pan of a node editor).
//
//     ImGui::CreateContext();
//     ImGuiTouch::Setup();      // once: the layer then runs at each frame, through Dear ImGui's context hooks
//
// The touch input: io.AddMouseSourceEvent(ImGuiMouseSource_TouchScreen) tells a finger from a mouse (the Auto mode),
// and io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen gives thin scroll bars. The pinch needs the fingers, from
// the platform: ImGuiTouch::AddFingerEvent(). In a browser, ImGuiTouch::SetupEmscripten() does all of it.

// Whether a finger that drags the content of a window scrolls it (the swipe)
enum ImGuiTouchScrollMode_
{
    ImGuiTouchScrollMode_Auto,      // only when the input is a touch screen (io.MouseSource)
    ImGuiTouchScrollMode_Always,    // also with the mouse (left button): to try the phone's behavior on a desktop
    ImGuiTouchScrollMode_Disabled,
};

// The direction in which a widget takes a finger's drags (ImGuiTouch::SetItemTakesDrags); the values of Dear ImGui's
// internal ImGuiAxis
enum ImGuiTouchAxis_
{
    ImGuiTouchAxis_Any = -1,  // every direction, at once: a canvas, a knob, a plot
    ImGuiTouchAxis_X = 0,     // a slider
    ImGuiTouchAxis_Y = 1,     // a vertical slider
};

// How a pinch reaches a widget as wheel events (ImGuiTouch::SetItemPinchAsWheel)
enum ImGuiTouchPinchDelivery_
{
    ImGuiTouchPinchDelivery_WheelContinuous,  // the exact amount at each frame: a widget that zooms by the wheel's
                                              // value
    ImGuiTouchPinchDelivery_WheelNotches,     // whole notches (+1, -1): a widget that reads only the wheel's sign
                                              // (ImPlot)
};

// What two fingers do, as the layer decides it once per gesture (ImGuiTouchPinch::Kind)
enum ImGuiTouchPinchKind_
{
    ImGuiTouchPinchKind_Undecided,  // until their distance changed a little, or their middle moved a little
    ImGuiTouchPinchKind_Pinch,      // they spread or close
    ImGuiTouchPinchKind_Pan,        // they move together
};

// Two fingers on a widget, at this frame (ImGuiTouch::ItemPinch). A widget applies both the zoom and the pan at each
// frame: the content stays under the fingers, as on a map.
//     ImGuiTouchPinch p = ImGuiTouch::ItemPinch();
//     if (p.Active)
//     {
//         offset = p.Center - (p.Center - offset) * p.ScaleDelta;  // zoom around the fingers
//         zoom *= p.ScaleDelta;
//         offset += p.CenterDelta;                                 // follow their middle
//     }
struct ImGuiTouchPinch
{
    bool   Active;       // two fingers are on this widget's gesture (false: the deltas are neutral)
    bool   Started;      // the gesture's first frame
    bool   Ended;        // the fingers lifted (or were cancelled) since the last frame: Active is false then
    ImVec2 Center;       // the fingers' middle, in screen coordinates
    ImVec2 CenterDelta;  // its move since the last frame: the pan
    float  ScaleDelta;   // the fingers' distance over the last frame's: 1 when unchanged
    float  Scale;        // their distance over the distance at the start: 1 at the start
    ImGuiTouchPinchKind Kind;  // pinch or pan, for a widget that wants one at a time

    ImGuiTouchPinch() : Active(false), Started(false), Ended(false), ScaleDelta(1.0f), Scale(1.0f),
                        Kind(ImGuiTouchPinchKind_Undecided) {}
};

// The phase of a finger, as the platforms report it (ImGuiTouch::AddFingerEvent)
enum ImGuiTouchPhase_
{
    ImGuiTouchPhase_Began,      // the finger landed
    ImGuiTouchPhase_Moved,      // it is still down (moved or not)
    ImGuiTouchPhase_Ended,      // it lifted
    ImGuiTouchPhase_Cancelled,  // the system took the touch over (a system gesture, a call): it lifted, and its press
                                // ends with no click
};

// The parts of the screen hidden by a phone's keyboard: see ImGuiTouch::GetKeyboardInsets(). Top and Bottom are
// distances inward from the display's top and bottom edges, in Dear ImGui's coordinates: the visible part is
// [Top, io.DisplaySize.y - Bottom]. Zero when no keyboard shows.
struct ImGuiTouchKeyboardInsets
{
    float Top;
    float Bottom;
    ImGuiTouchKeyboardInsets(float top = 0.0f, float bottom = 0.0f) : Top(top), Bottom(bottom) {}
};

// The settings, read at each frame: change them through ImGuiTouch::GetConfig()
struct ImGuiTouchConfig
{
    //
    // One finger: the swipe, the long press, the scroll bars
    //

    // The swipe: a finger that drags a window's content scrolls it, with inertia, even from a button: a tap still
    // clicks it (when the finger lifts), and a short hold then a drag goes to the widget (a slider, a text selection).
    ImGuiTouchScrollMode ScrollMode = ImGuiTouchScrollMode_Auto;
    // A finger still for this long hands its press to the widget under it, which then gets the drag (iOS: 150 ms).
    // Zero: the widget gets the press at once, and a swipe scrolls only from where no widget is.
    float HoldSeconds = 0.15f;
    // On a touch screen (never with a mouse): a finger still for LongPressSeconds, then lifted, is a right click (the
    // context menus); a ring around the finger shows when the lift will right click. The release reaches no widget:
    // a long press on a button is not a click. A move after the ring cancels it: the widget keeps its press.
    bool LongPressIsRightClick = true;
    float LongPressSeconds = 0.5f;  // iOS: about 500 ms
    // On a touch screen (ImGuiConfigFlags_IsTouchScreen): thin scroll bars (0.3 em, rounded), indicators rather than
    // controls, since a swipe scrolls anywhere. Applied at each frame (after a theme, a pinch). A hold on a bar hands
    // its thumb over, drawn wider while held (as on iOS). While the pointer in use is a mouse (io.MouseSource: a touch
    // laptop driven by its mouse), the theme's bars; thin again at the next finger. False keeps the theme's bars:
    // turned off (or when the input stops being a touch screen), the layer puts back the style's ScrollbarSize and
    // ScrollbarRounding.
    bool ThinScrollbars = true;

    //
    // Two fingers: the pinch, and a drag with two fingers
    //

    // Two fingers that pinch outside the widgets that claim them (ImGuiTouch::ItemPinch and its kin) scale the font
    // (style.FontScaleMain) while they stay on the screen. False: such a pinch does nothing, and the font keeps its
    // size. Either way, a widget that takes the pinch zooms, and two fingers that move together are a right drag (the
    // pan of a node editor, the box of a plot). A second finger that lands while a widget holds the first one (a slider
    // dragged after a hold, a text selection) leaves the widget its drag. A desktop touch screen reports no second
    // finger.
    bool PinchScalesFont = true;
    // The pinch's range of style.FontScaleMain
    float PinchFontScaleMin = 0.5f;
    float PinchFontScaleMax = 5.0f;

    //
    // Testing
    //

    // The mouse plays a finger, to try the phone's behavior on a desktop: the layer tags the mouse's events as a
    // touch screen's (io.MouseSource), with any backend, before Dear ImGui reads them. Everything that depends on a
    // touch screen follows: no hover tooltips, the long press, the hold before a widget's drag. (ScrollMode_Always
    // differs: the mouse's drag scrolls, but the mouse stays a mouse.)
    bool SimulateTouch = false;
};

namespace ImGuiTouch
{
// Installs the touch layer in the current Dear ImGui context. Call once, after ImGui::CreateContext(); a later call
// replaces the config. Its state is freed with the context. The functions a reusable widget calls (they say so) do
// nothing without it; the others assert.
IMGUI_API void   Setup(const ImGuiTouchConfig& config = ImGuiTouchConfig());
IMGUI_API bool   IsInstalled();  // true once a setup was called in the current context
IMGUI_API ImGuiTouchConfig& GetConfig();

// True while the touch layer moves something on its own (a fling's inertia, the bounce, a long press's ring): an app
// that sleeps when nothing happens keeps drawing frames meanwhile. False without the touch layer.
IMGUI_API bool   IsAnimating();

// Right after a widget that a finger drags: a drag that starts on it goes to it, not to the swipe.
//   - ImGuiTouchAxis_Any (a canvas, a knob, a plot): the widget gets the press at once, in every direction.
//   - ImGuiTouchAxis_X or _Y (a slider, a vertical slider): the layer waits until the finger moved a little, then gives
//     the drag to the widget if it goes along that axis, or scrolls the window if it goes across.
// long_press_is_right_click: a long press on it is still a right click. Nothing without the touch layer.
// Dear ImGui's own sliders and drags call it themselves when the build has IMGUI_HAS_ITEM_DRAG_AXIS_CALLBACK.
IMGUI_API void   SetItemTakesDrags(ImGuiTouchAxis axis = ImGuiTouchAxis_Any, bool long_press_is_right_click = true);

// Two fingers on a widget: by default they scale the font (a pinch) or are a right drag (moved together). A widget
// changes that with one of these calls, right after it, at each frame, hovered or not (the claims of the last frame
// decide where the fingers land, and there is no mouse between two touches). When claims nest, the smallest holding
// both fingers wins; the gesture stays with it until they lift. Nothing without the touch layer.
//
// 1. The gesture itself (a canvas, a map): returns its data at this frame. The widget gets nothing else: no font
//    scale, no wheel, no mouse drag. The item needs an id (an InvisibleButton, not an Image). Not with 2 or 3.
IMGUI_API ImGuiTouchPinch ItemPinch();
// The same, for a rectangle in screen coordinates and an id of the caller's: a widget that is not one item
IMGUI_API ImGuiTouchPinch RectPinch(ImGuiID id, const ImVec2& min, const ImVec2& max);
//
// 2. A widget that zooms with the wheel (a plot, an image): a pinch on it becomes wheel events, at the fingers' center.
//    wheel_zoom_factor: the widget's zoom for one wheel unit (1.1: +10%). delivery: WheelNotches for a widget that
//    reads only the wheel's sign (ImPlot).
IMGUI_API void   SetItemPinchAsWheel(float wheel_zoom_factor,
                                     ImGuiTouchPinchDelivery delivery = ImGuiTouchPinchDelivery_WheelContinuous);
IMGUI_API void   SetRectPinchAsWheel(const ImVec2& min, const ImVec2& max, float wheel_zoom_factor,
                                     ImGuiTouchPinchDelivery delivery = ImGuiTouchPinchDelivery_WheelContinuous,
                                     ImGuiID id = 0);
//
// 3. A widget that pans with a mouse button (a plot: left; a node editor: right): two fingers moved together on it
//    become a drag with that button. Combines with 2. Alone, a pinch on the widget does nothing (the font keeps its
//    size); with 2 alone, two fingers moved together are a right drag.
IMGUI_API void   SetItemTwoFingerPanAsMouseDrag(ImGuiMouseButton button);
IMGUI_API void   SetRectTwoFingerPanAsMouseDrag(const ImVec2& min, const ImVec2& max, ImGuiMouseButton button,
                                                ImGuiID id = 0);
//
// The rectangle's id (optional): the widget's item that the first finger may hold when the second one lands (a
// canvas's background); the gesture takes that press over, as with the item's calls. Otherwise a widget that holds the
// press keeps it, and the two fingers do nothing.

// A canvas that shows its widgets zoomed and moved (a node editor) wraps them in these, so that their claims (above,
// and SetItemTakesDrags) are where the widgets show: on screen, local * scale + offset. Nests. The gesture's data
// (ItemPinch) stays in screen coordinates. Nothing without the touch layer.
IMGUI_API void   PushClaimTransform(float scale, const ImVec2& offset);
IMGUI_API void   PopClaimTransform();

// A finger on a touch screen, as the platform reports it: its id (any number the platform gives, stable while the
// finger is down), its position (in the mouse's coordinates), and its phase. Report every finger, the first one
// included: Dear ImGui sees only the first finger, as the mouse; these events let the layer see the others. With two
// fingers down: they spread or close, a pinch; they move together, a right drag. A desktop app with a mouse never
// calls it. The platforms' phases: iOS (UITouchPhase), Android (ACTION_DOWN, _MOVE, _UP, _CANCEL), SDL (finger down,
// motion, up, and canceled in SDL3), Windows (WM_POINTER, its canceled flag). GLFW reports no fingers.
// Cancelled: the system took the touch over. The finger lifts, and the press it made ends with no click (a widget
// under it is not clicked, a swipe does not fling).
IMGUI_API void   AddFingerEvent(int id, const ImVec2& pos, ImGuiTouchPhase phase);

// The parts of the screen hidden by a phone's keyboard (in Dear ImGui's coordinates): a full-screen window keeps out
// of them. Zero when no keyboard shows, and without the touch layer.
IMGUI_API ImGuiTouchKeyboardInsets GetKeyboardInsets();
// A native app tells where its keyboard is (iOS: UIKeyboardWillChangeFrameNotification), and zero when it hides: the
// values stay until the next call. While a keyboard shows, the windows that hold the text field scroll its caret
// into view, above the keyboard. In a browser, SetupEmscriptenKeyboard() sets them (zero included). It works before
// Setup() too: a native app may report its keyboard first.
IMGUI_API void   SetKeyboardInsets(const ImGuiTouchKeyboardInsets& insets);

// A link that opens in a new tab, in a browser, whatever the browser and the pointer. Right after the link's item, at
// each frame. The page opens the url from inside the tap's or the click's own handler, where every browser allows a
// new tab (WebKit allows none from the frame loop, where Dear ImGui sees the click). A finger: a tap (lifted within
// half a second, not moved); a mouse: a press and its release on the link.
// Returns true when the page takes the link: the caller then does nothing on the click. False outside a browser,
// without SetupEmscripten(), or for a scheme other than http, https and mailto: the caller opens the url itself.
//     ImGui::TextLink("Read the docs");
//     if (!ImGuiTouch::SetItemOpensUrl(url) && ImGui::IsItemClicked())
//         OpenInBrowser(url);
// The same, for a rectangle in screen coordinates (a link that is not one item).
IMGUI_API bool   SetItemOpensUrl(const char* url);
IMGUI_API bool   SetRectOpensUrl(const ImVec2& min, const ImVec2& max, const char* url);

#ifdef __EMSCRIPTEN__
// In a browser, with any of Dear ImGui's Emscripten backends: tells Dear ImGui which pointer is a finger (the
// browser's pointer events), relays the fingers to AddFingerEvent() (the pinch), and sets
// ImGuiConfigFlags_IsTouchScreen on a touch screen. Call once, after the backend's init and ImGuiTouch::Setup(). The
// canvas must have the id "canvas".
// Also: the url zones (SetItemOpensUrl); Dear ImGui's own links (TextLinkOpenURL) open in a new tab where the browser
// allows it from the frame loop (Chrome, Firefox), else in the same tab (WebKit: Safari, every browser on iOS); and
// the phone's own long press (text selection, its menu) is turned off on the canvas.
IMGUI_API void   SetupEmscripten();
// The phone's keyboard, for Dear ImGui's text widgets (and any widget that wants text: io.WantTextInput). A phone's
// browser shows its keyboard only for a field focused inside a touch handler: a small keyboard button shows next to
// the active text widget, and a tap on it (or on the widget again) opens the keyboard. Call once, after
// SetupEmscripten(). Without it, the app has its own text input.
IMGUI_API void   SetupEmscriptenKeyboard();
#endif
} // namespace ImGuiTouch

//-----------------------------------------------------------------------------
// [SECTION] Demo
//-----------------------------------------------------------------------------

namespace ImGuiTouch
{
// The touch layer's demo window, as ImGui::ShowDemoWindow(): a list to swipe, sliders, a pinch on a canvas, a text
// field for the phone's keyboard. Removed by IMGUI_TOUCH_DISABLE_DEMO.
IMGUI_API void   ShowDemoWindow(bool* p_open = nullptr);
// The demo's content, in the current window (a host that places the window itself: a phone's full screen)
IMGUI_API void   ShowDemoContents();
} // namespace ImGuiTouch
