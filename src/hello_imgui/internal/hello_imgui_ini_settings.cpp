#include "hello_imgui/internal/hello_imgui_ini_settings.h"
#include "hello_imgui/internal/inicpp.h"
#include "hello_imgui/internal/functional_utils.h"
#include "imgui_internal.h"
#include <map>
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <cstdio>
#else
#include <filesystem>
#endif


namespace HelloImGui
{
    // Encapsulated in docking_details.cpp
    namespace SplitIdsHelper
    {
        std::string SaveSplitIds();
        void LoadSplitIds(const std::string&);
    }


    std::string IntPairToString(std::array<int, 2> v);
    std::optional<std::array<int, 2>> StringToIntPair(const std::string& s);

    namespace HelloImGuiIniSettings
    {
        namespace details
        {
            std::string AlnumOrUnderscore(const std::string& s)
            {
                std::string r;

                for (char c : s)
                {
                    if (isalnum(static_cast<unsigned char>(c)))
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

            bool _stringEndsWith(std::string const &fullString, std::string const &ending)
            {
                if (fullString.length() >= ending.length())
                    return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
                else
                    return false;
            };

            bool _stringStartsWith(std::string const &fullString, std::string const &start)
            {
                if (fullString.length() >= start.length())
                    return (0 == fullString.compare(0, start.length(), start));
                else
                    return false;
            };

            std::string _windowNameInImguiIniLine(const std::string& line)
            {
                // Search for a line like
                //     [Window][Commands]
                // And return "Commands"
                std::string token = "[Window][";
                if (line.empty())
                    return "";
                if (line[line.size() - 1] != ']')
                    return "";
                if (!_stringStartsWith(line, token))
                    return "";

                std::string windowName = line.substr(token.size(), line.size() - token.size() - 1);
                return windowName;
            }

        }


        // Test if a line looks like an IniPart intro, e.g.:
        //      ;;;<<<imgui>>>;;;
        // (";;;<<<>>>;;;", with no name, is not one)
        bool _IsIniPartName(const std::string& line)
        {
            return line.size() > 12 && (line.find(";;;<<<")) == 0 && (line.rfind(">>>;;;") == line.size() - 6);
        }

        // Decodes an ini part; false for a malformed one (a hand edit, a save cut short), which the loaders ignore:
        // the app then starts as if nothing was saved, instead of failing at each start
        bool _TryDecode(ini::IniFile& iniFile, const std::string& iniPartContent)
        {
            try
            {
                iniFile.decode(iniPartContent);
                return true;
            }
            catch (const std::exception&)
            {
                return false;
            }
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

            auto lines = FunctionalUtils::split_lines(s);
            // A text that ends with "\n" (as the file does) gives an empty last line: it is not a line of the last part
            // (else each read and write of the file would add a "\n" to that part)
            if (!lines.empty() && lines.back().empty())
                lines.pop_back();
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
                {
                    if (currentPart.has_value())
                        currentPart->Content += line + "\n";
                }
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
            std::string iniPartsContent = FunctionalUtils::read_text_file_or_empty(iniPartsFilename);
            auto iniParts = SplitIniParts(iniPartsContent);
            return iniParts;
        }

        void IniParts::WriteToFile(const std::string& iniPartsFilename)
        {
            std::string iniPartsContent = JoinIniParts(*this);
            FunctionalUtils::write_text_file(iniPartsFilename, iniPartsContent);
        }

        void SaveLastRunWindowBounds(const std::optional<std::string>& iniPartsFilenameOpt, const ScreenBounds& windowBounds)
        {
            if (!iniPartsFilenameOpt.has_value())
                return;

            const std::string& iniPartsFilename = iniPartsFilenameOpt.value();
            auto& dpiAwareParams = HelloImGui::GetRunnerParams()->dpiAwareParams;
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename);

            ini::IniFile iniFile;
            iniFile["AppWindow"]["WindowPosition"] = IntPairToString(windowBounds.position);
            iniFile["AppWindow"]["WindowSize"] = IntPairToString(windowBounds.size);
            iniFile["AppWindow"]["DpiWindowSizeFactor"] = dpiAwareParams.dpiWindowSizeFactor;
            std::string iniContent = iniFile.encode();

            iniParts.SetIniPart("AppWindow", iniContent);
            iniParts.WriteToFile(iniPartsFilename);
        }

