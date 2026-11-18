#include "records.h"

#include <chrono>
#include <cinttypes>
#include <cmath>
#include <map>
#include <mutex>
#include <vector>

#include "config.h"
#include "logger_lib/file_logger.h"
#include "panel.h"
#include "test_helper.h"

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

auto durationText(double seconds) -> std::string {
    if (!std::isfinite(seconds) || seconds < 0)
        return "-";
    const long s = std::lround(seconds);
    char buf[32];
    if (s < 90)
        snprintf(buf, sizeof(buf), "%ld s", s);
    else if (s < 5400)
        snprintf(buf, sizeof(buf), "%ld min", std::lround(s / 60.0));
    else
        snprintf(buf, sizeof(buf), "%ld h %02ld min", s / 3600, std::lround((s % 3600) / 60.0) % 60);
    return buf;
}

namespace {

using Clock = std::chrono::steady_clock;

// for a column: "45 s", "12 min", "2.5 h"
auto shortDuration(double seconds) -> std::string {
    if (!std::isfinite(seconds) || seconds < 0)
        return "-";
    char buf[32];
    if (seconds < 90)
        snprintf(buf, sizeof(buf), "%.0f s", seconds);
    else if (seconds < 5400)
        snprintf(buf, sizeof(buf), "%.0f min", seconds / 60);
    else
        snprintf(buf, sizeof(buf), "%.1f h", seconds / 3600);
    return buf;
}

// the colours of the panel, kept only on a terminal
auto forOutput(const std::string& s) -> std::string {
    if (isatty(fileno(stdout)))
        return s;
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\x1b' && i + 1 < s.size() && s[i + 1] == '[') {
            i += 2;
            while (i < s.size() && !((s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= 'a' && s[i] <= 'z')))
                i++;
            continue;
        }
        out += s[i];
    }
    return out;
}

// a stream of a board as polled: the status and the sample rate seen between two polls
struct Seen {
    StreamRecordStatus st;
    uint64_t lastSamples = 0;
    Clock::time_point lastTime{};
    double rate = 0;  // samples per channel a second
    uint64_t lastWritten = 0;
    double bytesRate = 0;  // bytes a second that went to the card (the measured speed of the
                           // card overstates one the data do not keep busy)
};

struct Board {
    bool answered = false;
    bool failed = false;  // an empty or broken answer
    uint64_t sdFree = 0;
    uint64_t sdTotal = 0;
    Seen adc;
    Seen gpioIn;
};

auto update(Seen& s, const StreamRecordStatus& st, Clock::time_point now) -> void {
    if (st.samples < s.lastSamples || st.written < s.lastWritten || s.lastTime == Clock::time_point{}) {
        s.rate = s.bytesRate = 0;  // a new run
    } else {
        const double dt = std::chrono::duration<double>(now - s.lastTime).count();
        auto smooth = [](double old, double inst) { return old > 0 ? 0.7 * old + 0.3 * inst : inst; };
        if (dt > 0.2 && st.samples > s.lastSamples)
            s.rate = smooth(s.rate, (st.samples - s.lastSamples) / dt);
        if (dt > 0.2 && st.written > s.lastWritten)
            s.bytesRate = smooth(s.bytesRate, (st.written - s.lastWritten) / dt);
    }
    s.lastSamples = st.samples;
    s.lastWritten = st.written;
    s.lastTime = now;
    s.st = st;
}

auto modeText(uint8_t m) -> const char* {
    return m == StreamRecordStatus::SD ? "sd" : m == StreamRecordStatus::NETWORK ? "network" : "none";
}

auto stateText(const StreamRecordStatus& st) -> std::string {
    if (st.running)
        return st.mode == StreamRecordStatus::SD ? Panel::green("writing") : Panel::green("streaming");
    if (st.mode == StreamRecordStatus::NONE)
        return Panel::dim("idle");
    return st.limit && st.samples >= st.limit ? "done" : "stopped";
}

// what is left: to the limit at the rate seen, else until the card is full at its speed
auto leftText(const Seen& s, uint64_t sdFree) -> std::string {
    const auto& st = s.st;
    if (!st.running || st.mode != StreamRecordStatus::SD)
        return st.limit && st.samples >= st.limit ? "done" : "-";
    if (st.limit)
        return s.rate > 0 ? shortDuration((st.limit - std::min(st.limit, st.samples)) / s.rate) : "-";
    return s.bytesRate > 0 ? "full " + shortDuration(sdFree / s.bytesRate) : "-";
}

