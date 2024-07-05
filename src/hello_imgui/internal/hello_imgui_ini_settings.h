#pragma once

#include "hello_imgui/screen_bounds.h"
#include "hello_imgui/hello_imgui.h"

#include <string>
#include <optional>


namespace HelloImGui
{
    namespace HelloImGuiIniSettings
    {
        /*
         HelloImGui stores its settings in a file that groups several ini file, and looks like this

            ;;;<<<imgui>>>;;;
            [Window][Main window (title bar invisible)]
            Pos=0,0
            Size=1000,800
            Collapsed=0

            [Window][Debug##Default]
            Pos=60,60
            Size=400,400
            Collapsed=0

            [Docking][Data]

            ;;;<<<appWindow>>>;;;
            [WIN]
            WindowPosition=393,238
            WindowSize=971,691

            ;;;<<<otherIniInfo>>>;;;
            ...
         */

        struct IniParts
        {
            struct IniPart
            {
                std::string Name;
                std::string Content;
            };

            bool           HasIniPart(const std::string& name);
            std::string    GetIniPart(const std::string& name);
            void           SetIniPart(const std::string& name, const std::string& content);

            static IniParts LoadFromFile(const std::string& iniPartsFilename);
            void            WriteToFile(const std::string& iniPartsFilename);

            std::vector<IniPart> Parts;
        };
        IniParts SplitIniParts(const std::string& s);
        std::string JoinIniParts(const IniParts& parts);

        //
        // The settings below are global to the app
        //
        void SaveLastRunWindowBounds(const std::string& iniPartsFilename, const ScreenBounds& windowBounds);
        std::optional<ScreenBounds> LoadLastRunWindowBounds(const std::string& iniPartsFilename);
        std::optional<float> LoadLastRunDpiWindowSizeFactor(const std::string& iniPartsFilename);
        void SaveHelloImGuiMiscSettings(const std::string& iniPartsFilename, const RunnerParams& runnerParams);
        void LoadHelloImGuiMiscSettings(const std::string& iniPartsFilename, RunnerParams* inOutRunnerParams);

        //
        // The settings below are saved with values that can differ from layout to layout
        //
        void LoadImGuiSettings(const std::string& iniPartsFilename, const std::string& layoutName);
        void SaveImGuiSettings(const std::string& iniPartsFilename, const std::string& layoutName);
        bool HasUserDockingSettingsInImguiSettings(const std::string& iniPartsFilename, const DockingParams& dockingParams);

        void SaveDockableWindowsVisibility(const std::string& iniPartsFilename, const DockingParams& dockingParams);
        void LoadDockableWindowsVisibility(const std::string& iniPartsFilename, DockingParams* inOutDockingParams);

        //
        // User prefs
        //
        void        SaveUserPref(const std::string& iniPartsFilename, const std::string& userPrefName, const std::string& userPrefContent);
        std::string LoadUserPref(const std::string& iniPartsFilename, const std::string& userPrefName);

    }
}
