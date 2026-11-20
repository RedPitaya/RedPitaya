#include "gpio_file_writer.h"

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <filesystem>

#include "file_helper.h"
#include "file_queue_manager.h"
#include "logger_lib/file_logger.h"

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

auto CGpioFileWriter::rate() const -> double {
    return m_rec.clock ? static_cast<double>(m_rec.clock) / std::max<uint32_t>(1, m_rec.decimation) : 0;
}

auto CGpioFileWriter::newLogger(const std::string& path) -> void {
    m_logger = CFileLogger::create(path + ".log", m_testMode);
    CFileLogger::GpioProperties properties;
    properties.baseRate = m_rec.clock;
    properties.decimation = m_rec.decimation;
    properties.rle = m_rec.rle;
    properties.triggerMode = m_rec.triggerMode;
    properties.waitCounter = m_rec.waitCounter;
    m_logger->setGpioProperties(properties);
    m_logger->setGpioSamplesLimit(m_rec.samplesLimit);
}

auto CGpioFileWriter::head(std::vector<uint8_t>& out) -> void {
    switch (m_rec.format.value) {
        case CStreamSettings::GPIOFormat::BIN: {
            bin_lib::FileInfo info;
            info.source = bin_lib::Source::GPIO_IN;
            info.clock = m_rec.clock;
            info.decimation = m_rec.decimation;
            // a part: the time of its first sample
            const double r = rate();
            info.startTime = m_rec.startTime && r > 0 ? m_rec.startTime + static_cast<int64_t>(std::llround(static_cast<double>(m_partStart) / r * 1e9)) : m_rec.startTime;
            if (m_rec.triggerMode)
                info.flags |= bin_lib::FILE_FLAG_TRIGGER_MODE;
            if (m_rec.waitCounter)
                info.flags |= bin_lib::FILE_FLAG_WAIT_COUNTER;
            info.channels = {{1, bin_lib::SampleType::GPIO_IN_WORD, static_cast<uint16_t>(m_rec.rle ? bin_lib::CHANNEL_FLAG_RLE : 0), 0}};
            out = bin_lib::fileHeader(info);
            break;
        }
        case CStreamSettings::GPIOFormat::CSV:
            text("sample,p,n\n", out);
            break;
        case CStreamSettings::GPIOFormat::VCD:
            text("$comment Red Pitaya GPIO input, DIO0..7 P and N $end\n"
                 "$timescale 1 ps $end\n"
                 "$scope module gpio $end\n"
                 "$var wire 8 p p[7:0] $end\n"
                 "$var wire 8 n n[7:0] $end\n"
                 "$upscope $end\n"
                 "$enddefinitions $end\n",
                 out);
            break;
    }
}

auto CGpioFileWriter::open(const std::string& path, const GpioRecording& recording) -> bool {
    close();
    m_rec = recording;
    m_base = path;
    m_part = 1;
    m_total = m_partStart = m_allBytes = m_partBytes = 0;
    const bool parts = !m_testMode && (m_rec.limits.splitSize || m_rec.limits.splitTime > 0);
    m_path = parts ? partFileName(path, 1) : path;
    m_limited = false;
    m_clock.start(m_rec.limits.time, m_rec.bufferSeconds);
    m_position = 0;
    m_line.reset();
    m_decoder = bin_lib::GpioInDecoder(recording.rle);
    m_haveLevels = false;
    m_periodPs = recording.clock ? static_cast<uint64_t>(std::llround(1e12 * recording.decimation / static_cast<double>(recording.clock))) : 1;

    m_file = std::make_unique<FileQueueManager>(m_testMode);
    m_file->outSpaceNotify.connect([this]() { outOfSpaceNotify(); });
    m_file->openFile(m_path, false);
    if (!std::filesystem::exists(m_path)) {
        m_file.reset();
        return false;
    }
    m_file->startWrite(CStreamSettings::DataFormat::BIN);
    m_open = true;
    newLogger(m_path);
    std::vector<uint8_t> h;
    head(h);
    m_allBytes += h.size();
    return queue(std::move(h));
}

// The part ends after what is queued, the next one starts with its head and its report; the
// samples of a part count from 0. The decoder goes on: the runs of the lanes do across the parts
auto CGpioFileWriter::beginPart() -> void {
    m_partStart = m_total;
    m_position = 0;
    m_line.reset();
    m_haveLevels = false;
}

