#ifndef DEVICE_LOGIC_H
#define DEVICE_LOGIC_H

#include <QHostInfo>
#include <QObject>
#include <QTimer>
#include "config_net_lib/client_net_config_manager.h"

// Finds the boards: those that answer the broadcast search and those the user added by their
// address (a name is looked up first: the connection takes an IP address).
class DeviceLogic : public QObject {
    Q_OBJECT
   public:
    static DeviceLogic* instance();
    auto getNewBoards() -> void;

   private:
    DeviceLogic();
    ~DeviceLogic();
    Q_DISABLE_COPY_MOVE(DeviceLogic)

    auto addManual(const QString& host) -> void;
    auto removeManual(const QString& host) -> void;
    auto addBoard(const QString& ip, bool manual) -> void;

    QTimer* m_timer;
    ClientNetConfigManager* m_client;
    // the address each manual host was found at
    QMap<QString, QString> m_manualIp;
};

#endif  // DEVICE_LOGIC_H
