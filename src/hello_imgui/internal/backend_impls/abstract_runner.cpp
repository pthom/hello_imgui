#include "hello_imgui/internal/backend_impls/abstract_runner.h"
#include "hello_imgui/internal/docking_details.h"
#include "hello_imgui/internal/menu_statusbar.h"
#include "hello_imgui/internal/runner_functions.h"
#include "imgui.h"

namespace HelloImGui
{

void AbstractRunner::Run()
{
    Setup();

#ifdef HELLOIMGUI_MOBILEDEVICE
    while (true)
        CreateFramesAndRender();
#else
    while (!params.appShallExit)
        CreateFramesAndRender();
#endif
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
    Impl_SetupImgGuiContext();
    Impl_SetupPlatformRendererBindings();

    RunnerFunctions::PrepareApp(params);
}


void AbstractRunner::CreateFramesAndRender()
{
    if (Impl_PollEvents())
        params.appShallExit = true;

    Impl_NewFrame_3D();
    Impl_NewFrame_Backend();
    ImGui::NewFrame();

    RunnerFunctions::RenderGui(params);

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
