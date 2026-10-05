#include "runner_glfw3_emscripten.h"
#if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_GLFW3)
#include "hello_imgui/internal/backend_impls/emscripten_pointer_probe.h"
#include "hello_imgui/internal/backend_impls/emscripten_keyboard.h"
#include "hello_imgui/internal/touch_pinch.h"
#include "imgui_internal.h"  // ImGuiInputEvent
#include <iostream>

#include <emscripten.h>

namespace HelloImGui
{
    RunnerGlfw3Emscripten* gRunnerGlfw3Emscripten = nullptr;

    void emscripten_imgui_main_loop_glfw3(void* arg)
    {
        (void) arg;
        gRunnerGlfw3Emscripten->CreateFramesAndRender();
    }

    void RunnerGlfw3Emscripten::Run()
    {
        #if defined(HELLOIMGUI_WITH_TEST_ENGINE) && !defined(HELLOIMGUI_EMSCRIPTEN_PTHREAD)
            printf("RunnerGlfw3Emscripten::Run Disabling useImGuiTestEngine since compiled without pthread\n");
            params.useImGuiTestEngine = false;
        #endif

        gRunnerGlfw3Emscripten = this;
        gRunnerGlfw3Emscripten->Setup();

        emscripten_cancel_main_loop();

        //SDL_GL_SetSwapInterval(1);  // Enable vsync
        glfwSwapInterval(1);  // Enable vsync

        // This function call won't return, and will engage in an infinite loop,
        // processing events from the browser, and dispatching them.
        // int fps = 0; // 0 <=> let the browser decide. This is the recommended way, see
        // https://emscripten.org/docs/api_reference/emscripten.h.html#browser-execution-environment
        emscripten_set_main_loop_arg(emscripten_imgui_main_loop_glfw3, NULL, params.emscripten_fps, true);
    }

    void RunnerGlfw3Emscripten::Impl_InitPlatformBackend()
    {
        RunnerGlfw3::Impl_InitPlatformBackend();
        InstallEmscriptenPointerProbe();
        InstallEmscriptenKeyboard();  // here rather than in Run(): ManualRender does not go through Run()
    }

    void RunnerGlfw3Emscripten::Impl_PollEvents()
    {
        // GLFW cannot tell a finger from a mouse. The GLFW callbacks run inside the browser's event handlers, between
        // two frames, and tagged their events with the source known then: the probe now knows the source of those
        // events (the pointer type of the same gestures), so they are retagged, and the source is set for what follows.
        ImGuiMouseSource source = LastEmscriptenPointerSource();
        ImGuiContext& g = *ImGui::GetCurrentContext();
        for (ImGuiInputEvent& e : g.InputEventsQueue)
        {
            if (e.AddedByTestEngine)  // the touch layer's own events (replayed, with the source they need)
                continue;
            if (e.Type == ImGuiInputEventType_MousePos)
                e.MousePos.MouseSource = source;
            else if (e.Type == ImGuiInputEventType_MouseButton)
                e.MouseButton.MouseSource = source;
            else if (e.Type == ImGuiInputEventType_MouseWheel)
                e.MouseWheel.MouseSource = source;
        }
        ImGui::GetIO().AddMouseSourceEvent(source);
        SetTouchPointers(EmscriptenFingerCount(), EmscriptenPinchScale());
        EmscriptenPushTapZone(ImGui::GetIO().DisplaySize.x);
        UpdateEmscriptenKeyboard();
        RunnerGlfw3::Impl_PollEvents();
    }

    void RunnerGlfw3Emscripten::Impl_NewFrame_PlatformBackend()
    {
        // The imgui GLFW backend's NewFrame re-adds the cursor position every frame while it believes the mouse is
        // outside the window (a finger triggers no mouseenter): on a touch screen, the last position would keep
        // hovering whatever passes under it (the headers lit up one after the other while the content coasted).
        // There is no pointer between two touches: the position is invalid, as when a mouse leaves the window.
        RunnerGlfw3::Impl_NewFrame_PlatformBackend();
        if (LastEmscriptenPointerSource() == ImGuiMouseSource_TouchScreen && !EmscriptenPointerIsDown())
            ImGui::GetIO().AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    void RunnerGlfw3Emscripten::Impl_Select_Gl_Version()
    {
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // Emscripten cannot provide a floating point framebuffer
        params.rendererBackendOptions.requestFloatBuffer = false;
    }

    std::string RunnerGlfw3Emscripten::Impl_GlslVersion() const
    {
        const char* glsl_version = "#version 300 es";
        //const char* glsl_version = "#version 100";
        return glsl_version;
    }

    void RunnerGlfw3Emscripten::Impl_InitGlLoader() {}

}  // namespace HelloImGui

#endif  // #if defined(__EMSCRIPTEN__) && defined(HELLOIMGUI_USE_GLFW3)
