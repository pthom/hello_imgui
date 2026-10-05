#pragma once
#ifdef __EMSCRIPTEN__

// The virtual keyboard of a phone, for ImGui's text widgets. The browser shows it only for a text field focused
// inside a touch handler, and ImGui learns of a tap two frames after the finger lifted: so the page keeps a hidden
// text field, and focuses it on the second tap: on a small keyboard button shown next to the active text widget, or
// on the widget itself. What is typed in the field goes to ImGui each frame (characters, and the special keys).
namespace HelloImGui
{
    void InstallEmscriptenKeyboard();  // once, after the pointer probe (a second call does nothing)
    void UpdateEmscriptenKeyboard();   // before each poll: tells the page what ImGui wants, feeds ImGui what was typed
}
#endif
