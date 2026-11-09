// imgui_kit: small, standalone additions to Dear ImGui
// https://github.com/pthom/imgui_kit (MIT license)
//
// Define IMGUI_KIT_USE_STL to get overloads that take std::string and std::vector.

/*

Index of this file:
// [SECTION] Header mess
// [SECTION] Demo
// [SECTION] Em units
// [SECTION] Layout
// [SECTION] Widgets
// [SECTION] Wheel session
// [SECTION] Idling
// [SECTION] Enums (ImGuiKit)
// [SECTION] Touch (ImGuiTouch)

*/

#pragma once

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#define IMGUI_KIT_VERSION       "0.5.1"
#define IMGUI_KIT_VERSION_NUM   500     // major * 10000 + minor * 100 + patch

#include "imgui.h"

#ifdef IMGUI_KIT_USE_STL
#include <string>
#include <vector>
#endif

typedef int ImGuiKitIcon;            // -> enum ImGuiKitIcon_
typedef int ImGuiTouchScrollMode;    // -> enum ImGuiTouchScrollMode_
typedef int ImGuiTouchPinchDelivery; // -> enum ImGuiTouchPinchDelivery_
typedef int ImGuiTouchAxis;          // -> enum ImGuiTouchAxis_
typedef int ImGuiTouchPhase;         // -> enum ImGuiTouchPhase_
typedef int ImGuiTouchPinchKind;     // -> enum ImGuiTouchPinchKind_

