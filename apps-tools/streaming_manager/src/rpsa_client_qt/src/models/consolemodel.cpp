#include "consolemodel.h"
#include <QQmlEngine>

#define LINES_LIMIT 1000

ConsoleModel* ConsoleModel::instance() {
    static ConsoleModel* _instance = 0;
    if (_instance == 0) {
        _instance = new ConsoleModel();
    }
    return _instance;
}

ConsoleModel::ConsoleModel() : QAbstractListModel(nullptr) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}

// The oldest line goes when there are too many.
auto ConsoleModel::addNewLine(QString str, int level) -> void {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_lines.append(str);
    m_levels.append(level);
    endInsertRows();
    if (rowCount() > LINES_LIMIT) {
        deleteByIndex(0);
    }
}

bool ConsoleModel::removeRows(int position, int rows, const QModelIndex& parent) {
    if (position < 0 || position >= m_lines.count())
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    m_lines.removeAt(position);
    m_levels.removeAt(position);
    endRemoveRows();
    return true;
}

auto ConsoleModel::data(const QModelIndex& index, int role) const -> QVariant {
    if (!index.isValid())
        return QVariant();
    int idx = index.row();
    if (role == Qt::UserRole + 2)
        return m_levels.at(idx);
    return QVariant::fromValue(m_lines.at(idx));
}

auto ConsoleModel::rowCount(const QModelIndex&) const -> int {
    return m_lines.size();
}

QHash<int, QByteArray> ConsoleModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole + 1] = "con_text";
    roles[Qt::UserRole + 2] = "con_level";
    return roles;
}

bool ConsoleModel::deleteByIndex(int index) {
    QModelIndex q_index = this->index(index);
    auto res = removeRows(index, 1, q_index.parent());
    return res;
}
