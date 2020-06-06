#pragma once
#ifdef HELLOIMGUI_USE_SDL
#include "hello_imgui_runner/abstract_runner/abstract_runner.h"

#include <SDL.h>

namespace HelloImGui
{
    class RunnerSdl : public AbstractRunner
{
       public:
        virtual ~RunnerSdl() = default;

       protected:
        void Impl_InitBackend() override;
        void Impl_Select_Gl_Version() override;
        std::string Impl_GlslVersion() override;
        void Impl_CreateWindowAndContext() override;
        void Impl_InitGlLoader() override;
        void Impl_SetupPlatformRendererBindings() override;

        bool Impl_PollEvents() override;
        void Impl_NewFrame_3D() override;
        void Impl_NewFrame_Backend() override;
        void Impl_Frame_3D_ClearColor() override;
        void Impl_RenderDrawData_To_3D() override;
        void Impl_UpdateAndRenderAdditionalPlatformWindows() override;
        void Impl_Cleanup() override;
        void Impl_SwapBuffers() override;

       private:
        SDL_Window* mWindow = nullptr;
        SDL_GLContext mGlContext = nullptr;
    };

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_SDL
