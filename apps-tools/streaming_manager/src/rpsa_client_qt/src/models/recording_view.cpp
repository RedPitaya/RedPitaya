#include "recording_view.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include "converter_lib/converter.h"

namespace {

auto isGpio(bin_lib::SampleType t) -> bool {
    return t == bin_lib::SampleType::GPIO_IN_WORD || t == bin_lib::SampleType::GPIO_OUT_WORD;
}

// a window of more samples than this comes from the overview, not from the file
constexpr uint64_t kDirect = 2000000;

}  // namespace

RecordingView::RecordingView(QObject* parent) : QObject(parent) {}

RecordingView::~RecordingView() {
    release();
}

void RecordingView::release() {
    m_stop = true;
    stopExport();
    if (m_overviewThread.joinable())
        m_overviewThread.join();
    if (m_exportThread.joinable())
        m_exportThread.join();
    // Windows deletes no file that is open
    m_reader.close();
    m_adc.clear();
}

auto RecordingView::sampleOf(const uint8_t* data, size_t i, bin_lib::SampleType type) const -> float {
    switch (type) {
        case bin_lib::SampleType::INT8:
            return reinterpret_cast<const int8_t*>(data)[i] / 128.0f;
        case bin_lib::SampleType::INT16:
            return reinterpret_cast<const int16_t*>(data)[i] / 32768.0f;
        case bin_lib::SampleType::FLOAT32:
            return reinterpret_cast<const float*>(data)[i];
        default:
            return 0;
    }
}

bool RecordingView::open(const QString& path) {
    m_path = path;
    m_info = QVariantMap{{"name", QFileInfo(path).fileName()}};
    if (!m_reader.open(path.toStdString())) {
        m_error = QString::fromStdString(m_reader.error());
        Q_EMIT changed();
        return false;
    }
    const auto& info = m_reader.info();
    m_adc.clear();
    QVariantList inputs;
    int bits = 0;
    bool volts = false;
    for (size_t i = 0; i < info.channels.size(); i++) {
        const auto& ch = info.channels[i];
        if (isGpio(ch.type) || bin_lib::sampleBytes(ch.type) == 0)
            continue;
        m_adc.push_back(static_cast<uint8_t>(i));
        inputs.append(ch.id);
        bits = std::max(bits, static_cast<int>(bin_lib::sampleBytes(ch.type) * 8));
        volts |= ch.type == bin_lib::SampleType::FLOAT32;
    }
    if (m_adc.empty()) {
        m_error = "No ADC channel in this recording (a GPIO recording: convert it to VCD)";
        Q_EMIT changed();
        return false;
    }
    m_total = m_reader.samples(m_adc[0], true);
    const double dec = info.decimation ? info.decimation : 1;
    m_rate = info.clock ? static_cast<double>(info.clock) / dec : 0;
    m_info = QVariantMap{{"rate", m_rate},
                         {"decimation", dec},
                         {"adcRate", static_cast<double>(info.clock)},
                         {"inputs", inputs},
                         {"bits", volts ? 0 : bits},
                         {"volts", volts},
                         {"total", static_cast<double>(m_total)},
                         {"seconds", m_rate > 0 ? static_cast<double>(m_total) / m_rate : 0.0},
                         {"startNs", static_cast<double>(info.startTime)},
                         {"name", QFileInfo(path).fileName()}};
    m_error.clear();
    Q_EMIT changed();
    m_stop = false;
    m_overviewThread = std::thread(&RecordingView::buildOverview, this);
    return true;
}

