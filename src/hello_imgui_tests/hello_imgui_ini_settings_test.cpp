#include "doctest.h"
#include "hello_imgui/internal/hello_imgui_ini_settings.h"

#include <cstdio>
#include <filesystem>
#include <fstream>


TEST_CASE("testing HelloImGuiIniSettings::SplitIniParts")
{
    std::string s = R"(;;; !!! This configuration is handled by HelloImGui and stores several Ini Files, separated by markers like this:
           ;;;<<<INI_NAME>>>;;;

;;;<<<imgui>>>;;;
[Window][Main window (title bar invisible)]
Pos=0,0
Size=1000,800
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Docking][Data]

;;;<<<appWindow>>>;;;
[WIN]
WindowPosition=393,238
WindowSize=971,691

;;;<<<otherIniInfo>>>;;;
[WIN]
WindowPosition=393,238
WindowSize=971,691

)";
    auto iniParts = HelloImGui::HelloImGuiIniSettings::SplitIniParts(s);

    CHECK(iniParts.Parts.size() == 3);
    CHECK(iniParts.Parts[0].Name == "imgui");
    CHECK(iniParts.Parts[1].Name == "appWindow");
    CHECK(iniParts.Parts[2].Name == "otherIniInfo");
    CHECK(iniParts.Parts[0].Content == R"([Window][Main window (title bar invisible)]
Pos=0,0
Size=1000,800
Collapsed=0

[Window][Debug##Default]
Pos=60,60
Size=400,400
Collapsed=0

[Docking][Data]

)");
    CHECK(iniParts.Parts[1].Content == R"([WIN]
WindowPosition=393,238
WindowSize=971,691

)");

    auto joined = HelloImGui::HelloImGuiIniSettings::JoinIniParts(iniParts);
    CHECK(joined == s + "\n");
}


// cf https://github.com/pthom/hello_imgui/issues/168
TEST_CASE("testing HelloImGuiIniSettings::LoadLastRunWindowBounds")
{
    auto writeTempIniFile = [](const std::string& appWindowContent) -> std::string
    {
        std::string filename = std::filesystem::temp_directory_path() / "hello_imgui_test_window_bounds.ini";
        std::ofstream ofs(filename);
        ofs << ";;;<<<AppWindow>>>;;;\n[AppWindow]\n" << appWindowContent;
        return filename;
    };

    SUBCASE("negative position is restored (monitor left of or above the primary monitor)")
    {
        auto filename = writeTempIniFile("WindowPosition=-1466,140\nWindowSize=800,600\n");
        auto bounds = HelloImGui::HelloImGuiIniSettings::LoadLastRunWindowBounds(filename);
        REQUIRE(bounds.has_value());
        CHECK(bounds->position == HelloImGui::ScreenPosition{-1466, 140});
        CHECK(bounds->size == HelloImGui::ScreenSize{800, 600});
        std::remove(filename.c_str());
    }
    SUBCASE("unparsable position is rejected")
    {
        auto filename = writeTempIniFile("WindowPosition=abc,140\nWindowSize=800,600\n");
        auto bounds = HelloImGui::HelloImGuiIniSettings::LoadLastRunWindowBounds(filename);
        CHECK(!bounds.has_value());
        std::remove(filename.c_str());
    }
    SUBCASE("non positive size is rejected")
    {
        auto filename = writeTempIniFile("WindowPosition=100,100\nWindowSize=-800,600\n");
        auto bounds = HelloImGui::HelloImGuiIniSettings::LoadLastRunWindowBounds(filename);
        CHECK(!bounds.has_value());
        std::remove(filename.c_str());
    }
}