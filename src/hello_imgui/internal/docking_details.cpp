#include "hello_imgui/internal/docking_details.h"
#include "imgui.h"
#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "hello_imgui/hello_imgui_theme.h"
#include "imgui_internal.h"
#include <map>

namespace HelloImGui
{
void _Themes_MenuGui(RunnerParams& runnerParams); // see hello_imgui_themes.cpp

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

void MenuItems_FontScale()
{
    float ratio = ImGui::GetIO().FontGlobalScale;
    char msgCurrentRatio[1000];
    snprintf(msgCurrentRatio, 1000, "Font Scale: %.1f", ratio);
    ImGui::MenuItem(msgCurrentRatio, nullptr, false, false);

    bool zoomChanged = false;
    if (ImGui::MenuItem( "Zoom ++"))
    {
        ratio = ratio * 1.2f * 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem( "Zoom +"))
    {
        ratio = ratio * 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem( "Zoom -"))
    {
        ratio = ratio / 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem(  "Zoom --"))
    {
        ratio = ratio / 1.2f / 1.2f;
        zoomChanged = true;
    }
    if (ImGui::MenuItem( "Restore Zoom"))
    {
        ratio = 1.f;
        zoomChanged = true;
    }

    if (zoomChanged)
        ImGui::GetIO().FontGlobalScale = ratio;

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
            if (dockableWindow.canBeClosed && dockableWindow.includeInViewMenu)
                dockableWindow.isVisible = true;
    if (ImGui::MenuItem("Hide All##DSQSDDF"))
        for (auto& dockableWindow: runnerParams.dockingParams.dockableWindows)
            if (dockableWindow.canBeClosed && dockableWindow.includeInViewMenu)
                dockableWindow.isVisible = false;

    for (auto& dockableWindow: runnerParams.dockingParams.dockableWindows)
    {
        if (!dockableWindow.includeInViewMenu)
            continue;
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
}


void ShowViewMenu(RunnerParams & runnerParams)
{
    (void)runnerParams;
    if (ImGui::BeginMenu("View##kdsflmkdflm"))
    {
        MenuView_DockableWindows(runnerParams);
        ImGui::Separator();
        Theme_MenuGui(runnerParams.imGuiWindowParams.tweakedTheme);
        ImGui::Separator();
        MenuItems_FontScale();
        ImGui::Separator();

        if (ImGui::MenuItem("View Status bar##xxxx", nullptr, runnerParams.imGuiWindowParams.showStatusBar))
            runnerParams.imGuiWindowParams.showStatusBar = ! runnerParams.imGuiWindowParams.showStatusBar;
        if (ImGui::MenuItem("FPS in status bar##xxxx", nullptr, runnerParams.imGuiWindowParams.showStatus_Fps))
            runnerParams.imGuiWindowParams.showStatus_Fps = ! runnerParams.imGuiWindowParams.showStatus_Fps;

        ImGui::EndMenu();
    }
}

void ShowDockableWindows(std::vector<DockableWindow>& dockableWindows)
{
    for (auto& dockableWindow: dockableWindows)
    {
        if (dockableWindow.focusWindowAtNextFrame)
            dockableWindow.isVisible = true;

        if (dockableWindow.isVisible)
        {
            if (dockableWindow.callBeginEnd)
            {
                if (dockableWindow.focusWindowAtNextFrame)
                {
                    ImGui::SetNextWindowFocus();
                    dockableWindow.focusWindowAtNextFrame = false;
                }
                if (dockableWindow.windowSize.x > 0.f)
                    ImGui::SetNextWindowSize(dockableWindow.windowSize, dockableWindow.windowSizeCondition);
                if (dockableWindow.windowPosition.x > 0.f)
                    ImGui::SetNextWindowPos(dockableWindow.windowPosition, dockableWindow.windowPositionCondition);
                bool not_collapsed = true;
                if (dockableWindow.canBeClosed)
                    not_collapsed = ImGui::Begin(dockableWindow.label.c_str(), &dockableWindow.isVisible, dockableWindow.imGuiWindowFlags);
                else
                    not_collapsed = ImGui::Begin(dockableWindow.label.c_str(), nullptr, dockableWindow.imGuiWindowFlags);
                if (not_collapsed && dockableWindow.GuiFunction)
                    dockableWindow.GuiFunction();
                ImGui::End();
            }
            else
            {
                dockableWindow.GuiFunction();
            }
        }
    }
}

void DoCreateFullScreenImGuiWindow(const ImGuiWindowParams& imGuiWindowParams, bool useDocking)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);

    ImVec2 viewportSize = viewport->Size;
    if (imGuiWindowParams.showStatusBar)
        viewportSize.y -= 30.f;
    ImGui::SetNextWindowSize(viewportSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    if (useDocking)
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
    std::string windowTitle = useDocking ? "MainDockSpace" : "Main window (title bar invisible)";
    ImGui::Begin(windowTitle.c_str(), &p_open, window_flags);
    ImGui::PopStyleVar(3);
}


void ImplProvideFullScreenImGuiWindow(const ImGuiWindowParams& imGuiWindowParams)
{
    DoCreateFullScreenImGuiWindow(imGuiWindowParams, false);
}

void ImplProvideFullScreenDockSpace(const ImGuiWindowParams& imGuiWindowParams)
{
    DoCreateFullScreenImGuiWindow(imGuiWindowParams, true);
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
    bool isFirstFrame = ImGui::GetFrameCount() <= 1;
    if (!dockingParams.wasDockLayoutApplied && !isFirstFrame)
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

DockableWindow * DockingParams::dockableWindowOfName(const std::string &name)
{
    for (auto & dockableWindow: dockableWindows)
        if (dockableWindow.label == name)
            return &dockableWindow;
    return nullptr;
}

void DockingParams::focusDockableWindow(const std::string& windowName)
{
    DockableWindow * win = dockableWindowOfName(windowName);
    IM_ASSERT(win != nullptr);
    win->focusWindowAtNextFrame = true;
}


}  // namespace HelloImGui