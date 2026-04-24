#pragma once

namespace HelloImGui
{
    namespace Internal
    {
        // When fpsIdlingMode is EarlyReturn, HelloImGui never sleeps inside
        // Render() and instead records the duration (in seconds) that the caller
        // should wait before calling Render() again, in order to respect fpsIdle
        // and fpsMax. Returns 0 when no wait is needed.
        //
        // Used only by the Python async runners (immapp.run_async,
        // hello_imgui.run_async, immapp.nb.start) to pace their render loop
        // without blocking the asyncio event loop. Not part of the public API.
        float _PrivIdleFrameWaitDurationForPythonAsyncIo();
    }
}
