#ifdef HELLOIMGUI_WITH_NETIMGUI
#include "NetImgui_Api.h"
#endif
#ifdef HELLOIMGUI_WITH_IMGUIWS
#include "imgui-ws/imgui-ws.h"
#endif

#include "hello_imgui/internal/backend_impls/remote_display_handler.h"
#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/internal/poor_man_log.h"
#include "hello_imgui/internal/clock_seconds.h"
#include "imgui_internal.h"

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

            void HeartBeat_PreImGuiNewFrame()
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

#ifdef HELLOIMGUI_WITH_IMGUIWS
    namespace ImguiWsUtils
    {
        // =====================================================================================================================
        //                              <ImguiWsUtils>
        // =====================================================================================================================

        std::string gIndexHtmlSource = R"(
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
        <title>_WS_TITLE_</title>

        <script src="incppect.js"></script>
        <script src="imgui-ws.js"></script>
    </head>

    <body style="font-family: Georgia, serif;">
        <script>
            function create_canvas_for_imgui(canvas_id, canvas_width, canvas_height)
            {
                var main_container = document.createElement('div');
                main_container.id = 'main_container_' + canvas_id;
                document.body.appendChild(main_container);

                var canvas_main = document.createElement('canvas');
                canvas_main.id = canvas_id;
                canvas_main.width = canvas_width;
                canvas_main.height = canvas_height;
                canvas_main.style.backgroundColor = 'black';
                canvas_main.tabIndex = '0';
                canvas_main.textContent = 'Your browser does not support the HTML5 canvas tag.';
                main_container.appendChild(canvas_main);
            }


            function init(canvas_id, canvas_width, canvas_height, update_freq_ms)
            {
                var gLastShownId = -1;

                incppect.render = function() {
                    imgui_ws.gl.clearColor(0.45, 0.55, 0.60, 1.00);
                    imgui_ws.gl.clear(imgui_ws.gl.COLOR_BUFFER_BIT);

                    imgui_ws.incppect_textures(this);
                    imgui_ws.incppect_draw_lists(this);
                    imgui_ws.render();

                    var my_id = this.get_int32('my_id[%d]', -1) || 0;
                    if (my_id != gLastShownId) {
                        console.log('imgui_ws: my_id = ' + my_id);
                        gLastShownId = my_id;
                    }
                }

                incppect.onerror = function(evt) {
                    console.error(evt);
                }

                // update_freq_ms: between 16 and 200ms
                // incppect.k_requests_update_freq_ms = document.getElementById('update_freq_ms').value;
                incppect.k_requests_update_freq_ms = update_freq_ms;


                create_canvas_for_imgui(canvas_id, canvas_width, canvas_height);
                incppect.init();
                imgui_ws.set_incppect_handlers(incppect);
                imgui_ws.init(canvas_id);
            }

            // onload: create canvas and init imgui-ws + incppect
            canvas_id = "my_canvas";
            canvas_width = _WS_CANVAS_WIDTH_;
            canvas_height = _WS_CANVAS_HEIGHT_;
            update_freq_ms = 16;
            window.addEventListener('load', () => init(canvas_id, canvas_width, canvas_height, update_freq_ms));

        </script>

    </body>
</html>
        )";

        static std::string ReplaceInString(std::string str, const std::string& from, const std::string& to)
        {
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
            return str;
        }

        static ImGuiKey toImGuiKey(int32_t keyCode) {
            switch (keyCode) {
                case 8: return ImGuiKey_Backspace;
                case 9: return ImGuiKey_Tab;
                case 13: return ImGuiKey_Enter;
                case 16: return ImGuiKey_ModShift;
                case 17: return ImGuiKey_ModCtrl;
                case 18: return ImGuiKey_ModAlt;
                case 19: return ImGuiKey_Pause;
                case 20: return ImGuiKey_CapsLock;
                case 27: return ImGuiKey_Escape;
                case 32: return ImGuiKey_Space;
                case 33: return ImGuiKey_PageUp;
                case 34: return ImGuiKey_PageDown;
                case 35: return ImGuiKey_End;
                case 36: return ImGuiKey_Home;
                case 37: return ImGuiKey_LeftArrow;
                case 38: return ImGuiKey_UpArrow;
                case 39: return ImGuiKey_RightArrow;
                case 40: return ImGuiKey_DownArrow;
                case 45: return ImGuiKey_Insert;
                case 46: return ImGuiKey_Delete;
                case 48: return ImGuiKey_0;
                case 49: return ImGuiKey_1;
                case 50: return ImGuiKey_2;
                case 51: return ImGuiKey_3;
                case 52: return ImGuiKey_4;
                case 53: return ImGuiKey_5;
                case 54: return ImGuiKey_6;
                case 55: return ImGuiKey_7;
                case 56: return ImGuiKey_8;
                case 57: return ImGuiKey_9;
                case 65: return ImGuiKey_A;
                case 66: return ImGuiKey_B;
                case 67: return ImGuiKey_C;
                case 68: return ImGuiKey_D;
                case 69: return ImGuiKey_E;
                case 70: return ImGuiKey_F;
                case 71: return ImGuiKey_G;
                case 72: return ImGuiKey_H;
                case 73: return ImGuiKey_I;
                case 74: return ImGuiKey_J;
                case 75: return ImGuiKey_K;
                case 76: return ImGuiKey_L;
                case 77: return ImGuiKey_M;
                case 78: return ImGuiKey_N;
                case 79: return ImGuiKey_O;
                case 80: return ImGuiKey_P;
                case 81: return ImGuiKey_Q;
                case 82: return ImGuiKey_R;
                case 83: return ImGuiKey_S;
                case 84: return ImGuiKey_T;
                case 85: return ImGuiKey_U;
                case 86: return ImGuiKey_V;
                case 87: return ImGuiKey_W;
                case 88: return ImGuiKey_X;
                case 89: return ImGuiKey_Y;
                case 90: return ImGuiKey_Z;

                    //case 91: return ImGuiKey_LWin;
                    //case 92: return ImGuiKey_RWin;
                    //case 93: return ImGuiKey_Apps;
                case 91: return ImGuiKey_ModSuper;
                case 92: return ImGuiKey_ModSuper;
                case 93: return ImGuiKey_ModSuper;

                case 96: return ImGuiKey_Keypad0;
                case 97: return ImGuiKey_Keypad1;
                case 98: return ImGuiKey_Keypad2;
                case 99: return ImGuiKey_Keypad3;
                case 100: return ImGuiKey_Keypad4;
                case 101: return ImGuiKey_Keypad5;
                case 102: return ImGuiKey_Keypad6;
                case 103: return ImGuiKey_Keypad7;
                case 104: return ImGuiKey_Keypad8;
                case 105: return ImGuiKey_Keypad9;
                case 106: return ImGuiKey_KeypadMultiply;
                case 107: return ImGuiKey_KeypadAdd;
                case 108: return ImGuiKey_KeypadEnter;
                case 109: return ImGuiKey_KeypadSubtract;
                case 110: return ImGuiKey_KeypadDecimal;
                case 111: return ImGuiKey_KeypadDivide;
                case 112: return ImGuiKey_F1;
                case 113: return ImGuiKey_F2;
                case 114: return ImGuiKey_F3;
                case 115: return ImGuiKey_F4;
                case 116: return ImGuiKey_F5;
                case 117: return ImGuiKey_F6;
                case 118: return ImGuiKey_F7;
                case 119: return ImGuiKey_F8;
                case 120: return ImGuiKey_F9;
                case 121: return ImGuiKey_F10;
                case 122: return ImGuiKey_F11;
                case 123: return ImGuiKey_F12;
                case 144: return ImGuiKey_NumLock;
                case 145: return ImGuiKey_ScrollLock;
                case 186: return ImGuiKey_Semicolon;
                case 187: return ImGuiKey_Equal;
                case 188: return ImGuiKey_Comma;
                case 189: return ImGuiKey_Minus;
                case 190: return ImGuiKey_Period;
                case 191: return ImGuiKey_Slash;
                case 219: return ImGuiKey_LeftBracket;
                case 220: return ImGuiKey_Backslash;
                case 221: return ImGuiKey_RightBracket;
                case 222: return ImGuiKey_Apostrophe;
                default: return ImGuiKey_COUNT;
            }

            return ImGuiKey_COUNT;
        }

        struct ImguiWsInputs {
            ImguiWsInputs() {}

            // client control management
            struct ClientData {
                bool hasControl = false;

                std::string ip = "---";
            };

            // client control
            float tControl_s = 10.0f;
            float tControlNext_s = 0.0f;

            int controlIteration = 0;
            int curIdControl = -1;
            std::map<int, ClientData> clients;

            struct InputEvent {
                enum Type {
                    EKey,
                    EMousePos,
                    EMouseButton,
                    EMouseWheel,
                };

                Type type;

                bool isDown = false;

                ImGuiKey key = ImGuiKey_COUNT;
                ImGuiMouseButton mouseButton = -1;
                ImVec2 mousePos;
                float mouseWheelX = 0.0f;
                float mouseWheelY = 0.0f;
            };

            // client input
            std::vector<InputEvent> inputEvents;
            std::string lastAddText = "";

            void handle(ImGuiWS::Event && event)
            {
                switch (event.type) {
                    case ImGuiWS::Event::Connected:
                    {
                        clients[event.clientId].ip = event.ip;
                    }
                        break;
                    case ImGuiWS::Event::Disconnected:
                    {
                        clients.erase(event.clientId);
                    }
                        break;
                    case ImGuiWS::Event::MouseMove:
                    {
                        if (event.clientId == curIdControl) {
                            inputEvents.push_back(InputEvent { InputEvent::Type::EMousePos, false, ImGuiKey_COUNT, -1, { event.mouse_x, event.mouse_y }, 0.0f, 0.0f });
                        }
                    }
                        break;
                    case ImGuiWS::Event::MouseDown:
                    {
                        if (event.clientId == curIdControl) {
                            // map the JS button code to Dear ImGui's button code
                            ImGuiMouseButton butImGui = event.mouse_but;
                            switch (event.mouse_but) {
                                case 1: butImGui = ImGuiMouseButton_Middle; break;
                                case 2: butImGui = ImGuiMouseButton_Right; break;
                            }

                            inputEvents.push_back(InputEvent { InputEvent::Type::EMouseButton, true, ImGuiKey_COUNT, butImGui, { event.mouse_x, event.mouse_y }, 0.0f, 0.0f });
                        }
                    }
                        break;
                    case ImGuiWS::Event::MouseUp:
                    {
                        if (event.clientId == curIdControl) {
                            // map the JS button code to Dear ImGui's button code
                            ImGuiMouseButton butImGui = event.mouse_but;
                            switch (event.mouse_but) {
                                case 1: butImGui = ImGuiMouseButton_Middle; break;
                                case 2: butImGui = ImGuiMouseButton_Right; break;
                            }

                            inputEvents.push_back(InputEvent { InputEvent::Type::EMouseButton, false, ImGuiKey_COUNT, butImGui, { event.mouse_x, event.mouse_y }, 0.0f, 0.0f });
                        }
                    }
                        break;
                    case ImGuiWS::Event::MouseWheel:
                    {
                        if (event.clientId == curIdControl) {
                            inputEvents.push_back(InputEvent { InputEvent::Type::EMouseWheel, false, ImGuiKey_COUNT, -1, { }, event.wheel_x, event.wheel_y });
                        }
                    }
                        break;
                    case ImGuiWS::Event::KeyUp:
                    {
                        if (event.clientId == curIdControl) {
                            if (event.key > 0) {
                                ImGuiKey keyImGui = toImGuiKey(event.key);
                                inputEvents.push_back(InputEvent { InputEvent::Type::EKey, false, keyImGui, -1, { }, 0.0f, 0.0f });
                            }
                        }
                    }
                        break;
                    case ImGuiWS::Event::KeyDown:
                    {
                        if (event.clientId == curIdControl) {
                            if (event.key > 0) {
                                ImGuiKey keyImGui = toImGuiKey(event.key);
                                inputEvents.push_back(InputEvent { InputEvent::Type::EKey, true, keyImGui, -1, { }, 0.0f, 0.0f });
                            }
                        }
                    }
                        break;
                    case ImGuiWS::Event::KeyPress:
                    {
                        if (event.clientId == curIdControl) {
                            lastAddText.resize(1);
                            lastAddText[0] = event.key;
                        }
                    }
                        break;
                    default:
                    {
                        printf("Unknown input event\n");
                    }
                }
            }

            void update()
            {
                if (clients.size() > 0 && (clients.find(curIdControl) == clients.end() || ImGui::GetTime() > tControlNext_s)) {
                    if (clients.find(curIdControl) != clients.end()) {
                        clients[curIdControl].hasControl = false;
                    }
                    int k = ++controlIteration % clients.size();
                    auto client = clients.begin();
                    std::advance(client, k);
                    client->second.hasControl = true;
                    curIdControl = client->first;
                    tControlNext_s = ImGui::GetTime() + tControl_s;
                    ImGui::GetIO().ClearInputKeys();
                }

                if (clients.size() == 0) {
                    curIdControl = -1;
                }

                if (curIdControl > 0) {
                    {
                        auto & io = ImGui::GetIO();

                        if (lastAddText.size() > 0) {
                            io.AddInputCharactersUTF8(lastAddText.c_str());
                        }

                        for (const auto & event : inputEvents) {
                            switch (event.type) {
                                case InputEvent::Type::EKey:
                                {
                                    if (ImGui::IsNamedKeyOrModKey(event.key))
                                        io.AddKeyEvent(event.key, event.isDown);
                                } break;
                                case InputEvent::Type::EMousePos:
                                {
                                    io.AddMousePosEvent(event.mousePos.x, event.mousePos.y);
                                } break;
                                case InputEvent::Type::EMouseButton:
                                {
                                    io.AddMouseButtonEvent(event.mouseButton, event.isDown);
                                    io.AddMousePosEvent(event.mousePos.x, event.mousePos.y);
                                } break;
                                case InputEvent::Type::EMouseWheel:
                                {
                                    io.AddMouseWheelEvent(event.mouseWheelX, event.mouseWheelY);
                                } break;
                            };
                        }
                    }

                    inputEvents.clear();
                    lastAddText = "";
                }
            }

        };

        ImGuiWS gImguiWS;
        ImguiWsInputs gImguiWsInputs;

        void Create()
        {
            auto& remoteParams = HelloImGui::GetRunnerParams()->remoteParams;
            int32_t port = (int32_t)HelloImGui::GetRunnerParams()->remoteParams.wsPort;
            gImguiWS.init(port, remoteParams.wsHttpRootFolder, {"", "index.html"});
            if (remoteParams.wsProvideIndexHtml)
            {
                std::string indexHtml = gIndexHtmlSource;
                indexHtml = ReplaceInString(indexHtml, "_WS_TITLE_", HelloImGui::GetRunnerParams()->appWindowParams.windowTitle);
                indexHtml = ReplaceInString(indexHtml, "_WS_CANVAS_WIDTH_", std::to_string(HelloImGui::GetRunnerParams()->appWindowParams.windowGeometry.size[0]));
                indexHtml = ReplaceInString(indexHtml, "_WS_CANVAS_HEIGHT_", std::to_string(HelloImGui::GetRunnerParams()->appWindowParams.windowGeometry.size[1]));
                gImguiWS.addResource("/index.html", indexHtml);
            }
        }

        static uint32_t ToUint32(void * ptr)
        {
            return (uint32_t) (uintptr_t) ptr;
        }
        static uint32_t ToUint32(unsigned int x)
        {
            return (uint32_t) x;
        }

        void SendFonts()
        {
            unsigned char * pixels;
            int width, height;
            // Try with Alpha8
            ImGui::GetIO().Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
            if (pixels != nullptr)
            {
                auto texID = ToUint32(ImGui::GetIO().Fonts->TexID);
                gImguiWS.setTexture(texID, ImGuiWS::Texture::Type::Alpha8, width, height, (const char *) pixels);
                return;
            }
            // Try with RGBA32
            ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
            if (pixels != nullptr)
            {
                auto texID = ToUint32(ImGui::GetIO().Fonts->TexID);
                gImguiWS.setTexture(texID, ImGuiWS::Texture::Type::RGBA32, width, height, (const char *) pixels);
                return;
            }
            fprintf(stderr, "SendFonts: Could not get font texture data\n");
        }

        void HeartBeat_PreImGuiNewFrame()
        {
            ScreenSize appSize = HelloImGui::GetRunnerParams()->appWindowParams.windowGeometry.size;
            ImVec2 appSizeImVec2 = ImVec2((float)appSize[0], (float)appSize[1]);
            auto& io = ImGui::GetIO();
            io.DisplaySize = appSizeImVec2;

            // websocket event handling
            auto events = gImguiWS.takeEvents();
            for (auto & event : events) {
                gImguiWsInputs.handle(std::move(event));
            }
            gImguiWsInputs.update();
        }

        void Heartbeat_PostImGuiRender()
        {
            // store ImDrawData for asynchronous dispatching to WS clients
            gImguiWS.setDrawData(ImGui::GetDrawData());
        }


        // =====================================================================================================================
        //                              </ImguiWsUtils>
        // =====================================================================================================================
    } // namespace ImguiWsUtils
