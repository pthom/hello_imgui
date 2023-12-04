#include <string>
#import <Foundation/Foundation.h>

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
