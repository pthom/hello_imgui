// imgui_kit: small, standalone additions to Dear ImGui
// https://github.com/pthom/imgui_kit (MIT license)
//
// One header, imgui_kit.h. Its sources, beside it: the imgui_kit*.cpp files, imgui_kit_demo.cpp being the demo
// (optional). They need only Dear ImGui (its docking or master branch, 1.93 or later).
// Define IMGUI_KIT_USE_STL to get overloads that take std::string and std::vector.

/*

Index of this file:
// [SECTION] Header mess
// [SECTION] Demo
// [SECTION] Em units
// [SECTION] Layout
// [SECTION] Widgets
// [SECTION] Wheel session
// [SECTION] Touch (ImGuiTouch)

*/

#pragma once

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#define IMGUI_KIT_VERSION       "0.1.0"
#define IMGUI_KIT_VERSION_NUM   100     // major * 10000 + minor * 100 + patch

#include "imgui.h"

#ifdef IMGUI_KIT_USE_STL
#include <string>
#include <vector>
#endif

typedef int ImGuiKitIcon;           // -> enum ImGuiKitIcon_
typedef int ImGuiTouchScrollMode;   // -> enum ImGuiTouchScrollMode_
typedef int ImGuiTouchPinchMode;    // -> enum ImGuiTouchPinchMode_

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

IMGUI_API float  EmSize();                          // The current font size, in pixels
IMGUI_API float  EmSize(float nb_lines);            // The height of nb_lines lines of text, in pixels
IMGUI_API ImVec2 EmToVec2(float x, float y);        // A size or a position in em, in pixels
IMGUI_API ImVec2 EmToVec2(ImVec2 v);
IMGUI_API ImVec2 PixelsToEm(ImVec2 pixels);         // Pixels, in em
IMGUI_API float  PixelSizeToEm(float pixel_size);

//-----------------------------------------------------------------------------
// [SECTION] Layout
//-----------------------------------------------------------------------------
// Dear ImGui sizes an item when it is called, before the items that follow it exist. The helpers below measure
// what they need on the previous frame (kept in the window's state storage), so a row can know the width of its
// end, or a group its own width, before drawing them. The first frame draws them hidden while they are measured.

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
IMGUI_API float  BeginFillRow(const char* str_id);   // Sets the next item's width, and returns it
IMGUI_API void   FillRowTail();
IMGUI_API void   EndFillRow();

// Aligned group: a group placed on the line from its width on the previous frame.
// align_x: 0.0 = left, 0.5 = centered, 1.0 = right. It aligns the group's box, not each line inside it.
// Inside a column (below), it aligns within the column's width instead of the line's.
//
//     |                         [Cancel] [Apply] |    <- BeginAlignedGroup("buttons", 1.0f)
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
// (Dear ImGui Bundle installs one that renders Markdown). nullptr restores the default.
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
// It reacts at the press, not the release, which suits touch screens. Returns true when *current changed.
IMGUI_API bool   SegmentedSwitch(const char* str_id, int* current, const char* const labels[], int labels_count);
#ifdef IMGUI_KIT_USE_STL
IMGUI_API bool   SegmentedSwitch(const char* str_id, int* current, const std::vector<std::string>& labels);
#endif

// Fit text: the text, cut at a word and ended with an ellipsis, so that it takes at most max_lines once wrapped at
// wrap_width (in the current font). Returns the text itself when it fits; else a buffer of the kit, valid until the
// next call (copy it to keep two results).
//
//     ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width);
//     ImGui::TextUnformatted(ImGuiKit::FitText(summary, width, 2));   // two lines at most, then "..."
//     ImGui::PopTextWrapPos();
IMGUI_API const char* FitText(const char* text, float wrap_width, int max_lines);

// Resize handle: a grip at the bottom-right corner of the last item, which resizes it. Call it right after the item,
// which takes *size; dragging the grip changes *size (at least min_size). Returns true when *size changed.
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

} // namespace ImGuiKit

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

// What two fingers do (in a browser; a desktop touch screen reports no second finger)
enum ImGuiTouchPinchMode_
{
    ImGuiTouchPinchMode_FontScale,  // the pinch scales the font (style.FontScaleMain); two fingers dragged together are a right drag
    ImGuiTouchPinchMode_Disabled,
};

// The parts of the screen hidden by a phone's keyboard: see ImGuiTouch::GetKeyboardInsets()
struct ImGuiTouchKeyboardInsets
{
    float Top;
    float Bottom;
    ImGuiTouchKeyboardInsets(float top = 0.0f, float bottom = 0.0f) : Top(top), Bottom(bottom) {}
};

