#pragma once

struct SDL_Renderer;
struct SDL_Window;

struct GLFWwindow;


namespace HelloImGui
{
/**
 @@md#BackendPointers

**BackendPointers** is a struct that contains optional pointers to the backend implementations (for SDL and GLFW).

These pointers will be filled when the application starts, and you can use them to customize
your application behavior using the selected backend.

 Members:
* `glfwWindow`: GLFWwindow *, default=nullptr_. Pointer to the main GLFW window.
  Only filled if the backend is GLFW.
* `sdlWindow`: SDL_Window *, default=nullptr_. Pointer to the main SDL window.
  Only filled if the backend is SDL (or emscripten + sdl)
* `sdlGlContext`: _void *, default=nullptr_. Pointer to SDL's GlContext (of type `SDL_GLContext`).
  Only filled if the backend is SDL + OpenGL (or emscripten + sdl).

Note: if using the Metal rendering backend, you can get additional pointers inside `hello_imgui/internal/backend_impls/rendering_metal.h`.
@@md
 */
struct BackendPointers
{
    GLFWwindow *   glfwWindow     = nullptr;

    SDL_Window *    sdlWindow      = nullptr;
    void *          sdlGlContext   = nullptr;
};

}  // namespace HelloImGui