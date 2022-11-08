#include "hello_imgui/hello_imgui.h"

#ifdef HELLOIMGUI_USE_SDL_OPENGL3
#include <SDL.h>
#endif

// Struct that holds the application's state
struct AppState
{
    float f = 0.0f;
    int counter = 0;

    enum class RocketState
    {
        Init,
        Preparing,
        Launched
    };
    float rocket_progress = 0.f;
    RocketState rocketState = RocketState::Init;
};

// MyLoadFonts: demonstrate
// * how to load additional fonts
// * how to use assets from the local assets/ folder
//   Files in the application assets/ folder are embedded automatically
//   (on iOS/Android/Emscripten)
ImFont * gAkronimFont = nullptr;
void MyLoadFonts()
{
    // First, we load the default fonts (the font that was loaded first is the default font)
    HelloImGui::ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesomeIcons();

    // Then we load a second font from
    // Since this font is in a local assets/ folder, it was embedded automatically
    std::string fontFilename = "fonts/Akronim-Regular.ttf";
    gAkronimFont = HelloImGui::LoadFontTTF_WithFontAwesomeIcons(fontFilename, 40.f);
}


// CommandGui: the widgets on the left panel
void CommandGui(AppState & state)
{
    ImGui::PushFont(gAkronimFont);
    ImGui::Text("Hello  " ICON_FA_SMILE);
    HelloImGui::ImageFromAsset("world.jpg");
    ImGui::PopFont();
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(
            "The custom font and the globe image below were loaded \n"
            "from the application assets folder\n"\
            "(those files are embedded automatically).");
    }

    ImGui::Separator();

    // Edit 1 float using a slider from 0.0f to 1.0f
    if (ImGui::SliderFloat("float", &state.f, 0.0f, 1.0f))
        HelloImGui::Log(HelloImGui::LogLevel::Warning, "state.f was changed to %f", state.f);

    // Buttons return true when clicked (most widgets return true when edited/activated)
    if (ImGui::Button("Button")) {
        state.counter++;
        HelloImGui::Log(HelloImGui::LogLevel::Info, "Button was pressed", state.f);
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", state.counter);

    switch(state.rocketState)
    {
        case AppState::RocketState::Init:
            if (ImGui::Button(ICON_FA_ROCKET " Launch rocket"))
            {
                state.rocketState = AppState::RocketState::Preparing;
                HelloImGui::Log(HelloImGui::LogLevel::Warning, "Rocket is being prepared");
            }
            break;
        case AppState::RocketState::Preparing:
            ImGui::Text(ICON_FA_ROCKET " Please Wait");
            state.rocket_progress += 0.003f;
            if (state.rocket_progress >= 1.f)
            {
                state.rocketState = AppState::RocketState::Launched;
                HelloImGui::Log(HelloImGui::LogLevel::Warning, "Rocket was launched!");
            }
            break;
        case AppState::RocketState::Launched:
            ImGui::Text(ICON_FA_ROCKET " Rocket Launched");
            if (ImGui::Button("Reset Rocket"))
            {
                state.rocketState = AppState::RocketState ::Init;
                state.rocket_progress = 0.f;
            }
            break;
    }
}

// Our Gui in the status bar
void StatusBarGui(const AppState &appState)
{
    if (appState.rocketState == AppState::RocketState::Preparing) {
        ImGui::Text("Rocket completion: ");
        ImGui::SameLine();
        ImGui::ProgressBar(appState.rocket_progress, ImVec2(100.f, 15.f));
    }
}

// Example of an optional native event callback for the backend (implemented here only for SDL)
bool NativeBackendEventCallback(void * event)
{
#ifdef HELLOIMGUI_USE_SDL_OPENGL3
    SDL_Event*  sdlEvent = (SDL_Event *)event;
        switch( sdlEvent->type)
        {
            case SDL_KEYDOWN:
                HelloImGui::Log(HelloImGui::LogLevel::Warning, "It SDL_KEYDOWN detected");
                return false; // if you return true, the event is not processd further
        }
        return false;
#else
    return false;
#endif
};


