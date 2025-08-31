#define IMGUI_DEFINE_MATH_OPERATORS
#include "hello_imgui/internal/docking_details.h"
#include "imgui.h"
#include "hello_imgui/internal/imgui_global_context.h" // must be included before imgui_internal.h
#include "hello_imgui/hello_imgui_theme.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/functional_utils.h"
#include "imgui_internal.h"
#include "nlohmann/json.hpp"
#include <map>
#include <vector>
#include <cassert>
#include <optional>


namespace HelloImGui
{
// From hello_imgui.cpp
bool ShouldRemoteDisplay();

namespace SplitIdsHelper
{
    std::map<DockSpaceName, ImGuiID> gImGuiSplitIDs;

    bool ContainsSplit(const DockSpaceName& dockSpaceName)
    {
        return gImGuiSplitIDs.find(dockSpaceName) != gImGuiSplitIDs.end();
    }

    ImGuiID GetSplitId(const DockSpaceName& dockSpaceName)
    {
        IM_ASSERT(ContainsSplit(dockSpaceName) && "GetSplitId: dockSpaceName not found in gImGuiSplitIDs");
        return gImGuiSplitIDs.at(dockSpaceName);
    }

    void SetSplitId(const DockSpaceName& dockSpaceName, ImGuiID imguiId)
    {
        gImGuiSplitIDs[dockSpaceName] = imguiId;
    }

    std::string SaveSplitIds()
    {
        // Serialize gImGuiSplitIDs using json
        nlohmann::json j;
        j["gImGuiSplitIDs"] = gImGuiSplitIDs;
        return j.dump();
    }

    void LoadSplitIds(const std::string& jsonStr)
    {
        // Deserialize gImGuiSplitIDs using json
        try
        {
            nlohmann::json j = nlohmann::json::parse(jsonStr);
            gImGuiSplitIDs = j.at("gImGuiSplitIDs").get<std::map<DockSpaceName, ImGuiID>>();
        }
        catch (const nlohmann::json::parse_error& e)
        {
            std::cerr << "LoadSplitIds: Failed to parse JSON: " << e.what() << std::endl;
        }
        catch (const nlohmann::json::type_error& e)
        {
            std::cerr << "LoadSplitIds: Type error during deserialization: " << e.what() << std::endl;
        }
        catch (const nlohmann::json::out_of_range& e)
        {
            std::cerr << "LoadSplitIds: Missing or incorrect key in JSON: " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << "LoadSplitIds: Unexpected error: " << e.what() << std::endl;
        }
    }
}


static bool gShowTweakWindow = false;

void ShowThemeTweakGuiWindow_Static()
{
    ShowThemeTweakGuiWindow(&gShowTweakWindow);
}

void MenuTheme()
{
    auto& tweakedTheme = HelloImGui::GetRunnerParams()->imGuiWindowParams.tweakedTheme;

    if (ImGui::BeginMenu("Theme"))
    {
        if (ImGui::MenuItem("Theme tweak window", nullptr, gShowTweakWindow))
            gShowTweakWindow = !gShowTweakWindow;
        ImGui::Separator();
        for (int i = 0; i < ImGuiTheme::ImGuiTheme_Count; ++i)
        {
            ImGuiTheme::ImGuiTheme_ theme = (ImGuiTheme::ImGuiTheme_)(i);
            bool selected = (theme == tweakedTheme.Theme);
            if (ImGui::MenuItem(ImGuiTheme::ImGuiTheme_Name(theme), nullptr, selected))
            {
                tweakedTheme.Theme = theme;
                ImGuiTheme::ApplyTheme(theme);
            }
        }
        ImGui::EndMenu();
    }
}


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
    IM_ASSERT(SplitIdsHelper::ContainsSplit(dockingSplit.initialDock) && "DoSplit: initialDock not found in gImGuiSplitIDs");

    ImGuiID initialDock_imguiId = SplitIdsHelper::GetSplitId(dockingSplit.initialDock);
    ImGuiID newDock_imguiId
        = ImGui::DockBuilderSplitNode(
            initialDock_imguiId,
            dockingSplit.direction,
            dockingSplit.ratio,
            nullptr,
            &initialDock_imguiId);

    SplitIdsHelper::SetSplitId(dockingSplit.initialDock, initialDock_imguiId);
    SplitIdsHelper::SetSplitId(dockingSplit.newDock, newDock_imguiId);

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
    {
        ImGui::DockBuilderDockWindow(
            dockableWindow.label.c_str(),
            SplitIdsHelper::GetSplitId(dockableWindow.dockSpaceName)
        );
    }
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
        ImGui::SeparatorText("Layouts");

