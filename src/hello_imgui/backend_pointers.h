#pragma once

namespace HelloImGui
{

/**
 @@md#BackendPointers

**BackendPointers** is a struct that contains optional pointers to the backend implementations (for SDL and GLFW).

These pointers will be filled when the application starts, and you can use them to customize
your application behavior using the selected backend.

 Members:
* `glfwWindow`: _void *, default=nullptr_. Pointer to the main GLFW window (of type `GLFWwindow*`).
  Only filled if the backend is GLFW.
* `sdlWindow`: _void *, default=nullptr_. Pointer to the main SDL window (of type `SDL_Window*`).
  Only filled if the backend is SDL (or emscripten + sdl)
* `sdlGlContext`: _void *, default=nullptr_. Pointer to SDL's GlContext (of type `SDL_GLContext`).
  Only filled if the backend is SDL (or emscripten + sdl)

@@md
 */
struct BackendPointers
{
  /* GLFWwindow*    */    void* glfwWindow     = nullptr;

  /* SDL_Window*    */    void* sdlWindow      = nullptr;
  /* SDL_GLContext  */    void* sdlGlContext   = nullptr;
};

}  // namespace HelloImGui