#ifdef __EMSCRIPTEN__
#include "hello_imgui/internal/backend_impls/emscripten_pointer_probe.h"

#include <emscripten.h>
#include <cstdio>
#include <vector>

// emscripten_run_script rather than EM_JS or EM_ASM: it also works in a side module, as in Pyodide (where the SDL
// backend labels its events itself: the probe is then only the touch screen hint)
namespace HelloImGui
{
    void InstallEmscriptenPointerProbe()
    {
        const char* script =
            "if (window.helloImGuiPointerType === undefined) {"
            "  window.helloImGuiPointerType = 0;"
            "  window.helloImGuiPointerDown = 0;"
            "  const onPointer = (e) => {"
            "    window.helloImGuiPointerType = (e.pointerType === 'touch') ? 1 : (e.pointerType === 'pen') ? 2 : 0; };"
            "  const options = {capture: true, passive: true};"
            "  const fingers = {};"
            "  window.helloImGuiPinch = {count: 0, startDist: 0, dist: 0, moveX: 0, moveY: 0, startCx: 0, startCy: 0};"
            "  const updatePinch = () => {"
            "    const pts = Object.values(fingers), p = window.helloImGuiPinch, wasTwo = p.count >= 2;"
            "    p.count = pts.length;"
            "    if (pts.length >= 2) {"
            "      const d = Math.hypot(pts[0][0] - pts[1][0], pts[0][1] - pts[1][1]);"
            "      const cx = (pts[0][0] + pts[1][0]) / 2, cy = (pts[0][1] + pts[1][1]) / 2;"
            "      if (!wasTwo) { p.startDist = d; p.startCx = cx; p.startCy = cy; }"
            "      p.dist = d; p.moveX = cx - p.startCx; p.moveY = cy - p.startCy; } };"
            "  const fingerDown = (e) => { if (e.pointerType === 'touch') { fingers[e.pointerId] = [e.clientX, e.clientY]; updatePinch(); } };"
            "  const fingerMove = (e) => { if (e.pointerId in fingers) { fingers[e.pointerId] = [e.clientX, e.clientY]; updatePinch(); } };"
            "  const fingerUp = (e) => { delete fingers[e.pointerId]; updatePinch(); };"
            "  window.helloImGuiTapZones = []; window.helloImGuiTapDisplayW = 0;"
            "  let tap = null;"  // the finger that may tap: it lands alone; where, when (ImGui's units)
            "  const toImGui = (t) => {"
            "    const c = document.getElementById('canvas'); if (!c) return null;"
            "    const r = c.getBoundingClientRect(), w = window.helloImGuiTapDisplayW, s = (w > 0 && r.width > 0) ? r.width / w : 1;"
            "    return [(t.clientX - r.left) / s, (t.clientY - r.top) / s]; };"
            "  const inZone = (p, z) => p[0] >= z.x0 && p[0] <= z.x1 && p[1] >= z.y0 && p[1] <= z.y1;"
            "  const zoneAt = (p) => window.helloImGuiTapZones.find((z) => inZone(p, z));"
            "  document.addEventListener('touchstart', (e) => {"
            "    const t = e.changedTouches[0], p = toImGui(t);"
            "    tap = (e.touches.length === 1 && p) ? {id: t.identifier, zone: zoneAt(p), time: performance.now()} : null; }, options);"
            "  document.addEventListener('touchend', (e) => {"
            "    const t = e.changedTouches[0], p = t && toImGui(t), start = tap;"
            "    tap = null;"
            "    if (!start || !start.zone || !p || t.identifier !== start.id || performance.now() - start.time > 500) return;"
            "    if (inZone(p, start.zone)) window.open(start.zone.url, '_blank'); }, options);"
            "  document.addEventListener('pointerdown', (e) => { onPointer(e); window.helloImGuiPointerDown = 1; fingerDown(e); }, options);"
            "  document.addEventListener('pointermove', (e) => { onPointer(e); fingerMove(e); }, options);"
            "  document.addEventListener('pointerup', (e) => { window.helloImGuiPointerDown = 0; fingerUp(e); }, options);"
            "  document.addEventListener('pointercancel', (e) => { window.helloImGuiPointerDown = 0; fingerUp(e); }, options);"
            "}";
        emscripten_run_script(script);
    }

