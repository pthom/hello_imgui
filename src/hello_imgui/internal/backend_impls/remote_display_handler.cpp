#ifdef HELLOIMGUI_WITH_NETIMGUI
#include "NetImgui_Api.h"
#endif

#include "hello_imgui/internal/backend_impls/remote_display_handler.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/poor_man_log.h"
#include "hello_imgui/internal/clock_seconds.h"

#include <string>
#include <thread>

#define ENABLE_NETIMGUI_LOG  // Enable or disable logging for NetImgui remoting
#ifdef ENABLE_NETIMGUI_LOG
#define NetimguiLog PoorManLog
#else
#define NetimguiLog(...)
#endif


namespace HelloImGui
{
#ifdef HELLOIMGUI_WITH_NETIMGUI
    namespace NetimguiUtils
    {
        // =====================================================================================================================
        //                              <NetimguiUtils>
        // =====================================================================================================================
        struct NetImGuiRaii {
            NetImGuiRaii() { NetImgui::Startup(); }
            ~NetImGuiRaii() { NetImgui::Shutdown(); }
        };


        class NetImGuiWrapper
        {
        public:
            NetImGuiWrapper() = default;
            ~NetImGuiWrapper() = default;

            void HeartBeat()
            {
                if (mNetImguiRaii == nullptr)
                {
                    InitiateConnection();
                    return;
                }

                int nbConnectionsPending = mNbConnectionsTentatives - mNbConnectionsSuccess - mNbConnectionsFailures;
                IM_ASSERT(nbConnectionsPending == 0 || nbConnectionsPending == 1);

                if (nbConnectionsPending == 1)
                {
                    if (NetImgui::IsConnected())
                    {
                        LogStatus("Connection success...\n");
                        ++ mNbConnectionsSuccess;
                        mLastConnectionSuccess = true;
                    }
                    else if (NetImgui::IsConnectionPending())
                    {
                        LogStatus("Connection pending...\n");
                    }
                    else
                    {
                        LogStatus("Connection failure...\n");
                        ++ mNbConnectionsFailures;
                        mLastConnectionSuccess = false;
                    }
                }
                else // nbConnectionsPending == 0
                {
                    if (mLastConnectionSuccess)
                    {
                        if (!NetImgui::IsConnected())
                        {
                            if (remoteParams().exitWhenServerDisconnected)
                            {
                                LogStatus("Connection lost... Exiting\n");
                                runnerParams().appShallExit = true;
                            }
                            else
                            {
                                mTimePointConnectionEnded = HelloImGui::Internal::ClockSeconds();
                                LogStatus("Connection lost... Reconnecting\n");
                                InitiateConnection();
                            }
                        }
                    }
                    else // Last connection was a failure
                    {
                        LogStatus("Last connection was a failure... Reconnecting\n");
                        InitiateConnection();
                    }
                }

                // Exit if failure since too long
                {
                    bool isDisconnected = !mLastConnectionSuccess;
                    if (isDisconnected)
                    {
                        double disconnectionTime = HelloImGui::Internal::ClockSeconds() - mTimePointConnectionEnded;
                        std::string msg = "Disconnected since " + std::to_string(disconnectionTime) + "s";
                        LogStatus(msg);
                        bool isTooLong = disconnectionTime > remoteParams().durationMaxDisconnected;
                        if (isTooLong)
                        {
                            LogStatus("Too long disconnected... Exiting\n");
                            runnerParams().appShallExit = true;
                        }
                    }
                }
            }

            void sendFonts()
            {
                _sendFonts_Impl();
            }

            bool isConnected()
            {
                return NetImgui::IsConnected();
            }

        private:
            void InitiateConnection()
            {
                NetimguiLog("NetImGuiWrapper: InitiateConnection\n");
                mNetImguiRaii.release();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                mNetImguiRaii = std::make_unique<NetImGuiRaii>();
                NetImgui::ConnectToApp(clientName().c_str(), remoteParams().serverHost.c_str(), remoteParams().serverPort);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                ++ mNbConnectionsTentatives;
                _sendFonts_Impl();
            }

            std::string clientName()
            {
                std::string clientName = runnerParams().appWindowParams.windowTitle;
                if (clientName.empty())
                    clientName = "HelloImGui";
                clientName += "##" + std::to_string(ImGui::GetTime());
                return clientName;
            }

            HelloImGui::RemoteParams& remoteParams() { return HelloImGui::GetRunnerParams()->remoteParams; }
            HelloImGui::RunnerParams& runnerParams() { return *HelloImGui::GetRunnerParams(); }

            void _sendFonts_Impl()
            {
                const ImFontAtlas* pFonts = ImGui::GetIO().Fonts;
                if( pFonts->TexPixelsAlpha8) // && (pFonts->TexPixelsAlpha8 != client.mpFontTextureData || client.mFontTextureID != pFonts->TexID ))
                {
                    uint8_t* pPixelData(nullptr); int width(0), height(0);
                    ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pPixelData, &width, &height);
                    NetImgui::SendDataTexture(pFonts->TexID, pPixelData, static_cast<uint16_t>(width), static_cast<uint16_t>(height), NetImgui::eTexFormat::kTexFmtA8);
                }
                if( pFonts->TexPixelsRGBA32) // && (pFonts->TexPixelsAlpha8 != client.mpFontTextureData || client.mFontTextureID != pFonts->TexID ))
                {
                    uint8_t* pPixelData(nullptr); int width(0), height(0);
                    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pPixelData, &width, &height);
                    NetImgui::SendDataTexture(pFonts->TexID, pPixelData, static_cast<uint16_t>(width), static_cast<uint16_t>(height), NetImgui::eTexFormat::kTexFmtRGBA8);
                }
            }