#endif

bool RemoteDisplayHandler::IsConnectedToRemoteDisplay()
{
    if (!ShouldRemoteDisplay())
        return false;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
        return NetimguiUtils::gNetImGuiWrapper->isConnected();
    #endif
    #ifdef HELLOIMGUI_WITH_IMGUIWS
        auto nConnected = ImguiWsUtils::gImguiWS.nConnected();
        if (nConnected > 0)
            return true;
        return nConnected > 0;
    #endif
    return false;
}

bool RemoteDisplayHandler::ShouldRemoteDisplay()
{
    #if defined(HELLOIMGUI_WITH_NETIMGUI) || defined(HELLOIMGUI_WITH_IMGUIWS)
    return HelloImGui::GetRunnerParams()->remoteParams.enableRemoting;
    #else
    return false;
    #endif
}

bool RemoteDisplayHandler::CheckDpiAwareParamsChanges()
{
    if (!IsConnectedToRemoteDisplay())
        return false;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    return NetimguiUtils::UpdateDpiAwareParams_Netimgui();
    #else
    return false;
    #endif
}

void RemoteDisplayHandler::Create()
{
    if (!ShouldRemoteDisplay())
        return;
    IM_ASSERT(!IsConnectedToRemoteDisplay() && "RemoteDisplayHandler::Create: Already connected to server");
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    NetimguiUtils::gNetImGuiWrapper = std::make_unique<NetimguiUtils::NetImGuiWrapper>();
    #endif
    #ifdef HELLOIMGUI_WITH_IMGUIWS
    ImguiWsUtils::Create();
    #endif
}

