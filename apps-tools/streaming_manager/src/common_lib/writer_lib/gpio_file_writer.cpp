#include "gpio_file_writer.h"

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <filesystem>

#include "file_queue_manager.h"

namespace {

auto bits(uint8_t v) -> std::string {
    std::string s(8, '0');
    for (int i = 0; i < 8; i++)
        if (v & (0x80 >> i))
            s[i] = '1';
    return s;
}

}  // namespace

CGpioFileWriter::CGpioFileWriter(bool testMode) : m_testMode(testMode) {}

CGpioFileWriter::~CGpioFileWriter() {
    close();
}

auto CGpioFileWriter::open(const std::string& path, const GpioRecording& recording) -> bool {
    close();
    m_rec = recording;
    m_path = path;
    m_limited = false;
    m_position = 0;
    m_line.reset();
    m_decoder = bin_lib::GpioInDecoder(recording.rle);
    m_haveLevels = false;
    m_periodPs = recording.clock ? static_cast<uint64_t>(std::llround(1e12 * recording.decimation / static_cast<double>(recording.clock))) : 1;

    m_file = std::make_unique<FileQueueManager>(m_testMode);
    m_file->outSpaceNotify.connect([this]() { outOfSpaceNotify(); });
    m_file->openFile(path, false);
    if (!std::filesystem::exists(path)) {
        m_file.reset();
        return false;
    }
    m_file->startWrite(CStreamSettings::DataFormat::BIN);
    m_open = true;

    std::vector<uint8_t> head;
    switch (m_rec.format.value) {
        case CStreamSettings::GPIOFormat::BIN: {
            bin_lib::FileInfo info;
            info.source = bin_lib::Source::GPIO_IN;
            info.clock = m_rec.clock;
            info.decimation = m_rec.decimation;
            info.startTime = m_rec.startTime;
            if (m_rec.triggerMode)
                info.flags |= bin_lib::FILE_FLAG_TRIGGER_MODE;
            if (m_rec.waitCounter)
                info.flags |= bin_lib::FILE_FLAG_WAIT_COUNTER;
            info.channels = {{1, bin_lib::SampleType::GPIO_IN_WORD, static_cast<uint16_t>(m_rec.rle ? bin_lib::CHANNEL_FLAG_RLE : 0), 0}};
            head = bin_lib::fileHeader(info);
            break;
        }
        case CStreamSettings::GPIOFormat::CSV:
            text("sample,p,n\n", head);
            break;
        case CStreamSettings::GPIOFormat::VCD:
            text("$comment Red Pitaya GPIO input, DIO0..7 P and N $end\n"
                 "$timescale 1 ps $end\n"
                 "$scope module gpio $end\n"
                 "$var wire 8 p p[7:0] $end\n"
                 "$var wire 8 n n[7:0] $end\n"
                 "$upscope $end\n"
                 "$enddefinitions $end\n",
                 head);
            break;
    }
    return queue(std::move(head));
}

auto CGpioFileWriter::write(const uint32_t* words, size_t count, uint64_t lost, uint64_t wait, uint32_t triggerCount) -> Result {
    if (!m_open)
        return Result::ERROR;
    if (m_limited)
        return Result::LIMIT;
    if (m_file->isOutOfSpace())
        return Result::FULL;
    std::vector<uint8_t> out;
    if (m_rec.format.value == CStreamSettings::GPIOFormat::BIN) {
        const uint64_t samples = bin_lib::gpioInSamples(words, count, m_rec.rle);
        bin_lib::BlockInfo block;
        block.triggerCount = triggerCount;
        block.samples = samples;
        block.lost = lost;
        block.wait = wait;
        block.position = m_line.next(0, lost, wait, samples);
        bin_lib::appendBlock(out, block, words, static_cast<uint32_t>(count * 4));
        m_position = block.position + samples;
    } else {
        if (lost + wait)
            gap(lost + wait, out);
        runs(words, count, out);
    }
    if (!queue(std::move(out)))
        return m_file->isOutOfSpace() ? Result::FULL : Result::ERROR;
    if (m_rec.samplesLimit && m_position >= m_rec.samplesLimit) {
        m_limited = true;
        return Result::LIMIT;
    }
    return Result::OK;
}

auto CGpioFileWriter::close() -> void {
    if (!m_open)
        return;
    m_open = false;
    m_file->stopWrite(true);
    m_file->closeFile();
    m_file.reset();
}

auto CGpioFileWriter::queue(std::vector<uint8_t>&& bytes) -> bool {
    if (bytes.empty())
        return true;
    auto chunk = std::make_unique<FileChunk>();
    chunk->add(std::move(bytes));
    return m_file->addChunkToWrite(std::move(chunk));
}

auto CGpioFileWriter::text(const std::string& s, std::vector<uint8_t>& out) -> void {
    out.insert(out.end(), s.begin(), s.end());
}

// The runs of the words; a row (CSV) or a change (VCD) where the levels change
auto CGpioFileWriter::runs(const uint32_t* words, size_t count, std::vector<uint8_t>& out) -> void {
    char line[96];
    m_decoder.feed(words, count, [&](const bin_lib::GpioRun& r) {
        const bool pChanged = !m_haveLevels || r.p != m_p;
        const bool nChanged = !m_haveLevels || r.n != m_n;
        if (pChanged || nChanged) {
            if (m_rec.format.value == CStreamSettings::GPIOFormat::CSV) {
                snprintf(line, sizeof(line), "%" PRIu64 ",0x%02X,0x%02X\n", m_position, r.p, r.n);
                text(line, out);
            } else {
                snprintf(line, sizeof(line), "#%" PRIu64 "\n", m_position * m_periodPs);
                text(line, out);
                if (pChanged)
                    text("b" + bits(r.p) + " p\n", out);
                if (nChanged)
                    text("b" + bits(r.n) + " n\n", out);
            }
            m_p = r.p;
            m_n = r.n;
            m_haveLevels = true;
        }
        m_position += r.samples;
    });
}

// Samples missing before a buffer: the lanes start again after it
auto CGpioFileWriter::gap(uint64_t samples, std::vector<uint8_t>& out) -> void {
    char line[64];
    m_decoder.reset();
    if (m_rec.format.value == CStreamSettings::GPIOFormat::CSV) {
        snprintf(line, sizeof(line), "%" PRIu64 ",-,-\n", m_position);
        text(line, out);
    } else {
        snprintf(line, sizeof(line), "#%" PRIu64 "\nbxxxxxxxx p\nbxxxxxxxx n\n", m_position * m_periodPs);
        text(line, out);
    }
    m_haveLevels = false;
    m_position += samples;
}
