#include "reader_controller.h"

#include <algorithm>
#include <cstring>

#include "logger_lib/file_logger.h"
#include "tdms_lib/tdms.h"
#include "wav_lib/wav_reader.h"

namespace {

const char* const kTDMSChannels[MAX_DAC_CHANNELS] = {"ch1", "ch2"};

// Unsigned samples to signed ones of the same width: the top bit flips
auto toSigned(uint8_t* data, uint64_t samples, uint8_t bits) -> void {
    if (bits == 8) {
        for (uint64_t i = 0; i < samples; i++)
            data[i] ^= 0x80;
    } else {
        for (uint64_t i = 0; i < samples; i++)
            data[i * 2 + 1] ^= 0x80;  // little-endian: the high byte is the second
    }
}

}  // namespace

CReaderController::Ptr CReaderController::Create(CStreamSettings::DataFormat _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count,
                                                 uint32_t blockSize) {
    return std::make_shared<CReaderController>(_fileType, _filePath, _repeat, _rep_count, blockSize);
}

CReaderController::Ptr CReaderController::Create(DataIn* dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize) {
    return std::make_shared<CReaderController>(dataIn, _repeat, _rep_count, blockSize);
}

CReaderController::CReaderController(CStreamSettings::DataFormat _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize)
    : m_fileType(_fileType),
      m_filePath(_filePath),
      m_repeat(_repeat),
      m_rep_count(_rep_count),
      m_blockSize(blockSize),
      m_result(OR_CLOSE),
      m_genData(nullptr),
      m_memSink(nullptr) {
    if (m_repeat.value == CStreamSettings::DACRepeat::DAC_REP_ON && m_rep_count == 0) {
        m_rep_count = 1;
    }
    if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
        m_result = openWav();
    } else if (m_fileType.value == CStreamSettings::DataFormat::TDMS) {
        m_result = openTDMS();
    }
}

CReaderController::CReaderController(DataIn* dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize)
    : m_fileType(CStreamSettings::DataFormat::BIN),
      m_filePath(""),
      m_repeat(_repeat),
      m_rep_count(_rep_count),
      m_blockSize(blockSize),
      m_result(OR_CLOSE),
      m_genData(dataIn),
      m_memSink(nullptr) {
    if (m_repeat.value == CStreamSettings::DACRepeat::DAC_REP_ON && m_rep_count == 0) {
        m_rep_count = 1;
    }
    if (dataIn) {
        if (dataIn->ch[0] && dataIn->ch[1] && dataIn->size[0] != dataIn->size[1])
            FATAL("The buffers of the first and second channels must be equal.")
        if (dataIn->size[0] & 0x1 || dataIn->size[1] & 0x1)
            FATAL("Data buffers must be a multiple of 2")
    } else {
        FATAL("Memory not initialized")
    }
    m_result = openMemory();
}

CReaderController::CReaderController(MemoryStreamSink* sink, uint32_t blockSize)
    : m_fileType(CStreamSettings::DataFormat::BIN),
      m_filePath(""),
      m_repeat(CStreamSettings::DACRepeat::DAC_REP_INF),
      m_rep_count(0),
      m_blockSize(blockSize),
      m_result(OR_CLOSE),
      m_genData(nullptr),
      m_memSink(sink) {
    m_result = openSink();
}

CReaderController::~CReaderController() {
    delete m_genData;
    delete m_memSink;
}

