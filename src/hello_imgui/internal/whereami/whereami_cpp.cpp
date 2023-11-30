#ifndef __EMSCRIPTEN__

#include "hello_imgui/internal/whereami/whereami.h"
#include "hello_imgui/internal/whereami/whereami_cpp.h"

std::string wai_getExecutablePath_string()
{
    int length = wai_getExecutablePath(NULL, 0, NULL);
    char* path = (char*)malloc(length + 1);
    wai_getExecutablePath(path, length, NULL);
    path[length] = '\0';
    std::string result(path);
    free(path);
    return result;
}

std::string wai_getModulePath_string()
{
    int length = wai_getModulePath(NULL, 0, NULL);
    char* path = (char*)malloc(length + 1);
    wai_getModulePath(path, length, NULL);
    path[length] = '\0';
    std::string result(path);
    free(path);
    return result;
}

namespace
{
    std::string dirname(const std::string& str)
    {
        size_t found;
        found=str.find_last_of("/\\");
        std::string dirname = str.substr(0,found);
        return dirname;
    }
}

std::string wai_getExecutableFolder_string()
{
    return dirname(wai_getExecutablePath_string());
}

#endif
