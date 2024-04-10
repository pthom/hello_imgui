#include "NetImguiServer_App.h"



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
    auto init_net_imgui_server = [&]()
    {
        // Start the NetImgui server
        std::string cmdArgs;
        for (size_t i = 0; i < size_t(argc); ++i)
            cmdArgs += std::string(argv[i]) + " ";
        if (!cmdArgs.empty())
            cmdArgs.pop_back();
        if (! NetImguiServer::App::Startup(cmdArgs.c_str()))
            return 1;
    };

    auto handle_dpi_awareness = []()
    {
        // @SAMPLE_EDIT (DPI Awareness)

        //        float scaleX, scaleY;
        //        ImGui_ImplGlfw_Data* bd = ImGui_ImplGlfw_GetBackendData();
        //        glfwGetWindowContentScale(bd->Window, &scaleX, &scaleY);
        //
        //        float maxScale = scaleX > scaleY ? scaleX : scaleY;
        //        float windowDPI = (float)(NetImguiServer::UI::kWindowDPIDefault) * maxScale; // = kWindowDPIDefault=96
        //        NetImguiServer::UI::SetWindowDPI((int)windowDPI);
        //        ImGui::GetIO().FontGlobalScale = NetImguiServer::UI::GetFontDPIScale();


        //        HelloImGui::GetRunnerParams()->dpiAwareParams.fontRenderingScale;
        //        HelloImGui::GetRunnerParams()->dpiAwareParams.dpiWindowSizeFactor;
        //        HelloImGui::GetRunnerParams()->dpiAwareParams.DpiFontLoadingFactor();
        float windowDPI = HelloImGui::GetRunnerParams()->dpiAwareParams.fontRenderingScale
                          * (float)(NetImguiServer::UI::kWindowDPIDefault); // 96
        NetImguiServer::UI::SetWindowDPI((int)windowDPI);

    };


    // Runner Params
    HelloImGui::RunnerParams runnerParams;

    runnerParams.callbacks.PostInit = [&init_net_imgui_server]() {
        init_net_imgui_server();
        //ImGui::GetIO().ConfigFlags = ImGui::GetIO().ConfigFlags | ImGuiConfigFlags_DockingEnable;
    };

    runnerParams.callbacks.PreNewFrame = [&handle_dpi_awareness]() {
        // Request each client to update their drawing content
        NetImguiServer::App::UpdateRemoteContent();
        handle_dpi_awareness();
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
