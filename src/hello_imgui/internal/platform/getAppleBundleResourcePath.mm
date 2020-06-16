#include <string>
#import <Foundation/Foundation.h>

std::string getAppleBundleResourcePath(const std::string & filename)
{
    NSString *filename_ns = [NSString stringWithUTF8String:filename.c_str()];
    NSString *filePath = [[NSBundle mainBundle] pathForResource:filename_ns ofType:@""];
    std::string path([filePath UTF8String]);
    return path;
}
