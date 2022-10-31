#pragma once

#include "hello_imgui/backend_api/opengl_setup_api.h"

#if defined(IMGUI_BACKEND_USE_SDL) && defined(IMGUI_BACKEND_USE_OPENGL)

namespace BackendApi
{
    class OpenGlSetupSdl: public IOpenGlSetup
    {
    public:
        virtual ~OpenGlSetupSdl() {}

        void SelectOpenGlVersion() override;
        void InitGlLoader() override;
        std::string GlslVersion()  override;

        virtual void SwapBuffer(void* window) override;
    };
} // namespace BackendApi

#endif // #if defined(IMGUI_BACKEND_USE_SDL) && defined(IMGUI_BACKEND_USE_OPENGLK)
