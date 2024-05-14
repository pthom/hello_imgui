// From https://github.com/leiradel/ImGuiAl

#include "imguial_term.h"
#include "hello_imgui/icons_font_awesome_4.h"
#include <algorithm>
#include <ctype.h>
#include <stdio.h>

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

ImGuiAl::Fifo::Fifo(void* const buffer, size_t const size)
    : _buffer(buffer)
    , _size(size)
    , _available(size)
    , _first(0)
    , _last(0) {}

void ImGuiAl::Fifo::reset() {
    _available = _size;
    _first = _last = 0;
}

void ImGuiAl::Fifo::read(void* const data, size_t const count) {
    size_t first_batch = count;
    size_t second_batch = 0;

    if (first_batch > _size - _first) {
        first_batch = _size - _first;
        second_batch = count - first_batch;
    }

    uint8_t const* const src = static_cast<uint8_t const*>(_buffer) + _first;
    memcpy(data, src, first_batch);
    memcpy(static_cast<uint8_t*>(data) + first_batch, _buffer, second_batch);

    _first = (_first + count) % _size;
    _available += count;
}

void ImGuiAl::Fifo::peek(size_t const pos, void* const data, size_t const count) const {
    size_t const position = (pos + _first) % _size;
    size_t first_batch = count;
    size_t second_batch = 0;

    if (first_batch > _size - position) {
        first_batch = _size - position;
        second_batch = count - first_batch;
    }

    uint8_t const* const src = static_cast<uint8_t const*>(_buffer) + position;
    memcpy(data, src, first_batch);
    memcpy(static_cast<uint8_t*>(data) + first_batch, _buffer, second_batch);
}

void ImGuiAl::Fifo::write(void const* const data, size_t const count) {
    size_t first_batch = count;
    size_t second_batch = 0;

    if (first_batch > _size - _last) {
        first_batch = _size - _last;
        second_batch = count - first_batch;
    }

    uint8_t* const dest = static_cast<uint8_t*>(_buffer) + _last;
    memcpy(dest, data, first_batch);
    memcpy(_buffer, static_cast<uint8_t const*>(data) + first_batch, second_batch);

    _last = (_last + count) % _size;
    _available -= count;
}

void ImGuiAl::Fifo::skip(size_t const count) {
    _first = (_first + count) % _size;
    _available += count;
}

ImGuiAl::Crt::Crt(void* const buffer, size_t const size)
    : _fifo(buffer, size)
    , _foregroundColor(CGA::White)
    , _metaData(0)
    , _scrollToBottom(false) {}

void ImGuiAl::Crt::setForegroundColor(ImU32 const color) {
    _foregroundColor = color;
}

void ImGuiAl::Crt::setMetaData(unsigned const meta_data) {
    _metaData = meta_data;
}

void ImGuiAl::Crt::printf(char const* const format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void ImGuiAl::Crt::vprintf(char const* const format, va_list args) {
    char temp[256];
    char* line = temp;

    va_list args_copy;
    va_copy(args_copy, args);

    // TODO does vsnprintf ever return a negative value?
    size_t const needed = ::vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);

    size_t const length = std::min(needed, _fifo.size() - sizeof(Info));

    if (length >= sizeof(temp)) {
        line = new char[length + 1];
        ::vsnprintf(line, length, format, args_copy);
    }

    va_end(args_copy);

    while (length + sizeof(Info) > _fifo.available()) {
        Info header;
        _fifo.read(&header, sizeof(header));
        _fifo.skip(header.length);
    }

    Info header;
    header.foregroundColor = _foregroundColor;
    header.length = static_cast<unsigned>(length);
    header.metaData = _metaData;

    _fifo.write(&header, sizeof(header));
    _fifo.write(line, length);

    if (line != temp) {
        delete[] line;
    }

    if (autoScrollToBotttom)
        scrollToBottom();
}

void ImGuiAl::Crt::scrollToBottom() {
    _scrollToBottom = true;
}

void ImGuiAl::Crt::clear() {
    _fifo.reset();
}

void ImGuiAl::Crt::iterate(const std::function<bool(Info const& header, char const* const line)>& iterator) const {
    size_t const occupied = _fifo.occupied();
    size_t pos = 0;

    char temp[256];
    char* line = temp;
    size_t max_length = sizeof(temp);

    while (pos < occupied) {
        Info header;
        _fifo.peek(pos, &header, sizeof(header));
        pos += sizeof(header);

        if (header.length >= max_length) {
            max_length = header.length + 1;
            char* new_line = static_cast<char*>(line == temp ? malloc(max_length) : realloc(line, max_length));

            if (new_line == nullptr) {
                if (line != temp) {
                    free(line);
                }

                break;
            }

            line = new_line;
        }

        _fifo.peek(pos, line, header.length);
        pos += header.length;
        line[header.length] = 0;

        if (!iterator(header, line)) {
            break;
        }
    }

    if (line != temp) {
        free(line);
    }
}

