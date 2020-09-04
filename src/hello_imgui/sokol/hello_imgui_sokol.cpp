#ifdef HELLOIMGUI_USE_SOKOL
#include "hello_imgui/sokol/hello_imgui_sokol.h"
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_time.h"
#include "sokol/sokol_glue.h"
#include "imgui.h"
#include "sokol/util/sokol_imgui.h"

#include "hello_imgui/runner_params.h"
#include "hello_imgui/internal/runner_functions.h"

namespace HelloImGui
{
namespace Sokol
{

namespace details
{

static HelloImGui::RunnerParams *gRunnerParams = nullptr;

static uint64_t last_time = 0;
static sg_pass_action pass_action;

static void set_bg_color()
{
    // initial clear color
    pass_action.colors[0].action = SG_ACTION_CLEAR;
    pass_action.colors[0].val[0] = gRunnerParams->imGuiWindowParams.backgroundColor.x;
    pass_action.colors[0].val[1] = gRunnerParams->imGuiWindowParams.backgroundColor.y;
    pass_action.colors[0].val[2] = gRunnerParams->imGuiWindowParams.backgroundColor.z;
    pass_action.colors[0].val[3] = gRunnerParams->imGuiWindowParams.backgroundColor.w;
}

static void init(void)
{
    // setup sokol-gfx, sokol-time and sokol-imgui
    sg_desc desc = { };
    desc.context = sapp_sgcontext();
    sg_setup(&desc);
    stm_setup();

    // use sokol-imgui with all default-options (we're not doing
    // multi-sampled rendering or using non-default pixel formats)
    simgui_desc_t simgui_desc = { };
    simgui_desc.no_default_font = false; // we should set this to true if we want to load font awesome. However this leads to some bugs
    simgui_setup(&simgui_desc);

    set_bg_color();
    HelloImGui::RunnerFunctions::PrepareApp(*gRunnerParams);
}

static void cleanup(void)
{
    simgui_shutdown();
    sg_shutdown();
}

static void input(const sapp_event* event)
{
    if (event && event->type == SAPP_EVENTTYPE_SUSPENDED)
        HelloImGui::RunnerFunctions::OnPause(*gRunnerParams);
    if (event && event->type == SAPP_EVENTTYPE_RESUMED)
        HelloImGui::RunnerFunctions::OnResume(*gRunnerParams);

    simgui_handle_event(event);
}

static void new_frame_backend()
{
    const int width = sapp_width();
    const int height = sapp_height();
    const double delta_time = stm_sec(stm_laptime(&last_time));
    simgui_new_frame(width, height, delta_time);
}

static void render_drawdata_to_3D()
{
    // the sokol_gfx draw pass
    const int width = sapp_width();
    const int height = sapp_height();
    sg_begin_default_pass(&pass_action, width, height);
    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void frame(void)
{
    if (gRunnerParams->appShallExit)
        sapp_quit();
    set_bg_color();
    new_frame_backend();
    HelloImGui::RunnerFunctions::RenderGui(*gRunnerParams);
    render_drawdata_to_3D();
}

} // namespace details

sapp_desc make_sapp_desc(HelloImGui::RunnerParams& params)
{
    details::gRunnerParams = &params;

    sapp_desc desc = { };
    desc.init_cb = details::init;
    desc.frame_cb = details::frame;
    desc.cleanup_cb = details::cleanup;
    desc.event_cb = details::input;

    desc.width = (int)(params.appWindowParams.windowSize.x);
    desc.height = (int)(params.appWindowParams.windowSize.y);
    desc.window_title = details::gRunnerParams->appWindowParams.windowTitle.c_str();

    desc.gl_force_gles2 = true;
    desc.ios_keyboard_resizes_canvas = false;

    return desc;
}


} // namespace Sokol
} // namespace HelloImGui



#endif // HELLOIMGUI_USE_SOKOL
