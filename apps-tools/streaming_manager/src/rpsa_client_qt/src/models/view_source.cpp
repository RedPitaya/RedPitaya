#include "view_source.h"
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstring>
#include <limits>
#include "bin_lib/bin_file.h"
#include "tdms_lib/tdms.h"
#include "wav_lib/wav_reader.h"

namespace view {

namespace {

constexpr uint64_t kChunk = 1 << 20;  // samples a channel read at a time for the overview

auto toFloat(const uint8_t* raw, uint64_t i, int bytes, bool isFloat, bool unsignedByte) -> float {
    if (isFloat) {
        if (bytes == 8) {
            double d;
            std::memcpy(&d, raw + i * 8, 8);
            return static_cast<float>(d);
        }
        float f;
        std::memcpy(&f, raw + i * 4, 4);
        return f;
    }
    switch (bytes) {
        case 1:
            return unsignedByte ? (static_cast<int>(raw[i]) - 128) / 128.0f : static_cast<int8_t>(raw[i]) / 128.0f;
        case 2: {
            int16_t v;
            std::memcpy(&v, raw + i * 2, 2);
            return v / 32768.0f;
        }
        case 3: {
            const int32_t v = (static_cast<int32_t>(raw[i * 3]) | (static_cast<int32_t>(raw[i * 3 + 1]) << 8) | (static_cast<int32_t>(static_cast<int8_t>(raw[i * 3 + 2])) << 16));
            return v / 8388608.0f;
        }
        case 4: {
            int32_t v;
            std::memcpy(&v, raw + i * 4, 4);
            return static_cast<float>(v / 2147483648.0);
        }
        default:
            return 0;
    }
}

// ---- BIN

class BinSource : public Source {
   public:
    auto open(const QString& path, const Progress& progress) -> bool override {
        m_reader.setProgress(progress);
        m_size = std::max<double>(1, static_cast<double>(QFileInfo(path).size()));
        const bool opened = m_reader.open(path.toStdString());
        // the callback holds what the caller had: not kept past the open
        m_reader.setProgress({});
        if (!opened) {
            m_error = QString::fromStdString(m_reader.error());
            return false;
        }
        const auto& fi = m_reader.info();
        for (size_t i = 0; i < fi.channels.size(); i++) {
            const auto t = fi.channels[i].type;
            if (t == bin_lib::SampleType::GPIO_IN_WORD || t == bin_lib::SampleType::GPIO_OUT_WORD || bin_lib::sampleBytes(t) == 0)
                continue;
            m_adc.push_back(static_cast<uint8_t>(i));
            m_info.ids.push_back(fi.channels[i].id);
            m_info.bits = std::max(m_info.bits, static_cast<int>(bin_lib::sampleBytes(t) * 8));
            m_info.volts |= t == bin_lib::SampleType::FLOAT32;
        }
        if (m_adc.empty()) {
            m_error = "No ADC channel in this recording (a GPIO recording: convert it to VCD)";
            return false;
        }
        if (m_info.volts)
            m_info.bits = 0;
        m_info.kind = "BIN";
        m_info.clock = static_cast<double>(fi.clock);
        m_info.decimation = fi.decimation ? fi.decimation : 1;
        m_info.rate = fi.clock ? m_info.clock / m_info.decimation : 0;
        m_info.startNs = static_cast<double>(fi.startTime);
        m_info.total = m_reader.samples(m_adc[0], true);
        for (const auto& g : m_reader.gaps(m_adc[0]))
            m_gaps.push_back(Gap{g.at, g.lost, g.wait});
        m_info.gapsFrom = "the file";
        return true;
    }
    auto close() -> void override {
        std::lock_guard lock(m_mtx);
        m_reader.close();
    }
    auto readAll(uint64_t from, uint64_t n, std::vector<std::vector<float>>& out) -> uint64_t override {
        std::lock_guard lock(m_mtx);
        out.resize(m_adc.size());
        uint64_t got = n;
        const auto& fi = m_reader.info();
        for (size_t k = 0; k < m_adc.size(); k++) {
            const auto t = fi.channels[m_adc[k]].type;
            const int bytes = static_cast<int>(bin_lib::sampleBytes(t));
            m_raw.resize(n * bytes);
            const uint64_t g = m_reader.read(m_adc[k], from, n, m_raw.data(), true);
            got = std::min(got, g);
            out[k].resize(g);
            const bool isFloat = t == bin_lib::SampleType::FLOAT32;
            for (uint64_t i = 0; i < g; i++)
                out[k][i] = toFloat(m_raw.data(), i, bytes, isFloat, false);
        }
        return got;
    }

