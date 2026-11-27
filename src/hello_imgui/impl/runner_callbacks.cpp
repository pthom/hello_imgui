#include "hello_imgui/runner_callbacks.h"
#include "hello_imgui/internal/runner_callbacks_internal.h"
#include "hello_imgui/internal/clock_seconds.h"

#include <algorithm>

namespace HelloImGui
{
    VoidFunction SequenceFunctions(const VoidFunction& f1, const VoidFunction& f2)
    {
        auto composedCallback = [f1, f2]() {
            if (f1)
                f1();
            if (f2)
                f2();
        };
        return composedCallback;
    }

    VoidFunction AppendCallback(const VoidFunction& previousCallback, const VoidFunction& newCallback)
    {
        return SequenceFunctions(previousCallback, newCallback);
    }

    std::vector<EdgeToolbarType> AllEdgeToolbarTypes()
    {
        return { EdgeToolbarType::Top, EdgeToolbarType::Bottom, EdgeToolbarType::Left, EdgeToolbarType::Right };
    }

    std::string EdgeToolbarTypeName(EdgeToolbarType e)
    {
        if (e == EdgeToolbarType::Right)
            return "Right";
        else if (e == EdgeToolbarType::Left)
            return "Left";
        else if (e == EdgeToolbarType::Top)
            return "Top";
        else if (e == EdgeToolbarType::Bottom)
            return "Bottom";
        else
            return "";
    }

    void RunnerCallbacks::AddEdgeToolbar(EdgeToolbarType edgeToolbarType,
                        VoidFunction guiFunction,
                        const EdgeToolbarOptions& options)
    {
        EdgeToolbar edgeToolbar;
        edgeToolbar.ShowToolbar = guiFunction;
        edgeToolbar.options = options;
        edgesToolbars[edgeToolbarType] = edgeToolbar;
    }

    void RunnerCallbacks::EnqueueBeforeExit(const HelloImGui::VoidFunction &callback)
    {
        BeforeExit = SequenceFunctions(BeforeExit, callback);
    }

    void RunnerCallbacks::EnqueuePostInit(const HelloImGui::VoidFunction &callback)
    {
        PostInit = SequenceFunctions(PostInit, callback);
    }



    // ---------------------------------------------------------------------------------------------------------------
    // Added callbacks
    // ---------------------------------------------------------------------------------------------------------------
    namespace
    {
        CallbackId gNextCallbackId = 1;
        int gFrameIndex = -1;  // the runner's current frame (-1 outside a run: a CallSoon added then counts from it)

        bool IsSetupSlot(CallbackSlot slot)
        {
            return slot == CallbackSlot::PostInit_AddPlatformBackendCallbacks || slot == CallbackSlot::PostInit ||
                   slot == CallbackSlot::SetupImGuiConfig || slot == CallbackSlot::SetupImGuiStyle ||
                   slot == CallbackSlot::RegisterTests;
        }
        bool IsExitSlot(CallbackSlot slot)
        {
            return slot == CallbackSlot::BeforeExit || slot == CallbackSlot::BeforeExit_PostCleanup;
        }
        // The slots called once in a run: a CallSoon's frame count does not apply
        bool IsCalledOnce(CallbackSlot slot) { return IsSetupSlot(slot) || IsExitSlot(slot); }

        bool RunsBeforeApp(const AddedCallback_& c)
        {
            if (c.Order == CallbackOrder::Before)
                return true;
            if (c.Order == CallbackOrder::After)
                return false;
            return IsSetupSlot(c.Slot) || c.Slot == CallbackSlot::PreNewFrame;
        }
        bool IsDue(const AddedCallback_& c)
        {
            if (c.DueTime >= 0. && Internal::ClockSeconds() < c.DueTime)
                return false;
            if (c.DueFrame >= 0 && !IsCalledOnce(c.Slot) && gFrameIndex < c.DueFrame)
                return false;
            return true;
        }
    }

