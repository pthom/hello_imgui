#pragma once
#include "hello_imgui/hello_imgui_error.h"
#include "hello_imgui/hello_imgui_screenshot.h"

#include <functional>


namespace HelloImGui
{
    using VoidFunction = std::function<void()>;

    //
    // Callbacks for the Rendering Backend (OpenGL, ...)
    //
    // There is no Init backend here, since it needs to account for the Windowing Backend and the Rendering Backend.
    // As a consequence, it is implemented in the Rendering Backend, with #ifdefs
    struct RenderingCallbacks
    {
        VoidFunction                 Impl_NewFrame_3D =          [] { HIMG_ERROR("Empty function"); };
        std::function<void(ImVec4)>  Impl_Frame_3D_ClearColor =  [] (ImVec4) { HIMG_ERROR("Empty function"); };
        VoidFunction                 Impl_RenderDrawData_To_3D = [] { HIMG_ERROR("Empty function"); };
        VoidFunction                 Impl_Shutdown_3D          = [] { HIMG_ERROR("Empty function"); };
        std::function<ImageBuffer()> Impl_ScreenshotRgb_3D     = [] { return ImageBuffer{}; };
    };
} // namespace HelloImGui