#ifndef WAV_LIB_WAVRAEDER_H
#define WAV_LIB_WAVRAEDER_H

#include <stdint.h>
#include <fstream>
#include <string>

/**
 * A RIFF/WAVE file read by frames.
 *
 * openFile() walks the chunks: "fmt " gives the format, "data" the samples; any other chunk
 * (LIST, fact, ...) before or after them is skipped. PCM and WAVE_FORMAT_EXTENSIBLE with a PCM
 * or float subformat are understood. Samples are read raw: interleaved, little-endian, 8 bit
 * unsigned as the format has them.
 */
class CWaveReader {
   public:
    enum class Format { PCM, FLOAT, OTHER };

    CWaveReader();
    ~CWaveReader();

    // False when the file is not there or not a RIFF/WAVE file with "fmt " and "data"
    auto openFile(std::string fileName) -> bool;
    auto error() const -> const std::string& { return m_error; }

    auto format() const -> Format { return m_format; }
    auto channels() const -> uint16_t { return m_channels; }
    auto bitsPerSample() const -> uint16_t { return m_bits; }
    auto sampleRate() const -> uint32_t { return m_rate; }
    auto frameSize() const -> uint32_t { return m_blockAlign; }  // bytes of one sample of every channel
    auto frames() const -> uint64_t;
    auto getDataSize() const -> uint64_t { return m_dataSize; }  // bytes of samples

    // Reads up to `count` frames from frame `first` into `dst` (count * frameSize() bytes) and
    // returns how many it read
    auto readFrames(uint64_t first, uint64_t count, void* dst) -> uint64_t;

   private:
    std::ifstream m_read_fs;
    std::string m_error;
    Format m_format = Format::OTHER;
    uint16_t m_channels = 0;
    uint16_t m_bits = 0;
    uint32_t m_rate = 0;
    uint32_t m_blockAlign = 0;
    uint64_t m_dataOffset = 0;
    uint64_t m_dataSize = 0;
};

#endif
