#ifdef HELLOIMGUI_HAS_OPENGL
#include "opengl_screenshot.h"
#include "imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/internal/pnm.h"

#ifdef __linux__
#include <unistd.h>
#endif


namespace HelloImGui
{
    ImageBuffer OpenglScreenshotRgb()
    {
        auto draw_data = ImGui::GetDrawData();
        int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
        int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);

        int depth = 3;

        auto r = ImageBuffer();
        r.width = fb_width;
        r.height = fb_height;

        size_t bufferSize= r.width * r.height * depth;
        r.bufferRgb.resize(bufferSize);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(
            0, 0,
            fb_width, fb_height,
            GL_RGB, GL_UNSIGNED_BYTE,
            r.bufferRgb.data());

        // Invert rows, since OpenGL (0,0) is at the bottomLeft
        for(std::size_t y = 0; y <= r.height / 2; ++y)
        {
            int yd = (int)r.height - 1 - (int)y;
            for(std::size_t x = 0; x < r.width * depth; ++x)
            {
                std::swap(r.bufferRgb[y * r.width * depth + x], r.bufferRgb[yd * r.width * depth + x]);
            }
        }

        if (false)
        {
            // Save as ppm
            pnm::image<pnm::rgb_pixel> ppm(r.width, r.height);
            for(std::size_t y=0; y<ppm.y_size(); ++y)
            {
                for(std::size_t x=0; x<ppm.x_size(); ++x)
                {
                    uint8_t * pos = & r.bufferRgb[(y * r.width + x) * depth];
                    ppm[y][x] = pnm::rgb_pixel(pos[0], pos[1], pos[2]);
                }
            }
            pnm::write("/tmp/out.ppm", ppm, pnm::format::binary);
        }

        return r;
    }


    //
    // Screenshot for imgui_test_engine
    // Inspired from imgui_test_engine: imgui_app.cpp
    //


    void _glCaptureFramebuffer(
        int x, int y, int w, int h,
        float frameBufferScaleY,    // We now need to know the frameBufferScaleY to be able to flip the y coordinate into y2
        unsigned int* pixels)
    {
#ifdef __linux__
        // FIXME: Odd timing issue is observed on linux (Plasma/X11 specifically), which causes outdated frames to be captured, unless we give compositor some time to update screen.
    // glFlush() didn't seem enough. Will probably need to revisit that.
    usleep(1000);   // 1ms
#endif
        int y2 = (int)((float)ImGui::GetIO().DisplaySize.y * frameBufferScaleY - (float)(y + h));
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(x, y2, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        // Flip vertically
        size_t comp = 4;
        size_t stride = (size_t)w * comp;
        auto* line_tmp = new unsigned char[stride];
        auto* line_a = (unsigned char*)pixels;
        unsigned char* line_b = (unsigned char*)pixels + (stride * ((size_t)h - 1));
        while (line_a < line_b)
        {
            memcpy(line_tmp, line_a, stride);
            memcpy(line_a, line_b, stride);
            memcpy(line_b, line_tmp, stride);
            line_a += stride;
            line_b -= stride;
        }
        delete[] line_tmp;
    }

    bool ImGuiApp_ImplGL_CaptureFramebuffer(ImGuiID viewport_id, int x, int y, int w, int h, unsigned int* pixels, void* user_data)
    {
        IM_UNUSED(viewport_id);
        IM_UNUSED(user_data);

        ImVec2 framebufferScale(1., 1.f);
        if (ImGui::GetDrawData() != nullptr)
            framebufferScale = ImGui::GetDrawData()->FramebufferScale; // WARNING: Sometimes GetDrawData() will return NULL
                                                                       //  when invoked from the CaptureTool window!
        // framebufferScale = ImVec2(2.f, 2.f);                        // Manual hack for when GetDrawData() returns null

        // Are we using a scaled frame buffer (for example on macOS with retina screen)
        bool hasFramebufferScale = (framebufferScale.x != 1.f) || (framebufferScale.y != 1.f);

        // if not using scaled frame buffer, perform simple capture
        if (!hasFramebufferScale)
        {
            _glCaptureFramebuffer(x, y, w, h, framebufferScale.y, pixels);
            return true;
        }

        //
        // else
        //

        // 1. Capture to temporary buffer capturePixels
        auto x_to_scaled = [framebufferScale](int _x) -> int { return (int)((float)_x * framebufferScale.x); };
        auto y_to_scaled = [framebufferScale](int _y) -> int { return (int)((float)_y * framebufferScale.y); };

        int xs = x_to_scaled(x), ys = y_to_scaled(y), ws = x_to_scaled(w), hs = y_to_scaled(h);

        auto* capturePixels = new unsigned int[ws * hs];
        _glCaptureFramebuffer(xs, ys, ws, hs, framebufferScale.y, capturePixels);

        // 2. Fill pixel from capturePixels: an atrocious and slow loop
        auto get_capture_pixel = [&](int _x, int _y) -> unsigned int {
            int _xs = x_to_scaled(_x), _ys = y_to_scaled(_y);
            return capturePixels[_ys * ws + _xs];
        };
        for (int _y = 0; _y < h; ++_y)
            for (int _x = 0; _x < w; ++_x)
                pixels[_y * w + _x] = get_capture_pixel(_x, _y);

        delete[] capturePixels;
        return true;
    }


}
#endif // HELLOIMGUI_HAS_OPENGL