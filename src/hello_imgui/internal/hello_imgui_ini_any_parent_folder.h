#pragma once
#include <optional>
#include <string>


namespace HelloImGui
{
    namespace HelloImGuiIniAnyParentFolder
    {
        std::optional<float> readFloatValue(const std::string &sectionName, const std::string &valueName);
        std::optional<bool> readBoolValue(const std::string &sectionName, const std::string &valueName);
        std::optional<std::string> readStringValue(const std::string &sectionName, const std::string &valueName);
        std::optional<int> readIntValue(const std::string &sectionName, const std::string &valueName);
    }
}