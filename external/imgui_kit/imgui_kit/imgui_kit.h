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

*/

#pragma once

//-----------------------------------------------------------------------------
// [SECTION] Header mess
//-----------------------------------------------------------------------------

#define IMGUI_KIT_VERSION       "0.6.0"
#define IMGUI_KIT_VERSION_NUM   600     // major * 10000 + minor * 100 + patch

#include "imgui.h"

#ifdef IMGUI_KIT_USE_STL
#include <string>
#include <vector>
#endif

typedef int ImGuiKitIcon;            // -> enum ImGuiKitIcon_

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

// True while one of the kit's widgets moves on its own (a segmented switch that slides). An app that sleeps when
// nothing happens keeps drawing frames meanwhile; one that draws every frame can ignore it. The touch layer
// (https://github.com/pthom/imgui_touch) has its own: ImGuiTouch::IsAnimating().
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
