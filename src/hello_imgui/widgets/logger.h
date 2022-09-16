#pragma once
#include "hello_imgui/docking_params.h"
#include "hello_imgui/internal/imguial_term.h"

// This header is deprecated! Prefer to use hello_imgui_logger.h!
namespace HelloImGui
{
namespace Widgets
{
    class Logger: public DockableWindow
    {
       public:

        Logger(
            std::string label_ = "Logs",
            DockSpaceName dockSpaceName_ = "LogsSpace");

        void debug(char const* const format, ...);
        void info(char const* const format, ...);
        void warning(char const* const format, ...);
        void error(char const* const format, ...);

        void clear();

       private:
        static constexpr size_t maxBufferSize = 300000;
        char logBuffer_[maxBufferSize];
        ImGuiAl::Log log_;
    };
}  // namespace Widgets
}  // namespace HelloImGui
