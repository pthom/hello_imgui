#include <string>
#include <cstdio>
#include <cstdarg>

#ifdef _MSC_VER
#include <Windows.h>
#endif


namespace HelloImGui
{
void PoorManLog(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    char buffer[2000];
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

#ifdef _MSC_VER
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
    wchar_t wbuffer[2000];
    mbstowcs(wbuffer, buffer, strlen(buffer) + 1);
    OutputDebugString(wbuffer);
    OutputDebugString(buffer);
#else
     OutputDebugString(buffer);
#endif
#else
    printf("%s", buffer);
#endif
}

}