void ImGuiAl::Crt::draw(ImVec2 const& size) {
    draw(size, [](Info const& header, char const* const line) -> bool {
        (void)header;
        (void)line;
        return true;
    });
}

void ImGuiAl::Crt::draw(ImVec2 const& size, const std::function<bool(Info const& header, char const* const line)>& filter) {
    char id[64];
    snprintf(id, sizeof(id), "ImGuiAl::Crt@%p", (void *)this);

    ImGui::BeginChild(id, size, false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 1.0f));

    iterate([&filter](Info const& header, char const* const line) -> bool {
        if (filter(header, line)) {
            ImGui::PushStyleColor(ImGuiCol_Text, header.foregroundColor);
            ImGui::TextUnformatted(line);
            ImGui::PopStyleColor();
        }

        return true;
    });

    if (_scrollToBottom) {
        ImGui::SetScrollHereY();
        _scrollToBottom = false;
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();
}

static ImU32 changeValue(ImU32 const color, float const delta_value) {
    ImVec4 rgba = ImGui::ColorConvertU32ToFloat4(color);

    float h, s, v;
    ImGui::ColorConvertRGBtoHSV(rgba.x, rgba.y, rgba.z, h, s, v);
    v += delta_value;

    if (v < 0.0f) {
        v = 0.0f;
    }
    else if (v > 1.0f) {
        v = 1.0f;
    }

    ImGui::ColorConvertHSVtoRGB(h, s, v, rgba.x, rgba.y, rgba.z);
    return ImGui::ColorConvertFloat4ToU32(rgba);
}

ImGuiAl::Log::Log(void* const buffer, size_t const buffer_size)
    : Crt(buffer, buffer_size)
    , _debugLabel("Debug")
    , _infoLabel(ICON_FA_INFO " Info")
    , _warningLabel(ICON_FA_EXCLAMATION " Warning")
    , _errorLabel(ICON_FA_BOMB " Error")
    , _cumulativeLabel("Cumulative")
    , _filterLabel(ICON_FA_FILTER " Filter (inc,-exc)")
    , _filterHeaderLabel(nullptr)
    , _showFilters(true)
    , _actions(nullptr)
    , _level(Level::Debug)
    , _cumulative(true) {

    setColor(Level::Debug, CGA::BrightBlue);
    setColor(Level::Info, CGA::BrightGreen);
    setColor(Level::Warning, CGA::Yellow);
    setColor(Level::Error, CGA::BrightRed);
    autoScrollToBotttom = true;
}

void ImGuiAl::Log::debug(char const* const format, ...) {
    va_list args;
    va_start(args, format);
    debug(format, args);
    va_end(args);
}

void ImGuiAl::Log::info(char const* const format, ...) {
    va_list args;
    va_start(args, format);
    info(format, args);
    va_end(args);
}

void ImGuiAl::Log::warning(char const* const format, ...) {
    va_list args;
    va_start(args, format);
    warning(format, args);
    va_end(args);
}

void ImGuiAl::Log::error(char const* const format, ...) {
    va_list args;
    va_start(args, format);
    error(format, args);
    va_end(args);
}

void ImGuiAl::Log::debug(char const* const format, va_list args) {
    setForegroundColor(_debugTextColor);
    setMetaData(static_cast<unsigned>(Level::Debug));
    vprintf(format, args);
}

void ImGuiAl::Log::info(char const* const format, va_list args) {
    setForegroundColor(_infoTextColor);
    setMetaData(static_cast<unsigned>(Level::Info));
    vprintf(format, args);
}

void ImGuiAl::Log::warning(char const* const format, va_list args) {
    setForegroundColor(_warningTextColor);
    setMetaData(static_cast<unsigned>(Level::Warning));
    vprintf(format, args);
}

void ImGuiAl::Log::error(char const* const format, va_list args) {
    setForegroundColor(_errorTextColor);
    setMetaData(static_cast<unsigned>(Level::Error));
    vprintf(format, args);
}

// Utilities to find colors that contrast well with the background (WindowBg color)
namespace AutomatiColors
{
    enum class LogColorClass { Debug_Blue, Info_Green, Warning_Yellow, Error_Red };

    enum class LogColorState { Button, ButtonHovered, Text };

