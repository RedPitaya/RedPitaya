/**
 * Helpers for the reader_lib tests: build a TDMS file on disk with a chosen
 * segment shape, and drain a CReaderController through its public interface.
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>
#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

#include "reader_lib/reader_controller.h"
#include "tdms_lib/file.h"

namespace reader_test {

// getpid() is POSIX and absent from the MinGW runtime, which builds this tree
// for Windows; _getpid() in <process.h> is the equivalent there. The id only has
// to make the scratch directory unique between concurrent runs of the binary.
inline auto CurrentProcessId() -> long {
#if defined(_WIN32)
    return static_cast<long>(_getpid());
#else
    return static_cast<long>(::getpid());
#endif
}

// A uniquely named file removed on destruction.
class TempFile {
   public:
    explicit TempFile(const std::string& name) {
        static int counter = 0;
        m_dir = std::filesystem::temp_directory_path() / ("reader_test_" + std::to_string(++counter) + "_" + std::to_string(CurrentProcessId()));
        std::filesystem::create_directories(m_dir);
        m_path = (m_dir / name).string();
    }
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    ~TempFile() {
        std::error_code ignored;
        std::filesystem::remove_all(m_dir, ignored);
    }
    auto path() const -> const std::string& { return m_path; }

   private:
    std::filesystem::path m_dir;
    std::string m_path;
};

// One segment: optionally a group object, then one int16 channel named
// `channel` carrying `count` samples starting at `base`.
//
// `withGroup == false` is the interesting case: the segment's FIRST object is
// then the channel itself, which is the layout NI's incremental metadata
// produces and the one the metadata cursor used to mishandle.
inline auto AppendSegment(std::iostream& out, bool withGroup, const std::string& channel, std::int16_t base, int count,
                          std::vector<std::shared_ptr<std::uint8_t[]>>& keepAlive) -> void {
    TDMS::WriterSegment segment;
    std::vector<std::shared_ptr<TDMS::Metadata>> nodes;

    auto root = segment.GenerateRoot();
    root->TableOfContents.HasMetaData = true;
    root->TableOfContents.HasRawData = true;
    nodes.push_back(root);

    if (withGroup) {
        nodes.push_back(segment.GenerateGroup("Group"));
    }

    auto node = segment.GenerateChannel("Group", channel);
    nodes.push_back(node);

    auto buffer = std::shared_ptr<std::uint8_t[]>(new std::uint8_t[static_cast<std::size_t>(count) * sizeof(std::int16_t)]);
    for (int i = 0; i < count; i++) {
        const std::int16_t value = static_cast<std::int16_t>(base + i);
        std::memcpy(buffer.get() + static_cast<std::size_t>(i) * sizeof(value), &value, sizeof(value));
    }
    keepAlive.push_back(buffer);
    segment.AddRaw(node, TDMS::TDMSType::Integer16, count, buffer);

    segment.LoadMetadata(nodes);
    TDMS::Writer writer(out, true);
    writer.Write(segment);
}

struct FileShape {
    bool withGroup = true;
    int segments = 1;
    int samplesPerSegment = 4;
    std::string channel = "ch1";
};

// Writes the described file and returns the int16 samples it should contain.
inline auto WriteFile(const std::string& path, const FileShape& shape) -> std::vector<std::int16_t> {
    std::vector<std::shared_ptr<std::uint8_t[]>> keepAlive;
    std::vector<std::int16_t> expected;
    {
        std::fstream fs(path, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
        for (int s = 0; s < shape.segments; s++) {
            const std::int16_t base = static_cast<std::int16_t>(10 * (s + 1));
            AppendSegment(fs, shape.withGroup, shape.channel, base, shape.samplesPerSegment, keepAlive);
            for (int i = 0; i < shape.samplesPerSegment; i++) {
                expected.push_back(static_cast<std::int16_t>(base + i));
            }
        }
    }
    return expected;
}

inline auto ResultName(CReaderController::BufferResult result) -> const char* {
    switch (result) {
        case CReaderController::BR_OK:
            return "BR_OK";
        case CReaderController::BR_ENDED:
            return "BR_ENDED";
        case CReaderController::BR_BROKEN:
            return "BR_BROKEN";
        case CReaderController::BR_EMPTY:
            return "BR_EMPTY";
    }
    return "BR_?";
}

inline auto OpenResultName(CReaderController::OpenResult result) -> const char* {
    switch (result) {
        case CReaderController::OR_OK:
            return "OR_OK";
        case CReaderController::OR_MISSING_CHANNELS:
            return "OR_MISSING_CHANNELS";
        case CReaderController::OR_WRONG_DATA_TYPE:
            return "OR_WRONG_DATA_TYPE";
        case CReaderController::OR_DATA_NOT_EQUAL:
            return "OR_DATA_NOT_EQUAL";
        case CReaderController::OR_CLOSE:
            return "OR_CLOSE";
    }
    return "OR_?";
}

/**
 * Everything the tdms_lib layer reports about a file, as text.
 *
 * This exists so that a failure on a machine I cannot reach still says WHICH
 * layer broke. The same one File object is reused across all segments, exactly
 * as CReaderController does, because File::GetMetadata carries the
 * previous-metadata lookup from one segment to the next and a fresh File per
 * segment would not reproduce the reader's view.
 */