namespace ImGuiKit
{

//-----------------------------------------------------------------------------
// [SECTION] Demo
//-----------------------------------------------------------------------------

// The kit's demo window, as ImGui::ShowDemoWindow(). Removed by IMGUI_KIT_DISABLE_DEMO.
IMGUI_API void ShowDemoWindow(bool* p_open = nullptr);
// The demo's content, in the current window (a host that places the window itself)
IMGUI_API void ShowDemoContents();

//-----------------------------------------------------------------------------
// [SECTION] Em units
//-----------------------------------------------------------------------------
// Sizes in multiples of the current font size, as the em unit of CSS: they follow the font,
// so they stay right on high-DPI screens and when the user zooms the text.
// Size your widgets in em rather than in bare pixels (whose size in millimeters varies with the DPI):
//     ImGui::Button("Click", ImVec2(130, 39));                 // 130 x 39 pixels: small on a high-DPI screen
//     ImGui::Button("Click", ImGuiKit::EmToVec2(10, 3));       // 10 em x 3 em (the same at the default font size of
//                                                              // 13 px): the same size on every screen, it follows
//                                                              // the font
IMGUI_API float  EmSize();                          // The current font size, in pixels
IMGUI_API float  EmSize(float lines);               // The height of this many lines of text, in pixels
IMGUI_API ImVec2 EmToVec2(float x, float y);        // A size or a position in em, in pixels
IMGUI_API ImVec2 EmToVec2(ImVec2 v);
IMGUI_API ImVec2 PixelsToEm(ImVec2 pixels);         // Pixels, in em
IMGUI_API float  PixelSizeToEm(float pixel_size);

//-----------------------------------------------------------------------------
// [SECTION] Layout
//-----------------------------------------------------------------------------
// Dear ImGui sizes an item when it is called, before the items that follow it exist. Most helpers below measure
// what they need on the previous frame (kept in the window's state storage, under their str_id: keep it stable), so
// a row can know the width of its end, or a group its own width, before drawing them. Each helper says what its
// first frame does; what is hidden then takes no input either.

// Fill row: the next item takes the line's width, minus the items after FillRowTail().
//
//     | [====== slider, it fills the line ======] [>] (i) |
//       ^ the fill item                           ^ the tail, on the right
//
// Every widget that takes its width from SetNextItemWidth() fills the line this way: sliders, drags, InputText,
// Combo, ColorEdit... A widget that takes a size argument instead (a plot, a child window) gets the width that
// BeginFillRow() returns. Without a fill row, the room for the tail must be computed by hand, and an error of a
// few pixels gives the window a horizontal scrollbar.
//
//     ImGuiKit::BeginFillRow("slice");
//     ImGui::SliderInt("##slice", &z, 0, 59);        // fills the line
//     ImGuiKit::FillRowTail();                        // the items after this call stay on the right
//     ImGui::ArrowButton("##next", ImGuiDir_Right);
//     ImGuiKit::EndFillRow();
//
// First frame: the fill item takes the whole line, and the tail is hidden while it is measured.
IMGUI_API float  BeginFillRow(const char* str_id);   // Sets the next item's width, and returns it
IMGUI_API void   FillRowTail();
IMGUI_API void   EndFillRow();

// Aligned group: a group placed on the line from its width on the previous frame.
// align_x: 0.0 = left, 0.5 = centered, 1.0 = right. It aligns the group's box, not each line inside it.
// Inside a column (below), it aligns within the column's width instead of the line's.
//
//     |                         [Cancel] [Apply] |    <- BeginAlignedGroup("buttons", 1.0f)
//
// First frame: the group is hidden while its width is measured (and while its column's width is unknown).
IMGUI_API void   BeginAlignedGroup(const char* str_id, float align_x);
IMGUI_API void   EndAlignedGroup();

// Column: a group that knows its width (from the previous frame), so that aligned groups inside it align within it.
// Items under a knob, centered under it:
//
//     ImGuiKit::BeginColumn("clahe");
//     ImGuiKnobs::Knob("CLAHE", &clip, 1.0f, 6.0f);
//     ImGuiKit::BeginAlignedGroup("help", 0.5f);   // centered under the knob
//     ImGuiKit::HelpMarker("Contrast-limited adaptive histogram equalization...");
//     ImGuiKit::EndAlignedGroup();
//     ImGuiKit::EndColumn();
//     ImGui::SameLine();                             // the next column
//
// First frame: drawn as a plain group; it only measures its width.
IMGUI_API void   BeginColumn(const char* str_id);
IMGUI_API void   EndColumn();

// Label column: labels on the left, and the widgets after them aligned, at the widest label (of the previous frame).
// Label() starts a row; the rest of the row is a normal line.
// labels_align_x: 0.0 = left-aligned labels, 1.0 = right-aligned (as the x of the style's ButtonTextAlign).
//
//     Volume   [=======o========]
//     Output   [x] Muted  [Test sound]
//
//     ImGuiKit::BeginLabelColumn("audio");
//     ImGuiKit::Label("Volume");
//     ImGui::SliderFloat("##volume", &volume, 0.0f, 1.0f);
//     ImGuiKit::Label("Output");
//     ImGui::Checkbox("Muted", &muted);
//     ImGui::SameLine();
//     ImGui::Button("Test sound");
//     ImGuiKit::EndLabelColumn();
//
// First frame: the whole label column is hidden while its labels are measured.
IMGUI_API void   BeginLabelColumn(const char* str_id, float labels_align_x = 0.0f);
IMGUI_API void   Label(const char* text);
IMGUI_API void   EndLabelColumn();

// Group panel: a group in a frame, its label set in the frame's top border (as an HTML fieldset). The look of
// thedmd's group panel: https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353
// size: (0, 0) = the content's size; else at least that size.
//
//     +- Image info ----------+
//     | Size: 640 x 480       |
//     +-----------------------+
//
// No measurement: it draws its frame around its content in the same frame. The label is an ID scope: two panels may
// hold items with the same labels.
IMGUI_API void   BeginGroupPanel(const char* label, const ImVec2& size = ImVec2(0, 0));
IMGUI_API void   EndGroupPanel();

// Flow: items on the same line while they fit, then on the next one, as words in a paragraph.
// Call FlowNext() before each item (the first one too). An item wider than the line takes a line of its own.
//
//     | [Coral] [Lavender] [Mint]   |
//     | [Sunflower yellow] [Sky]    |
//
//     ImGuiKit::BeginFlow("presets");
//     for (int i = 0; i < count; i++)
//     {
//         ImGuiKit::FlowNext();
//         if (ImGui::Button(labels[i])) ...
//     }
//     ImGuiKit::EndFlow();
//
// First frame: the whole flow is hidden while its items are measured.
IMGUI_API void   BeginFlow(const char* str_id);
IMGUI_API void   FlowNext();
IMGUI_API void   EndFlow();

//-----------------------------------------------------------------------------
// [SECTION] Widgets
//-----------------------------------------------------------------------------

// Help marker: a small drawn (i). It shows its text:
//   - with a mouse: in a tooltip while hovered; a click pins it in a popup, until a click elsewhere or Escape;
//   - on a touch screen (which has no hover): in a popup, opened by a tap.
// width_em: the window's width, in em (a parameter named _em is in em; the kit's other sizes are in pixels, as
// Dear ImGui's); 0 = the text's width on one line, up to 35 em (Dear ImGui's tooltips do not wrap; its demo wraps
// its help at 35 em). Never wider than the screen. The height: at most 60% of the screen; a
// longer text scrolls in the popup, and the tooltip says to click to read it all.
// The text: '\n' starts a line with the default renderer; with a Markdown renderer, Markdown's rules apply (a
// single '\n' joins the lines of a paragraph, an empty line separates paragraphs).
// It goes to the next line when the current one has no room for it. Its ID is its text: two markers with the
// same text in the same ID scope share their popup (use PushID() or BeginHelpMarker() to tell them apart).
IMGUI_API void   HelpMarker(const char* text, float width_em = 0.0f);
// The same, with any content: draw it between the two calls when BeginHelpMarker() returns true.
// width_em: 0 = the content sizes the window (its text wraps at 35 em); give a width for a content that wraps to
// its window (Markdown).
//     if (ImGuiKit::BeginHelpMarker("##level_help")) { ImGui::Text("..."); ImGuiKit::EndHelpMarker(); }
IMGUI_API bool   BeginHelpMarker(const char* str_id, float width_em = 0.0f);
IMGUI_API void   EndHelpMarker();
// The width a help marker takes at the end of a line, with the spacing before it
IMGUI_API float  HelpMarkerWidth();
// How HelpMarker() draws its text. By default: wrapped plain text. An app can install a richer renderer
// (Dear ImGui Bundle installs one that renders Markdown). nullptr restores the default. Per Dear ImGui context (as
// its PlatformIO callbacks): call it after ImGui::CreateContext().
typedef void (*HelpMarkerTextRenderer)(const char* text);
IMGUI_API void   SetHelpMarkerTextRenderer(HelpMarkerTextRenderer renderer);

// Icon button: a button with a symbol drawn with the draw list, centered (a font's glyph often sits a little
// high in a button), in the text's color. size: (0, 0) = a square of the frame's height.
IMGUI_API bool   IconButton(const char* str_id, ImGuiKitIcon icon, const ImVec2& size = ImVec2(0, 0));

// Segmented switch: exclusive choices in one frame, the current one filled. Its corners follow the style's
// FrameRounding, as buttons and sliders do.
//
//     ( Demo | Code )        <- current = 1: "Code" is filled
//
// It reacts at the press, not the release, which suits touch screens. With the keyboard or a gamepad: activated
// (Space, Enter), Left and Right move the choice, and the same key leaves it. Returns true when *current changed.
IMGUI_API bool   SegmentedSwitch(const char* str_id, int* current, const char* const labels[], int labels_count);
// The width a segmented switch takes, to place it: e.g. on the next line when the current one is too short
IMGUI_API float  SegmentedSwitchWidth(const char* const labels[], int labels_count);
#ifdef IMGUI_KIT_USE_STL
IMGUI_API bool   SegmentedSwitch(const char* str_id, int* current, const std::vector<std::string>& labels);
#endif

// Fit text: the text, cut at a word and ended with an ellipsis, so that it takes at most max_lines once wrapped at
// wrap_width (in the current font). Returns the text itself when it fits; else a buffer of the kit, valid for this
// frame (a small cache: the cut is computed once per text and width, not at each frame; copy it to keep it).
//
//     ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width);
//     ImGui::TextUnformatted(ImGuiKit::FitText(summary, width, 2));   // two lines at most, then "..."
//     ImGui::PopTextWrapPos();
IMGUI_API const char* FitText(const char* text, float wrap_width, int max_lines);

// Resize handle: a grip in the bottom-right corner of the last item, which resizes it. Call it right after the item,
// which takes *size; dragging the grip changes *size (at least min_size). Returns true when *size changed.
// The grip lies over the item's corner: an item that acts on a bare press (a button) also sees the press that starts
// a resize, unless ImGui::SetNextItemAllowOverlap() is called before it. Plots, fields and child windows need nothing.
//
//     static ImVec2 size = ImGuiKit::EmToVec2(24, 12);
//     if (ImPlot::BeginPlot("Plot", size)) { ...; ImPlot::EndPlot(); }
//     ImGuiKit::ResizeHandle("##plot_size", &size);
IMGUI_API bool   ResizeHandle(const char* str_id, ImVec2* size, const ImVec2& min_size = ImVec2(0, 0));

//-----------------------------------------------------------------------------
// [SECTION] Wheel session
//-----------------------------------------------------------------------------

// The wheel session: the mouse wheel keeps scrolling the window it started on, as in a browser. A page scrolled with
// the wheel keeps scrolling when a plot or an image passes under the mouse, and the plot does not zoom; a wheel that
// starts on the plot zooms it. The page lets go of the wheel when the mouse moves, or 0.7 s after its last event.
// A widget that reads the wheel claims it while hovered, with Dear ImGui's own idiom (as ImPlot does):
//     if (ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY)) zoom *= powf(1.1f, ImGui::GetIO().MouseWheel);
// Call it once, after ImGui::CreateContext(); a later call changes `enabled` (false: Dear ImGui's own behavior).
IMGUI_API void   SetupWheelSession(bool enabled = true);

//-----------------------------------------------------------------------------
// [SECTION] Idling
//-----------------------------------------------------------------------------

// True while something of the kit moves on its own: a widget's animation (a segmented switch that slides), or the
// touch layer's (ImGuiTouch::IsAnimating(): a fling, a bounce, a long press's ring). An app that sleeps when nothing
// happens keeps drawing frames meanwhile; one that draws every frame can ignore it. With or without the touch layer.
IMGUI_API bool   IsAnimating();
// A widget that animates calls it at each frame of its animation, so that IsAnimating() is true
IMGUI_API void   RequestAnimation();
} // namespace ImGuiKit

