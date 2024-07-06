#pragma once
#ifdef HELLOIMGUI_USE_SDL2

#include "opengl_setup_api.h"


namespace HelloImGui { namespace BackendApi
{
    class OpenGlSetupSdl: public IOpenGlSetup
    {
    public:
        virtual ~OpenGlSetupSdl() {}

        void SelectOpenGlVersion(const OpenGlOptionsFilled_& options) override;
        void InitGlLoader() override;
        OpenGlOptionsFilled_ Impl_MakeDefaultOpenGlOptionsForPlatform() override;
    };
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL2