    float logColorClassHue(LogColorClass c, LogColorState state)
    {
        constexpr float hBlue = 130.f / 255.f, hGreen = 94.f / 255.f,
                        hRed = 0.f, hYellow = 42.f / 255.f;
        constexpr float hBlueHovered = 117.f / 255.f, hGreenHovered = 56.f / 255.f,
                        hRedHovered = 228.f / 255.f, hYellowHovered = 36.f / 255.f;
        if (state == LogColorState::ButtonHovered)
        {
            if (c == LogColorClass::Info_Green)
                return hGreenHovered;
            else if (c == LogColorClass::Warning_Yellow)
                return hYellowHovered;
            else if (c == LogColorClass::Error_Red)
                return hRedHovered;
            else
                return hBlueHovered;
        }
        else
        {
            if (c == LogColorClass::Info_Green)
                return hGreen;
            else if (c == LogColorClass::Warning_Yellow)
                return hYellow;
            else if (c == LogColorClass::Error_Red)
                return hRed;
            else
                return hBlue;
        }
    }

    ImU32 makeColor(LogColorClass logColorClass, LogColorState logColorState, float windowBgHsv_Value)
    {
        float wantedHue = logColorClassHue(logColorClass, logColorState);

        float adaptedValue = 1.f;
        if (windowBgHsv_Value < 0.8f)
            adaptedValue = 0.9f;
        else
            adaptedValue = 0.6f;

        float resultHsv[3] = { wantedHue, 1.f, adaptedValue };

        float resultRgb[3];
        ImGui::ColorConvertHSVtoRGB(resultHsv[0], resultHsv[1], resultHsv[2], resultRgb[0], resultRgb[1], resultRgb[2]);

        ImVec4 r { resultRgb[0], resultRgb[1], resultRgb[2], 1.f };
        return ImGui::GetColorU32(r);
    }
}

void ImGuiAl::Log::setColorsAutoFromWindowBg()
{
    ImVec4 windowBgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    float windowBgHsv[3];
    ImGui::ColorConvertRGBtoHSV(windowBgColor.x, windowBgColor.y, windowBgColor.z, windowBgHsv[0], windowBgHsv[1], windowBgHsv[2]);
    float windowBgHsv_Value = windowBgHsv[2];

    using namespace AutomatiColors;

    _debugButtonColor = makeColor(LogColorClass::Debug_Blue, LogColorState::Button, windowBgHsv_Value);
    _debugButtonHoveredColor = makeColor(LogColorClass::Debug_Blue, LogColorState::ButtonHovered, windowBgHsv_Value);
    _debugTextColor = makeColor(LogColorClass::Debug_Blue, LogColorState::Text, windowBgHsv_Value);

    _infoButtonColor = makeColor(LogColorClass::Info_Green, LogColorState::Button, windowBgHsv_Value);
    _infoButtonHoveredColor = makeColor(LogColorClass::Info_Green, LogColorState::ButtonHovered, windowBgHsv_Value);
    _infoTextColor = makeColor(LogColorClass::Info_Green, LogColorState::Text, windowBgHsv_Value);

    _warningButtonColor = makeColor(LogColorClass::Warning_Yellow, LogColorState::Button, windowBgHsv_Value);
    _warningButtonHoveredColor = makeColor(LogColorClass::Warning_Yellow, LogColorState::ButtonHovered, windowBgHsv_Value);
    _warningTextColor = makeColor(LogColorClass::Warning_Yellow, LogColorState::Text, windowBgHsv_Value);

    _errorButtonColor = makeColor(LogColorClass::Error_Red, LogColorState::Button, windowBgHsv_Value);
    _errorButtonHoveredColor = makeColor(LogColorClass::Error_Red, LogColorState::ButtonHovered, windowBgHsv_Value);
    _errorTextColor = makeColor(LogColorClass::Error_Red, LogColorState::Text, windowBgHsv_Value);
}


