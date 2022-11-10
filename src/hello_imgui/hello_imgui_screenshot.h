#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>


namespace HelloImGui
{
/**
@@md#HelloImGui::AppWindowScreenshotRgbBuffer

* `AppWindowScreenshotRgbBuffer()` returns a screenshot of the app window (under the form of a RGB buffer)

* `FinalAppWindowScreenshotRgbBuffer()` returns a screenshot of the final screen of the last app window
  (this should be called after HelloImGui::Run() has ended)

@@md
*/
    struct ImageBuffer
    {
        std::size_t width = 0, height = 0;
        std::vector<uint8_t> bufferRgb;
    };

    ImageBuffer AppWindowScreenshotRgbBuffer();

    ImageBuffer FinalAppWindowScreenshotRgbBuffer();
}
