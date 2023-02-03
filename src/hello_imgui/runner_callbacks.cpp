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

}  // namespace HelloImGui
