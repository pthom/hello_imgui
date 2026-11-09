// imgui_kit: the touch layer in a browser (ImGuiTouch::SetupEmscripten)
// https://github.com/pthom/imgui_kit (MIT license)
#ifdef __EMSCRIPTEN__
#include "imgui_kit.h"
#include "imgui_kit_touch_internal.h"
#include "imgui_internal.h"  // AddContextHook, g.InputEventsQueue

#include <emscripten.h>
#include <ctype.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The browser knows whether a pointer is a mouse, a finger or a pen (the pointerType of its pointer events), and
// where each finger is; Dear ImGui's GLFW backend knows neither. A probe on the document keeps them, and a hook reads
// them at the start of each frame (NewFramePre: after the backend's NewFrame, before Dear ImGui reads the inputs).
// emscripten_run_script rather than EM_JS or EM_ASM: it also works in a side module, as in Pyodide. Each call evaluates
// a script: the hook makes one per frame (imguiKitFrame(), installed by the probe), and allocates nothing.
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
            "  const cancelled = [];"  // the fingers the browser took over since the last frame (pointercancel)
            "  window.imguiKitFingers = fingers;"
            // The url zones (SetItemOpensUrl): the links of the last frame, in Dear ImGui's units; a press that lands
            // on one and lifts on it (a tap, or a mouse's click) opens its url here, inside the handler: the only
            // place WebKit allows a new tab. What was opened, for the fallback not to open it again (imguiKitUrlOpened)
            "  window.imguiKitUrlZones = []; window.imguiKitUrlZonesDisplayW = 0; window.imguiKitUrlOpened = null;"
            "  let press = null;"
            "  const toImGui = (e) => { const c = document.getElementById('canvas'); if (!c) return null;"
            "    const r = c.getBoundingClientRect(), w = window.imguiKitUrlZonesDisplayW, s = (w > 0 && r.width > 0) ? r.width / w : 1;"
            "    return [(e.clientX - r.left) / s, (e.clientY - r.top) / s]; };"
            "  const inZone = (p, z) => p[0] >= z.x0 && p[0] <= z.x1 && p[1] >= z.y0 && p[1] <= z.y1;"
            "  const options = {capture: true, passive: true};"
            "  const onPointer = (e) => {"
            "    window.imguiKitPointerType = (e.pointerType === 'touch') ? 1 : (e.pointerType === 'pen') ? 2 : 0; };"
            "  const onUp = (e) => { window.imguiKitPointerDown = 0; delete fingers[e.pointerId]; };"
            "  document.addEventListener('pointerdown', (e) => {"
            "    onPointer(e); window.imguiKitPointerDown = 1;"
            "    if (e.pointerType === 'touch') fingers[e.pointerId] = [e.clientX, e.clientY];"
            "    const p = toImGui(e);"
            "    press = (e.isPrimary && e.button === 0 && p) ? {id: e.pointerId, zone: window.imguiKitUrlZones.find((z) => inZone(p, z)),"
            "      at: performance.now(), x: e.clientX, y: e.clientY, touch: e.pointerType === 'touch'} : null; }, options);"
            "  document.addEventListener('pointermove', (e) => {"
            "    onPointer(e); if (e.pointerId in fingers) fingers[e.pointerId] = [e.clientX, e.clientY]; }, options);"
            "  document.addEventListener('pointerup', (e) => {"
            "    onUp(e); const s = press; press = null;"
            "    if (!s || !s.zone || e.pointerId !== s.id || (s.touch && performance.now() - s.at > 500)) return;"
            "    if (Math.hypot(e.clientX - s.x, e.clientY - s.y) > 8) return;"
            "    const p = toImGui(e); if (!p || !inZone(p, s.zone)) return;"
            "    const w = window.open(s.zone.url, '_blank');"
            "    if (w) { w.opener = null; window.imguiKitUrlOpened = {url: s.zone.url, at: performance.now()}; } }, options);"
            "  document.addEventListener('pointercancel', (e) => {"
            "    press = null; if (e.pointerId in fingers) cancelled.push(e.pointerId); onUp(e); }, options);"
            // The frame's state in one string: "type down cssWidth" then ";id x y" for each finger (CSS px from the
            // canvas's corner), then "|id id" for the fingers cancelled since the last frame
            "  window.imguiKitFrame = () => {"
            "    const c = document.getElementById('canvas'), r = c ? c.getBoundingClientRect() : {left: 0, top: 0, width: 0};"
            "    let s = window.imguiKitPointerType + ' ' + window.imguiKitPointerDown + ' ' + r.width;"
            "    for (const id in fingers) s += ';' + id + ' ' + (fingers[id][0] - r.left) + ' ' + (fingers[id][1] - r.top);"
            "    if (cancelled.length > 0) { s += '|' + cancelled.join(' '); cancelled.length = 0; }"
            "    return s; };"
            "}"
            // iOS's own long press on the canvas (text selection, the callout menu) would pause the page's events
            "{ const c = document.getElementById('canvas');"
            "  if (c) { c.style.webkitTouchCallout = 'none'; c.style.webkitUserSelect = 'none'; c.style.userSelect = 'none'; } }";
        emscripten_run_script(script);
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

    // The fingers of the frame's state ("...;id x y;id x y"), to ImGuiTouch::AddFingerEvent(), scaled to Dear ImGui's
    // units (the canvas's CSS width against the display width); then the fingers that lifted since the last frame: as
    // cancelled when the browser took them over (a system gesture: their press must not click), else as ended
    void RelayFingers(ImGuiIO& io, ImGuiTouchBrowserState& b, const char* list, float cssWidth)
    {
        float scale = (cssWidth > 0.f) ? io.DisplaySize.x / cssWidth : 1.f;
        b.FingersNow.resize(0);
        for (const char* p = list; p != nullptr && *p == ';'; p = strchr(p + 1, ';'))
        {
            char* end = nullptr;
            int id = (int)strtol(p + 1, &end, 10);
            float x = strtof(end, &end), y = strtof(end, &end);
            if (end == p + 1)
                break;
            const ImGuiTouchPhase phase = b.FingersDown.contains(id) ? ImGuiTouchPhase_Moved : ImGuiTouchPhase_Began;
            ImGuiTouch::AddFingerEvent(id, ImVec2(x * scale, y * scale), phase);
            b.FingersNow.push_back(id);
        }
        b.FingersCancelled.resize(0);
        for (const char* c = (list != nullptr) ? strchr(list, '|') : nullptr; c != nullptr && *c != '\0';)
        {
            char* end = nullptr;
            int id = (int)strtol(c + 1, &end, 10);
            if (end == c + 1)
                break;
            b.FingersCancelled.push_back(id);
            c = end;
        }
        for (int id : b.FingersDown)
        {
            if (b.FingersNow.contains(id))
                continue;
            const bool cancelled = b.FingersCancelled.contains(id);
            ImGuiTouch::AddFingerEvent(id, ImVec2(0.f, 0.f), cancelled ? ImGuiTouchPhase_Cancelled : ImGuiTouchPhase_Ended);
        }
        b.FingersDown.swap(b.FingersNow);
    }

    // Dear ImGui's own links (TextLinkOpenURL), seen at the click, two or three frames after the gesture, from the
    // frame loop. Chrome and Firefox allow a new tab from there for a few seconds after a gesture; WebKit (Safari, and
    // every browser on iOS) allows none: the page itself goes there, and the back button returns (an attempt would
    // only show a "popup blocked" notice). A link a url zone just opened (SetItemOpensUrl after a TextLinkOpenURL) is
    // not opened twice. Checked on 2026-11-09 on an iPhone (Safari, Chrome), a Mac (Safari, Chrome, Firefox), Android.
    // Web addresses only: a javascript: url would run script in the page.
    bool OpenUrl(ImGuiContext*, const char* url)
    {
        if (ImStrnicmp(url, "http://", 7) != 0 && ImStrnicmp(url, "https://", 8) != 0 && ImStrnicmp(url, "mailto:", 7) != 0)
            return false;
        ImVector<char>& script = ImGuiTouch::GetCurrentContext().Browser.Script;
        script.resize(0);
        ImGuiTouchScriptAppend(script, "(u => {"
            " const o = window.imguiKitUrlOpened; if (o && o.url === u && performance.now() - o.at < 1500) return;"
            " const ua = navigator.userAgent;"
            " const webkit = /iPhone|iPad|iPod/.test(ua) || (/AppleWebKit/.test(ua) && !/Chrome|Chromium|Edg|OPR|Firefox/.test(ua));"
            " if (webkit) { window.location.assign(u); return; }"
            " const w = window.open(u); if (w) w.opener = null; else window.location.assign(u); })(");
        ImGuiTouchScriptAppendString(script, url, (int)strlen(url));
        ImGuiTouchScriptAppend(script, ")");
        emscripten_run_script(ImGuiTouchScriptEnd(script));
        return true;
    }

    // The last frame's url zones, to the page (when they changed), in one script: "[{x0, y0, x1, y1, url}, ...]"
    void PushUrlZones(ImGuiIO& io, ImGuiTouchBrowserState& b)
    {
        ImVector<char>& s = b.UrlZonesScript;
        s.resize(0);
        ImGuiTouchScriptAppend(s, "window.imguiKitUrlZones = [");
        char buffer[96];
        for (const ImGuiTouchUrlZone& z : b.UrlZones)
        {
            snprintf(buffer, sizeof(buffer), "{x0:%.1f,y0:%.1f,x1:%.1f,y1:%.1f,url:", z.rect.Min.x, z.rect.Min.y,
                     z.rect.Max.x, z.rect.Max.y);
            ImGuiTouchScriptAppend(s, buffer);
            const char* url = b.UrlZoneUrls.Data + z.urlOffset;
            ImGuiTouchScriptAppendString(s, url, (int)strlen(url));
            ImGuiTouchScriptAppend(s, "},");
        }
        snprintf(buffer, sizeof(buffer), "]; window.imguiKitUrlZonesDisplayW = %.1f;", io.DisplaySize.x);
        ImGuiTouchScriptAppend(s, buffer);
        const char* script = ImGuiTouchScriptEnd(s);
        const ImGuiID hash = ImHashStr(script);
        if (hash != b.UrlZonesPushed)
        {
            b.UrlZonesPushed = hash;
            emscripten_run_script(script);
        }
        b.UrlZones.resize(0);
        b.UrlZoneUrls.resize(0);
    }

    void BeforeNewFrame(ImGuiContext* ctx, ImGuiContextHook*)
    {
        ImGuiContext& g = *ctx;
        ImGuiTouchBrowserState& b = ImGuiTouch::GetCurrentContext().Browser;
        PushUrlZones(g.IO, b);
        // The frame's state: "type down cssWidth;id x y;...|cancelled ids"
        const char* frame = emscripten_run_script_string("window.imguiKitFrame ? window.imguiKitFrame() : ''");
        if (frame == nullptr)
            frame = "";
        char* p = nullptr;
        int type = (int)strtol(frame, &p, 10);
        bool pointerDown = strtol(p, &p, 10) != 0;
        float cssWidth = strtof(p, &p);
        ImGuiMouseSource source = (type == 1) ? ImGuiMouseSource_TouchScreen : (type == 2) ? ImGuiMouseSource_Pen
                                                                              : ImGuiMouseSource_Mouse;
        if (source != b.LastPointerSource && !ImGuiTouch::GetConfig().SimulateTouch)
            SetMouseSource(g, source);
        b.LastPointerSource = source;
        // There is no pointer between two touches: the position is invalid, as when a mouse leaves the window.
        // Otherwise the last touch keeps hovering what passes under it (the content that coasts after a fling), and
        // the GLFW backend re-adds that position at each frame (a finger triggers no mouseenter).
        if (source == ImGuiMouseSource_TouchScreen && !pointerDown)
            g.IO.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        RelayFingers(g.IO, b, p, cssWidth);
    }
}

