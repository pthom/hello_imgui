// Demo of EDR (Extended Dynamic Range) support in Hello ImGui, on the OpenGL3 + Glfw backend.
//
// This requires a version of Glfw which defines GLFW_FLOATBUFFER (no stock Glfw release does yet;
// HDR-enabling forks such as https://github.com/Tom94/glfw do).
// With a stock Glfw, this demo still runs, but on a standard 8 bits framebuffer.
//
// Note: contrary to the Metal backend, hasEdrSupport() cannot be used here (it returns false):
// on OpenGL, the framebuffer format is negotiated at window creation time. Instead, we set
// requestFloatBuffer = true, and read it back after Run() has created the window.

#ifdef HELLOIMGUI_HAS_OPENGL3
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include <memory>
#include <vector>


struct ImageEdr
{
    ImageEdr(int width, int height)
    {
        Width = width;
        Height = height;
        ImageData.resize(Width * Height * 4, 0.f);
    }

    // Buffer to store the image data, as RGBA float
    // (it will be uploaded to a GL_RGBA16F texture)
    std::vector<float> ImageData;
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
            imageEdr->ImageData[index + 0] = r;
            imageEdr->ImageData[index + 1] = g;
            imageEdr->ImageData[index + 2] = b;
            imageEdr->ImageData[index + 3] = 1.0f;
        }
    }
}


struct AppState
{
    float maxR = 1.0f, maxG = 1.0f, maxB = 1.0f;

    ImageEdr imageEdr = ImageEdr(512, 512);
    GLuint textureGl = 0;

    // Info about the framebuffer we actually obtained
    GLint framebufferComponentType = 0;
    GLint framebufferRedBits = 0;

    AppState()
    {
        glGenTextures(1, &textureGl);
        glBindTexture(GL_TEXTURE_2D, textureGl);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Update();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glGetFramebufferAttachmentParameteriv(
            GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &framebufferComponentType);
        glGetFramebufferAttachmentParameteriv(
            GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &framebufferRedBits);
        printf("hello_edr_opengl: requestFloatBuffer read back as %s / framebuffer: %d bits per channel, %s\n",
               HelloImGui::GetRunnerParams()->rendererBackendOptions.requestFloatBuffer ? "true" : "false",
               framebufferRedBits,
               framebufferComponentType == GL_FLOAT ? "float" : "integer");
        fflush(stdout);
    }

    ~AppState()
    {
        glDeleteTextures(1, &textureGl);
    }

    void Update()
    {
        CreateFloatPattern(&imageEdr, maxR, maxG, maxB);
        glBindTexture(GL_TEXTURE_2D, textureGl);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, imageEdr.Width, imageEdr.Height,
                     0, GL_RGBA, GL_FLOAT, imageEdr.ImageData.data());
    }

    ImTextureID TextureID()
    {
        return (ImTextureID)(intptr_t)textureGl;
    }
};


void Gui(AppState& appState)
{
    bool gotFloatBuffer = HelloImGui::GetRunnerParams()->rendererBackendOptions.requestFloatBuffer;
    if (gotFloatBuffer)
        ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "Floating point framebuffer obtained!");
    else
        ImGui::TextColored(ImVec4(1.f, 0.5f, 0.f, 1.f), "No floating point framebuffer, using a standard one.");
    ImGui::Text("Framebuffer: %d bits per channel, component type: %s",
                appState.framebufferRedBits,
                appState.framebufferComponentType == GL_FLOAT ? "float" : "integer");

    ImGui::Separator();
    ImGui::TextWrapped(
        "The image below is a GL_RGBA16F texture, i.e. RGBA 16 bits per channel, float\n"
        "If your screen supports EDR (Extended Dynamic Range), you can experiment with setting\n"
        "the maxR, maxG, maxB values to values > 1.0f: they should appear brighter than white.\n");
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

    // Request a floating point framebuffer: HelloImGui will reset this to false
    // if it cannot be obtained (we display the outcome in the GUI).
    runnerParams.rendererBackendOptions.requestFloatBuffer = true;

    // AppState can be instantiated only after OpenGL is initialized, and must be destroyed before exit
    std::unique_ptr<AppState> appState;
    runnerParams.callbacks.EnqueuePostInit([&]() { appState = std::make_unique<AppState>(); });
    runnerParams.callbacks.EnqueueBeforeExit([&]() { appState.reset(); });

    runnerParams.callbacks.ShowGui = [&]() { Gui(*appState); };
    HelloImGui::Run(runnerParams);
    return 0;
}

#else // HELLOIMGUI_HAS_OPENGL3
#include <cstdio>
int main() { printf("This demo requires the OpenGL3 rendering backend.\n"); return 0; }
#endif
