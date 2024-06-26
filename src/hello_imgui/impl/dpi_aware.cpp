#include "imgui_internal.h"
#include "hello_imgui/dpi_aware.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/backend_impls/abstract_runner.h"


namespace HelloImGui
{
    AbstractRunner *GetAbstractRunner();

    float EmSize()
    {
        IM_ASSERT(GImGui != NULL && "EmSize can only be called after ImGui context was created!");
        float r = ImGui::GetFontSize();
        return r;
    }

    float EmSize(float nbLines)
    {
        IM_ASSERT(GImGui != NULL && "EmSize can only be called after ImGui context was created!");
        return ImGui::GetFontSize() * nbLines;
    }

    ImVec2 EmToVec2(float x, float y)
    {
        IM_ASSERT(GImGui != NULL && "EmToVec2 can only be called after ImGui context was created!");
        float k = ImGui::GetFontSize();
        return ImVec2(k * x, k * y);
    }

    ImVec2 EmToVec2(ImVec2 v)
    {
        IM_ASSERT(GImGui != NULL && "EmToVec2 can only be called after ImGui context was created!");
        float k = ImGui::GetFontSize();
        return ImVec2(k * v.x, k * v.y);
    }

    ImVec2 PixelsToEm(ImVec2 pixels)
    {
        IM_ASSERT(GImGui != NULL && "PixelsToEm can only be called after ImGui context was created!");
        float k = ImGui::GetFontSize();
        return ImVec2(pixels.x / k, pixels.y / k);
    }

    float  PixelSizeToEm(float pixelSize)
    {
        IM_ASSERT(GImGui != NULL && "PixelSizeToEm can only be called after ImGui context was created!");
        float k = ImGui::GetFontSize();
        return pixelSize / k;
    }

    // Returns the current DpiAwareParams, which are use for font loading and window size scaling
    // - if using Hello ImGui, this will point to `HelloImGui::GetRunnerParams().dpiAwareParams`
    // - if not running an Hello ImGui application, this will return a static instance
    //   (used in rare cases, when people want to use Hello ImGui font loading functions without running
    //   an Hello ImGui application, for example using imgui_md, for markdown rendering in imgui_bundle)
    DpiAwareParams* GetDpiAwareParams()
    {
        if (!IsUsingHelloImGui())
        {
            static DpiAwareParams gDpiAwareParams;
            static bool wasInited = false;
            if (!wasInited)
            {
                gDpiAwareParams.fontRenderingScale = 1.f;
                gDpiAwareParams.dpiWindowSizeFactor = 1.f;
                wasInited = true;
            }
            return &gDpiAwareParams;
        }
        else
        {
            return &GetRunnerParams()->dpiAwareParams;
        }
    }


    float DpiWindowSizeFactor()
    {
        return GetDpiAwareParams()->dpiWindowSizeFactor;
    }

    float DpiFontLoadingFactor()
    {
        return GetDpiAwareParams()->DpiFontLoadingFactor();
    }

    float ImGuiDefaultFontGlobalScale()
    {
        return GetDpiAwareParams()->fontRenderingScale;
    }

}