// The whole recording once: min and max of each bucket of the time line, the gaps of the first
// channel (all channels of a buffer lose together)
auto RecordingView::buildOverview() -> void {
    bin_lib::FileReader r;
    if (!r.open(m_path.toStdString()) || m_total == 0)
        return;
    const auto& info = r.info();
    const size_t n = m_adc.size();
    std::vector<int> slot(info.channels.size(), -1);
    for (size_t k = 0; k < n; k++)
        slot[m_adc[k]] = static_cast<int>(k);
    std::vector<std::vector<float>> lo(n, std::vector<float>(BUCKETS, std::numeric_limits<float>::max()));
    std::vector<std::vector<float>> hi(n, std::vector<float>(BUCKETS, std::numeric_limits<float>::lowest()));
    std::vector<Gap> gaps;
    const double size = std::max<double>(1, QFileInfo(m_path).size());
    double done = 0;
    double lost = 0, waited = 0;
    bin_lib::BlockInfo b;
    std::vector<uint8_t> data;
    int tick = 0;
    while (!m_stop && r.nextBlock(b, data)) {
        done += bin_lib::blockSize(b.dataSize);
        const int k = slot[b.channel];
        if (k < 0)
            continue;
        const auto type = info.channels[b.channel].type;
        if (k == 0 && b.lost + b.wait > 0) {
            gaps.push_back(Gap{b.position - b.lost - b.wait, b.lost, b.wait});
            lost += static_cast<double>(b.lost);
            waited += static_cast<double>(b.wait);
        }
        auto& l = lo[k];
        auto& h = hi[k];
        for (uint64_t i = 0; i < b.samples; i++) {
            const uint64_t bucket = std::min<uint64_t>(BUCKETS - 1, (b.position + i) * BUCKETS / m_total);
            const float v = sampleOf(data.data(), i, type);
            if (v < l[bucket])
                l[bucket] = v;
            if (v > h[bucket])
                h[bucket] = v;
        }
        if (++tick % 64 == 0) {
            m_progress = std::min(0.99, done / size);
            QMetaObject::invokeMethod(this, [this]() { Q_EMIT progressChanged(); }, Qt::QueuedConnection);
        }
    }
    if (m_stop)
        return;
    {
        std::lock_guard lock(m_mtx);
        m_lo = std::move(lo);
        m_hi = std::move(hi);
        m_gaps = std::move(gaps);
        m_overviewReady = true;
    }
    m_progress = 1;
    QMetaObject::invokeMethod(
        this,
        [this, lost, waited]() {
            size_t places = 0;
            {
                std::lock_guard lock(m_mtx);
                for (const auto& g : m_gaps)
                    places += g.lost > 0 ? 1 : 0;
            }
            m_info["lost"] = lost;
            m_info["waited"] = waited;
            m_info["places"] = static_cast<double>(places);
            Q_EMIT changed();
            Q_EMIT progressChanged();
            Q_EMIT overviewChanged();
        },
        Qt::QueuedConnection);
}

auto RecordingView::overview() -> QVariantMap {
    std::lock_guard lock(m_mtx);
    QVariantMap r;
    if (!m_overviewReady)
        return r;
    QVariantList lo, hi;
    for (size_t k = 0; k < m_lo.size(); k++) {
        QVector<qreal> l(BUCKETS), h(BUCKETS);
        for (int i = 0; i < BUCKETS; i++) {
            // an empty bucket (a gap): nothing
            const bool empty = m_lo[k][i] > m_hi[k][i];
            l[i] = empty ? 0 : m_lo[k][i];
            h[i] = empty ? 0 : m_hi[k][i];
        }
        lo.append(QVariant::fromValue(l));
        hi.append(QVariant::fromValue(h));
    }
    r["buckets"] = BUCKETS;
    r["lo"] = lo;
    r["hi"] = hi;
    return r;
}

auto RecordingView::gaps() -> QVariantList {
    std::lock_guard lock(m_mtx);
    QVariantList r;
    for (const auto& g : m_gaps)
        r.append(QVariantMap{{"at", static_cast<double>(g.at)}, {"lost", static_cast<double>(g.lost)}, {"wait", static_cast<double>(g.wait)}});
    return r;
}

