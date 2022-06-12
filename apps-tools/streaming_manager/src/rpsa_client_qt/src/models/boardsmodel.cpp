#include <QDateTime>
#include "boardsmodel.h"

BoardsModel * BoardsModel::instance()
{
    static BoardsModel * _instance = 0;
    if ( _instance == 0 ) {
        _instance = new BoardsModel();
    }
    return _instance;
}

BoardsModel::BoardsModel()
    : QAbstractListModel(nullptr)
{

}


auto BoardsModel::addNewBoard(CBoard::Ptr item) -> void{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_boards.append(item);
    endInsertRows();
}

auto BoardsModel::setIsOnline(QString &ip) -> bool{
    Q_FOREACH(auto item,m_boards){
        if (item->getIP() == ip){
            item->setOnline();
            return true;
        }
    }
    return false;
}

auto BoardsModel::data(const QModelIndex &index, int role) const -> QVariant{
    Q_UNUSED(role)
    if (!index.isValid())
        return QVariant();
    int idx = index.row();
    QObject* item = m_boards.at(idx).get();
    return QVariant::fromValue(item);
}

//auto BoardsModel::removeRows(int position, int rows, const QModelIndex &parent)  -> bool{
//}

auto BoardsModel::rowCount(const QModelIndex &parent) const -> int{
    return m_boards.size();
}

QHash<int, QByteArray> BoardsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole+1] = "board";
    return roles;
}

void BoardsModel::runAll(QDateTime time){
    QVector<CBoard::Ptr> m_sboards;
    QVector<CBoard::Ptr> m_mboards;
    for(auto &b:m_boards){
        if (b->getIsMaster()){
            m_mboards.append(b);
        }else{
            m_sboards.append(b);
        }
    }

    for(auto &b:m_sboards){
        b->startStreaming(time);
        auto startT = QDateTime::currentMSecsSinceEpoch();
        while(!b->getIsADCStarted()){
            if (QDateTime::currentMSecsSinceEpoch() - startT > 3000) break;
        }
    }

    for(auto &b:m_mboards){
        b->startStreaming(time);
        auto startT = QDateTime::currentMSecsSinceEpoch();
        while(!b->getIsADCStarted()){
            if (QDateTime::currentMSecsSinceEpoch() - startT > 3000) break;
        }
    }
}

void BoardsModel::stopAll(){
    for(auto &b:m_boards){
        b->stopStreaming();
    }
}