    if (! runnerParams.dockingParams.dockableWindows.empty())
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

    ImGui::PopID();
}

void MenuView_DockableWindows(RunnerParams& runnerParams)
{
    auto & dockableWindows = runnerParams.dockingParams.dockableWindows;
    if (dockableWindows.empty())
        return;

    ImGui::PushID("DockableWindows##asldqsl");

    ImGui::SeparatorText("Windows");


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

    ImGui::PopID();
}

void MenuView_Misc(RunnerParams& runnerParams)
{
    ImGui::SeparatorText("Misc");

    if (ImGui::MenuItem("View Status bar##xxxx", nullptr, runnerParams.imGuiWindowParams.showStatusBar))
        runnerParams.imGuiWindowParams.showStatusBar = ! runnerParams.imGuiWindowParams.showStatusBar;

	if (ImGui::BeginMenu("FPS"))
	{
		if (ImGui::MenuItem("FPS in status bar##xxxx", nullptr, runnerParams.imGuiWindowParams.showStatus_Fps))
			runnerParams.imGuiWindowParams.showStatus_Fps = ! runnerParams.imGuiWindowParams.showStatus_Fps;

		if (!ShouldRemoteDisplay())
			ImGui::MenuItem("Enable Idling", nullptr, &runnerParams.fpsIdling.enableIdling);
		ImGui::EndMenu();
	}

    if (runnerParams.imGuiWindowParams.showMenu_View_Themes)
        MenuTheme();
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

ImRect FullScreenRect_MinusInsets(const RunnerParams& runnerParams)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImGuiWindowParams& imGuiWindowParams = runnerParams.imGuiWindowParams;

    ImVec2 fullScreenSize, fullScreenPos;
    {
        // One some platform, like iOS, we need to take into account the insets
        // so that our app does not go under the notch or the home indicator
        EdgeInsets edgeInsets;
        if (runnerParams.appWindowParams.handleEdgeInsets)
            edgeInsets = runnerParams.appWindowParams.edgeInsets;
        fullScreenPos = viewport->Pos;
        fullScreenPos.x += (float)edgeInsets.left;
        fullScreenPos.y += (float)edgeInsets.top;
        fullScreenSize = viewport->Size;
        fullScreenSize.x -= (float)edgeInsets.left + (float)edgeInsets.right;
        fullScreenSize.y -= (float)edgeInsets.top + (float)edgeInsets.bottom;
        if (imGuiWindowParams.showStatusBar)
            fullScreenSize.y -= ImGui::GetFrameHeight() * 1.35f;
    }

    // Take fullScreenWindow_MarginTopLeft and fullScreenWindow_MarginBottomRight into account
    {
        fullScreenPos += HelloImGui::EmToVec2(imGuiWindowParams.fullScreenWindow_MarginTopLeft);
        fullScreenSize -= HelloImGui::EmToVec2(
            imGuiWindowParams.fullScreenWindow_MarginTopLeft + imGuiWindowParams.fullScreenWindow_MarginBottomRight
        );
    }

    ImRect r(fullScreenPos, fullScreenPos + fullScreenSize);
    return r;
}


