#pragma once
#ifdef HELLOIMGUI_HAS_OPENGL

#if defined(HELLOIMGUI_USE_GLES3)
    #if defined(__APPLE__)
        #include <TargetConditionals.h>
        #if TARGET_OS_IOS || TARGET_OS_SIMULATOR
            #include <OpenGLES/ES3/gl.h>
            #include <OpenGLES/ES3/glext.h>
        #else
            #include <GLES3/gl3.h>
            #include <GLES3/gl3ext.h>
        #endif
    #elif defined(__EMSCRIPTEN__)
        #include <GLES3/gl3.h>
        //#include <GLES3/gl2ext.h>
    #else
        #include <GLES3/gl3.h>
        #include <GLES3/gl3ext.h>
    #endif
#elif defined(HELLOIMGUI_USE_GLES2)
    #if defined(__APPLE__)
        #include <TargetConditionals.h>
        #if TARGET_OS_IOS || TARGET_OS_SIMULATOR
            #include <OpenGLES/ES2/gl.h>
            #include <OpenGLES/ES2/glext.h>
        #else
            #include <GLES2/gl2.h>
            #include <GLES2/gl2ext.h>
        #endif
    #else
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
    #endif
#elif defined(HELLOIMGUI_USE_GLAD)
#include <glad/glad.h>
#endif

#endif // #ifdef HELLOIMGUI_HAS_OPENGL