// Demo of EDR (Extended Dynamic Range) support in Hello ImGui.

#ifdef HELLOIMGUI_HAS_METAL
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/image_metal.h"
#include <memory>

#include <cstdint>

namespace Float16_Emulation
{
uint16_t float32_to_float16(float value)
{
    uint32_t floatInt = *((uint32_t*)&value);
    uint16_t sign = (floatInt & 0x80000000) >> 16;
    uint16_t exponent = ((floatInt & 0x7F800000) >> 23) - 127 + 15;
    uint16_t mantissa = (floatInt & 0x007FFFFF) >> 13;
    return sign | (exponent << 10) | mantissa;
}

float float16_to_float32(uint16_t value)
{
    uint32_t sign = (value & 0x8000) << 16;
    uint32_t exponent = ((value & 0x7C00) >> 10) + 127 - 15;
    uint32_t mantissa = (value & 0x03FF) << 13;
    uint32_t floatInt = sign | (exponent << 23) | mantissa;
    return *((float*)&floatInt);
}
} // namespace Float16_Emulation


struct ImageEdr
{
    ImageEdr(int width, int height)
    {
        Width = width;
        Height = height;
        ImageData.resize(Width * Height * 4);
        for (int i = 0; i < Width * Height * 4; i++)
        {
            ImageData[i] = Float16_Emulation::float32_to_float16(0.0f);
        }
    }

    // Buffer to store the image data, in format MTLPixelFormatRGBA16Float
    // i.e. RGBA 16 bits per channel, float
    // However, since C++ does not have a float16 type, we store the data as uint16_t
    std::vector<uint16_t> ImageData;
    int Width, Height;
};


void CreateFloatPattern(ImageEdr* imageEdr, float maxR, float maxG, float maxB)
{
    // TopLeft color will be (0, 0, 0)
    // TopRight color will be (maxR, 0, 0)
    // BottomLeft color will be (0, maxG, 0)
    // BottomRight color will be (0, 0, maxB)
    for (int y = 0; y < imageEdr->Height; y++)
    {
        float yf = (float)y / (float)imageEdr->Height;
        for (int x = 0; x < imageEdr->Width; x++)
        {
            float xf = (float)x / (float)imageEdr->Width;
            float r = xf * maxR;
            float g = yf * maxG;
            float b = (1.0f - xf) * maxB;
            int index = (y * imageEdr->Width + x) * 4;
            imageEdr->ImageData[index + 0] = Float16_Emulation::float32_to_float16(r);
            imageEdr->ImageData[index + 1] = Float16_Emulation::float32_to_float16(g);
            imageEdr->ImageData[index + 2] = Float16_Emulation::float32_to_float16(b);
            imageEdr->ImageData[index + 3] = Float16_Emulation::float32_to_float16(1.0f);
        }
    }
}


struct AppState
{
    float maxR = 1.0f, maxG = 1.0f, maxB = 1.0f;

    ImageEdr imageEdr = ImageEdr(512, 512);
    HelloImGui::ImageMetal imageMetal;

    AppState()
    {
        Update();
    }

    void Update()
    {
        CreateFloatPattern(&imageEdr, maxR, maxG, maxB);
        imageMetal.StoreTextureFloat16Rgba(imageEdr.Width, imageEdr.Height, imageEdr.ImageData.data());
    }


    ImTextureID TextureID()
    {
        return imageMetal.TextureID();
    }
};


void Gui(AppState& appState)
{

    ImGui::TextWrapped(
        "The image below is of format MTLPixelFormatRGBA16Float, i.e. RGBA 16 bits per channel, float\n"
        "If your screen support EDR(Extended Dynamic Range), you can experience with setting\n"
        "the maxR, maxG, maxB values to values > 1.0f\n");
    bool changed = false;
    changed |= ImGui::SliderFloat("maxR", &appState.maxR, 0.0f, 2.5f);
    changed |= ImGui::SliderFloat("maxG", &appState.maxG, 0.0f, 2.5f);
    changed |= ImGui::SliderFloat("maxB", &appState.maxB, 0.0f, 2.5f);

    if (changed)
        appState.Update();

    ImGui::Image(appState.TextureID(), ImVec2(appState.imageEdr.Width, appState.imageEdr.Height));
}


int main()
{
    HelloImGui::RunnerParams runnerParams;

    if (HelloImGui::hasEdrSupport())
    {
        runnerParams.rendererBackendOptions.requestFloatBuffer = true;
        printf("EDR support detected, enabling it.\n");
    }
    else
    {
        printf("EDR support not detected, exit.\n");
        return -1;
    }

    // AppState can be instantiated only after Metal is initialized, and must be destroyed before Metal is destroyed
    std::unique_ptr<AppState> appState;
    runnerParams.callbacks.EnqueuePostInit([&]() { appState = std::make_unique<AppState>(); });
    runnerParams.callbacks.EnqueueBeforeExit([&]() { appState.reset(); });

    runnerParams.callbacks.ShowGui= [&]() { Gui(*appState); };
    HelloImGui::Run(runnerParams);
    return 0;
}

#endif