        std::optional<ScreenBounds> LoadLastRunWindowBounds(const std::optional<std::string>& iniPartsFilenameOpt)
        {
            if (!iniPartsFilenameOpt.has_value())
                return std::nullopt;

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilenameOpt.value());

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

            if (iniFile.find("AppWindow") == iniFile.end())
                return std::nullopt;
            auto & appWindowSection = iniFile["AppWindow"];

            // Read Window Position
            {
                if (appWindowSection.find("WindowPosition") == appWindowSection.end())
                    return std::nullopt;
                auto strValue = appWindowSection["WindowPosition"].as<std::string>();
                auto intPair = StringToIntPair(strValue);
                // Positions may be negative, e.g. on a monitor left of or above the primary monitor
                if (intPair.has_value())
                    screenBounds.position = intPair.value();
                else
                    failed = true;
            }
            // Read Window Size
            {
                if (appWindowSection.find("WindowSize") == appWindowSection.end())
                    return std::nullopt;
                auto strValue = appWindowSection["WindowSize"].as<std::string>();
                auto intPair = StringToIntPair(strValue);
                if (intPair.has_value() && (*intPair)[0] > 0 && (*intPair)[1] > 0)
                    screenBounds.size = intPair.value();
                else
                    failed = true;
            }

            if (failed)
                return std::nullopt;
            else
                return screenBounds;

        }

        std::optional<float> LoadLastRunDpiWindowSizeFactor(const std::optional<std::string>& iniPartsFilename)
        {
            if (!iniPartsFilename.has_value())
                return std::nullopt;
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());

            if (!iniParts.HasIniPart("AppWindow"))
                return std::nullopt;

            auto iniPartContent = iniParts.GetIniPart("AppWindow");
            ini::IniFile iniFile;
            try
            {
                iniFile.decode(iniPartContent);
            }
            catch (const std::exception &)
            {
                return std::nullopt;
            }

            ScreenBounds screenBounds;
            bool failed = false;

            if (iniFile.find("AppWindow") == iniFile.end())
                return std::nullopt;
            auto &appWindowSection = iniFile["AppWindow"];

            if (appWindowSection.find("DpiWindowSizeFactor") != appWindowSection.end())
            {
                float dpiWindowSizeFactor_WhenSaved;
                try
                {
                    dpiWindowSizeFactor_WhenSaved = iniFile["AppWindow"]["DpiWindowSizeFactor"].as<float>();
                }
                catch (const std::exception&)  // not a number
                {
                    return std::nullopt;
                }
                bool isDpiSane = (dpiWindowSizeFactor_WhenSaved >= 0.1f) &&
                                 (dpiWindowSizeFactor_WhenSaved <= 10.f);
                if (isDpiSane)
                    return dpiWindowSizeFactor_WhenSaved;
            }
            return std::nullopt;
        }


        void LoadImGuiSettings(const std::optional<std::string>& iniPartsFilename, const std::string& layoutName)
        {
            if (!iniPartsFilename.has_value())
                return;
            std::string iniPartName = "ImGui_" + details::SanitizeIniNameOrCategory(layoutName);

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
            if (!iniParts.HasIniPart(iniPartName))
                return;
            auto imguiSettingsContent = iniParts.GetIniPart(iniPartName);
            ImGui::LoadIniSettingsFromMemory(imguiSettingsContent.c_str());
        }

        void SaveImGuiSettings(const std::optional<std::string>& iniPartsFilename, const std::string& layoutName)
        {
            if (!iniPartsFilename.has_value())
                return;
            std::string iniPartName = "ImGui_" + details::SanitizeIniNameOrCategory(layoutName);

            std::string imguiSettingsContent = ImGui::SaveIniSettingsToMemory();

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
            iniParts.SetIniPart(iniPartName, imguiSettingsContent);
            iniParts.WriteToFile(iniPartsFilename.value());
        }

