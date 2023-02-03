#include "imgui_internal.h"
#include "hello_imgui/dpi_aware.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/abstract_runner.h"


namespace HelloImGui
{
    AbstractRunner *GetAbstractRunner();

    float EmSize()
    {
        IM_ASSERT(GImGui != NULL); // EmSize can only be called after ImGui context was created!
        float r = ImGui::GetFontSize();
        return r;
    }

    float EmSize(float nbLines)
    {
        return ImGui::GetFontSize() * nbLines;
    }

    ImVec2 EmToVec2(float x, float y)
    {
        IM_ASSERT(GImGui != NULL);
        float k = ImGui::GetFontSize();
        return ImVec2(k * x, k * y);
    }

    ImVec2 EmToVec2(ImVec2 v)
    {
        IM_ASSERT(GImGui != NULL);
        float k = ImGui::GetFontSize();
        return ImVec2(k * v.x, k * v.y);
    }


    float DpiWindowSizeFactor()
    {
        auto runner = GetAbstractRunner();
        IM_ASSERT(runner != nullptr);
        float dpiFactor = runner->DpiWindowSizeFactor();
        return dpiFactor;
    }

    float DpiFontLoadingFactor()
    {
        auto runner = GetAbstractRunner();
        IM_ASSERT(runner != nullptr);
        return runner->DpiFontLoadingFactor();
    }

    float ImGuiDefaultFontGlobalScale()
    {
        auto runner = GetAbstractRunner();
        IM_ASSERT(runner != nullptr);
        return runner->ImGuiDefaultFontGlobalScale();
    }

}