auto CReaderController::openTDMS() -> OpenResult {
    m_tdms = std::make_unique<TDMS::FileReader>();
    if (!m_tdms->open(m_filePath)) {
        ERROR_LOG("Cannot read the TDMS file %s: %s", m_filePath.c_str(), m_tdms->error().c_str());
        m_tdms.reset();
        return OR_CLOSE;
    }
    // The group "Group" of a recording, or the first group with a channel ch1 or ch2
    auto groups = m_tdms->groups();
    if (std::find(groups.begin(), groups.end(), "Group") != groups.end())
        groups.insert(groups.begin(), "Group");
    for (const auto& g : groups) {
        if (m_tdms->channel(g, kTDMSChannels[0]) || m_tdms->channel(g, kTDMSChannels[1])) {
            m_tdmsGroup = g;
            break;
        }
    }
    if (m_tdmsGroup.empty())
        return OR_MISSING_CHANNELS;

    TDMS::TDMSType type = TDMS::TDMSType::Empty;
    for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++) {
        const auto* info = m_tdms->channel(m_tdmsGroup, kTDMSChannels[ch]);
        if (!info || info->count == 0)
            continue;
        switch (info->type) {
            case TDMS::TDMSType::Integer8:
            case TDMS::TDMSType::UnsignedInteger8:
            case TDMS::TDMSType::Integer16:
            case TDMS::TDMSType::UnsignedInteger16:
                break;
            default:
                return OR_WRONG_DATA_TYPE;
        }
        if (type != TDMS::TDMSType::Empty && info->type != type)
            return OR_WRONG_DATA_TYPE;  // both channels have one sample format
        if (m_present[0] && info->count != m_samples)
            return OR_DATA_NOT_EQUAL;
        type = info->type;
        m_present[ch] = true;
        m_samples = info->count;
    }
    if (!m_present[0] && !m_present[1])
        return OR_MISSING_CHANNELS;
    m_bits = (type == TDMS::TDMSType::Integer8 || type == TDMS::TDMSType::UnsignedInteger8) ? 8 : 16;
    m_unsigned = type == TDMS::TDMSType::UnsignedInteger8 || type == TDMS::TDMSType::UnsignedInteger16;
    return OR_OK;
}

auto CReaderController::openWav() -> OpenResult {
    m_wav = std::make_unique<CWaveReader>();
    if (!m_wav->openFile(m_filePath)) {
        ERROR_LOG("Cannot read the WAV file %s: %s", m_filePath.c_str(), m_wav->error().c_str());
        m_wav.reset();
        return OR_CLOSE;
    }
    if (m_wav->format() != CWaveReader::Format::PCM || !(m_wav->bitsPerSample() == 8 || m_wav->bitsPerSample() == 16))
        return OR_WRONG_DATA_TYPE;
    if (m_wav->channels() != 1 && m_wav->channels() != 2)
        return OR_MISSING_CHANNELS;
    m_present[0] = true;
    m_present[1] = m_wav->channels() == 2;
    m_bits = static_cast<uint8_t>(m_wav->bitsPerSample());
    m_unsigned = m_bits == 8;  // 8 bit WAV samples are unsigned, 128 is zero
    m_samples = m_wav->frames();
    return OR_OK;
}

auto CReaderController::openMemory() -> OpenResult {
    m_bits = m_genData->bits;
    const unsigned bytes = m_bits / 8 ? m_bits / 8 : 1;
    for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++) {
        m_present[ch] = m_genData->ch[ch] != nullptr;
        if (m_present[ch])
            m_samples = std::max<uint64_t>(m_samples, m_genData->size[ch] / bytes);
    }
    return (m_present[0] || m_present[1]) ? OR_OK : OR_MISSING_CHANNELS;
}

auto CReaderController::openSink() -> OpenResult {
    if (!m_memSink || m_memSink->callback == nullptr) {
        WARNING("Missing callback in memory sink")
        return OR_CLOSE;
    }
    m_present[0] = m_memSink->channels.isEnabled(DACChannels::DAC_CH1);
    m_present[1] = m_memSink->channels.isEnabled(DACChannels::DAC_CH2);
    m_bits = m_memSink->memoryStreamBits;
    return OR_OK;
}

auto CReaderController::isOpen() -> OpenResult {
    return m_result;
}

auto CReaderController::getChannels(dac_channels_t& channels) -> void {
    channels[DACChannels::DAC_CH1] = m_result == OR_OK && m_present[0];
    channels[DACChannels::DAC_CH2] = m_result == OR_OK && m_present[1];
}

auto CReaderController::getChannelsSize(size_t* ch1Size, size_t* ch2Size) -> void {
    if (m_memSink) {
        *ch1Size = m_present[0] ? m_blockSize : 0;
        *ch2Size = m_present[1] ? m_blockSize : 0;
        return;
    }
    const uint64_t bytes = m_samples * (m_bits / 8);
    *ch1Size = m_result == OR_OK && m_present[0] ? bytes : 0;
    *ch2Size = m_result == OR_OK && m_present[1] ? bytes : 0;
}

