#pragma once
#ifdef HELLOIMGUI_USE_SDL_OPENGL3
#include "hello_imgui/internal/backend_impls/abstract_runner.h"

#include <SDL.h>


namespace HelloImGui
{
    class RunnerSdlOpenGl3 : public AbstractRunner
{
       public:
        RunnerSdlOpenGl3(RunnerParams & runnerParams);
        virtual ~RunnerSdlOpenGl3() = default;

       protected:
        void Impl_InitBackend() override;
        void Impl_Select_Gl_Version() override;
        std::string Impl_GlslVersion() override;
        void Impl_CreateWindow() override;
        void Impl_CreateGlContext() override;
        void Impl_InitGlLoader() override;
        void Impl_SetupPlatformRendererBindings() override;

        void Impl_PollEvents() override;
        void Impl_NewFrame_3D() override;
        void Impl_NewFrame_Backend() override;
        void Impl_Frame_3D_ClearColor() override;
        void Impl_RenderDrawData_To_3D() override;
        void Impl_UpdateAndRenderAdditionalPlatformWindows() override;
        void Impl_Cleanup() override;
        void Impl_SwapBuffers() override;

        ImageBuffer Impl_ScreenshotRgb() override;
    public:
        bool priv_HandleMobileDeviceEvent(unsigned int sdl_EventType);

       private:
        SDL_GLContext mGlContext = nullptr;
    };

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_SDL_OPENGL3
