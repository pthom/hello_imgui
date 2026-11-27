// The added callbacks (RunnerCallbacks::AddCallback, CallSoon, CallLater), through the runner's side
// (RunnerCallbacksInternal_): the order around the app's function, the removals, an assignment that keeps them.
#include "doctest.h"
#include "hello_imgui/runner_callbacks.h"
#include "hello_imgui/internal/runner_callbacks_internal.h"
#include "hello_imgui/internal/idling.h"

#include <chrono>
#include <string>
#include <thread>

using namespace HelloImGui;

namespace
{
    void Call(RunnerCallbacks& callbacks, CallbackSlot slot, const VoidFunction& app)
    {
        RunnerCallbacksInternal_::CallSlot(callbacks, slot, app);
    }
}

TEST_CASE("Added callbacks: the order around the app's function")
{
    RunnerCallbacks callbacks;
    std::string log;
    auto add = [&](CallbackSlot slot, const std::string& name, CallbackOrder order = CallbackOrder::Default) {
        callbacks.AddCallback(slot, [&log, name] { log += name; }, "test", order);
    };
    auto app = [&log] { log += "app,"; };

    add(CallbackSlot::PostInit, "a1,");
    add(CallbackSlot::PostInit, "a2,");
    Call(callbacks, CallbackSlot::PostInit, app);
    CHECK(log == "a1,a2,app,");  // setup: the add-ons first, in their order

    log.clear();
    add(CallbackSlot::BeforeExit, "b1,");
    add(CallbackSlot::BeforeExit, "b2,");
    Call(callbacks, CallbackSlot::BeforeExit, app);
    CHECK(log == "app,b2,b1,");  // exit: the app first, then the add-ons in reverse order

    log.clear();
    add(CallbackSlot::PreNewFrame, "p,");
    add(CallbackSlot::PostNewFrame, "n,");
    Call(callbacks, CallbackSlot::PreNewFrame, app);
    Call(callbacks, CallbackSlot::PostNewFrame, app);
    CHECK(log == "p,app,app,n,");  // PreNewFrame: before the app; the other slots: after

    log.clear();
    add(CallbackSlot::BeforeSwap, "before,", CallbackOrder::Before);
    add(CallbackSlot::PostInit, "after,", CallbackOrder::After);
    Call(callbacks, CallbackSlot::BeforeSwap, app);
    Call(callbacks, CallbackSlot::PostInit, app);
    CHECK(log == "before,app,a1,a2,app,after,");  // the override

    CHECK(CallbackSlot::BeforeGui == CallbackSlot::PostNewFrame);
    CHECK(CallbackSlot::AfterGui == CallbackSlot::BeforeImGuiRender);
}

TEST_CASE("Added callbacks: an assignment of the slot keeps them; removal by id and by owner")
{
    RunnerCallbacks callbacks;
    int addon = 0, other = 0, app = 0;
    CallbackId id = callbacks.AddCallback(CallbackSlot::BeforeExit, [&] { ++addon; }, "immapp");
    callbacks.AddCallback(CallbackSlot::BeforeExit, [&] { ++other; }, "other");
    callbacks.BeforeExit = [&] { ++app; };  // the app's plain assignment
    Call(callbacks, CallbackSlot::BeforeExit, callbacks.BeforeExit);
    CHECK((addon == 1 && other == 1 && app == 1));

    CHECK(id != 0);
    callbacks.RemoveCallback(id);
    Call(callbacks, CallbackSlot::BeforeExit, callbacks.BeforeExit);
    CHECK((addon == 1 && other == 2 && app == 2));

    callbacks.AddCallback(CallbackSlot::BeforeExit, [&] { ++addon; }, "immapp");
    callbacks.RemoveCallbacks("other");
    Call(callbacks, CallbackSlot::BeforeExit, callbacks.BeforeExit);
    CHECK((addon == 2 && other == 2 && app == 3));

    // A copy of the callbacks (of the RunnerParams) copies the added ones
    RunnerCallbacks copy = callbacks;
    Call(copy, CallbackSlot::BeforeExit, copy.BeforeExit);
    CHECK(addon == 3);
}

