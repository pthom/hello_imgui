// From https://github.com/leiradel/ImGuiAl
/*
The MIT License (MIT)

Copyright (c) 2019 Andre Leiradella

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <imgui.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#include <functional>

namespace ImGuiAl {
class Fifo {
   public:
    Fifo(void* const buffer, size_t const size);

    void reset();
    size_t size() const { return _size; }
    size_t occupied() const { return _size - _available; }
    size_t available() const { return _available; }

    void read(void* const data, size_t const count);
    void peek(size_t const pos, void* const data, size_t const count) const;
    void write(void const* const data, size_t const count);
    void skip(size_t const count);

   protected:
    void* _buffer;
    size_t const _size;
    size_t _available;
    size_t _first;
    size_t _last;
};

class Crt {
   public:
    struct CGA {
        enum : ImU32 {
            Black         = IM_COL32(0x00, 0x00, 0x00, 0xff),
            Blue          = IM_COL32(0x00, 0x00, 0xaa, 0xff),
            Green         = IM_COL32(0x00, 0xaa, 0x00, 0xff),
            Cyan          = IM_COL32(0x00, 0xaa, 0xaa, 0xff),
            Red           = IM_COL32(0xaa, 0x00, 0x00, 0xff),
            Magenta       = IM_COL32(0xaa, 0x00, 0xaa, 0xff),
            Brown         = IM_COL32(0xaa, 0x55, 0x00, 0xff),
            White         = IM_COL32(0xaa, 0xaa, 0xaa, 0xff),
            Gray          = IM_COL32(0x55, 0x55, 0x55, 0xff),
            BrightBlue    = IM_COL32(0x55, 0x55, 0xff, 0xff),
            BrightGreen   = IM_COL32(0x55, 0xff, 0x55, 0xff),
            BrightCyan    = IM_COL32(0x55, 0xff, 0xff, 0xff),
            BrightRed     = IM_COL32(0xff, 0x55, 0x55, 0xff),
            BrightMagenta = IM_COL32(0xff, 0x55, 0xff, 0xff),
            Yellow        = IM_COL32(0xff, 0xff, 0x55, 0xff),
            BrightWhite   = IM_COL32(0xff, 0xff, 0xff, 0xff)
        };
    };

    struct Info {
        ImU32 foregroundColor;
        unsigned length;
        unsigned metaData;
    };

    Crt(void* const buffer, size_t const size);

    void setForegroundColor(ImU32 const color);
    void setMetaData(unsigned const meta_data);

    void printf(char const* const format, ...);
    void vprintf(char const* const format, va_list args);
    void scrollToBottom();
    void clear();

    void iterate(const std::function<bool(Info const& header, char const* const line)>& iterator) const;
    void draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

   protected:
    void draw(ImVec2 const& size, const std::function<bool(Info const& header, char const* const line)>& filter);

    Fifo _fifo;
    ImU32 _foregroundColor;
    unsigned _metaData;
    bool _scrollToBottom;
    bool autoScrollToBotttom = false;
};

class Log : protected Crt {
   public:
    typedef Crt::Info Info;

    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

    Log(void* const buffer, size_t const buffer_size);

    void debug(char const* const format, ...);
    void info(char const* const format, ...);
    void warning(char const* const format, ...);
    void error(char const* const format, ...);

    void debug(char const* const format, va_list args);
    void info(char const* const format, va_list args);
    void warning(char const* const format, va_list args);
    void error(char const* const format, va_list args);

    void clear() { Crt::clear(); }
    void iterate(const std::function<bool(Info const& header, char const* const line)>& iterator) const { Crt::iterate(iterator); }
    void scrollToBottom() { Crt::scrollToBottom(); }

    int draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

    void setColor(Level const level, ImU32 const color);
    void setLabel(Level const level, char const* const label);
    void setCumulativeLabel(char const* const label);
    void setFilterLabel(char const* const label);
    void setFilterHeaderLabel(char const* const label);
    void setActions(char const* actions[]);
    void setColorsAutoFromWindowBg();

   protected:
    ImU32 _debugTextColor;
    ImU32 _debugButtonColor;
    ImU32 _debugButtonHoveredColor;

    ImU32 _infoTextColor;
    ImU32 _infoButtonColor;
    ImU32 _infoButtonHoveredColor;

    ImU32 _warningTextColor;
    ImU32 _warningButtonColor;
    ImU32 _warningButtonHoveredColor;

    ImU32 _errorTextColor;
    ImU32 _errorButtonColor;
    ImU32 _errorButtonHoveredColor;
    bool  _useAutomaticColors = true;

    char const* _debugLabel;
    char const* _infoLabel;
    char const* _warningLabel;
    char const* _errorLabel;

    char const* _cumulativeLabel;
    char const* _filterLabel;
    char const* _filterHeaderLabel;

    bool _showFilters;
    char const* const* _actions;

    Level _level;
    bool _cumulative;
    ImGuiTextFilter _filter;
};

class Terminal : protected Crt {
   public:
    typedef Crt::Info Info;

    Terminal(void* const buffer,
             size_t const buffer_size,
             void* const cmd_buf,
             size_t const cmd_size,
             std::function<void(Terminal& self, char* const command)>&& execute);

    void setForegroundColor(ImU32 const color) { Crt::setForegroundColor(color); }

    void printf(char const* const format, ...);
    void vprintf(char const* const format, va_list args) { Crt::vprintf(format, args); }

    void clear() { Crt::clear(); }
    void iterate(const std::function<bool(Info const& header, char const* const line)>& iterator) const { Crt::iterate(iterator); }
    void scrollToBottom() { Crt::scrollToBottom(); }

    void draw(ImVec2 const& size = ImVec2(0.0f, 0.0f));

   protected:
    char* _commandBuffer;
    size_t _cmdBufferSize;
    std::function<void(Terminal& self, char* const command)> _execute;
};

template<size_t S>
class BufferedCrt : public Crt {
   public:
    BufferedCrt() : Crt(_buffer, S) {}

   protected:
    uint8_t _buffer[S];
};

template<size_t S>
class BufferedLog : public Log {
   public:
    BufferedLog() : Log(_buffer, S) {}

   protected:
    uint8_t _buffer[S];
};

template<size_t S, size_t R>
class BufferedTerminal : public Terminal {
   public:
    BufferedTerminal(std::function<void(Terminal& self, char* const command)>&& execute)
        : Terminal(_buffer, S, _commandBuffer, R, std::move(execute)) {}

   protected:
    uint8_t _buffer[S];
    uint8_t _commandBuffer[R];
};
}
