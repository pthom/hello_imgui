#pragma once
#ifdef HELLOIMGUI_USE_SDL

#include "opengl_setup_api.h"


namespace BackendApi
{
    class OpenGlSetupSdl: public IOpenGlSetup
    {
    public:
        virtual ~OpenGlSetupSdl() {}

        void SelectOpenGlVersion() override;
        void InitGlLoader() override;
        std::string GlslVersion()  override;
    };
} // namespace BackendApi

#endif // #ifdef HELLOIMGUI_USE_SDL