// two lines: the stream, then its SD card
auto streamLines(const std::string& host, const char* name, const Seen& s, const Board& b, std::vector<std::string>& lines) -> void {
    const auto& st = s.st;
    char buf[64];
    std::string limit = "-";
    if (st.limit) {
        snprintf(buf, sizeof(buf), "%.0f %%", std::min(1.0, (double)st.samples / st.limit) * 100);
        limit = buf;
    }
    std::string lost = std::to_string(st.lostBoard) + "/" + std::to_string(st.lostQueue);
    if (st.lostBoard || st.lostQueue)
        lost = Panel::red(lost);
    lines.push_back(Panel::left("  " + host, 16) + Panel::left(name, 8) + Panel::left(stateText(st), 10) + Panel::right(std::to_string(st.samples), 12) +
                    Panel::right(limit, 7) + Panel::right(leftText(s, b.sdFree), 12) + Panel::right(lost, 12));
    if (st.mode != StreamRecordStatus::SD)
        return;
    std::string sd = "SD ";
    if (st.running) {
        if (s.bytesRate > 0)
            sd += sizeText(static_cast<uint64_t>(s.bytesRate)) + "/s · ";
        snprintf(buf, sizeof(buf), "queue %.0f %% · ", st.queue * 100);
        sd += buf;
    }
    sd += sizeText(st.written) + " written · " + sizeText(b.sdFree) + " free of " + sizeText(b.sdTotal);
    const bool low = b.sdTotal && b.sdFree < b.sdTotal / 20;
    lines.push_back(Panel::left("", 16) + (low ? Panel::red(sd) : Panel::dim(sd)));
}

auto keyValues(const std::string& host, const char* name, const StreamRecordStatus& st) -> std::string {
    char buf[512];
    snprintf(buf, sizeof(buf),
             "%s %s mode=%s running=%d samples=%" PRIu64 " limit=%" PRIu64 " written=%" PRIu64 " write_rate=%.0f queue=%.3f lost_board=%" PRIu64
             " lost_queue=%" PRIu64 " file=%s\n",
             host.c_str(), name, modeText(st.mode), st.running ? 1 : 0, st.samples, st.limit, st.written, st.writeRate, st.queue, st.lostBoard,
             st.lostQueue, st.file.c_str());
    return buf;
}

}  // namespace

