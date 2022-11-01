#pragma once
#ifdef HELLOIMGUI_USE_GLFW
#include "opengl_setup_api.h"


namespace HelloImGui { namespace BackendApi
{
    class OpenGlSetupGlfw: public IOpenGlSetup
    {
    public:
        virtual ~OpenGlSetupGlfw() {}

        void SelectOpenGlVersion() override;
        void InitGlLoader() override;
        std::string GlslVersion()  override;
    };
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_GLFW
