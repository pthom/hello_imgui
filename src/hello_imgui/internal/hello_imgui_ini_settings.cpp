#include "hello_imgui/internal/hello_imgui_ini_settings.h"
#include "hello_imgui/internal/inicpp.h"

namespace HelloImGui
{
    namespace HelloImGuiIniSettings
    {
        void WriteLastRunWindowBounds(const std::string& iniFilename, const ScreenBounds& windowBounds)
        {
            ini::IniFile iniFile;
            try
            {
                iniFile.load(iniFilename);
            }
            catch(const std::exception&)
            {
                fprintf(stderr, "WindowGeometryHelper::WriteLastRunWindowBounds: Corrupt ini file %s\n", iniFilename.c_str());
                iniFile = ini::IniFile();
            }

            iniFile["WIN"]["WindowPosition"] = IntPairToString(windowBounds.position);
            iniFile["WIN"]["WindowSize"] = IntPairToString(windowBounds.size);
            iniFile.save(iniFilename);
        }

        std::optional<ScreenBounds> ReadLastRunWindowBounds(const std::string& iniFilename)
        {
            ini::IniFile iniFile;
            try
            {
                iniFile.load(iniFilename);
            }
            catch(const std::exception&)
            {
                return std::nullopt;
            }

            ScreenBounds screenBounds;
            bool failed = false;

            {
                auto strValue = iniFile["WIN"]["WindowPosition"].as<std::string>();
                auto intPair = StringToIntPair(strValue);
                if (intPair.has_value())
                    screenBounds.position = *intPair;
                else
                    failed = true;
            }
            {
                auto strValue = iniFile["WIN"]["WindowSize"].as<std::string>();
                auto intPair = StringToIntPair(strValue);
                if (intPair.has_value())
                    screenBounds.size = *intPair;
                else
                    failed = true;
            }

            if (failed)
                return std::nullopt;
            else
                return screenBounds;

        }
    } // namespace HelloImGuiIniSettings
} // namespace HelloImGui
