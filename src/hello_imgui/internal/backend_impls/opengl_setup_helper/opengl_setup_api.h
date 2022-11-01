#pragma once
#include "internal/backend_impls/backend_window_helper/screen_bounds.h"

#include <string>


namespace BackendApi
{
    class IOpenGlSetup
    {
    public:
        IOpenGlSetup() {}
        virtual ~IOpenGlSetup() {}

        virtual void SelectOpenGlVersion() = 0;
        virtual void InitGlLoader() = 0;
        virtual std::string GlslVersion() = 0;
    };
} // namespace BackendApi