QVariantMap RecordingView::window(double fromD, double toD, int width) {
    QVariantMap r;
    if (m_adc.empty() || width <= 0)
        return r;
    const uint64_t from = static_cast<uint64_t>(std::clamp(fromD, 0.0, static_cast<double>(m_total)));
    const uint64_t to = static_cast<uint64_t>(std::clamp(toD, 0.0, static_cast<double>(m_total)));
    if (to <= from)
        return r;
    const uint64_t n = to - from;
    const auto& info = m_reader.info();
    QVariantList channels;
    if (n > kDirect) {
        // far out: the buckets of the overview
        std::lock_guard lock(m_mtx);
        if (!m_overviewReady) {
            r["pending"] = true;
            return r;
        }
        const double b0 = static_cast<double>(from) * BUCKETS / m_total;
        const double b1 = static_cast<double>(to) * BUCKETS / m_total;
        for (size_t k = 0; k < m_adc.size(); k++) {
            QVector<qreal> lo(width), hi(width);
            for (int j = 0; j < width; j++) {
                int s = static_cast<int>(b0 + (b1 - b0) * j / width);
                int e = std::max(s + 1, static_cast<int>(b0 + (b1 - b0) * (j + 1) / width));
                float l = std::numeric_limits<float>::max(), h = std::numeric_limits<float>::lowest();
                for (int i = std::max(0, s); i < std::min(e, BUCKETS); i++) {
                    l = std::min(l, m_lo[k][i]);
                    h = std::max(h, m_hi[k][i]);
                }
                lo[j] = l > h ? 0 : l;
                hi[j] = l > h ? 0 : h;
            }
            channels.append(QVariantMap{{"id", info.channels[m_adc[k]].id}, {"lo", QVariant::fromValue(lo)}, {"hi", QVariant::fromValue(hi)}});
        }
        r["mode"] = "band";
    } else {
        // near: the samples of the file (the gaps read as zeros, the plot leaves them out)
        std::vector<uint8_t> buf;
        for (size_t k = 0; k < m_adc.size(); k++) {
            const auto type = info.channels[m_adc[k]].type;
            buf.resize(n * bin_lib::sampleBytes(type));
            const uint64_t got = m_reader.read(m_adc[k], from, n, buf.data(), true);
            if (got <= static_cast<uint64_t>(width)) {
                QVector<qreal> v(static_cast<int>(got));
                for (uint64_t i = 0; i < got; i++)
                    v[static_cast<int>(i)] = sampleOf(buf.data(), i, type);
                channels.append(QVariantMap{{"id", info.channels[m_adc[k]].id}, {"values", QVariant::fromValue(v)}});
                r["mode"] = "line";
            } else {
                QVector<qreal> lo(width), hi(width);
                for (int j = 0; j < width; j++) {
                    const uint64_t s = got * j / width, e = std::max(s + 1, got * (j + 1) / width);
                    float l = std::numeric_limits<float>::max(), h = std::numeric_limits<float>::lowest();
                    for (uint64_t i = s; i < e; i++) {
                        const float x = sampleOf(buf.data(), i, type);
                        l = std::min(l, x);
                        h = std::max(h, x);
                    }
                    lo[j] = l;
                    hi[j] = h;
                }
                channels.append(QVariantMap{{"id", info.channels[m_adc[k]].id}, {"lo", QVariant::fromValue(lo)}, {"hi", QVariant::fromValue(hi)}});
                r["mode"] = "band";
            }
        }
    }
    r["from"] = static_cast<double>(from);
    r["to"] = static_cast<double>(to);
    r["channels"] = channels;
    return r;
}

QString RecordingView::exportName(double from, double to, const QString& format) {
    const QFileInfo fi(m_path);
    // as many decimals as the two ends differ in (3 at least)
    const double span = m_rate > 0 ? std::max(1e-9, (to - from) / m_rate) : 1;
    const int dec = std::clamp(static_cast<int>(std::ceil(-std::log10(span))) + 2, 3, 9);
    auto secs = [this, dec](double p) { return m_rate > 0 ? QString::number(p / m_rate, 'f', dec) : QString::number(static_cast<qint64>(p)); };
    return QDir::toNativeSeparators(fi.absolutePath() + "/" + fi.completeBaseName() + "_" + secs(from) + "-" + secs(to) + "." + format.toLower());
}

bool RecordingView::exists() {
    return QFileInfo::exists(m_path);
}

