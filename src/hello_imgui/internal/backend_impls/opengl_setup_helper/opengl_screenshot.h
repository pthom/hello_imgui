#pragma once
#ifdef HELLOIMGUI_HAS_OPENGL
#include "imgui.h"
#include "hello_imgui/hello_imgui_screenshot.h"


namespace HelloImGui
{
    ImageBuffer OpenglScreenshotRgb();

    bool ImGuiApp_ImplGL_CaptureFramebuffer(ImGuiID viewport_id, int x, int y, int w, int h, unsigned int* pixels, void* user_data);
}
#endif