    // the blocks one after the other (the file read once in its order), min and max of the codes
    // of a bucket at a time, made values once a bucket
    auto overview(int buckets, std::vector<std::vector<float>>& lo, std::vector<std::vector<float>>& hi, const Progress& progress) -> bool override {
        const size_t nc = m_adc.size();
        lo.assign(nc, std::vector<float>(buckets, std::numeric_limits<float>::max()));
        hi.assign(nc, std::vector<float>(buckets, std::numeric_limits<float>::lowest()));
        const uint64_t total = m_info.total;
        if (total == 0)
            return true;
        const uint64_t B = static_cast<uint64_t>(buckets);
        const auto& fi = m_reader.info();
        std::vector<int> slot(fi.channels.size(), -1);
        for (size_t k = 0; k < nc; k++)
            slot[m_adc[k]] = static_cast<int>(k);
        {
            std::lock_guard lock(m_mtx);
            m_reader.rewind();
        }
        bin_lib::BlockInfo b;
        std::vector<uint8_t> data;
        double done = 0;
        int tick = 0;
        while (true) {
            {
                std::lock_guard lock(m_mtx);
                if (!m_reader.nextBlock(b, data))
                    break;
            }
            done += static_cast<double>(bin_lib::blockSize(b.dataSize));
            if ((++tick & 63) == 0 && !progress(done / m_size))
                return false;
            const int k = slot[b.channel];
            if (k < 0)
                continue;
            const auto type = fi.channels[b.channel].type;
            // no more samples than the data has (a damaged header)
            const uint64_t samples = std::min<uint64_t>(b.samples, b.dataSize / std::max<uint32_t>(1, bin_lib::sampleBytes(type)));
            uint64_t s = b.position;
            const uint64_t e = std::min(total, b.position + samples);
            while (s < e) {
                const uint64_t bk = s * B / total;
                const uint64_t next = std::min(e, ((bk + 1) * total + B - 1) / B);
                const uint64_t i0 = s - b.position, i1 = next - b.position;
                float l, h;
                if (type == bin_lib::SampleType::INT16) {
                    const auto* v = reinterpret_cast<const int16_t*>(data.data());
                    int mn = 32767, mx = -32768;
                    for (uint64_t i = i0; i < i1; i++) {
                        mn = std::min<int>(mn, v[i]);
                        mx = std::max<int>(mx, v[i]);
                    }
                    l = mn / 32768.0f;
                    h = mx / 32768.0f;
                } else if (type == bin_lib::SampleType::INT8) {
                    const auto* v = reinterpret_cast<const int8_t*>(data.data());
                    int mn = 127, mx = -128;
                    for (uint64_t i = i0; i < i1; i++) {
                        mn = std::min<int>(mn, v[i]);
                        mx = std::max<int>(mx, v[i]);
                    }
                    l = mn / 128.0f;
                    h = mx / 128.0f;
                } else {
                    const auto* v = reinterpret_cast<const float*>(data.data());
                    l = std::numeric_limits<float>::max();
                    h = std::numeric_limits<float>::lowest();
                    for (uint64_t i = i0; i < i1; i++) {
                        l = std::min(l, v[i]);
                        h = std::max(h, v[i]);
                    }
                }
                lo[k][bk] = std::min(lo[k][bk], l);
                hi[k][bk] = std::max(hi[k][bk], h);
                s = next;
            }
        }
        return progress(1);
    }

