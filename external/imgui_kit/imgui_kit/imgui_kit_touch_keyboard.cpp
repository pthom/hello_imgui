// imgui_kit: the keyboard's insets, and the caret kept above the keyboard (ImGuiTouch::SetKeyboardInsets)
// https://github.com/pthom/imgui_kit (MIT license)
// Any platform: the browser's keyboard code sets the insets (imgui_kit_emscripten_keyboard.cpp), a native app sets them
// from its keyboard's notifications.
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_kit.h"
#include "imgui_kit_touch_internal.h"
#include "imgui_internal.h"

namespace ImGuiTouch
{
namespace
{

    // While the keyboard is shown, the caret's line stays in view, as in native apps once the keyboard shrinks them:
    // the windows that hold it scroll. Only when the caret moves in its content, or the display changes: the user may
    // scroll elsewhere meanwhile.

    void KeepCaretInView(ImGuiContext& g, ImGuiTouchCaretView& view, bool keyboardShown, float visibleTop,
                         float visibleBottom)
    {
        const ImGuiPlatformImeData& ime = g.PlatformImeData;  // the last complete frame's
        ImGuiWindow* window = g.NavWindow;
        if (!keyboardShown || !ime.WantTextInput || window == nullptr)
        {
            view = ImGuiTouchCaretView();
            return;
        }
        ImVec2 inContent(ime.InputPos.x + window->Scroll.x, ime.InputPos.y + window->Scroll.y);
        bool changed = window->ID != view.window || inContent.x != view.inContent.x || inContent.y != view.inContent.y
                       || visibleTop != view.visible.x || visibleBottom != view.visible.y;
        if (changed)
            view.framesLeft = 3;  // the layout takes a frame or two to follow a new visible area
        view.window = window->ID;
        view.inContent = inContent;
        view.visible = ImVec2(visibleTop, visibleBottom);
        if (view.framesLeft == 0)
            return;
        --view.framesLeft;
        float margin = ime.InputLineHeight * 0.5f;
        float top = ime.InputPos.y - margin, bottom = ime.InputPos.y + ime.InputLineHeight + margin;
        for (ImGuiWindow* w = window; w != nullptr; w = w->ParentWindow)
        {
            ImRect clip = w->InnerClipRect;
            clip.Min.y = ImMax(clip.Min.y, visibleTop);
            clip.Max.y = ImMin(clip.Max.y, visibleBottom);
            float delta = (bottom > clip.Max.y) ? bottom - clip.Max.y : (top < clip.Min.y) ? top - clip.Min.y : 0.f;
            // A window moves the field at most to its own top (a child whose top is under the keyboard cannot show
            // it): its parents scroll for the rest. A field out of its window's view is not drawn, and Dear ImGui then
            // wants no text: the keyboard would close.
            if (delta > 0.f)
                delta = ImMin(delta, ImMax(0.f, top - w->InnerClipRect.Min.y));
            // A top-level window that reaches below the keyboard shrinks above it (the app's full-screen window), and
            // can then scroll that much further than the previous frame's ScrollMax
            float growth = (w->Flags & ImGuiWindowFlags_ChildWindow)
                           ? 0.f : ImMax(0.f, w->InnerClipRect.Max.y - visibleBottom);
            float target = ImClamp(w->Scroll.y + delta, 0.f, w->ScrollMax.y + growth);
            float moved = target - w->Scroll.y;
            if (moved == 0.f)
                continue;
            ImGui::SetScrollY(w, target);
            top -= moved;
            bottom -= moved;
        }
    }
}  // namespace

void SetKeyboardInsets(const ImGuiTouchKeyboardInsets& insets)
{
    CreateCurrentContextIfNeeded().KeyboardInsets = insets;  // a native app may report its keyboard before Setup()
    if (ImGuiContext* ctx = ImGui::GetCurrentContext())
        SetupKeepCaretInView(ctx);
}

ImGuiTouchKeyboardInsets GetKeyboardInsets()
{
    ImGuiTouchKeyboardInsets none;  // no keyboard reported yet: an app reads its insets at each frame
    ImGuiTouchContext* state = FindTouchContext();
    return state ? state->KeyboardInsets : none;
}

// Before each frame (a NewFramePre hook, installed once): the caret's line stays above the keyboard
void SetupKeepCaretInView(ImGuiContext* ctx)
{
    const ImGuiID owner = ImHashStr("ImGuiTouch_KeepCaretInView");
    for (const ImGuiContextHook& hook : ctx->Hooks)
        if (hook.Owner == owner && hook.Type != ImGuiContextHookType_PendingRemoval_)
            return;
    ImGuiContextHook hook;
    hook.Type = ImGuiContextHookType_NewFramePre;
    hook.Owner = owner;
    hook.Callback = [](ImGuiContext* g, ImGuiContextHook*) {
        ImGuiTouchContext& tc = GetTouchContext();
        const ImGuiTouchKeyboardInsets& insets = tc.KeyboardInsets;
        bool shown = insets.Top > 0.f || insets.Bottom > 0.f;
        KeepCaretInView(*g, tc.CaretView, shown, insets.Top, g->IO.DisplaySize.y - insets.Bottom);
    };
    ImGui::AddContextHook(ctx, &hook);
}
} // namespace ImGuiTouch
