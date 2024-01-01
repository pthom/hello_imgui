#pragma once
#ifdef HELLOIMGUI_USE_SDL
#include "hello_imgui/internal/backend_impls/abstract_runner.h"

#include <SDL.h>


namespace HelloImGui
{
    class RunnerSdl2 : public AbstractRunner
    {
       public:
        RunnerSdl2(RunnerParams & runnerParams);
        virtual ~RunnerSdl2() = default;

       protected:
        //
        // Methods related to the platform backend (SDL, Glfw, ...)
        //
        void Impl_InitPlatformBackend() override;
        void Impl_CreateWindow() override;
        void Impl_PollEvents() override;
        void Impl_NewFrame_PlatformBackend() override;
        void Impl_UpdateAndRenderAdditionalPlatformWindows() override;
        void Impl_Cleanup() override;
        void Impl_SwapBuffers() override;
        void Impl_SetWindowIcon() override;

        //
        // Linking the platform backend (SDL, Glfw, ...) to the rendering backend (OpenGL, ...)
        //
        void Impl_LinkPlatformAndRenderBackends() override;
        // Specific to OpenGL
        #ifdef HELLOIMGUI_HAS_OPENGL
            void Impl_Select_Gl_Version() override;
            std::string Impl_GlslVersion() override;
            void Impl_CreateGlContext() override;
            void Impl_InitGlLoader() override;
        #endif

        //
        // Rendering backend (OpenGL, ...): Impl_InitRenderBackendCallbacks will fill the callbacks
        //
        void Impl_InitRenderBackendCallbacks() override;


    public:
        bool priv_HandleMobileDeviceEvent(unsigned int sdl_EventType);

       private:
        SDL_GLContext mGlContext = nullptr;
    };

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_SDL
