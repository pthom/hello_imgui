#include "hello_imgui/hello_imgui_logger.h"
#include "hello_imgui/internal/imguial_term.h"
#include "hello_imgui/hello_imgui.h"

namespace HelloImGui
{

namespace InternalLogBuffer
{
    static constexpr size_t gMaxBufferSize = 600000;
    char gLogBuffer_[gMaxBufferSize];
    ImGuiAl::Log gLog(gLogBuffer_, gMaxBufferSize);

}

void Log(LogLevel level, char const* const format, ...)
{
    va_list args;
    va_start(args, format);

    if (level == LogLevel::Debug)
        InternalLogBuffer::gLog.debug(format, args);
    else if (level == LogLevel::Info)
        InternalLogBuffer::gLog.info(format, args);
    else if (level == LogLevel::Warning)
        InternalLogBuffer::gLog.warning(format, args);
    else if (level == LogLevel::Error)
        InternalLogBuffer::gLog.error(format, args);
    else
        throw std::runtime_error("Log: bad LogLevel !");

    va_end(args);
}

void LogClear()
{
    InternalLogBuffer::gLog.clear();
}

void LogGui(ImVec2 size)
{
    InternalLogBuffer::gLog.draw(size);
}

}  // namespace HelloImGui

