#pragma once
#include "hello_imgui/runner_params.h"  // FpsIdling

// The idling's decision for a frame, apart from the runner: no backend, no clock. The runner gathers the inputs at
// the start of each frame, and applies the decision (a wait, a sleep, or a frame skipped).
namespace HelloImGui
{
    // What the runner knows at the start of a frame
    struct IdlingInputs
    {
        double now = 0.;            // s, the runner's clock
        double timeLastEvent = -1.; // s: an event keeps the app awake for FpsIdling::timeActiveAfterLastEvent
        bool busy = false;          // the first frames, a test, a remote display, a button held, a refresh requested
        bool earlyReturn = false;   // the frames are skipped (a browser, a notebook), instead of waited for
    };

    // The idling's memory, from one frame to the next
    struct IdlingState
    {
        double lastFrameTime = 0.;  // s: the start of the last frame drawn (the runner sets it, after the waits)
        double lastCallTime = 0.;   // s: the last call, a frame drawn or skipped (the caller's pace, in a browser)
    };

    // What the runner does with this frame
    struct IdlingDecision
    {
        bool isIdling = false;            // no activity: the frame rate falls to FpsIdling::fpsIdle
        bool skipFrame = false;           // earlyReturn: no frame on this call
        double waitForEventsSeconds = 0.; // wait this long, unless an event comes (fpsIdle)
        double sleepSeconds = 0.;         // then sleep this long (fpsMax)
        double asyncWaitSeconds = 0.;     // earlyReturn: how long the caller may wait before its next call
    };

    // Decides the idling of a frame, and notes the call in `state`
    IdlingDecision DecideIdling(const FpsIdling& fpsIdling, const IdlingInputs& inputs, IdlingState& state);
}
