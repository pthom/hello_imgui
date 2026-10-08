// The idling's decision (internal/idling.h), alone: a clock and a caller are simulated, the frames drawn are counted.
#include "doctest.h"
#include "hello_imgui/internal/idling.h"

namespace
{
using namespace HelloImGui;

// A caller that calls at a fixed pace for `seconds` (a browser at the display's rate), with no activity: the frames
// drawn per second. Each frame drawn starts at its call.
double FramesPerSecond(FpsIdling fpsIdling, double callRate, double seconds = 4.)
{
    IdlingState state;
    IdlingInputs inputs;
    inputs.earlyReturn = true;
    int frames = 0;
    int nbCalls = (int)(seconds * callRate);
    for (int i = 0; i < nbCalls; ++i)
    {
        inputs.now = 10. + i / callRate;  // past timeActiveAfterLastEvent
        if (DecideIdling(fpsIdling, inputs, state).skipFrame)
            continue;
        state.lastFrameTime = inputs.now;
        ++frames;
    }
    return frames / seconds;
}

FpsIdling WithIdle(float fpsIdle)
{
    FpsIdling fpsIdling;
    fpsIdling.fpsIdle = fpsIdle;
    return fpsIdling;
}
}  // namespace

TEST_CASE("Idling: a browser at 60 Hz draws the idle rate, rounded to the nearest rate it can")
{
    CHECK(FramesPerSecond(WithIdle(30.f), 60.) == doctest::Approx(30.).epsilon(0.02));
    CHECK(FramesPerSecond(WithIdle(27.f), 60.) == doctest::Approx(30.).epsilon(0.02));  // not 20
    CHECK(FramesPerSecond(WithIdle(9.f), 60.) == doctest::Approx(8.6).epsilon(0.05));   // one call in 7
    CHECK(FramesPerSecond(WithIdle(30.f), 120.) == doctest::Approx(30.).epsilon(0.02));
}

TEST_CASE("Idling: natively, the wait counts from the start of the last frame")
{
    IdlingState state;
    state.lastFrameTime = 10.;
    IdlingInputs inputs;
    inputs.now = 10.010;  // the frame took 10 ms
    IdlingDecision decision = DecideIdling(WithIdle(30.f), inputs, state);
    CHECK(decision.isIdling);
    CHECK(!decision.skipFrame);
    CHECK(decision.waitForEventsSeconds == doctest::Approx(1. / 30. - 0.010));
    inputs.now = 10.050;  // later than the idle period: no wait
    CHECK(DecideIdling(WithIdle(30.f), inputs, state).waitForEventsSeconds == 0.);
}

TEST_CASE("Idling: an activity prevents it (busy, a recent event), and so do the params")
{
    IdlingState state;
    IdlingInputs inputs;
    inputs.now = 10.;
    inputs.busy = true;  // e.g. a refresh request
    CHECK(!DecideIdling(WithIdle(9.f), inputs, state).isIdling);
    inputs.busy = false;
    inputs.timeLastEvent = 9.;  // 1 s ago, within timeActiveAfterLastEvent (3 s)
    CHECK(!DecideIdling(WithIdle(9.f), inputs, state).isIdling);
    inputs.timeLastEvent = 6.;
    CHECK(DecideIdling(WithIdle(9.f), inputs, state).isIdling);
    CHECK(!DecideIdling(WithIdle(0.f), inputs, state).isIdling);
    FpsIdling disabled = WithIdle(9.f);
    disabled.enableIdling = false;
    IdlingDecision decision = DecideIdling(disabled, inputs, state);
    CHECK(!decision.isIdling);
    CHECK(decision.waitForEventsSeconds == 0.);
}

TEST_CASE("Idling: fpsMax caps the frame rate, by a sleep or by skipped frames")
{
    FpsIdling fpsIdling;
    fpsIdling.enableIdling = false;
    fpsIdling.fpsMax = 50.f;
    IdlingState state;
    state.lastFrameTime = 10.;
    IdlingInputs inputs;
    inputs.now = 10.005;
    IdlingDecision decision = DecideIdling(fpsIdling, inputs, state);
    CHECK(decision.sleepSeconds == doctest::Approx(0.015));
    CHECK(!decision.skipFrame);
    inputs.earlyReturn = true;
    decision = DecideIdling(fpsIdling, inputs, state);
    CHECK(decision.skipFrame);
    CHECK(decision.asyncWaitSeconds == doctest::Approx(0.015));
    // In a browser, fpsMax rounds down (a frame every 3 calls: 25 ms, not 20): it lacks the rounding to the nearest
    // call that fpsIdle has
    CHECK(FramesPerSecond(fpsIdling, 120.) == doctest::Approx(40.).epsilon(0.02));
}
