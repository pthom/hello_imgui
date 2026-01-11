#pragma once

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

// Platform detection: define HELLOIMGUI_IS_DESKTOP_PLATFORM for desktop platforms only
// (excludes mobile platforms iOS/Android and web platform Emscripten)
#if !defined(__EMSCRIPTEN__) && !(defined(__APPLE__) && TARGET_OS_IOS) && !defined(__ANDROID__)
    #define HELLOIMGUI_IS_DESKTOP_PLATFORM
#endif
