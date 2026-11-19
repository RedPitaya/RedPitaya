#include "recordings.h"
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QStorageInfo>
#include <QTextStream>
#include <QUrl>
#include <algorithm>
#include "bin_lib/bin_file.h"
#include "converter_lib/converter.h"
#include "src/app_settings.h"
#include "src/models/board.h"
#include "src/models/boardsmodel.h"

namespace {

const QStringList kFormats = {"bin", "wav", "tdms", "csv", "vcd"};
const QStringList kCompanions = {".log.txt", ".log.lost.txt", ".console.txt"};
const char* const kAddedKey = "recordingFolders";

auto settings() -> QSettings {
    return QSettings("RedPitaya", "rpsa_client_qt");
}

auto native(const QString& path) -> QString {
    return QDir::toNativeSeparators(QDir::cleanPath(path));
}

// "rp-f01c35_2026-11-19_10-06-01": the time of the name (a Start all gives it to every board)
const QRegularExpression kTime("(\\d{4})-(\\d{2})-(\\d{2})_(\\d{2})-(\\d{2})-(\\d{2})");

// the report of a recording (<file>.log.txt, writer_lib CFileLogger)
auto readReport(const QString& path, QVariantMap& d) -> void {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&f);
    QVariantList inputs;
    int channel = 0;
    bool samplesNext = false;
    while (!in.atEnd()) {
        const QString line = in.readLine();
        const QString t = line.trimmed();
        auto value = [&](const char* key) -> QString { return t.startsWith(key) ? t.mid(static_cast<int>(strlen(key))).trimmed() : QString(); };
        if (!value("Sample rate:").isEmpty())
            d["rate"] = value("Sample rate:").toDouble();
        else if (!value("Decimation:").isEmpty())
            d["decimation"] = value("Decimation:").toDouble();
        else if (!value("ADC rate:").isEmpty())
            d["adcRate"] = value("ADC rate:").toDouble();
        else if (!value("Trigger mode:").isEmpty())
            d["trigger"] = value("Trigger mode:");
        else if (!value("Buffers:").isEmpty())
            d["blocks"] = value("Buffers:").toDouble();
        else if (t.startsWith("Lost data due to file write buffer overflow:"))
            d["lostPcBlocks"] = t.section(':', 1).trimmed().section(' ', 0, 0).toDouble();
        else if (t.contains("transmitted on: Channel")) {
            channel = t.section("Channel", 1).trimmed().toInt();
            inputs.append(channel);
            samplesNext = true;
        } else if (samplesNext && t.endsWith("Samples")) {
            if (!d.contains("samples"))
                d["samples"] = t.mid(1).section(' ', 0, 0).toDouble();
            samplesNext = false;
        } else if (t.startsWith("- FPGA:") && inputs.size() == 1) {
            d["lostBoard"] = t.section(':', 1).trimmed().section(' ', 0, 0).toDouble();
        }
    }
    if (!inputs.isEmpty())
        d["inputs"] = inputs;
}

// the losses block by block (<file>.log.lost.txt): how many places of the first channel and the
// first of them, in samples from the start
auto readLost(const QString& path, QVariantMap& d) -> void {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    static const QRegularExpression row("^Channel (\\d+): Pos\\s+(\\d+)\\s+Get:\\s+(\\d+)\\s+\\((\\d+)\\)");
    QTextStream in(&f);
    QString first;
    int places = 0;
    double firstAt = -1;
    while (!in.atEnd()) {
        const auto m = row.match(in.readLine());
        if (!m.hasMatch())
            continue;
        if (first.isEmpty())
            first = m.captured(1);
        if (m.captured(1) != first)
            continue;
        const double lost = m.captured(4).toDouble();
        if (lost > 0) {
            places++;
            if (firstAt < 0)
                firstAt = std::max(0.0, m.captured(2).toDouble() - lost);
        }
    }
    d["lostPlaces"] = places;
    if (firstAt >= 0)
        d["firstLostSample"] = firstAt;
}

