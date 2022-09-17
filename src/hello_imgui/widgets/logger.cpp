#include "hello_imgui/widgets/logger.h"

namespace HelloImGui
{
namespace Widgets
{
Logger::Logger(std::string label_, DockSpaceName dockSpaceName_)
    : DockableWindow(label_, dockSpaceName_, {})
    , log_(logBuffer_, maxBufferSize)
{
    this->GuiFunction = [this]() {
        log_.draw();
    };
}

void Logger::debug(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    log_.debug(format, args);
    va_end(args);
}
void Logger::info(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    log_.info(format, args);
    va_end(args);
}
void Logger::warning(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    log_.warning(format, args);
    va_end(args);
}
void Logger::error(char const* const format, ...)
{
    va_list args;
    va_start(args, format);
    log_.error(format, args);
    va_end(args);
}
void Logger::clear()
{
    log_.clear();
}

}  // namespace Widgets
}  // namespace HelloImGui

