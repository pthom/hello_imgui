#include "hello_imgui/internal/docking_details.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <map>

namespace HelloImGui
{

std::map<DockSpaceId, ImGuiID> gImGuiSplitIDs;

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
    const std::vector<std::pair<WindowId, DockSpaceId>> & windowDockingLocations)
{
    for (const auto & windowDockingLocation: windowDockingLocations)
        ImGui::DockBuilderDockWindow(
            windowDockingLocation.first.c_str(),
            gImGuiSplitIDs[windowDockingLocation.second]
        );
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

void ProvideWindowOrDock(const ImGuiWindowParams& imGuiWindowParams, DockingParams &dockingParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenWindow)
        ImplProvideFullScreenImGuiWindow(imGuiWindowParams);

    if (imGuiWindowParams.defaultImGuiWindowType == DefaultImGuiWindowType::ProvideFullScreenDockSpace)
    {
        ImplProvideFullScreenDockSpace(imGuiWindowParams);
        if (!dockingParams.wasDockLayoutApplied)
        {
            ImGuiID mainDockspaceId = ImGui::GetID("MainDockSpace");
            ImGui::DockBuilderRemoveNodeChildNodes(mainDockspaceId);
            ApplyDockingSplits(dockingParams.dockingSplits);
            ApplyWindowDockingLocations(dockingParams.windowDockingLocations);
            dockingParams.wasDockLayoutApplied = true;
        }
    }
}

void CloseWindowOrDock(ImGuiWindowParams& imGuiWindowParams)
{
    if (imGuiWindowParams.defaultImGuiWindowType != DefaultImGuiWindowType ::NoDefaultWindow)
        ImGui::End();
}


/// Handle docking splits


//ImGuiID DoSplit(ImGuiID* io_dockToSplit, ImGuiDir_ splitDirection, float ratio)
//{
//    ImGuiID new_dock
//        = ImGui::DockBuilderSplitNode(*io_dockToSplit, splitDirection, ratio, nullptr, io_dockToSplit);
//    return new_dock;
//}
//
//void BabylonStudioLayout::PrepareLayout(ImGuiID fullDockSpaceId)
//{
//    ImGui::DockBuilderRemoveNode(fullDockSpaceId); // Clear out existing layout
//    ImGui::DockBuilderAddNode(fullDockSpaceId);    // Add empty node
//    ImGui::DockBuilderSetNodeSize(fullDockSpaceId, ImGui::GetIO().DisplaySize);
//
//    ImGuiID dock_main_id = fullDockSpaceId;
//    ImGuiID dock_id_bottom = DoSplit(&dock_main_id, ImGuiDir_Down, 0.25f);
//    ImGuiID dock_id_left = DoSplit(&dock_main_id, ImGuiDir_Left, 0.33f);
//    ImGuiID dock_id_right = DoSplit(&dock_main_id, ImGuiDir_Right, 0.45f);
//
//#ifdef BABYLON_BUILD_PLAYGROUND
//    DoDock(DockableWindowId::PlaygroundEditor, dock_main_id);
//#endif
//    DoDock(DockableWindowId::SamplesCodeViewer, dock_main_id);
//
//    if (_layoutMode == LayoutMode::Dev) {
//        DoDock(DockableWindowId::Inspector, dock_main_id);
//        DoDock(DockableWindowId::SampleBrowser, dock_main_id);
//    }
//    else {
//        DoDock(DockableWindowId::Inspector, dock_id_left);
//        DoDock(DockableWindowId::SampleBrowser, dock_id_left);
//    }
//
//    DoDock(DockableWindowId::Scene3d, dock_id_right);
//    DoDock(DockableWindowId::Logs, dock_id_bottom);
//
//    ImGui::DockBuilderFinish(fullDockSpaceId);
//}



}  // namespace DockingDetails

}  // namespace HelloImGui