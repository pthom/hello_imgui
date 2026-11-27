#pragma once
#include "hello_imgui/runner_callbacks.h"

namespace HelloImGui
{
    // The runner's side of the added callbacks (RunnerCallbacks::AddCallback, CallSoon, CallLater)
    struct RunnerCallbacksInternal_
    {
        // Calls a slot: the added callbacks and the app's function, in their order (see CallbackOrder)
        static void CallSlot(RunnerCallbacks& callbacks, CallbackSlot slot, VoidFunction appFunction);
        // Whether the slot has a function to call: the app's, or an added one
        static bool HasCallbacks(const RunnerCallbacks& callbacks, CallbackSlot slot, const VoidFunction& appFunction);
        // A CallSoon is waiting: the app must not idle
        static bool HasCallSoonWaiting(const RunnerCallbacks& callbacks);
        // The time of the next CallLater, on the runner's clock (-1: none)
        static double NextCallLaterTime(const RunnerCallbacks& callbacks);
        // The runner's current frame, for CallSoon (-1 outside a run)
        static void SetFrameIndex(int frameIndex);
    };
}