void ImGuiTouchScriptAppend(ImVector<char>& script, const char* text)
{
    if (!script.empty() && script.back() == '\0')
        script.pop_back();
    int len = (int)strlen(text), size = script.Size;
    script.resize(size + len);
    memcpy(script.Data + size, text, (size_t)len);
}

void ImGuiTouchScriptAppendString(ImVector<char>& script, const char* text, int len)
{
    if (!script.empty() && script.back() == '\0')
        script.pop_back();
    script.push_back('\'');
    for (int i = 0; i < len; ++i)
    {
        unsigned char c = (unsigned char)text[i];
        if (c == '\'' || c == '"' || c == '\\')
        {
            script.push_back('\\');
            script.push_back((char)c);
        }
        else if (c < 0x20)
        {
            char escaped[8];
            snprintf(escaped, sizeof(escaped), "\\u%04x", c);
            for (int k = 0; escaped[k] != '\0'; ++k)
                script.push_back(escaped[k]);
        }
        else
            script.push_back((char)c);
    }
    script.push_back('\'');
}

const char* ImGuiTouchScriptEnd(ImVector<char>& script)
{
    if (script.empty() || script.back() != '\0')
        script.push_back('\0');
    return script.Data;
}

namespace ImGuiTouch
{
bool SetRectOpensUrl(const ImVec2& min, const ImVec2& max, const char* url)
{
    ImGuiTouchContext* touch = FindCurrentContext();
    if (touch == nullptr || !touch->Browser.Installed)
        return false;
    if (ImStrnicmp(url, "http://", 7) != 0 && ImStrnicmp(url, "https://", 8) != 0 && ImStrnicmp(url, "mailto:", 7) != 0)
        return false;
    // Its visible part only (a link scrolled out of a child: no zone, and nothing to click either)
    ImRect visible(min, max);
    visible.ClipWith(GImGui->CurrentWindow->ClipRect);
    if (visible.GetWidth() > 0.f && visible.GetHeight() > 0.f)
    {
        ImGuiTouchBrowserState& b = touch->Browser;
        const int len = (int)strlen(url);
        const int offset = b.UrlZoneUrls.Size;
        b.UrlZoneUrls.resize(offset + len + 1);
        memcpy(b.UrlZoneUrls.Data + offset, url, (size_t)len + 1);
        b.UrlZones.push_back(ImGuiTouchUrlZone{visible, offset});
    }
    return true;
}

bool SetItemOpensUrl(const char* url)
{
    return SetRectOpensUrl(GImGui->LastItemData.Rect.Min, GImGui->LastItemData.Rect.Max, url);
}

void SetupEmscripten()
{
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    IM_ASSERT(ctx != nullptr && "ImGuiTouch::SetupEmscripten() needs a context: call ImGui::CreateContext() first");
    CreateCurrentContextIfNeeded().Browser.Installed = true;  // the page's state (ImGuiTouchContext::Browser)
    InstallProbe();
    ImGui::GetPlatformIO().Platform_OpenInShellFn = OpenUrl;
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

#else  // not a browser: the caller opens its links itself
#include "imgui_kit.h"

namespace ImGuiTouch
{
bool SetItemOpensUrl(const char*) { return false; }
bool SetRectOpensUrl(const ImVec2&, const ImVec2&, const char*) { return false; }
} // namespace ImGuiTouch
#endif // #ifdef __EMSCRIPTEN__
