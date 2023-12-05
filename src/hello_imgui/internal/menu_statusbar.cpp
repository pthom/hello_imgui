#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/docking_details.h"

#include "imgui.h"
#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "hello_imgui/hello_imgui.h"
#include "imgui_internal.h"

#include <string>

#ifdef HELLOIMGUI_USE_GLFW
#include "GLFW/glfw3.h"
#endif

namespace HelloImGui
{
namespace Menu_StatusBar
{

void ShowDefaultAppMenu_Quit(RunnerParams & runnerParams)
{
    std::string menuAppTitle = runnerParams.imGuiWindowParams.menuAppTitle;
    if (menuAppTitle.empty())
        menuAppTitle = runnerParams.appWindowParams.windowTitle;
    if (menuAppTitle.empty())
        menuAppTitle = "App";

#ifdef HELLOIMGUI_CANNOTQUIT
    const bool displayQuitItem = false;
#else
    const bool displayQuitItem = runnerParams.imGuiWindowParams.showMenu_App_Quit;
#endif

    const bool hasCustomAppMenuItems = static_cast<bool>(runnerParams.callbacks.ShowAppMenuItems);

    const bool isAppMenuEmpty = !displayQuitItem && !hasCustomAppMenuItems;
    if (isAppMenuEmpty)
        return;

    if (ImGui::BeginMenu(menuAppTitle.c_str()))
    {
        if (runnerParams.callbacks.ShowAppMenuItems)
            runnerParams.callbacks.ShowAppMenuItems();

        if (displayQuitItem)
        {
            if (hasCustomAppMenuItems)
                ImGui::Separator();
            if (ImGui::MenuItem( "Quit"))
                runnerParams.appShallExit = true;
        }
        ImGui::EndMenu();
    }
}

void ShowMenu(RunnerParams & runnerParams, HelloImGui::BackendApi::WindowPointer window)
{
    bool hasMenu = (ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_MenuBar) != 0;
    if (!hasMenu)
        return;

    bool borderless = runnerParams.appWindowParams.borderless;
    ImGui::BeginMenuBar();
    if (borderless)
    {
        const float windowUpBorderHeight = 5.0f;
        const float windowCornerBorderWidth = 10.0f;
        ImVec2 drag_rect_size = ImGui::GetCurrentWindow()->MenuBarRect().GetSize();
        ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextItemAllowOverlap();
        ImGui::InvisibleButton("titleBarDragLeftCornerZone", ImVec2(windowCornerBorderWidth, windowUpBorderHeight));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        }
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 MouseDelta = ImGui::GetIO().MouseDelta;
            int winHeight = 0;
            int winWidth = 0;
            #ifdef HELLOIMGUI_USE_GLFW
                auto glfwWindow = (GLFWwindow *)(window);
                glfwGetWindowSize((GLFWwindow *)window, &winWidth, &winHeight);
                glfwSetWindowPos(glfwWindow, (int)(windowPos.x + MouseDelta.x), (int)(windowPos.y + MouseDelta.y));
                glfwSetWindowSize(glfwWindow, (int)(winWidth - MouseDelta.x), (int)(winHeight - MouseDelta.y));
            #endif
        }
        ImGui::SetCursorPos(ImVec2(windowCornerBorderWidth, 0.0f));
        ImGui::SetNextItemAllowOverlap();
        ImGui::InvisibleButton("titleBarDragUpZone", ImVec2(drag_rect_size.x - windowCornerBorderWidth, windowUpBorderHeight));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 MouseDelta = ImGui::GetIO().MouseDelta;
            int winHeight = 0;
            int winWidth = 0;
            #ifdef HELLOIMGUI_USE_GLFW
                auto glfwWindow = (GLFWwindow *)(window);
                glfwGetWindowSize((GLFWwindow *)window, &winWidth, &winHeight);
                glfwSetWindowPos(glfwWindow, (int)windowPos.x, (int)(windowPos.y + MouseDelta.y));
                glfwSetWindowSize(glfwWindow, winWidth, (int)(winHeight - MouseDelta.y));
            #endif
        }

        ImVec2 windowUpBorderPadding = ImVec2(0, windowUpBorderHeight);
        ImGui::SetNextItemAllowOverlap();
        ImGui::SetCursorPos(windowUpBorderPadding);
        ImGui::InvisibleButton("titleBarDragZone", drag_rect_size - windowUpBorderPadding);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        }
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 MouseDelta = ImGui::GetIO().MouseDelta;
            #ifdef HELLOIMGUI_USE_GLFW
                auto glfwWindow = (GLFWwindow *)(window);
                glfwSetWindowPos(glfwWindow, (int)(windowPos.x + MouseDelta.x), (int)(windowPos.y + + MouseDelta.y));
            #endif
        }
        ImGui::SetCursorPos(ImVec2(windowCornerBorderWidth, 0.0f));
    }

    if (runnerParams.imGuiWindowParams.showMenu_App)
        ShowDefaultAppMenu_Quit(runnerParams);

    if (runnerParams.imGuiWindowParams.showMenu_View)
        DockingDetails::ShowViewMenu(runnerParams);

    if (runnerParams.callbacks.ShowMenus)
        runnerParams.callbacks.ShowMenus();

    ImGui::EndMenuBar();
}


void ShowStatusBar(RunnerParams & params)
{
    float statusWindowHeight = ImGui::GetFrameHeight() * 1.4f;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusWindowHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, statusWindowHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking;
    ImGui::Begin("StatusBar", nullptr, windowFlags);

    if (params.callbacks.ShowStatus)
        params.callbacks.ShowStatus();

    if (params.imGuiWindowParams.showStatus_Fps)
    {
        float dy = ImGui::GetFontSize() * 0.15f;

        ImGui::SameLine(ImGui::GetIO().DisplaySize.x - 14.f * ImGui::GetFontSize());

        const char* idlingInfo = params.fpsIdling.isIdling ? " (Idling)" : "";

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - dy); // The checkbox seems visually misaligned, let's fix this
        ImGui::Checkbox("Enable idling", &params.fpsIdling.enableIdling);
        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - dy);
        ImGui::Text("FPS: %.1f%s", HelloImGui::FrameRate(), idlingInfo);
    }

    ImGui::End();
}


}  // namespace Menu_StatusBar
}  // namespace HelloImGui
