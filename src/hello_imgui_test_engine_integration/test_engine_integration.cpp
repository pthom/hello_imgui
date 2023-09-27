//
// Created by Pascal Thomet on 26/09/2023.
//
#include "imgui_test_engine/imgui_te_engine.h"
#include "hello_imgui/runner_params.h"
#include "hello_imgui/internal/functional_utils.h"

namespace HelloImGui
{
    struct TestEngineIntegration
    {
        ImGuiTestEngine *mTestEngine = nullptr;

        void _SetOptions()
        {
            ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(mTestEngine);
            test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
            test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
            test_io.ConfigRunSpeed = ImGuiTestRunSpeed_Cinematic; // Default to slowest mode in this demo

            // test_io.ScreenCaptureFunc = ImGuiApp_ScreenCaptureFunc;
            // test_io.ScreenCaptureUserData = (void*)app;

            // Optional: save test output in junit-compatible XML format.
            //test_io.ExportResultsFile = "./results.xml";
            //test_io.ExportResultsFormat = ImGuiTestEngineExportFormat_JUnitXml;
        }

        void Setup()
        {
            // Setup test engine
            mTestEngine = ImGuiTestEngine_CreateContext();

            _SetOptions();

            // Start test engine
            ImGuiTestEngine_Start(mTestEngine, ImGui::GetCurrentContext());
            ImGuiTestEngine_InstallDefaultCrashHandler();
        }

        void Shutdown_Before_ImGui_DestroyContext()
        {
            ImGuiTestEngine_Stop(mTestEngine);
        }
        void Shutdown_After_ImGui_DestroyContext()
        {
            // IMPORTANT: we need to destroy the Dear ImGui context BEFORE the test engine context, so .ini data may be saved.
            ImGuiTestEngine_DestroyContext(mTestEngine);
        }

        void PostSwap()
        {
            // Call after your rendering. This is mostly to support screen/video capturing features.
            ImGuiTestEngine_PostSwap(mTestEngine);
        }
    };

    TestEngineIntegration gTestEngineIntegration;


    void _AddTestEngineCallbacks(RunnerParams* runnerParams)
    {
        auto &callbacks = runnerParams->callbacks;

        callbacks.PostInit = FunctionalUtils::sequence_functions(
            []() { gTestEngineIntegration.Setup(); },
            callbacks.PostInit
            );

        callbacks.AfterSwap = FunctionalUtils::sequence_functions(
            []() { gTestEngineIntegration.PostSwap(); },
            callbacks.AfterSwap
            );

        callbacks.BeforeExit = FunctionalUtils::sequence_functions(
            []() { gTestEngineIntegration.Shutdown_Before_ImGui_DestroyContext(); },
            callbacks.BeforeExit
            );

        callbacks.BeforeExit_PostCleanup = FunctionalUtils::sequence_functions(
            []() { gTestEngineIntegration.Shutdown_After_ImGui_DestroyContext(); },
            callbacks.BeforeExit_PostCleanup
        );
    }

    ImGuiTestEngine* GetImGuiTestEngine()
    {
        return gTestEngineIntegration.mTestEngine;
    }
}