int ImGuiAl::Log::draw(ImVec2 const& size)
{
    if (_useAutomaticColors)
        setColorsAutoFromWindowBg();

    int action = 0;

    for (unsigned i = 0; _actions != nullptr && _actions[i] != nullptr; i++) {
        if (i != 0) {
            ImGui::SameLine();
        }

        if (ImGui::Button(_actions[i])) {
            action = i + 1;
        }
    }


    if ((_filterHeaderLabel != nullptr && ImGui::CollapsingHeader(_filterHeaderLabel)) || _showFilters) {

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.f));

        ImGui::PushStyleColor(ImGuiCol_Button, _debugButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, _debugButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, _debugTextColor);
        bool ok = ImGui::Button(_debugLabel);
        ImGui::PopStyleColor(3);

        if (ok) {
            _level = Level::Debug;
            scrollToBottom();
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, _infoButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, _infoButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, _infoTextColor);
        ok = ImGui::Button(_infoLabel);
        ImGui::PopStyleColor(3);

        if (ok) {
            _level = Level::Info;
            scrollToBottom();
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, _warningButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, _warningButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, _warningTextColor);
        ok = ImGui::Button(_warningLabel);
        ImGui::PopStyleColor(3);

        if (ok) {
            _level = Level::Warning;
            scrollToBottom();
        }

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, _errorButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, _errorButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, _errorTextColor);
        ok = ImGui::Button(_errorLabel);
        ImGui::PopStyleColor(3);

        if (ok) {
            _level = Level::Error;
            scrollToBottom();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::Checkbox(_cumulativeLabel, &_cumulative);

        ImGui::SameLine();
        if (ImGui::Button("Clear##LogsClear"))
            clear();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 15.f);
        _filter.Draw(_filterLabel);
    }

    Crt::draw(size, [this](Info const& header, char const* const line) -> bool {
        unsigned const level = static_cast<unsigned>(_level);

        bool show = (_cumulative && header.metaData >= level) || header.metaData == level;
        show = show && _filter.PassFilter(line);

        return show;
    });


    return action;
}

void ImGuiAl::Log::setColor(Level const level, ImU32 const color) {
    ImU32 const button_color = changeValue(color, -0.2f);
    ImU32 const hovered_color = changeValue(color, -0.1f);

    switch (level) {
        case Level::Debug:
            _debugTextColor = color;
            _debugButtonColor = button_color;
            _debugButtonHoveredColor = hovered_color;
            break;

        case Level::Info:
            _infoTextColor = color;
            _infoButtonColor = button_color;
            _infoButtonHoveredColor = hovered_color;
            break;

        case Level::Warning:
            _warningTextColor = color;
            _warningButtonColor = button_color;
            _warningButtonHoveredColor = hovered_color;
            break;

        case Level::Error:
            _errorTextColor = color;
            _errorButtonColor = button_color;
            _errorButtonHoveredColor = hovered_color;
            break;
    }
}

void ImGuiAl::Log::setLabel(Level const level, char const* const label) {
    switch (level) {
        case Level::Debug:
            _debugLabel = label;
            break;

        case Level::Info:
            _infoLabel = label;
            break;

        case Level::Warning:
            _warningLabel = label;
            break;

        case Level::Error:
            _errorLabel = label;
            break;
    }
}

void ImGuiAl::Log::setCumulativeLabel(char const* const label) {
    _cumulativeLabel = label;
}

void ImGuiAl::Log::setFilterLabel(char const* const label) {
    _filterLabel = label;
}

void ImGuiAl::Log::setFilterHeaderLabel(char const* const label) {
    _filterHeaderLabel = label;
}

void ImGuiAl::Log::setActions(char const* actions[]) {
    _actions = actions;
}

ImGuiAl::Terminal::Terminal(void* const buffer,
                            size_t const buffer_size,
                            void* const cmd_buf,
                            size_t const cmd_size,
                            std::function<void(Terminal& self, char* const command)>&& execute)
    : Crt(buffer, buffer_size)
    , _commandBuffer(static_cast<char*>(cmd_buf))
    , _cmdBufferSize(cmd_size)
    , _execute(std::move(execute)) {}

void ImGuiAl::Terminal::printf(char const* const format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void ImGuiAl::Terminal::draw(ImVec2 const& size) {
    ImVec2 new_size(size.x, size.y - ImGui::GetTextLineHeight() - ImGui::GetTextLineHeightWithSpacing());
    Crt::draw(new_size);
    ImGui::Separator();

    ImGuiInputTextFlags const flags = ImGuiInputTextFlags_EnterReturnsTrue;

    bool reclaim_focus = false;

    if (ImGui::InputText("Command", _commandBuffer, _cmdBufferSize, flags, nullptr, nullptr)) {
        char* begin = _commandBuffer;

        while (*begin != 0 && isspace(*begin)) {
            begin++;
        }

        if (*begin != 0) {
            char* end = begin + strlen(begin) - 1;

            while (isspace(*end)) {
                end--;
            }

            end[1] = 0;
            _execute(*this, begin);
        }

        reclaim_focus = true;
    }

    ImGui::SetItemDefaultFocus();

    if (reclaim_focus) {
        ImGui::SetKeyboardFocusHere(-1);
    }
}
