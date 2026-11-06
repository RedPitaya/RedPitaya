#include "gpio_file_reader.h"

#include <algorithm>

#include "bin_lib/bin_file.h"

using namespace bin_lib;

CGpioFileReader::CGpioFileReader(const std::string& path, CStreamSettings::GPIORepeat repeat, uint32_t count, bool outRLE, bool gaps)
    : m_repeat(repeat), m_count(count), m_outRLE(outRLE), m_gaps(gaps) {
    if (m_repeat.value == CStreamSettings::GPIORepeat::GPIO_REP_ON && m_count == 0)
        m_count = 1;
    m_file = std::make_unique<FileReader>();
    if (!m_file->open(path)) {
        m_error = m_file->error();
        m_open = Open::CLOSE;
        return;
    }
    const auto& info = m_file->info();
    if (info.channels.size() != 1 || (info.channels[0].type != SampleType::GPIO_IN_WORD && info.channels[0].type != SampleType::GPIO_OUT_WORD)) {
        m_error = "not a file of GPIO words";
        m_open = Open::WRONG_TYPE;
        return;
    }
    m_inWords = info.channels[0].type == SampleType::GPIO_IN_WORD;
    m_fileRLE = (info.channels[0].flags & CHANNEL_FLAG_RLE) != 0;
    m_decoder = GpioInDecoder(m_fileRLE);
    m_encoder = GpioOutEncoder(m_outRLE);
    BlockInfo b;
    std::vector<uint8_t> data;
    if (!m_file->nextBlock(b, data)) {
        m_open = Open::EMPTY;
        return;
    }
    m_file->rewind();
    m_open = Open::OK;
}

CGpioFileReader::~CGpioFileReader() = default;

auto CGpioFileReader::isOpen() const -> Open {
    return m_open;
}

auto CGpioFileReader::error() const -> const std::string& {
    return m_error;
}

auto CGpioFileReader::read(uint32_t* dst, size_t max, size_t& got) -> Result {
    got = 0;
    if (m_open != Open::OK)
        return Result::ENDED;
    while (got < max) {
        if (m_words.empty() && !more())
            break;
        const size_t take = std::min(max - got, m_words.size());
        std::copy(m_words.begin(), m_words.begin() + static_cast<std::ptrdiff_t>(take), dst + got);
        m_words.erase(m_words.begin(), m_words.begin() + static_cast<std::ptrdiff_t>(take));
        got += take;
    }
    return m_words.empty() && m_ended ? Result::ENDED : Result::OK;
}

// One block more into m_words: the runs of its words, after its gap when the gaps play
auto CGpioFileReader::more() -> bool {
    if (m_ended)
        return false;
    BlockInfo b;
    std::vector<uint8_t> data;
    while (!m_file->nextBlock(b, data)) {
        // the end of the file: once more or the end
        bool again = m_repeat.value == CStreamSettings::GPIORepeat::GPIO_REP_INF;
        if (m_repeat.value == CStreamSettings::GPIORepeat::GPIO_REP_ON)
            again = --m_count > 0;
        if (!again) {
            m_encoder.flush(m_out);
            m_words.insert(m_words.end(), m_out.begin(), m_out.end());
            m_out.clear();
            m_ended = true;
            return !m_words.empty();
        }
        m_file->rewind();
        m_decoder.reset();
    }
    m_out.clear();
    auto add = [&](const GpioRun& r) {
        m_encoder.add(r, m_out);
        m_p = r.p;
        m_n = r.n;
        m_haveLevels = true;
    };
    if (b.lost + b.wait) {
        if (m_gaps && m_haveLevels)
            add({b.lost + b.wait, m_p, m_n});  // the lines hold
        m_decoder.reset();                      // the lanes start again after a gap
    }
    const auto* words = reinterpret_cast<const uint32_t*>(data.data());
    const size_t count = data.size() / 4;
    if (m_inWords) {
        m_decoder.feed(words, count, add);
    } else {
        for (size_t i = 0; i < count; i++)
            add({m_fileRLE ? outCount(words[i]) + 1 : 1, outP(words[i]), outN(words[i])});
    }
    m_words.insert(m_words.end(), m_out.begin(), m_out.end());
    m_out.clear();
    return true;
}