    void InstallEmscriptenViewportResizeRelay()
    {
        // Safari applies a page zoom just after the load: it fires the window's resize while innerWidth is still the
        // unzoomed width, then changes innerWidth with only a visualViewport resize. A canvas sized from the window's
        // resize events (the GLFW port's "window" mode) kept the unzoomed size, wider than the page. The window's
        // resize is fired again when the page's size differs from what the last one told; once at the install too (a
        // zoom that landed before).
        const char* script =
            "if (window.visualViewport && !window.helloImGuiViewportRelay) {"
            "  window.helloImGuiViewportRelay = true;"
            "  let told = [-1, -1];"
            "  window.addEventListener('resize', () => { told = [innerWidth, innerHeight]; }, true);"
            "  const relay = () => {"
            "    if (innerWidth !== told[0] || innerHeight !== told[1]) window.dispatchEvent(new Event('resize')); };"
            "  visualViewport.addEventListener('resize', relay);"
            "  relay();"
            "}";
        emscripten_run_script(script);
    }

    ImGuiMouseSource LastEmscriptenPointerSource()
    {
        int type = emscripten_run_script_int("window.helloImGuiPointerType | 0");
        return (type == 1) ? ImGuiMouseSource_TouchScreen : (type == 2) ? ImGuiMouseSource_Pen : ImGuiMouseSource_Mouse;
    }

    bool EmscriptenPointerIsDown()
    {
        return emscripten_run_script_int("window.helloImGuiPointerDown | 0") != 0;
    }

    ImVec2 EmscriptenTwoFingerMove()
    {
        // CSS px, scaled to ImGui's units (the canvas's CSS width against ImGui's display width)
        int moveX = emscripten_run_script_int("window.helloImGuiPinch ? Math.round(window.helloImGuiPinch.moveX) : 0");
        int moveY = emscripten_run_script_int("window.helloImGuiPinch ? Math.round(window.helloImGuiPinch.moveY) : 0");
        int cssWidth = emscripten_run_script_int("(() => { const c = document.getElementById('canvas'); return c ? Math.round(c.getBoundingClientRect().width) : 0; })()");
        float scale = (cssWidth > 0) ? ImGui::GetIO().DisplaySize.x / (float)cssWidth : 1.f;
        return ImVec2((float)moveX * scale, (float)moveY * scale);
    }

    int EmscriptenFingerCount()
    {
        return emscripten_run_script_int("window.helloImGuiPinch ? window.helloImGuiPinch.count : 0");
    }

    float EmscriptenPinchScale()
    {
        int permille = emscripten_run_script_int(
            "(window.helloImGuiPinch && window.helloImGuiPinch.startDist > 0)"
            " ? Math.round(window.helloImGuiPinch.dist / window.helloImGuiPinch.startDist * 1000) : 1000");
        return (float)permille / 1000.f;
    }

    namespace
    {
        struct TapZone { ImVec2 rectMin, rectMax; std::string url; };
        std::vector<TapZone> gTapZoneRequests;  // this frame's
    }

    void EmscriptenRequestTapZone(ImVec2 rectMin, ImVec2 rectMax, const std::string& url)
    {
        gTapZoneRequests.push_back({rectMin, rectMax, url});
    }

    void EmscriptenPushTapZones(float displayWidth)
    {
        static std::string lastScript;
        std::string script = "window.helloImGuiTapZones = [";
        char buffer[160];
        for (const TapZone& z : gTapZoneRequests)
        {
            std::string safeUrl;
            for (char c : z.url)  // a quote in a url: never valid there, dropped
                if (c != '\'' && c != '\\' && c != '\n')
                    safeUrl += c;
            snprintf(buffer, sizeof(buffer), "{x0: %.1f, y0: %.1f, x1: %.1f, y1: %.1f, url: '",
                     z.rectMin.x, z.rectMin.y, z.rectMax.x, z.rectMax.y);
            script += std::string(buffer) + safeUrl + "'},";
        }
        snprintf(buffer, sizeof(buffer), "]; window.helloImGuiTapDisplayW = %.1f", displayWidth);
        script += buffer;
        gTapZoneRequests.clear();
        if (script != lastScript)
        {
            lastScript = script;
            emscripten_run_script(script.c_str());
        }
    }

    bool EmscriptenHasTouchScreen()
    {
        return emscripten_run_script_int("(navigator.maxTouchPoints > 0) ? 1 : 0") != 0;
    }
}
#endif
