#include <QtImGui.h>
#include <imgui.h>
#include <QGuiApplication>
#include <QTimer>
#include <QSurfaceFormat>
#include <QOpenGLWindow>
#include <QOpenGLExtraFunctions>

#include "imgui_runner.h"

namespace ImGuiRunner {

namespace DetailsQt {

void SetOpenGlVersion()
{
    // Use OpenGL 3 Core Profile, when available
    QSurfaceFormat glFormat;
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        glFormat.setVersion(3, 3);
        glFormat.setProfile(QSurfaceFormat::CoreProfile);
    }
    QSurfaceFormat::setDefaultFormat(glFormat);
}

void BeginFullWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin(
        "##FullWindowHiddenTitle",
        nullptr,
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar);
}

class ImGuiQtRunnerWindow : public QOpenGLWindow, private QOpenGLExtraFunctions
{
   public:
    ImGuiQtRunnerWindow(
        const GuiLoopWithExitFunction &guiLoopFunction,
        const AppWindowParams &params)
        : gui_loop_function(guiLoopFunction), window_params(params)
    {
        setTitle(QString::fromStdString(params.window_title));
        if (params.show_fullscreen)
            showFullScreen();
        else if (params.show_maximized)
            showMaximized();
        else
        {
            resize(static_cast<int>(params.window_width), static_cast<int>(params.window_height));
            show();
        }
    }

    void closeIfRequired()
    {
        if (shall_close)
            close();
    }

   protected:
    void initializeGL() override
    {
        initializeOpenGLFunctions();
        QtImGui::initialize(this);
    }

    void paintGL() override
    {
        bool quit_required = false;
        QtImGui::newFrame();

        if (window_params.provide_full_background_window)
            BeginFullWindow();

        if (gui_loop_function)
            quit_required = gui_loop_function();

        if (window_params.provide_full_background_window)
            ImGui::End();

        // Do render before ImGui UI is rendered
        glViewport(0, 0, width(), height());
        glClearColor(window_params.clear_color.x, window_params.clear_color.y, window_params.clear_color.z, window_params.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui::Render();

        if (quit_required)
            this->shall_close = true;
        //destroy();
        //emit close();
    }

   private:
    GuiLoopWithExitFunction gui_loop_function;
    AppWindowParams window_params;
    bool shall_close = false;
};

} // namespace DetailsQt


void RunWithExit(
    GuiLoopWithExitFunction guiLoopFunction,
    const AppWindowParams &params
)
{
    DetailsQt::SetOpenGlVersion();

    int dummy_argc = 0;
    char **dummy_argv = nullptr;
    QGuiApplication a(dummy_argc, dummy_argv);

    // Show window
    DetailsQt::ImGuiQtRunnerWindow window(guiLoopFunction, params);

    auto fn_update_and_close_if_required = [&window] {
        window.update();
        window.closeIfRequired();
    };

    // Update at 60 fps
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, fn_update_and_close_if_required);
    timer.start(16);

    int return_code = a.exec();
    (void)return_code;
}

} // namespace ImGuiRunner
