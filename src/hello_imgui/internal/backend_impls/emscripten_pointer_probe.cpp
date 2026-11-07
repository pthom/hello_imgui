#ifdef __EMSCRIPTEN__
#include "hello_imgui/internal/backend_impls/emscripten_pointer_probe.h"

#include <emscripten.h>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

// emscripten_run_script rather than EM_JS or EM_ASM: it also works in a side module, as in Pyodide
namespace HelloImGui
{
    void InstallEmscriptenTapZones()
    {
        const char* script =
            "if (window.helloImGuiTapZones === undefined) {"
            "  const options = {capture: true, passive: true};"
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
            "    if (inZone(p, start.zone)) { const w = window.open(start.zone.url, '_blank'); if (w) w.opener = null; } }, options);"
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

    namespace
    {
        struct TapZone { ImVec2 rectMin, rectMax; std::string url; };
        std::vector<TapZone> gTapZoneRequests;  // this frame's
    }

    void EmscriptenRequestTapZone(ImVec2 rectMin, ImVec2 rectMax, const std::string& url)
    {
        // Web addresses only: a javascript: url would run script in the page
        std::string lower = url;
        for (char& c : lower)
            c = (char)tolower((unsigned char)c);
        if (lower.rfind("http://", 0) != 0 && lower.rfind("https://", 0) != 0 && lower.rfind("mailto:", 0) != 0)
            return;
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
}
#endif
