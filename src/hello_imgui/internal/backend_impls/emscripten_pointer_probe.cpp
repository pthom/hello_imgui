#ifdef __EMSCRIPTEN__
#include "hello_imgui/internal/backend_impls/emscripten_pointer_probe.h"

#include <emscripten.h>
#include <cstdio>

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
            "  window.helloImGuiPinch = {count: 0, startDist: 0, dist: 0};"
            "  const updatePinch = () => {"
            "    const pts = Object.values(fingers), p = window.helloImGuiPinch, wasTwo = p.count >= 2;"
            "    p.count = pts.length;"
            "    if (pts.length >= 2) {"
            "      const d = Math.hypot(pts[0][0] - pts[1][0], pts[0][1] - pts[1][1]);"
            "      if (!wasTwo) p.startDist = d;"
            "      p.dist = d; } };"
            "  const fingerDown = (e) => { if (e.pointerType === 'touch') { fingers[e.pointerId] = [e.clientX, e.clientY]; updatePinch(); } };"
            "  const fingerMove = (e) => { if (e.pointerId in fingers) { fingers[e.pointerId] = [e.clientX, e.clientY]; updatePinch(); } };"
            "  const fingerUp = (e) => { delete fingers[e.pointerId]; updatePinch(); };"
            "  window.helloImGuiTapZone = null;"
            "  document.addEventListener('touchend', (e) => {"
            "    const z = window.helloImGuiTapZone;"
            "    if (!z || !e.changedTouches.length) return;"
            "    const c = document.getElementById('canvas'); if (!c) return;"
            "    const r = c.getBoundingClientRect(), s = (z.displayW > 0 && r.width > 0) ? r.width / z.displayW : 1;"
            "    const x = e.changedTouches[0].clientX, y = e.changedTouches[0].clientY;"
            "    if (x >= r.left + z.x0 * s && x <= r.left + z.x1 * s && y >= r.top + z.y0 * s && y <= r.top + z.y1 * s)"
            "      window.open(z.url, '_blank'); }, options);"
            "  document.addEventListener('pointerdown', (e) => { onPointer(e); window.helloImGuiPointerDown = 1; fingerDown(e); }, options);"
            "  document.addEventListener('pointermove', (e) => { onPointer(e); fingerMove(e); }, options);"
            "  document.addEventListener('pointerup', (e) => { window.helloImGuiPointerDown = 0; fingerUp(e); }, options);"
            "  document.addEventListener('pointercancel', (e) => { window.helloImGuiPointerDown = 0; fingerUp(e); }, options);"
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

    void EmscriptenSetTapOpensUrl(ImVec2 rectMin, ImVec2 rectMax, const std::string& url, float displayWidth)
    {
        static std::string lastScript;
        std::string script;
        if (url.empty())
            script = "window.helloImGuiTapZone = null";
        else
        {
            std::string safeUrl;
            for (char c : url)  // a quote in a url: never valid there, dropped
                if (c != '\'' && c != '\\' && c != '\n')
                    safeUrl += c;
            char buffer[512];
            snprintf(buffer, sizeof(buffer), "window.helloImGuiTapZone = {x0: %.1f, y0: %.1f, x1: %.1f, y1: %.1f, displayW: %.1f, url: '",
                     rectMin.x, rectMin.y, rectMax.x, rectMax.y, displayWidth);
            script = std::string(buffer) + safeUrl + "'}";
        }
        if (script != lastScript)
        {
            lastScript = script;
            emscripten_run_script(script.c_str());
        }
    }

    namespace
    {
        struct TapZoneRequest { bool set = false; ImVec2 rectMin, rectMax; std::string url; };
        TapZoneRequest gTapZoneRequest;
    }

    void EmscriptenRequestTapZone(ImVec2 rectMin, ImVec2 rectMax, const std::string& url)
    {
        gTapZoneRequest = {true, rectMin, rectMax, url};
    }

    void EmscriptenPushTapZone(float displayWidth)
    {
        if (gTapZoneRequest.set)
            EmscriptenSetTapOpensUrl(gTapZoneRequest.rectMin, gTapZoneRequest.rectMax, gTapZoneRequest.url, displayWidth);
        else
            EmscriptenSetTapOpensUrl(ImVec2(), ImVec2(), "", displayWidth);
        gTapZoneRequest.set = false;
    }

    bool EmscriptenHasTouchScreen()
    {
        return emscripten_run_script_int("(navigator.maxTouchPoints > 0) ? 1 : 0") != 0;
    }
}
#endif
