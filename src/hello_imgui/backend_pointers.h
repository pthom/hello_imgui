#pragma once

namespace HelloImGui
{

// @@md#BackendPointers
//
// BackendPointers is a struct that contains optional pointers to the
// backend implementations (for SDL and GLFW).
//
// These pointers will be filled when the application starts, and you can use them
// to customize your application behavior using the selected backend.
//
// Note: If using the Metal, Vulkan or DirectX rendering backend, you can find
// some interesting pointers inside
//     `src/hello_imgui/internal/backend_impls/rendering_metal.h`
//     `src/hello_imgui/internal/backend_impls/rendering_vulkan.h`
//     `src/hello_imgui/internal/backend_impls/rendering_dx11.h`
//     `src/hello_imgui/internal/backend_impls/rendering_dx12.h`
struct BackendPointers
{
    //* `glfwWindow`: Pointer to the main GLFW window (of type `GLFWwindow*`).
    //  Only filled if the backend is GLFW.
    void* glfwWindow     = nullptr; /* GLFWwindow*    */

    //* `sdlWindow`: Pointer to the main SDL window (of type `SDL_Window*`).
    //  Only filled if the backend is SDL (or emscripten + sdl)
    void* sdlWindow      = nullptr; /* SDL_Window*    */

    //* `sdlGlContext`: Pointer to SDL's GlContext (of type `SDL_GLContext`).
    //  Only filled if the backend is SDL (or emscripten + sdl)
    void* sdlGlContext   = nullptr; /* SDL_GLContext  */
};
// @@md

}  // namespace HelloImGui