auto RecordingView::exporting() -> QVariantMap {
    std::lock_guard lock(m_mtx);
    return m_export;
}

QString RecordingView::exportRange(double fromD, double toD, const QString& format, const QVariantList& inputs, bool index, bool time, const QString& output) {
    const QString f = format.toUpper();
    if (!QStringList{"CSV", "WAV", "TDMS", "BIN"}.contains(f))
        return "Not a format: " + format;
    const uint64_t from = static_cast<uint64_t>(std::clamp(fromD, 0.0, static_cast<double>(m_total)));
    const uint64_t to = static_cast<uint64_t>(std::clamp(toD, 0.0, static_cast<double>(m_total)));
    if (to <= from)
        return "The range is empty";
    if (output.trimmed().isEmpty())
        return "Give the name of the file";
    if (QFileInfo::exists(output))
        return QDir::toNativeSeparators(output) + " is there: give another name";
    std::vector<uint8_t> chans;
    const auto& info = m_reader.info();
    for (const auto& idx : m_adc)
        for (const auto& i : inputs)
            if (i.toInt() == info.channels[idx].id)
                chans.push_back(idx);
    if (chans.empty())
        return "No input chosen";
    {
        std::lock_guard lock(m_mtx);
        if (m_export.value("running").toBool())
            return "An export runs already";
        m_export = QVariantMap{{"running", true}, {"progress", 0.0}, {"output", QDir::toNativeSeparators(output)}};
    }
    if (m_exportThread.joinable())
        m_exportThread.join();
    m_exportStop = false;
    m_exportThread = std::thread(&RecordingView::runExport, this, from, to, f, chans, index, time, output);
    Q_EMIT exportingChanged();
    return "";
}

void RecordingView::stopExport() {
    m_exportStop = true;
    std::lock_guard lock(m_mtx);
    if (m_conv) {
        m_conv->stopWriteToCSV();
        m_conv->stopWriteToWAV();
        m_conv->stopWriteToTDMS();
    }
}

