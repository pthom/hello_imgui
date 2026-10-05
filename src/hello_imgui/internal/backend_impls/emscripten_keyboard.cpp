#ifdef __EMSCRIPTEN__
#include "hello_imgui/internal/backend_impls/emscripten_keyboard.h"

#include "imgui.h"
#include "imgui_internal.h"  // g.PlatformImeDataPrev
#include <emscripten.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace HelloImGui
{
namespace
{
    // The hidden field and the keyboard button, created by the page. The field is placed at the active widget's
    // line (ImGui's coordinates, scaled to the canvas's CSS size), so that the browser scrolls to the right place.
    // The focus happens inside touch handlers only (a focus from a frame shows no keyboard on iOS).
    // The field's keys never reach the backends (stopPropagation at the field, which is outside the canvas).
    const char* kScript = R"JS(
if (!window.helloImGuiKeyboard) {
  const K = window.helloImGuiKeyboard = {want: 0, x: 0, y: 0, h: 0, displayW: 0, typed: '', keys: []};
  const input = document.createElement('input');
  input.id = 'helloImGuiTextInput'; input.type = 'text'; input.autocapitalize = 'off'; input.autocomplete = 'off';
  input.setAttribute('autocorrect', 'off'); input.spellcheck = false;
  input.style.cssText = 'position:fixed;left:0;top:0;width:2px;height:2px;opacity:0.01;border:0;padding:0;font-size:16px;z-index:1000;';
  const button = document.createElement('button');
  button.id = 'helloImGuiKeyboardButton'; button.textContent = '⌨'; button.title = 'Keyboard';
  button.style.cssText = 'position:fixed;display:none;z-index:1001;font-size:24px;line-height:1;padding:6px 12px;border-radius:10px;border:1px solid #999;background:#333;color:#eee;';
  document.body.appendChild(input); document.body.appendChild(button);
  const canvasRect = () => { const c = document.getElementById('canvas'); return c ? c.getBoundingClientRect() : {left: 0, top: 0, width: 0}; };
  const scale = (r) => (K.displayW > 0 && r.width > 0) ? r.width / K.displayW : 1;
  const place = () => {
    const r = canvasRect(), s = scale(r), x = r.left + K.x * s, y = r.top + K.y * s, h = K.h * s;
    input.style.left = x + 'px'; input.style.top = (y + h) + 'px';
    button.style.left = x + 'px'; button.style.top = (y + h + 8) + 'px'; };
  const refresh = () => {
    const focused = document.activeElement === input;
    place();
    button.style.display = (K.want && !focused && window.helloImGuiPointerType === 1) ? 'block' : 'none';
    if (!K.want && focused) input.blur(); };
  K.set = (want, x, y, h, displayW) => { K.want = want; K.x = x; K.y = y; K.h = h; K.displayW = displayW; refresh(); };
  const focusNow = (e) => { e.preventDefault(); input.focus(); refresh(); };
  button.addEventListener('touchend', focusNow);
  button.addEventListener('mouseup', focusNow);
  button.addEventListener('click', (e) => e.preventDefault());
  // The second tap on the active widget's line (ImGui gives the line, not the widget's width)
  document.addEventListener('touchend', (e) => {
    if (!K.want || document.activeElement === input || e.target === button || !e.changedTouches.length) return;
    const r = canvasRect(), s = scale(r), top = r.top + K.y * s, h = K.h * s, y = e.changedTouches[0].clientY;
    if (y >= top - h && y <= top + 2 * h) { input.focus(); refresh(); }
  }, {capture: true});
  const special = ['Backspace', 'Enter', 'Tab', 'Escape', 'ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown', 'Delete', 'Home', 'End'];
  input.addEventListener('input', () => { K.typed += input.value; input.value = ''; });
  input.addEventListener('keydown', (e) => { if (special.includes(e.key)) { K.keys.push(e.key); e.preventDefault(); } e.stopPropagation(); });
  input.addEventListener('keyup', (e) => e.stopPropagation());
  input.addEventListener('keypress', (e) => e.stopPropagation());
  input.addEventListener('focus', refresh);
  input.addEventListener('blur', refresh);
  K.drainTyped = () => { const t = K.typed; K.typed = ''; return t; };
  K.drainKeys = () => { const k = K.keys.join(','); K.keys = []; return k; };
}
)JS";

    ImGuiKey KeyFromName(const std::string& name)
    {
        static const struct { const char* name; ImGuiKey key; } kKeys[] = {
            {"Backspace", ImGuiKey_Backspace}, {"Enter", ImGuiKey_Enter}, {"Tab", ImGuiKey_Tab},
            {"Escape", ImGuiKey_Escape}, {"ArrowLeft", ImGuiKey_LeftArrow}, {"ArrowRight", ImGuiKey_RightArrow},
            {"ArrowUp", ImGuiKey_UpArrow}, {"ArrowDown", ImGuiKey_DownArrow}, {"Delete", ImGuiKey_Delete},
            {"Home", ImGuiKey_Home}, {"End", ImGuiKey_End},
        };
        for (const auto& k : kKeys)
            if (name == k.name)
                return k.key;
        return ImGuiKey_None;
    }

    struct State
    {
        bool want = false;
        ImVec2 pos;
        float lineHeight = 0.f;
        float displayW = 0.f;
    };
    State gLast;
}  // namespace

void InstallEmscriptenKeyboard()
{
    emscripten_run_script(kScript);
}

void UpdateEmscriptenKeyboard()
{
    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;

    // What ImGui wants, when it changed (the last complete frame's IME data: the cursor's line)
    const ImGuiPlatformImeData& ime = g.PlatformImeDataPrev;
    State now;
    now.want = io.WantTextInput;
    now.pos = ime.InputPos;
    now.lineHeight = ime.InputLineHeight;
    now.displayW = io.DisplaySize.x;
    if (now.want != gLast.want || now.pos.x != gLast.pos.x || now.pos.y != gLast.pos.y
        || now.lineHeight != gLast.lineHeight || now.displayW != gLast.displayW)
    {
        gLast = now;
        char script[256];
        snprintf(script, sizeof(script), "window.helloImGuiKeyboard && window.helloImGuiKeyboard.set(%d, %.1f, %.1f, %.1f, %.1f)",
                 now.want ? 1 : 0, now.pos.x, now.pos.y, now.lineHeight, now.displayW);
        emscripten_run_script(script);
    }

    // What was typed in the field
    if (!now.want)
        return;
    const char* typed = emscripten_run_script_string("window.helloImGuiKeyboard ? window.helloImGuiKeyboard.drainTyped() : ''");
    if (typed != nullptr && *typed != '\0')
        io.AddInputCharactersUTF8(typed);
    const char* keys = emscripten_run_script_string("window.helloImGuiKeyboard ? window.helloImGuiKeyboard.drainKeys() : ''");
    std::string names = keys ? keys : "";
    size_t start = 0;
    while (start < names.size())
    {
        size_t end = names.find(',', start);
        if (end == std::string::npos)
            end = names.size();
        ImGuiKey key = KeyFromName(names.substr(start, end - start));
        if (key != ImGuiKey_None)
        {
            io.AddKeyEvent(key, true);
            io.AddKeyEvent(key, false);
        }
        start = end + 1;
    }
}

}  // namespace HelloImGui
#endif
