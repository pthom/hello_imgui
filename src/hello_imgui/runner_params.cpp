#include "hello_imgui/runner_params.h"

namespace HelloImGui
{
    RunnerParams SimpleRunnerParams::ToRunnerParams() const
    {
        auto& self = *this;
        RunnerParams r;

        r.callbacks.ShowGui = self.guiFunction;

        r.appWindowParams.windowGeometry.size = self.windowSize;
        r.appWindowParams.windowGeometry.sizeAuto = self.windowSizeAuto;
        r.appWindowParams.restorePreviousGeometry = self.windowRestorePreviousGeometry;

        r.appWindowParams.windowTitle = self.windowTitle;

        r.fpsIdling.fpsIdle = self.fpsIdle;
        r.fpsIdling.enableIdling = self.enableIdling;

        return r;
    }
}  // namespace HelloImGui