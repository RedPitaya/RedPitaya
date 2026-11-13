#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <QObject>

// What QML asks of the application that is not a board: files, folders, the version.
class ui_controller : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)

   public:
    static ui_controller* instance();

    ui_controller(QObject* parent = nullptr);
    ~ui_controller() {};

    auto version() const -> QString;

    Q_INVOKABLE QObject* getBoardsModel();
    Q_INVOKABLE void runAll();
    Q_INVOKABLE void stopAll();
    Q_INVOKABLE void openFolder(const QString& path);
    // a text file chosen in a dialog (a file URL); empty with an error in lastError
    Q_INVOKABLE QString readTextFile(const QString& url);
    Q_INVOKABLE bool writeTextFile(const QString& url, const QString& text);
    Q_INVOKABLE QString lastError() const;

   private:
    QString m_lastError;
};

#endif  // UI_CONTROLLER_H
