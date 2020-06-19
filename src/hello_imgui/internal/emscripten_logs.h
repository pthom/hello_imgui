#pragma once

#ifdef __EMSCRIPTEN__
#include <iostream>
#define EMS_RUN_SHOW(...)                 \
{                                         \
    std::cout << "EMS_RUN_SHOW" << "\n";  \
    std::cout << #__VA_ARGS__ << "\n";    \
    std::cout << __VA_ARGS__ << "\n\n";   \
}

#define EMS_LOG(...) \
{\
    std::cout << "EMS_LOG: " << __VA_ARGS__ << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
}

#define EMS_LOG_VALUE(value) \
{\
    std::cout << "EMS_LOG_VALUE: " << #value << "=" << value << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
}

#define EMS_LOG_POINTER(value) \
{\
    std::cout << "EMS_LOG_POINTEr: " << #value << "=" << (size_t)(void *)value << "\t\t at " << __FILE__ << ":" << __LINE__ << "\n"; \
}

#else
#define EMS_RUN_SHOW(...) {}
#define EMS_LOG(...) {}
#define EMS_LOG_VALUE(value) {}
#define EMS_LOG_POINTER(value) {}
#endif
