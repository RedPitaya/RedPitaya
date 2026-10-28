#ifndef READER_CONTROLLER_H
#define READER_CONTROLLER_H

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "settings_lib/channels.hpp"
#include "settings_lib/stream_settings.h"

namespace TDMS {
class FileReader;
}
class CWaveReader;

/**
 * The samples of the DAC channels, block by block, from a TDMS or WAV file, from memory or from a
 * callback.
 *
 * readBlock() writes the next block of every channel straight into the memory it is given (the
 * DMA buffers of a DAC pack): from a file the samples are read into it with no buffer in between,
 * a callback fills it itself. Samples come out signed, 8 or 16 bit, as the DAC takes them:
 * unsigned TDMS channels and 8 bit WAV (unsigned by the format) are converted in place.
 *
 * TDMS: the channels "ch1" and "ch2" of the group "Group", or of the first group that has them;
 * any other channel (the time column, the buffer rows of a recording) is ignored.
 * WAV: PCM, 8 or 16 bit, mono or stereo.
 */
class CReaderController {
   public:
    struct DataIn {
        uint8_t* ch[MAX_DAC_CHANNELS] = {nullptr, nullptr};
        size_t size[MAX_DAC_CHANNELS] = {0, 0};
        uint8_t bits = {0};
        size_t readPosition = 0;
    };

    // What readBlock() put into a block, per channel: the block size, or 0 for a channel it did not
    // write; the bytes of samples before the zero fill at the end; the bits of a sample
    struct Data {
        size_t size[MAX_DAC_CHANNELS] = {0, 0};
        size_t real_size[MAX_DAC_CHANNELS] = {0, 0};
        uint8_t bits[MAX_DAC_CHANNELS] = {0, 0};
    };

    typedef std::function<bool(uint8_t bitsBySample, std::array<uint8_t*, MAX_DAC_CHANNELS>& buffers, const uint32_t bufferSize)> MemoryStreamDataCallback_t;

    struct MemoryStreamSink {
        uint8_t memoryStreamBits;
        dac_channels_t channels;
        MemoryStreamDataCallback_t callback = nullptr;
    };

    enum OpenResult { OR_OK = 0, OR_MISSING_CHANNELS = 1, OR_WRONG_DATA_TYPE = 2, OR_DATA_NOT_EQUAL = 3, OR_CLOSE = 4 };

    enum BufferResult { BR_OK = 0, BR_ENDED = 1, BR_BROKEN = 2, BR_EMPTY = 3 };

    using Ptr = std::shared_ptr<CReaderController>;
    static Ptr Create(CStreamSettings::DataFormat _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize);

    static Ptr Create(DataIn* dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize);

    CReaderController(CStreamSettings::DataFormat _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize);

    // Takes over dataIn; its sample memory stays the caller's
    CReaderController(DataIn* dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize);

    // Takes over sink
    CReaderController(MemoryStreamSink* sink, uint32_t blockSize);

    ~CReaderController();

    auto isOpen() -> CReaderController::OpenResult;
    auto getChannels(dac_channels_t& channels) -> void;
    // Bytes of samples per channel in the whole source
    auto getChannelsSize(size_t* ch1Size, size_t* ch2Size) -> void;
    auto disableRepeatMode() -> void;

    // Writes the next block (the block size in bytes) of every channel there is into dst; the
    // block of a channel without samples, or with dst null, is not touched. At the end of the
    // samples the rest of the block is zero; BR_ENDED then, or BR_OK with repeat on. BR_EMPTY when
    // the source has no samples at all.
    auto readBlock(std::array<uint8_t*, MAX_DAC_CHANNELS> dst, Data& data) -> BufferResult;

   private:
    CReaderController(CReaderController const&) = delete;
    CReaderController& operator=(CReaderController const&) = delete;

    auto openTDMS() -> OpenResult;
    auto openWav() -> OpenResult;
    auto openMemory() -> OpenResult;
    auto openSink() -> OpenResult;
    // Up to n samples of every channel from m_position into dst + offset; how many
    auto readSamples(std::array<uint8_t*, MAX_DAC_CHANNELS>& dst, uint64_t offset, uint64_t n) -> uint64_t;

    CStreamSettings::DataFormat m_fileType;
    std::string m_filePath;
    CStreamSettings::DACRepeat m_repeat;
    uint32_t m_rep_count;
    uint32_t m_blockSize;
    OpenResult m_result;

    bool m_present[MAX_DAC_CHANNELS] = {false, false};
    uint8_t m_bits = 0;           // of a sample of every present channel
    bool m_unsigned = false;      // samples to turn into signed ones
    uint64_t m_samples = 0;       // per channel
    uint64_t m_position = 0;      // the next sample
    bool m_anySample = false;     // a sample was read since the start

    std::unique_ptr<TDMS::FileReader> m_tdms;
    std::string m_tdmsGroup;
    std::unique_ptr<CWaveReader> m_wav;
    std::vector<uint8_t> m_frames;  // interleaved stereo frames on their way to the two blocks
    DataIn* m_genData;
    MemoryStreamSink* m_memSink;
};

#endif
