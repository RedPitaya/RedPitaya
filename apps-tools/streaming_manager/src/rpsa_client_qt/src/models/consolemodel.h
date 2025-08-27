#ifndef CONSOLEMODEL_H
#define CONSOLEMODEL_H

#include <QAbstractListModel>
#include <QObject>

class ConsoleModel : public QAbstractListModel {
    Q_OBJECT
   public:
    static ConsoleModel* instance();

    ConsoleModel();

    auto roleNames() const -> QHash<int, QByteArray> override;
    auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
    auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
    auto removeRows(int position, int rows, const QModelIndex& parent) -> bool override;
    auto deleteByIndex(int index) -> bool;

   public slots:
    void addNewLine(QString str);

   private:
    QList<QString> m_lines;
};

#endif  // CONSOLEMODEL_H