        bool HasUserDockingSettingsInImguiSettings(const std::optional<std::string>& iniPartsFilename, const DockingParams& dockingParams)
        {
            if (!iniPartsFilename.has_value())
                return false;
            std::string iniPartName = "ImGui_" + details::SanitizeIniNameOrCategory(dockingParams.layoutName);

            auto iniParts = HelloImGuiIniSettings::IniParts::LoadFromFile(iniPartsFilename.value());
            if (!iniParts.HasIniPart(iniPartName))
                return false;

            auto iniPartContent = iniParts.GetIniPart(iniPartName);
            std::stringstream ss(iniPartContent);

            std::vector<std::string> windowsWithSettings;
            std::string line;
            while (ss)
            {
                std::getline (ss, line);
                std::string w = details::_windowNameInImguiIniLine(line);
                if (!w.empty())
                    windowsWithSettings.push_back(w);
            }

            for (const auto& dockableWindow: dockingParams.dockableWindows)
            {
                if (
                std::find(windowsWithSettings.begin(), windowsWithSettings.end(), dockableWindow.label)
                == windowsWithSettings.end())
                {
                    return false;
                }
            }
            return true;
        }


        void SaveDockableWindowsVisibility(const std::optional<std::string>& iniPartsFilename, const DockingParams& dockingParams)
        {
            if (!iniPartsFilename.has_value())
                return;
            std::string iniPartName = "Layout_" + details::SanitizeIniNameOrCategory(dockingParams.layoutName);

            ini::IniFile iniFile;
            for (const auto& dockableWindow: dockingParams.dockableWindows)
            {
                if (dockableWindow.rememberIsVisible)
                {
                    std::string iniValueName = details::SanitizeIniNameOrCategory(dockableWindow.label);
                    iniFile["Visibility"][iniValueName] = dockableWindow.isVisible;
                }
            }

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
            iniParts.SetIniPart(iniPartName, iniFile.encode());
            iniParts.WriteToFile(iniPartsFilename.value());
        }

