#include "hello_imgui/imgui_window_params.h"

namespace HelloImGui
{
void ImGuiWindowParams::ResetDockLayout()
{
    WasDockLayoutApplied = false;
}

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
        if (imGuiWindowParams.ShowMenuBar)
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
        if (imGuiWindowParams.ShowMenuBar)
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

    bool WasDockLayoutDone(const ImGuiWindowParams& p) { return p.WasDockLayoutApplied; }
    void SetDockLayout_Done(ImGuiWindowParams& p) { p.WasDockLayoutApplied = true; }
    void SetDockLayout_NotDone(ImGuiWindowParams& p)
    {
        remove("imgui.ini");
        p.WasDockLayoutApplied = false;
    }

    void ConfigureImGuiDocking(const ImGuiWindowParams& imGuiWindowParams)
    {
        if (imGuiWindowParams.DefaultWindowType ==
            ImGuiWindowParams::ImGuiDefaultWindowType::ProvideFullScreenDockSpace)
            ImGui::GetIO().ConfigFlags = ImGui::GetIO().ConfigFlags | ImGuiConfigFlags_DockingEnable;

        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = imGuiWindowParams.ConfigWindowsMoveFromTitleBarOnly;
    }

    void ProvideWindowOrDock(ImGuiWindowParams& imGuiWindowParams)
    {
        if (imGuiWindowParams.DefaultWindowType == ImGuiWindowParams::ImGuiDefaultWindowType::ProvideFullScreenWindow)
            ImplProvideFullScreenImGuiWindow(imGuiWindowParams);

        if (imGuiWindowParams.DefaultWindowType ==
            ImGuiWindowParams::ImGuiDefaultWindowType::ProvideFullScreenDockSpace)
        {
            if (!WasDockLayoutDone(imGuiWindowParams))
            {
                if (imGuiWindowParams.InitialDockLayoutFunction)
                    imGuiWindowParams.InitialDockLayoutFunction(MainDockSpaceId());
                SetDockLayout_Done(imGuiWindowParams);
            }
            ImplProvideFullScreenDockSpace(imGuiWindowParams);
        }
    };

    void CloseWindowOrDock(ImGuiWindowParams& imGuiWindowParams)
    {
        if (imGuiWindowParams.DefaultWindowType != ImGuiWindowParams::ImGuiDefaultWindowType ::NoDefaultWindow)
            ImGui::End();
    }


}  // namespace DockingDetails

}  // namespace HelloImGui