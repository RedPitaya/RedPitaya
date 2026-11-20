#include "recording_view.h"
#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>
#include "bin_lib/bin_file.h"
#include "converter_lib/converter.h"

namespace {

constexpr uint64_t kChunk = 1 << 20;
constexpr size_t kMaxMarks = 100000;

auto localPath(const QString& url) -> QString {
    const QUrl u(url);
    return u.isLocalFile() ? u.toLocalFile() : url;
}

auto bytesText(double b) -> QString {
    if (b >= 1073741824)
        return QString::number(b / 1073741824, 'f', 2) + " GB";
    if (b >= 1048576)
        return QString::number(b / 1048576, 'f', 1) + " MB";
    return QString::number(b / 1024, 'f', 0) + " kB";
}

auto now() -> qint64 {
    return QDateTime::currentMSecsSinceEpoch();
}

}  // namespace

RecordingView::RecordingView(QObject* parent) : QObject(parent) {}

RecordingView::~RecordingView() {
    release();
}

void RecordingView::release() {
    m_released = true;
    m_stop = true;
    m_findStop = true;
    stopExport();
    if (m_openThread.joinable())
        m_openThread.join();
    if (m_findThread.joinable())
        m_findThread.join();
    if (m_exportThread.joinable())
        m_exportThread.join();
    // Windows deletes no file that is open: the source of the window, and one the open handed
    // over that the window has not taken yet
    if (m_src)
        m_src->close();
    m_src.reset();
    {
        std::lock_guard lock(m_mtx);
        if (m_pendingSrc)
            m_pendingSrc->close();
        m_pendingSrc.reset();
    }
    m_ready = false;
}

auto RecordingView::setBusy(const QString& what, const QString& text, double progress) -> void {
    {
        std::lock_guard lock(m_mtx);
        m_busy = what.isEmpty() ? QVariantMap() : QVariantMap{{"what", what}, {"text", text}, {"progress", progress}};
        // a few times a second is enough for a progress bar
        const qint64 t = now();
        if (!what.isEmpty() && t - m_busySent < 100)
            return;
        m_busySent = t;
    }
    QMetaObject::invokeMethod(this, [this]() { Q_EMIT busyChanged(); }, Qt::QueuedConnection);
}

auto RecordingView::busy() -> QVariantMap {
    std::lock_guard lock(m_mtx);
    return m_busy;
}

// the cache of the viewer (overviews, the rows of a CSV): what was not used for 60 days goes
auto RecordingView::pruneCache() -> void {
    const QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/viewer");
    const QDateTime old = QDateTime::currentDateTime().addDays(-60);
    for (const auto& fi : dir.entryInfoList(QDir::Files))
        if (fi.lastModified() < old && fi.lastRead() < old)
            QFile::remove(fi.absoluteFilePath());
}

auto RecordingView::cacheFile(const QString& kind) const -> QString {
    const QFileInfo fi(m_path);
    const QByteArray key = (fi.absoluteFilePath() + "|" + QString::number(fi.size()) + "|" + QString::number(fi.lastModified().toMSecsSinceEpoch())).toUtf8();
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/viewer";
    QDir().mkpath(dir);
    return dir + "/" + QString::fromLatin1(QCryptographicHash::hash(key, QCryptographicHash::Sha1).toHex()) + "." + kind;
}

void RecordingView::open(const QString& path) {
    // again (a delete that failed): what the open before left goes first
    m_stop = true;
    if (m_openThread.joinable())
        m_openThread.join();
    {
        std::lock_guard lock(m_mtx);
        m_pendingSrc.reset();
        m_overviewReady = false;
    }
    if (m_src)
        m_src->close();
    m_src.reset();
    m_ready = false;
    m_released = false;
    m_path = path;
    m_info = QVariantMap{{"name", QFileInfo(path).fileName()}, {"ready", false}};
    m_error.clear();
    m_stop = false;
    Q_EMIT changed();
    setBusy("open", "Opening", 0);
    m_openThread = std::thread(&RecordingView::runOpen, this);
}

