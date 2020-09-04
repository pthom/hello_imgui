#if defined(HELLOIMGUI_USE_SOKOL) && !defined(__APPLE__)
#define SOKOL_IMPL
#include "sokol/sokol_app.h"
#include "sokol/sokol_gfx.h"
#include "sokol/sokol_time.h"
#include "sokol/sokol_audio.h"
#include "sokol/sokol_glue.h"
#include "imgui.h"
#define SOKOL_IMGUI_IMPL
#include "sokol/util/sokol_imgui.h"
#endif
