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
            "  const onPointer = (e) => {"
            "    window.helloImGuiPointerType = (e.pointerType === 'touch') ? 1 : (e.pointerType === 'pen') ? 2 : 0; };"
            "  document.addEventListener('pointerdown', onPointer, {capture: true, passive: true});"
            "  document.addEventListener('pointermove', onPointer, {capture: true, passive: true});"
            "}";
        emscripten_run_script(script);
    }

    ImGuiMouseSource LastEmscriptenPointerSource()
    {
        int type = emscripten_run_script_int("window.helloImGuiPointerType | 0");
        return (type == 1) ? ImGuiMouseSource_TouchScreen : (type == 2) ? ImGuiMouseSource_Pen : ImGuiMouseSource_Mouse;
    }

    bool EmscriptenHasTouchScreen()
    {
        return emscripten_run_script_int("(navigator.maxTouchPoints > 0) ? 1 : 0") != 0;
    }
}
#endif
