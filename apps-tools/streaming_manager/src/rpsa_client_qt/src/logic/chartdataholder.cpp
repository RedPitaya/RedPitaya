#include "chartdataholder.h"
#include "src/dsp/spectrum.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <QQmlEngine>
#include <QThread>

namespace {
auto nowMs() -> int64_t {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
}  // namespace

// the spectrum of a block (src/dsp, after the DSP of the Red Pitaya API) and its buffers
struct ChartDataHolder::Dsp {
    static constexpr uint32_t MAX = 65536;
    dsp::Spectrum spectrum;
    std::vector<float> in;
};

ChartDataHolder* ChartDataHolder::instance() {
    static ChartDataHolder* _instance = 0;
    if (_instance == 0) {
        _instance = new ChartDataHolder();
    }
    return _instance;
}

ChartDataHolder::ChartDataHolder() : QObject(nullptr) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    m_IsWorkThread = true;
    m_chartThread = new std::thread(&ChartDataHolder::chartPackThread, this);
}

ChartDataHolder::~ChartDataHolder() {
    m_IsWorkThread = false;
    if (m_chartThread->joinable()) {
        m_chartThread->join();
    }
    delete m_dsp;
}

auto ChartDataHolder::regRP(const QString& ip) -> void {
    std::lock_guard<std::mutex> lock(m_add_mutex);
    m_rp.append(ip);
    m_packId[ip] = 0;
    m_chartPoints[0][ip].clear();
    m_chartPoints[1][ip].clear();
    m_chartPoints[2][ip].clear();
    m_chartPoints[3][ip].clear();
}

auto ChartDataHolder::removeRP(const QString& ip) -> void {
    std::lock_guard<std::mutex> lock(m_add_mutex);
    m_rp.removeAll(ip);
    m_pack.remove(ip);
    m_packId.remove(ip);
}

auto ChartDataHolder::addBuffer(DataLib::CDataBuffersPackDMA::Ptr pack, uint64_t id, const QString& ip, bool skip) -> void {
    std::lock_guard<std::mutex> lock(m_add_mutex);

    // skip some
    if (m_packId[ip] != 0)
        return;

    uint64_t maxID = 0;
    for (auto i : m_packId.keys()) {
        if (i != ip) {
            maxID = maxID < m_packId[i] ? m_packId[i] : maxID;
        }
    }
    if (skip) {
        m_packId[ip] = maxID;
        m_pack[ip] = nullptr;
        //        qDebug() << "Skip " << ip;
        return;
    }
    //    qDebug() << ip << "Max" << maxID << " id " << id;
    if (maxID < id) {
        for (auto& i : m_rp) {
            m_packId[i] = 0;
            m_pack[i] = nullptr;
        }
        //        qDebug() << "Reset all";
    }
    //    qDebug() << "Set " << ip << id;
    m_pack[ip] = pack;
    m_packId[ip] = id;
    if (!m_frameSince)
        m_frameSince = nowMs();
}

