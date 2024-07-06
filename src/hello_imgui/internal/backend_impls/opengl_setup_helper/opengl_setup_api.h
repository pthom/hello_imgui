#pragma once
#include "hello_imgui/screen_bounds.h"
#include "hello_imgui/renderer_backend_options.h"

#include <string>


namespace HelloImGui { namespace BackendApi
{
    class IOpenGlSetup
    {
    public:
        IOpenGlSetup() {}
        virtual ~IOpenGlSetup() {}

        virtual void SelectOpenGlVersion(const OpenGlOptionsFilled_& options) = 0;
        virtual void InitGlLoader() = 0;
        virtual OpenGlOptionsFilled_ Impl_MakeDefaultOpenGlOptionsForPlatform() = 0;

        OpenGlOptionsFilled_ OpenGlOptionsWithUserSettings();
    };
}} // namespace HelloImGui { namespace BackendApi