   private:
    bin_lib::FileReader m_reader;
    std::vector<uint8_t> m_adc;
    std::vector<uint8_t> m_raw;
    double m_size = 1;
};

// ---- WAV

class WavSource : public Source {
   public:
    auto open(const QString& path, const Progress& progress) -> bool override {
        m_wav = std::make_unique<CWaveReader>();
        if (!m_wav->openFile(path.toStdString())) {
            m_error = QString::fromStdString(m_wav->error());
            return false;
        }
        if (m_wav->format() == CWaveReader::Format::OTHER || m_wav->channels() == 0) {
            m_error = "A WAV of a format the viewer does not read";
            return false;
        }
        m_info.kind = "WAV";
        m_info.volts = m_wav->format() == CWaveReader::Format::FLOAT;
        m_info.bits = m_info.volts ? 0 : m_wav->bitsPerSample();
        for (int i = 1; i <= m_wav->channels(); i++)
            m_info.ids.push_back(i);
        const uint64_t frames = m_wav->frames();
        const Report rep = readReport(path);
        m_info.rate = rep.rate > 0 ? rep.rate : m_wav->sampleRate();
        m_info.clock = rep.clock;
        m_info.decimation = rep.decimation;
        if (rep.total > 0) {
            m_gaps = rep.gaps;
            m_map.set(m_gaps, rep.gets, frames, rep.total, rep.data);
            m_info.total = rep.total;
            m_info.gapsFrom = "the report beside it";
        } else {
            m_map.set({}, {}, frames, frames, frames);
            m_info.total = frames;
        }
        progress(1);
        return true;
    }
    // the reader goes: Windows deletes no file that is open
    auto close() -> void override {
        std::lock_guard lock(m_mtx);
        m_wav.reset();
    }
    auto readAll(uint64_t from, uint64_t n, std::vector<std::vector<float>>& out) -> uint64_t override {
        std::lock_guard lock(m_mtx);
        if (!m_wav)
            return 0;
        n = from < m_info.total ? std::min(n, m_info.total - from) : 0;
        const int chs = m_wav->channels();
        out.resize(chs);
        for (auto& o : out)
            o.assign(n, 0.0f);
        const int bytes = m_wav->bitsPerSample() / 8;
        const bool isFloat = m_info.volts;
        for (const auto& run : m_map.runs(from, n)) {
            m_raw.resize(run.count * m_wav->frameSize());
            const uint64_t got = m_wav->readFrames(run.file, run.count, m_raw.data());
            for (uint64_t i = 0; i < got; i++)
                for (int c = 0; c < chs; c++)
                    out[c][run.at - from + i] = toFloat(m_raw.data(), i * chs + c, bytes, isFloat, bytes == 1);
        }
        return n;
    }

   private:
    std::unique_ptr<CWaveReader> m_wav;
    Mapping m_map;
    std::vector<uint8_t> m_raw;
};

// ---- TDMS

class TdmsSource : public Source {
   public:
    auto open(const QString& path, const Progress& progress) -> bool override {
        if (!m_tdms.open(path.toStdString())) {
            m_error = QString::fromStdString(m_tdms.error());
            return false;
        }
        static const QRegularExpression chName("(\\d+)$");
        uint64_t count = std::numeric_limits<uint64_t>::max();
        std::string timeGroup;
        for (const auto& c : m_tdms.channels()) {
            const int bytes = sizeOf(c.type);
            if (c.name == "time") {
                if (c.type == TDMS::TDMSType::Integer64)
                    timeGroup = c.group;
                continue;
            }
            if (bytes == 0 || c.count == 0)
                continue;
            const auto m = chName.match(QString::fromStdString(c.name));
            m_chs.push_back(Ch{c.group, c.name, c.type});
            m_info.ids.push_back(m.hasMatch() ? m.captured(1).toInt() : static_cast<int>(m_chs.size()));
            m_info.volts |= c.type == TDMS::TDMSType::SingleFloat || c.type == TDMS::TDMSType::DoubleFloat;
            m_info.bits = std::max(m_info.bits, bytes * 8);
            count = std::min(count, c.count);
        }
        if (m_chs.empty()) {
            m_error = "No channel of samples in this TDMS";
            return false;
        }
        if (m_info.volts)
            m_info.bits = 0;
        m_info.kind = "TDMS";
        const Report rep = readReport(path);
        m_info.rate = rep.rate;
        m_info.clock = rep.clock;
        m_info.decimation = rep.decimation;
        // the time column (ns): the start, and the rate when there is no report
        if (!timeGroup.empty()) {
            const auto t = m_tdms.read<int64_t>(timeGroup, "time", 0, 2);
            if (!t.empty())
                m_info.startNs = static_cast<double>(t[0]);
            if (m_info.rate <= 0 && t.size() == 2 && t[1] > t[0])
                m_info.rate = 1e9 / static_cast<double>(t[1] - t[0]);
        }
        if (rep.total > 0) {
            m_gaps = rep.gaps;
            m_map.set(m_gaps, rep.gets, count, rep.total, rep.data);
            m_info.total = rep.total;
            m_info.gapsFrom = "the report beside it";
        } else {
            m_map.set({}, {}, count, count, count);
            m_info.total = count;
        }
        progress(1);
        return true;
    }
    auto close() -> void override {
        std::lock_guard lock(m_mtx);
        m_tdms.close();
    }
    auto readAll(uint64_t from, uint64_t n, std::vector<std::vector<float>>& out) -> uint64_t override {
        std::lock_guard lock(m_mtx);
        n = from < m_info.total ? std::min(n, m_info.total - from) : 0;
        out.resize(m_chs.size());
        for (auto& o : out)
            o.assign(n, 0.0f);
        const auto runs = m_map.runs(from, n);
        for (size_t k = 0; k < m_chs.size(); k++) {
            const auto& c = m_chs[k];
            const int bytes = sizeOf(c.type);
            const bool isFloat = c.type == TDMS::TDMSType::SingleFloat || c.type == TDMS::TDMSType::DoubleFloat;
            for (const auto& run : runs) {
                m_raw.clear();  // read() appends
                const uint64_t got = m_tdms.read(c.group, c.name, run.file, run.count, m_raw);
                for (uint64_t i = 0; i < got; i++)
                    out[k][run.at - from + i] = toFloat(m_raw.data(), i, bytes, isFloat, c.type == TDMS::TDMSType::UnsignedInteger8);
            }
        }
        return n;
    }

