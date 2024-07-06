#include "opengl_setup_api.h"
#include "hello_imgui/hello_imgui_include_opengl.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/hello_imgui_ini_any_parent_folder.h"

#include <string>


namespace HelloImGui { namespace BackendApi
{
    void ModifyOpenGlOptionsFromUserSettingsInParams(OpenGlOptionsFilled_ *openGlOptions)
    {
        OpenGlOptions& userOptions =HelloImGui::GetRunnerParams()->rendererBackendOptions.openGlOptions;

        if (userOptions.MajorVersion.has_value())
            openGlOptions->MajorVersion = userOptions.MajorVersion.value();
        if (userOptions.MinorVersion.has_value())
            openGlOptions->MinorVersion = userOptions.MinorVersion.value();
        if (userOptions.UseCoreProfile.has_value())
            openGlOptions->UseCoreProfile = userOptions.UseCoreProfile.value();
        if (userOptions.UseForwardCompat.has_value())
            openGlOptions->UseForwardCompat = userOptions.UseForwardCompat.value();
        if (userOptions.AntiAliasingSamples.has_value())
            openGlOptions->AntiAliasingSamples = userOptions.AntiAliasingSamples.value();
    }


    void ModifyOpenGlOptionsFromUserSettingsInHelloImGuiIniFile(OpenGlOptionsFilled_ *openGlOptions)
    {
        // cf renderer_backend_options.h:
        //
        // OpenGlOptions contains advanced options used at the startup of OpenGL.
        // These parameters are reserved for advanced users.
        // By default, Hello ImGui will select reasonable default values, and these parameters are not used.
        // Use at your own risk, as they make break the multi-platform compatibility of your application!
        // All these parameters are platform dependent.
        // For real multiplatform examples, see
        //     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_glfw.cpp
        // and
        //     hello_imgui/src/hello_imgui/internal/backend_impls/opengl_setup_helper/opengl_setup_sdl.cpp
        //
        // How to set those values manually:
        // ---------------------------------
        // you may set them manually:
        //    (1) Either by setting them programmatically in your application
        //        (set their values in `runnerParams.rendererBackendOptions.openGlOptions`)
        //    (2) Either by setting them in a `hello_imgui.ini` file. See hello_imgui/hello_imgui_example.ini for more info
        //    Note: if several methods are used, the order of priority is (1) > (2)

        std::string openGlSection = "OpenGlOptions";

        auto majorVersion = HelloImGuiIniAnyParentFolder::readIntValue(openGlSection, "MajorVersion");
        if (majorVersion.has_value())
            openGlOptions->MajorVersion = majorVersion.value();

        auto minorVersion = HelloImGuiIniAnyParentFolder::readIntValue(openGlSection, "MinorVersion");
        if (minorVersion.has_value())
            openGlOptions->MinorVersion = minorVersion.value();

        auto useCoreProfile = HelloImGuiIniAnyParentFolder::readBoolValue(openGlSection, "UseCoreProfile");
        if (useCoreProfile.has_value())
            openGlOptions->UseCoreProfile = useCoreProfile.value();

        auto useForwardCompat = HelloImGuiIniAnyParentFolder::readBoolValue(openGlSection, "UseForwardCompat");
        if (useForwardCompat.has_value())
            openGlOptions->UseForwardCompat = useForwardCompat.value();

        auto glslVersion = HelloImGuiIniAnyParentFolder::readStringValue(openGlSection, "GlslVersion");
        if (glslVersion.has_value())
            openGlOptions->GlslVersion = glslVersion.value();

        auto antiAliasingSamples = HelloImGuiIniAnyParentFolder::readIntValue(openGlSection, "AntiAliasingSamples");
        if (antiAliasingSamples.has_value())
            openGlOptions->AntiAliasingSamples = antiAliasingSamples.value();
    }

    OpenGlOptionsFilled_ IOpenGlSetup::OpenGlOptionsWithUserSettings()
    {
        // We compute this only once
        static OpenGlOptionsFilled_ openGlOptions;
        static bool wasFilled = false;
        if (! wasFilled)
        {
            openGlOptions = Impl_MakeDefaultOpenGlOptionsForPlatform();
            ModifyOpenGlOptionsFromUserSettingsInHelloImGuiIniFile(&openGlOptions);
            ModifyOpenGlOptionsFromUserSettingsInParams(&openGlOptions);
            wasFilled = true;
        }

        OpenGlOptionsFilled_ r = openGlOptions;
        return r;
    }

}} // namespace HelloImGui { namespace BackendApi