auto CReaderController::disableRepeatMode() -> void {
    m_repeat = CStreamSettings::DACRepeat::DAC_REP_OFF;
}

auto CReaderController::readSamples(std::array<uint8_t*, MAX_DAC_CHANNELS>& dst, uint64_t offset, uint64_t n) -> uint64_t {
    const uint64_t bytes = m_bits / 8;
    n = std::min(n, m_samples - m_position);
    if (n == 0)
        return 0;
    uint64_t got = n;
    if (m_tdms) {
        for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++)
            if (m_present[ch] && dst[ch])
                got = std::min(got, m_tdms->read(m_tdmsGroup, kTDMSChannels[ch], m_position, n, dst[ch] + offset * bytes));
    } else if (m_wav) {
        if (!m_present[1]) {
            got = dst[0] ? m_wav->readFrames(m_position, n, dst[0] + offset * bytes) : n;
        } else {
            // Stereo frames: read once, then each sample to its channel
            m_frames.resize(static_cast<size_t>(n * 2 * bytes));
            got = m_wav->readFrames(m_position, n, m_frames.data());
            for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++) {
                if (!dst[ch])
                    continue;
                uint8_t* out = dst[ch] + offset * bytes;
                for (uint64_t i = 0; i < got; i++)
                    std::memcpy(out + i * bytes, m_frames.data() + (i * 2 + ch) * bytes, bytes);
            }
        }
    } else if (m_genData) {
        for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++) {
            if (!m_present[ch] || !dst[ch])
                continue;
            const uint64_t have = m_genData->size[ch] / bytes;
            const uint64_t from = std::min(m_position, have);
            const uint64_t take = std::min(n, have - from);
            std::memcpy(dst[ch] + offset * bytes, m_genData->ch[ch] + from * bytes, take * bytes);
            std::memset(dst[ch] + (offset + take) * bytes, 0, (n - take) * bytes);
        }
    }
    if (m_unsigned) {
        for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++)
            if (m_present[ch] && dst[ch])
                toSigned(dst[ch] + offset * bytes, got, m_bits);
    }
    m_position += got;
    return got;
}

auto CReaderController::readBlock(std::array<uint8_t*, MAX_DAC_CHANNELS> dst, Data& data) -> BufferResult {
    data = Data();
    for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++)
        if (!m_present[ch])
            dst[ch] = nullptr;
    if (m_result != OR_OK)
        return BR_EMPTY;

    if (m_memSink) {
        for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++) {
            if (dst[ch]) {
                data.size[ch] = data.real_size[ch] = m_blockSize;
                data.bits[ch] = m_bits;
            }
        }
        auto isLastBuffer = m_memSink->callback(m_bits, dst, m_blockSize);
        return isLastBuffer == false ? BR_OK : BR_ENDED;
    }

    const uint64_t bytes = m_bits / 8;
    const uint64_t perBlock = bytes ? m_blockSize / bytes : 0;
    uint64_t filled = 0;
    BufferResult result = BR_OK;
    while (filled < perBlock) {
        const uint64_t got = readSamples(dst, filled, perBlock - filled);
        filled += got;
        m_anySample |= got > 0;
        if (filled == perBlock)
            break;
        if (got > 0 && m_position < m_samples)
            continue;  // a short read inside the samples: the file ends early
        // The end of the samples
        if (!m_anySample)
            return BR_EMPTY;
        if (m_repeat.value == CStreamSettings::DACRepeat::DAC_REP_OFF) {
            result = BR_ENDED;
            break;
        }
        if (m_repeat.value == CStreamSettings::DACRepeat::DAC_REP_ON) {
            m_rep_count--;
            if (m_rep_count <= 0) {
                result = BR_ENDED;
                break;
            }
        }
        if (m_position == 0)
            return BR_BROKEN;  // nothing could be read even from the start
        m_position = 0;
    }

    if (filled == 0)
        return result;
    for (int ch = 0; ch < MAX_DAC_CHANNELS; ch++) {
        if (!dst[ch])
            continue;
        std::memset(dst[ch] + filled * bytes, 0, (perBlock - filled) * bytes);
        data.size[ch] = m_blockSize;
        data.real_size[ch] = filled * bytes;
        data.bits[ch] = m_bits;
    }
    return result;
}
