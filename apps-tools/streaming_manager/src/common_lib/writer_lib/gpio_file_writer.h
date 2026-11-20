#ifndef WRITER_LIB_GPIO_FILE_WRITER_H
#define WRITER_LIB_GPIO_FILE_WRITER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bin_lib/bin_file.h"
#include "bin_lib/gpio_words.h"
#include "data_lib/signal.hpp"
#include "settings_lib/stream_settings.h"
#include "writer_lib/time_limit_clock.h"

class FileQueueManager;
class CFileLogger;

// What the GPIO input records
struct GpioRecording {
    CStreamSettings::GPIOFormat format = CStreamSettings::GPIOFormat::BIN;
    bool rle = false;
    uint64_t clock = 0;  // Hz before the decimation
    uint32_t decimation = 1;
    bool triggerMode = false;
    bool waitCounter = false;
    int64_t startTime = 0;  // UTC ns
    uint64_t samplesLimit = 0;  // 0 - until the stop
    // besides the sample limit: the time line, the data; the parts (streaming_improvements 4.11)
    CStreamSettings::FileLimits limits;
    double bufferSeconds = 0;  // about how long a buffer lasts at most (the time limit on the clock), 0 - unknown
};

// The GPIO input into a file on the file thread of writer_lib, buffer by buffer:
//   BIN - the BIN stream file (bin_lib): the input words as they come, a block per buffer;
//   CSV - "sample,p,n": a row where the lines change, the levels in hex; "-" where samples were
//         lost or waited for a trigger;
//   VCD - the value change dump of p[7:0] and n[7:0], in ps; x where samples are missing.
// The sample limit is taken at a buffer: the buffer that reaches it is the last one in the file,
// so the file may run past the limit by up to a buffer; so is the time limit. A size limit keeps
// out the buffer that would go over it. The report next to the file (<file>.log.txt, a line per
// buffer in <file>.log.lost.txt) gives the limit and the time line recorded.
// Parts (limits.splitSize or splitTime): <name>_0001.<ext> ..., each a whole file with its head
// and its report, its samples from 0, the time of its first sample in a BIN header.
class CGpioFileWriter {
   public:
    using Ptr = std::shared_ptr<CGpioFileWriter>;

    enum class Result { OK, LIMIT, FULL, ERROR };

    explicit CGpioFileWriter(bool testMode = false);
    ~CGpioFileWriter();

    auto open(const std::string& path, const GpioRecording& recording) -> bool;
    // One buffer of input words, after lost + wait missing samples: OK; LIMIT when the samples
    // reached the limit (this buffer is the last one taken); FULL when the disk is full; ERROR
    auto write(const uint32_t* words, size_t count, uint64_t lost, uint64_t wait, uint32_t triggerCount) -> Result;
    // The rest goes to the file, then it closes
    auto close() -> void;
    // While no buffer comes (a trigger waited for): true once the time limit passed on the clock,
    // a buffer and a margin with it (TimeLimitClock); the recording is then done, as at LIMIT
    auto timeUp() -> bool;
    // the tests: the margin of that clock
    auto setTimeMargin(double seconds) -> void { m_clock.setMargin(seconds); }

    // the time line written so far (samples, lost and wait), of all the parts
    auto samples() const -> uint64_t { return m_total; }
    // the part written now, from 1; path() is its file
    auto part() const -> int { return m_part; }
    // bytes of the part written now
    auto partBytes() -> uint64_t;
    auto samplesLimit() const -> uint64_t { return m_rec.samplesLimit; }
    auto path() const -> const std::string& { return m_path; }
    // while open: the bytes written, the write speed of the card (0 before it is measured), the
    // fill of the write queue 0..1
    auto writtenBytes() -> uint64_t;
    auto writeRate() -> double;
    auto queueFill() -> double;

    // the disk filled up while writing
    sigslot::signal<> outOfSpaceNotify;

   private:
    auto queue(std::vector<uint8_t>&& bytes) -> bool;
    auto text(const std::string& s, std::vector<uint8_t>& out) -> void;
    auto runs(const uint32_t* words, size_t count, std::vector<uint8_t>& out) -> void;
    auto gap(uint64_t samples, std::vector<uint8_t>& out) -> void;
    // the file header (BIN with the time of the first sample of the part), CSV head, VCD head
    auto head(std::vector<uint8_t>& out) -> void;
    auto newLogger(const std::string& path) -> void;
    auto beginPart() -> void;
    auto nextPart(std::vector<uint8_t>&& head) -> bool;
    auto rate() const -> double;

    bool m_testMode;
    std::unique_ptr<FileQueueManager> m_file;
    std::shared_ptr<CFileLogger> m_logger;
    std::string m_path;  // the part written now
    std::string m_base;  // the name given to open()
    int m_part = 0;
    uint64_t m_total = 0;     // the time line of all the parts
    uint64_t m_partStart = 0; // where the part starts on it
    uint64_t m_allBytes = 0;
    uint64_t m_partBytes = 0;
    GpioRecording m_rec;
    bool m_open = false;
    bool m_limited = false;
    uint64_t m_position = 0;  // the time line of the part in samples: the next sample
    bin_lib::Timeline m_line;
    bin_lib::GpioInDecoder m_decoder{false};
    bool m_haveLevels = false;  // CSV, VCD: the levels before
    uint8_t m_p = 0;
    uint8_t m_n = 0;
    uint64_t m_periodPs = 0;  // VCD
    TimeLimitClock m_clock;
};

#endif
