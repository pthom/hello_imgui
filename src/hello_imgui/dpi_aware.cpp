#include "hello_imgui/dpi_aware.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/abstract_runner.h"

namespace HelloImGui
{
    AbstractRunner *GetAbstractRunner();

    // High DPI handling on windows & linux
    // Part 1: store scale factor, so that font size is multiplied by this factor when loading
    // (this is for windows High DPI screen.     // cf https://github.com/pthom/imgui_bundle/issues/7 )
    //
    // Note: On Mac Retina screen, macOS "hides" the DPI, and we are using virtual pixels)
    //       => see macOS_BackingScaleFactor inside imgui_default_settings.cpp

    // Get the scaling factor for the main window. This is OS dependent!
    // - Under windows and linux, if the screen size if 3200x1600, but the display settings is scaled at 200%
    //   GetWindowDpiFactor() will return 2.f   (since the screen size if actually seen as 3200x1600 by the application)
    // -

    float DpiWindowFactor()
    {
        auto runner = GetAbstractRunner();
        IM_ASSERT(runner != nullptr);
        float dpiFactor = runner->DpiWindowFactor();
        return dpiFactor;
    }

    float DpiFontLoadingFactor()
    {
        return GetAbstractRunner()->DpiFontLoadingFactor();
    }

}