#pragma once

#include "hello_imgui/screen_bounds.h"
#include "hello_imgui/docking_params.h"

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

            std::vector<IniPart> Parts;
        };
        IniParts SplitIniParts(const std::string& s);
        std::string JoinIniParts(const IniParts& parts);

        void WriteLastRunWindowBounds(const std::string& iniFilename, const ScreenBounds& windowBounds);
        std::optional<ScreenBounds> ReadLastRunWindowBounds(const std::string& iniFilename);

        // void WriteDockableWindowsVisibility(const std::string& iniFilename, const DockingParams& dockingParams);
        // void ReadDockableWindowsVisibility(const std::string& iniFilename, DockingParams* inOutDockingParams);
    }
}