// what the BIN header says (bin_lib)
auto readBin(const QString& path, QVariantMap& d) -> void {
    bin_lib::FileReader reader;
    if (!reader.open(path.toStdString()))
        return;
    const auto& info = reader.info();
    const double dec = info.decimation ? info.decimation : 1;
    if (info.clock) {
        d["adcRate"] = static_cast<double>(info.clock);
        d["decimation"] = dec;
        if (!d.contains("rate"))
            d["rate"] = static_cast<double>(info.clock) / dec;
    }
    QVariantList inputs;
    int bits = 0;
    bool volts = false, gpio = false;
    for (const auto& ch : info.channels) {
        if (ch.type == bin_lib::SampleType::GPIO_IN_WORD || ch.type == bin_lib::SampleType::GPIO_OUT_WORD)
            gpio = true;
        else
            inputs.append(ch.id);
        bits = std::max(bits, static_cast<int>(bin_lib::sampleBytes(ch.type) * 8));
        volts |= ch.type == bin_lib::SampleType::FLOAT32;
    }
    if (!inputs.isEmpty() && !d.contains("inputs"))
        d["inputs"] = inputs;
    d["gpio"] = gpio;
    d["bits"] = volts ? 0 : bits;
    d["values"] = gpio ? "GPIO words" : volts ? "volts" : "codes";
    if (!d.contains("trigger"))
        d["trigger"] = (info.flags & bin_lib::FILE_FLAG_TRIGGER_MODE) ? "on" : "off";
    if (info.startTime)
        d["startNs"] = static_cast<double>(info.startTime);
}

// the WAV header: channels, rate, bits, data
auto readWav(const QString& path, QVariantMap& d) -> void {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return;
    const QByteArray h = f.read(44);
    if (h.size() < 44 || !h.startsWith("RIFF"))
        return;
    auto u16 = [&](int o) { return static_cast<uint32_t>(static_cast<uint8_t>(h[o]) | (static_cast<uint8_t>(h[o + 1]) << 8)); };
    auto u32 = [&](int o) { return u16(o) | (u16(o + 2) << 16); };
    const uint32_t channels = u16(22), rate = u32(24), bits = u16(34);
    if (!channels || !rate || !bits)
        return;
    if (!d.contains("rate"))
        d["rate"] = static_cast<double>(rate);
    d["bits"] = static_cast<double>(bits);
    if (!d.contains("samples"))
        d["samples"] = static_cast<double>(f.size() - 44) / (channels * (bits / 8));
    if (!d.contains("inputs")) {
        QVariantList inputs;
        for (uint32_t i = 1; i <= channels; i++)
            inputs.append(static_cast<int>(i));
        d["inputs"] = inputs;
    }
}

}  // namespace

RecordingsManager* RecordingsManager::instance() {
    static RecordingsManager* s = new RecordingsManager();
    return s;
}

RecordingsManager::RecordingsManager() : QObject(nullptr) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    m_liveTimer.setInterval(2000);
    connect(&m_liveTimer, &QTimer::timeout, this, &RecordingsManager::rescan);
}

RecordingsManager::~RecordingsManager() {
    stopConvert();
    if (m_jobThread.joinable())
        m_jobThread.join();
}

