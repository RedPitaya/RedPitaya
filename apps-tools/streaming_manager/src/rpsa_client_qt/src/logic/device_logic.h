#ifndef DEVICE_LOGIC_H
#define DEVICE_LOGIC_H
#include <QObject>
#include <QTimer>
#include "config_net_lib/client_net_config_manager.h"

class DeviceLogic : public QObject
{
	Q_OBJECT
public:
	static DeviceLogic *instance();

	auto getNewBoards() -> void;

private:
	DeviceLogic();
	~DeviceLogic();
	Q_DISABLE_COPY_MOVE(DeviceLogic);

	QTimer *m_timer;
	ClientNetConfigManager *m_client;
};

#endif // DEVICE_LOGIC_H
