#pragma once
#ifdef __EMSCRIPTEN__

#include "hello_imgui/backend_impls/runner_sdl.h"

namespace HelloImGui
{
    class RunnerEmscripten : public RunnerSdl
    {
       public:
        virtual ~RunnerEmscripten() = default;
        void Run() override;

       protected:
        void Select_Gl_Version() override;
        std::string GlslVersion() override;
        void InitGlLoader() override;
        void SetupImgGuiContext() override;
    };

}  // namespace HelloImGui

#endif  // #ifdef __EMSCRIPTEN__