   private:
    static auto sizeOf(TDMS::TDMSType t) -> int {
        switch (t) {
            case TDMS::TDMSType::Integer8:
            case TDMS::TDMSType::UnsignedInteger8:
                return 1;
            case TDMS::TDMSType::Integer16:
                return 2;
            case TDMS::TDMSType::Integer32:
            case TDMS::TDMSType::SingleFloat:
                return 4;
            case TDMS::TDMSType::DoubleFloat:
                return 8;
            default:
                return 0;
        }
    }
    struct Ch {
        std::string group;
        std::string name;
        TDMS::TDMSType type;
    };
    TDMS::FileReader m_tdms;
    std::vector<Ch> m_chs;
    Mapping m_map;
    std::vector<uint8_t> m_raw;
};

// ---- CSV

// The text read once: where every K-th row starts, min and max of every K rows, the gaps (the
// rows without values, "-"); then a window is read by its rows
class CsvSource : public Source {
   public:
    static constexpr uint64_t K = 256;

    auto open(const QString& path, const Progress& progress) -> bool override {
        m_file.setFileName(path);
        if (!m_file.open(QIODevice::ReadOnly)) {
            m_error = m_file.errorString();
            return false;
        }
        const double size = std::max<double>(1, static_cast<double>(m_file.size()));
        // the head: the names of the columns, or the first row
        QByteArray first = m_file.readLine();
        const QList<QByteArray> cols = first.trimmed().split(',');
        const bool head = !first.isEmpty() && std::isalpha(static_cast<unsigned char>(first.at(0)));
        static const QRegularExpression chName("(\\d+)$");
        for (int c = 0; c < cols.size(); c++) {
            const QString name = QString::fromUtf8(cols[c]).trimmed().toLower();
            if (head && (name == "index" || name.startsWith("time"))) {
                if (name.startsWith("time"))
                    m_timeCol = c;
                continue;
            }
            const auto m = chName.match(name);
            m_cols.push_back(c);
            m_info.ids.push_back(head && m.hasMatch() ? m.captured(1).toInt() : static_cast<int>(m_cols.size()));
        }
        if (m_cols.empty()) {
            m_error = "No column of values in this CSV";
            return false;
        }
        m_dataStart = head ? static_cast<uint64_t>(first.size()) : 0;
        m_file.seek(static_cast<qint64>(m_dataStart));
        const size_t nc = m_cols.size();
        std::vector<double> lo(nc, std::numeric_limits<double>::max()), hi(nc, std::numeric_limits<double>::lowest());
        std::vector<double> vals(nc);
        uint64_t row = 0;
        uint64_t offset = m_dataStart;
        bool inGap = false;
        uint64_t gapAt = 0;
        bool isFloat = false;
        int64_t t0 = 0, t1 = 0;
        QByteArray carry;
        while (true) {
            QByteArray buf = carry + m_file.read(4 << 20);
            if (buf.size() == carry.size() && carry.isEmpty())
                break;
            const bool last = m_file.atEnd();
            const char* p = buf.constData();
            const char* end = p + buf.size();
            while (p < end) {
                const char* nl = static_cast<const char*>(std::memchr(p, '\n', static_cast<size_t>(end - p)));
                if (!nl && !last)
                    break;
                const char* le = nl ? nl : end;
                if (le > p && !(le - p == 1 && *p == '\r')) {
                    if (row % K == 0) {
                        m_offsets.push_back(offset);
                        if (row) {
                            m_lo.push_back(lo);
                            m_hi.push_back(hi);
                            std::fill(lo.begin(), lo.end(), std::numeric_limits<double>::max());
                            std::fill(hi.begin(), hi.end(), std::numeric_limits<double>::lowest());
                        }
                    }
                    bool gap = false;
                    int64_t t = 0;
                    parseRow(p, le, vals, gap, isFloat, row < 2 && m_timeCol >= 0 ? &t : nullptr);
                    if (row == 0)
                        t0 = t;
                    else if (row == 1)
                        t1 = t;
                    if (gap) {
                        if (!inGap) {
                            inGap = true;
                            gapAt = row;
                        }
                    } else {
                        if (inGap) {
                            m_gaps.push_back(Gap{gapAt, row - gapAt, 0});
                            inGap = false;
                        }
                        for (size_t c = 0; c < nc; c++) {
                            lo[c] = std::min(lo[c], vals[c]);
                            hi[c] = std::max(hi[c], vals[c]);
                        }
                    }
                    row++;
                }
                offset += static_cast<uint64_t>((nl ? nl + 1 : end) - p);
                p = nl ? nl + 1 : end;
            }
            carry = QByteArray(p, static_cast<int>(end - p));
            if (!progress(static_cast<double>(offset) / size)) {
                m_error = "stopped";
                return false;
            }
            if (last && carry.isEmpty())
                break;
            if (last) {
                carry.append('\n');
            }
        }
        // the last K rows (whole or not)
        if (m_lo.size() < m_offsets.size()) {
            m_lo.push_back(lo);
            m_hi.push_back(hi);
        }
        if (inGap)
            m_gaps.push_back(Gap{gapAt, row - gapAt, 0});
        m_info.kind = "CSV";
        m_info.total = row;
        m_info.volts = isFloat;
        m_info.bits = isFloat ? 0 : 16;
        m_scale = isFloat ? 1.0 : 1.0 / 32768.0;
        if (t1 > t0)
            m_info.rate = 1e9 / static_cast<double>(t1 - t0);
        m_info.startNs = m_startNs;
        m_info.gapsFrom = m_gaps.empty() ? QString() : "the rows";
        if (row == 0) {
            m_error = "An empty CSV";
            return false;
        }
        return true;
    }
    auto close() -> void override {
        std::lock_guard lock(m_mtx);
        m_file.close();
    }
    auto directLimit() const -> uint64_t override { return 200000; }
    auto saveIndex(QDataStream& out) const -> void override {
        out << quint32(3) << QVector<int>(m_cols.begin(), m_cols.end()) << QVector<int>(m_info.ids.begin(), m_info.ids.end()) << qint32(m_timeCol)
            << quint64(m_dataStart) << m_scale << m_startNs << m_info.volts << qint32(m_info.bits) << m_info.rate << quint64(m_info.total)
            << QVector<quint64>(m_offsets.begin(), m_offsets.end()) << quint64(m_lo.size());
        for (size_t j = 0; j < m_lo.size(); j++)
            out << QVector<double>(m_lo[j].begin(), m_lo[j].end()) << QVector<double>(m_hi[j].begin(), m_hi[j].end());
        out << quint64(m_gaps.size());
        for (const auto& g : m_gaps)
            out << quint64(g.at) << quint64(g.lost);
    }
    auto loadIndex(const QString& path, QDataStream& in) -> bool override {
        quint32 v = 0;
        QVector<int> cols, ids;
        qint32 timeCol = -1, bits = 0;
        quint64 dataStart = 0, total = 0, blocks = 0, ngaps = 0;
        QVector<quint64> offsets;
        in >> v;
        if (v != 3)
            return false;
        in >> cols >> ids >> timeCol >> dataStart >> m_scale >> m_startNs >> m_info.volts >> bits >> m_info.rate >> total >> offsets >> blocks;
        if (in.status() != QDataStream::Ok || cols.isEmpty() || blocks != static_cast<quint64>(offsets.size()))
            return false;
        for (quint64 j = 0; j < blocks; j++) {
            QVector<double> l, h;
            in >> l >> h;
            m_lo.emplace_back(l.begin(), l.end());
            m_hi.emplace_back(h.begin(), h.end());
        }
        in >> ngaps;
        for (quint64 j = 0; j < ngaps && in.status() == QDataStream::Ok; j++) {
            quint64 at = 0, lost = 0;
            in >> at >> lost;
            m_gaps.push_back(Gap{at, lost, 0});
        }
        if (in.status() != QDataStream::Ok)
            return false;
        m_file.setFileName(path);
        if (!m_file.open(QIODevice::ReadOnly))
            return false;
        m_cols.assign(cols.begin(), cols.end());
        m_info.ids.assign(ids.begin(), ids.end());
        m_timeCol = timeCol;
        m_dataStart = dataStart;
        m_info.bits = bits;
        m_info.total = total;
        m_offsets.assign(offsets.begin(), offsets.end());
        m_info.kind = "CSV";
        m_info.startNs = m_startNs;
        m_info.gapsFrom = m_gaps.empty() ? QString() : "the rows";
        return true;
    }
    auto readAll(uint64_t from, uint64_t n, std::vector<std::vector<float>>& out) -> uint64_t override {
        std::lock_guard lock(m_mtx);
        n = from < m_info.total ? std::min(n, m_info.total - from) : 0;
        const size_t nc = m_cols.size();
        out.resize(nc);
        for (auto& o : out)
            o.assign(n, 0.0f);
        if (n == 0)
            return 0;
        const uint64_t block = from / K;
        uint64_t row = block * K;
        m_file.seek(static_cast<qint64>(m_offsets[block]));
        std::vector<double> vals(nc);
        bool isFloat = m_info.volts;
        uint64_t done = 0;
        QByteArray carry;
        while (done < n) {
            QByteArray buf = carry + m_file.read(1 << 20);
            const bool last = m_file.atEnd();
            if (buf.isEmpty())
                break;
            const char* p = buf.constData();
            const char* end = p + buf.size();
            while (p < end && done < n) {
                const char* nl = static_cast<const char*>(std::memchr(p, '\n', static_cast<size_t>(end - p)));
                if (!nl && !last)
                    break;
                const char* le = nl ? nl : end;
                if (le > p && !(le - p == 1 && *p == '\r')) {
                    if (row >= from) {
                        bool gap = false;
                        parseRow(p, le, vals, gap, isFloat, nullptr);
                        if (!gap)
                            for (size_t c = 0; c < nc; c++)
                                out[c][done] = static_cast<float>(vals[c] * m_scale);
                        done++;
                    }
                    row++;
                }
                p = nl ? nl + 1 : end;
            }
            carry = QByteArray(p, static_cast<int>(end - p));
            if (last && (carry.isEmpty() || done >= n))
                break;
            if (last)
                carry.append('\n');
        }
        return n;
    }
    auto overview(int buckets, std::vector<std::vector<float>>& lo, std::vector<std::vector<float>>& hi, const Progress& progress) -> bool override {
        const size_t nc = m_cols.size();
        lo.assign(nc, std::vector<float>(buckets, std::numeric_limits<float>::max()));
        hi.assign(nc, std::vector<float>(buckets, std::numeric_limits<float>::lowest()));
        const uint64_t total = std::max<uint64_t>(1, m_info.total);
        for (size_t j = 0; j < m_lo.size(); j++) {
            const uint64_t a = j * K, b = std::min(total, (j + 1) * K) - 1;
            const int ba = static_cast<int>(a * buckets / total), bb = static_cast<int>(std::min<uint64_t>(buckets - 1, b * buckets / total));
            for (size_t c = 0; c < nc; c++) {
                if (m_lo[j][c] > m_hi[j][c])
                    continue;
                const float l = static_cast<float>(m_lo[j][c] * m_scale), h = static_cast<float>(m_hi[j][c] * m_scale);
                for (int x = ba; x <= bb; x++) {
                    lo[c][x] = std::min(lo[c][x], l);
                    hi[c][x] = std::max(hi[c][x], h);
                }
            }
        }
        progress(1);
        return true;
    }

