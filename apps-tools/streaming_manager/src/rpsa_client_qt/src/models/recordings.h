#ifndef RECORDINGS_H
#define RECORDINGS_H

#include <QHash>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

namespace converter_lib {
class CConverter;
}

// The recordings on this computer (streaming_improvements 4.7, the manager first): the files of
// the boards in the folders of the application, of the boards and the ones added, what they hold
// (their report and header), and what is done with them: convert, delete, show.
class RecordingsManager : public QObject {
    Q_OBJECT
    // a map a recording: path, name, dir, board, format (BIN, WAV, TDMS, CSV, VCD), size, started
    // (ms since the epoch, from the name or the file), run (the time of its name: a Start all has
    // it on several boards), recording (a board writes it now), convertible (a BIN recording)
    Q_PROPERTY(QVariantList list READ list NOTIFY listChanged)
    // the folders looked in: path, kind (app, board, added), board
    Q_PROPERTY(QVariantList folders READ folders NOTIFY listChanged)
    Q_PROPERTY(double diskFree READ diskFree NOTIFY listChanged)
    // the conversion going on: path (the input), output, progress (0..1), queued (more after it);
    // empty when none
    Q_PROPERTY(QVariantMap job READ job NOTIFY jobChanged)

   public:
    static RecordingsManager* instance();
    ~RecordingsManager();

    auto list() -> QVariantList { return m_list; }
    auto folders() -> QVariantList;
    auto diskFree() -> double { return m_diskFree; }
    auto job() -> QVariantMap;

    // looks in the folders again; live: while a board records, again every 2 s
    Q_INVOKABLE void rescan();
    Q_INVOKABLE void setLive(bool on);
    // what a recording holds, in its own thread: detailsReady(path, map) with rate, decimation,
    // adcRate, samples (per channel), seconds, inputs (list of numbers), bits, values (codes or
    // volts), trigger, blocks, lostBoard, lostPc, lostPlaces, firstLost (seconds), files (the files
    // of the recording: path, kind, size)
    Q_INVOKABLE void details(const QString& path);
    // BIN to CSV, WAV, TDMS (a GPIO recording: CSV, VCD), one after the other: format, inputs (list
    // of 1..4), index, time, timeNs (CSV columns), output (the file, "" - beside the input with the
    // extension of the format; for several the name of each is its own). "" when queued, else why not.
    Q_INVOKABLE QString convert(const QStringList& paths, const QVariantMap& options);
    Q_INVOKABLE void stopConvert();
    // the file names a conversion would write, and whether they are there
    Q_INVOKABLE QString outputFor(const QString& path, const QString& format);
    // the recording and its files (.log.txt, .log.lost.txt, .console.txt); "" or what failed
    Q_INVOKABLE QString remove(const QStringList& paths);
    Q_INVOKABLE QStringList companions(const QString& path);
    Q_INVOKABLE void showInFolder(const QString& path);
    Q_INVOKABLE void copyPath(const QString& path);
    Q_INVOKABLE void openFile(const QString& path);
    Q_INVOKABLE QString addFolder(const QString& path);
    Q_INVOKABLE void removeFolder(const QString& path);

   signals:
    void listChanged();
    void jobChanged();
    void detailsReady(QString path, QVariantMap details);
    // a conversion ended: ok, the file written, why not
    void converted(QString path, bool ok, QString output, QString error);

   private:
    RecordingsManager();
    auto scanFolders() -> QList<QPair<QString, QPair<QString, QString>>>;  // path, kind, board
    auto runJobs() -> void;

    QVariantList m_list;
    // what the list reads of a file (its report, its header), kept while the file is the same
    struct Quick {
        qint64 modified = 0;
        qint64 size = -1;
        QVariantMap q;
    };
    QHash<QString, Quick> m_quick;
    double m_diskFree = 0;
    QTimer m_liveTimer;

    struct Job {
        QString path;
        QString output;
        QVariantMap options;
    };
    std::mutex m_jobMtx;
    QList<Job> m_jobs;
    QVariantMap m_job;
    std::thread m_jobThread;
    bool m_jobRunning = false;  // under m_jobMtx: a job queued finds the thread or starts it
    std::shared_ptr<converter_lib::CConverter> m_converter;
};

#endif