// The file read (its index; a CSV whole), then the whole recording once for the overview; both
// kept in the cache of the application, so the next open is at once
auto RecordingView::runOpen() -> void {
    const auto t0 = std::chrono::steady_clock::now();
    const double size = static_cast<double>(QFileInfo(m_path).size());
    auto fail = [this](const QString& e) {
        setBusy("", "", 0);
        QMetaObject::invokeMethod(
            this,
            [this, e]() {
                // released meanwhile (the window closed): nothing to say
                if (m_released)
                    return;
                m_error = e;
                Q_EMIT changed();
                Q_EMIT busyChanged();
            },
            Qt::QueuedConnection);
    };
    // "Reading the rows · 1.2 of 3.1 GB · 20 s left"
    auto report = [this, t0, size](const QString& what, double p, double from, double weight) {
        const double all = from + p * weight;
        const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        QString text = what + " · " + bytesText(size * p) + " of " + bytesText(size);
        if (all > 0.02 && s > 1)
            text += " · " + QString::number(std::max(1.0, std::round(s / all * (1 - all)))) + " s left";
        setBusy("open", text, all);
        return !m_stop;
    };
    std::shared_ptr<view::Source> src(view::Source::create(m_path).release());
    if (!src)
        return fail("Not a format the viewer reads (BIN, WAV, TDMS, CSV)");
    // a recording that got no buffer (a trigger that never came, a stop at once): its report says why
    if (QFileInfo(m_path).size() == 0)
        return fail("The recording is empty: no data came (a trigger that did not come, or a stop before the first buffer). Its report (.log.txt) tells more.");
    const bool csv = QFileInfo(m_path).suffix().toLower() == "csv";
    bool loaded = false;
    if (csv) {
        QFile c(cacheFile("rows"));
        if (c.open(QIODevice::ReadOnly)) {
            QDataStream in(&c);
            loaded = src->loadIndex(m_path, in);
        }
        if (!loaded)
            src = std::shared_ptr<view::Source>(view::Source::create(m_path).release());
    }
    if (!loaded) {
        const QString what = csv ? "Reading the rows" : "Step 1 of 2, the index of the blocks";
        const double weight = csv ? 1.0 : 0.3;
        if (!src->open(m_path, [&](double p) { return report(what, p, 0, weight); }))
            return fail(m_stop ? "stopped" : src->error());
        if (csv) {
            QFile c(cacheFile("rows"));
            if (c.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                QDataStream out(&c);
                src->saveIndex(out);
            }
        }
    }
    const size_t nc = src->info().ids.size();
    std::vector<std::vector<float>> lo, hi;
    bool cached = false;
    {
        QFile c(cacheFile("overview"));
        if (c.open(QIODevice::ReadOnly)) {
            QDataStream in(&c);
            quint32 magic = 0, buckets = 0, n = 0;
            in >> magic >> buckets >> n;
            if (magic == 0x52505632 && buckets == BUCKETS && n == nc) {
                lo.resize(n);
                hi.resize(n);
                for (quint32 k = 0; k < n; k++) {
                    QVector<float> l, h;
                    in >> l >> h;
                    lo[k].assign(l.begin(), l.end());
                    hi[k].assign(h.begin(), h.end());
                }
                cached = in.status() == QDataStream::Ok;
            }
        }
    }
    if (!cached) {
        const double from = csv ? 1.0 : 0.3;
        if (!src->overview(BUCKETS, lo, hi, [&](double p) { return report(csv ? "The whole recording" : "Step 2 of 2, the whole recording", p, from, 1.0 - from); }))
            return fail("stopped");
        QFile c(cacheFile("overview"));
        if (c.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QDataStream out(&c);
            out << quint32(0x52505632) << quint32(BUCKETS) << quint32(nc);
            for (size_t k = 0; k < nc; k++)
                out << QVector<float>(lo[k].begin(), lo[k].end()) << QVector<float>(hi[k].begin(), hi[k].end());
        }
    }
    {
        std::lock_guard lock(m_mtx);
        m_lo = std::move(lo);
        m_hi = std::move(hi);
        m_overviewReady = true;
    }
    pruneCache();
    setBusy("", "", 0);
    // the UI script of the tests: how long it took
    if (qEnvironmentVariableIsSet("RPSA_QT_UI"))
        qInfo("viewer: %s opened in %.0f ms (overview %s)", qPrintable(QFileInfo(m_path).fileName()),
              std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count(), cached ? "from the cache" : "read");
    // handed over through a member, so that release() can close it before the window takes it
    {
        std::lock_guard lock(m_mtx);
        m_pendingSrc = src;
    }
    src.reset();
    QMetaObject::invokeMethod(
        this,
        [this]() {
            std::shared_ptr<view::Source> src;
            {
                std::lock_guard lock(m_mtx);
                src = std::move(m_pendingSrc);
            }
            // released meanwhile: the window goes
            if (m_released || !src)
                return;
            m_src = src;
            m_vi = src->info();
            m_gaps = src->gaps();
            double lost = 0, waited = 0, places = 0;
            for (const auto& g : m_gaps) {
                lost += static_cast<double>(g.lost);
                waited += static_cast<double>(g.wait);
                places += g.lost > 0 ? 1 : 0;
            }
            QVariantList inputs;
            for (int id : m_vi.ids)
                inputs.append(id);
            m_info = QVariantMap{{"name", QFileInfo(m_path).fileName()},
                                 {"kind", m_vi.kind},
                                 {"rate", m_vi.rate},
                                 {"decimation", m_vi.decimation},
                                 {"adcRate", m_vi.clock},
                                 {"inputs", inputs},
                                 {"bits", m_vi.bits},
                                 {"volts", m_vi.volts},
                                 {"total", static_cast<double>(m_vi.total)},
                                 {"seconds", m_vi.rate > 0 ? static_cast<double>(m_vi.total) / m_vi.rate : 0.0},
                                 {"startNs", m_vi.startNs},
                                 {"gapsFrom", m_vi.gapsFrom},
                                 {"lost", lost},
                                 {"waited", waited},
                                 {"places", places},
                                 {"ready", true}};
            m_ready = true;
            loadMarks();
            Q_EMIT changed();
            Q_EMIT busyChanged();
            Q_EMIT overviewChanged();
            Q_EMIT marksChanged();
        },
        Qt::QueuedConnection);
}