void RemoteDisplayHandler::Shutdown()
{
    if (!IsConnectedToRemoteDisplay())
        return;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    NetimguiUtils::gNetImGuiWrapper.reset();
    #endif
}

void RemoteDisplayHandler::Heartbeat_PreImGuiNewFrame()
{
    if (!ShouldRemoteDisplay())
        return;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    NetimguiUtils::gNetImGuiWrapper->HeartBeat_PreImGuiNewFrame();
    #endif
    #ifdef HELLOIMGUI_WITH_IMGUIWS
    ImguiWsUtils::HeartBeat_PreImGuiNewFrame();
    #endif
}

void RemoteDisplayHandler::Heartbeat_PostImGuiRender()
{
    if (!ShouldRemoteDisplay())
        return;
    #ifdef HELLOIMGUI_WITH_IMGUIWS
    ImguiWsUtils::Heartbeat_PostImGuiRender();
    #endif
}


void RemoteDisplayHandler::TransmitWindowSizeToDisplay(ScreenSize windowSize)
{
    if (IsConnectedToRemoteDisplay())
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
    if (ShouldRemoteDisplay())
    {
        #ifdef HELLOIMGUI_WITH_NETIMGUI
        NetimguiUtils::gNetImGuiWrapper = std::make_unique<NetimguiUtils::NetImGuiWrapper>();
        NetimguiUtils::gNetImGuiWrapper->sendFonts();
        #endif
        #ifdef HELLOIMGUI_WITH_IMGUIWS
        ImguiWsUtils::SendFonts();
        #endif
    }
}

bool RemoteDisplayHandler::CanQuitApp()
{
    if (!ShouldRemoteDisplay())
        return true;
    #ifdef HELLOIMGUI_WITH_NETIMGUI
    return true; // ?
    #endif
    #ifdef HELLOIMGUI_WITH_IMGUIWS
    return false;
    #endif
    return true;
}

}  // namespace HelloImGui
