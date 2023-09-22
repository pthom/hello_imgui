#include "hello_imgui/internal/docking_details.h"
#include "imgui.h"
#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "hello_imgui/hello_imgui_theme.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/functional_utils.h"
#include "imgui_internal.h"
#include <map>
#include <cassert>


namespace HelloImGui
{
void _Themes_MenuGui(RunnerParams& runnerParams); // see hello_imgui_themes.cpp

std::map<DockSpaceName, ImGuiID> gImGuiSplitIDs;

namespace DockingDetails
{
bool _makeImGuiWindowTabVisible(const std::string& windowName)
{
    ImGuiWindow* window = ImGui::FindWindowByName(windowName.c_str());
    if (window == NULL || window->DockNode == NULL || window->DockNode->TabBar == NULL)
        return false;
    window->DockNode->TabBar->NextSelectedTabId = window->TabId;
    return true;
}

void DoSplit(const DockingSplit & dockingSplit)
{
    IM_ASSERT(gImGuiSplitIDs.find(dockingSplit.initialDock) != gImGuiSplitIDs.end());

    ImGuiID initialDock_imguiId = gImGuiSplitIDs.at(dockingSplit.initialDock);
    ImGuiID newDock_imguiId
        = ImGui::DockBuilderSplitNode(
            initialDock_imguiId,
            dockingSplit.direction,
            dockingSplit.ratio,
            nullptr,
            &initialDock_imguiId);

    gImGuiSplitIDs[dockingSplit.initialDock] = initialDock_imguiId;
    gImGuiSplitIDs[dockingSplit.newDock] = newDock_imguiId;

    // apply flags
    ImGuiDockNode* newDockNode = ImGui::DockBuilderGetNode(newDock_imguiId);
    newDockNode->SetLocalFlags(dockingSplit.nodeFlags);
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

std::vector<std::string> _GetStaticallyOrderedLayoutsList(const RunnerParams& runnerParams)
{
    static std::vector<std::string> staticallyOrderedLayoutNames;

    // First fill the static vector with currently available layouts
    if (! FunctionalUtils::vector_contains(staticallyOrderedLayoutNames, runnerParams.dockingParams.layoutName))
        staticallyOrderedLayoutNames.push_back(runnerParams.dockingParams.layoutName);
    for (const auto& layout: runnerParams.alternativeDockingLayouts)
        if (! FunctionalUtils::vector_contains(staticallyOrderedLayoutNames, layout.layoutName))
            staticallyOrderedLayoutNames.push_back(layout.layoutName);

    // Then, fill currently available layouts
    std::vector<std::string> currentLayoutNames;
    currentLayoutNames.push_back(runnerParams.dockingParams.layoutName);
    for (const auto& layout: runnerParams.alternativeDockingLayouts)
        currentLayoutNames.push_back(layout.layoutName);

    // Only display currently available layout, but with the static order
    std::vector<std::string> layoutNames;
    for (const auto& staticalLayoutName: staticallyOrderedLayoutNames)
    {
        if (FunctionalUtils::vector_contains(currentLayoutNames, staticalLayoutName))
            layoutNames.push_back(staticalLayoutName);
    }

    return layoutNames;
}

void MenuView_Layouts(RunnerParams& runnerParams)
{
    bool hasAlternativeDockingLayouts = (runnerParams.alternativeDockingLayouts.size() > 0);

    if (hasAlternativeDockingLayouts)
        ImGui::MenuItem("------ Layouts ------", nullptr, false, false);

    if (ImGui::MenuItem("Restore default layout##szzz"))
        runnerParams.dockingParams.layoutReset = true;

    ImGui::PushID("Layouts##asldqsl");

    if (hasAlternativeDockingLayouts)
    {
        if (ImGui::BeginMenu("Select Layout"))
        {
            auto layoutNames = _GetStaticallyOrderedLayoutsList(runnerParams);
            for (const auto& layoutName: layoutNames)
            {
                bool isSelected = (layoutName == runnerParams.dockingParams.layoutName);
                if (ImGui::MenuItem(layoutName.c_str(), nullptr, isSelected))
                    HelloImGui::SwitchLayout(layoutName);
            }
            ImGui::EndMenu();
        }
    }
    ImGui::Separator();

    ImGui::PopID();
}

void MenuView_DockableWindows(RunnerParams& runnerParams)
{
    auto & dockableWindows = runnerParams.dockingParams.dockableWindows;
    if (dockableWindows.empty())
        return;

    ImGui::PushID("DockableWindows##asldqsl");

    ImGui::MenuItem("------ Windows ------", nullptr, false, false);


    if (ImGui::MenuItem("View All##DSQSDDF"))
        for (auto& dockableWindow: runnerParams.dockingParams.dockableWindows)
            if (dockableWindow.canBeClosed && dockableWindow.includeInViewMenu)
                dockableWindow.isVisible = true;
    if (ImGui::MenuItem("Hide All##DSQSDDF"))
        for (auto& dockableWindow: runnerParams.dockingParams.dockableWindows)
            if (dockableWindow.canBeClosed && dockableWindow.includeInViewMenu)
                dockableWindow.isVisible = false;

    {
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

    ImGui::Separator();

    ImGui::PopID();
}

void MenuView_Misc(RunnerParams& runnerParams)
{
    ImGui::MenuItem("------ Misc ------", nullptr, false, false);

    if (ImGui::MenuItem("View Status bar##xxxx", nullptr, runnerParams.imGuiWindowParams.showStatusBar))
        runnerParams.imGuiWindowParams.showStatusBar = ! runnerParams.imGuiWindowParams.showStatusBar;

    if (ImGui::BeginMenu("FPS"))
    {
        if (ImGui::MenuItem("FPS in status bar##xxxx", nullptr, runnerParams.imGuiWindowParams.showStatus_Fps))
            runnerParams.imGuiWindowParams.showStatus_Fps = ! runnerParams.imGuiWindowParams.showStatus_Fps;

        ImGui::MenuItem("Enable Idling", nullptr, &runnerParams.fpsIdling.enableIdling);
        ImGui::EndMenu();
    }

    if (runnerParams.imGuiWindowParams.showMenu_View_Themes)
        Theme_MenuGui(runnerParams.imGuiWindowParams.tweakedTheme);
}

void ShowViewMenu(RunnerParams & runnerParams)
{
    if (ImGui::BeginMenu("View##kdsflmkdflm"))
    {
        MenuView_Layouts(runnerParams);
        MenuView_DockableWindows(runnerParams);
        MenuView_Misc(runnerParams);

        ImGui::EndMenu();
    }
}

void ShowDockableWindows(std::vector<DockableWindow>& dockableWindows)
{
    bool wereAllDockableWindowsInited = (ImGui::GetFrameCount() > 1);

    for (auto& dockableWindow: dockableWindows)
    {
        bool shallFocusWindow = dockableWindow.focusWindowAtNextFrame && wereAllDockableWindowsInited;

        if (shallFocusWindow)
            dockableWindow.isVisible = true;

        if (dockableWindow.isVisible)
        {
            if (dockableWindow.callBeginEnd)
            {
                if (shallFocusWindow)
                    ImGui::SetNextWindowFocus();
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

                if (shallFocusWindow)
                    DockingDetails::_makeImGuiWindowTabVisible(dockableWindow.label);

                if (shallFocusWindow)
                    dockableWindow.focusWindowAtNextFrame = false;
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
        viewportSize.y -= ImGui::GetFrameHeight() * 1.35f;

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

void ImplProvideFullScreenDockSpace(const RunnerParams& runnerParams)
{
    DoCreateFullScreenImGuiWindow(runnerParams.imGuiWindowParams, true);
    ImGuiID mainDockspaceId = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(mainDockspaceId, ImVec2(0.0f, 0.0f), runnerParams.dockingParams.mainDockSpaceNodeFlags);
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
    if (isFirstFrame)
        return;
    if (dockingParams.layoutReset)
    {
        ImGuiID mainDockspaceId = ImGui::GetID("MainDockSpace");
        ImGui::DockBuilderRemoveNodeChildNodes(mainDockspaceId);
        //if (!IsMainDockSpaceAlreadySplit(mainDockspaceId))
            ApplyDockingSplits(dockingParams.dockingSplits);
        ApplyWindowDockingLocations(dockingParams.dockableWindows);
        dockingParams.layoutReset = false;
    }
}

void ProvideWindowOrDock(RunnerParams& runnerParams)
{
    if (runnerParams.imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenWindow)
        ImplProvideFullScreenImGuiWindow(runnerParams.imGuiWindowParams);

    if (runnerParams.imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
    {
        ImplProvideFullScreenDockSpace(runnerParams);
        ApplyDockLayout(runnerParams.dockingParams);
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

bool DockingParams::focusDockableWindow(const std::string& windowName)
{
    DockableWindow * win = dockableWindowOfName(windowName);
    if (win != nullptr)
    {
        win->focusWindowAtNextFrame = true;
        return true;
    }
    else
        return false;
}


std::optional<ImGuiID> DockingParams::dockSpaceIdFromName(const std::string& dockSpaceName)
{
    if (gImGuiSplitIDs.find(dockSpaceName) == gImGuiSplitIDs.end())
        return std::nullopt;
    else
        return gImGuiSplitIDs.at(dockSpaceName);
}


}  // namespace HelloImGui