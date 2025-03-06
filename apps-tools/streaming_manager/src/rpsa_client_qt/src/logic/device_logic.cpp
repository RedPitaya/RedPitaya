#include "device_logic.h"
#include "QDebug"
#include "src/models/boardsmodel.h"

DeviceLogic* DeviceLogic::instance() {
    static DeviceLogic* _instance = 0;
    if (_instance == 0) {
        _instance = new DeviceLogic();
    }
    return _instance;
}

DeviceLogic::DeviceLogic() : QObject(nullptr) {
    m_client = new ClientNetConfigManager("", false);
    m_client->startBroadcast("127.0.0.1", NET_BROADCAST_PORT);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&DeviceLogic::getNewBoards));
    m_timer->start(1000);
}

DeviceLogic::~DeviceLogic() {
    delete m_client;
    delete m_timer;
}

auto DeviceLogic::getNewBoards() -> void {
    auto clients = m_client->getBroadcastClients();
    for (auto& client : clients) {
        QString ip = QString::fromStdString(client.host);
        if (!BoardsModel::instance()->setIsOnline(ip)) {
            CBoard::Ptr board = std::make_shared<CBoard>(ip);
            board->setMode(client.mode);
            board->setModel(client.model);
            BoardsModel::instance()->addNewBoard(board);
        }
    }
}