        void LoadDockableWindowsVisibility(const std::optional<std::string>& iniPartsFilename, DockingParams* inOutDockingParams)
        {
            if (!iniPartsFilename.has_value())
                return;
            std::string iniPartName = "Layout_" + details::SanitizeIniNameOrCategory(inOutDockingParams->layoutName);

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
            if (! iniParts.HasIniPart(iniPartName))
                return;

            ini::IniFile iniFile;
            if (!_TryDecode(iniFile, iniParts.GetIniPart(iniPartName)))
                return;
            for (auto& dockableWindow: inOutDockingParams->dockableWindows)
            {
                if (dockableWindow.rememberIsVisible)
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

        void LoadSplitIds(const std::optional<std::string>& iniPartsFilename)
        {
            const std::string iniPartName = "SplitIds";
            std::string serialized = LoadUserPref(iniPartsFilename, iniPartName);
            if (!serialized.empty())
                SplitIdsHelper::LoadSplitIds(serialized);
        }

        void SaveSplitIds(const std::optional<std::string>& iniPartsFilename)
        {
            const std::string iniPartName = "SplitIds";
            std::string serialized = SplitIdsHelper::SaveSplitIds();
            SaveUserPref(iniPartsFilename, iniPartName, serialized);
        }

        void LoadHelloImGuiMiscSettings(const std::optional<std::string>& iniPartsFilename, RunnerParams* inOutRunnerParams)
        {
            if (!iniPartsFilename.has_value())
                return;
            std::string iniPartName = "HelloImGui_Misc";

            std::string layoutName = "";
            std::string themeName = "";
            {
                IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
                ini::IniFile iniFile;
                if (iniParts.HasIniPart(iniPartName) && _TryDecode(iniFile, iniParts.GetIniPart(iniPartName)))
                {
                    if (inOutRunnerParams->rememberSelectedAlternativeLayout)
                        layoutName = iniFile["Layout"]["Name"].as<std::string>();
                    if (inOutRunnerParams->imGuiWindowParams.rememberTheme)
                        themeName = iniFile["Theme"]["Name"].as<std::string>();

                    if (inOutRunnerParams->imGuiWindowParams.rememberStatusBarSettings)
                    {
                        {
                            std::string s = iniFile["StatusBar"]["Show"].as<std::string>();
                            if (s == "true")
                                inOutRunnerParams->imGuiWindowParams.showStatusBar = true;
                            if (s == "false")
                                inOutRunnerParams->imGuiWindowParams.showStatusBar = false;
                        }
                        {
                            std::string s = iniFile["StatusBar"]["ShowFps"].as<std::string>();
                            if (s == "true")
                                inOutRunnerParams->imGuiWindowParams.showStatus_Fps = true;
                            if (s == "false")
                                inOutRunnerParams->imGuiWindowParams.showStatus_Fps = false;
                        }
                    }
                    if (inOutRunnerParams->fpsIdling.rememberEnableIdling)
                    {
                        std::string s = iniFile["Idling"]["EnableIdling"].as<std::string>();
                        if (s == "true")
                            inOutRunnerParams->fpsIdling.enableIdling = true;
                        if (s == "false")
                            inOutRunnerParams->fpsIdling.enableIdling = false;
                    }
                }
            }

            if (!themeName.empty())
            {
                auto theme = ImGuiTheme::ImGuiTheme_FromName(themeName.c_str());
                inOutRunnerParams->imGuiWindowParams.tweakedTheme.Theme = theme;
                ImGuiTheme::ApplyTheme(theme);
            }
            HelloImGui::SwitchLayout(layoutName);

            LoadSplitIds(iniPartsFilename);
        }

        void SaveHelloImGuiMiscSettings(const std::optional<std::string>& iniPartsFilename, const RunnerParams& runnerParams)
        {
            if (!iniPartsFilename.has_value())
                return;
            std::string iniPartName = "HelloImGui_Misc";
            ini::IniFile iniFile;
            if (runnerParams.rememberSelectedAlternativeLayout)
                iniFile["Layout"]["Name"] = runnerParams.dockingParams.layoutName;
            if (runnerParams.imGuiWindowParams.rememberTheme)
                iniFile["Theme"]["Name"] = ImGuiTheme::ImGuiTheme_Name(runnerParams.imGuiWindowParams.tweakedTheme.Theme);
            if (runnerParams.imGuiWindowParams.rememberStatusBarSettings)
            {
                iniFile["StatusBar"]["Show"] = runnerParams.imGuiWindowParams.showStatusBar;
                iniFile["StatusBar"]["ShowFps"] = runnerParams.imGuiWindowParams.showStatus_Fps;
            }
            if (runnerParams.fpsIdling.rememberEnableIdling)
            {
                iniFile["Idling"]["EnableIdling"] = runnerParams.fpsIdling.enableIdling;
            }

            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
            iniParts.SetIniPart(iniPartName, iniFile.encode());
            iniParts.WriteToFile(iniPartsFilename.value());

            SaveSplitIds(iniPartsFilename);
        }


        // A user pref is stored as an ini part, line by line: a line of its content that looks like a part's marker
        // (";;;<<<name>>>;;;") would end the part. Such a line is stored with a backslash after ";;;", and so is a
        // line that already starts with ";;;\": loading removes one, and gives back the content as it was saved.
        std::string _TransformLines(const std::string& s, void (*fn)(std::string& line))
        {
            std::string r;
            size_t lineStart = 0;
            while (lineStart <= s.size())
            {
                size_t lineEnd = s.find('\n', lineStart);
                if (lineEnd == std::string::npos)
                    lineEnd = s.size();
                std::string line = s.substr(lineStart, lineEnd - lineStart);
                fn(line);
                r += line;
                if (lineEnd < s.size())
                    r += '\n';
                lineStart = lineEnd + 1;
            }
            return r;
        }
        void _EscapeUserPrefLine(std::string& line)
        {
            if (line.rfind(";;;<<<", 0) == 0 || line.rfind(";;;\\", 0) == 0)
                line.insert(3, "\\");
        }
        void _UnescapeUserPrefLine(std::string& line)
        {
            if (line.rfind(";;;\\", 0) == 0)
                line.erase(3, 1);
        }

        void  SaveUserPref(const std::optional<std::string>& iniPartsFilename, const std::string& userPrefName, const std::string& userPrefContent)
        {
            if (!iniPartsFilename.has_value())
                return;
            // The name goes into the part's marker line
            if (userPrefName.empty() || userPrefName.find_first_of("\r\n") != std::string::npos)
            {
                fprintf(stderr, "SaveUserPref: invalid name \"%s\" (empty, or with a line break): not saved\n", userPrefName.c_str());
                return;
            }
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
            iniParts.SetIniPart(userPrefName, _TransformLines(userPrefContent, _EscapeUserPrefLine) + "\n");
            iniParts.WriteToFile(iniPartsFilename.value());
        }

        std::string LoadUserPref(const std::optional<std::string>& iniPartsFilename, const std::string& userPrefName)
        {
            if (!iniPartsFilename.has_value())
                return "";
            IniParts iniParts = IniParts::LoadFromFile(iniPartsFilename.value());
            if (iniParts.HasIniPart(userPrefName))
            {
                std::string contentWithNewLine = iniParts.GetIniPart(userPrefName);
                return _TransformLines(contentWithNewLine.substr(0, contentWithNewLine.size() - 1), _UnescapeUserPrefLine);
            }
            else
                return "";
        }


        // The lasting prefs. On the desktop: the ini parts format, in "<ini name>_lasting.ini". On the web: the
        // browser's localStorage, with the keys "<ini name>/<pref name>". emscripten_run_script rather than EM_JS: it
        // also works in a side module, as in Pyodide. localStorage throws when the site's storage is blocked.
#ifdef __EMSCRIPTEN__
        static std::string JsStringLiteral(const std::string& s)
        {
            std::string r = "\"";
            for (unsigned char c : s)
            {
                if (c == '"' || c == '\\')
                {
                    r += '\\';
                    r += static_cast<char>(c);
                }
                else if (c < 0x20)
                {
                    char escaped[8];
                    snprintf(escaped, sizeof(escaped), "\\u%04x", c);
                    r += escaped;
                }
                else
                    r += static_cast<char>(c);
            }
            return r + "\"";
        }
#else
        static std::string LastingPrefsFilename(const std::string& iniPartsFilename)
        {
            std::filesystem::path path(iniPartsFilename);  // "folder/App.ini" -> "folder/App_lasting.ini"
            path.replace_filename(path.stem().string() + "_lasting.ini");
            return path.string();
        }
#endif

        // Read once from their store, then from this cache: an app may read them at each frame
        static std::map<std::string, std::string> gLastingPrefsCache;  // key: "<ini name>/<pref name>"

        void SaveLastingPref(const std::optional<std::string>& iniPartsFilename,
                             const std::string& prefName, const std::string& prefContent)
        {
            if (!iniPartsFilename.has_value())
                return;
            std::string key = iniPartsFilename.value() + "/" + prefName;
            gLastingPrefsCache[key] = prefContent;
#ifdef __EMSCRIPTEN__
            std::string script = "try { localStorage.setItem(" + JsStringLiteral(key) + ", "
                                 + JsStringLiteral(prefContent) + "); } catch (e) {}";
            emscripten_run_script(script.c_str());
#else
            SaveUserPref(LastingPrefsFilename(iniPartsFilename.value()), prefName, prefContent);
#endif
        }

        std::string LoadLastingPref(const std::optional<std::string>& iniPartsFilename, const std::string& prefName)
        {
            if (!iniPartsFilename.has_value())
                return "";
            std::string key = iniPartsFilename.value() + "/" + prefName;
            auto cached = gLastingPrefsCache.find(key);
            if (cached != gLastingPrefsCache.end())
                return cached->second;
#ifdef __EMSCRIPTEN__
            std::string script = "(function () { try { return localStorage.getItem(" + JsStringLiteral(key)
                                 + ") || ''; } catch (e) { return ''; } })()";
            const char* value = emscripten_run_script_string(script.c_str());
            std::string prefContent = value ? value : "";
#else
            std::string prefContent = LoadUserPref(LastingPrefsFilename(iniPartsFilename.value()), prefName);
#endif
            gLastingPrefsCache[key] = prefContent;
            return prefContent;
        }


    } // namespace HelloImGuiIniSettings
} // namespace HelloImGui
