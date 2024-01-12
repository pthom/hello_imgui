#include "hello_imgui/runner_callbacks.h"

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


}  // namespace HelloImGui
