#include "boardsmodel.h"
#include <algorithm>
#include <QDir>
#include <QStorageInfo>
#include <QQmlEngine>
#include "src/app_settings.h"

namespace {

// how long Start all waits for a board to confirm its ADC runs (the server start, the memory,
// the data connection and the FPGA start) before it gives up
constexpr qint64 START_TIMEOUT_MS = 10000;

}  // namespace

BoardsModel* BoardsModel::instance() {
    static BoardsModel* s_instance = new BoardsModel();
    return s_instance;
}

BoardsModel::BoardsModel() : QAbstractListModel(nullptr) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    connect(&m_summaryTimer, &QTimer::timeout, this, &BoardsModel::summaryChanged);
    m_summaryTimer.start(500);
    connect(&m_startTimer, &QTimer::timeout, this, &BoardsModel::startNext);
}

auto BoardsModel::addNewBoard(CBoard::Ptr item) -> void {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_boards.append(item);
    endInsertRows();
    connect(item.get(), &CBoard::dataLost, this, &BoardsModel::onDataLost);
    Q_EMIT countChanged();
    Q_EMIT summaryChanged();
}

auto BoardsModel::find(const QString& ip) -> CBoard::Ptr {
    for (auto& b : m_boards)
        if (b->getIP() == ip)
            return b;
    return nullptr;
}

auto BoardsModel::setIsOnline(const QString& ip) -> bool {
    auto b = find(ip);
    if (!b)
        return false;
    b->setOnline();
    return true;
}

void BoardsModel::removeBoard(const QString& ip) {
    for (int i = 0; i < m_boards.size(); i++) {
        if (m_boards[i]->getIP() != ip)
            continue;
        auto b = m_boards[i];
        b->stopStreaming();
        b->stopDac();
        b->stopGpioIn();
        b->stopGpioOut();
        beginRemoveRows(QModelIndex(), i, i);
        m_boards.removeAt(i);
        endRemoveRows();
        m_startQueue.removeAll(ip);
        Q_EMIT countChanged();
        Q_EMIT summaryChanged();
        // deleted once QML has let go of it
        QTimer::singleShot(0, this, [b]() {});
        return;
    }
}

auto BoardsModel::data(const QModelIndex& index, int role) const -> QVariant {
    Q_UNUSED(role)
    if (!index.isValid() || index.row() >= m_boards.size())
        return QVariant();
    QObject* item = m_boards.at(index.row()).get();
    return QVariant::fromValue(item);
}

auto BoardsModel::rowCount(const QModelIndex&) const -> int {
    return m_boards.size();
}

QHash<int, QByteArray> BoardsModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole + 1] = "board";
    return roles;
}

QObject* BoardsModel::board(int index) {
    return index >= 0 && index < m_boards.size() ? m_boards[index].get() : nullptr;
}

QObject* BoardsModel::boardByIp(const QString& ip) {
    return find(ip).get();
}

QObject* BoardsModel::boardByName(const QString& name) {
    for (auto& b : m_boards)
        if (b->getName() == name || b->getIP() == name)
            return b.get();
    return nullptr;
}

auto BoardsModel::summary() -> QVariantMap {
    // others: boards whose DAC or GPIO runs (Stop all stops them too)
    int online = 0, running = 0, manual = 0, errors = 0, others = 0;
    double speed = 0, received = 0, lost = 0;
    QString master;
    int masters = 0;
    for (auto& b : m_boards) {
        if (b->isOnline())
            online++;
        if (b->getIsADCStarted())
            running++;
        if (b->isDacRunning() || b->isGpioInRunning() || b->isGpioOutRunning())
            others++;
        if (b->isManual())
            manual++;
        if (!b->getError().isEmpty())
            errors++;
        if (b->isOnline() && b->getIsMaster()) {
            masters++;
            master = b->getName();
        }
        const auto s = b->getStats();
        speed += s["speed"].toDouble();
        received += s["received"].toDouble();
        lost += s["lostBoard"].toDouble() + s["lostFile"].toDouble();
    }
    QVariantMap m;
    m["boards"] = int(m_boards.size());
    m["online"] = online;
    m["running"] = running;
    m["others"] = others;
    m["manual"] = manual;
    m["errors"] = errors;
    m["broadcast"] = int(m_boards.size()) - manual;
    m["speed"] = speed;
    m["received"] = received;
    m["lost"] = lost;
    // a clock master only means something with slaves around
    m["master"] = masters == 1 && online > 1 ? master : QString();
    return m;
}

auto BoardsModel::starting() -> bool {
    return !m_startCurrent.isEmpty() || !m_startQueue.isEmpty();
}

void BoardsModel::runAll() {
    if (starting())
        return;
    QList<QString> slaves, masters;
    for (auto& b : m_boards) {
        if (!b->getConfigManagerConnected() || b->getIsADCStarted())
            continue;
        (b->getIsMaster() ? masters : slaves).append(b->getIP());
    }
    m_startQueue = slaves + masters;
    m_startedRun.clear();
    m_startTime = QDateTime::currentDateTime();
    if (!slaves.isEmpty())
        for (auto& ip : m_startQueue)
            if (auto b = find(ip))
                b->note("Start all: " + QString::number(slaves.size()) + " slave(s) first, then " + QString::number(masters.size()) +
                            " master(s); each board starts when the one before it confirms its ADC runs",
                        false);
    Q_EMIT startingChanged();
    startNext();
    if (starting())
        m_startTimer.start(50);
}

