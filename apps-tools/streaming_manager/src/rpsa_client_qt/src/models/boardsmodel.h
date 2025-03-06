#ifndef BOARDSMODEL_H
#define BOARDSMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include "board.h"

class BoardsModel : public QAbstractListModel {
   public:
    static BoardsModel* instance();

    BoardsModel();

    auto addNewBoard(CBoard::Ptr item) -> void;
    auto setIsOnline(QString& ip) -> bool;

    auto roleNames() const -> QHash<int, QByteArray> override;
    auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
    //       auto removeRows(int position, int rows, const QModelIndex &parent)  -> bool override;
    auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;

    void runAll(QDateTime time = QDateTime::currentDateTime());
    void stopAll();

   private:
    QList<CBoard::Ptr> m_boards;
};

#endif  // BOARDSMODEL_H