// This function returns many different positions:
// - position of the main dock space (if edgeToolbarTypeOpt==nullopt)
// - position of an edge toolbar (if edgeToolbarTypeOpt!=nullopt)
ImRect FixedWindowRect(
    const RunnerParams& runnerParams,
    std::optional<EdgeToolbarType> edgeToolbarTypeOpt)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImGuiWindowParams& imGuiWindowParams = runnerParams.imGuiWindowParams;

    ImRect fullScreenRectMinusInsets = FullScreenRect_MinusInsets(runnerParams);
    ImVec2 fullScreenPos = fullScreenRectMinusInsets.Min;
    ImVec2 fullScreenSize = fullScreenRectMinusInsets.GetSize();

    // EdgeToolbar positions
    if (edgeToolbarTypeOpt.has_value())
    {
        auto edgeToolbarType = *edgeToolbarTypeOpt;
        auto& edgesToolbarsMap = runnerParams.callbacks.edgesToolbars;
        if (edgesToolbarsMap.find(edgeToolbarType) != edgesToolbarsMap.end())
        {
            auto& edgeToolbar = edgesToolbarsMap.at(edgeToolbarType);
            if (edgeToolbar.ShowToolbar)
            {
                if ( edgeToolbarType == EdgeToolbarType::Top)
                {
                    if (imGuiWindowParams.showMenuBar)
                    {
                        float menuHeight = ImGui::GetFrameHeight() * 1.f;
                        fullScreenPos.y += menuHeight;
                    }
                    fullScreenSize.y = HelloImGui::EmSize(edgeToolbar.options.sizeEm);
                }
                if ( edgeToolbarType == EdgeToolbarType::Bottom)
                {
                    float height = HelloImGui::EmSize(edgeToolbar.options.sizeEm);
                    fullScreenPos.y  = fullScreenPos.y + fullScreenSize.y - height - 1.f; // -1 to avoid a thin line
                    fullScreenSize.y = height;
                }
                if ( (edgeToolbarType == EdgeToolbarType::Left) || (edgeToolbarType == EdgeToolbarType::Right))
                {
                    float width = HelloImGui::EmSize(edgeToolbar.options.sizeEm);
                    if (imGuiWindowParams.showMenuBar)
                    {
                        float menuHeight = ImGui::GetFrameHeight() * 1.f;
                        fullScreenPos.y += menuHeight;
                        fullScreenSize.y -= menuHeight;
                    }
                    if (runnerParams.callbacks.edgesToolbars.find(EdgeToolbarType::Top) != runnerParams.callbacks.edgesToolbars.end())
                    {
                        auto height = HelloImGui::EmSize(
                            runnerParams.callbacks.edgesToolbars.at(EdgeToolbarType::Top).options.sizeEm);
                        fullScreenPos.y += height;
                        fullScreenSize.y -= height - 1.f; // -1 to avoid a thin line between the left and bottom toolbar
                    }
                    if (runnerParams.callbacks.edgesToolbars.find(EdgeToolbarType::Bottom) != runnerParams.callbacks.edgesToolbars.end())
                    {
                        auto height = HelloImGui::EmSize(
                            runnerParams.callbacks.edgesToolbars.at(EdgeToolbarType::Bottom).options.sizeEm);
                        fullScreenSize.y -= height - 1.f; // -1 to avoid a thin line between the left and bottom toolbar
                    }
                }
                if ( edgeToolbarType == EdgeToolbarType::Left)
                {
                    auto width = HelloImGui::EmSize(
                        runnerParams.callbacks.edgesToolbars.at(EdgeToolbarType::Left).options.sizeEm);
                    fullScreenSize.x = width;
                }
                if ( edgeToolbarType == EdgeToolbarType::Right)
                {
                    auto width = HelloImGui::EmSize(
                        runnerParams.callbacks.edgesToolbars.at(EdgeToolbarType::Right).options.sizeEm);
                    fullScreenPos.x = fullScreenPos.x + fullScreenSize.x - width;
                    fullScreenSize.x = width + 1.f; // + 1 to avoid a thin line
                }
            }
        }
    }

    // Update full screen window: take toolbars into account
    if (! edgeToolbarTypeOpt.has_value())
    {
        if (imGuiWindowParams.showMenuBar)
        {
            float menuHeight = ImGui::GetFrameHeight() * 1.f;
            fullScreenPos.y += menuHeight;
            fullScreenSize.y -= menuHeight;
        }

        auto& edgesToolbarsMap = runnerParams.callbacks.edgesToolbars;

        for(auto edgeToolbarType: HelloImGui::AllEdgeToolbarTypes())
        {
            if (edgesToolbarsMap.find(edgeToolbarType) != edgesToolbarsMap.end())
            {
                auto& edgeToolbar = edgesToolbarsMap.at(edgeToolbarType);
                if (edgeToolbar.ShowToolbar)
                {
                    if (edgeToolbarType == EdgeToolbarType::Top)
                    {
                        float height = HelloImGui::EmSize(edgeToolbar.options.sizeEm);
                        fullScreenPos.y += height;
                        fullScreenSize.y -= height;
                    }
                    if (edgeToolbarType == EdgeToolbarType::Bottom)
                    {
                        float height = HelloImGui::EmSize(edgeToolbar.options.sizeEm);
                        fullScreenSize.y -= height;
                    }
                    if (edgeToolbarType == EdgeToolbarType::Left)
                    {
                        float width = HelloImGui::EmSize(edgeToolbar.options.sizeEm);
                        fullScreenPos.x += width;
                        fullScreenSize.x -= width;
                    }
                    if (edgeToolbarType == EdgeToolbarType::Right)
                    {
                        float width = HelloImGui::EmSize(edgeToolbar.options.sizeEm);
                        fullScreenSize.x -= width;
                    }
                }
            }
        }
    }

    ImRect r(fullScreenPos, fullScreenPos + fullScreenSize);
    return r;
}

