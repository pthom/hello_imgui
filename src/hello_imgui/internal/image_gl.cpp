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
    this->imTextureId = (ImTextureID)(intptr_t)textureId_Gl;
}

ImageGl::ImageGl(const char *assetPath, bool isGif)
{
    this->isGif = isGif;
    if(isGif)
    {
        auto assetData = LoadAssetFileData(assetPath);
        assert(assetData.data != nullptr);

        int image_width, image_height, image_fps;
        int req_comp = 4;
        unsigned char*image_data = stbi_load_gif_from_memory(
            (unsigned char *)assetData.data, (int)assetData.dataSize,
            NULL, &image_width, &image_height, &image_fps, NULL, req_comp);
        assert(image_data != nullptr);
        GLuint* textureId_Gl_Array = new GLuint[image_fps];
        for (int i = 0; i < image_fps; i++)
        {
            glGenTextures(1, &textureId_Gl_Array[i]);
            glBindTexture(GL_TEXTURE_2D, textureId_Gl_Array[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if defined(HELLOIMGUI_USE_GLES2) || defined(HELLOIMGUI_USE_GLES3)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 image_width,
                 image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data + i * image_width * image_height * req_comp);
        }
        this->Gif_Fps = image_fps;
        this->imageSize = ImVec2((float)image_width, (float) image_height);
        this->imTextureId_GifArray = (ImTextureID*)(intptr_t)textureId_Gl_Array;
    }
    else
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
        this->imTextureId = (ImTextureID)(intptr_t)textureId_Gl;
        this->imTextureId_StaticImage = &this->imTextureId;
        }
}

ImageGl::~ImageGl()
{
    if(this->isGif)
    {
        for(int i = 0; i < this->Gif_Fps; i++)
        {
            GLuint textureId_Gl = (GLuint)(intptr_t)imTextureId_GifArray[i];
            glDeleteTextures(1, &textureId_Gl);
        }
    }
    else
    {
        GLuint textureId_Gl = (GLuint)(intptr_t)imTextureId;
        glDeleteTextures(1, &textureId_Gl);
    }
}


ImVec2 ImageProportionalSize(const ImVec2& askedSize, const ImVec2& imageSize)
{
    ImVec2 r(askedSize);

    if ((r.x == 0.f) && (r.y == 0.f))
        r = imageSize;
    else if (r.y == 0.f)
        r.y = imageSize.y / imageSize.x * r.x;
    else if (r.x == 0.f)
        r.x = imageSize.x / imageSize.y * r.y;
    return r;
}


void ImageGl::Draw(
    const ImVec2& size,
    const ImVec2& uv0,
    const ImVec2& uv1,
    const ImVec4& tint_col,
    const ImVec4& border_col)
{
    ImVec2 displayedSize = ImageProportionalSize(size, this->imageSize);
    ImGui::Image(this->imTextureId, displayedSize, uv0, uv1, tint_col, border_col);
}

void ImageGl::Draw(
    bool isGif,
    const ImVec2& size,
    const ImVec2& uv0,
    const ImVec2& uv1,
    const ImVec4& tint_col,
    const ImVec4& border_col)
{
    if(!isGif)
        return Draw(size, uv0, uv1, tint_col, border_col);
    this->Gif_Time+=0.1;
    if((int)Gif_Time>=this->Gif_Fps/2)
        Gif_Time=0.0f;
    ImVec2 displayedSize = ImageProportionalSize(size, this->imageSize);
    ImGui::Image(this->imTextureId_GifArray[(int)this->Gif_Time], displayedSize, uv0, uv1, tint_col, border_col);
}

bool ImageGl::DrawButton(
    const ImVec2& size,
    const ImVec2& uv0,
    const ImVec2& uv1,
    int frame_padding,
    const ImVec4& bg_col,
    const ImVec4& tint_col)
{
    ImVec2 displayedSize = ImageProportionalSize(size, this->imageSize);
    return ImGui::ImageButton(this->imTextureId, displayedSize, uv0, uv1, frame_padding, bg_col, tint_col);
}

bool ImageGl::DrawButton(
    bool isGif,
    const ImVec2& size,
    const ImVec2& uv0,
    const ImVec2& uv1,
    int frame_padding,
    const ImVec4& bg_col,
    const ImVec4& tint_col)
{
    if(!isGif)
        return DrawButton(size, uv0, uv1, frame_padding, bg_col, tint_col);
    this->Gif_Time += 0.1;
    if((int)Gif_Time >= this->Gif_Fps/2)
        Gif_Time = 0.0f;
    ImVec2 displayedSize = ImageProportionalSize(size, this->imageSize);
    return ImGui::ImageButton(this->imTextureId_GifArray[(int)this->Gif_Time], displayedSize, uv0, uv1, frame_padding, bg_col, tint_col);
}

ImageGlPtr ImageGl::FactorImage(const char *assetPath)
{
    return std::unique_ptr<ImageGl>( new ImageGl(assetPath) );
}

ImageGlPtr ImageGl::FactorImage(const char *assetPath, bool isGif)
{
    return std::unique_ptr<ImageGl>( new ImageGl(assetPath, isGif) );
}

} // namespace HelloImGui

#endif // #ifdef HELLOIMGUI_HAS_OPENGL
