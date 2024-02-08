#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include <cstdio>
#include <cstring>

std::string CurrentRenderingBackend()
{
#ifdef HELLOIMGUI_HAS_OPENGL
    return "HELLOIMGUI_HAS_OPENGL";
#endif
#ifdef HELLOIMGUI_HAS_METAL
    return "HELLOIMGUI_HAS_METAL";
#endif
#ifdef HELLOIMGUI_HAS_VULKAN
    return "HELLOIMGUI_HAS_VULKAN";
#endif
#ifdef HELLOIMGUI_HAS_DIRECTX11
    return "HELLOIMGUI_HAS_DIRECTX11";
#endif
#ifdef HELLOIMGUI_HAS_DIRECTX12
    return "HELLOIMGUI_HAS_DIRECTX12";
#endif
    return "Unknown rendering backend";
}


#ifdef _WIN32
void WriteWin32SuccessFile()
{
    // Under windows, the app works well a local machine and on GitHub CI
    // However it ends with a segfault on Github CI, althought it did run to the end.
    // Let's detect this via an old-style hack.
    printf("Exiting ci_automation_test_app\n");
    FILE* f;
    auto err = fopen_s(&f, "ci_vcpkg_package_test_app_success.txt", "w");
    if (err == 0)
    {
        fprintf(f, "success!");
        fclose(f);
    }
}
#endif


int main(int argc, char**argv)
{
    HelloImGui::RunnerParams runnerParams;

    runnerParams.callbacks.ShowGui = []()
    {
        ImGui::Text("%s", CurrentRenderingBackend().c_str());
        ImGui::ShowDemoWindow();

        double now = ImGui::GetTime();
        ImGui::Text("Time: %lf", now);
        if (ImGui::Button("Exit"))
            HelloImGui::GetRunnerParams()->appShallExit = true;
        if (now >= 1.0)
            HelloImGui::GetRunnerParams()->appShallExit = true;
    };

    printf("Rendering backend: %s\n", CurrentRenderingBackend().c_str());

    try
    {
        runnerParams.platformBackendType = HelloImGui::PlatformBackendType::Glfw;

        if (argc == 2 && strcmp(argv[1], "sdl") == 0)
            runnerParams.platformBackendType = HelloImGui::PlatformBackendType::Sdl;

        if (runnerParams.platformBackendType == HelloImGui::PlatformBackendType::Glfw)
            printf("Using GLFW backend\n");
        else
            printf("Using SDL backend\n");

        HelloImGui::Run(runnerParams);
    }
    catch(...)
    {
        printf("ERROR: exception in ci_vcpkg_package_tests_app\n");
        return 1;
    }

#ifdef _WIN32
    WriteWin32SuccessFile();
#endif
    printf("Exiting ci_vcpkg_package_tests_app with success (ignore exit seg fault on GH runners)\n");
    return 0;
}