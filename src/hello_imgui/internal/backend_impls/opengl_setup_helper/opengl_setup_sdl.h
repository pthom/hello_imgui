#pragma once
#ifdef HELLOIMGUI_USE_SDL2

#include "opengl_setup_api.h"


namespace HelloImGui { namespace BackendApi
{
    class OpenGlSetupSdl: public IOpenGlSetup
    {
    public:
        virtual ~OpenGlSetupSdl() {}

        void SelectOpenGlVersion() override;
        void InitGlLoader() override;
        std::string GlslVersion()  override;
    };
}} // namespace HelloImGui { namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL2
