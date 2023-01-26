#include "opengl_screenshot.h"
#include "imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/internal/pnm.h"


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
}