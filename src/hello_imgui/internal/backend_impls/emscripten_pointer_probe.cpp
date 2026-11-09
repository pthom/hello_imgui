#ifdef __EMSCRIPTEN__
#include "hello_imgui/internal/backend_impls/emscripten_pointer_probe.h"

#include <emscripten.h>

// emscripten_run_script rather than EM_JS or EM_ASM: it also works in a side module, as in Pyodide
namespace HelloImGui
{
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
}
#endif