auto watchRecords(std::shared_ptr<ClientNetConfigManager> cl, const std::list<std::string>& hosts, const ClientOpt::Options& option, bool once,
                  const std::function<bool()>& stop, bool reportMissing) -> bool {
    std::mutex m;
    std::map<std::string, Board> boards;
    for (auto& h : hosts)
        boards[h];
    cl->getRecordStatusNofiy.connect([&](std::string& host, RecordStatus st) {
        const std::lock_guard lock(m);
        auto& b = boards[host];
        const auto now = Clock::now();
        b.answered = true;
        b.failed = false;
        b.sdFree = st.sdFree;
        b.sdTotal = st.sdTotal;
        update(b.adc, st.adc, now);
        update(b.gpioIn, st.gpioIn, now);
    });
    cl->failRecordStatusNofiy.connect([&](std::string& host) {
        const std::lock_guard lock(m);
        boards[host].answered = true;
        boards[host].failed = true;
    });
    auto ask = [&]() {
        for (auto& h : hosts)
            cl->requestRecordStatus(h);
    };
    auto allAnswered = [&]() {
        const std::lock_guard lock(m);
        for (auto& [h, b] : boards)
            if (!b.answered)
                return false;
        return true;
    };
    // the first answers: an older server never answers
    ask();
    const auto deadline = Clock::now() + std::chrono::milliseconds(option.reply_timeout_ms);
    while (!allAnswered() && Clock::now() < deadline && !stop())
        sleepMs(20);
    bool ok = true;
    {
        const std::lock_guard lock(m);
        for (auto& [h, b] : boards) {
            if ((!b.answered || b.failed) && reportMissing) {
                aprintf(stderr, "%s [ERROR] %s: no record status (a streaming server older than 2026-11-18?)\n", getTS(": ").c_str(), h.c_str());
                ok = false;
            }
        }
    }

    if (!ok && !reportMissing) {
        cl->getRecordStatusNofiy.disconnect_all();
        cl->failRecordStatusNofiy.disconnect_all();
        return false;  // an older server: no panel
    }
    if (once) {
        const std::lock_guard lock(m);
        for (auto& [h, b] : boards) {
            if (!b.answered || b.failed)
                continue;
            aprintf(stdout, "%s", keyValues(h, "adc", b.adc.st).c_str());
            aprintf(stdout, "%s", keyValues(h, "gpio_in", b.gpioIn.st).c_str());
            aprintf(stdout, "%s sd free=%" PRIu64 " total=%" PRIu64 "\n", h.c_str(), b.sdFree, b.sdTotal);
        }
        cl->getRecordStatusNofiy.disconnect_all();
        cl->failRecordStatusNofiy.disconnect_all();
        return ok;
    }

    auto lines = [&](bool keys) {
        const std::lock_guard lock(m);
        std::vector<std::string> out;
        int writing = 0;
        for (auto& [h, b] : boards)
            writing += (b.adc.st.running && b.adc.st.mode == StreamRecordStatus::SD) + (b.gpioIn.st.running && b.gpioIn.st.mode == StreamRecordStatus::SD);
        out.push_back(Panel::bold("rpsa_client -r watch") + "  " + std::to_string(boards.size()) + (boards.size() == 1 ? " board  " : " boards  ") +
                      std::to_string(writing) + (writing == 1 ? " recording on an SD card" : " recordings on SD cards"));
        out.push_back("");
        out.push_back(Panel::dim(Panel::left("  board", 16) + Panel::left("stream", 8) + Panel::left("state", 10) + Panel::right("samples/ch", 12) +
                                 Panel::right("limit", 7) + Panel::right("left", 12) + Panel::right("lost b/q", 12)));
        for (auto& [h, b] : boards) {
            if (!b.answered || b.failed) {
                out.push_back(Panel::left("  " + h, 16) + Panel::red("no record status"));
                continue;
            }
            streamLines(h, "adc", b.adc, b, out);
            if (b.gpioIn.st.mode != StreamRecordStatus::NONE)
                streamLines(h, "gpio in", b.gpioIn, b, out);
        }
        out.push_back("");
        out.push_back(Panel::dim(std::string("  the recordings go on when this ends    ") + (keys ? "[q] leave   " : "") + "[Ctrl+C] leave"));
        return out;
    };
    auto anyWriting = [&]() {
        const std::lock_guard lock(m);
        for (auto& [h, b] : boards)
            if ((b.adc.st.running && b.adc.st.mode == StreamRecordStatus::SD) || (b.gpioIn.st.running && b.gpioIn.st.mode == StreamRecordStatus::SD))
                return true;
        return false;
    };

    // start_stop: no board writing its card, nothing to show
    if (!reportMissing && !anyWriting()) {
        cl->getRecordStatusNofiy.disconnect_all();
        cl->failRecordStatusNofiy.disconnect_all();
        return true;
    }
    Panel panel(option);
    auto lastPrint = Clock::now() - std::chrono::seconds(10);
    auto lastAsk = Clock::now();
    while (!stop()) {
        if (Clock::now() - lastAsk >= std::chrono::seconds(1)) {
            ask();
            lastAsk = Clock::now();
            sleepMs(150);  // the answers of this round
        }
        if (panel.enabled()) {
            panel.draw(lines(panel.keys()));
            const int k = panel.key();
            if (k == 'q' || k == 'Q')
                break;
        } else if (Clock::now() - lastPrint >= std::chrono::seconds(5)) {
            for (auto& l : lines(false))
                if (!l.empty())
                    aprintf(stdout, "%s\n", forOutput(l).c_str());
            lastPrint = Clock::now();
        }
        if (!anyWriting())
            break;  // every recording ended (or none ran)
        sleepMs(250);
    }
    panel.close();
    // the last state stays on the screen
    for (auto& l : lines(false))
        aprintf(stdout, "%s\n", forOutput(l).c_str());
    cl->getRecordStatusNofiy.disconnect_all();
    cl->failRecordStatusNofiy.disconnect_all();
    return ok;
}
