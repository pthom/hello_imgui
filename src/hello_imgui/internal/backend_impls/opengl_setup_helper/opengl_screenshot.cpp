#include "opengl_screenshot.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/internal/pnm.h"


namespace HelloImGui
{
    ImageBuffer OpenglScreenshotRgb()
    {
        int depth = 3;

        GLint glDimensions[4] = {0};
        glGetIntegerv(GL_SCISSOR_BOX, glDimensions);

        auto r = ImageBuffer();
        r.width = glDimensions[2];
        r.height = glDimensions[3];

        size_t bufferSize= r.width * r.height * depth;
        r.bufferRgb.resize(bufferSize);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(
            glDimensions[0], glDimensions[1],
            glDimensions[2], glDimensions[3],
            GL_RGB, GL_UNSIGNED_BYTE,
            r.bufferRgb.data());

        // Invert rows, since OpenGL (0,0) is at the bottomLeft
        for(std::size_t y = 0; y <= r.height / 2; ++y)
        {
            int yd = r.height - 1 - y;
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