auto ChartDataHolder::chartPackThread() -> void {
    while (m_IsWorkThread) {
        QMap<QString, DataLib::CDataBuffersPackDMA::Ptr> packForWork;
        QMap<QString, int> decimation;
        QMap<QString, bool> measureOn;
        QMap<QString, SpectrumSet> spectrumSet;
        m_add_mutex.lock();
        const int chartWidth = m_chartWidth;
        // one frame of all boards at the same moment; the boards that sent nothing within an
        // interval (waiting for a trigger, slower) are left out of it
        bool all = m_rp.size();
        bool any = false;
        for (auto& i : m_rp) {
            all &= m_packId[i] != 0;
            any |= m_packId[i] != 0;
        }
        const bool canGet = all || (any && m_frameSince && nowMs() - m_frameSince >= m_intervalMs);
        if (canGet) {
            packForWork = m_pack;
            decimation = m_decimation;
            measureOn = m_measureOn;
            spectrumSet = m_spectrumSet;
            for (auto i : m_packId.keys()) {
                if (!m_packId[i])
                    packForWork.remove(i);  // an old block of a board left out
                m_packId[i] = 0;
            }
            m_frameSince = 0;
        }
        m_add_mutex.unlock();

        auto pack = [&](const QString ip, int index, DataLib::CDataBufferDMA::Ptr buf) {
            if (!buf)
                return;
            const int pointsCount = static_cast<int>(buf->getSamplesCount());
            auto bitSize = buf->getBitBySample();
            auto buffer = buf->getMappedDataMemory();
            auto sample = [&](int i) -> int {
                return bitSize == 8 ? reinterpret_cast<int8_t*>(buffer)[i] << 8 : reinterpret_cast<int16_t*>(buffer)[i];
            };
            const int dec = decimation.value(ip, 1);
            if (dec > 0) {
                // a window from the start of the block: dec samples (their mean) per point
                const int count = std::min(chartWidth, pointsCount / dec);
                auto& out = m_chartPoints[index][ip];
                out.resize(count);
                for (int w = 0; w < count; w++) {
                    int64_t sum = 0;
                    for (int j = 0; j < dec; j++)
                        sum += sample(w * dec + j);
                    out[w] = (float)(sum / dec) / 32768.0;
                }
                return;
            }
            int step = pointsCount / chartWidth;
            if (step <= 1) {
                // every sample: fewer than two of them per point
                m_chartPoints[index][ip].resize(pointsCount);
                for (int i = 0; i < pointsCount; i++) {
                    if (bitSize == 8) {
                        auto b = reinterpret_cast<int8_t*>(buffer);
                        m_chartPoints[index][ip][i] = (float)b[i] / 128.0;
                    }
                    if (bitSize == 16) {
                        auto b = reinterpret_cast<int16_t*>(buffer);
                        m_chartPoints[index][ip][i] = (float)b[i] / 32768.0;
                    }
                }
            } else {
                m_chartPoints[index][ip].resize(chartWidth);
                for (int i = 1, w = 0; i < pointsCount - 1 && w < chartWidth; i += step, w++) {
                    int64_t sum = 0;
                    for (int j = 0u; j < step && i + j < pointsCount; j++) {
                        if (bitSize == 8) {
                            auto b = reinterpret_cast<int8_t*>(buffer);
                            sum += b[i + j] << 8;
                        }
                        if (bitSize == 16) {
                            auto b = reinterpret_cast<int16_t*>(buffer);
                            sum += b[i + j];
                        }
                    }
                    m_chartPoints[index][ip][w] = (float)(sum / step) / 32768.0;
                }
            }
        };

        if (canGet) {
            //            qDebug() << Q_FUNC_INFO;
            m_chartGetPointMutex.lock();
            if (m_chartNeedDraw.size() == 0) {
                for (auto i : packForWork.keys()) {
                    if (packForWork[i]) {
                        pack(i, 0, packForWork[i]->getBuffer(DataLib::CH1));
                        pack(i, 1, packForWork[i]->getBuffer(DataLib::CH2));
                        pack(i, 2, packForWork[i]->getBuffer(DataLib::CH3));
                        pack(i, 3, packForWork[i]->getBuffer(DataLib::CH4));
                        if (spectrumSet.value(i).on)
                            spectrumOf(i, packForWork[i], spectrumSet.value(i), chartWidth);
                        if (measureOn.value(i, false)) {
                            QVector<Measure> ms(4);
                            const DataLib::EDataBuffersPackChannel chs[4] = {DataLib::CH1, DataLib::CH2, DataLib::CH3, DataLib::CH4};
                            for (int c = 0; c < 4; c++) {
                                auto b = packForWork[i]->getBuffer(chs[c]);
                                if (b)
                                    ms[c] = measure(b->getMappedDataMemory(), b->getBitBySample(), static_cast<int>(b->getSamplesCount()));
                            }
                            m_measures[i] = ms;
                        }
                        m_chartNeedDraw.append(i);
                    }
                }
            }
            m_chartGetPointMutex.unlock();
        }
        // in short steps, so the end of the application does not wait for a long one
        for (int waited = 0; waited < m_intervalMs && m_IsWorkThread; waited += 20)
            QThread::usleep(1000 * 20);
    }
}

