#pragma once
#include "hello_imgui/screen_bounds.h"

namespace HelloImGui
{
    class RemoteDisplayHandler
    {
    public:
        // Will initialize the remote display handler
        // (do nothing if no remote display is configured/compiled)
        void Create();

        // Will shut down the remote display handler
        // (do nothing if no remote display is configured/compiled)
        void Shutdown();

        // Will update the remote display handler
        // (do nothing if no remote display is configured/compiled)
        void Heartbeat_PreImGuiNewFrame();
        void Heartbeat_PostImGuiRender();

        // Will send the window size to the remote display
        // (do nothing if no remote display is configured/compiled)
        void TransmitWindowSizeToDisplay(ScreenSize windowSize);

        // Will send the fonts to the remote display
        // (do nothing if no remote display is configured/compiled)
        void SendFonts();

        // Returns true if the application should display on a remote server
        // (return false if no remote display is configured/compiled)
        bool ShouldRemoteDisplay();

        // May update GetRunnerParams()->dpiAwareParams by interrogating the remote display
        // (return false if no remote display is configured/compiled)
        bool CheckDpiAwareParamsChanges();

        // Can the user quit the application?
        bool CanQuitApp();

    private:
        // Returns true if the application is connected to a remote server
        // (return false if no remote display is configured/compiled)
        bool IsConnectedToRemoteDisplay();
    };
}