#include "panel.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include "logger_lib/file_logger.h"

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {

#ifndef _WIN32
struct termios g_savedTerm;
bool g_termSaved = false;

auto restoreTerminal() -> void {
    if (g_termSaved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_savedTerm);
        g_termSaved = false;
    }
}
#endif

// the characters a line shows: escape sequences "\033[...m" take no room
auto visible(const std::string& s) -> size_t {
    size_t n = 0;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\033') {
            while (i < s.size() && s[i] != 'm')
                i++;
            continue;
        }
        n++;
    }
    return n;
}

auto cut(const std::string& s, size_t width) -> std::string {
    std::string out;
    size_t n = 0;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\033') {
            size_t j = i;
            while (j < s.size() && s[j] != 'm')
                j++;
            out += s.substr(i, j - i + 1);
            i = j;
            continue;
        }
        if (n >= width)
            continue;  // the escape codes after the cut still close the colours
        out += s[i];
        n++;
    }
    return out;
}

}  // namespace

Panel::Panel(const ClientOpt::Options& option) {
    if (option.quiet || option.verbous)
        return;
#ifdef _WIN32
    if (!_isatty(_fileno(stdout)))
        return;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (!GetConsoleMode(out, &mode) || !SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
        return;  // an old console: no panel
    m_enabled = true;
    m_keys = true;
#else
    if (!isatty(STDOUT_FILENO))
        return;
    m_enabled = true;
    if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &g_savedTerm) == 0) {
        struct termios raw = g_savedTerm;
        raw.c_lflag &= ~(ICANON | ECHO);  // a key at a time, not echoed; Ctrl+C still works
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
            g_termSaved = true;
            m_keys = true;
            atexit(restoreTerminal);
        }
    }
#endif
    setConsoleHooks([this]() { eraseLocked(); }, nullptr);
}

Panel::~Panel() {
    close();
}

auto Panel::close() -> void {
    if (!m_enabled)
        return;
    setConsoleHooks(nullptr, nullptr);
    consoleLocked([this]() { eraseLocked(); });
#ifndef _WIN32
    restoreTerminal();
#endif
    m_enabled = false;
    m_keys = false;
}

auto Panel::eraseLocked() -> void {
    if (m_drawn > 0) {
        fprintf(stdout, "\033[%dA\r\033[J", m_drawn);  // up to the first line, clear to the end
        fflush(stdout);
        m_drawn = 0;
    }
}

auto Panel::width() const -> int {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return info.srWindow.Right - info.srWindow.Left + 1;
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
#endif
    const char* c = getenv("COLUMNS");
    return c ? atoi(c) : 80;
}

auto Panel::draw(const std::vector<std::string>& lines) -> void {
    if (!m_enabled)
        return;
    // one column less than the terminal: a full line would wrap and break the count
    const size_t w = static_cast<size_t>(std::max(20, width() - 1));
    consoleLocked([&]() {
        eraseLocked();
        for (auto& l : lines)
            fprintf(stdout, "%s\033[0m\n", cut(l, w).c_str());
        fflush(stdout);
        m_drawn = static_cast<int>(lines.size());
    });
}

auto Panel::key() -> int {
    if (!m_keys)
        return 0;
#ifdef _WIN32
    return _kbhit() ? _getch() : 0;
#else
    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    struct timeval tv = {0, 0};
    if (select(STDIN_FILENO + 1, &set, nullptr, nullptr, &tv) <= 0)
        return 0;
    char c = 0;
    return read(STDIN_FILENO, &c, 1) == 1 ? c : 0;
#endif
}

auto Panel::green(const std::string& s) -> std::string {
    return "\033[32m" + s + "\033[0m";
}

auto Panel::yellow(const std::string& s) -> std::string {
    return "\033[33m" + s + "\033[0m";
}

auto Panel::red(const std::string& s) -> std::string {
    return "\033[1;31m" + s + "\033[0m";
}

auto Panel::bold(const std::string& s) -> std::string {
    return "\033[1m" + s + "\033[0m";
}

auto Panel::dim(const std::string& s) -> std::string {
    return "\033[2m" + s + "\033[0m";
}

auto Panel::bar(double fraction, int width) -> std::string {
    if (fraction < 0)
        fraction = 0;
    if (fraction > 1)
        fraction = 1;
    const int full = static_cast<int>(fraction * width + 0.5);
    return "[" + std::string(full, '#') + std::string(width - full, '-') + "]";
}

auto Panel::left(const std::string& s, size_t n) -> std::string {
    const size_t v = visible(s);
    return v >= n ? s : s + std::string(n - v, ' ');
}

auto Panel::right(const std::string& s, size_t n) -> std::string {
    const size_t v = visible(s);
    return v >= n ? s : std::string(n - v, ' ') + s;
}