void RecordingView::stopBusy() {
    const QString what = busy().value("what").toString();
    if (what == "open")
        m_stop = true;
    else if (what == "find")
        m_findStop = true;
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
    QVariantList r;
    for (const auto& g : m_gaps)
        r.append(QVariantMap{{"at", static_cast<double>(g.at)}, {"lost", static_cast<double>(g.lost)}, {"wait", static_cast<double>(g.wait)}});
    return r;
}

QVariantMap RecordingView::window(double fromD, double toD, int width) {
    QVariantMap r;
    if (!m_ready || width <= 0)
        return r;
    const uint64_t total = m_vi.total;
    const uint64_t from = static_cast<uint64_t>(std::clamp(fromD, 0.0, static_cast<double>(total)));
    const uint64_t to = static_cast<uint64_t>(std::clamp(toD, 0.0, static_cast<double>(total)));
    if (to <= from)
        return r;
    const uint64_t n = to - from;
    QVariantList channels;
    if (n > m_src->directLimit()) {
        // far out: the buckets of the overview
        std::lock_guard lock(m_mtx);
        const double b0 = static_cast<double>(from) * BUCKETS / total;
        const double b1 = static_cast<double>(to) * BUCKETS / total;
        for (size_t k = 0; k < m_lo.size(); k++) {
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
            channels.append(QVariantMap{{"id", m_vi.ids[k]}, {"lo", QVariant::fromValue(lo)}, {"hi", QVariant::fromValue(hi)}});
        }
        r["mode"] = "band";
    } else {
        // near: the samples of the file (the gaps read as zeros, the plot leaves them out)
        std::vector<std::vector<float>> buf;
        const uint64_t got = m_src->readAll(from, n, buf);
        for (size_t k = 0; k < buf.size() && k < m_vi.ids.size(); k++) {
            const auto& v = buf[k];
            if (got <= static_cast<uint64_t>(width)) {
                QVector<qreal> vals(static_cast<int>(got));
                for (uint64_t i = 0; i < got; i++)
                    vals[static_cast<int>(i)] = v[i];
                channels.append(QVariantMap{{"id", m_vi.ids[k]}, {"values", QVariant::fromValue(vals)}});
                r["mode"] = "line";
            } else {
                QVector<qreal> lo(width), hi(width);
                for (int j = 0; j < width; j++) {
                    const uint64_t s = got * j / width, e = std::max(s + 1, got * (j + 1) / width);
                    float l = std::numeric_limits<float>::max(), h = std::numeric_limits<float>::lowest();
                    for (uint64_t i = s; i < e; i++) {
                        l = std::min(l, v[i]);
                        h = std::max(h, v[i]);
                    }
                    lo[j] = l;
                    hi[j] = h;
                }
                channels.append(QVariantMap{{"id", m_vi.ids[k]}, {"lo", QVariant::fromValue(lo)}, {"hi", QVariant::fromValue(hi)}});
                r["mode"] = "band";
            }
        }
    }
    r["from"] = static_cast<double>(from);
    r["to"] = static_cast<double>(to);
    r["channels"] = channels;
    return r;
}

