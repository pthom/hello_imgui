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
            std::string r = ";;; !!! This configuration is handled by HelloImGui and stores several Ini Files, separated by markers like this:\n           ;;;<<<INI_NAME>>>;;;\n\n";
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


        IniParts IniParts::LoadFromFile(const std::string& iniPartsFilename)
        {
            std::string iniPartsContent = fplus::read_text_file(iniPartsFilename)();
            auto iniParts = SplitIniParts(iniPartsContent);
            return iniParts;
        }

        void IniParts::WriteToFile(const std::string& iniPartsFilename)
        {
            std::string iniPartsContent = JoinIniParts(*this);
            fplus::write_text_file(iniPartsFilename, iniPartsContent)();
        }

        void SaveLastRunWindowBounds(const std::string& iniPartsFilename, const ScreenBounds& windowBounds)
        {
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename);

            ini::IniFile iniFile;
            iniFile["AppWindow"]["WindowPosition"] = IntPairToString(windowBounds.position);
            iniFile["AppWindow"]["WindowSize"] = IntPairToString(windowBounds.size);
            std::string iniContent = iniFile.encode();

            iniParts.SetIniPart("AppWindow", iniContent);
            iniParts.WriteToFile(iniPartsFilename);
        }

        std::optional<ScreenBounds> LoadLastRunWindowBounds(const std::string& iniPartsFilename)
        {
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename);

            if (!iniParts.HasIniPart("AppWindow"))
                return std::nullopt;


            auto iniPartContent = iniParts.GetIniPart("AppWindow");
            ini::IniFile iniFile;
            try
            {
                iniFile.decode(iniPartContent);
            }
            catch(const std::exception&)
            {
                return std::nullopt;
            }

            ScreenBounds screenBounds;
            bool failed = false;

            {
                auto strValue = iniFile["AppWindow"]["WindowPosition"].as<std::string>();
                auto intPair = StringToIntPair(strValue);
                if (intPair.has_value())
                    screenBounds.position = *intPair;
                else
                    failed = true;
            }
            {
                auto strValue = iniFile["AppWindow"]["WindowSize"].as<std::string>();
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

        void LoadImGuiSettings(const std::string& iniPartsFilename)
        {
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename);
            if (!iniParts.HasIniPart("ImGui"))
                return;
            auto imguiSettingsContent = iniParts.GetIniPart("ImGui");
            ImGui::LoadIniSettingsFromMemory(imguiSettingsContent.c_str());
        }

        void SaveImGuiSettings(const std::string& iniPartsFilename)
        {
            std::string imguiSettingsContent = ImGui::SaveIniSettingsToMemory();

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename);
            iniParts.SetIniPart("ImGui", imguiSettingsContent);
            iniParts.WriteToFile(iniPartsFilename);
        }


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

        void SaveDockableWindowsVisibility(const std::string& iniPartsFilename, const DockingParams& dockingParams)
        {
            ini::IniFile iniFile;
            for (const auto& dockableWindow: dockingParams.dockableWindows)
            {
                if (dockableWindow.saveIsVisible)
                {
                    std::string iniValueName = details::SanitizeIniNameOrCategory(dockableWindow.label);
                    iniFile["Visibility"][iniValueName] = dockableWindow.isVisible;
                }
            }

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename);
            iniParts.SetIniPart("Layout_Default", iniFile.encode());
            iniParts.WriteToFile(iniPartsFilename);
        }

        void LoadDockableWindowsVisibility(const std::string& iniPartsFilename, DockingParams* inOutDockingParams)
        {
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename);
            if (! iniParts.HasIniPart("Layout_Default"))
                return;

            ini::IniFile iniFile;
            iniFile.decode(iniParts.GetIniPart("Layout_Default"));
            for (auto& dockableWindow: inOutDockingParams->dockableWindows)
            {
                if (dockableWindow.saveIsVisible)
                {
                    std::string iniValueName = details::SanitizeIniNameOrCategory(dockableWindow.label);
                    std::string boolString = iniFile["Visibility"][iniValueName].as<std::string>();

                    if (boolString == "true")
                        dockableWindow.isVisible = true;
                    if (boolString == "false")
                        dockableWindow.isVisible = false;
                }
            }
        }
    } // namespace HelloImGuiIniSettings
} // namespace HelloImGui