auto CGpioFileWriter::nextPart(std::vector<uint8_t>&& h) -> bool {
    m_part++;
    m_path = partFileName(m_base, m_part);
    if (m_logger) {
        m_logger->setFileWrite(m_file->writeRate(), m_file->queueLimit());
        if (!m_testMode)
            m_logger->dumpToFile();
    }
    newLogger(m_path);
    if (!m_file->addFileSwitch(m_path))
        return false;
    m_partBytes = 0;
    m_allBytes += h.size();
    return queue(std::move(h));
}

auto CGpioFileWriter::write(const uint32_t* words, size_t count, uint64_t lost, uint64_t wait, uint32_t triggerCount) -> Result {
    if (!m_open)
        return Result::ERROR;
    if (m_limited)
        return Result::LIMIT;
    if (m_file->isOutOfSpace())
        return Result::FULL;
    const auto& lim = m_rec.limits;
    const double r = rate();
    // the time limit: a buffer whose data starts past it (after a wait for the trigger) stays out
    if (lim.time > 0 && r > 0 && static_cast<double>(m_total + lost + wait) >= lim.time * r) {
        m_limited = true;
        return Result::LIMIT;
    }
    const uint64_t samples = bin_lib::gpioInSamples(words, count, m_rec.rle);
    if (r > 0)
        m_clock.buffer(static_cast<double>(samples) / r);
    // the next part: the data of the part, or its time line (the buffer before reached it)
    bool split = false;
    if (!m_testMode && m_partBytes > 0) {
        const bool bySize = lim.splitSize && m_partBytes + count * 4 + 64 > lim.splitSize;
        const bool byTime = lim.splitTime > 0 && r > 0 && static_cast<double>(m_total - m_partStart) >= lim.splitTime * r;
        split = bySize || byTime;
    }
    // the positions of the new part, the head of its file once the buffer is taken
    if (split)
        beginPart();
    std::vector<uint8_t> h;
    if (split)
        head(h);
    std::vector<uint8_t> out;
    const uint64_t before = m_position;
    if (m_rec.format.value == CStreamSettings::GPIOFormat::BIN) {
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
    // the size limit (the heads too): the buffer that would go over it stays out, and no part
    // of a head only is begun for it
    if (lim.size && m_allBytes + h.size() + out.size() > lim.size) {
        m_position = before;
        m_limited = true;
        return Result::LIMIT;
    }
    if (split && !nextPart(std::move(h)))
        return m_file->isOutOfSpace() ? Result::FULL : Result::ERROR;
    m_allBytes += out.size();
    m_partBytes += out.size();
    if (!queue(std::move(out)))
        return m_file->isOutOfSpace() ? Result::FULL : Result::ERROR;
    m_total += m_position - before;
    m_logger->addMetric(CFileLogger::EMetric::RECIVE_DATE, count * 4);
    m_logger->addGpioMetric(count * 4, count, samples, lost, wait, triggerCount);
    if ((m_rec.samplesLimit && m_total >= m_rec.samplesLimit) || (lim.time > 0 && r > 0 && static_cast<double>(m_total) >= lim.time * r)) {
        m_limited = true;
        return Result::LIMIT;
    }
    return Result::OK;
}

auto CGpioFileWriter::timeUp() -> bool {
    if (!m_open || m_limited || !m_clock.due())
        return false;
    m_limited = true;
    return true;
}

auto CGpioFileWriter::partBytes() -> uint64_t {
    return m_file ? m_file->fileSize() : 0;
}

auto CGpioFileWriter::writtenBytes() -> uint64_t {
    return m_file ? m_file->getWritedSize() : 0;
}

auto CGpioFileWriter::writeRate() -> double {
    return m_file ? m_file->writeRate() : 0;
}

auto CGpioFileWriter::queueFill() -> double {
    return m_file ? m_file->queueFill() : 0;
}

auto CGpioFileWriter::close() -> void {
    if (!m_open)
        return;
    m_open = false;
    m_file->stopWrite(true);
    m_file->closeFile();
    if (m_logger)
        m_logger->setFileWrite(m_file->writeRate(), m_file->queueLimit());
    m_file.reset();
    if (m_logger && !m_testMode)
        m_logger->dumpToFile();
    m_logger.reset();
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
