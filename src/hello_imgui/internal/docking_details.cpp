#include "hello_imgui/internal/docking_details.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <map>

namespace HelloImGui
{

std::map<DockSpaceName, ImGuiID> gImGuiSplitIDs;

namespace DockingDetails
{

void DoSplit(const DockingSplit & dockingSplit)
{
    assert(gImGuiSplitIDs.find(dockingSplit.initialDock) != gImGuiSplitIDs.end());

    ImGuiID initalDock_imguiId = gImGuiSplitIDs.at(dockingSplit.initialDock);
    ImGuiID newDock_imguiId
        = ImGui::DockBuilderSplitNode(
            initalDock_imguiId,
            dockingSplit.direction,
            dockingSplit.ratio,
            nullptr,
            &initalDock_imguiId);

    gImGuiSplitIDs[dockingSplit.initialDock] = initalDock_imguiId;
    gImGuiSplitIDs[dockingSplit.newDock] = newDock_imguiId;
}

void ApplyDockingSplits(const std::vector<DockingSplit>& dockingSplits)
{
    for (const auto & dockingSplit: dockingSplits)
        DoSplit(dockingSplit);
}

void ApplyWindowDockingLocations(
    const std::vector<DockableWindow> & dockableWindows)
{
    for (const auto & dockableWindow: dockableWindows)
        ImGui::DockBuilderDockWindow(
            dockableWindow.label.c_str(),
            gImGuiSplitIDs[dockableWindow.dockSpaceName]
        );
}

void MenuView_DockableWindows(RunnerParams& runnerParams)
{
    auto & dockableWindows = runnerParams.dockingParams.dockableWindows;
    if (dockableWindows.empty())
        return;

    ImGui::MenuItem("Dockable windows##asldqsl", nullptr, false, false);

    if (ImGui::MenuItem("Restore default layout##szzz"))
    {
        runnerParams.dockingParams.resetUserDockLayout = true;
        runnerParams.dockingParams.wasDockLayoutApplied = false;
    }

    if (ImGui::MenuItem("View All##DSQSDDF"))
        for (auto& dockableWindow: runnerParams.dockingParams.dockableWindows)
            if (dockableWindow.canBeClosed)
                dockableWindow.isVisible = true;
    if (ImGui::MenuItem("Hide All##DSQSDDF"))
        for (auto& dockableWindow: runnerParams.dockingParams.dockableWindows)
            if (dockableWindow.canBeClosed)
                dockableWindow.isVisible = false;

    for (auto& dockableWindow: runnerParams.dockingParams.dockableWindows)
    {
        if (dockableWindow.canBeClosed)
        {
            if (ImGui::MenuItem(dockableWindow.label.c_str(), nullptr, dockableWindow.isVisible))
                dockableWindow.isVisible = ! dockableWindow.isVisible;
        }
        else
        {
            ImGui::MenuItem(dockableWindow.label.c_str(), nullptr, dockableWindow.isVisible, false);
        }
    }

    ImGui::Separator();
}

void ShowViewMenu(RunnerParams & runnerParams)
{
    (void)runnerParams;
    if (ImGui::BeginMenu("View##kdsflmkdflm"))
    {
        MenuView_DockableWindows(runnerParams);
        ImGui::Separator();

        if (ImGui::MenuItem("View Status bar##kwlmcldmdsl", nullptr, runnerParams.imGuiWindowParams.showStatusBar))
            runnerParams.imGuiWindowParams.showStatusBar = ! runnerParams.imGuiWindowParams.showStatusBar;

        ImGui::EndMenu();
    }
}

void ShowDockableWindows(std::vector<DockableWindow>& dockableWindows)
{
    for (auto& dockableWindow: dockableWindows)
    {
        if (dockableWindow.isVisible)
        {
            bool collapsed = false;
            if (dockableWindow.canBeClosed)
                collapsed = ImGui::Begin(dockableWindow.label.c_str(), &dockableWindow.isVisible);
            else
                collapsed = ImGui::Begin(dockableWindow.label.c_str());
            if (!collapsed)
                dockableWindow.guiFonction();
            ImGui::End();
        }
    }
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

    ImGuiID mainDockspaceId = ImGui::GetID("MainDockSpace");
    ImGuiDockNodeFlags dockspace_flags =
        ImGuiDockNodeFlags_PassthruCentralNode;  // ImGuiDockNodeFlags_PassthruDockspace;
    ImGui::DockSpace(mainDockspaceId, ImVec2(0.0f, 0.0f), dockspace_flags);

    gImGuiSplitIDs["MainDockSpace"] = mainDockspaceId;
}

void ConfigureImGuiDocking(const ImGuiWindowParams& imGuiWindowParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
        ImGui::GetIO().ConfigFlags = ImGui::GetIO().ConfigFlags | ImGuiConfigFlags_DockingEnable;

    ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = imGuiWindowParams.configWindowsMoveFromTitleBarOnly;
}

bool IsMainDockSpaceAlreadySplit(ImGuiID mainDockspaceId)
{
    auto *ctx = GImGui;
    ImGuiDockNode* node = (ImGuiDockNode*)ctx->DockContext.Nodes.GetVoidPtr(mainDockspaceId);
    bool result = node->IsSplitNode();
    return result;
}

void ApplyDockLayout(DockingParams& dockingParams)
{
    if (!dockingParams.wasDockLayoutApplied)
    {
        ImGuiID mainDockspaceId = ImGui::GetID("MainDockSpace");
        if (dockingParams.resetUserDockLayout)
            ImGui::DockBuilderRemoveNodeChildNodes(mainDockspaceId);
        if (!IsMainDockSpaceAlreadySplit(mainDockspaceId))
            ApplyDockingSplits(dockingParams.dockingSplits);
        ApplyWindowDockingLocations(dockingParams.dockableWindows);
        dockingParams.wasDockLayoutApplied = true;
    }
}
void ProvideWindowOrDock(const ImGuiWindowParams& imGuiWindowParams, DockingParams &dockingParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenWindow)
        ImplProvideFullScreenImGuiWindow(imGuiWindowParams);

    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
    {
        ImplProvideFullScreenDockSpace(imGuiWindowParams);
        ApplyDockLayout(dockingParams);
    }
}

void CloseWindowOrDock(ImGuiWindowParams& imGuiWindowParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType != DefaultImGuiWindowType ::NoDefaultWindow)
        ImGui::End();
}



}  // namespace DockingDetails

}  // namespace HelloImGui