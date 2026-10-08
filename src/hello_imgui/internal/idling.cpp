#include "hello_imgui/internal/idling.h"

#include <algorithm>
#include <cassert>

namespace HelloImGui
{
// Two forms of idling:
// - inactive idling: no activity, the frame rate falls to fpsIdle;
// - max fps idling: the frame rate stays under fpsMax.
// Each one either waits (the runner owns the loop) or skips the frame (earlyReturn: the browser, or a notebook,
// calls at its own pace). The periods count from the start of the last frame drawn: the frame's own time and its wait
// for vsync are part of them (otherwise, the frame rate falls short of fpsIdle).
IdlingDecision DecideIdling(const FpsIdling& fpsIdling, const IdlingInputs& inputs, IdlingState& state)
{
    assert(fpsIdling.fpsIdle >= 0.f && "fpsIdle must be >= 0");
    IdlingDecision decision;
    double callInterval = inputs.now - state.lastCallTime;
    state.lastCallTime = inputs.now;
    double sinceLastFrame = inputs.now - state.lastFrameTime;

    bool hasRecentEvent = (inputs.now - inputs.timeLastEvent) < (double)fpsIdling.timeActiveAfterLastEvent;
    bool idlingEnabled = fpsIdling.enableIdling && fpsIdling.fpsIdle > 0.f;
    decision.isIdling = idlingEnabled && !hasRecentEvent && !inputs.busy;
    if (decision.isIdling)
    {
        double idlePeriod = 1. / (double)fpsIdling.fpsIdle;
        double idleWait = idlePeriod - sinceLastFrame;
        if (inputs.earlyReturn)
        {
            // The caller calls at the display's rate: a frame is drawn at the call nearest to the idle period, half a
            // call early at most. Otherwise the frame rate rounds down, or alternates: on a 60 Hz display, 30 would
            // give 20 or 30 at random, 27 would give 20.
            if (sinceLastFrame < idlePeriod - callInterval * 0.5)
            {
                decision.skipFrame = true;
                decision.asyncWaitSeconds = std::max(idleWait, 0.);
            }
        }
        else if (idleWait > 0.)
            decision.waitForEventsSeconds = idleWait;
    }

    if (fpsIdling.fpsMax > 0.f)
    {
        double maxFpsWait = 1. / (double)fpsIdling.fpsMax - sinceLastFrame;
        if (maxFpsWait > 0.0005)  // 0.5 ms: not worth a sleep
        {
            if (inputs.earlyReturn)
            {
                decision.skipFrame = true;
                decision.asyncWaitSeconds = std::max(decision.asyncWaitSeconds, maxFpsWait);
            }
            else
                decision.sleepSeconds = maxFpsWait;
        }
    }
    return decision;
}
}  // namespace HelloImGui
