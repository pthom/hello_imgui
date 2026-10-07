#pragma once

namespace HelloImGui
{
    // The mouse wheel keeps scrolling the window it started on: when the mouse travels over an item that takes the
    // wheel (a plot that zooms, an image), the item sees no wheel until the session ends (no event for 0.7 s, as
    // ImGui's own lock of the scrolled window). A wheel that starts on such an item is the item's, as today.
    // Called by the runner right after ImGui::NewFrame(), before any widget. Nothing to do with touch screens.
    void UpdateWheelSession();
    // Called by the runner right before ImGui::NewFrame(): notes which item claimed the wheel in the last frame
    void WheelSessionBeforeNewFrame();
}