// min, max, mean and RMS of a whole block, and its period: the rising crossings of the middle of
// min..max with a hysteresis of 10 % of peak-peak, the mean of the whole periods between the first
// and the last (none below two periods)
auto ChartDataHolder::measure(const void* data, int bits, int count) -> Measure {
    Measure m;
    if (!data || count <= 0)
        return m;
    auto sample = [&](int i) -> double {
        return bits == 8 ? reinterpret_cast<const int8_t*>(data)[i] / 128.0 : reinterpret_cast<const int16_t*>(data)[i] / 32768.0;
    };
    double mn = 2, mx = -2, sum = 0, sq = 0;
    for (int i = 0; i < count; i++) {
        const double v = sample(i);
        mn = std::min(mn, v);
        mx = std::max(mx, v);
        sum += v;
        sq += v * v;
    }
    m.valid = true;
    m.samples = static_cast<uint64_t>(count);
    m.min = mn;
    m.max = mx;
    m.mean = sum / count;
    m.rms = std::sqrt(sq / count);
    // the codes at an end of the range (99 %: the last codes of a 14 bit ADC in 16 bit)
    m.clipped = mn <= -0.99 || mx >= 0.99;
    const double pp = mx - mn;
    if (pp > 0.002) {  // above a few codes of noise
        const double mid = (mx + mn) / 2, hi = mid + 0.05 * pp, lo = mid - 0.05 * pp;
        bool below = sample(0) < lo;
        int first = -1, last = -1, crossings = 0;
        for (int i = 1; i < count; i++) {
            const double v = sample(i);
            if (below && v > hi) {
                below = false;
                if (first < 0)
                    first = i;
                last = i;
                crossings++;
            } else if (!below && v < lo) {
                below = true;
            }
        }
        if (crossings >= 3)
            m.period = static_cast<double>(last - first) / (crossings - 1);
    }
    return m;
}

QVariantList ChartDataHolder::getMeasure(QString ip) {
    std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
    QVariantList r;
    const auto list = m_measures.value(ip);
    for (const auto& m : list) {
        QVariantMap x;
        x["valid"] = m.valid;
        x["samples"] = static_cast<double>(m.samples);
        x["min"] = m.min;
        x["max"] = m.max;
        x["mean"] = m.mean;
        x["rms"] = m.rms;
        x["period"] = m.period;
        x["clipped"] = m.clipped;
        r.append(x);
    }
    return r;
}

auto ChartDataHolder::setMeasure(const QString& ip, bool on) -> void {
    {
        std::lock_guard<std::mutex> lock(m_add_mutex);
        m_measureOn[ip] = on;
    }
    if (!on) {
        std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
        m_measures.remove(ip);
    }
}

