#ifndef RECORDING_VIEW_H
#define RECORDING_VIEW_H

#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "view_source.h"

namespace converter_lib {
class CConverter;
}

// A recording opened in a viewer window (streaming_improvements 4.7): BIN, WAV, TDMS or CSV
// (view::Source). What it holds, the whole of it in a few thousand points (min and max), its
// losses and trigger waits, the samples of a window of it, the marks (found at a level or set by
// hand, kept beside it in <file>.marks.csv) and the export of a range. Positions are samples on
// the time line of the recording (the lost and waited samples stand on it too). What takes long
// (the open and the overview, a find) runs in a thread and says so in `busy`.
class RecordingView : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    // kind, rate (samples a second, 0 not known), decimation, adcRate, inputs (the ids), bits,
    // volts (the values are volts), total (samples on the time line), seconds, startNs, gapsFrom,
    // lost (samples per channel), waited, places (losses); ready: it can be drawn
    Q_PROPERTY(QVariantMap info READ info NOTIFY changed)
    // what runs and keeps the window waiting: what ("open", "find"), text, progress 0..1;
    // empty when nothing
    Q_PROPERTY(QVariantMap busy READ busy NOTIFY busyChanged)
    // buckets, lo and hi: a list a channel of values -1..1 (volts for a volts recording)
    Q_PROPERTY(QVariantMap overview READ overview NOTIFY overviewChanged)
    // the gaps: at (the first missing sample), lost, wait
    Q_PROPERTY(QVariantList gaps READ gaps NOTIFY overviewChanged)
    // the marks in their order: pos, input (an id, 0 none), edge (rising, falling, ""), from
    // (find, hand), name
    Q_PROPERTY(QVariantList marks READ marks NOTIFY marksChanged)
    // the export going on: running, progress, output; the last: message, ok
    Q_PROPERTY(QVariantMap exporting READ exporting NOTIFY exportingChanged)

   public:
    explicit RecordingView(QObject* parent = nullptr);
    ~RecordingView() override;

    // starts reading the file (a thread); changed() with info.ready, or error
    Q_INVOKABLE void open(const QString& path);
    // the samples of [from, to) for a plot of `width` points: mode "band" (lo and hi a point) or
    // "line" (values, one a sample, fewer than the points); channels: a list of {id, lo, hi} or
    // {id, values}
    Q_INVOKABLE QVariantMap window(double from, double to, int width);
    // the marks where input `id` goes through `level` (volts: the values times `scale` for codes)
    // on the edge ("rising", "falling", "both") with the hysteresis, not closer than `minGap`
    // samples, in [from, to); the marks found before go, those set by hand stay. "" when it
    // started, else why not
    Q_INVOKABLE QString find(int id, const QString& edge, double level, double hysteresis, double scale, double minGap, double from, double to);
    // stops what `busy` says (the open: the window closes)
    Q_INVOKABLE void stopBusy();
    Q_INVOKABLE void addMark(double pos, int id);
    Q_INVOKABLE void removeMark(int index);
    Q_INVOKABLE void renameMark(int index, const QString& name);
    Q_INVOKABLE void clearMarks();
    // the marks or the losses as a CSV (a file URL of a dialog); "" or why not
    Q_INVOKABLE QString exportTable(const QString& url, bool losses);
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
    // the marks and the gaps as plain arrays: a list read from a property is a reference that
    // reads the whole property again for every element (Qt 6), slow with thousands of them
    Q_INVOKABLE QVariantList marksList() { return marks(); }
    Q_INVOKABLE QVariantList gapsList() { return gaps(); }

    auto path() -> QString { return m_path; }
    auto error() -> QString { return m_error; }
    auto info() -> QVariantMap { return m_info; }
    auto busy() -> QVariantMap;
    auto overview() -> QVariantMap;
    auto gaps() -> QVariantList;
    auto marks() -> QVariantList;
    auto exporting() -> QVariantMap;

   signals:
    void changed();
    void busyChanged();
    void overviewChanged();
    void marksChanged();
    void exportingChanged();
    void exported(bool ok, QString output, QString message);
    // a find done: how many, or why it stopped
    void found(int count, QString message);

   private:
    struct Mark {
        uint64_t pos;
        int id;
        QString edge;
        QString from;
        QString name;
    };
    auto runOpen() -> void;
    auto runFind(int k, int edges, double level, double hyst, double scale, uint64_t minGap, uint64_t from, uint64_t to) -> void;
    auto runExport(uint64_t from, uint64_t to, QString format, std::vector<int> inputs, bool index, bool time, QString output) -> void;
    auto writeBinFromBin(uint64_t from, uint64_t to, const std::vector<int>& inputs, const QString& out, const std::function<void(double)>& progress) -> QString;
    auto writeBinFromSource(uint64_t from, uint64_t to, const std::vector<int>& inputs, const QString& out, const std::function<void(double)>& progress) -> QString;
    auto setBusy(const QString& what, const QString& text, double progress) -> void;
    auto cacheFile(const QString& kind) const -> QString;
    static auto pruneCache() -> void;
    auto loadMarks() -> void;
    auto saveMarks() -> void;

    QString m_path;
    QString m_error;
    QVariantMap m_info;
    std::shared_ptr<view::Source> m_src;
    std::shared_ptr<view::Source> m_pendingSrc;  // the open done, the window to take it (m_mtx)
    view::Info m_vi;
    std::vector<view::Gap> m_gaps;
    bool m_ready = false;  // the GUI thread: the open is done, the source is there
    std::atomic<bool> m_released{false};

    static constexpr int BUCKETS = 4096;
    std::mutex m_mtx;
    std::vector<std::vector<float>> m_lo, m_hi;  // an input, BUCKETS each
    bool m_overviewReady = false;
    QVariantMap m_busy;
    qint64 m_busySent = 0;
    std::thread m_openThread;
    std::atomic<bool> m_stop{false};

    std::vector<Mark> m_marks;
    std::thread m_findThread;
    std::atomic<bool> m_findStop{false};

    std::thread m_exportThread;
    std::atomic<bool> m_exportStop{false};
    QVariantMap m_export;
    std::shared_ptr<converter_lib::CConverter> m_conv;  // the converter of the export, to stop it
};

#endif