// ---- marks

QString RecordingView::find(int id, const QString& edge, double level, double hysteresis, double scale, double minGap, double fromD, double toD) {
    if (!m_ready)
        return "The recording is not read yet";
    if (busy().value("what").toString() == "find")
        return "A find runs already";
    const auto it = std::find(m_vi.ids.begin(), m_vi.ids.end(), id);
    if (it == m_vi.ids.end())
        return "Not an input of this recording";
    if (!std::isfinite(level) || !std::isfinite(hysteresis) || hysteresis < 0)
        return "Give the level and the hysteresis in volts";
    const uint64_t from = static_cast<uint64_t>(std::clamp(fromD, 0.0, static_cast<double>(m_vi.total)));
    const uint64_t to = static_cast<uint64_t>(std::clamp(toD, 0.0, static_cast<double>(m_vi.total)));
    if (to <= from)
        return "The range is empty";
    const int edges = edge == "rising" ? 1 : edge == "falling" ? 2 : 3;
    if (m_findThread.joinable())
        m_findThread.join();
    m_findStop = false;
    setBusy("find", "Finding the level", 0);
    m_findThread = std::thread(&RecordingView::runFind, this, static_cast<int>(it - m_vi.ids.begin()), edges, level, hysteresis, scale,
                               static_cast<uint64_t>(std::max(0.0, minGap)), from, to);
    return "";
}