    CallbackId RunnerCallbacks::AddCallback(CallbackSlot slot, const VoidFunction& callback, const std::string& owner,
                                            CallbackOrder order)
    {
        AddedCallback_ c;
        c.Id = gNextCallbackId++;
        c.Slot = slot;
        c.Callback = callback;
        c.Owner = owner;
        c.Order = order;
        mAddedCallbacks.push_back(c);
        return c.Id;
    }

    CallbackId RunnerCallbacks::CallSoon(const VoidFunction& callback, int afterFrames, CallbackSlot slot,
                                         const std::string& owner)
    {
        CallbackId id = AddCallback(slot, callback, owner);
        mAddedCallbacks.back().Once = true;
        mAddedCallbacks.back().DueFrame = std::max(gFrameIndex + std::max(afterFrames, 0), 0);
        return id;
    }

    CallbackId RunnerCallbacks::CallLater(const VoidFunction& callback, double seconds, CallbackSlot slot,
                                          const std::string& owner)
    {
        CallbackId id = AddCallback(slot, callback, owner);
        mAddedCallbacks.back().Once = true;
        mAddedCallbacks.back().DueTime = Internal::ClockSeconds() + std::max(seconds, 0.);
        return id;
    }

    void RunnerCallbacks::RemoveCallback(CallbackId id)
    {
        mAddedCallbacks.erase(
            std::remove_if(mAddedCallbacks.begin(), mAddedCallbacks.end(), [id](const AddedCallback_& c) { return c.Id == id; }),
            mAddedCallbacks.end());
    }

    void RunnerCallbacks::RemoveCallbacks(const std::string& owner)
    {
        mAddedCallbacks.erase(
            std::remove_if(mAddedCallbacks.begin(), mAddedCallbacks.end(), [&owner](const AddedCallback_& c) { return c.Owner == owner; }),
            mAddedCallbacks.end());
    }

    void RunnerCallbacksInternal_::CallSlot(RunnerCallbacks& callbacks, CallbackSlot slot, VoidFunction appFunction)
    {
        // A callback may add or remove callbacks: the ids are read first, and each one is looked up before its call
        std::vector<CallbackId> before, after;
        for (const AddedCallback_& c : callbacks.mAddedCallbacks)
            if (c.Slot == slot)
                (RunsBeforeApp(c) ? before : after).push_back(c.Id);
        if (IsExitSlot(slot))
            std::reverse(after.begin(), after.end());

        auto call = [&callbacks](CallbackId id) {
            auto& list = callbacks.mAddedCallbacks;
            auto it = std::find_if(list.begin(), list.end(), [id](const AddedCallback_& c) { return c.Id == id; });
            if (it == list.end() || !IsDue(*it))
                return;
            VoidFunction fn = it->Callback;
            if (it->Once)
                list.erase(it);
            if (fn)
                fn();
        };
        for (CallbackId id : before)
            call(id);
        if (appFunction)
            appFunction();
        for (CallbackId id : after)
            call(id);
    }

    bool RunnerCallbacksInternal_::HasCallbacks(const RunnerCallbacks& callbacks, CallbackSlot slot,
                                                const VoidFunction& appFunction)
    {
        if (appFunction)
            return true;
        return std::any_of(callbacks.mAddedCallbacks.begin(), callbacks.mAddedCallbacks.end(),
                           [slot](const AddedCallback_& c) { return c.Slot == slot; });
    }

    bool RunnerCallbacksInternal_::HasCallSoonWaiting(const RunnerCallbacks& callbacks)
    {
        return std::any_of(callbacks.mAddedCallbacks.begin(), callbacks.mAddedCallbacks.end(),
                           [](const AddedCallback_& c) { return c.Once && c.DueFrame >= 0; });
    }

    double RunnerCallbacksInternal_::NextCallLaterTime(const RunnerCallbacks& callbacks)
    {
        double next = -1.;
        for (const AddedCallback_& c : callbacks.mAddedCallbacks)
            if (c.Once && c.DueTime >= 0. && (next < 0. || c.DueTime < next))
                next = c.DueTime;
        return next;
    }

    void RunnerCallbacksInternal_::SetFrameIndex(int frameIndex) { gFrameIndex = frameIndex; }


}  // namespace HelloImGui