   private:
    // the values of a row (a gap: "-" or nothing in the first column of values); t: the time
    // column in seconds
    auto parseRow(const char* p, const char* end, std::vector<double>& vals, bool& gap, bool& isFloat, int64_t* t) -> void {
        int col = 0;
        size_t k = 0;
        gap = false;
        while (p <= end && k < m_cols.size()) {
            const char* comma = static_cast<const char*>(std::memchr(p, ',', static_cast<size_t>(end - p)));
            const char* fe = comma ? comma : end;
            if (col == m_timeCol && t)
                *t = timeOf(p, fe);
            if (col == m_cols[k]) {
                const char* s = p;
                while (s < fe && *s == ' ')
                    s++;
                const char* e = fe;
                while (e > s && (e[-1] == '\r' || e[-1] == ' '))
                    e--;
                if (s == e || *s == '-' && e - s == 1) {
                    gap = true;
                    vals[k] = 0;
                } else {
                    double v = 0;
                    std::from_chars(s, e, v);
                    vals[k] = v;
                    if (!isFloat && std::find_if(s, e, [](char c) { return c == '.' || c == 'e' || c == 'E'; }) != e)
                        isFloat = true;
                }
                k++;
            }
            if (!comma)
                break;
            p = comma + 1;
            col++;
        }
    }
    // "2025-11-19 09:00:11.300000256" or a number (seconds with a point, ns without), in ns: a
    // double of seconds since 1970 is too coarse for a sample of a few hundred ns
    auto timeOf(const char* s, const char* e) -> int64_t {
        const QString text = QString::fromLatin1(s, static_cast<int>(e - s)).trimmed();
        if (text.contains(':')) {
            const int dot = text.indexOf('.');
            const QDateTime d = QDateTime::fromString(text.left(dot < 0 ? text.size() : dot), "yyyy-MM-dd hh:mm:ss");
            const QString frac = dot < 0 ? QString() : text.mid(dot + 1);
            const int64_t ns = static_cast<int64_t>(d.toSecsSinceEpoch()) * 1000000000LL + (frac.isEmpty() ? 0 : frac.left(9).leftJustified(9, '0').toLongLong());
            if (m_startNs == 0)
                m_startNs = static_cast<double>(ns);
            return ns;
        }
        return text.contains('.') ? static_cast<int64_t>(std::llround(text.toDouble() * 1e9)) : text.toLongLong();
    }