static ImGuiWindowFlags WindowFlagsNothing()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    return window_flags;
}

void DoShowToolbar(
    ImRect positionRect,
    VoidFunction toolbarFunction,
    const std::string& windowId,
    ImVec2 windowPaddingEm,
    ImVec4 windowBg
    )
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(positionRect.Min);
    ImGui::SetNextWindowSize(positionRect.GetSize());
    ImGui::SetNextWindowViewport(viewport->ID);


    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, HelloImGui::EmToVec2(windowPaddingEm));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.f, 0.f));
    if (windowBg.w != 0.f)
        ImGui::PushStyleColor(ImGuiCol_WindowBg, windowBg);
    static bool p_open = true;
    ImGui::Begin(windowId.c_str(), &p_open, WindowFlagsNothing() | ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar(4);
    if (windowBg.w != 0.f)
        ImGui::PopStyleColor();
    toolbarFunction();
    ImGui::End();
}


void ShowToolbars(const RunnerParams& runnerParams)
{
    for (auto edgeToolbarType: HelloImGui::AllEdgeToolbarTypes())
    {
        if (runnerParams.callbacks.edgesToolbars.find(edgeToolbarType) != runnerParams.callbacks.edgesToolbars.end())
        {
            auto& edgeToolbar = runnerParams.callbacks.edgesToolbars.at(edgeToolbarType);
            auto fullScreenRect = FixedWindowRect(runnerParams, edgeToolbarType);
            std::string windowName = std::string("##") + HelloImGui::EdgeToolbarTypeName(edgeToolbarType) + "_2123243";
            DoShowToolbar(fullScreenRect, edgeToolbar.ShowToolbar, windowName, edgeToolbar.options.WindowPaddingEm, edgeToolbar.options.WindowBg);
        }
    }
}

void DoCreateFullScreenImGuiWindow(const RunnerParams& runnerParams, bool useDocking)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImRect fullScreenRect = FixedWindowRect(runnerParams, std::nullopt);

    ImGui::SetNextWindowPos(fullScreenRect.Min);
    ImGui::SetNextWindowSize(fullScreenRect.GetSize());
    ImGui::SetNextWindowViewport(viewport->ID);
    if (useDocking)
        ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    if (useDocking)
    {
        // When using docking, we want the window to have no padding, so that docked windows
        // stick to the edges of the app window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }

    static bool p_open = true;
    std::string windowTitle = useDocking ? "MainDockSpace" : "Main window (title bar invisible)";
    ImGui::Begin(windowTitle.c_str(), &p_open, WindowFlagsNothing());

    if (useDocking)
        ImGui::PopStyleVar(3);
    else
        ImGui::PopStyleVar(2);
}


void ImplProvideFullScreenImGuiWindow(const RunnerParams& runnerParams)
{
    DoCreateFullScreenImGuiWindow(runnerParams, false);
}

void ImplProvideFullScreenDockSpace(const RunnerParams& runnerParams)
{
    DoCreateFullScreenImGuiWindow(runnerParams, true);
    ImGuiID mainDockspaceId = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(mainDockspaceId, ImVec2(0.0f, 0.0f), runnerParams.dockingParams.mainDockSpaceNodeFlags);
    SplitIdsHelper::SetSplitId("MainDockSpace", mainDockspaceId);
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
        ImplProvideFullScreenImGuiWindow(runnerParams);

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
    if (SplitIdsHelper::ContainsSplit(dockSpaceName))
        return SplitIdsHelper::GetSplitId(dockSpaceName);
    else
        return std::nullopt;
}


// `AddDockableWindow()` implementation and helper
namespace AddDockableWindowHelper
{
    // Adding dockable windows is a three-step process:
    // - First, the user calls `AddDockableWindow()`: the dockable window is added to gDockableWindowsToAdd
    //   with the state `DockableWindowAdditionState::Waiting`
    // - Then, in the first callback, the dockable window is added to ImGui as a dummy window:
    //   we call `ImGui::Begin()` and `ImGui::End()` to create the window, but we don't draw anything in it,
    //   then we call `ImGui::DockBuilderDockWindow()` to dock the window to the correct dockspace
    // - Finally, in the second callback, the dockable window is added to HelloImGui::RunnerParams.dockingParams.dockableWindows

