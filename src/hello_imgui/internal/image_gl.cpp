#ifdef HELLOIMGUI_HAS_OPENGL

#include "hello_imgui/hello_imgui_include_opengl.h"
#define STB_IMAGE_IMPLEMENTATION
#include "hello_imgui/internal/stb_image.h"
#include "hello_imgui/hello_imgui_assets.h"
#include "hello_imgui/image_gl.h"

namespace HelloImGui
{

ImageGl::ImageGl(const char *assetPath)
{
    auto assetData = LoadAssetFileData(assetPath);
    assert(assetData.data != nullptr);

    int image_width, image_height;
    unsigned char*image_data = stbi_load_from_memory(
        (unsigned char *)assetData.data, (int)assetData.dataSize,
        &image_width, &image_height, NULL, 4);
    GLuint textureId_Gl;
    glGenTextures(1, &textureId_Gl);
    glBindTexture(GL_TEXTURE_2D, textureId_Gl);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if defined(HELLOIMGUI_USE_GLES2) || defined(HELLOIMGUI_USE_GLES3)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image_width,
                 image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    this->imageSize = ImVec2((float)image_width, (float) image_height);
    this->imTextureId = (void*)(intptr_t)textureId_Gl;
}

ImageGl::~ImageGl()
{
    GLuint textureId_Gl = (GLuint)(intptr_t)imTextureId;
    glDeleteTextures(1, &textureId_Gl);
}

void ImageGl::Draw(
    const ImVec2& size,
    const ImVec2& uv0,
    const ImVec2& uv1,
    const ImVec4& tint_col,
    const ImVec4& border_col)
{
    ImVec2 size_(size);
    if (size.x == 0.f)
        size_ = this->imageSize;
    ImGui::Image(this->imTextureId, size_, uv0, uv1, tint_col, border_col);
}

bool ImageGl::DrawButton(
    const ImVec2& size,
    const ImVec2& uv0,
    const ImVec2& uv1,
    int frame_padding,
    const ImVec4& bg_col,
    const ImVec4& tint_col)
{
    ImVec2 size_(size);
    if (size.x == 0.f)
        size_ = this->imageSize;
    return ImGui::ImageButton(this->imTextureId, size_, uv0, uv1, frame_padding, bg_col, tint_col);
}

ImageGlPtr ImageGl::FactorImage(const char *assetPath)
{
    return std::unique_ptr<ImageGl>( new ImageGl(assetPath) );
}

} // namespace HelloImGui

#endif // #ifdef HELLOIMGUI_HAS_OPENGL
