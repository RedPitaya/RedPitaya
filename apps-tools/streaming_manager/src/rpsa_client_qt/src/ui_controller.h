#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <QObject>

class ui_controller : public QObject {
    Q_OBJECT
   public:
    static ui_controller* instance();

    ui_controller(QObject* parent = nullptr);
    ~ui_controller(){};

    Q_INVOKABLE QObject* getBoardsModel();
    Q_INVOKABLE QObject* getConsoleModel();
    Q_INVOKABLE void runAll();
    Q_INVOKABLE void stopAll();
    Q_INVOKABLE void openFolder();
};

#endif  // UI_CONTROLLER_H
