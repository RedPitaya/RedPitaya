#ifndef READER_LIB_GPIO_FILE_READER_H
#define READER_LIB_GPIO_FILE_READER_H

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <vector>

#include "bin_lib/gpio_words.h"
#include "settings_lib/stream_settings.h"

namespace bin_lib {
class FileReader;
}

// The words of the GPIO output from a BIN stream file: a recording of the GPIO input (its runs
// become output words) or output words. The words come out as the output takes them, with or
// without RLE whatever the file has. With gaps the samples the recording lost or waited for a
// trigger hold the lines of the sample before, so that it plays in its own time.
class CGpioFileReader {
   public:
    enum class Open { OK, CLOSE, WRONG_TYPE, EMPTY };
    enum class Result { OK, ENDED };

    CGpioFileReader(const std::string& path, CStreamSettings::GPIORepeat repeat, uint32_t count, bool outRLE, bool gaps);
    ~CGpioFileReader();

    auto isOpen() const -> Open;
    auto error() const -> const std::string&;

    // Up to `max` words into dst, `got` of them: ENDED with the last words of the last repeat
    auto read(uint32_t* dst, size_t max, size_t& got) -> Result;

   private:
    auto more() -> bool;  // the words of the next block; false at the end of the last repeat

    std::unique_ptr<bin_lib::FileReader> m_file;
    std::string m_error;
    Open m_open = Open::CLOSE;
    CStreamSettings::GPIORepeat m_repeat;
    uint32_t m_count;
    bool m_outRLE;
    bool m_gaps;
    bool m_inWords = false;  // the file holds input words
    bool m_fileRLE = false;
    bool m_ended = false;

    bin_lib::GpioInDecoder m_decoder{false};
    bin_lib::GpioOutEncoder m_encoder{false};
    std::deque<uint32_t> m_words;
    std::vector<uint32_t> m_out;
    bool m_haveLevels = false;
    uint8_t m_p = 0;
    uint8_t m_n = 0;
};

#endif
