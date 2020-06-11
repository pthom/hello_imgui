#include "hello_imgui/hello_imgui.h"
#include "hello_imgui/widgets/logger.h"

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
    RocketState rocketState;
};

// CommandGui: the widgets on the left panel
void CommandGui(AppState & state, HelloImGui::Widgets::Logger & logger)
{
    // Edit 1 float using a slider from 0.0f to 1.0f
    if (ImGui::SliderFloat("float", &state.f, 0.0f, 1.0f))
        logger.warning("state.f was changed to %f", state.f);

    // Buttons return true when clicked (most widgets return true when edited/activated)
    if (ImGui::Button("Button")) {
        state.counter++;
        logger.info("Button was pressed", state.f);
    }
    ImGui::SameLine();
    ImGui::Text("counter = %d", state.counter);

    if (state.rocketState == AppState::RocketState::Init)
        if (ImGui::Button("Launch rocket")) {
            state.rocketState = AppState::RocketState::Preparing;
            logger.warning("Rocket is being prepared");
        }
    if (state.rocketState == AppState::RocketState::Preparing)
    {
        ImGui::Text("Please Wait");
        state.rocket_progress += 0.003f;
        if (state.rocket_progress >= 1.f)
        {
            state.rocketState = AppState::RocketState::Launched;
            logger.warning("Rocket was launched!");
        }
    }
    if (state.rocketState == AppState::RocketState::Launched) {
        ImGui::Text("Rocket Launched");
        if (ImGui::Button("Reset Rocket"))
        {
            state.rocketState = AppState::RocketState ::Init;
            state.rocket_progress = 0.f;
        }
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

int main()
{
    // Our application state
    AppState appState;

    // Hello ImGui params (they hold the settings as well as the Gui callbacks)
    HelloImGui::RunnerParams runnerParams;

    // Provide a full screen dock space
    runnerParams.imGuiWindowParams.defaultImGuiWindowType =
        HelloImGui::DefaultImGuiWindowType::ProvideFullScreenDockSpace;

    // Define the docking splits, i.e the way the screen space is split in different
    // target zones for the dockable windows
    runnerParams.dockingParams.dockingSplits =
        {
            // First, add a space named "BottomSpace" whose height is 25% of the app height
            // This will split the preexisting default dockspace "MainDockSpace"
            // (which is provided by "Hello ImGui") in two parts.
            { "MainDockSpace", "BottomSpace", ImGuiDir_Down, 0.25 },
            // Then, add a space to the left which occupies a column
            // whose width is 25% of the app height
            { "MainDockSpace", "LeftSpace", ImGuiDir_Left, 0.25 }
            // We now have three spaces: "MainDockSpace", "BottomSpace", and "LeftSpace"
        };

    // HelloImGui::Widgets::Logger is a Dockable Window, with the title "Logs"
    // and placed in the dockspace "BottomSpace"
    // (see src/hello_imgui/widgets/logger.h)
    HelloImGui::Widgets::Logger logger("Logs", "BottomSpace");

    // Define our dockable windows :
    //  - Each window provide a Gui callback
    runnerParams.dockingParams.dockableWindows =
        {
            // A Command panel named "Commands" will be placed in "LeftSpace".
            // Its Gui is provided by a lambda that calls "CommandGui"
            {"Commands", "LeftSpace", [&]() { CommandGui(appState, logger); }},

            // A Log  window named "Logs" will be placed in "BottomSpace"
            // It uses HelloImGui::Widgets::Logger
            logger,

            // A Window named "Dear ImGui Demo" will be placed in "MainDockSpace".
            // Its Gui function is *not* provided here.
            // This way, we can define the Gui of this window elsewhere: as long
            // as we create a window named "Dear ImGui Demo", it will be placed
            // in "MainDockSpace".
            {"Dear ImGui Demo", "MainDockSpace", {}},
        };

    // We use the default Menu and status bar of Hello ImGui
    runnerParams.imGuiWindowParams.showStatusBar = true;

    // runnerParams.callbacks.ShowGui is the default Gui callback
    runnerParams.callbacks.ShowGui = []() {
        // We call ImGui::ShowDemoWindow, which will create a window named "Dear ImGui Demo".
        // It will automatically be placed in "MainDockSpace"
        ImGui::ShowDemoWindow();
    };

    // Menu bar: we use the default menu of Hello ImGui,
    // to which we add some more items
    runnerParams.imGuiWindowParams.showMenuBar = true;
    runnerParams.callbacks.ShowMenus = [&logger]() {
        if (ImGui::BeginMenu("My Menu"))
        {
            if (ImGui::MenuItem("Test me"))
                logger.warning("It works");
            ImGui::EndMenu();
        }
    };

    // Status bar:
    runnerParams.imGuiWindowParams.showStatusBar = true;
    // uncomment next line in order to hide the FPS in the status bar
    // runnerParams.imGuiWindowParams.showStatus_Fps = false;
    runnerParams.callbacks.ShowStatus = [&appState] { StatusBarGui(appState); };

    // Then, we run the app
    HelloImGui::Run(runnerParams);
}

