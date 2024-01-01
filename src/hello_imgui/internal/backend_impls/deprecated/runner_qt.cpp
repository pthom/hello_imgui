#ifdef HELLOIMGUI_USE_QT
#include <QGuiApplication>
#include <QOpenGLExtraFunctions>
#include <QOpenGLWindow>
#include <QSurfaceFormat>
#include <QTimer>
#include <QtImGui.h>
#include <imgui.h>

#include "runner_qt.h"

namespace HelloImGui
{
namespace DetailsQt
{
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
        ImGui::Begin("##FullWindowHiddenTitle",
                     nullptr,
                     ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar);
    }
}  // namespace DetailsQt

class RunnerQt_WindowImpl : public QOpenGLWindow, private QOpenGLExtraFunctions
{
   public:
    RunnerQt_WindowImpl(RunnerQt* runnerQt) : runnerQt_(runnerQt)
    {
        setTitle(QString::fromStdString(runnerQt->params.appWindowParams.windowTitle));
        if (runnerQt->params.appWindowParams.fullScreen)
            showFullScreen();
        else if (runnerQt->params.appWindowParams.maximized)
            showMaximized();
        else
        {
            resize(static_cast<int>(runnerQt->params.appWindowParams.windowSize[0]),
                   static_cast<int>(runnerQt->params.appWindowParams.windowSize[1]));
            if (runnerQt->params.appWindowParams.windowPosition.x >= -10000.f)
            {
                setPosition(static_cast<int>(runnerQt->params.appWindowParams.windowPosition[0]),
                            static_cast<int>(runnerQt->params.appWindowParams.windowPosition[1]));
            }
            show();
        }
    }

    ~RunnerQt_WindowImpl() {
    }

    void closeIfRequired()
    {
        if (runnerQt_->params.appShallExit)
            close();
    }

    void ApplyClearColor()
    {
        auto color = runnerQt_->params.imGuiWindowParams.backgroundColor;
        glViewport(0, 0, width(), height());
        glClearColor(color.x, color.y, color.z, color.w);
        glClear(GL_COLOR_BUFFER_BIT);
    }

   protected:
    void initializeGL() override
    {
        initializeOpenGLFunctions();
        QtImGui::initialize(this);
    }

    void paintGL() override
    {
        QtImGui::newFrame();

        ApplyClearColor();

        runnerQt_->RenderGui();

        ImGui::Render();
    }

   private:
    RunnerQt* runnerQt_;
};

RunnerQt::RunnerQt(RunnerParams& runnerParams) : AbstractRunner(runnerParams) {
}

RunnerQt::~RunnerQt() = default;

void RunnerQt::Run()
{
    Setup();
    RunQGuiApplication();
    TearDown();
}

void RunnerQt::RunQGuiApplication()
{
    int dummy_argc = 0;
    char **dummy_argv = nullptr;
    QGuiApplication a(dummy_argc, dummy_argv);
    DetailsQt::SetOpenGlVersion();

    impl_ = std::make_unique<RunnerQt_WindowImpl>(this);

    // Show window
    auto fn_update_and_close_if_required = [this] {
        impl_->update();
        impl_->closeIfRequired();
    };

    // Update at 60 fps
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, fn_update_and_close_if_required);
    timer.start(16);

    int return_code = a.exec();
    (void)return_code;

    impl_.reset();
}


void RunnerQt::Impl_InitPlatformBackend()
{
}

void RunnerQt::Impl_Select_Gl_Version()
{
}

std::string RunnerQt::Impl_GlslVersion()
{
    IM_ASSERT(false); //RunnerQt::Impl_GlslVersion should not be called
}

void RunnerQt::Impl_CreateWindow()
{
}

void RunnerQt::Impl_CreateGlContext()
{
}

void RunnerQt::Impl_InitGlLoader()
{
}

void RunnerQt::Impl_SetupPlatformRendererBindings()
{
}

void RunnerQt::Impl_PollEvents()
{
}

void RunnerQt::Impl_NewFrame_3D()
{
}

void RunnerQt::Impl_NewFrame_PlatformBackend()
{
}

void RunnerQt::Impl_Frame_3D_ClearColor()
{
}

void RunnerQt::Impl_RenderDrawData_To_3D()
{
}

void RunnerQt::Impl_UpdateAndRenderAdditionalPlatformWindows()
{
}

void RunnerQt::Impl_Cleanup()
{
}

void RunnerQt::Impl_SwapBuffers()
{
}

}  // namespace HelloImGui

#endif  // #ifdef HELLOIMGUI_USE_QT
