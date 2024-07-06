#pragma once
#ifdef HELLOIMGUI_USE_GLFW3
#include "opengl_setup_api.h"


namespace HelloImGui { namespace BackendApi
{
    class OpenGlSetupGlfw: public IOpenGlSetup
    {
    public:
        virtual ~OpenGlSetupGlfw() {}

        void SelectOpenGlVersion(const OpenGlOptionsFilled_& options) override;
        void InitGlLoader() override;
        OpenGlOptionsFilled_ Impl_MakeDefaultOpenGlOptionsForPlatform() override;
    };
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_GLFW3
