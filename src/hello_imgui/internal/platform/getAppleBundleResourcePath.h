#pragma once
#include <string>

std::string getAppleBundleResourcePath(const std::string & filename);

std::string GetBundlePath();

namespace AppleIniFolderLocations
{
    std::string GetTempPath();
    std::string GetAppUserConfigFolder();
    std::string GetDocumentsPath();
    std::string GetHomePath();
}