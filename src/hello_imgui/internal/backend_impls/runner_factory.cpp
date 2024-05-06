#include "hello_imgui/internal/backend_impls/runner_factory.h"

#include "hello_imgui/internal/backend_impls/runner_glfw3.h"
#include "hello_imgui/internal/backend_impls/runner_sdl2.h"
#include "hello_imgui/internal/backend_impls/runner_sdl_emscripten.h"
#include "hello_imgui/internal/backend_impls/runner_null.h"

namespace HelloImGui
{

void ChooseBackendTypesIfSelectedAsFirstAvailable(RunnerParams* runnerParams)
{
    if (runnerParams->platformBackendType == PlatformBackendType::FirstAvailable)
    {
        #if defined(HELLOIMGUI_USE_GLFW3)
            runnerParams->platformBackendType = PlatformBackendType::Glfw;
        #elif defined(HELLOIMGUI_USE_SDL2)
            runnerParams->platformBackendType = PlatformBackendType::Sdl;
        #else
            runnerParams->platformBackendType = PlatformBackendType::Null;
        #endif
    }

    if (runnerParams->rendererBackendType == RendererBackendType::FirstAvailable)
    {
        #if defined(HELLOIMGUI_HAS_OPENGL)
            runnerParams->rendererBackendType = RendererBackendType::OpenGL3;
        #elif defined(HELLOIMGUI_HAS_METAL)
            runnerParams->rendererBackendType = RendererBackendType::Metal;
        #elif defined(HELLOIMGUI_HAS_VULKAN)
            runnerParams->rendererBackendType = RendererBackendType::Vulkan;
        #elif defined(HELLOIMGUI_HAS_DIRECTX11)
            runnerParams->rendererBackendType = RendererBackendType::DirectX11;
        #elif defined(HELLOIMGUI_HAS_DIRECTX12)
            runnerParams->rendererBackendType = RendererBackendType::DirectX12;
        #else
            runnerParams->rendererBackendType = RendererBackendType::Null;
        #endif
    }
}

void ChooseNullBackendsIfUsingRemote(RunnerParams* runnerParams)
{
    #ifdef HELLOIMGUI_WITH_REMOTE_DISPLAY
    if (runnerParams->remoteParams.enableRemoting)
    {
        runnerParams->platformBackendType = PlatformBackendType::Null;
        runnerParams->rendererBackendType = RendererBackendType::Null;
    }
    #endif
}


std::unique_ptr<AbstractRunner> FactorRunner(RunnerParams& params)
{
    ChooseBackendTypesIfSelectedAsFirstAvailable(&params);
    ChooseNullBackendsIfUsingRemote(&params);
    if (params.platformBackendType == PlatformBackendType::Glfw)
    {
        #ifdef HELLOIMGUI_USE_GLFW3
            return std::make_unique<RunnerGlfw3>(params);
        #else
            return nullptr;
        #endif
    }
    else if (params.platformBackendType == PlatformBackendType::Sdl)
    {
        #if defined(__EMSCRIPTEN__)
            return std::make_unique<RunnerSdlEmscripten>(params);
        #elif defined(HELLOIMGUI_USE_SDL2)
            return std::make_unique<RunnerSdl2>(params);
        #else
            return nullptr;
        #endif
    }
    else if (params.platformBackendType == PlatformBackendType::Null)
    {
        return std::make_unique<RunnerNull>(params);
    }
    else
        return nullptr;
}

}  // namespace HelloImGui
