#include "NetImguiServer_App.h"
#include "NetImguiServer_UI.h"


// @SAMPLE_EDIT
// Note: The 'imgui_impl_opengl3_loader.h' included by Dear Imgui Backend has been replaced 
// with our own version without stripped symbol. The server App needs a few extra 
// function that are not shipped with Dear ImGui.
// The unstripped version comes from : https://github.com/dearimgui/gl3w_stripped/releases 

#include <string>
#include "NetImguiServer_UI.h"
#include "hello_imgui/hello_imgui.h"
//#include "backends/imgui_impl_opengl3.cpp"
//#include "backends/imgui_impl_glfw.cpp"
////=================================================================================================

//
//#include "imgui.h"
//#include "imgui_impl_glfw.h"
//#include "imgui_impl_opengl3.h"
//#include <stdio.h>
//#if defined(IMGUI_IMPL_OPENGL_ES2)
//#include <GLES2/gl2.h>
//#endif
//#include <GLFW/glfw3.h> // Will drag system OpenGL headers

int main(int argc, char **argv)
{
    auto pass_cmd_line_args_to_server = [&]()
    {
        // Start the NetImgui server
        std::string cmdArgs;
        for (size_t i = 0; i < size_t(argc); ++i)
            cmdArgs += std::string(argv[i]) + " ";
        if (!cmdArgs.empty())
            cmdArgs.pop_back();
        if (! NetImguiServer::App::Startup(cmdArgs.c_str()))
            exit(1);
    };

    // Runner Params
    HelloImGui::RunnerParams runnerParams;

    //NetImguiServer::UI::SetAllowOnlyOneClient();
	//NetImguiServer::UI::SetHideMainMenu();

    NetImguiServer::UI::SetCallbackWindowSize96PPI([](uint16_t width, uint16_t height) {
        HelloImGui::ChangeWindowSize({(int)width, (int)height});
    });

    runnerParams.callbacks.PostInit = [&pass_cmd_line_args_to_server]() {
        pass_cmd_line_args_to_server();
        NetImguiServer::UI::SetUseServerDPISettings(HelloImGui::DpiFontLoadingFactor());
    };

    runnerParams.callbacks.PreNewFrame = []() {
        // Request each client to update their drawing content
        NetImguiServer::App::UpdateRemoteContent();
    };

    runnerParams.callbacks.ShowGui = []() {
        ImVec4 clear_color = NetImguiServer::UI::DrawImguiContent();
        HelloImGui::GetRunnerParams()->imGuiWindowParams.backgroundColor = clear_color;
    };


    runnerParams.callbacks.BeforeExit = []() {
        NetImguiServer::App::Shutdown();
    };

    runnerParams.imGuiWindowParams.defaultImGuiWindowType = HelloImGui::DefaultImGuiWindowType::NoDefaultWindow;

//    runnerParams.dpiAwareParams.dpiWindowSizeFactor = 1.0f;
//    runnerParams.dpiAwareParams.fontRenderingScale = 1.0f;

    runnerParams.fpsIdling.fpsIdle = 30.f;

    // Start the HelloImGui runner
    HelloImGui::Run(runnerParams);
}

/*
int main(int argc, char **argv)
{

    while (ok && !glfwWindowShouldClose(window))
	//=========================================================================================
    {
        glfwPollEvents();


		//=========================================================================================
        // @SAMPLE_EDIT (Invoke our own Imgui content drawing, instead of the sample code)
        clear_color = NetImguiServer::UI::DrawImguiContent(); 
        IM_UNUSED(show_demo_window);
        IM_UNUSED(show_another_window);
		//=========================================================================================

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    	
        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
	NetImguiServer::App::Shutdown(); // @SAMPLE_EDIT (Deallocate resources)
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
*/