// The FFT of the whole block of each channel (the largest power of two in it, 65536 at most):
// the window, the power of the full range (1 V); the mean power of the last blocks; the points of
// the width keep the highest bin each, in dB. Under m_chartGetPointMutex.
auto ChartDataHolder::spectrumOf(const QString& ip, DataLib::CDataBuffersPackDMA::Ptr pack, const SpectrumSet& set, int width) -> void {
    if (!m_dsp)
        m_dsp = new Dsp();
    const DataLib::EDataBuffersPackChannel chs[4] = {DataLib::CH1, DataLib::CH2, DataLib::CH3, DataLib::CH4};
    uint32_t samples = 0;
    for (auto ch : chs)
        if (auto b = pack->getBuffer(ch))
            samples = samples ? std::min<uint32_t>(samples, b->getSamplesCount()) : static_cast<uint32_t>(b->getSamplesCount());
    uint32_t n = 256;
    while (n * 2 <= std::min(samples, Dsp::MAX))
        n *= 2;
    if (samples < 256)
        return;
    auto& spec = m_dsp->spectrum;
    if (!spec.setup(n, static_cast<dsp::Spectrum::Window>(set.window)))
        return;
    auto& in = m_dsp->in;
    in.resize(n);
    const uint32_t bins = n / 2;
    Spectrum out;
    out.n = n;
    for (int c = 0; c < 4; c++) {
        auto& hist = m_history[c][ip];
        auto b = pack->getBuffer(chs[c]);
        if (!b || b->getSamplesCount() < n) {
            hist.clear();
            continue;
        }
        const void* raw = b->getMappedDataMemory();
        if (b->getBitBySample() == 8) {
            auto s = reinterpret_cast<const int8_t*>(raw);
            for (uint32_t k = 0; k < n; k++)
                in[k] = s[k] / 128.0f;
        } else {
            auto s = reinterpret_cast<const int16_t*>(raw);
            for (uint32_t k = 0; k < n; k++)
                in[k] = s[k] / 32768.0f;
        }
        // the power of this block, then the mean of the last ones
        std::vector<float> p(bins);
        spec.power(in.data(), p.data());
        if (!hist.empty() && hist.front().size() != bins)
            hist.clear();
        hist.push_back(std::move(p));
        while (static_cast<int>(hist.size()) > std::max(1, set.average))
            hist.erase(hist.begin());
        std::vector<float> mean(bins, 0.0f);
        for (const auto& h : hist)
            for (uint32_t k = 0; k < bins; k++)
                mean[k] += h[k];
        const float count = static_cast<float>(hist.size());
        out.avg = static_cast<int>(hist.size());
        auto dbOf = [&](uint32_t k) { return mean[k] > 0 ? 10.0 * std::log10(mean[k] / count) : -300.0; };
        // the highest bin, over the DC (the first bins carry the offset and the window)
        int peak = 3;
        for (uint32_t k = 3; k < bins; k++)
            if (mean[k] > mean[peak])
                peak = static_cast<int>(k);
        out.peakBin[c] = peak;
        out.peakDb[c] = dbOf(peak);
        out.has[c] = true;
        const uint32_t w = std::max(1, std::min<int>(width, static_cast<int>(bins)));
        auto& pts = out.points[c];
        pts.resize(w);
        for (uint32_t j = 0; j < w; j++) {
            const uint32_t from = static_cast<uint32_t>(static_cast<uint64_t>(j) * bins / w);
            const uint32_t to = std::max(from + 1, static_cast<uint32_t>(static_cast<uint64_t>(j + 1) * bins / w));
            float m = 0;
            for (uint32_t k = from; k < to; k++)
                m = std::max(m, mean[k]);
            pts[j] = m > 0 ? 10.0 * std::log10(m / count) : -300.0;
        }
    }
    m_spectra[ip] = out;
}

QVariantMap ChartDataHolder::getSpectrum(QString ip) {
    std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
    QVariantMap r;
    if (!m_spectra.contains(ip))
        return r;
    const auto& s = m_spectra[ip];
    r["n"] = static_cast<double>(s.n);
    r["bins"] = static_cast<double>(s.n / 2);
    r["avg"] = s.avg;
    QVariantList points, peakBin, peakDb, has;
    for (int c = 0; c < 4; c++) {
        points.append(QVariant::fromValue(s.points[c]));
        peakBin.append(s.peakBin[c]);
        peakDb.append(s.peakDb[c]);
        has.append(s.has[c]);
    }
    r["points"] = points;
    r["peakBin"] = peakBin;
    r["peakDb"] = peakDb;
    r["has"] = has;
    return r;
}

auto ChartDataHolder::setSpectrum(const QString& ip, bool on, int window, int average) -> void {
    {
        std::lock_guard<std::mutex> lock(m_add_mutex);
        m_spectrumSet[ip] = SpectrumSet{on, window, std::max(1, average)};
    }
    if (!on) {
        std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
        m_spectra.remove(ip);
        for (auto& h : m_history)
            h.remove(ip);
    }
}

auto ChartDataHolder::setRate(int perSecond) -> void {
    m_intervalMs = 1000 / qBound(1, perSecond, 10);
}

auto ChartDataHolder::setDecimation(const QString& ip, int samplesPerPoint) -> void {
    std::lock_guard<std::mutex> lock(m_add_mutex);
    m_decimation[ip] = std::max(0, samplesPerPoint);
}

auto ChartDataHolder::setPoints(int points) -> void {
    m_chartWidth = qBound(200, points, 4000);
}

QVector<qreal> ChartDataHolder::getChartSignal(QString ip, int index) {
    std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
    return m_chartPoints[index][ip];
}

bool ChartDataHolder::getChartNeedUpdate(QString ip) {
    std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
    return m_chartNeedDraw.contains(ip);
}

void ChartDataHolder::clearChartBuffer(QString ip) {
    std::lock_guard<std::mutex> lock(m_chartGetPointMutex);
    m_chartNeedDraw.removeAll(ip);
}
