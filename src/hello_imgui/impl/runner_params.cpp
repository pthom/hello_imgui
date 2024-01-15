#include "hello_imgui/runner_params.h"

#include <filesystem>

namespace HelloImGui
{
    RunnerParams SimpleRunnerParams::ToRunnerParams() const
    {
        auto& self = *this;
        RunnerParams r;

        r.callbacks.ShowGui = self.guiFunction;

        r.appWindowParams.windowGeometry.size = self.windowSize;
        r.appWindowParams.windowGeometry.sizeAuto = self.windowSizeAuto;
        r.appWindowParams.restorePreviousGeometry = self.windowRestorePreviousGeometry;

        r.appWindowParams.windowTitle = self.windowTitle;

        r.fpsIdling.fpsIdle = self.fpsIdle;
        r.fpsIdling.enableIdling = self.enableIdling;

        return r;
    }


    // IniSettingsLocation returns the path to the ini file for the application settings.
    std::string IniSettingsLocation(const RunnerParams& runnerParams)
    {
        auto _getIniFileName = [& runnerParams]() -> std::string
        {
            auto _stringToSaneFilename=[](const std::string& s, const std::string& extension) -> std::string
            {
                std::string filenameSanitized;
                for (char c : s)
                    if (isalnum(c))
                        filenameSanitized += c;
                    else
                        filenameSanitized += "_";
                filenameSanitized += extension;
                return filenameSanitized;
            };

            if (! runnerParams.iniFilename.empty())
                return runnerParams.iniFilename;
            else
            {
                if (runnerParams.iniFilename_useAppWindowTitle && !runnerParams.appWindowParams.windowTitle.empty())
                {
                    std::string iniFilenameSanitized = _stringToSaneFilename(runnerParams.appWindowParams.windowTitle, ".ini");
                    return iniFilenameSanitized;
                }
                else
                    return "imgui.ini";
            }
        };

        auto mkdirToFilename = [](const std::string& filename) -> bool
        {
            std::filesystem::path p(filename);
            std::filesystem::path dir = p.parent_path();
            if (dir.empty())
                return true;
            if (std::filesystem::exists(dir))
            {
                if (std::filesystem::is_directory(dir) || std::filesystem::is_symlink(dir))
                    return true;
                else
                    return false;
            }
            else
                return std::filesystem::create_directories(dir);
        };


        std::string iniFilename = _getIniFileName();
        std::string folder = HelloImGui::IniFolderLocation(runnerParams.iniFolderType);

        std::string iniFullFilename = folder.empty() ? iniFilename : folder + "/" + iniFilename;
        bool settingsDirIsAccessible = mkdirToFilename(iniFullFilename);
        IM_ASSERT(settingsDirIsAccessible);

        return iniFullFilename;
    }

    // DeleteIniSettings deletes the ini file for the application settings.
    void DeleteIniSettings(const RunnerParams& runnerParams)
    {
        std::string iniFullFilename = IniSettingsLocation(runnerParams);
        if (iniFullFilename.empty())
            return;
        if (!std::filesystem::exists(iniFullFilename))
            return;
        bool success = std::filesystem::remove(iniFullFilename);
        IM_ASSERT(success && "Failed to delete ini file %s");
    }

    // HasIniSettings returns true if the ini file for the application settings exists.
    bool HasIniSettings(const RunnerParams& runnerParams)
    {
        std::string iniFullFilename = IniSettingsLocation(runnerParams);
        if (iniFullFilename.empty())
            return false;
        return std::filesystem::exists(iniFullFilename);
    }

}  // namespace HelloImGui