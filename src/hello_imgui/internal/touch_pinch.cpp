#include "hello_imgui/internal/touch_pinch.h"
#include "hello_imgui/internal/touch_scroll.h"

#include "imgui.h"
#include "imgui_internal.h"  // ImClamp

namespace HelloImGui
{
namespace
{
    constexpr float kFontScaleMin = 0.5f, kFontScaleMax = 5.f;  // the range of the explorer's "Font scale" slider

    struct State
    {
        int count = 0;
        float pinchScale = 1.f;
        bool pinching = false;
        float baseScale = 1.f;  // style.FontScaleMain when the second finger landed
    };
    State gState;
}  // namespace

void SetTouchPointers(int count, float pinchScale)
{
    gState.count = count;
    gState.pinchScale = pinchScale;
}

void UpdateTouchPinch(TouchPinchMode mode, bool interruptsWidgets)
{
    State& s = gState;
    bool twoFingers = (mode != TouchPinchMode::Disabled) && s.count >= 2;
    if (twoFingers && !s.pinching)
    {
        // The first finger was a press for the swipe layer, or for a widget: the pinch takes it
        if (!TouchScrollLetGo(interruptsWidgets))
            return;  // a widget keeps its drag: no pinch this time
        s.pinching = true;
        s.baseScale = ImGui::GetStyle().FontScaleMain;
    }
    if (s.pinching && !twoFingers)
        s.pinching = false;
    if (s.pinching && mode == TouchPinchMode::FontScale)
        ImGui::GetStyle().FontScaleMain = ImClamp(s.baseScale * s.pinchScale, kFontScaleMin, kFontScaleMax);
}

}  // namespace HelloImGui
