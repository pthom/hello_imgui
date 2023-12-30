#pragma once
#include "imgui.h"
/**
@@md#HelloImGui::Log

HelloImGui provides a simple Log utility that is able to collect message and display them with a specific widget.

* __HelloImGui::Log(LogLevel level, char const* const format, ... )__ will log a message (printf like format)
* __HelloImGui::LogClear()__ will clear the Log list
* __HelloImGui::LogGui()__ will display the Log widget

@@md
*/
namespace HelloImGui
{
    enum class LogLevel
    {
        Debug,
        Info,
        Warning,
        Error
    };

    void Log(LogLevel level, char const* const format, ...);
    void LogClear();
    void LogGui(ImVec2 size=ImVec2(0.f, 0.f));
}
