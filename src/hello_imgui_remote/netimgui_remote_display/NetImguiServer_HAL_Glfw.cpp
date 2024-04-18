#include "NetImguiServer_App.h"

#if HAL_API_PLATFORM_GLFW_GL3

#include <Private/NetImgui_Shared.h>
#include "hello_imgui/hello_imgui.h"
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


namespace NetImguiServer { namespace App
{

//=================================================================================================
// HAL STARTUP
// Additional initialisation that are platform specific
//=================================================================================================
bool HAL_Startup(const char* CmdLine)
{
	IM_UNUSED(CmdLine);
	return true;
}

//=================================================================================================
// HAL SHUTDOWN
// Prepare for shutdown of application, with platform specific code
//=================================================================================================
void HAL_Shutdown()
{
}

//=================================================================================================
// HAL GET SOCKET INFO
// Take a platform specific socket (based on the NetImguiNetworkXXX.cpp implementation) and
// fetch informations about the client IP connected
//=================================================================================================
bool HAL_GetSocketInfo(NetImgui::Internal::Network::SocketInfo* pClientSocket, char* pOutHostname, size_t HostNameLen, int& outPort)
{
#ifdef _WIN32
	sockaddr socketAdr;
	int sizeSocket(sizeof(sockaddr));
	SOCKET* pClientSocketWin = reinterpret_cast<SOCKET*>(pClientSocket);
	if( getsockname(*pClientSocketWin, &socketAdr, &sizeSocket) == 0 )
	{
		char zPortBuffer[32];
		if( getnameinfo(&socketAdr, sizeSocket, pOutHostname, static_cast<DWORD>(HostNameLen), zPortBuffer, sizeof(zPortBuffer), NI_NUMERICSERV) == 0 )
		{
			outPort = atoi(zPortBuffer);
			return true;
		}
	}
#endif
	return false;
}

//=================================================================================================
// HAL GET USER SETTING FOLDER
// Request the directory where to the 'shared config' clients should be saved
// Return 'nullptr' to disable this feature
//=================================================================================================
const char* HAL_GetUserSettingFolder()
{
    static std::string result = HelloImGui::IniFolderLocation(HelloImGui::IniFolderType::AppUserConfigFolder);
    return result.c_str();
}

//=================================================================================================
// HAL GET CLIPBOARD UPDATED
// Detect when clipboard had a content change and we should refetch it on the Server and
// forward it to the Clients
//=================================================================================================
bool HAL_GetClipboardUpdated()
{
    auto updateClipboardEverySecond = []() -> bool
    {
        static std::chrono::steady_clock::time_point sLastCheck = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if( (now - sLastCheck) > std::chrono::seconds(1) )
        {
            sLastCheck = now;
            return true;
        }
        else
        {
            return false;
        }
    };

#ifdef HELLOIMGUI_USE_GLFW3
    GLFWwindow* mainWindow = (GLFWwindow*)HelloImGui::GetRunnerParams()->backendPointers.glfwWindow;
    if (! mainWindow)
        return updateClipboardEverySecond();
    else
    {
        static std::string lastClipboardContent;
        const char* clipboardContent = glfwGetClipboardString(mainWindow);
        if (clipboardContent && lastClipboardContent != clipboardContent)
        {
            lastClipboardContent = clipboardContent;
            return true;
        }
        else
        {
            return false;
        }
    }
#else
    return updateClipboardEverySecond();
#endif
}

}} // namespace NetImguiServer { namespace App

#endif // HAL_API_PLATFORM_WIN32DX11