    QFile m_file;
    std::vector<int> m_cols;  // the columns of the values
    int m_timeCol = -1;
    uint64_t m_dataStart = 0;
    double m_scale = 1;
    double m_startNs = 0;
    std::vector<uint64_t> m_offsets;          // of every K-th row
    std::vector<std::vector<double>> m_lo, m_hi;  // of every K rows
};

}  // namespace

auto Source::create(const QString& path) -> std::unique_ptr<Source> {
    const QString s = QFileInfo(path).suffix().toLower();
    if (s == "bin")
        return std::make_unique<BinSource>();
    if (s == "wav")
        return std::make_unique<WavSource>();
    if (s == "tdms")
        return std::make_unique<TdmsSource>();
    if (s == "csv")
        return std::make_unique<CsvSource>();
    return nullptr;
}

auto Source::viewable(const QString& path) -> bool {
    const QString s = QFileInfo(path).suffix().toLower();
    return s == "bin" || s == "wav" || s == "tdms" || s == "csv";
}

// one pass over the whole: a bucket of the samples between the gaps at a time, no division a
// sample
auto Source::overview(int buckets, std::vector<std::vector<float>>& lo, std::vector<std::vector<float>>& hi, const Progress& progress) -> bool {
    const size_t nc = m_info.ids.size();
    lo.assign(nc, std::vector<float>(buckets, std::numeric_limits<float>::max()));
    hi.assign(nc, std::vector<float>(buckets, std::numeric_limits<float>::lowest()));
    const uint64_t total = m_info.total;
    if (total == 0)
        return true;
    const uint64_t B = static_cast<uint64_t>(buckets);
    std::vector<std::vector<float>> buf;
    size_t gi = 0;
    for (uint64_t pos = 0; pos < total; pos += kChunk) {
        const uint64_t got = readAll(pos, std::min(kChunk, total - pos), buf);
        if (got == 0)
            break;
        uint64_t s = pos;
        const uint64_t end = pos + got;
        while (s < end) {
            while (gi < m_gaps.size() && m_gaps[gi].end() <= s)
                gi++;
            if (gi < m_gaps.size() && m_gaps[gi].at <= s) {
                s = m_gaps[gi].end();
                continue;
            }
            const uint64_t e = gi < m_gaps.size() ? std::min(end, m_gaps[gi].at) : end;
            while (s < e) {
                const uint64_t b = s * B / total;
                const uint64_t next = std::min(e, ((b + 1) * total + B - 1) / B);
                for (size_t c = 0; c < nc && c < buf.size(); c++) {
                    float l = lo[c][b], h = hi[c][b];
                    const float* v = buf[c].data() + (s - pos);
                    for (uint64_t i = 0; i < next - s; i++) {
                        l = std::min(l, v[i]);
                        h = std::max(h, v[i]);
                    }
                    lo[c][b] = l;
                    hi[c][b] = h;
                }
                s = next;
            }
        }
        if (!progress(static_cast<double>(end) / static_cast<double>(total)))
            return false;
    }
    return true;
}

auto readReport(const QString& path) -> Report {
    Report r;
    QFile f(path + ".log.lost.txt");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        static const QRegularExpression row("^(\\S+ ?\\d*):\\s+Pos\\s+(\\d+)\\s+Get:\\s+(\\d+)\\s+\\((\\d+)\\)\\s+Wait:\\s+(\\d+)");
        QString first;
        QTextStream in(&f);
        while (!in.atEnd()) {
            const QString line = in.readLine();
            if (!line.startsWith("Channel"))
                continue;
            const auto m = row.match(line);
            if (!m.hasMatch())
                continue;
            if (first.isEmpty())
                first = m.captured(1);
            if (m.captured(1) != first)
                continue;
            const uint64_t pos = m.captured(2).toULongLong(), get = m.captured(3).toULongLong();
            const uint64_t lost = m.captured(4).toULongLong(), wait = m.captured(5).toULongLong();
            if (lost + wait) {
                r.gaps.push_back(Gap{pos, lost, wait});
                r.gets.push_back(get);
            }
            r.data += get;
            r.total = pos + lost + wait + get;
        }
    }
    QFile rep(path + ".log.txt");
    if (rep.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&rep);
        while (!in.atEnd()) {
            const QString line = in.readLine();
            const int tab = line.indexOf(':');
            if (tab < 0)
                continue;
            const QString key = line.left(tab).trimmed();
            const double v = line.mid(tab + 1).trimmed().toDouble();
            if (key == "Sample rate")
                r.rate = v;
            else if (key == "ADC rate")
                r.clock = v;
            else if (key == "Decimation")
                r.decimation = v;
        }
    }
    return r;
}

