#include "hello_imgui/hello_imgui_screenshot.h"
#include "hello_imgui/internal/backend_impls/abstract_runner.h"


namespace HelloImGui
{
    AbstractRunner *GetAbstractRunner();

    ImageBuffer AppWindowScreenshotRgbBuffer()
    {
        auto r = GetAbstractRunner()->ScreenshotRgb();
        return r;
    }

    ImageBuffer gFinalAppWindowScreenshotRgbBuffer;

    void setFinalAppWindowScreenshotRgbBuffer(const ImageBuffer& b)
    {
        gFinalAppWindowScreenshotRgbBuffer = b;
    }

    ImageBuffer FinalAppWindowScreenshotRgbBuffer()
    {
        return gFinalAppWindowScreenshotRgbBuffer;
    }

}
