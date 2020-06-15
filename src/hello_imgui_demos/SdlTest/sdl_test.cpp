#include "SDL.h"

#ifdef IOS
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#else
#include <glad/glad.h>          // Needs to be initialized with gladLoadGL() in user's code.
#endif

#include <sstream>

void absdl()
{
    //Impl_InitBackend();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::stringstream msg;
        msg << "RunnerSdlOpenGl3::Impl_InitBackend error " << SDL_GetError();
        throw std::runtime_error(msg.str().c_str());
    }

    //Impl_Select_Gl_Version();
    {
        // Decide GL+GLSL versions
        // GL 3.2 Core + GLSL 150
        // const char* glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // Always required on Mac
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    }


    //Impl_CreateWindowAndContext();
    {
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        //const auto &backendWindowParams = params.appWindowParams;
        //ImVec2 windowPosition = backendWindowParams.windowPosition;
        //ImVec2 windowSize = backendWindowParams.windowSize;
        SDL_WindowFlags window_flags =
            (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        int xPos = SDL_WINDOWPOS_CENTERED, yPos = SDL_WINDOWPOS_CENTERED;
        SDL_Window* mWindow = SDL_CreateWindow("MY WINDOW",
                                   xPos,
                                   yPos,
                                   800,
                                   600,
                                   window_flags);


        SDL_GLContext mGlContext = SDL_GL_CreateContext(mWindow);
        if (!mGlContext)
            throw std::runtime_error("RunnerSdlOpenGl3::Impl_CreateWindowAndContext(): Failed to initialize WebGL context!");

        SDL_GL_MakeCurrent(mWindow, mGlContext);
        SDL_GL_SetSwapInterval(1);  // Enable vsync
    }


    //Impl_InitGlLoader();
    {
    }


//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//    params.callbacks.SetupImGuiConfig();
//    params.callbacks.SetupImGuiStyle();
//    Impl_SetupPlatformRendererBindings();
//    params.callbacks.LoadAdditionalFonts();
//    DockingDetails::ConfigureImGuiDocking(params.imGuiWindowParams);
//    params.callbacks.PostInit();

}


int main(int argc, char *argv[])
{
    absdl();
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialize SDL\n");
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_EGL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_DisplayMode displayMode;
    SDL_GetDesktopDisplayMode(0, &displayMode);

    SDL_Window* window = SDL_CreateWindow("TESTGL", 0, 0, 640, 1150,

                                          SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE);

    SDL_GLContext gl = SDL_GL_CreateContext(window);

    glClearColor ( 1.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT );
    SDL_GL_SwapWindow(window);

    SDL_Event event;
    auto done = false;
    while (!done)
    {

        SDL_PumpEvents();
        while (SDL_PollEvent(&event))
        {

            switch (event.type)
            {
                case SDL_QUIT:
                    done = true;
                    break;
            }
        }
    }

    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

}