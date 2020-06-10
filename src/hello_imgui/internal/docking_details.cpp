#include "hello_imgui/internal/docking_details.h"

namespace HelloImGui
{
namespace DockingDetails
{
ImGuiID MainDockSpaceId()
{
    static ImGuiID id = ImGui::GetID("MainDockSpace");
    return id;
}

void ImplProvideFullScreenImGuiWindow(const ImGuiWindowParams& imGuiWindowParams)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImVec2 winSize = ImGui::GetIO().DisplaySize;
    // winSize.y -= 10.f;
    ImGui::SetNextWindowSize(winSize);
    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (imGuiWindowParams.showMenuBar)
        windowFlags |= ImGuiWindowFlags_MenuBar;
    ImGui::Begin("Main window (title bar invisible)", nullptr, windowFlags);
}

void ImplProvideFullScreenDockSpace(const ImGuiWindowParams& imGuiWindowParams)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    if (imGuiWindowParams.showMenuBar)
        window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    static bool p_open = true;
    ImGui::Begin("MainDockSpace", &p_open, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = MainDockSpaceId();
    ImGuiDockNodeFlags dockspace_flags =
        ImGuiDockNodeFlags_PassthruCentralNode;  // ImGuiDockNodeFlags_PassthruDockspace;
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
}

void ConfigureImGuiDocking(const ImGuiWindowParams& imGuiWindowParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
        ImGui::GetIO().ConfigFlags = ImGui::GetIO().ConfigFlags | ImGuiConfigFlags_DockingEnable;

    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = imGuiWindowParams.configWindowsMoveFromTitleBarOnly;
}

void ProvideWindowOrDock(const ImGuiWindowParams& imGuiWindowParams, DockingParams &dockingParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenWindow)
        ImplProvideFullScreenImGuiWindow(imGuiWindowParams);

    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
    {
        if (!dockingParams.wasDockLayoutApplied)
        {
            if (dockingParams.InitialDockLayoutFunction)
                dockingParams.InitialDockLayoutFunction(MainDockSpaceId());
            dockingParams.wasDockLayoutApplied = true;
        }
        ImplProvideFullScreenDockSpace(imGuiWindowParams);
    }
}

void CloseWindowOrDock(ImGuiWindowParams& imGuiWindowParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType != DefaultImGuiWindowType ::NoDefaultWindow)
        ImGui::End();
}

}  // namespace DockingDetails

}  // namespace HelloImGui