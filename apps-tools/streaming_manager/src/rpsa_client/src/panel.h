#ifndef RPSA_CLIENT_PANEL_H
#define RPSA_CLIENT_PANEL_H

#include <string>
#include <vector>

#include "options.h"

// A block of status lines redrawn in place on a terminal (no scrolling). Text printed with
// aprintf/acprintf meanwhile goes above the block: the block clears itself first and is drawn
// again below. Off without a terminal, with --quiet or -v: then nothing is drawn.
class Panel {
   public:
    explicit Panel(const ClientOpt::Options& option);
    ~Panel();

    auto enabled() const -> bool { return m_enabled; }
    // the lines, cut to the width of the terminal (escape codes do not count)
    auto draw(const std::vector<std::string>& lines) -> void;
    // the block cleared, the hooks off: what follows prints below as usual
    auto close() -> void;
    auto width() const -> int;
    // a key pressed since the last call ('q'), 0 for none; keys are read only with the panel on
    auto key() -> int;
    auto keys() const -> bool { return m_keys; }

    // colours for the cells
    static auto green(const std::string& s) -> std::string;
    static auto red(const std::string& s) -> std::string;
    static auto bold(const std::string& s) -> std::string;
    static auto dim(const std::string& s) -> std::string;
    // "[####------]"
    static auto bar(double fraction, int width) -> std::string;
    // s padded with spaces to n visible characters (left aligned) or right aligned
    static auto left(const std::string& s, size_t n) -> std::string;
    static auto right(const std::string& s, size_t n) -> std::string;

   private:
    auto eraseLocked() -> void;

    bool m_enabled = false;
    bool m_keys = false;
    int m_drawn = 0;  // lines on the screen
};

#endif