auto Mapping::set(const std::vector<Gap>& gaps, const std::vector<uint64_t>& gets, uint64_t fileSamples, uint64_t total, uint64_t data) -> void {
    m_pieces.clear();
    m_total = total;
    uint64_t lost = 0, clipped = 0;
    for (size_t i = 0; i < gaps.size(); i++) {
        lost += gaps[i].lost;
        clipped += std::min(gaps[i].lost, i < gets.size() ? gets[i] : gaps[i].lost);
    }
    // the zeros of a gap in the file: all of it, the lost ones, the lost ones up to the buffer, none
    const uint64_t counts[4] = {total, data + lost, data + clipped, data};
    int mode = 0;
    uint64_t best = std::numeric_limits<uint64_t>::max();
    for (int m = 0; m < 4; m++) {
        const uint64_t d = counts[m] > fileSamples ? counts[m] - fileSamples : fileSamples - counts[m];
        if (d < best) {
            best = d;
            mode = m;
        }
    }
    m_identity = gaps.empty() || mode == 0;
    if (m_identity) {
        m_total = std::min(fileSamples, total ? total : fileSamples);
        return;
    }
    uint64_t p = 0, f = 0;
    for (size_t i = 0; i < gaps.size(); i++) {
        const auto& g = gaps[i];
        if (g.at > p) {
            m_pieces.push_back(Run{p, f, g.at - p});
            f += g.at - p;
        }
        const uint64_t z = mode == 1 ? g.lost : mode == 2 ? std::min(g.lost, i < gets.size() ? gets[i] : g.lost) : 0;
        if (z) {
            m_pieces.push_back(Run{g.at, f, z});
            f += z;
        }
        p = g.end();
    }
    if (total > p && fileSamples > f)
        m_pieces.push_back(Run{p, f, std::min(total - p, fileSamples - f)});
}

auto Mapping::runs(uint64_t from, uint64_t n) const -> std::vector<Run> {
    std::vector<Run> r;
    if (n == 0)
        return r;
    if (m_identity) {
        if (from < m_total)
            r.push_back(Run{from, from, std::min(n, m_total - from)});
        return r;
    }
    const uint64_t to = from + n;
    auto it = std::upper_bound(m_pieces.begin(), m_pieces.end(), from, [](uint64_t v, const Run& x) { return v < x.at; });
    if (it != m_pieces.begin())
        --it;
    for (; it != m_pieces.end() && it->at < to; ++it) {
        const uint64_t a = std::max(from, it->at), b = std::min(to, it->at + it->count);
        if (b > a)
            r.push_back(Run{a, it->file + (a - it->at), b - a});
    }
    return r;
}

}  // namespace view
