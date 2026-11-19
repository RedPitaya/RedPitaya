#ifndef VIEW_SOURCE_H
#define VIEW_SOURCE_H

#include <QDataStream>
#include <QString>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

// The samples of a recording for the viewer (streaming_improvements 4.7), whatever its format:
// BIN, WAV, TDMS or CSV. Positions are samples on the time line of the recording: the lost and
// the waited samples stand on it too, read as zeros (the viewer leaves them out by the gaps).
// Values are a fraction of the full scale (-1..1) for codes, volts for volts. Qt Core only: the
// tests build it alone. read() and readAll() may be called from several threads.
namespace view {

struct Gap {
    uint64_t at = 0;  // the first missing sample
    uint64_t lost = 0;
    uint64_t wait = 0;  // after the lost ones
    auto end() const -> uint64_t { return at + lost + wait; }
};

struct Info {
    QString kind;              // BIN, WAV, TDMS, CSV
    std::vector<int> ids;      // the inputs (1 = IN1)
    bool volts = false;        // the values are volts
    int bits = 0;              // of the codes
    double rate = 0;           // samples a second, 0: not known (the time is the sample count)
    double clock = 0;          // of the ADC
    double decimation = 0;
    uint64_t total = 0;        // samples on the time line
    double startNs = 0;        // the time of the first sample (ns since 1970), 0: not known
    QString gapsFrom;          // the file, the report beside it, the rows; empty: none known
};

// 0..1 done; false stops
using Progress = std::function<bool(double)>;

class Source {
   public:
    virtual ~Source() = default;
    // by the suffix: bin, wav, tdms, csv; null for another
    static auto create(const QString& path) -> std::unique_ptr<Source>;
    static auto viewable(const QString& path) -> bool;

    // reads what the file is (a CSV: all of it once); false with error()
    virtual auto open(const QString& path, const Progress& progress) -> bool = 0;
    virtual auto close() -> void = 0;
    auto info() const -> const Info& { return m_info; }
    auto gaps() const -> const std::vector<Gap>& { return m_gaps; }
    auto error() const -> const QString& { return m_error; }

    // n values of every input from position from (out: one vector an input, resized); how many
    virtual auto readAll(uint64_t from, uint64_t n, std::vector<std::vector<float>>& out) -> uint64_t = 0;
    // a window of more samples than this is drawn from the overview, not read
    virtual auto directLimit() const -> uint64_t { return 2000000; }
    // min and max of `buckets` parts of the whole time line an input (the gaps left out); a
    // source that knows them already (a CSV) gives them at once; false: stopped
    virtual auto overview(int buckets, std::vector<std::vector<float>>& lo, std::vector<std::vector<float>>& hi, const Progress& progress) -> bool;
    // a source slow to open (a CSV is read whole) keeps what open() found in the cache of the
    // application: open again from it
    virtual auto saveIndex(QDataStream& out) const -> void { Q_UNUSED(out) }
    virtual auto loadIndex(const QString& path, QDataStream& in) -> bool {
        Q_UNUSED(path)
        Q_UNUSED(in)
        return false;
    }

   protected:
    Info m_info;
    std::vector<Gap> m_gaps;
    QString m_error;
    std::mutex m_mtx;  // a reader of a file is one position
};

// the gaps and the time line of a WAV or a TDMS from the report beside it (<file>.log.lost.txt):
// a line a buffer, Pos (its gaps come first), Get, (lost), Wait. Empty when there is none.
struct Report {
    std::vector<Gap> gaps;
    std::vector<uint64_t> gets;  // the samples of the buffer after each gap
    uint64_t data = 0;   // the samples got
    uint64_t total = 0;  // with the gaps
    double rate = 0;     // from <file>.log.txt
    double clock = 0;
    double decimation = 0;
};
auto readReport(const QString& path) -> Report;

// The positions of a WAV or a TDMS on the time line. The writer puts zeros for the lost samples
// (on the SD card of a board no more than the buffer after them) and nothing for the trigger
// waits; a converted BIN may have all of a gap as zeros. Which one: the count of the file.
class Mapping {
   public:
    // gets: the samples of the buffer after each gap; data: all the samples got
    auto set(const std::vector<Gap>& gaps, const std::vector<uint64_t>& gets, uint64_t fileSamples, uint64_t total, uint64_t data) -> void;
    // the runs of [from, from + n) that are in the file: (line position, file sample, count)
    struct Run {
        uint64_t at;
        uint64_t file;
        uint64_t count;
    };
    auto runs(uint64_t from, uint64_t n) const -> std::vector<Run>;
    auto identity() const -> bool { return m_identity; }

   private:
    bool m_identity = true;
    uint64_t m_total = 0;
    std::vector<Run> m_pieces;
};

}  // namespace view

#endif
