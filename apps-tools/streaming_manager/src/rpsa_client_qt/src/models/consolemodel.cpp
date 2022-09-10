#include <QQmlEngine>
#include "consolemodel.h"

#define LINES_LIMIT 1000

ConsoleModel * ConsoleModel::instance()
{
    static ConsoleModel * _instance = 0;
    if ( _instance == 0 ) {
        _instance = new ConsoleModel();
    }
    return _instance;
}

ConsoleModel::ConsoleModel()
    : QAbstractListModel(nullptr)
{
    QQmlEngine::setObjectOwnership(this,QQmlEngine::CppOwnership);
}

auto getTS(std::string suffix) -> std::string{

    using namespace std;
    using namespace std::chrono;
    system_clock::time_point timeNow = system_clock::now();
    auto ttime_t = system_clock::to_time_t(timeNow);
    auto tp_sec = system_clock::from_time_t(ttime_t);
    milliseconds ms = duration_cast<milliseconds>(timeNow - tp_sec);

    std::tm * ttm = localtime(&ttime_t);

    char date_time_format[] = "%Y.%m.%d-%H.%M.%S";

    char time_str[] = "yyyy.mm.dd.HH-MM.SS.fff";

    strftime(time_str, strlen(time_str), date_time_format, ttm);

    string result(time_str);
    result.append(".");
    result.append(to_string(ms.count()));
    result.append(suffix);
    return result;
}

auto ConsoleModel::addNewLine(QString str) -> void{    
    beginInsertRows(QModelIndex(), 0, 0);
    auto ts = QString::fromStdString(getTS(":"));
    m_lines.insert(0,ts + str);
    endInsertRows();

    if (rowCount() > LINES_LIMIT){
        deleteByIndex(rowCount() - 1);
    }
}

bool ConsoleModel::removeRows(int position, int rows, const QModelIndex &parent){
    if (position < 0 || position >= m_lines.count())
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    m_lines.removeAt(position);
    endRemoveRows();
    return true;
}

auto ConsoleModel::data(const QModelIndex &index, int role) const -> QVariant{
    Q_UNUSED(role)
    if (!index.isValid())
        return QVariant();
    int idx = index.row();
    return QVariant::fromValue(m_lines.at(idx));
}

auto ConsoleModel::rowCount(const QModelIndex &parent) const -> int{
    return m_lines.size();
}

QHash<int, QByteArray> ConsoleModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole+1] = "con_text";
    return roles;
}

bool ConsoleModel::deleteByIndex(int  index){
    QModelIndex q_index = this->index(index);
    auto res = removeRows(index,1,q_index.parent());
    return res;
}

