#include "hello_imgui/internal/hello_imgui_ini_any_parent_folder.h"
#include "hello_imgui/internal/inicpp.h"

#include <vector>
#include <filesystem>
#include <string>
#include <optional>


namespace HelloImGui
{
    namespace HelloImGuiIniAnyParentFolder
    {
        //
        // Utilities that read any settings in a hello_imgui.ini file located in the current folder or any of its parents
        //
        std::vector<std::string> _folderAndAllParents(const std::string& folder)
        {
            std::vector<std::string> result;
            std::string currentFolder = folder;
            while (true)
            {
                result.push_back(currentFolder);
                std::filesystem::path p(currentFolder);
                if (p.has_parent_path() && (p.parent_path() != p))
                    currentFolder = p.parent_path().string();
                else
                    break;
            }
            return result;
        };

        std::vector<std::string> _allHelloImGuiIniFilesToSearch()
        {
            std::vector<std::string> allIniFileToSearch;

            std::string currentFolder = std::filesystem::current_path().string();
            for (const auto& folder : _folderAndAllParents(currentFolder))
                allIniFileToSearch.push_back(folder + "/hello_imgui.ini");

            return allIniFileToSearch;
        };

        template<typename T>
        std::optional<T> _readIniValue(const std::string& iniFilePath, const std::string& sectionName, const std::string& valueName)
        {
            if (! std::filesystem::exists(iniFilePath))
                return std::nullopt;
            if (! std::filesystem::is_regular_file(iniFilePath))
                return std::nullopt;

            try
            {

                std::optional<T> result = std::nullopt;
                ini::IniFile ini;
                ini.load(iniFilePath);
                if (ini.find(sectionName) != ini.end())
                {
                    auto& section = ini.at(sectionName);
                    if (section.find(valueName) != section.end())
                        result =  ini[sectionName][valueName].as<T>();
                }
                return result;
            }
            catch(...)
            {
                return std::nullopt;
            }
        };

        template <typename T>
        std::optional<T> _readIniValueInParentFolders(const std::string& sectionName, const std::string& valueName)
        {
            for (const auto& iniFile: _allHelloImGuiIniFilesToSearch())
            {
                auto value = _readIniValue<T>(iniFile, sectionName, valueName);
                if (value.has_value())
                    return value;
            }
            return std::nullopt;
        };


        std::optional<float> readFloatValue(const std::string& sectionName, const std::string& valueName)
        {
            return _readIniValueInParentFolders<float>(sectionName, valueName);
        }

        std::optional<bool> readBoolValue(const std::string& sectionName, const std::string& valueName)
        {
            return _readIniValueInParentFolders<bool>(sectionName, valueName);
        }

        std::optional<std::string> readStringValue(const std::string& sectionName, const std::string& valueName)
        {
            return _readIniValueInParentFolders<std::string>(sectionName, valueName);
        }

        std::optional<int> readIntValue(const std::string &sectionName, const std::string &valueName)
        {
            return _readIniValueInParentFolders<int>(sectionName, valueName);
        }

    } // namespace HelloImGuiIniAnyParentFolder

} // namespace HelloImGui
