#include "rendering_null.h"


namespace HelloImGui
{
    RenderingCallbacksPtr CreateBackendCallbacks_Null()
    {
        auto callbacks = std::make_shared<RenderingCallbacks>();

        callbacks->Impl_NewFrame_3D = [] {};

        callbacks->Impl_RenderDrawData_To_3D = [] {};

        callbacks->Impl_ScreenshotRgb_3D = []() { return ImageBuffer{}; };

        callbacks->Impl_Frame_3D_ClearColor = [](ImVec4) {};

        callbacks->Impl_Shutdown_3D = [] {};

        callbacks->Impl_CreateFontTexture = [] {};
        callbacks->Impl_DestroyFontTexture = [] {};

        return callbacks;
    }

}
