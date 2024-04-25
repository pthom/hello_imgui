#pragma once
#include <string>
#include <cstdint>


namespace HelloImGui
{


// @@md#RemoteParams

// RemoteParams is a struct that contains the settings for displaying the application on a remote device.
// using https://github.com/sammyfreg/netImgui
// or using https://github.com/ggerganov/imgui-ws
// Those features are experimental and not supported with the standard version of HelloImGui,
struct RemoteParams
{
    bool enableRemoting = false;

    //
    // Params used only by imgui-ws
    //
    int wsPort = 5003;
    std::string wsHttpRootFolder = "";  // Optional folder were some additional files can be served
    bool wsProvideIndexHtml = true;     // If true, will automatically serve a simple index.html file that contains the canvas and the imgui-ws client code

    //
    // Params used only by netImgui
    //
    bool exitWhenServerDisconnected = false;
    double durationMaxDisconnected = 30.0;
    // The server host (if empty, will use "localhost")
    // The server is the app that simply displays the application on a remote device
    std::string serverHost = "localhost";
    // The server port (default is 8888)
    uint32_t serverPort = 8888;
    // If true, transmit the window size to the server
    bool transmitWindowSize = false;
};

// @@md


}  // namespace HelloImGui