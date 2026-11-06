#ifndef BIN_LIB_GPIO_WORDS_H
#define BIN_LIB_GPIO_WORDS_H

#include <cstdint>
#include <functional>
#include <vector>

// The 32 bit words of the GPIO block of stream_app.
//
// Input {N count, N lines, P count, P lines}: bits 7:0 the P lines, 15:8 the P count, 23:16 the
// N lines, 31:24 the N count. With RLE the P and N lanes run on their own: a word holds a run of
// count + 1 samples of each lane, the runs of the two lanes need not end together. Without RLE a
// word is one sample of both.
//
// Output {P lines, N lines, unused, count}: bits 31:24 the P lines, 23:16 the N lines, 7:0 the
// count. With RLE the lines hold count + 1 samples, without it one.
namespace bin_lib {

constexpr auto inP(uint32_t w) -> uint8_t {
    return static_cast<uint8_t>(w);
}
constexpr auto inPCount(uint32_t w) -> uint32_t {
    return (w >> 8) & 0xFF;
}
constexpr auto inN(uint32_t w) -> uint8_t {
    return static_cast<uint8_t>(w >> 16);
}
constexpr auto inNCount(uint32_t w) -> uint32_t {
    return w >> 24;
}
constexpr auto inWord(uint8_t p, uint32_t pCount, uint8_t n, uint32_t nCount) -> uint32_t {
    return p | (pCount & 0xFF) << 8 | static_cast<uint32_t>(n) << 16 | (nCount & 0xFF) << 24;
}
constexpr auto outWord(uint8_t p, uint8_t n, uint32_t count) -> uint32_t {
    return static_cast<uint32_t>(p) << 24 | static_cast<uint32_t>(n) << 16 | (count & 0xFF);
}
constexpr auto outP(uint32_t w) -> uint8_t {
    return static_cast<uint8_t>(w >> 24);
}
constexpr auto outN(uint32_t w) -> uint8_t {
    return static_cast<uint8_t>(w >> 16);
}
constexpr auto outCount(uint32_t w) -> uint32_t {
    return w & 0xFF;
}

// The levels of all lines over a number of samples
struct GpioRun {
    uint64_t samples = 0;
    uint8_t p = 0;
    uint8_t n = 0;
};

// The input words to runs of both lanes on one time line. The words may come in pieces (the
// buffers of a stream): a lane whose run goes on in the next piece is kept for it.
class GpioInDecoder {
   public:
    explicit GpioInDecoder(bool rle) : m_rle(rle) {}
    // the runs the words complete, in order
    auto feed(const uint32_t* words, size_t count, const std::function<void(const GpioRun&)>& run) -> void;
    // the samples of the input words given so far (of the P lane)
    auto samples() const -> uint64_t { return m_samples; }
    auto reset() -> void;

   private:
    bool m_rle;
    std::vector<uint32_t> m_words;  // not yet taken by both lanes
    size_t m_p = 0;                 // the next word of each lane in m_words
    size_t m_n = 0;
    uint64_t m_pLeft = 0;  // samples left of the run of each lane
    uint64_t m_nLeft = 0;
    uint8_t m_pLevel = 0;
    uint8_t m_nLevel = 0;
    uint64_t m_samples = 0;
};

// Runs to output words; neighbouring runs of the same levels are one
class GpioOutEncoder {
   public:
    explicit GpioOutEncoder(bool rle) : m_rle(rle) {}
    auto add(const GpioRun& run, std::vector<uint32_t>& out) -> void;
    // the run held back for a neighbour of the same levels
    auto flush(std::vector<uint32_t>& out) -> void;

   private:
    auto emit(const GpioRun& run, std::vector<uint32_t>& out) -> void;
    bool m_rle;
    GpioRun m_held;
};

// The samples output words stand for
auto gpioOutSamples(const uint32_t* words, size_t count, bool rle) -> uint64_t;
// The samples of the P lane of input words
auto gpioInSamples(const uint32_t* words, size_t count, bool rle) -> uint64_t;

}  // namespace bin_lib

#endif
