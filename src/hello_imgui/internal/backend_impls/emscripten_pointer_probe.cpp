#ifdef __EMSCRIPTEN__
#include "hello_imgui/internal/backend_impls/emscripten_pointer_probe.h"

#include <emscripten.h>

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

    bool EmscriptenHasTouchScreen()
    {
        return emscripten_run_script_int("(navigator.maxTouchPoints > 0) ? 1 : 0") != 0;
    }
}
#endif
