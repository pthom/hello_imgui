#pragma once
#include "imgui.h"
#include "hello_imgui/hello_imgui_screenshot.h"


namespace HelloImGui
{
    ImageBuffer OpenglScreenshotRgb();

    // Screenshot for imgui_test_engine
    // Inspired from imgui_test_engine: imgui_app.cpp
    bool ImGuiApp_ImplGL_CaptureFramebuffer(ImGuiID viewport_id, int x, int y, int w, int h, unsigned int* pixels, void* user_data);
}