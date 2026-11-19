#ifndef RECORDING_VIEW_H
#define RECORDING_VIEW_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "bin_lib/bin_file.h"

namespace converter_lib {
class CConverter;
}

// A BIN recording opened in a viewer window (streaming_improvements 4.7): what it holds, the
// whole of it in a few thousand points (min and max), its losses and trigger waits, the samples
// of a window of it, and the export of a range. Positions are samples on the time line of the
// recording (the lost and waited samples stand on it too).
class RecordingView : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    // rate (samples a second), decimation, adcRate, inputs (the ids of the ADC channels), bits,
    // volts (the samples are volts already), total (samples on the time line), seconds, startNs,
    // lost (samples per channel), waited, places (losses)
    Q_PROPERTY(QVariantMap info READ info NOTIFY changed)
    // 0..1 while the whole recording is read for the overview, 1 when it is there
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    // buckets, lo and hi: a list a channel of values -1..1 (volts for a volts recording)
    Q_PROPERTY(QVariantMap overview READ overview NOTIFY overviewChanged)
    // the gaps: at (the first missing sample), lost, wait
    Q_PROPERTY(QVariantList gaps READ gaps NOTIFY overviewChanged)
    // the export going on: running, progress, output; the last: message, ok
    Q_PROPERTY(QVariantMap exporting READ exporting NOTIFY exportingChanged)

   public:
    explicit RecordingView(QObject* parent = nullptr);
    ~RecordingView() override;

    Q_INVOKABLE bool open(const QString& path);
    // the samples of [from, to) for a plot of `width` points: mode "band" (lo and hi a point) or
    // "line" (values, one a sample, fewer than the points); channels: a list of {id, lo, hi} or
    // {id, values}
    Q_INVOKABLE QVariantMap window(double from, double to, int width);
    // [from, to) to CSV, WAV, TDMS or BIN; inputs: ids; index, time: CSV columns; "" when it
    // started, else why not
    Q_INVOKABLE QString exportRange(double from, double to, const QString& format, const QVariantList& inputs, bool index, bool time, const QString& output);
    Q_INVOKABLE void stopExport();
    // the file name of an export of [from, to): beside the recording, the range in its name
    Q_INVOKABLE QString exportName(double from, double to, const QString& format);
    // the file is still there (a viewer of a deleted one closes)
    Q_INVOKABLE bool exists();
    // the threads stopped and the file closed: before the window goes or the file is deleted
    Q_INVOKABLE void release();

    auto path() -> QString { return m_path; }
    auto error() -> QString { return m_error; }
    auto info() -> QVariantMap { return m_info; }
    auto progress() -> double { return m_progress; }
    auto overview() -> QVariantMap;
    auto gaps() -> QVariantList;
    auto exporting() -> QVariantMap;

   signals:
    void changed();
    void progressChanged();
    void overviewChanged();
    void exportingChanged();
    void exported(bool ok, QString output, QString message);

   private:
    struct Gap {
        uint64_t at;
        uint64_t lost;
        uint64_t wait;
    };
    auto buildOverview() -> void;
    auto runExport(uint64_t from, uint64_t to, QString format, std::vector<uint8_t> channels, bool index, bool time, QString output) -> void;
    auto sampleOf(const uint8_t* data, size_t i, bin_lib::SampleType type) const -> float;

    QString m_path;
    QString m_error;
    QVariantMap m_info;
    bin_lib::FileReader m_reader;    // the GUI thread: the windows
    std::vector<uint8_t> m_adc;      // the indexes of the ADC channels in the file
    uint64_t m_total = 0;
    double m_rate = 0;

    static constexpr int BUCKETS = 4096;
    std::mutex m_mtx;
    std::vector<std::vector<float>> m_lo, m_hi;  // a channel of m_adc, BUCKETS each
    std::vector<Gap> m_gaps;
    bool m_overviewReady = false;
    std::atomic<double> m_progress{0};
    std::thread m_overviewThread;
    std::atomic<bool> m_stop{false};

    std::thread m_exportThread;
    std::atomic<bool> m_exportStop{false};
    QVariantMap m_export;
    std::shared_ptr<converter_lib::CConverter> m_conv;  // the converter of the export, to stop it
};

#endif
