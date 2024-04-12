#include <string>
#include <cstdio>
#include <cstdarg>


namespace HelloImGui
{
	std::string _DoSnPrintf(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		char buffer[2000];
		vsnprintf(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		return std::string(buffer);
	}


	void PoorManLog(const char* msg, ...)
	{
		// call _DoSnPrintf
		va_list args;
		va_start(args, msg);
		std::string buffer = _DoSnPrintf(msg, args);
		va_end(args);

		#ifdef _MSC_VER
		OutputDebugStringA(buffer.c_str());
		#else
		printf("%s", buffer.c_str());
		#endif
	}
}
