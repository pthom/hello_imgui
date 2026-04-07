#include "hello_imgui/texture_gpu.h"


namespace HelloImGui
{
    // Out-of-line virtual destructor: keeps the vtable pinned to this TU
    // and gives the linker a single home for the type info.
    TextureGpu::~TextureGpu() = default;
}