inline auto DescribeTdmsFile(const std::string& path) -> std::string {
    std::ostringstream out;
    std::error_code ec;
    const auto size = std::filesystem::file_size(path, ec);
    out << "\n--- tdms layer: " << path << " (" << (ec ? -1 : static_cast<std::int64_t>(size)) << " bytes) ---\n";

    TDMS::File file;
    const auto segments = file.ReadFileWithoutClose(path);
    out << "segments=" << segments.size() << "\n";
    for (std::size_t s = 0; s < segments.size(); s++) {
        const auto& seg = segments[s];
        out << "  seg[" << s << "] id='" << seg->Identifier << "' version=" << seg->Version << " offset=" << seg->Offset
            << " metaOffset=" << seg->MetadataOffset << " rawOffset=" << seg->RawDataOffset << " nextOffset=" << seg->NextSegmentOffset
            << " toc{meta=" << seg->TableOfContents.HasMetaData << " raw=" << seg->TableOfContents.HasRawData
            << " newObj=" << seg->TableOfContents.ContainsNewObjects << " interleaved=" << seg->TableOfContents.RawDataIsInterleaved
            << " daqmx=" << seg->TableOfContents.HasDaqMxData << " bigEndian=" << seg->TableOfContents.NumbersAreBigEndian << "}\n";

        const auto records = file.GetMetadata(segments[s]);
        out << "    records=" << records.size() << "\n";
        for (std::size_t r = 0; r < records.size(); r++) {
            const auto& m = records[r];
            const auto blocks = m->RawData.DataType.GetRawVector();
            std::int64_t rawBytes = 0;
            for (const auto& block : blocks) {
                rawBytes += static_cast<std::int64_t>(block ? block->size : 0);
            }
            out << "      [" << r << "] pathStr='" << m->PathStr << "' pathParts=" << m->Path.size()
                << " dataType=0x" << std::hex << static_cast<unsigned>(m->RawData.DataType.GetDataType()) << std::dec
                << " count=" << m->RawData.Count << " size=" << m->RawData.Size << " offset=" << m->RawData.Offset
                << " dim=" << m->RawData.Dimension << " interleaved=" << m->RawData.IsInterleaved
                << " stride=" << m->RawData.InterleaveStride << " rawBlocks=" << blocks.size() << " rawBytes=" << rawBytes << "\n";
        }
    }
    file.Close();
    out << "--- end tdms layer ---\n";
    return out.str();
}

// The samples one drain produced, plus a block-by-block record of how the
// controller behaved while producing them.
struct DrainTrace {
    std::vector<std::int16_t> samples;
    std::string report;
};

// Drains the controller through its public interface and returns every int16
// sample it produced for channel 1, in order, without the zero fill of the last
// block.
inline auto DrainChannel1Traced(CReaderController& controller, std::uint32_t blockSize, int maxBlocks = 64) -> DrainTrace {
    DrainTrace trace;
    std::ostringstream out;

    dac_channels_t channels = {};
    controller.getChannels(channels);
    std::size_t ch1Size = 0;
    std::size_t ch2Size = 0;
    controller.getChannelsSize(&ch1Size, &ch2Size);
    out << "\n--- drain: blockSize=" << blockSize << " isOpen=" << OpenResultName(controller.isOpen())
        << " present{ch1=" << channels[DACChannels::DAC_CH1] << " ch2=" << channels[DACChannels::DAC_CH2] << "}"
        << " declared{ch1=" << ch1Size << " ch2=" << ch2Size << "} ---\n";

    std::vector<std::uint8_t> block1(blockSize);
    std::vector<std::uint8_t> block2(blockSize);
    for (int block = 0; block < maxBlocks; block++) {
        CReaderController::Data data;
        const auto result = controller.readBlock({block1.data(), block2.data()}, data);
        out << "  block " << block << ": " << ResultName(result) << " size0=" << data.size[0] << " realSize0=" << data.real_size[0]
            << " bits=" << static_cast<int>(data.bits[0]) << "\n";
        for (std::size_t at = 0; at + sizeof(std::int16_t) <= data.real_size[0]; at += sizeof(std::int16_t)) {
            std::int16_t value = 0;
            std::memcpy(&value, block1.data() + at, sizeof(value));
            trace.samples.push_back(value);
        }
        if (result != CReaderController::BR_OK) {
            break;
        }
    }
    out << "--- end drain: " << trace.samples.size() << " samples ---\n";
    trace.report = out.str();
    return trace;
}

}  // namespace reader_test