// The level as the trigger of the board takes it: armed below level - hysteresis, a mark where
// it reaches the level (falling: the other way round); a gap disarms
auto RecordingView::runFind(int k, int edges, double level, double hyst, double scale, uint64_t minGap, uint64_t from, uint64_t to) -> void {
    const auto t0 = std::chrono::steady_clock::now();
    std::vector<Mark> hits;
    std::vector<std::vector<float>> buf;
    const int id = m_vi.ids[k];
    bool armR = false, armF = false;
    bool any = false;
    uint64_t last = 0;
    size_t gi = 0;
    QString message;
    const double span = static_cast<double>(to - from);
    for (uint64_t pos = from; pos < to && message.isEmpty(); pos += kChunk) {
        const uint64_t got = m_src->readAll(pos, std::min(kChunk, to - pos), buf);
        if (got == 0)
            break;
        const float* v = buf[k].data();
        for (uint64_t i = 0; i < got; i++) {
            const uint64_t p = pos + i;
            while (gi < m_gaps.size() && m_gaps[gi].end() <= p)
                gi++;
            if (gi < m_gaps.size() && m_gaps[gi].at <= p) {
                armR = armF = false;
                i = std::min(got, m_gaps[gi].end() - pos) - 1;
                continue;
            }
            const double x = v[i] * scale;
            QString hit;
            if (edges & 1) {
                if (hyst > 0 ? x <= level - hyst : x < level)
                    armR = true;
                else if (armR && x >= level) {
                    hit = "rising";
                    armR = false;
                }
            }
            if (edges & 2) {
                if (hyst > 0 ? x >= level + hyst : x > level)
                    armF = true;
                else if (armF && x <= level) {
                    hit = "falling";
                    armF = false;
                }
            }
            if (!hit.isEmpty() && (!any || p - last >= minGap)) {
                hits.push_back(Mark{p, id, hit, "find", ""});
                any = true;
                last = p;
                if (hits.size() >= kMaxMarks) {
                    message = "More than 100 000 marks: the first are kept; narrow the find (a higher hysteresis, not closer than)";
                    break;
                }
            }
        }
        const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
        const double done = static_cast<double>(pos + got - from) / span;
        QString text = "Finding the level · " + QString::number(hits.size()) + " found";
        if (done > 0.02 && s > 1)
            text += " · " + QString::number(std::max(1.0, std::round(s / done * (1 - done)))) + " s left";
        setBusy("find", text, done);
        if (m_findStop) {
            message = "Stopped: the marks found until then are kept";
            break;
        }
    }
    const double secs = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    setBusy("", "", 0);
    QMetaObject::invokeMethod(
        this,
        [this, hits, message, secs]() {
            // released meanwhile (the recording maybe deleted): no marks file again
            if (m_released)
                return;
            std::vector<Mark> keep;
            for (const auto& m : m_marks)
                if (m.from != "find")
                    keep.push_back(m);
            keep.insert(keep.end(), hits.begin(), hits.end());
            std::stable_sort(keep.begin(), keep.end(), [](const Mark& a, const Mark& b) { return a.pos < b.pos; });
            m_marks = std::move(keep);
            saveMarks();
            Q_EMIT busyChanged();
            Q_EMIT marksChanged();
            Q_EMIT found(static_cast<int>(hits.size()), message.isEmpty() ? QString::number(hits.size()) + " found in " + QString::number(secs, 'f', 1) + " s" : message);
        },
        Qt::QueuedConnection);
}

auto RecordingView::marks() -> QVariantList {
    QVariantList r;
    for (const auto& m : m_marks)
        r.append(QVariantMap{{"pos", static_cast<double>(m.pos)}, {"input", m.id}, {"edge", m.edge}, {"from", m.from}, {"name", m.name}});
    return r;
}

void RecordingView::addMark(double pos, int id) {
    if (!m_ready || pos < 0 || pos >= static_cast<double>(m_vi.total))
        return;
    const Mark m{static_cast<uint64_t>(pos), id, "", "hand", ""};
    const auto it = std::upper_bound(m_marks.begin(), m_marks.end(), m.pos, [](uint64_t p, const Mark& x) { return p < x.pos; });
    m_marks.insert(it, m);
    saveMarks();
    Q_EMIT marksChanged();
}

void RecordingView::removeMark(int i) {
    if (!m_ready || i < 0 || i >= static_cast<int>(m_marks.size()))
        return;
    m_marks.erase(m_marks.begin() + i);
    saveMarks();
    Q_EMIT marksChanged();
}

void RecordingView::renameMark(int i, const QString& name) {
    if (!m_ready || i < 0 || i >= static_cast<int>(m_marks.size()))
        return;
    m_marks[i].name = QString(name).replace(',', ';').replace('\n', ' ').trimmed();
    saveMarks();
    Q_EMIT marksChanged();
}

