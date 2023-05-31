#include "doctest.h"
#include "hello_imgui/internal/hello_imgui_ini_settings.h"


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