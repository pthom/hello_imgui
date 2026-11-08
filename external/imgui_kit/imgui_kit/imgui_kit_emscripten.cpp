// imgui_kit: the touch layer in a browser (ImGuiTouch::SetupEmscripten)
// https://github.com/pthom/imgui_kit (MIT license)
#ifdef __EMSCRIPTEN__
#include "imgui_kit.h"
#include "imgui_internal.h"  // AddContextHook, g.InputEventsQueue

#include <emscripten.h>
#include <float.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

// The browser knows whether a pointer is a mouse, a finger or a pen (the pointerType of its pointer events), and
// where each finger is; Dear ImGui's GLFW backend knows neither. A probe on the document keeps them, and a hook reads
// them at the start of each frame (NewFramePre: after the backend's NewFrame, before Dear ImGui reads the inputs).
// emscripten_run_script rather than EM_JS or EM_ASM: it also works in a side module, as in Pyodide.
namespace
{
    const ImGuiID kEmscriptenHooksOwner = ImHashStr("ImGuiTouch_Emscripten");

    void InstallProbe()
    {
        const char* script =
            "if (window.imguiKitPointerType === undefined) {"
            "  window.imguiKitPointerType = 0;"
            "  window.imguiKitPointerDown = 0;"
            "  const fingers = {};"  // the fingers down: their pointer id -> [clientX, clientY]
            "  window.imguiKitFingers = fingers;"
            "  const options = {capture: true, passive: true};"
            "  const onPointer = (e) => {"
            "    window.imguiKitPointerType = (e.pointerType === 'touch') ? 1 : (e.pointerType === 'pen') ? 2 : 0; };"
            "  const onUp = (e) => { window.imguiKitPointerDown = 0; delete fingers[e.pointerId]; };"
            "  document.addEventListener('pointerdown', (e) => {"
            "    onPointer(e); window.imguiKitPointerDown = 1;"
            "    if (e.pointerType === 'touch') fingers[e.pointerId] = [e.clientX, e.clientY]; }, options);"
            "  document.addEventListener('pointermove', (e) => {"
            "    onPointer(e); if (e.pointerId in fingers) fingers[e.pointerId] = [e.clientX, e.clientY]; }, options);"
            "  document.addEventListener('pointerup', onUp, options);"
            "  document.addEventListener('pointercancel', onUp, options);"
            "}"
            // iOS's own long press on the canvas (text selection, the callout menu) would pause the page's events
            "{ const c = document.getElementById('canvas');"
            "  if (c) { c.style.webkitTouchCallout = 'none'; c.style.webkitUserSelect = 'none'; c.style.userSelect = 'none'; } }";
        emscripten_run_script(script);
    }

    ImGuiMouseSource LastPointerSource()
    {
        int type = emscripten_run_script_int("window.imguiKitPointerType | 0");
        return (type == 1) ? ImGuiMouseSource_TouchScreen : (type == 2) ? ImGuiMouseSource_Pen : ImGuiMouseSource_Mouse;
    }

    // The backend's callbacks run inside the browser's event handlers, between two frames, and tagged their events
    // with the source known then: when the probe's pointer type changed, the events of those gestures are retagged,
    // and the source is set for what follows. Only on a change: between changes, the source is the app's to set (a
    // demo that simulates a finger with the mouse).
    void SetMouseSource(ImGuiContext& g, ImGuiMouseSource source)
    {
        for (ImGuiInputEvent& e : g.InputEventsQueue)
        {
            if (e.AddedByTestEngine)  // the touch layer's own events (replayed, with the source they need)
                continue;
            if (e.Type == ImGuiInputEventType_MousePos)
                e.MousePos.MouseSource = source;
            else if (e.Type == ImGuiInputEventType_MouseButton)
                e.MouseButton.MouseSource = source;
            else if (e.Type == ImGuiInputEventType_MouseWheel)
                e.MouseWheel.MouseSource = source;
        }
        g.IO.AddMouseSourceEvent(source);
    }

    // The fingers down, to ImGuiTouch::AddFingerEvent(): "id x y;..." in CSS pixels from the canvas's corner, scaled
    // to Dear ImGui's units (the canvas's CSS width against the display width)
    void RelayFingers(ImGuiIO& io)
    {
        std::string list = emscripten_run_script_string(
            "(() => { const c = document.getElementById('canvas'); if (!c) return '';"
            " const r = c.getBoundingClientRect(); window.imguiKitCanvasCssWidth = r.width;"
            " return Object.entries(window.imguiKitFingers || {})"
            "   .map(([id, p]) => id + ' ' + (p[0] - r.left) + ' ' + (p[1] - r.top)).join(';'); })()");
        int cssWidth = emscripten_run_script_int("Math.round(window.imguiKitCanvasCssWidth || 0)");
        float scale = (cssWidth > 0) ? io.DisplaySize.x / (float)cssWidth : 1.f;

        static std::vector<int> sDown;  // the fingers down at the last frame
        std::vector<int> down;
        std::istringstream entries(list);
        std::string entry;
        while (std::getline(entries, entry, ';'))
        {
            std::istringstream fields(entry);
            int id = 0;
            float x = 0.f, y = 0.f;
            if (!(fields >> id >> x >> y))
                continue;
            ImGuiTouch::AddFingerEvent(id, ImVec2(x * scale, y * scale), true);
            down.push_back(id);
        }
        for (int id : sDown)
            if (std::find(down.begin(), down.end(), id) == down.end())
                ImGuiTouch::AddFingerEvent(id, ImVec2(0.f, 0.f), false);
        sDown = down;
    }

    void BeforeNewFrame(ImGuiContext* ctx, ImGuiContextHook*)
    {
        ImGuiContext& g = *ctx;
        static ImGuiMouseSource sLastSource = ImGuiMouseSource_Mouse;  // the probe's, at the last frame
        ImGuiMouseSource source = LastPointerSource();
        if (source != sLastSource && !ImGuiTouch::GetConfig().SimulateTouch)
            SetMouseSource(g, source);
        sLastSource = source;
        // There is no pointer between two touches: the position is invalid, as when a mouse leaves the window.
        // Otherwise the last touch keeps hovering what passes under it (the content that coasts after a fling), and
        // the GLFW backend re-adds that position at each frame (a finger triggers no mouseenter).
        if (source == ImGuiMouseSource_TouchScreen && emscripten_run_script_int("window.imguiKitPointerDown | 0") == 0)
            g.IO.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        RelayFingers(g.IO);
    }
}

namespace ImGuiTouch
{
void SetupEmscripten()
{
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "ImGuiTouch::SetupEmscripten() needs a context: call ImGui::CreateContext() first");
    InstallProbe();
    if (emscripten_run_script_int("(navigator.maxTouchPoints > 0) ? 1 : 0") != 0)
        ctx->IO.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    for (const ImGuiContextHook& hook : ctx->Hooks)
        if (hook.Owner == kEmscriptenHooksOwner && hook.Type != ImGuiContextHookType_PendingRemoval_)
            return;
    ImGuiContextHook hook;
    hook.Type = ImGuiContextHookType_NewFramePre;
    hook.Owner = kEmscriptenHooksOwner;
    hook.Callback = BeforeNewFrame;
    ImGui::AddContextHook(ctx, &hook);
}
} // namespace ImGuiTouch
#endif // #ifdef __EMSCRIPTEN__