void RecordingView::clearMarks() {
    if (!m_ready)
        return;
    m_marks.clear();
    saveMarks();
    Q_EMIT marksChanged();
}

// <file>.marks.csv: name, pos (the sample on the time line), time (s), input, edge, from
auto RecordingView::loadMarks() -> void {
    m_marks.clear();
    QFile f(m_path + ".marks.csv");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&f);
    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.startsWith('#') || line.startsWith("name,") || line.trimmed().isEmpty())
            continue;
        const QStringList c = line.split(',');
        if (c.size() < 6)
            continue;
        bool ok = false;
        const uint64_t pos = c[1].toULongLong(&ok);
        if (!ok || pos >= m_vi.total)
            continue;
        const QString in1 = c[3].trimmed();
        m_marks.push_back(Mark{pos, in1.startsWith("IN") ? in1.mid(2).toInt() : 0, c[4].trimmed(), c[5].trimmed() == "find" ? "find" : "hand", c[0].trimmed()});
    }
    std::stable_sort(m_marks.begin(), m_marks.end(), [](const Mark& a, const Mark& b) { return a.pos < b.pos; });
}

auto RecordingView::saveMarks() -> void {
    if (m_released)
        return;
    const QString path = m_path + ".marks.csv";
    if (m_marks.empty()) {
        QFile::remove(path);
        return;
    }
    QFile f(path + ".tmp");
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return;
    QTextStream out(&f);
    out << "# The marks of " << QFileInfo(m_path).fileName() << " (the viewer of the streaming client): pos - the sample on the time line, time - seconds from its start\n";
    out << "name,pos,time,input,edge,from\n";
    for (const auto& m : m_marks)
        out << m.name << "," << m.pos << "," << (m_vi.rate > 0 ? QString::number(static_cast<double>(m.pos) / m_vi.rate, 'f', 9) : QString()) << ","
            << (m.id > 0 ? "IN" + QString::number(m.id) : QString()) << "," << m.edge << "," << m.from << "\n";
    out.flush();
    f.close();
    QFile::remove(path);
    QFile::rename(path + ".tmp", path);
}

QString RecordingView::exportTable(const QString& url, bool losses) {
    if (!m_ready)
        return "The recording is not read yet";
    QFile f(localPath(url));
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return "Not saved: " + f.errorString();
    QTextStream out(&f);
    auto secs = [this](uint64_t p) { return m_vi.rate > 0 ? QString::number(static_cast<double>(p) / m_vi.rate, 'f', 9) : QString(); };
    if (losses) {
        out << "number,time,pos,lost,wait\n";
        int n = 0;
        for (const auto& g : m_gaps)
            out << ++n << "," << secs(g.at) << "," << g.at << "," << g.lost << "," << g.wait << "\n";
    } else {
        out << "number,name,time,pos,input,edge,from,after_previous\n";
        for (size_t i = 0; i < m_marks.size(); i++) {
            const auto& m = m_marks[i];
            out << i + 1 << "," << m.name << "," << secs(m.pos) << "," << m.pos << "," << (m.id > 0 ? "IN" + QString::number(m.id) : QString()) << "," << m.edge << ","
                << m.from << "," << (i > 0 ? secs(m.pos - m_marks[i - 1].pos) : QString()) << "\n";
        }
    }
    out.flush();
    return f.error() == QFile::NoError ? QString() : "Not saved: " + f.errorString();
}

// ---- export

