#include "device_logic.h"
#include <QHostAddress>
#include "src/app_settings.h"
#include "src/models/boardsmodel.h"

DeviceLogic* DeviceLogic::instance() {
    static DeviceLogic* s_instance = new DeviceLogic();
    return s_instance;
}

DeviceLogic::DeviceLogic() : QObject(nullptr) {
    m_client = new ClientNetConfigManager("", false);
    m_client->startBroadcast("127.0.0.1", NET_BROADCAST_PORT);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DeviceLogic::getNewBoards);
    m_timer->start(1000);

    auto* app = AppSettings::instance();
    connect(app, &AppSettings::manualHostAdded, this, &DeviceLogic::addManual);
    connect(app, &AppSettings::manualHostRemoved, this, &DeviceLogic::removeManual);
    for (const auto& host : app->manualHosts())
        addManual(host);
}

DeviceLogic::~DeviceLogic() {
    delete m_client;
}

auto DeviceLogic::addBoard(const QString& ip, bool manual) -> void {
    auto* model = BoardsModel::instance();
    if (auto b = model->find(ip)) {
        if (manual)
            b->setManual(true);
        return;
    }
    model->addNewBoard(std::make_shared<CBoard>(ip, manual));
}

auto DeviceLogic::addManual(const QString& host) -> void {
    QHostAddress address;
    if (address.setAddress(host)) {
        m_manualIp[host] = address.toString();
        addBoard(address.toString(), true);
        return;
    }
    QHostInfo::lookupHost(host, this, [this, host](const QHostInfo& info) {
        if (!AppSettings::instance()->isManualHost(host))
            return;
        for (const auto& a : info.addresses()) {
            if (a.protocol() == QAbstractSocket::IPv4Protocol) {
                m_manualIp[host] = a.toString();
                addBoard(a.toString(), true);
                return;
            }
        }
        // tried again with the next search
        QTimer::singleShot(10000, this, [this, host]() {
            if (AppSettings::instance()->isManualHost(host) && !m_manualIp.contains(host))
                addManual(host);
        });
    });
}

auto DeviceLogic::removeManual(const QString& host) -> void {
    const QString ip = m_manualIp.take(host);
    if (ip.isEmpty())
        return;
    BoardsModel::instance()->removeBoard(ip);
}

auto DeviceLogic::getNewBoards() -> void {
    if (!AppSettings::instance()->discovery())
        return;
    for (auto& client : m_client->getBroadcastClients()) {
        const QString ip = QString::fromStdString(client.host);
        auto* model = BoardsModel::instance();
        if (!model->find(ip))
            addBoard(ip, false);
        auto b = model->find(ip);
        b->setMode(client.mode);
        b->setModel(client.model);
        b->setMac(QString::fromStdString(client.mac));
        b->setGpio(client.gpio);
        // only a fresh answer marks it online
        if (std::time(nullptr) - client.ts <= 5)
            b->setOnline();
    }
}
