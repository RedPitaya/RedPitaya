#include "chartdataholder.h"
#include <algorithm>
#include <chrono>
#include <QQmlEngine>
#include <QThread>

namespace {
auto nowMs() -> int64_t {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
}  // namespace

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
