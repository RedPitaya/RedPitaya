#ifndef CHARTDATAHOLDER_H
#define CHARTDATAHOLDER_H

#include <QMap>
#include <QObject>
#include <QVariantList>
#include <atomic>
#include <mutex>
#include <thread>
#include "data_lib/buffers_pack.h"

class ChartDataHolder : public QObject {
    Q_OBJECT
   public:
    static ChartDataHolder* instance();

    ChartDataHolder();
    ~ChartDataHolder();

    auto regRP(const QString& ip) -> void;
    auto removeRP(const QString& ip) -> void;
    auto addBuffer(DataLib::CDataBuffersPackDMA::Ptr pack, uint64_t id, const QString& ip, bool skip) -> void;

    Q_INVOKABLE QVector<qreal> getChartSignal(QString ip, int index);
    Q_INVOKABLE bool getChartNeedUpdate(QString ip);
    Q_INVOKABLE void clearChartBuffer(QString ip);
    // The measurements of the last block of a board (streaming_improvements 4.4), a map per
    // channel 1..4: valid, samples (of the block), min, max, mean, rms (of the full range, -1..1),
    // period (samples, 0: fewer than two periods), clipped (at an end of the range)
    Q_INVOKABLE QVariantList getMeasure(QString ip);
    // measured only for the boards that show them
    auto setMeasure(const QString& ip, bool on) -> void;
    // The spectrum of the last block of a board (streaming_improvements 4.5): n (FFT samples),
    // bins (n / 2), avg (the blocks averaged), points: a list a channel 1..4 of dB values of the
    // full range (dBV of 1 V RMS at 1 V full scale; max of the bins of each point), peakBin and
    // peakDb a channel. An empty map before the first spectrum.
    Q_INVOKABLE QVariantMap getSpectrum(QString ip);
    // window: dsp::Spectrum::Window (0 rectangular, 1 Hann, 4 flat top); average: blocks
    auto setSpectrum(const QString& ip, bool on, int window, int average) -> void;

    // blocks a second and points across of the preview
    auto setRate(int perSecond) -> void;
    auto setPoints(int points) -> void;
    // samples of the stream per point of the preview of a board, from the start of each block;
    // 0: the whole block (each point the mean of its share)
    auto setDecimation(const QString& ip, int samplesPerPoint) -> void;

   public slots:

   private:
    auto chartPackThread() -> void;

    std::mutex m_add_mutex;
    std::mutex m_chartGetPointMutex;
    QList<QString> m_rp;
    QMap<QString, DataLib::CDataBuffersPackDMA::Ptr> m_pack;
    QMap<QString, uint64_t> m_packId;
    QMap<QString, int> m_decimation;
    QMap<QString, bool> m_measureOn;
    struct Measure {
        bool valid = false;
        uint64_t samples = 0;
        double min = 0, max = 0, mean = 0, rms = 0, period = 0;
        bool clipped = false;
    };
    QMap<QString, QVector<Measure>> m_measures;  // under m_chartGetPointMutex
    static auto measure(const void* data, int bits, int count) -> Measure;
    struct SpectrumSet {
        bool on = false;
        int window = 1;
        int average = 1;
    };
    QMap<QString, SpectrumSet> m_spectrumSet;  // under m_add_mutex
    struct Spectrum {
        uint32_t n = 0;
        int avg = 0;
        QVector<qreal> points[4];
        int peakBin[4] = {0, 0, 0, 0};
        double peakDb[4] = {0, 0, 0, 0};
        bool has[4] = {false, false, false, false};
    };
    QMap<QString, Spectrum> m_spectra;  // under m_chartGetPointMutex
    // the power of the last blocks of each board and channel (the mean of them: Average)
    QMap<QString, std::vector<std::vector<float>>> m_history[4];
    struct Dsp;
    Dsp* m_dsp = nullptr;  // the chart thread only
    auto spectrumOf(const QString& ip, DataLib::CDataBuffersPackDMA::Ptr pack, const SpectrumSet& set, int width) -> void;
    // when the first block of the current frame came: a board that sends none (it waits for a
    // trigger) holds the others for one interval at most
    int64_t m_frameSince = 0;

    std::thread* m_chartThread;
    std::atomic<bool> m_IsWorkThread{false};
    std::atomic<int> m_chartWidth{1000};
    std::atomic<int> m_intervalMs{500};
    QMap<QString, QVector<qreal>> m_chartPoints[4];
    QList<QString> m_chartNeedDraw;
};

#endif  // CONSOLEMODEL_H