//-----------------------------------------------------------------------------
// [SECTION] Enums (ImGuiKit)
//-----------------------------------------------------------------------------

// The symbols of IconButton()
enum ImGuiKitIcon_
{
    ImGuiKitIcon_Minus,
    ImGuiKitIcon_Plus,
    ImGuiKitIcon_ZoomIn,
    ImGuiKitIcon_ZoomOut,
    ImGuiKitIcon_ZoomOneToOne,
    ImGuiKitIcon_ZoomFit,
};

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
// that sleeps when nothing happens keeps drawing frames meanwhile. False without the touch layer. An app uses
// ImGuiKit::IsAnimating() instead, which adds the kit's widgets.
IMGUI_API bool   IsAnimating();

// Right after a widget that a finger drags: a drag that starts on it goes to it, not to the swipe.
//   - ImGuiTouchAxis_Any (a canvas, a knob, a plot): the widget gets the press at once, in every direction.
//   - ImGuiTouchAxis_X or _Y (a slider, a vertical slider): the layer waits until the finger moved a little, then gives
//     the drag to the widget if it goes along that axis, or scrolls the window if it goes across.
// long_press_is_right_click: a long press on it is still a right click. Nothing without the touch layer.
// Dear ImGui's own sliders and drags call it themselves when the build has IMGUI_HAS_ITEM_DRAG_AXIS_CALLBACK.
IMGUI_API void   SetItemTakesDrags(ImGuiTouchAxis axis = ImGuiTouchAxis_Any, bool long_press_is_right_click = true);

// Two fingers on a widget: by default they scale the font (a pinch) or are a right drag (moved together). A widget
// changes that with one of these calls, right after it, at each frame (the claims of the last frame decide where the
// fingers land). When claims nest, the smallest holding both fingers wins; the gesture stays with it until they lift.
// Nothing without the touch layer.
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
                                     ImGuiTouchPinchDelivery delivery = ImGuiTouchPinchDelivery_WheelContinuous);
//
// 3. A widget that pans with a mouse button (a plot: left; a node editor: right): two fingers moved together on it
//    become a drag with that button. Combines with 2. Alone, a pinch on the widget does nothing (the font keeps its
//    size); with 2 alone, two fingers moved together are a right drag.
IMGUI_API void   SetItemTwoFingerPanAsMouseDrag(ImGuiMouseButton button);
IMGUI_API void   SetRectTwoFingerPanAsMouseDrag(const ImVec2& min, const ImVec2& max, ImGuiMouseButton button);

// A finger on a touch screen, as the platform reports it: its id (any number the platform gives, stable while the
// finger is down), its position (in the mouse's coordinates), and its phase. Report every finger, the first one
// included: Dear ImGui sees only the first finger, as the mouse; these events let the kit see the others. With two
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
