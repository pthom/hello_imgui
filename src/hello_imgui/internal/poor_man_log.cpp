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
    OutputDebugString(buffer);
#else
    printf("%s", buffer);
#endif
}

}
