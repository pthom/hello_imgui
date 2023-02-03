#pragma once
#include <stdexcept>
#include <iostream>
#include "imgui.h"


#define HIMG_ERROR(msg) \
    { \
        std::cerr << "HelloImGui ERROR: " << msg << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
        IM_ASSERT(false); \
    }

#ifdef __EMSCRIPTEN__
// Log utilities for emscripten, where the best debug tool is printf

#include <iostream>

#define HIMG_LOG(...) \
{\
    std::cout << "HIMG_LOG: " << __VA_ARGS__ << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
}

#define HIMG_LOG_VALUE(...) \
{\
    std::cout << "HIMG_LOG_VALUE: " << #__VA_ARGS__ << "=" << (__VA_ARGS__) << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
}

#define HIMG_LOG_POINTER(value) \
{\
    std::cout << "HIMG_LOG_POINTEr: " << #value << "=" << (size_t)(void *)value << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
}

#else
#define HIMG_LOG(...) {}
#define HIMG_LOG_VALUE(...) {}
#define HIMG_LOG_POINTER(value) {}
#endif
