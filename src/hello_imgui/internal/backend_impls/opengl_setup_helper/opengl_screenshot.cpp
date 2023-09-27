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


    bool ImGuiApp_ImplGL_CaptureFramebuffer(ImGuiID viewport_id, int x, int y, int w, int h, unsigned int* pixels, void* user_data)
    {
        (void)viewport_id;
        (void)user_data;

#ifdef __linux__
        // FIXME: Odd timing issue is observed on linux (Plasma/X11 specifically), which causes outdated frames to be captured, unless we give compositor some time to update screen.
    // glFlush() didn't seem enough. Will probably need to revisit that.
    usleep(1000);   // 1ms
#endif

        int y2 = (int)ImGui::GetIO().DisplaySize.y - (y + h);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(x, y2, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        // Flip vertically
        size_t comp = 4;
        size_t stride = (size_t)w * comp;
        unsigned char* line_tmp = new unsigned char[stride];
        unsigned char* line_a = (unsigned char*)pixels;
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
        return true;
    }

}