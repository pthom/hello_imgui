#include "hello_imgui/backend_api/backend_api.h"
#include <memory>
#include <iostream>

#define BACKEND_API_USE_GLFW


#ifdef BACKEND_API_USE_GLFW

#include <GLFW/glfw3.h>


namespace BackendApi
{
    class BackendGlfwWindow: public IBackendWindow
    {
    public:
        GLFWwindow *mWindow;
        BackendGlfwWindow(GLFWwindow *window): mWindow(window) {}
        ~BackendGlfwWindow() {}
    };


    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }


    class BackendGlfw: public IBackend
    {
        BackendGlfwWindow* GetBackendGlfwWindow(IBackendWindow *bw);
        GLFWwindow* GetGlfwWindow(IBackendWindow *bw);

    public:
        void Init() override;
        void DeInit() override;

        IBackendWindow* CreateWindow(WindowOptions &info) override;
        void DestroyWindow(IBackendWindow *window) override;

        size_t GetNbMonitors() override;
        ScreenBounds GetOneMonitorWorkArea(int monitorIndex) override;

        bool IsWindowIconified(IBackendWindow *window) override;
        bool ShouldWindowClose(IBackendWindow *window) override;
        void RaiseWindow(IBackendWindow *window) override;

        ScreenBounds GetWindowBounds(IBackendWindow *window) override;
        void SetWindowBounds(IBackendWindow *window, ScreenBounds windowBounds) override;

        void WaitForEvent(IBackendWindow *window, int timeOutMilliseconds) override;
        void PollEvents(IBackendWindow *window, const AnyEventCallback &anyEventCallback) override;
    };

#endif // #ifdef BACKEND_API_USE_GLFW
}

