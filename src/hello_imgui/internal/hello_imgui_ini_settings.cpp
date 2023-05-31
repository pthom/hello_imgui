#include "hello_imgui/internal/hello_imgui_ini_settings.h"
#include "hello_imgui/internal/inicpp.h"
#include "hello_imgui/internal/fplus_all_in_one.hpp"
#include "imgui_internal.h"


namespace HelloImGui
{
    namespace HelloImGuiIniSettings
    {
        // Test if a line looks like an IniPart intro, e.g.:
        //      ;;;<<<imgui>>>;;;
        bool _IsIniPartName(const std::string& line)
        {
            return (line.find(";;;<<<")) == 0 && (line.rfind(">>>;;;") == line.size() - 6);
        }

        // Reads an IniPart intro name, e.g.:
        //      ;;;<<<imgui>>>;;;
        // =>
        //      imgui
        std::string _ReadIniPartName(const std::string& line)
        {
            IM_ASSERT(_IsIniPartName(line));
            IM_ASSERT(line.size() > 12);
            std::string name = line.substr(6, line.size() - 12);
            return name;
        }

        IniParts SplitIniParts(const std::string& s)
        {
            IniParts iniParts;
            std::optional<IniParts::IniPart> currentPart;

            auto lines = fplus::split_lines(true, s);
            for (const std::string& line: lines)
            {
                if (_IsIniPartName(line))
                {
                    if (currentPart.has_value())
                        iniParts.Parts.push_back(*currentPart);

                    currentPart = IniParts::IniPart();
                    currentPart->Name = _ReadIniPartName(line);
                }
                else
                    currentPart->Content += line + "\n";
            }

            if (currentPart.has_value())
                iniParts.Parts.push_back(*currentPart);

            return iniParts;
        }

        std::string JoinIniParts(const IniParts& iniParts)
        {
            std::string r;
            for (const auto& iniPart: iniParts.Parts)
            {
                r += ";;;<<<" + iniPart.Name + ">>>;;;\n";
                r += iniPart.Content;
            }
            return r;
        }

        bool  IniParts::HasIniPart(const std::string& name)
        {
            bool r = false;
            for(const auto& iniPart: Parts)
                if (iniPart.Name == name)
                    return true;
            return false;
        }
        std::string IniParts::GetIniPart(const std::string& name)
        {
            IM_ASSERT(HasIniPart(name));
            for(const auto& iniPart: Parts)
                if (iniPart.Name == name)
                    return iniPart.Content;
            return "ERROR call HasIniPart before GetIniPart!!!!";
        }
        void IniParts::SetIniPart(const std::string& name, const std::string& content)
        {
            if (HasIniPart(name))
            {
                for(auto& iniPart: Parts)
                    if (iniPart.Name == name)
                        iniPart.Content = content;

            }
            else
                Parts.push_back(IniPart{name, content});
        }


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

        /*
        namespace details
        {
            std::string AlnumOrUnderscore(const std::string& s)
            {
                std::string r;
                for (char c : s)
                {
                    if (isalnum(c))
                        r += c;
                    else
                        r += "_";
                }
                return r;
            }

            std::string SanitizeIniNameOrCategory(const std::string& s)
            {
                std::stringstream ss;
                ss << ImHashStr(s.c_str()) << "_" << AlnumOrUnderscore(s);
                return ss.str();
            }
        }

        void WriteDockableWindowsVisibility(const std::string& iniFilename, const DockingParams& dockingParams)
        {
            ini::IniFile iniFile;
            try
            {
                iniFile.load(iniFilename);
            }
            catch(const std::exception&)
            {
                fprintf(stderr, "TryReadIniFile: Corrupt ini file %s\n", iniFilename.c_str());
                iniFile = ini::IniFile();
            }


            for (const auto& dockableWindow: dockingParams.dockableWindows)
            {
                if (dockableWindow.saveVisibilityInSettings)
                {
                    std::string iniValueName = details::SanitizeIniNameOrCategory(dockableWindow.label);
                    iniFile["DOCKABLE_VISIBILITY"][iniValueName] = dockableWindow.isVisible;
                }
            }
            iniFile.save(iniFilename);
        }

        void ReadDockableWindowsVisibility(const std::string& iniFilename, DockingParams* inOutDockingParams)
        {
            ini::IniFile iniFile;
            try
            {
                iniFile.load(iniFilename);
            }
            catch(const std::exception&)
            {
                return;
            }

            for (auto& dockableWindow: inOutDockingParams->dockableWindows)
            {
                if (dockableWindow.saveVisibilityInSettings)
                {
                    std::string iniValueName = details::SanitizeIniNameOrCategory(dockableWindow.label);
                    std::string boolString = iniFile["DOCKABLE_VISIBILITY"][iniValueName].as<std::string>();

                    if (boolString == "true")
                        dockableWindow.isVisible = true;
                    if (boolString == "false")
                        dockableWindow.isVisible = false;
                }
            }
        }
        */
    } // namespace HelloImGuiIniSettings
} // namespace HelloImGui
