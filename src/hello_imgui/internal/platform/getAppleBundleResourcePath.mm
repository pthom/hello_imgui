#include <string>
#include <TargetConditionals.h>
#import <Foundation/Foundation.h>
#include "hello_imgui/internal/platform/getAppleBundleResourcePath.h"

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif


std::string getAppleBundleResourcePath(const std::string & filename)
{
    NSString *filename_ns = [NSString stringWithUTF8String:filename.c_str()];
    NSString *filePath = [[NSBundle mainBundle] pathForResource:filename_ns ofType:@""];
    if (filePath == nullptr)
        return "";
    std::string path([filePath UTF8String]);
    return path;
}

std::string GetBundlePath()
{
    @autoreleasepool {
        // Get the main bundle for the app
        NSBundle* mainbundle = [NSBundle mainBundle];
        // Get the bundle path as an NSString
        NSString* bundlePath = [mainbundle bundlePath];
        // Convert the NSString to a C++ string
        return [bundlePath UTF8String];
    }
}


namespace AppleIniFolderLocations
{
    std::string GetTempPath()
    {
        NSString *tempPath = NSTemporaryDirectory();
        return [tempPath UTF8String];
    }

    std::string GetAppUserConfigFolder()
    {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        NSString *appDataPath = [paths firstObject];
        return [appDataPath UTF8String];
    }

    std::string GetDocumentsPath()
    {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsPath = [paths firstObject];
        return [documentsPath UTF8String];
    }

    std::string GetHomePath()
    {
        NSString *homePath = NSHomeDirectory();
        return [homePath UTF8String];
    }
}


#if TARGET_OS_IOS
namespace HelloImGui
{
    TmpEdgeInsets GetIPhoneSafeAreaInsets()
    {
        UIEdgeInsets insets = UIEdgeInsetsZero;
        if (@available(iOS 11.0, *))
        {
            //UIWindow *mainWindow = [[[UIApplication sharedApplication] delegate] window];
            //UIWindow *mainWindow = UIApplication.sharedApplication.keyWindow;
            //insets = mainWindow.safeAreaInsets;

            UIWindow *mainWindow = UIApplication.sharedApplication.keyWindow;
            if (mainWindow && mainWindow.rootViewController.view)
                insets = mainWindow.rootViewController.view.safeAreaInsets;
        }

        TmpEdgeInsets r { insets.top, insets.left, insets.bottom, insets.right };
        return r;
    }
}
#endif