int main(int, char **)
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Part 1: Define the application state, fill the status and menu bars, and load additional font
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Our application state
    AppState appState;

    // Hello ImGui params (they hold the settings as well as the Gui callbacks)
    HelloImGui::RunnerParams runnerParams;

    runnerParams.appWindowParams.windowTitle = "Docking demo";
    runnerParams.appWindowParams.windowGeometry.size = {800, 600};
    runnerParams.appWindowParams.restorePreviousGeometry = true;

    //
    // Status bar
    //
    // We use the default status bar of Hello ImGui
    runnerParams.imGuiWindowParams.showStatusBar = true;
    // uncomment next line in order to hide the FPS in the status bar
    // runnerParams.imGuiWindowParams.showStatus_Fps = false;
    runnerParams.callbacks.ShowStatus = [&appState] { StatusBarGui(appState); };

    //
    // Menu bar
    //
    // We use the default menu of Hello ImGui, to which we add some more items
    runnerParams.imGuiWindowParams.showMenuBar = true;
    runnerParams.callbacks.ShowMenus = []() {
        if (ImGui::BeginMenu("My Menu"))
        {
            if (ImGui::MenuItem("Test me"))
                HelloImGui::Log(HelloImGui::LogLevel::Debug, "It works");
            ImGui::EndMenu();
        }
    };

    // Custom load fonts
    runnerParams.callbacks.LoadAdditionalFonts = MyLoadFonts;

    // optional native events handling
    runnerParams.callbacks.AnyBackendEventCallback = NativeBackendEventCallback;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Part 2: Define the application layout and windows
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //
    //    2.1 Define the docking splits,
    //    i.e. the way the screen space is split in different target zones for the dockable windows
    //     We want to split "MainDockSpace" (which is provided automatically) into three zones, like this:
    //
    //    ___________________________________________
    //    |        |                                |
    //    | Left   |                                |
    //    | Space  |    MainDockSpace               |
    //    |        |                                |
    //    |        |                                |
    //    |        |                                |
    //    -------------------------------------------
    //    |     BottomSpace                         |
    //    -------------------------------------------
    //

    // First, tell HelloImGui that we want full screen dock space (this will create "MainDockSpace")
    runnerParams.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;
    // In this demo, we also demonstrate multiple viewports.
    // you can drag windows outside out the main window in order to put their content into new native windows
    runnerParams.imGuiWindowParams.enableViewports = true;

    // Then, add a space named "BottomSpace" whose height is 25% of the app height.
    // This will split the preexisting default dockspace "MainDockSpace" in two parts.
    HelloImGui::DockingSplit splitMainBottom;
    splitMainBottom.initialDock = "MainDockSpace";
    splitMainBottom.newDock = "BottomSpace";
    splitMainBottom.direction = ImGuiDir_Down;
    splitMainBottom.ratio = 0.25f;

    // Then, add a space to the left which occupies a column whose width is 25% of the app width
    HelloImGui::DockingSplit splitMainLeft;
    splitMainLeft.initialDock = "MainDockSpace";
    splitMainLeft.newDock = "LeftSpace";
    splitMainLeft.direction = ImGuiDir_Left;
    splitMainLeft.ratio = 0.25f;

    // Finally, transmit these splits to HelloImGui
    runnerParams.dockingParams.dockingSplits = { splitMainBottom, splitMainLeft };

    //
    // 2.1 Define our dockable windows : each window provide a Gui callback, and will be displayed
    //     in a docking split.
    //

    // A Command panel named "Commands" will be placed in "LeftSpace". Its Gui is provided calls "CommandGui"
    HelloImGui::DockableWindow commandsWindow;
    commandsWindow.label = "Commands";
    commandsWindow.dockSpaceName = "LeftSpace";
    commandsWindow.GuiFunction = [&appState]() { CommandGui(appState); };
    // A Log  window named "Logs" will be placed in "BottomSpace". It uses the HelloImGui logger gui
    HelloImGui::DockableWindow logsWindow;
    logsWindow.label = "Logs";
    logsWindow.dockSpaceName = "BottomSpace";
    logsWindow.GuiFunction = HelloImGui::LogGui;
    // A Window named "Dear ImGui Demo" will be placed in "MainDockSpace"
    HelloImGui::DockableWindow demoWindow;
    demoWindow.label = "Dear ImGui Demo";
    demoWindow.dockSpaceName = "MainDockSpace";
    demoWindow.GuiFunction = [] { ImGui::ShowDemoWindow(); };
    // Finally, transmit these windows to HelloImGui
    runnerParams.dockingParams.dockableWindows = { commandsWindow, logsWindow, demoWindow };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Part 3: Run the app
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    HelloImGui::Run(runnerParams);
    return 0;
}