// the application's folder (the part before {board}, and its folders), the folder of every board,
// the folders added
auto RecordingsManager::scanFolders() -> QList<QPair<QString, QPair<QString, QString>>> {
    QList<QPair<QString, QPair<QString, QString>>> r;
    QSet<QString> seen;
    auto add = [&](const QString& dir, const QString& kind, const QString& board) {
        const QString d = QDir::cleanPath(dir);
        if (d.isEmpty() || seen.contains(d))
            return;
        seen.insert(d);
        r.append({d, {kind, board}});
    };
    QString app = AppSettings::instance()->recordDir();
    const int at = app.indexOf("{board}");
    if (at >= 0) {
        const QString base = QDir::cleanPath(AppSettings::expandHome(app.left(at)));
        add(base, "app", "");
        for (const auto& sub : QDir(base).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
            add(sub.absoluteFilePath(), "app", sub.fileName());
    } else {
        add(AppSettings::expandHome(app), "app", "");
    }
    auto* boards = BoardsModel::instance();
    for (int i = 0; i < boards->rowCount(); i++)
        if (auto* b = qobject_cast<CBoard*>(boards->board(i)))
            add(QDir::fromNativeSeparators(b->recordDir()), "board", b->getName());
    for (const auto& d : settings().value(kAddedKey).toStringList())
        add(d, "added", "");
    return r;
}

auto RecordingsManager::folders() -> QVariantList {
    QVariantList r;
    QSet<QString> shown;
    for (const auto& f : scanFolders()) {
        // the folders of the boards under the application's folder are the application's
        if (f.second.first == "app" && !f.second.second.isEmpty())
            continue;
        if (shown.contains(f.first))
            continue;
        shown.insert(f.first);
        r.append(QVariantMap{{"path", native(f.first)}, {"kind", f.second.first}, {"board", f.second.second}});
    }
    return r;
}

void RecordingsManager::rescan() {
    // the files the boards write now
    QSet<QString> live;
    bool anyLive = false;
    auto* boards = BoardsModel::instance();
    for (int i = 0; i < boards->rowCount(); i++) {
        auto* b = qobject_cast<CBoard*>(boards->board(i));
        if (!b || !b->getIsADCStarted())
            continue;
        const QString f = b->getStats().value("file").toString();
        if (!f.isEmpty()) {
            live.insert(native(f));
            anyLive = true;
        }
    }
    QVariantList list;
    QSet<QString> files;
    for (const auto& folder : scanFolders()) {
        const QDir dir(folder.first);
        for (const auto& fi : dir.entryInfoList(QDir::Files)) {
            const QString suffix = fi.suffix().toLower();
            if (!kFormats.contains(suffix))
                continue;
            const QString path = native(fi.absoluteFilePath());
            if (files.contains(path))
                continue;
            files.insert(path);
            QString board = folder.second.second;
            const auto m = kTime.match(fi.fileName());
            if (board.isEmpty()) {
                // "<board>_<date>_<time>" of the client, "data_file_<ip>_<date>_<time>" of rpsa_client
                board = m.hasMatch() ? fi.fileName().left(m.capturedStart()).chopped(m.capturedStart() > 0 ? 1 : 0) : QString();
                if (board.startsWith("data_file_"))
                    board = board.mid(10);
            }
            QDateTime started = m.hasMatch() ? QDateTime(QDate(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt()),
                                                         QTime(m.captured(4).toInt(), m.captured(5).toInt(), m.captured(6).toInt()))
                                             : fi.birthTime();
            if (!started.isValid())
                started = fi.lastModified();
            // what the list shows of it: the report (a few lines) and the header, not the data
            // (the report and the header again only when the file or its report changed)
            const QFileInfo report(path + ".log.txt");
            const qint64 stamp = fi.lastModified().toMSecsSinceEpoch() ^ (report.exists() ? report.lastModified().toMSecsSinceEpoch() << 1 : 0);
            auto& cached = m_quick[path];
            if (cached.modified != stamp || cached.size != fi.size()) {
                QVariantMap fresh;
                readReport(path + ".log.txt", fresh);
                if (suffix == "bin")
                    readBin(path, fresh);
                else if (suffix == "wav")
                    readWav(path, fresh);
                cached = Quick{stamp, fi.size(), fresh};
            }
            const QVariantMap& q = cached.q;
            const double rate = q.value("rate").toDouble();
            list.append(QVariantMap{{"seconds", rate > 0 && q.contains("samples") ? q["samples"].toDouble() / rate : -1.0},
                                    {"inputs", q.value("inputs")},
                                    {"lost", q.contains("lostBoard") ? q["lostBoard"].toDouble() : -1.0},
                                    {"path", path},
                                    {"name", fi.fileName()},
                                    {"dir", native(fi.absolutePath())},
                                    {"board", board},
                                    {"format", suffix.toUpper()},
                                    {"size", static_cast<double>(fi.size())},
                                    {"started", static_cast<double>(started.toMSecsSinceEpoch())},
                                    {"run", m.hasMatch() ? m.captured(0) : QString()},
                                    {"recording", live.contains(path)},
                                    {"convertible", suffix == "bin"}});
        }
    }
    for (auto it = m_quick.begin(); it != m_quick.end();)
        it = files.contains(it.key()) ? std::next(it) : m_quick.erase(it);
    m_list = list;
    const auto folders = scanFolders();
    const QStorageInfo disk(folders.isEmpty() ? QDir::homePath() : folders.first().first);
    m_diskFree = disk.isValid() ? static_cast<double>(disk.bytesAvailable()) : 0;
    Q_EMIT listChanged();
    // while a board writes, again and again
    if (m_liveTimer.isActive() != anyLive && property("live").toBool()) {
        if (anyLive)
            m_liveTimer.start();
        else
            m_liveTimer.stop();
    }
}

void RecordingsManager::setLive(bool on) {
    setProperty("live", on);
    if (!on)
        m_liveTimer.stop();
    else
        rescan();
}

QStringList RecordingsManager::companions(const QString& path) {
    QStringList r;
    for (const auto& c : kCompanions)
        if (QFileInfo::exists(path + c))
            r.append(path + c);
    return r;
}

void RecordingsManager::details(const QString& path) {
    const QStringList extra = companions(path);
    std::thread([this, path, extra]() {
        QVariantMap d;
        readReport(path + ".log.txt", d);
        readLost(path + ".log.lost.txt", d);
        const QString suffix = QFileInfo(path).suffix().toLower();
        if (suffix == "bin")
            readBin(path, d);
        else if (suffix == "wav")
            readWav(path, d);
        const double rate = d.value("rate").toDouble();
        if (rate > 0 && d.contains("samples"))
            d["seconds"] = d["samples"].toDouble() / rate;
        if (rate > 0 && d.contains("firstLostSample"))
            d["firstLost"] = d["firstLostSample"].toDouble() / rate;
        QVariantList files;
        files.append(QVariantMap{{"path", path}, {"kind", QFileInfo(path).suffix().toLower()}, {"size", static_cast<double>(QFileInfo(path).size())}});
        for (const auto& c : extra)
            files.append(QVariantMap{{"path", c}, {"kind", c.mid(path.size() + 1)}, {"size", static_cast<double>(QFileInfo(c).size())}});
        d["files"] = files;
        QMetaObject::invokeMethod(this, [this, path, d]() { Q_EMIT detailsReady(path, d); }, Qt::QueuedConnection);
    }).detach();
}

QString RecordingsManager::outputFor(const QString& path, const QString& format) {
    const QFileInfo fi(path);
    return native(fi.absolutePath() + "/" + fi.completeBaseName() + "." + format.toLower());
}

QString RecordingsManager::convert(const QStringList& paths, const QVariantMap& options) {
    const QString format = options.value("format").toString().toUpper();
    if (!QStringList{"CSV", "WAV", "TDMS", "VCD"}.contains(format))
        return "Not a format of the converter: " + format;
    QList<Job> jobs;
    for (const auto& p : paths) {
        if (QFileInfo(p).suffix().toLower() != "bin")
            return QFileInfo(p).fileName() + " is not a BIN recording: only those convert";
        const QString out = paths.size() == 1 && !options.value("output").toString().isEmpty() ? options.value("output").toString() : outputFor(p, format);
        if (QFileInfo::exists(out))
            return native(out) + " is there: give another name or delete it first";
        jobs.append(Job{p, out, options});
    }
    bool start = false;
    {
        std::lock_guard lock(m_jobMtx);
        m_jobs.append(jobs);
        start = !m_jobRunning;
        m_jobRunning = true;
    }
    if (start) {
        // the thread of the jobs before has left its loop (m_jobRunning false): it ends at once
        if (m_jobThread.joinable())
            m_jobThread.join();
        m_jobThread = std::thread(&RecordingsManager::runJobs, this);
    }
    Q_EMIT jobChanged();
    return "";
}

auto RecordingsManager::job() -> QVariantMap {
    std::lock_guard lock(m_jobMtx);
    QVariantMap j = m_job;
    if (!j.isEmpty())
        j["queued"] = static_cast<int>(m_jobs.size());
    return j;
}

auto RecordingsManager::runJobs() -> void {
    while (true) {
        Job job;
        std::shared_ptr<converter_lib::CConverter> conv;
        {
            std::lock_guard lock(m_jobMtx);
            if (m_jobs.isEmpty()) {
                m_job.clear();
                m_converter = nullptr;
                m_jobRunning = false;
                break;
            }
            job = m_jobs.takeFirst();
            m_job = QVariantMap{{"path", job.path}, {"output", native(job.output)}, {"progress", 0.0}};
            conv = std::make_shared<converter_lib::CConverter>();
            m_converter = conv;
        }
        QMetaObject::invokeMethod(this, [this]() { Q_EMIT jobChanged(); }, Qt::QueuedConnection);
        uint32_t mask = 0;
        for (const auto& i : job.options.value("inputs").toList())
            if (i.toInt() >= 1 && i.toInt() <= 4)
                mask |= 1u << (i.toInt() - 1);
        conv->setOutput(job.output.toStdString());
        conv->setChannels(mask);
        conv->setOverwrite(false);
        conv->setCSVHeader(true);
        auto last = std::make_shared<double>(-1);
        conv->setProgress([this, last](double p) {
            // a few times a second is enough for the bar
            if (p - *last < 0.01 && p < 1)
                return;
            *last = p;
            {
                std::lock_guard lock(m_jobMtx);
                m_job["progress"] = p;
            }
            QMetaObject::invokeMethod(this, [this]() { Q_EMIT jobChanged(); }, Qt::QueuedConnection);
        });
        const QString format = job.options.value("format").toString().toUpper();
        const std::string in = job.path.toStdString();
        bool ok = false;
        if (format == "CSV") {
            int mode = FH_CSV_NONE;
            if (job.options.value("index").toBool())
                mode |= FH_CSV_ADD_INDEX;
            if (job.options.value("timeNs").toBool())
                mode |= FH_CSV_ADD_TIME_COL_NS;
            else if (job.options.value("time").toBool())
                mode |= FH_CSV_ADD_TIME_COL;
            ok = conv->convertToCSV(in, "", static_cast<FH_CSVMode>(mode));
        } else if (format == "WAV") {
            ok = conv->convertToWAV(in, "");
        } else if (format == "TDMS") {
            ok = conv->convertToTDMS(in, "");
        } else {
            ok = conv->convertToVCD(in, "");
        }
        const QString error = ok ? QString() : QString::fromStdString(conv->lastError().empty() ? "stopped" : conv->lastError());
        const QString output = native(job.output);
        QMetaObject::invokeMethod(
            this,
            [this, path = job.path, ok, output, error]() {
                Q_EMIT converted(path, ok, output, error);
                rescan();
            },
            Qt::QueuedConnection);
    }
    QMetaObject::invokeMethod(this, [this]() { Q_EMIT jobChanged(); }, Qt::QueuedConnection);
}

void RecordingsManager::stopConvert() {
    std::lock_guard lock(m_jobMtx);
    m_jobs.clear();
    if (m_converter) {
        m_converter->stopWriteToCSV();
        m_converter->stopWriteToWAV();
        m_converter->stopWriteToTDMS();
    }
}

QString RecordingsManager::remove(const QStringList& paths) {
    QStringList failed;
    for (const auto& p : paths) {
        bool live = false;
        for (const auto& r : m_list)
            if (r.toMap().value("path").toString() == p && r.toMap().value("recording").toBool())
                live = true;
        if (live) {
            failed.append(QFileInfo(p).fileName() + " (a board writes it)");
            continue;
        }
        for (const auto& f : QStringList{p} + companions(p))
            if (!QFile::remove(f))
                failed.append(QFileInfo(f).fileName());
    }
    rescan();
    return failed.isEmpty() ? QString() : "Not deleted: " + failed.join(", ");
}

void RecordingsManager::showInFolder(const QString& path) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void RecordingsManager::copyPath(const QString& path) {
    if (auto* c = QGuiApplication::clipboard())
        c->setText(native(path));
}

void RecordingsManager::openFile(const QString& path) {
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

QString RecordingsManager::addFolder(const QString& path) {
    const QString d = QDir::cleanPath(AppSettings::expandHome(path.trimmed()));
    if (d.isEmpty() || !QDir(d).exists())
        return "No such folder: " + path;
    auto s = settings();
    QStringList added = s.value(kAddedKey).toStringList();
    if (!added.contains(d))
        added.append(d);
    s.setValue(kAddedKey, added);
    rescan();
    return "";
}

void RecordingsManager::removeFolder(const QString& path) {
    auto s = settings();
    QStringList added = s.value(kAddedKey).toStringList();
    added.removeAll(QDir::cleanPath(QDir::fromNativeSeparators(path)));
    s.setValue(kAddedKey, added);
    rescan();
}