            void LogStatus(const std::string& msg)
            {
                NetimguiLog("NetImGuiWrapper: %s\n", msg.c_str());
            }

        private:  // Members
            std::unique_ptr<NetImGuiRaii> mNetImguiRaii;
            int mNbConnectionsTentatives = 0;
            int mNbConnectionsSuccess = 0;
            int mNbConnectionsFailures = 0;
            bool mLastConnectionSuccess = false;
            double mTimePointConnectionEnded = 0.0;
        };

        std::unique_ptr<NetImGuiWrapper> gNetImGuiWrapper;


        bool UpdateDpiAwareParams_Netimgui()
        {
            bool didFontLoadingRatioChangeOnRemoteServer = false;
            auto& dpiAwareParams = HelloImGui::GetRunnerParams()->dpiAwareParams;

            float newFontLoadingRatio = NetImgui::GetFontSizeLoadingRatio();
            float currentFontLoadingRatio = dpiAwareParams.DpiFontLoadingFactor();
            if (fabs(currentFontLoadingRatio - newFontLoadingRatio) > 0.001f)
            {
                didFontLoadingRatioChangeOnRemoteServer = true;

                float oldDpiWindowSizeFactor = dpiAwareParams.dpiWindowSizeFactor;
                float ratioScaling = newFontLoadingRatio / currentFontLoadingRatio;
                dpiAwareParams.dpiWindowSizeFactor = dpiAwareParams.dpiWindowSizeFactor * ratioScaling;
                ImGui::GetStyle().ScaleAllSizes(ratioScaling);
                float new_diff = fabs(dpiAwareParams.DpiFontLoadingFactor() - newFontLoadingRatio);
                IM_ASSERT(new_diff < 0.001f);
                NetimguiLog("UpdateDpiAwareParams_Netimgui: didFontLoadingRatioChange=true \n"
                       "    currentFontLoadingRatio=%f newFontLoadingRatio=%f\n"
                       "    oldDpiWindowSizeFactor=%f newDpiWindowSizeFactor=%f\n",
                       currentFontLoadingRatio, newFontLoadingRatio, oldDpiWindowSizeFactor, dpiAwareParams.dpiWindowSizeFactor);
            }
            return didFontLoadingRatioChangeOnRemoteServer;
        }

        // =====================================================================================================================
    //                              </NetimguiUtils>
    // =====================================================================================================================
    } // namespace NetimguiUtils
#endif


bool RemoteDisplayHandler::IsConnectedToServer()
{
    #ifndef HELLOIMGUI_WITH_NETIMGUI
        return false;
    #else
    if (!ShouldDisplayOnRemoteServer())
        return false;
    return NetimguiUtils::gNetImGuiWrapper->isConnected();
    #endif
}

bool RemoteDisplayHandler::ShouldDisplayOnRemoteServer()
{
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    return HelloImGui::GetRunnerParams()->remoteParams.enableRemoting;
    #else
    return false;
    #endif
}

bool RemoteDisplayHandler::CheckDpiAwareParamsChanges()
{
    if (!IsConnectedToServer())
        return false;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    return NetimguiUtils::UpdateDpiAwareParams_Netimgui();
    #else
    return false;
    #endif
}

void RemoteDisplayHandler::Create()
{
    if (!ShouldDisplayOnRemoteServer())
        return;
    IM_ASSERT(!IsConnectedToServer() && "RemoteDisplayHandler::Create: Already connected to server");
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    NetimguiUtils::gNetImGuiWrapper = std::make_unique<NetimguiUtils::NetImGuiWrapper>();
    #endif
}

void RemoteDisplayHandler::Shutdown()
{
    if (!IsConnectedToServer())
        return;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    NetimguiUtils::gNetImGuiWrapper.reset();
    #endif
}

void RemoteDisplayHandler::Heartbeat()
{
    if (!ShouldDisplayOnRemoteServer())
        return;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    NetimguiUtils::gNetImGuiWrapper->HeartBeat();
    #endif
}

void RemoteDisplayHandler::TransmitWindowSizeToDisplay(ScreenSize windowSize)
{
    if (IsConnectedToServer())
    {
        static bool wasSizeTransmitted = false;
        if (!wasSizeTransmitted)
        {
            wasSizeTransmitted = true;
            // When running on a remote server, we do not know the window DPI factor,
            // so we treat the window size as if it was 96PPI
            uint16_t width_96PPI = (uint16_t)windowSize[0];
            uint16_t height_96PPI = (uint16_t)windowSize[1];
            #ifdef HELLOIMGUI_WITH_NETIMGUI
            NetImgui::SetWindowSize_96PPI(width_96PPI, height_96PPI);
            #endif
        }
    }
}

void RemoteDisplayHandler::SendFonts()
{
    if (ShouldDisplayOnRemoteServer())
    {
        #ifdef HELLOIMGUI_WITH_NETIMGUI
        NetimguiUtils::gNetImGuiWrapper = std::make_unique<NetimguiUtils::NetImGuiWrapper>();
        NetimguiUtils::gNetImGuiWrapper->sendFonts();
        #endif
    }
}

}