// The settings, read at each frame: change them through ImGuiTouch::GetConfig()
struct ImGuiTouchConfig
{
    // The swipe: a finger that drags a window's content scrolls it, with inertia, even from a button: a tap still
    // clicks it (when the finger lifts), and a short hold then a drag goes to the widget (a slider, a text selection).
    ImGuiTouchScrollMode ScrollMode = ImGuiTouchScrollMode_Auto;
    // On a touch screen (never with a mouse): a finger still for half a second, then lifted, is a right click (the
    // context menus); a ring around the finger shows when the lift will right click. The release reaches no widget:
    // a long press on a button is not a click. A move after the ring cancels it: the widget keeps its press.
    bool LongPressIsRightClick = true;
    // On a touch screen (ImGuiConfigFlags_IsTouchScreen): thin scroll bars (0.3 em, rounded), indicators rather than
    // controls, since a swipe scrolls anywhere. Applied at each frame (after a theme, a pinch). A hold on a bar hands
    // its thumb over, drawn wider while held (as on iOS). False keeps the theme's bars.
    bool ThinScrollbars = true;
    // Two fingers that pinch scale the font while they stay on the screen; two fingers that move together are a
    // right drag (the pan of a node editor, the box of a plot)
    ImGuiTouchPinchMode PinchMode = ImGuiTouchPinchMode_FontScale;
    // The pinch's range of style.FontScaleMain
    float PinchFontScaleMin = 0.5f;
    float PinchFontScaleMax = 5.0f;
    // When the second finger lands while a widget holds the first one (a slider dragged after a hold, a text
    // selection): the widget keeps its drag and the pinch is ignored (false), or the pinch takes over (true)
    bool PinchInterruptsWidgets = false;
    // The mouse plays a finger, to try the phone's behavior on a desktop: the layer tags the mouse's events as a
    // touch screen's (io.MouseSource), with any backend, before Dear ImGui reads them
    bool SimulateTouch = false;
};

namespace ImGuiTouch
{
// Installs the touch layer in the current Dear ImGui context, once (a second call only changes the config); it creates
// the layer's state, freed with the context. Call it first: the other ImGuiTouch functions assert without it, except
// IsAnimating(), SetItemTakesDrags() and GetKeyboardInsets(), which do nothing (or return zero) then.
IMGUI_API void   Setup(const ImGuiTouchConfig& config = ImGuiTouchConfig());
IMGUI_API ImGuiTouchConfig& GetConfig();

// True while the content moves on its own (a fling's inertia, the bounce, an animation): an app that sleeps when
// nothing happens keeps drawing frames meanwhile. An app that draws every frame can ignore it.
IMGUI_API bool   IsAnimating();

// Right after a widget that a finger drags (a plot, a knob, a node editor's canvas): a drag that starts on it goes to
// it, not to the swipe. long_press_is_right_click: a long press on it is still a right click.
IMGUI_API void   SetItemTakesDrags(bool long_press_is_right_click = true);

// A finger on a touch screen, as the platform reports it (a browser's touch or pointer events, SDL's finger events):
// its id (any number the platform gives, stable while the finger is down), its position (in the mouse's coordinates),
// and whether it is down. Dear ImGui sees only the first finger, as the mouse: these events let the kit see the others.
// With two fingers down: they spread or close, a pinch (the font scales); they move together, a right drag.
// A desktop app with a mouse never calls it.
IMGUI_API void   AddFingerEvent(int id, const ImVec2& pos, bool down);

// The parts of the screen hidden by a phone's keyboard (in Dear ImGui's coordinates): a full-screen window keeps out
// of them. Zero when no keyboard shows.
IMGUI_API ImGuiTouchKeyboardInsets GetKeyboardInsets();
// A native app tells where its keyboard is (iOS: UIKeyboardWillChangeFrameNotification), and zero when it hides.
// While a keyboard shows, the windows that hold the text field scroll its caret into view, above the keyboard.
// In a browser, SetupEmscriptenKeyboard() sets them.
IMGUI_API void   SetKeyboardInsets(const ImGuiTouchKeyboardInsets& insets);

#ifdef __EMSCRIPTEN__
// In a browser, with any of Dear ImGui's Emscripten backends: tells Dear ImGui which pointer is a finger (the
// browser's pointer events), relays the fingers to AddFingerEvent() (the pinch), and sets ImGuiConfigFlags_IsTouchScreen
// on a touch screen. Call once, after the backend's init and ImGuiTouch::Setup(). The canvas must have the id "canvas".
// Also: Dear ImGui's links (TextLinkOpenURL) open on a phone (in the same tab: a phone's browser refuses new tabs
// there), and the phone's own long press (text selection, its menu) is turned off on the canvas.
IMGUI_API void   SetupEmscripten();
// The phone's keyboard, for Dear ImGui's text widgets (and any widget that wants text: io.WantTextInput). A phone's
// browser shows its keyboard only for a field focused inside a touch handler: a small keyboard button shows next to
// the active text widget, and a tap on it (or on the widget again) opens the keyboard. Call once, after
// SetupEmscripten(). Without it, the app has its own text input.
IMGUI_API void   SetupEmscriptenKeyboard();
#endif
} // namespace ImGuiTouch

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