    enum class DockableWindowAdditionState
    {
        Waiting,
        AddedAsDummyToImGui,
        AddedToHelloImGui
    };

    struct DockableWindowWaitingForAddition
    {
        DockableWindow dockableWindow;
        DockableWindowAdditionState state = DockableWindowAdditionState::Waiting;
        bool forceDockspace;
    };

    std::vector<DockableWindowWaitingForAddition> gDockableWindowsToAdd;
    std::vector<std::string> gDockableWindowsToRemove;

    void AddDockableWindow(const DockableWindow& dockableWindow, bool forceDockspace)
    {
        gDockableWindowsToAdd.push_back({dockableWindow, DockableWindowAdditionState::Waiting, forceDockspace});
    }

    void Callback_1_GuiRender()
    {
        for (auto & dockableWindow: gDockableWindowsToAdd)
        {
            if (dockableWindow.state == DockableWindowAdditionState::Waiting)
            {
                bool doesWindowHavePreviousSetting;
                {
                    ImGuiID window_id = ImHashStr(dockableWindow.dockableWindow.label.c_str());
                    ImGuiWindowSettings* previousWindowSettings = ImGui::FindWindowSettingsByID(window_id);
                    doesWindowHavePreviousSetting = (previousWindowSettings != nullptr);
                }
                if (!doesWindowHavePreviousSetting || dockableWindow.forceDockspace)
                {
                    auto dockSpaceName = dockableWindow.dockableWindow.dockSpaceName;
                    if (!dockSpaceName.empty())
                    {
                        auto dockId = HelloImGui::GetRunnerParams()->dockingParams.dockSpaceIdFromName(dockSpaceName);
                        if (dockId.has_value())
                        {
                            ImGui::Begin(dockableWindow.dockableWindow.label.c_str());
                            //ImGui::Dummy(ImVec2(10, 10));
                            dockableWindow.dockableWindow.GuiFunction();
                            ImGui::End();

                            ImGui::DockBuilderDockWindow(dockableWindow.dockableWindow.label.c_str(), dockId.value());
                        }
                        else
                        {
                            fprintf(stderr, "DockableWindow %s: dockSpaceName %s not found\n", dockableWindow.dockableWindow.label.c_str(), dockableWindow.dockableWindow.dockSpaceName.c_str());
                        }
                    }
                }

                dockableWindow.state = DockableWindowAdditionState::AddedAsDummyToImGui;
            }
        }
    }

    void Callback_2_PreNewFrame()
    {
        // Add the dockable windows that have been added as dummy to ImGui to HelloImGui
        for (auto & dockableWindow: gDockableWindowsToAdd)
        {
            if (dockableWindow.state == DockableWindowAdditionState::AddedAsDummyToImGui)
            {
                HelloImGui::GetRunnerParams()->dockingParams.dockableWindows.push_back(dockableWindow.dockableWindow);
                dockableWindow.state = DockableWindowAdditionState::AddedToHelloImGui;
            }
        }

        // Forget about the dockable windows that have been added to HelloImGui
        gDockableWindowsToAdd.erase(  // typical C++ shenanigans
            std::remove_if(
                gDockableWindowsToAdd.begin(),
                gDockableWindowsToAdd.end(),
                [](const DockableWindowWaitingForAddition& dockableWindow) {
                    return dockableWindow.state == DockableWindowAdditionState::AddedToHelloImGui;
                }
            ),
            gDockableWindowsToAdd.end()
        );

        // Remove the dockable windows that have been requested to be removed
        auto& dockableWindows = HelloImGui::GetRunnerParams()->dockingParams.dockableWindows;
        for (const auto& dockableWindowName: gDockableWindowsToRemove)
        {
            dockableWindows.erase(
                std::remove_if(
                    dockableWindows.begin(),
                    dockableWindows.end(),
                    [&dockableWindowName](const DockableWindow& dockableWindow) {
                        return dockableWindow.label == dockableWindowName;
                    }
                ),
                dockableWindows.end()
            );
        }
        gDockableWindowsToRemove.clear();
    }

} // namespace AddDockableWindowHelper


void AddDockableWindow(const DockableWindow& dockableWindow, bool forceDockspace)
{
    AddDockableWindowHelper::AddDockableWindow(dockableWindow, forceDockspace);
}

void RemoveDockableWindow(const std::string& dockableWindowName)
{
    AddDockableWindowHelper::gDockableWindowsToRemove.push_back(dockableWindowName);
}


}  // namespace HelloImGui
