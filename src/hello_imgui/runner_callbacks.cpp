#include "hello_imgui/runner_callbacks.h"

namespace HelloImGui
{
    VoidFunction AppendCallback(const VoidFunction& previousCallback, const VoidFunction& newCallback)
    {
        auto composedCallback = [previousCallback, newCallback]() {
            if (previousCallback)
                previousCallback();
            if (newCallback)
                newCallback();
        };
        return composedCallback;
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


}  // namespace HelloImGui
