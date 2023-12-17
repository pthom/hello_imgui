#pragma once
#include <string>
#include <TargetConditionals.h>

std::string getAppleBundleResourcePath(const std::string & filename);

std::string GetBundlePath();

namespace AppleIniFolderLocations
{
    std::string GetTempPath();
    std::string GetAppUserConfigFolder();
    std::string GetDocumentsPath();
    std::string GetHomePath();
}

#if TARGET_OS_IOS
namespace HelloImGui
{
    struct TmpEdgeInsets
    {
        double top, left, bottom, right;
    };
    TmpEdgeInsets GetIPhoneSafeAreaInsets();
}

#endif
