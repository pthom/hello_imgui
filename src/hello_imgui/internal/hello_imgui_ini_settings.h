#pragma once

#include "hello_imgui/screen_bounds.h"

#include <string>
#include <optional>


namespace HelloImGui
{
    namespace HelloImGuiIniSettings
    {
        void WriteLastRunWindowBounds(const std::string& iniFilename, const ScreenBounds& windowBounds);
        std::optional<ScreenBounds> ReadLastRunWindowBounds(const std::string& iniFilename);
    }
}
