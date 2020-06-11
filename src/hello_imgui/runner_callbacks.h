#pragma once
#include "hello_imgui/imgui_default_settings.h"
#include <functional>

namespace HelloImGui
{
/**
@@md
**VoidFunctionPointer** can hold any void(void) function.
````cpp
using VoidFunction = std::function<void(void)>
````
__NoAction__ is a VoidFunction that does nothing.
@@md
**/
using VoidFunction = std::function<void(void)>;
inline void NoAction() {}

/**
 @@md
 **RunnerCallbacks** is a struct that contains the callbacks that are called by the application

 _Members_

* `ShowGui`: _VoidFunction, default=empty_.
  Fill it with a function that will add your widgets.

* `ShowMenus`: _VoidFunction, default=empty_.
    A function that will render your menus. Fill it with a function that will add ImGui menus by calling:
    _ImGui::BeginMenu(...) / ImGui::MenuItem(...) / ImGui::EndMenu()_
    
    _Notes:_
    * you do not need to call _ImGui::BeginMenuBar_ and _ImGui::EndMenuBar_
    * Some default menus can be provided: see _ImGuiWindowParams_ options 
      (_showMenuBar, showMenu_App_QuitZoom, showMenu_View_)

* `ShowStatus`: _VoidFunction, default=empty_.
  A function that will add items to the status bar. Use small items (ImGui::Text for example),
  since the height of the status is 30. Also, remember to call ImGui::SameLine() between items.

* `PostInit`: _VoidFunction, default=empty_.
    You can here add a function that will be called once after OpenGL and ImGui are inited

* `LoadAdditionalFonts`: *VoidFunction, default=_LoadDefaultFont_WithFontAwesome*.
   A function that is called when fonts are ready to be loaded.
   By default, _LoadDefaultFont_WithFontAwesome_ is called but you can copy-customize it.

* `SetupImGuiConfig`: _VoidFunction, default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig_.
    If needed, change ImGui config via SetupImGuiConfig (enable docking, gamepad, etc)

* `SetupImGuiStyle`: _VoidFunction, default=_ImGuiDefaultSettings::SetupDefaultImGuiConfig_.
    If needed, setup your own style by providing your own SetupImGuiStyle callback

@@md
 */
struct RunnerCallbacks
{
    VoidFunction ShowGui = {};

    VoidFunction ShowMenus = {};

    VoidFunction ShowStatus = {};

    VoidFunction PostInit = NoAction;

    VoidFunction LoadAdditionalFonts = ImGuiDefaultSettings::LoadDefaultFont_WithFontAwesome;

    VoidFunction SetupImGuiConfig = ImGuiDefaultSettings::SetupDefaultImGuiConfig;

    VoidFunction SetupImGuiStyle = ImGuiDefaultSettings::SetupDefaultImGuiStyle;
};

}  // namespace HelloImGui
