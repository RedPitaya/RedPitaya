#include "gpio_words.h"

#include <algorithm>

using namespace bin_lib;

auto GpioInDecoder::feed(const uint32_t* words, size_t count, const std::function<void(const GpioRun&)>& run) -> void {
    m_words.insert(m_words.end(), words, words + count);
    m_samples += gpioInSamples(words, count, m_rle);
    for (;;) {
        if (m_pLeft == 0) {
            if (m_p >= m_words.size())
                break;
            const uint32_t w = m_words[m_p++];
            m_pLevel = inP(w);
            m_pLeft = m_rle ? inPCount(w) + 1 : 1;
        }
        if (m_nLeft == 0) {
            if (m_n >= m_words.size())
                break;
            const uint32_t w = m_words[m_n++];
            m_nLevel = inN(w);
            m_nLeft = m_rle ? inNCount(w) + 1 : 1;
        }
        const uint64_t len = std::min(m_pLeft, m_nLeft);
        run({len, m_pLevel, m_nLevel});
        m_pLeft -= len;
        m_nLeft -= len;
    }
    // the words both lanes have taken go
    const size_t done = std::min(m_p, m_n);
    if (done) {
        m_words.erase(m_words.begin(), m_words.begin() + static_cast<std::ptrdiff_t>(done));
        m_p -= done;
        m_n -= done;
    }
}

auto GpioInDecoder::reset() -> void {
    m_words.clear();
    m_p = m_n = 0;
    m_pLeft = m_nLeft = 0;
    m_pLevel = m_nLevel = 0;
    m_samples = 0;
}

auto GpioOutEncoder::add(const GpioRun& run, std::vector<uint32_t>& out) -> void {
    if (run.samples == 0)
        return;
    if (m_held.samples && run.p == m_held.p && run.n == m_held.n) {
        m_held.samples += run.samples;
        return;
    }
    flush(out);
    m_held = run;
}

auto GpioOutEncoder::flush(std::vector<uint32_t>& out) -> void {
    if (m_held.samples)
        emit(m_held, out);
    m_held = GpioRun();
}

auto GpioOutEncoder::emit(const GpioRun& run, std::vector<uint32_t>& out) -> void {
    uint64_t left = run.samples;
    if (!m_rle) {
        out.insert(out.end(), left, outWord(run.p, run.n, 0));
        return;
    }
    while (left) {
        const uint64_t take = std::min<uint64_t>(left, 256);
        out.push_back(outWord(run.p, run.n, static_cast<uint32_t>(take - 1)));
        left -= take;
    }
}

auto bin_lib::gpioOutSamples(const uint32_t* words, size_t count, bool rle) -> uint64_t {
    if (!rle)
        return count;
    uint64_t s = 0;
    for (size_t i = 0; i < count; i++)
        s += outCount(words[i]) + 1;
    return s;
}

auto bin_lib::gpioInSamples(const uint32_t* words, size_t count, bool rle) -> uint64_t {
    if (!rle)
        return count;
    uint64_t s = 0;
    for (size_t i = 0; i < count; i++)
        s += inPCount(words[i]) + 1;
    return s;
}
