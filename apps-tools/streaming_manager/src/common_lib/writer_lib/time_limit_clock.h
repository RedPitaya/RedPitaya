#ifndef WRITER_LIB_TIME_LIMIT_CLOCK_H
#define WRITER_LIB_TIME_LIMIT_CLOCK_H

#include <algorithm>
#include <chrono>
#include <mutex>

// The time limit of a recording on the clock of the computer (streaming_improvements 4.11).
// The limit is taken on the time line of the stream when a buffer comes; while the FPGA waits for
// a trigger no buffer comes, and a trigger that never comes would keep the recording on. This
// clock is due once both:
//   - the limit, the longest buffer (the one that crosses the limit is written whole; before the
//     first one: the hint of the settings) and a margin have passed since start();
//   - no buffer came for that buffer and the margin: buffers that come late (a ring behind a slow
//     network or card) still end the limit on the time line, not this clock.
// Without a hint and before the first buffer it is never due: a buffer may last long (a large
// decimation). A GPIO buffer of RLE words may last up to 256 samples a word: its hint is that
// long, the clock waits for it. Thread safe.
class TimeLimitClock {
   public:
    using Clock = std::chrono::steady_clock;
    static constexpr double kMargin = 2.0;  // seconds: the start of the board, the network

    // `limit` seconds, 0 - none; `bufferHint`: about how long a buffer lasts, 0 - unknown
    auto start(double limit, double bufferHint) -> void {
        std::lock_guard lock(m_mtx);
        m_limit = limit;
        m_buffer = std::max(0.0, bufferHint);
        m_start = m_last = Clock::now();
    }
    // a buffer of that many seconds of the time line came
    auto buffer(double seconds) -> void { buffer(seconds, Clock::now()); }
    auto buffer(double seconds, Clock::time_point now) -> void {
        std::lock_guard lock(m_mtx);
        m_buffer = std::max(m_buffer, seconds);
        m_last = now;
    }
    // the tests: a shorter margin
    auto setMargin(double seconds) -> void {
        std::lock_guard lock(m_mtx);
        m_margin = seconds;
    }
    auto off() -> void {
        std::lock_guard lock(m_mtx);
        m_limit = 0;
    }
    auto due() -> bool { return due(Clock::now()); }
    auto due(Clock::time_point now) -> bool {
        std::lock_guard lock(m_mtx);
        if (m_limit <= 0 || m_buffer <= 0)
            return false;
        const double sinceStart = std::chrono::duration<double>(now - m_start).count();
        const double sinceBuffer = std::chrono::duration<double>(now - m_last).count();
        return sinceStart >= m_limit + m_buffer + m_margin && sinceBuffer >= m_buffer + m_margin;
    }

   private:
    std::mutex m_mtx;
    double m_limit = 0;
    double m_buffer = 0;
    double m_margin = kMargin;
    Clock::time_point m_start{};
    Clock::time_point m_last{};  // the last buffer, else the start
};

#endif
