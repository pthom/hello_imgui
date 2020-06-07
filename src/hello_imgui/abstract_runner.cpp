#include "hello_imgui/abstract_runner.h"
#include "imgui.h"

namespace HelloImGui
{

void AbstractRunner::Run()
{
    Setup();
    while (!params.appShallExit)
        CreateFramesAndRender();
    TearDown();
}


void AbstractRunner::Setup()
{
    Impl_InitBackend();
    Impl_Select_Gl_Version();
    Impl_CreateWindowAndContext();
    Impl_InitGlLoader();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    params.callbacks.SetupImGuiConfig();
    params.callbacks.SetupImGuiStyle();
    Impl_SetupPlatformRendererBindings();
    params.callbacks.LoadAdditionalFonts();

    DockingDetails::ConfigureImGuiDocking(params.imGuiWindowParams);

    params.callbacks.PostInit();
}

void AbstractRunner::RenderGui()
{
    DockingDetails::ProvideWindowOrDock(params.imGuiWindowParams);
    params.callbacks.ShowGui();
    DockingDetails::CloseWindowOrDock(params.imGuiWindowParams);
}


void AbstractRunner::CreateFramesAndRender()
{
    if (Impl_PollEvents())
        params.appShallExit = true;

    Impl_NewFrame_3D();
    Impl_NewFrame_Backend();
    ImGui::NewFrame();

    RenderGui();

    ImGui::Render();
    Impl_Frame_3D_ClearColor();
    Impl_RenderDrawData_To_3D();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        Impl_UpdateAndRenderAdditionalPlatformWindows();

    Impl_SwapBuffers();
}

void AbstractRunner::TearDown()
{
    Impl_Cleanup();
}


}  // namespace HelloImGui
