#ifndef BOARDSMODEL_H
#define BOARDSMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QTimer>
#include <QVariantMap>
#include "board.h"

// The boards on the dashboard, in the order they were found.
class BoardsModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    // boards, online, running, manual, broadcast, speed, received, lost, master (its name)
    Q_PROPERTY(QVariantMap summary READ summary NOTIFY summaryChanged)
    // Start all is still starting boards one by one
    Q_PROPERTY(bool starting READ starting NOTIFY startingChanged)

   public:
    static BoardsModel* instance();

    BoardsModel();

    auto addNewBoard(CBoard::Ptr item) -> void;
    auto find(const QString& ip) -> CBoard::Ptr;
    // true when the board is in the list (and is marked online)
    auto setIsOnline(const QString& ip) -> bool;

    auto roleNames() const -> QHash<int, QByteArray> override;
    auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
    auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;

    auto summary() -> QVariantMap;
    auto starting() -> bool;

    Q_INVOKABLE QObject* board(int index);
    Q_INVOKABLE QObject* boardByIp(const QString& ip);
    Q_INVOKABLE void removeBoard(const QString& ip);
    // the slaves first, each started before the next, then the masters: the clock and the
    // trigger of a master drive its slaves
    // The recordings to this computer that do not fit their disk (streaming_improvements 2.4):
    // the boards a Start of that board ("" - the ones Start all starts) records with a limit,
    // grouped by the disk of their folders. The first disk short of space: need, free, disk,
    // boards [{name, samples, inputs, need, running}] (running: already recording there, the rest
    // of its limit); an empty map when they fit or have no limit.
    Q_INVOKABLE QVariantMap diskShortage(const QString& ip);
    Q_INVOKABLE void runAll();
    Q_INVOKABLE void stopAll();
    Q_INVOKABLE void triggerAll();

   signals:
    void countChanged();
    void summaryChanged();
    void startingChanged();

   private:
    auto startNext() -> void;
    auto abortStart(const QString& why) -> void;
    auto onDataLost() -> void;

    QList<CBoard::Ptr> m_boards;
    QTimer m_summaryTimer;
    QTimer m_startTimer;
    QList<QString> m_startQueue;
    // the boards Start all started in this run
    QList<QString> m_startedRun;
    QString m_startCurrent;
    qint64 m_startSince = 0;
    QDateTime m_startTime;
};

#endif  // BOARDSMODEL_H
