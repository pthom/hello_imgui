#pragma once

namespace HelloImGui
{
    namespace BackendApi { class IBackendWindowHelper; }

    // True when a refresh was asked (RequestRefresh(), SetItemIsLive()) since the last call: the runner does not idle
    // on this frame. Called by the runner once per frame, when it decides whether to idle.
    bool ConsumeRefreshRequest();

    // The window helper whose wait for events RequestRefresh() ends, from any thread. Set by the runner once its
    // platform backend is up, and reset to nullptr before the backend shuts down.
    void SetRefreshWakeUp(BackendApi::IBackendWindowHelper* windowHelper);
}