TEST_CASE("CallSoon: once, at the first call of its slot in a frame at least afterFrames later")
{
    RunnerCallbacks callbacks;
    RunnerCallbacksInternal_::SetFrameIndex(10);
    int next = 0, soon = 0, later2 = 0;
    callbacks.CallSoon([&] { ++next; });                                     // default: the next frame, BeforeGui
    callbacks.CallSoon([&] { ++soon; }, 0, CallbackSlot::BeforeImGuiRender);  // this frame, still to come
    callbacks.CallSoon([&] { ++later2; }, 2, CallbackSlot::PreNewFrame);
    CHECK(RunnerCallbacksInternal_::HasCallSoonWaiting(callbacks));

    Call(callbacks, CallbackSlot::BeforeGui, {});
    Call(callbacks, CallbackSlot::BeforeImGuiRender, {});
    CHECK((next == 0 && soon == 1 && later2 == 0));  // frame 10

    RunnerCallbacksInternal_::SetFrameIndex(11);
    Call(callbacks, CallbackSlot::PreNewFrame, {});
    Call(callbacks, CallbackSlot::BeforeGui, {});
    Call(callbacks, CallbackSlot::BeforeImGuiRender, {});
    CHECK((next == 1 && soon == 1 && later2 == 0));  // frame 11

    RunnerCallbacksInternal_::SetFrameIndex(12);
    Call(callbacks, CallbackSlot::PreNewFrame, {});
    Call(callbacks, CallbackSlot::BeforeGui, {});
    CHECK((next == 1 && soon == 1 && later2 == 1));  // frame 12; each ran once
    CHECK(!RunnerCallbacksInternal_::HasCallSoonWaiting(callbacks));

    // A slot called once ignores the count; a CallSoon can be canceled
    int init = 0, canceled = 0;
    callbacks.CallSoon([&] { ++init; }, 5, CallbackSlot::PostInit);
    CallbackId id = callbacks.CallSoon([&] { ++canceled; });
    callbacks.RemoveCallback(id);
    Call(callbacks, CallbackSlot::PostInit, {});
    RunnerCallbacksInternal_::SetFrameIndex(13);
    Call(callbacks, CallbackSlot::BeforeGui, {});
    CHECK((init == 1 && canceled == 0));

    // A callback that calls CallSoon from its own call: the new one waits for the next frame
    int chained = 0;
    callbacks.CallSoon([&] { callbacks.CallSoon([&] { ++chained; }); }, 0);
    Call(callbacks, CallbackSlot::BeforeGui, {});
    CHECK(chained == 0);
    RunnerCallbacksInternal_::SetFrameIndex(14);
    Call(callbacks, CallbackSlot::BeforeGui, {});
    CHECK(chained == 1);
    RunnerCallbacksInternal_::SetFrameIndex(-1);
}

TEST_CASE("CallLater: once, when its time has come; the idling's wait ends then")
{
    RunnerCallbacks callbacks;
    int later = 0;
    callbacks.CallLater([&] { ++later; }, 0.05);
    double due = RunnerCallbacksInternal_::NextCallLaterTime(callbacks);
    CHECK(due > 0.);
    Call(callbacks, CallbackSlot::BeforeGui, {});
    CHECK(later == 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    Call(callbacks, CallbackSlot::BeforeGui, {});
    Call(callbacks, CallbackSlot::BeforeGui, {});
    CHECK(later == 1);
    CHECK(RunnerCallbacksInternal_::NextCallLaterTime(callbacks) < 0.);

    // The idling: an idle app (fpsIdle 9: a wait of 0.11 s) waits only until the call is due
    FpsIdling fpsIdling;
    fpsIdling.fpsIdle = 9.f;
    fpsIdling.fpsMax = 0.f;
    IdlingState state;
    IdlingInputs inputs;
    inputs.now = 100.;
    state.lastFrameTime = 100.;
    inputs.nextCallTime = 100.03;
    IdlingDecision decision = DecideIdling(fpsIdling, inputs, state);
    CHECK(decision.isIdling);
    CHECK(decision.waitForEventsSeconds == doctest::Approx(0.03));

    // In a browser (early return): no frame is skipped once the call is due
    inputs.earlyReturn = true;
    inputs.now = 100.04;
    decision = DecideIdling(fpsIdling, inputs, state);
    CHECK(!decision.skipFrame);
}
