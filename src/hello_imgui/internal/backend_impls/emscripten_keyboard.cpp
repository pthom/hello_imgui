#ifdef __EMSCRIPTEN__
#include "hello_imgui/internal/backend_impls/emscripten_keyboard.h"

#include "imgui.h"
#include "imgui_internal.h"  // g.PlatformImeDataPrev, ImGui::GetInputTextState
#include <emscripten.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace HelloImGui
{
namespace
{
    // The hidden field and the keyboard button, created by the page. The field mirrors the active text widget (its
    // text and its caret), so that the keyboard's own editing (the caret moved from the space bar on iOS, a word
    // replaced by the autocorrection) has something to act on; ImGui stays the source of truth:
    // - what is typed changes the field, the page diffs it against the mirror: so many characters removed at the
    //   caret, this text inserted. C++ sets ImGui's caret, sends Backspace keys, then the characters;
    // - a caret moved without typing (the keyboard's trackpad, a tap in the field) is written to ImGui's caret;
    // - Enter, Tab and Escape are keys, not text (a newline in the field would be text).
    // The field is placed at the active widget's line (ImGui's coordinates, scaled to the canvas's CSS size), so that
    // the browser scrolls to the right place. The focus happens inside touch handlers only (a focus from a frame
    // shows no keyboard on iOS). The field's keys never reach the backends (stopPropagation at the field, which is
    // outside the canvas). The field counts in UTF-16 units, ImGui's caret in bytes: converted here.
    const char* kScript = R"JS(
if (!window.helloImGuiKeyboard) {
  const K = window.helloImGuiKeyboard = {want: 0, x: 0, y: 0, h: 0, displayW: 0, mirror: '', mirrorCaret: 0,
                                         edits: [], caret: -1, keys: []};
  const input = document.createElement('textarea');
  input.id = 'helloImGuiTextInput'; input.autocapitalize = 'off'; input.autocomplete = 'off';
  input.setAttribute('autocorrect', 'on'); input.spellcheck = false; input.rows = 1;
  input.style.cssText = 'position:fixed;left:0;top:0;width:2px;height:2px;opacity:0.01;border:0;padding:0;font-size:16px;z-index:1000;resize:none;';
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
  // The mirror of ImGui's text and caret (UTF-16 units): the field follows, without events of its own
  K.syncing = false;
  K.mirrorTo = (text, caret) => {
    K.syncing = true;
    if (input.value !== text) input.value = text;
    if (input.selectionStart !== caret || input.selectionEnd !== caret) { try { input.setSelectionRange(caret, caret); } catch (e) {} }
    K.mirror = text; K.mirrorCaret = caret;
    K.syncing = false; };
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
  // Typing: the field's new value against the mirror: a removal at the caret, an insertion
  input.addEventListener('input', () => {
    if (K.syncing) return;
    const o = K.mirror, n = input.value;
    let p = 0; while (p < o.length && p < n.length && o[p] === n[p]) p++;
    let s = 0; while (s < o.length - p && s < n.length - p && o[o.length - 1 - s] === n[n.length - 1 - s]) s++;
    K.edits.push({at: p, removed: o.length - p - s, inserted: n.slice(p, n.length - s)});
    K.mirror = n; K.mirrorCaret = input.selectionStart; });
  // The caret moved by itself (the keyboard's trackpad, a tap in the field)
  document.addEventListener('selectionchange', () => {
    if (K.syncing || document.activeElement !== input) return;
    if (input.selectionStart !== K.mirrorCaret && input.selectionStart === input.selectionEnd) {
      K.mirrorCaret = input.selectionStart; K.caret = input.selectionStart; } });
  input.addEventListener('keydown', (e) => {
    if (e.key === 'Enter' || e.key === 'Tab' || e.key === 'Escape') { K.keys.push(e.key); e.preventDefault(); }
    e.stopPropagation(); });
  input.addEventListener('keyup', (e) => e.stopPropagation());
  input.addEventListener('keypress', (e) => e.stopPropagation());
  input.addEventListener('focus', refresh);
  input.addEventListener('blur', refresh);
  // The frame drains: the edits as "at|removed|inserted" lines, the caret, the keys
  K.drainEdits = () => { const e = K.edits.map(x => x.at + '|' + x.removed + '|' + x.inserted).join('\n'); K.edits = []; return e; };
  K.drainCaret = () => { const c = K.caret; K.caret = -1; return c; };
  K.drainKeys = () => { const k = K.keys.join(','); K.keys = []; return k; };
}
)JS";

    ImGuiKey KeyFromName(const std::string& name)
    {
        if (name == "Enter") return ImGuiKey_Enter;
        if (name == "Tab") return ImGuiKey_Tab;
        if (name == "Escape") return ImGuiKey_Escape;
        return ImGuiKey_None;
    }

    // UTF-8 bytes <-> UTF-16 units (what the field counts)
    int Utf16Units(const char* text, int bytes)
    {
        int units = 0;
        for (int i = 0; i < bytes && text[i] != '\0';)
        {
            unsigned char c = (unsigned char)text[i];
            int len = (c < 0x80) ? 1 : (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
            units += (len == 4) ? 2 : 1;
            i += len;
        }
        return units;
    }
    int BytesOfUtf16Units(const char* text, int textLen, int units)
    {
        int i = 0;
        while (i < textLen && units > 0)
        {
            unsigned char c = (unsigned char)text[i];
            int len = (c < 0x80) ? 1 : (c < 0xE0) ? 2 : (c < 0xF0) ? 3 : 4;
            units -= (len == 4) ? 2 : 1;
            i += len;
        }
        return i;
    }

    // A string literal for a script: the quotes and the control characters escaped
    std::string JsString(const char* text, int len)
    {
        std::string s = "'";
        for (int i = 0; i < len; ++i)
        {
            char c = text[i];
            if (c == '\'' || c == '\\') { s += '\\'; s += c; }
            else if (c == '\n') s += "\\n";
            else if (c == '\r') s += "\\r";
            else if (c == '\t') s += "\\t";
            else if ((unsigned char)c < 0x20) continue;
            else s += c;
        }
        return s + "'";
    }

    struct State
    {
        bool want = false;
        ImVec2 pos;
        float lineHeight = 0.f;
        float displayW = 0.f;
        std::string mirrorText;
        int mirrorCaret = -1;
    };
    State gLast;

    void SendKey(ImGuiIO& io, ImGuiKey key)
    {
        io.AddKeyEvent(key, true);
        io.AddKeyEvent(key, false);
    }
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
    State now = gLast;
    now.want = io.WantTextInput;
    now.pos = ime.InputPos;
    now.lineHeight = ime.InputLineHeight;
    now.displayW = io.DisplaySize.x;
    if (now.want != gLast.want || now.pos.x != gLast.pos.x || now.pos.y != gLast.pos.y
        || now.lineHeight != gLast.lineHeight || now.displayW != gLast.displayW)
    {
        char script[256];
        snprintf(script, sizeof(script), "window.helloImGuiKeyboard && window.helloImGuiKeyboard.set(%d, %.1f, %.1f, %.1f, %.1f)",
                 now.want ? 1 : 0, now.pos.x, now.pos.y, now.lineHeight, now.displayW);
        emscripten_run_script(script);
    }
    gLast.want = now.want; gLast.pos = now.pos; gLast.lineHeight = now.lineHeight; gLast.displayW = now.displayW;
    if (!now.want)
    {
        gLast.mirrorText.clear();
        gLast.mirrorCaret = -1;
        return;
    }
    ImGuiInputTextState* state = ImGui::GetInputTextState(g.ActiveId);
    if (state == nullptr)
        return;

    // The edits typed in the field: the caret to the end of the removal, Backspace keys, then the text
    const char* edits = emscripten_run_script_string("window.helloImGuiKeyboard ? window.helloImGuiKeyboard.drainEdits() : ''");
    std::string lines = edits ? edits : "";
    bool edited = false;
    size_t start = 0;
    while (start < lines.size())
    {
        size_t end = lines.find('\n', start);
        if (end == std::string::npos)
            end = lines.size();
        std::string line = lines.substr(start, end - start);
        start = end + 1;
        size_t bar1 = line.find('|'), bar2 = (bar1 == std::string::npos) ? bar1 : line.find('|', bar1 + 1);
        if (bar2 == std::string::npos)
            continue;
        int at = atoi(line.substr(0, bar1).c_str()), removed = atoi(line.substr(bar1 + 1, bar2 - bar1 - 1).c_str());
        std::string inserted = line.substr(bar2 + 1);
        if (removed > 0)
        {
            int cursorBytes = BytesOfUtf16Units(state->GetText(), state->TextLen, at + removed);
            state->SetSelection(cursorBytes, cursorBytes);
            for (int i = 0; i < removed; ++i)
                SendKey(io, ImGuiKey_Backspace);
        }
        else
        {
            int cursorBytes = BytesOfUtf16Units(state->GetText(), state->TextLen, at);
            if (cursorBytes != state->GetCursorPos())
                state->SetSelection(cursorBytes, cursorBytes);
        }
        if (!inserted.empty())
            io.AddInputCharactersUTF8(inserted.c_str());
        edited = true;
    }
    // The caret moved in the field
    int caret = emscripten_run_script_int("window.helloImGuiKeyboard ? window.helloImGuiKeyboard.drainCaret() : -1");
    if (caret >= 0 && !edited)
    {
        int cursorBytes = BytesOfUtf16Units(state->GetText(), state->TextLen, caret);
        state->SetSelection(cursorBytes, cursorBytes);
        state->CursorAnimReset();
        state->CursorFollow = true;
    }
    // The keys
    const char* keys = emscripten_run_script_string("window.helloImGuiKeyboard ? window.helloImGuiKeyboard.drainKeys() : ''");
    std::string names = keys ? keys : "";
    start = 0;
    while (start < names.size())
    {
        size_t end = names.find(',', start);
        if (end == std::string::npos)
            end = names.size();
        ImGuiKey key = KeyFromName(names.substr(start, end - start));
        if (key != ImGuiKey_None)
            SendKey(io, key);
        start = end + 1;
    }

    // The mirror: ImGui's text and caret into the field, when they changed (not while edits are in flight: the
    // keys above take a frame or two, the mirror would show the text before them)
    if (!edited && names.empty())
    {
        std::string text(state->GetText(), (size_t)state->TextLen);
        int caretUnits = Utf16Units(state->GetText(), state->GetCursorPos());
        if (text != gLast.mirrorText || caretUnits != gLast.mirrorCaret)
        {
            gLast.mirrorText = text;
            gLast.mirrorCaret = caretUnits;
            std::string script = "window.helloImGuiKeyboard && window.helloImGuiKeyboard.mirrorTo(" + JsString(text.c_str(), (int)text.size())
                                 + ", " + std::to_string(caretUnits) + ")";
            emscripten_run_script(script.c_str());
        }
    }
}

}  // namespace HelloImGui
#endif
