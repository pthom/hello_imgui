#pragma once

#include "hello_imgui/backend_api/opengl_setup_api.h"

#if defined(IMGUI_BACKEND_USE_GLFW) && defined(IMGUI_BACKEND_USE_OPENGL)

namespace BackendApi
{
    class OpenGlSetupGlfw: public IOpenGlSetup
    {
    public:
        virtual ~OpenGlSetupGlfw() {}

        void SelectOpenGlVersion() override;
        void InitGlLoader() override;
        std::string GlslVersion()  override;
    };
} // namespace BackendApi

#endif // #if defined(IMGUI_BACKEND_USE_SDL) && defined(IMGUI_BACKEND_USE_OPENGLK)
