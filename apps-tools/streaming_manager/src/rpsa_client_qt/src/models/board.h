#ifndef BOARD_H
#define BOARD_H

#include <QDateTime>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <memory>

#include "broadcast_lib/asio_broadcast_socket.h"
#include "config_net_lib/client_net_config_manager.h"
#include "net_lib/asio_net.h"
#include "src/models/consolemodel.h"
#include "streaming_lib/streaming_file.h"

using namespace streaming_lib;

class CBoard : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isOnline MEMBER m_IsOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(bool isMaster READ getIsMaster NOTIFY isMasterChanged)
    Q_PROPERTY(bool isADCStarted READ getIsADCStarted NOTIFY isADCStartedChanged)
    Q_PROPERTY(QString model READ getModel NOTIFY modelChanged)
    Q_PROPERTY(bool isConfigConnected READ getConfigManagerConnected NOTIFY configManagerConnectedChanged)
    Q_PROPERTY(QString ip READ getIP CONSTANT)
    Q_PROPERTY(int maxChannels MEMBER m_adcChannels NOTIFY modelChanged)
    Q_PROPERTY(bool isACDC MEMBER m_IsACDC NOTIFY modelChanged)
    Q_PROPERTY(bool isAttenuator MEMBER m_IsAttenuator NOTIFY modelChanged)

   public:
    using Ptr = std::shared_ptr<CBoard>;

    CBoard(QString ip);
    ~CBoard();

    auto setOnline() -> void;
    auto getIP() -> QString;
    auto setMode(broadcast_lib::EMode mode) -> void;
    auto setModel(broadcast_lib::EModel model) -> void;
    auto getModel() -> QString;
    auto getIsMaster() -> bool;
    auto getConfigManagerConnected() -> bool;
    auto getIsADCStarted() -> bool;

    Q_INVOKABLE void setChartEnable(bool enable);
    Q_INVOKABLE bool getChartEnable();
    Q_INVOKABLE QObject* getConsoleModel();
    Q_INVOKABLE void sendConfig();
    Q_INVOKABLE void getConfig();

    Q_INVOKABLE int getSaveType();
    Q_INVOKABLE void setSaveType(int);

    Q_INVOKABLE int getDecimation();
    Q_INVOKABLE void setDecimation(int);

    Q_INVOKABLE int getSampleLimit();
    Q_INVOKABLE void setSampleLimit(int);

    Q_INVOKABLE int getResolution();
    Q_INVOKABLE void setResolution(int);

    Q_INVOKABLE int getAttenuator(int channel);
    Q_INVOKABLE void setAttenuator(int channel, int);

    Q_INVOKABLE int getCalibration();
    Q_INVOKABLE void setCalibration(int);

    Q_INVOKABLE int getChannels(int channel);
    Q_INVOKABLE void setChannels(int channel, int);

    Q_INVOKABLE int getDataFormat();
    Q_INVOKABLE void setDataFormat(int);

    Q_INVOKABLE int getType();
    Q_INVOKABLE void setType(int);

    Q_INVOKABLE int getCoupling(int channel);
    Q_INVOKABLE void setCoupling(int channel, int);

    Q_INVOKABLE void startStreaming(QDateTime time = QDateTime::currentDateTime());
    Q_INVOKABLE void stopStreaming();

    Q_INVOKABLE QString getRecivedBytes();
    Q_INVOKABLE QString getBandwidth();
    Q_INVOKABLE QString getSamplesCH1();
    Q_INVOKABLE QString getSamplesCH2();
    Q_INVOKABLE QString getSamplesCH3();
    Q_INVOKABLE QString getSamplesCH4();
    Q_INVOKABLE QString getLostCount();
    Q_INVOKABLE QString getFLostCount();
    Q_INVOKABLE QString getSaveFileName();

    Q_INVOKABLE bool getCouplingVisible();

    Q_INVOKABLE bool getTestMode();
    Q_INVOKABLE void setTestMode(bool mode);

   signals:
    void isOnlineChanged();
    void isMasterChanged();
    void modelChanged();
    void configManagerConnectedChanged();
    void getNewSettingSignal();
    void isADCStartedChanged();
    void updateStatistic();
    void updateSaveFileName();

   private:
    struct SStat {
        uint64_t bytes = 0;
        std::string bw = "";
        uint64_t samples1 = 0;
        uint64_t samples2 = 0;
        uint64_t samples3 = 0;
        uint64_t samples4 = 0;
        uint64_t lost = 0;
        uint64_t flost = 0;
        uint64_t broken_b = 0;
    };

    auto updateOffline() -> void;
    auto configManagerConnected(std::string host) -> void;
    auto configMangerError(ClientNetConfigManager::Errors errors, std::string host, error_code err) -> void;
    auto getNewSettings(std::string host) -> void;
    auto sendSettings(std::string host) -> void;
    auto addLog(QString msg) -> void;
    auto createStreaming() -> void;
    auto startADCFPGAStreaming() -> void;
    auto chartPackThread() -> void;

    QTimer* m_offlineTimer;
    bool m_IsOnline;
    QString m_ip;
    qint64 m_lastOnline;
    broadcast_lib::EMode m_mode;
    broadcast_lib::EModel m_model;
    ClientNetConfigManager* m_configManager;
    bool m_chartEnable;
    ConsoleModel m_consoleModel;
    bool m_isADCStarted;
    QDateTime m_streamingDT;

    CStreamingFile::Ptr m_file_manager;
    net_lib::CAsioNet::Ptr m_asionet;

    SStat m_stat;

    bool m_testMode;
    uint8_t m_adcChannels;
    bool m_IsACDC;
    bool m_IsAttenuator;
    uint8_t m_activeChannels;
    uint32_t m_blockSize;
    DataLib::CBuffersCached::Ptr m_buffer;
};

#endif  // BOARD_H