// The range as a BIN of its own (the losses and the waits inside it kept, the time line from its
// start); CSV, WAV and TDMS are converted from that one, which then goes
auto RecordingView::runExport(uint64_t from, uint64_t to, QString format, std::vector<uint8_t> channels, bool index, bool time, QString output) -> void {
    auto progress = [this](double p) {
        {
            std::lock_guard lock(m_mtx);
            m_export["progress"] = p;
        }
        QMetaObject::invokeMethod(this, [this]() { Q_EMIT exportingChanged(); }, Qt::QueuedConnection);
    };
    auto finish = [this](bool ok, const QString& out, const QString& message) {
        {
            std::lock_guard lock(m_mtx);
            m_export = QVariantMap{{"running", false}, {"ok", ok}, {"output", QDir::toNativeSeparators(out)}, {"message", message}};
        }
        QMetaObject::invokeMethod(
            this,
            [this, ok, out, message]() {
                Q_EMIT exportingChanged();
                Q_EMIT exported(ok, QDir::toNativeSeparators(out), message);
            },
            Qt::QueuedConnection);
    };
    const bool bin = format == "BIN";
    // written under a name the recordings do not list (not .bin), then renamed or converted
    const QString binPath = output + ".part";
    bin_lib::FileReader r;
    if (!r.open(m_path.toStdString()))
        return finish(false, output, QString::fromStdString(r.error()));
    const auto& info = r.info();
    bin_lib::FileInfo out = info;
    out.channels.clear();
    std::vector<int> remap(info.channels.size(), -1);
    for (auto idx : channels) {
        remap[idx] = static_cast<int>(out.channels.size());
        out.channels.push_back(info.channels[idx]);
    }
    if (info.startTime && m_rate > 0)
        out.startTime = info.startTime + static_cast<int64_t>(static_cast<double>(from) / m_rate * 1e9);
    std::ofstream file(binPath.toStdString(), std::ios::binary | std::ios::trunc);
    if (!file)
        return finish(false, output, "Can't write " + QDir::toNativeSeparators(binPath));
    const auto head = bin_lib::fileHeader(out);
    file.write(reinterpret_cast<const char*>(head.data()), static_cast<std::streamsize>(head.size()));
    bin_lib::Timeline line;
    bin_lib::BlockInfo b;
    std::vector<uint8_t> data, block;
    const double size = std::max<double>(1, QFileInfo(m_path).size());
    double done = 0;
    int tick = 0;
    while (!m_exportStop && r.nextBlock(b, data)) {
        done += bin_lib::blockSize(b.dataSize);
        if (++tick % 64 == 0)
            progress((bin ? 1.0 : 0.5) * std::min(1.0, done / size));
        if (remap[b.channel] < 0)
            continue;
        const uint64_t pos = b.position, gap = b.lost + b.wait, end = pos + b.samples;
        // a gap is written with the data after it: a block from `to` on is out, its gap too
        if (end <= from || pos >= to)
            continue;
        // the part of the gap before the block that is in the range, the lost first
        const uint64_t gapIn = pos > from ? std::min(gap, pos - from) : 0;
        const uint64_t waitIn = std::min(b.wait, gapIn), lostIn = gapIn - waitIn;
        const uint64_t s0 = std::max(from, pos) - pos, s1 = std::min(to, end) - pos;
        if (s1 <= s0)
            continue;
        const uint32_t bytes = bin_lib::sampleBytes(info.channels[b.channel].type);
        bin_lib::BlockInfo nb = b;
        nb.channel = static_cast<uint8_t>(remap[b.channel]);
        nb.samples = s1 - s0;
        nb.lost = lostIn;
        nb.wait = waitIn;
        nb.position = line.next(nb.channel, lostIn, waitIn, nb.samples);
        nb.dataSize = static_cast<uint32_t>(nb.samples * bytes);
        block.clear();
        bin_lib::appendBlock(block, nb, data.data() + s0 * bytes, nb.dataSize);
        file.write(reinterpret_cast<const char*>(block.data()), static_cast<std::streamsize>(block.size()));
    }
    file.close();
    if (m_exportStop) {
        QFile::remove(binPath);
        return finish(false, output, "Stopped");
    }
    if (!file)
        return finish(false, output, "Can't write " + QDir::toNativeSeparators(binPath));
    if (bin) {
        if (!QFile::rename(binPath, output)) {
            QFile::remove(binPath);
            return finish(false, output, "Can't write " + QDir::toNativeSeparators(output));
        }
        return finish(true, output, "");
    }
    auto c = std::make_shared<converter_lib::CConverter>();
    {
        std::lock_guard lock(m_mtx);
        m_conv = c;
    }
    auto& conv = *c;
    conv.setOutput(output.toStdString());
    conv.setOverwrite(false);
    conv.setCSVHeader(true);
    // a convert*() clears the stop of the converter as it starts: a stop before that is passed on here
    conv.setProgress([this, &progress, cp = c.get()](double p) {
        if (m_exportStop) {
            cp->stopWriteToCSV();
            cp->stopWriteToWAV();
            cp->stopWriteToTDMS();
        }
        progress(0.5 + p / 2);
    });
    bool ok = false;
    if (format == "CSV") {
        int mode = FH_CSV_NONE;
        if (index)
            mode |= FH_CSV_ADD_INDEX;
        if (time)
            mode |= FH_CSV_ADD_TIME_COL;
        ok = conv.convertToCSV(binPath.toStdString(), "", static_cast<FH_CSVMode>(mode));
    } else if (format == "WAV") {
        ok = conv.convertToWAV(binPath.toStdString(), "");
    } else {
        ok = conv.convertToTDMS(binPath.toStdString(), "");
    }
    {
        std::lock_guard lock(m_mtx);
        m_conv.reset();
    }
    QFile::remove(binPath);
    if (m_exportStop) {
        // the part written goes, with the reports of the converter (the name was free before)
        for (const auto& f : {output, output + ".log.txt", output + ".log.lost.txt"})
            QFile::remove(f);
        return finish(false, output, "Stopped");
    }
    finish(ok, output, ok ? QString() : QString::fromStdString(conv.lastError()));
}