auto BoardsModel::startNext() -> void {
    if (!m_startCurrent.isEmpty()) {
        auto b = find(m_startCurrent);
        const QString st = b ? b->getState() : "stopped";
        const bool timedOut = QDateTime::currentMSecsSinceEpoch() - m_startSince >= START_TIMEOUT_MS;
        if (st == "starting" && !timedOut)
            return;
        const bool running = st == "streaming" || st == "sd" || st == "armed";
        const bool wasSlave = b && !b->getIsMaster();
        if (running) {
            m_startedRun.append(m_startCurrent);
        } else if (wasSlave) {
            // a master started without one of its slaves ready would take the slave's start away
            abortStart((b ? b->getName() : m_startCurrent) + " (slave) " + (timedOut ? "did not confirm its start in 10 s" : "did not start") +
                       ": the master is not started and the boards started by Start all are stopped");
            return;
        } else if (b) {
            b->note("Start all: this board did not start", true);
        }
        m_startCurrent.clear();
    }
    while (!m_startQueue.isEmpty()) {
        const QString ip = m_startQueue.takeFirst();
        auto b = find(ip);
        if (!b)
            continue;
        // one file name time for the whole run
        b->startStreaming(m_startTime);
        m_startCurrent = ip;
        m_startSince = QDateTime::currentMSecsSinceEpoch();
        return;
    }
    m_startTimer.stop();
    Q_EMIT startingChanged();
}

auto BoardsModel::abortStart(const QString& why) -> void {
    // the board that failed shows the error; the others only log why they were stopped
    const QString failed = m_startCurrent;
    m_startTimer.stop();
    const auto queued = m_startQueue;
    m_startQueue.clear();
    QList<QString> touched = m_startedRun;
    touched << m_startCurrent << queued;
    m_startCurrent.clear();
    for (auto& ip : touched) {
        auto b = find(ip);
        if (!b)
            continue;
        b->note("Start all stopped: " + why, ip == failed);
        if (m_startedRun.contains(ip) || b->getState() == "starting")
            b->stopStreaming();
    }
    m_startedRun.clear();
    Q_EMIT startingChanged();
}

void BoardsModel::stopAll() {
    m_startQueue.clear();
    m_startCurrent.clear();
    m_startTimer.stop();
    Q_EMIT startingChanged();
    // the masters first: the slaves stop getting the clock and the trigger
    for (auto& b : m_boards)
        if (b->getIsMaster()) {
            b->stopStreaming();
            b->stopDac();
            b->stopGpioIn();
            b->stopGpioOut();
        }
    for (auto& b : m_boards)
        if (!b->getIsMaster()) {
            b->stopStreaming();
            b->stopDac();
            b->stopGpioIn();
            b->stopGpioOut();
        }
}

void BoardsModel::triggerAll() {
    for (auto& b : m_boards)
        if (b->getIsADCStarted())
            b->softwareTrigger();
}

auto BoardsModel::onDataLost() -> void {
    if (!AppSettings::instance()->stopAllOnLoss())
        return;
    bool any = false;
    for (auto& b : m_boards)
        any |= b->getIsADCStarted();
    if (any)
        stopAll();
}

QVariantMap BoardsModel::diskShortage(const QString& ip) {
    struct Disk {
        double need = 0;
        double free = 0;
        QVariantList boards;
    };
    QMap<QString, Disk> disks;
    for (auto& b : m_boards) {
        // the boards of this start, and the ones already recording with a limit: the rest of
        // theirs goes to the same disks
        const bool starts = ip.isEmpty() ? b->getConfigManagerConnected() && !b->getIsADCStarted() : b->getIP() == ip;
        const bool records = b->getIsADCStarted() && !starts;
        if (!starts && !records)
            continue;
        double need = b->expectedBytes();
        if (need <= 0)
            continue;
        if (records) {
            const auto r = b->getRecord();
            if (r["dest"].toString() != "pc" || !r["running"].toBool() || r["limitKind"].toString() == "none")
                continue;
            need *= std::max(0.0, 1.0 - r["frac"].toDouble());
            if (need <= 0)
                continue;
        }
        const QStorageInfo info = CBoard::storageOf(b->recordDir());
        if (!info.isValid())
            continue;
        auto& d = disks[info.rootPath()];
        d.free = double(info.bytesAvailable());
        d.need += need;
        int on = 0;
        for (int i = 1; i <= 4; i++)
            if (b->value("channel_state_" + QString::number(i)) == "ON")
                on++;
        d.boards.append(QVariantMap{{"name", b->getName()}, {"samples", b->value("samples_limit_sd").toDouble()}, {"limitTime", b->value("limit_time_sd").toDouble()}, {"limitSize", b->value("limit_size_sd").toDouble()}, {"inputs", on}, {"need", need}, {"running", records}});
    }
    for (auto it = disks.begin(); it != disks.end(); ++it) {
        if (it->need > it->free)
            return QVariantMap{{"need", it->need}, {"free", it->free}, {"disk", it.key()}, {"boards", it->boards}};
    }
    return {};
}
