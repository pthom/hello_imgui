#pragma once
#include "imgui.h"

#include <functional>
#include <string>

namespace ImGuiRunner
{
/// AppWindowParams: defines the application window display params
struct AppWindowParams
{
    AppWindowParams(std::string windowTitle = "",
                    int windowWidth = 800,
                    int windowHeight = 600,
                    bool provideFullBackgroundWindow = true)
        : provide_full_background_window(provideFullBackgroundWindow),
          window_title(windowTitle),
          window_width(windowWidth),
          window_height(windowHeight)

    {
    }

    /// flag (true by default). If true, a default imgui window is provided
    bool provide_full_background_window = true;

    /// Background color
    ImVec4 clear_color = ImColor(114, 144, 154);

    /// title of the application window (empty by default)
    std::string window_title = "";

    /// width of the window (default 800)
    int window_width = 800;

    /// height of the window (default 600)
    int window_height = 600;

    /// If this boolean flag is true, the application window will occupy the full space of the primary screen
    bool show_maximized = false;

    /// If this boolean flag is true, the application window will be full screen, with no decorations.
    /// _Note: on a mobile device, the application will always be full screen._
    bool show_fullscreen = false;
};

/**
 * GuiLoopFunction is any void(void) function that can render ImGui widgets
 * (by calling ImGui::Button() and other ImGui widgets)
 *
 * ImGuiRunner::run() can call this type of function
 */
using GuiLoopFunction = std::function<void(void)>;

/**
 * GuiLoopFunction is any bool(void) function that can render ImGui widgets
 * and can return either false when the application shall continue,
 * or true when it shall exit.
 *
 * ImGuiRunner::run_with_quit() can call this type of function
 */
using GuiLoopWithExitFunction = std::function<bool(void)>;

/**
 * Use RunWithExit version if you want to be able to quit the app via code.
 * `RunWithExit` runs an application whose gui loop is provided by guiLoopFunction
 * and will exit when guiLoopFunction returns false.
 * @param guiLoopFunction a function of type GuiLoopWithExitFunction (aka bool(void))
 * that will render ImGui widgets and return true when the application shall exit.
 * @param appWindowParams application window params
 *
 \verbatim embed:rst
 .. note::

  `RunWithExit` is implemented differently depending on the backend. A Qt and a Glfw version are
  provided. You need to link with the correct version (imgui_runner_qt or imgui_runner_glfw)
 \endverbatim
 */
void RunWithExit(GuiLoopWithExitFunction guiLoopFunction, const AppWindowParams &params = AppWindowParams());

/**
 * Runs an application whose gui loop is provided by guiLoopFunction.
 * @param guiLoopFunction a function of type GuiLoopFunction (aka void(void)) that will render ImGui widgets
 * @param appWindowParams application window params
 */
void Run(GuiLoopFunction guiLoopFunction, const AppWindowParams &appWindowParams = AppWindowParams());

}  // namespace ImGuiRunner
