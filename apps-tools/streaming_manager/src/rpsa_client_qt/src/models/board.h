#ifndef BOARD_H
#define BOARD_H

#include <QDateTime>
#include <QFile>
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include <atomic>
#include <memory>
#include <mutex>

#include "broadcast_lib/asio_broadcast_socket.h"
#include "config_net_lib/client_net_config_manager.h"
#include "net_lib/asio_net.h"
#include "src/models/board_models.h"
#include "src/models/consolemodel.h"
#include "streaming_lib/streaming_file.h"
#include "uio_lib/memory_manager.h"

// One board: its settings connection, its ADC stream to a file on this computer and what the
// dashboard shows of it. Lives on the GUI thread; the network handlers post their work to it.
class CBoard : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString ip READ getIP CONSTANT)
    // the board as the user knows it: rp-xxxxxx from its MAC, else the address it was added by
    Q_PROPERTY(QString name READ getName NOTIFY modelChanged)
    // added by its address, not found by the broadcast
    Q_PROPERTY(bool manual READ isManual NOTIFY sourceChanged)
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY isOnlineChanged)
    Q_PROPERTY(bool isConfigConnected READ getConfigManagerConnected NOTIFY configManagerConnectedChanged)
    // the settings connection: connecting, connected, taken (another client took the board),
    // refused (no streaming application on the board), unreachable (no answer)
    Q_PROPERTY(QString link READ getLink NOTIFY configManagerConnectedChanged)
    Q_PROPERTY(bool isMaster READ getIsMaster NOTIFY isMasterChanged)
    Q_PROPERTY(QString model READ getModel NOTIFY modelChanged)
    // what the board has: adcChannels, adcRate, adcBits, dacChannels, dacRate, adcRanges, adcACDC,
    // dacGain, adc, dac, gpio, adcBlockMode, dacTriggerOutputs, extTrigger, extTriggerLevel,
    // fixedRange, memorySize, memoryMaxBlock, headerSize, fromBoard (told by the board itself)
    Q_PROPERTY(QVariantMap caps READ getCaps NOTIFY modelChanged)
    // stopped, starting, streaming, sd (the board writes its SD card), armed, stopping
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged)
    Q_PROPERTY(bool isADCStarted READ getIsADCStarted NOTIFY stateChanged)
    // received, speed (bytes a second), samples (per channel), lostBoard, lostFile, file
    Q_PROPERTY(QVariantMap stats READ getStats NOTIFY statsChanged)
    // the ADC trigger while streaming in the trigger mode: known (the board answered), armed,
    // count (triggers the FPGA has seen)
    Q_PROPERTY(QVariantMap trigger READ getTrigger NOTIFY triggerChanged)
    // bumped when the settings of the board change: bindings that read value() depend on it
    Q_PROPERTY(int settingsRev READ settingsRev NOTIFY settingsChanged)
    Q_PROPERTY(bool settingsLoaded READ settingsLoaded NOTIFY settingsChanged)
    // the same for the settings of this board kept on this computer (pcValue)
    Q_PROPERTY(int pcRev READ pcRev NOTIFY pcSettingsChanged)
    // the preview shows the signal (else the console)
    Q_PROPERTY(bool chartEnable READ getChartEnable WRITE setChartEnable NOTIFY pcSettingsChanged)

   public:
    using Ptr = std::shared_ptr<CBoard>;

    CBoard(QString ip, bool manual);
    ~CBoard();

    auto setOnline() -> void;
    auto getIP() -> QString;
    auto getName() -> QString;
    auto isManual() -> bool;
    auto setManual(bool manual) -> void;
    auto isOnline() -> bool;
    auto setMode(broadcast_lib::EMode mode) -> void;
    auto setModel(uint8_t model) -> void;
    auto setMac(const QString& mac) -> void;
    auto setGpio(bool gpio) -> void;
    auto getModel() -> QString;
    auto getIsMaster() -> bool;
    auto getConfigManagerConnected() -> bool;
    auto getLink() -> QString;
    auto getIsADCStarted() -> bool;
    auto getCaps() -> QVariantMap;
    auto getState() -> QString;
    auto getStats() -> QVariantMap;
    auto getTrigger() -> QVariantMap;
    auto settingsRev() -> int;
    auto settingsLoaded() -> bool;
    auto pcRev() -> int;
    auto getChartEnable() -> bool;
    auto setChartEnable(bool enable) -> void;

    Q_INVOKABLE QObject* getConsoleModel();

    // A setting of the board by its key (settings_lib), as the board last told it
    Q_INVOKABLE QString value(const QString& key);
    // Sets a setting on the board at once; false when the value is not valid for the key
    Q_INVOKABLE bool setValue(const QString& key, const QString& value);
    Q_INVOKABLE void readSettings();
    Q_INVOKABLE void saveDefaults();
    Q_INVOKABLE QString settingsJson();
    // false when the document is not one of settings
    Q_INVOKABLE bool loadSettingsJson(const QString& json);

    // The settings of this board kept on this computer: recordDir ("" - the one of the
    // application), nameTemplate, openFolderOnStop, saveConsole, consoleLevel
    Q_INVOKABLE QVariant pcValue(const QString& key);
    Q_INVOKABLE void setPcValue(const QString& key, const QVariant& value);
    // the folder and the first file name a recording gets now
    Q_INVOKABLE QString recordDir();
    Q_INVOKABLE QString exampleFileName();
    Q_INVOKABLE void openRecordFolder();

    Q_INVOKABLE void startStreaming();
    Q_INVOKABLE void stopStreaming();
    Q_INVOKABLE void softwareTrigger();
    Q_INVOKABLE void reconnect();

    auto startStreaming(QDateTime time) -> void;
    // a line from Start all / Stop all in the console of the board
    auto note(const QString& msg, bool error) -> void;

   signals:
    void isOnlineChanged();
    void isMasterChanged();
    void modelChanged();
    void sourceChanged();
    void configManagerConnectedChanged();
    void stateChanged();
    void statsChanged();
    void triggerChanged();
    void settingsChanged();
    void pcSettingsChanged();
    // a stream lost data (for Stop all on loss)
    void dataLost();
    // a setting the board refused
    void settingRefused(QString key);

   private:
    struct SStat {
        uint64_t bytes = 0;
        uint64_t samples[4] = {0, 0, 0, 0};
        uint64_t lostBoard = 0;
        uint64_t lostFile = 0;
    };

    // Error and Warning show with "Errors only"; the console colours them
    enum class Log { Error = 0, Warning = 1, Normal = 2, Detail = 3 };

    auto post(std::function<void()> f) -> void;
    auto addLog(const QString& msg, Log level = Log::Normal) -> void;
    auto setState(const QString& state) -> void;
    auto connectHandlers() -> void;
    auto connectConfig() -> void;
    auto updateOffline() -> void;
    auto applyBoardInfo(const BoardInfoMessage& info) -> void;
    auto settings() -> CStreamSettings*;
    auto adcStopped(const QString& why, bool error) -> void;
    auto createStreaming() -> void;
    auto startADCFPGAStreaming() -> void;
    auto closeStreaming() -> void;
    auto updateSpeed() -> void;
    auto pollTrigger() -> void;
    auto fileBaseName() -> QString;
    auto pcKey(const QString& key) -> QString;
    auto setLink(const QString& link) -> bool;
    auto adcSummary() -> QString;
    auto statSummary() -> QString;

    QString m_ip;
    bool m_manual;
    QTimer* m_offlineTimer;
    QTimer* m_reconnectTimer;
    QTimer* m_statTimer;
    QTimer* m_trigTimer;
    bool m_broadcastOnline = false;
    qint64 m_lastOnline = 0;
    broadcast_lib::EMode m_mode = broadcast_lib::AB_NONE;
    uint8_t m_model = 255;
    QString m_mac;
    bool m_gpio = false;
    bool m_infoFromBoard = false;
    BoardInfoMessage m_info;
    ClientNetConfigManager* m_configManager;
    bool m_connected = false;
    QString m_link = "connecting";
    // false once another client took the board: no fight over it, the user reconnects
    bool m_autoReconnect = true;
    qint64 m_lastLossLog = 0;
    uint64_t m_loggedLost = 0;
    bool m_settingsLoaded = false;
    int m_settingsRev = 0;
    int m_pcRev = 0;
    bool m_chartEnable = true;
    ConsoleModel m_consoleModel;
    QString m_state = "stopped";
    bool m_adcArmed = false;
    bool m_trigKnown = false;
    uint32_t m_adcTriggerCount = 0;
    bool m_stopRequested = false;
    QDateTime m_streamingDT;
    QString m_fileName;
    QFile m_consoleFile;

    streaming_lib::CStreamingFile::Ptr m_file_manager;
    net_lib::CAsioNet::Ptr m_asionet;
    DataLib::CBuffersCached::Ptr m_buffer;
    // owns the memory of the buffers of m_buffer (the client allocates it on the heap)
    std::shared_ptr<uio_lib::CMemoryManager> m_memory;
    adc_channels_t m_activeChannels;
    uint32_t m_blockSize = 0;

    std::mutex m_statMtx;
    SStat m_stat;
    uint64_t m_lastBytes = 0;
    qint64 m_lastBytesTime = 0;
    double m_speed = 0;
    uint64_t m_reportedLost = 0;
};

#endif  // BOARD_H