QString RecordingView::exportName(double from, double to, const QString& format) {
    const QFileInfo fi(m_path);
    const double rate = m_vi.rate;
    // as many decimals as the two ends differ in (3 at least)
    const double span = rate > 0 ? std::max(1e-9, (to - from) / rate) : 1;
    const int dec = std::clamp(static_cast<int>(std::ceil(-std::log10(span))) + 2, 3, 9);
    auto secs = [rate, dec](double p) { return rate > 0 ? QString::number(p / rate, 'f', dec) : QString::number(static_cast<qint64>(p)); };
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
    if (!m_ready)
        return "The recording is not read yet";
    if (!QStringList{"CSV", "WAV", "TDMS", "BIN"}.contains(f))
        return "Not a format: " + format;
    const uint64_t from = static_cast<uint64_t>(std::clamp(fromD, 0.0, static_cast<double>(m_vi.total)));
    const uint64_t to = static_cast<uint64_t>(std::clamp(toD, 0.0, static_cast<double>(m_vi.total)));
    if (to <= from)
        return "The range is empty";
    if (output.trimmed().isEmpty())
        return "Give the name of the file";
    if (QFileInfo(output).absoluteFilePath() == QFileInfo(m_path).absoluteFilePath())
        return "That is the recording itself: give another name";
    if (QFileInfo::exists(output))
        return QDir::toNativeSeparators(output) + " is there: give another name";
    std::vector<int> ids;
    for (int id : m_vi.ids)
        for (const auto& i : inputs)
            if (i.toInt() == id)
                ids.push_back(id);
    if (ids.empty())
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
    m_exportThread = std::thread(&RecordingView::runExport, this, from, to, f, ids, index, time, output);
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

// The range of a BIN as a BIN of its own: the blocks cut to it, the losses and the waits inside
// it kept, the time line from its start
auto RecordingView::writeBinFromBin(uint64_t from, uint64_t to, const std::vector<int>& inputs, const QString& binPath, const std::function<void(double)>& progress) -> QString {
    bin_lib::FileReader r;
    if (!r.open(m_path.toStdString()))
        return QString::fromStdString(r.error());
    const auto& info = r.info();
    bin_lib::FileInfo out = info;
    out.channels.clear();
    std::vector<int> remap(info.channels.size(), -1);
    for (int id : inputs)
        for (size_t i = 0; i < info.channels.size(); i++)
            if (info.channels[i].id == id && bin_lib::sampleBytes(info.channels[i].type) > 0 && info.channels[i].type != bin_lib::SampleType::GPIO_IN_WORD &&
                info.channels[i].type != bin_lib::SampleType::GPIO_OUT_WORD && remap[i] < 0) {
                remap[i] = static_cast<int>(out.channels.size());
                out.channels.push_back(info.channels[i]);
            }
    if (info.startTime && m_vi.rate > 0)
        out.startTime = info.startTime + static_cast<int64_t>(static_cast<double>(from) / m_vi.rate * 1e9);
    std::ofstream file(binPath.toStdString(), std::ios::binary | std::ios::trunc);
    if (!file)
        return "Can't write " + QDir::toNativeSeparators(binPath);
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
            progress(std::min(1.0, done / size));
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
        // no more samples than the data has (a damaged header)
        if (static_cast<uint64_t>(s1) * bytes > data.size())
            continue;
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
    return file ? QString() : "Can't write " + QDir::toNativeSeparators(binPath);
}

// The range of a WAV, a TDMS or a CSV as a BIN: blocks of the samples between the gaps, a gap on
// the block after it; codes as 16 bit, volts as volts
auto RecordingView::writeBinFromSource(uint64_t from, uint64_t to, const std::vector<int>& inputs, const QString& binPath, const std::function<void(double)>& progress) -> QString {
    std::vector<int> ks;
    bin_lib::FileInfo out;
    out.clock = static_cast<uint64_t>(m_vi.clock > 0 ? m_vi.clock : m_vi.rate);
    out.decimation = static_cast<uint32_t>(m_vi.clock > 0 && m_vi.decimation > 0 ? m_vi.decimation : 1);
    if (m_vi.startNs > 0 && m_vi.rate > 0)
        out.startTime = static_cast<int64_t>(m_vi.startNs + static_cast<double>(from) / m_vi.rate * 1e9);
    const auto type = m_vi.volts ? bin_lib::SampleType::FLOAT32 : bin_lib::SampleType::INT16;
    for (int id : inputs)
        for (size_t k = 0; k < m_vi.ids.size(); k++)
            if (m_vi.ids[k] == id) {
                ks.push_back(static_cast<int>(k));
                bin_lib::ChannelInfo c;
                c.id = static_cast<uint8_t>(id);
                c.type = type;
                out.channels.push_back(c);
            }
    std::ofstream file(binPath.toStdString(), std::ios::binary | std::ios::trunc);
    if (!file)
        return "Can't write " + QDir::toNativeSeparators(binPath);
    const auto head = bin_lib::fileHeader(out);
    file.write(reinterpret_cast<const char*>(head.data()), static_cast<std::streamsize>(head.size()));
    bin_lib::Timeline line;
    std::vector<std::vector<float>> buf;
    std::vector<uint8_t> raw, block;
    uint64_t pendLost = 0, pendWait = 0;
    size_t gi = 0;
    while (gi < m_gaps.size() && m_gaps[gi].end() <= from)
        gi++;
    uint64_t p = from;
    const double span = static_cast<double>(to - from);
    while (p < to && !m_exportStop) {
        if (gi < m_gaps.size() && m_gaps[gi].at <= p) {
            const auto& g = m_gaps[gi];
            const uint64_t e = std::min(to, g.end());
            const uint64_t lostEnd = g.at + g.lost;
            const uint64_t lostPart = p < lostEnd ? std::min(e, lostEnd) - p : 0;
            pendLost += lostPart;
            pendWait += (e - p) - lostPart;
            p = e;
            if (e == g.end())
                gi++;
            continue;
        }
        const uint64_t e = gi < m_gaps.size() ? std::min(to, m_gaps[gi].at) : to;
        while (p < e && !m_exportStop) {
            const uint64_t n = std::min<uint64_t>(65536, e - p);
            const uint64_t got = m_src->readAll(p, n, buf);
            if (got == 0)
                return "Not read at " + QString::number(p);
            for (size_t j = 0; j < ks.size(); j++) {
                const auto& v = buf[ks[j]];
                bin_lib::BlockInfo b;
                b.channel = static_cast<uint8_t>(j);
                b.samples = got;
                b.lost = pendLost;
                b.wait = pendWait;
                b.position = line.next(b.channel, pendLost, pendWait, got);
                if (m_vi.volts) {
                    raw.resize(got * 4);
                    std::memcpy(raw.data(), v.data(), got * 4);
                } else {
                    raw.resize(got * 2);
                    auto* d = reinterpret_cast<int16_t*>(raw.data());
                    for (uint64_t i = 0; i < got; i++)
                        d[i] = static_cast<int16_t>(std::clamp(std::lround(v[i] * 32768.0f), -32768L, 32767L));
                }
                b.dataSize = static_cast<uint32_t>(raw.size());
                block.clear();
                bin_lib::appendBlock(block, b, raw.data(), b.dataSize);
                file.write(reinterpret_cast<const char*>(block.data()), static_cast<std::streamsize>(block.size()));
            }
            pendLost = pendWait = 0;
            p += got;
            progress(static_cast<double>(p - from) / span);
        }
    }
    file.close();
    return file ? QString() : "Can't write " + QDir::toNativeSeparators(binPath);
}

// The range as a BIN of its own; CSV, WAV and TDMS are converted from that one, which then goes
auto RecordingView::runExport(uint64_t from, uint64_t to, QString format, std::vector<int> inputs, bool index, bool time, QString output) -> void {
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
    const double share = bin ? 1.0 : 0.5;
    const QString err = m_vi.kind == "BIN" ? writeBinFromBin(from, to, inputs, binPath, [&](double p) { progress(p * share); })
                                           : writeBinFromSource(from, to, inputs, binPath, [&](double p) { progress(p * share); });
    if (m_exportStop) {
        QFile::remove(binPath);
        return finish(false, output, "Stopped");
    }
    if (!err.isEmpty()) {
        QFile::remove(binPath);
        return finish(false, output, err);
    }
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
