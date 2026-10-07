// HelloImGui::PrefersReducedMotion(): the system's request for less motion, per platform.
// (hello_imgui.h is not included: its declaration is repeated below, so that <windows.h> and its macros stay here)
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#elif defined(__APPLE__)
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"
#elif defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace HelloImGui
{
    bool PrefersReducedMotion();

    bool PrefersReducedMotion()
    {
#if defined(__EMSCRIPTEN__)
        // (emscripten_run_script_int rather than EM_ASM: it also works in a side module, as in Pyodide)
        return emscripten_run_script_int(
            "(window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches) ? 1 : 0") != 0;
#elif defined(__APPLE__)
        return ApplePrefersReducedMotion();  // "Reduce motion" (getAppleBundleResourcePath.mm)
#elif defined(_WIN32) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
        // The opposite of "Show animations in Windows" (Settings > Accessibility > Visual effects)
    #ifndef SPI_GETCLIENTAREAANIMATION  // declared for Windows Vista and later targets only
        const UINT SPI_GETCLIENTAREAANIMATION = 0x1042;
    #endif
        BOOL animations = TRUE;
        if (::SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &animations, 0))
            return !animations;
        return false;
#else
        return false;  // no such setting (Linux, Android, UWP)
#endif
    }
}
