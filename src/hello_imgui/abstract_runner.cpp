#include "hello_imgui/abstract_runner.h"
#include "imgui.h"

namespace HelloImGui
{

void AbstractRunner::Run()
{
    Setup();
    while (!CreateFramesAndRender()) // Render() returns true when exit is required
        ;
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

bool AbstractRunner::RenderGui()
{
    bool exitRequired = false;
    DockingDetails::ProvideWindowOrDock(params.imGuiWindowParams);
    if (params.callbacks.ShowGui())
        exitRequired = true;
    DockingDetails::CloseWindowOrDock(params.imGuiWindowParams);
    return exitRequired;
}


bool AbstractRunner::CreateFramesAndRender()
{
    bool exitRequired = false;
    if (Impl_PollEvents())
        exitRequired = true;

    Impl_NewFrame_3D();
    Impl_NewFrame_Backend();
    ImGui::NewFrame();

    if (RenderGui())
        exitRequired = true;

    ImGui::Render();
    Impl_Frame_3D_ClearColor();
    Impl_RenderDrawData_To_3D();

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        Impl_UpdateAndRenderAdditionalPlatformWindows();

    Impl_SwapBuffers();

    return exitRequired;
}

void AbstractRunner::TearDown()
{
    Impl_Cleanup();
}


}  // namespace HelloImGui
