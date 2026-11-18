#include "board.h"
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSettings>
#include <QUrl>
#include "asio.hpp"

#include "src/app_settings.h"
#include "src/logic/chartdataholder.h"
#include "uio_lib/memory_manager.h"

namespace {

const char* const DEFAULT_NAME_TEMPLATE = "{board}_{date}_{time}";

auto pcSettings() -> QSettings {
    return QSettings("RedPitaya", "rpsa_client_qt");
}

auto formatExtension(const QString& format) -> QString {
    const QString f = format.toLower();
    return f == "tdms" ? ".tdms" : f == "bin" ? ".bin" : ".wav";
}

}  // namespace

CBoard::CBoard(QString ip, bool manual) : QObject{nullptr}, m_ip(ip), m_manual(manual) {
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    m_chartEnable = pcValue("preview").toBool();
    updatePreviewWanted();
    ChartDataHolder::instance()->setDecimation(m_ip, previewDecimation());
    connect(AppSettings::instance(), &AppSettings::changed, this, &CBoard::updatePreviewWanted);

    m_offlineTimer = new QTimer(this);
    connect(m_offlineTimer, &QTimer::timeout, this, &CBoard::updateOffline);
    m_offlineTimer->start(2000);

    // the settings connection again while the board is away or its application is not running;
    // not when another client took the board: the two would take it from each other for ever
    m_reconnectTimer = new QTimer(this);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (m_autoReconnect && !m_configManager->isConnected(m_ip.toStdString()) && (m_manual || m_broadcastOnline))
            connectConfig();
    });
    m_reconnectTimer->start(5000);

    m_statTimer = new QTimer(this);
    connect(m_statTimer, &QTimer::timeout, this, &CBoard::updateSpeed);
    m_statTimer->start(500);

    m_trigTimer = new QTimer(this);
    connect(m_trigTimer, &QTimer::timeout, this, &CBoard::pollTrigger);
    m_trigTimer->start(1000);

    m_configManager = new ClientNetConfigManager("", false);
    connectHandlers();
    connectConfig();
}

CBoard::~CBoard() {
    closeStreaming();
    // joins the threads of the connection: no handler runs after this
    delete m_configManager;
    ChartDataHolder::instance()->removeRP(m_ip);
}

auto CBoard::post(std::function<void()> f) -> void {
    QMetaObject::invokeMethod(this, f, Qt::QueuedConnection);
}

auto CBoard::addLog(const QString& msg, Log level) -> void {
    const QString tag = level == Log::Error ? "Error: " : level == Log::Warning ? "Warning: " : "";
    const QString line = QTime::currentTime().toString("hh:mm:ss") + "  " + tag + msg;
    post([this, line, level]() {
        // consoleLevel: 0 errors and warnings, 1 normal, 2 detailed
        if (static_cast<int>(level) > pcValue("consoleLevel").toInt() + 1)
            return;
        m_consoleModel.addNewLine(line, static_cast<int>(level));
        if (m_consoleFile.isOpen()) {
            m_consoleFile.write((line + "\n").toUtf8());
            m_consoleFile.flush();
        }
    });
}

auto CBoard::setState(const QString& state) -> void {
    if (m_state == state)
        return;
    m_state = state;
    Q_EMIT stateChanged();
}

auto CBoard::setLink(const QString& link) -> bool {
    if (m_link == link)
        return false;
    m_link = link;
    Q_EMIT configManagerConnectedChanged();
    return true;
}

auto CBoard::connectConfig() -> void {
    if (m_link != "refused" && m_link != "unreachable")
        setLink("connecting");
    addLog("Connecting to " + m_ip + ":" + QString::number(NET_CONFIG_PORT), Log::Detail);
    std::vector<std::string> hosts{m_ip.toStdString()};
    m_configManager->connectToServers(hosts, NET_CONFIG_PORT);
}

auto CBoard::connectHandlers() -> void {
    auto* cm = m_configManager;

    cm->serverConnectedNofiy.connect([this](std::string&) {
        post([this]() {
            m_connected = true;
            setMode(m_configManager->getModeByHost(m_ip.toStdString()));
            setLink("connected");
            addLog("Connected to " + m_ip + " (" + (getIsMaster() ? "clock master" : "daisy chain slave") + ")");
            Q_EMIT configManagerConnectedChanged();
            Q_EMIT isOnlineChanged();
            m_configManager->requestBoardInfo(m_ip.toStdString());
            m_configManager->requestConfig(m_ip.toStdString());
        });
    });

    cm->errorNofiy.connect([this](ClientNetConfigManager::Errors error, std::string, std::error_code err) {
        if (error == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG) {
            // the connection is fine: a value of the board did not fit the settings of this program
            addLog("The settings of the board could not be taken: one of its values is not accepted by this program "
                   "(an older or newer version?). The settings shown are not the ones of the board.",
                   Log::Error);
            return;
        }
        const QString msg = QString::fromStdString(err.message());
        const bool closed = err == asio::error::eof || err == asio::error::connection_reset || err == asio::error::broken_pipe;
        const bool refused = err == asio::error::connection_refused;
        const bool timeout = error == ClientNetConfigManager::Errors::CONNECT_TIMEOUT || err == asio::error::timed_out ||
                             err == asio::error::host_unreachable || err == asio::error::network_unreachable;
        post([this, msg, closed, refused, timeout]() {
            const bool was = m_connected;
            m_connected = m_configManager->isConnected(m_ip.toStdString());
            if (was && !m_connected && m_state != "stopped")
                adcStopped("the connection to the board is lost", true);
            if (was && !m_connected && closed) {
                // the board serves one client: a new one closes the connection of the old one
                m_autoReconnect = false;
                setLink("taken");
                addLog("The board closed the connection: another client took it (another copy of this program, rpsa_client, "
                       "a script with the streaming API or the Streaming page of the board), or its streaming application stopped. "
                       "The board serves one client at a time. Reconnect takes it back.",
                       Log::Error);
            } else if (refused) {
                if (setLink("refused"))
                    addLog(m_ip + " refuses the connection on port " + QString::number(NET_CONFIG_PORT) +
                               ": the streaming application is not running on the board. Start it in the web interface of the "
                               "board (Streaming); the client connects by itself.",
                           Log::Error);
            } else if (timeout) {
                if (setLink("unreachable"))
                    addLog("No answer from " + m_ip + ": the board is off, booting or not reachable from this computer. "
                               "Trying again every 5 s.",
                           Log::Error);
            } else if (was && !m_connected) {
                setLink("unreachable");
                addLog("The connection to the board is lost: " + msg, Log::Error);
            } else if (!m_connected) {
                addLog("No connection: " + msg, Log::Detail);
            }
            Q_EMIT configManagerConnectedChanged();
            Q_EMIT isOnlineChanged();
        });
    });

    cm->getBoardInfoNofiy.connect([this](std::string&, BoardInfoMessage info) { post([this, info]() { applyBoardInfo(info); }); });
    cm->failBoardInfoNofiy.connect([this](std::string&) {
        addLog("The streaming application of the board is older and does not tell what the board has: the client goes by its model",
               Log::Warning);
    });

    cm->getNewSettingsNofiy.connect([this](std::string&) {
        post([this]() {
            const bool first = !m_settingsLoaded;
            m_settingsLoaded = true;
            m_settingsRev++;
            if (first)
                addLog("Settings read from the board. ADC: " + adcSummary());
            Q_EMIT settingsChanged();
        });
    });
    cm->getNewSettingsItemNofiy.connect([this](std::string&) {
        post([this]() {
            m_settingsRev++;
            Q_EMIT settingsChanged();
        });
    });
    cm->failSendConfigNofiy.connect([this](std::string&) {
        post([this]() {
            addLog("The board refused the last setting (see the log of the board): the settings are read again", Log::Error);
            m_configManager->requestConfig(m_ip.toStdString());
        });
    });

    // ADC: 17 start -> 25 TCP (or 26 SD) -> block size -> channels -> data connection -> 27 -> 28
    cm->serverStartedTCPNofiy.connect([this](std::string& host) {
        addLog("The board is ready to send; asking for its buffers", Log::Detail);
        m_configManager->requestMemoryBlockSize(host);
    });
    cm->serverStartedSDNofiy.connect([this](std::string&) {
        post([this]() {
            addLog("The ADC writes to the SD card of the board (/home/redpitaya/streaming_files): nothing comes to this computer");
            setState("sd");
            m_configManager->sendADCFPGAStart(m_ip.toStdString());
        });
    });
    cm->getMemBlockSizeNofiy.connect([this](std::string& host, std::string size) {
        try {
            m_blockSize = std::stoul(size);
        } catch (...) {
            m_blockSize = 0;
        }
        addLog("Buffers of " + QString::fromStdString(size) + " bytes", Log::Detail);
        m_configManager->requestActiveChannels(host);
    });
    cm->getActiveChannelsNofiy.connect([this](std::string&, adc_channels_t channels) {
        post([this, channels]() mutable {
            m_activeChannels = channels;
            addLog("Inputs on the board: " + QString::fromStdString(channels.format()), Log::Detail);
            if (m_stopRequested)
                return;
            createStreaming();
        });
    });
    cm->startADCDoneNofiy.connect([this](std::string&) {
        post([this]() {
            if (m_state == "starting")
                setState("streaming");
            addLog("ADC started: " + adcSummary());
        });
    });

    auto stopped = [this](const char* why, bool error) {
        return [this, why, error](std::string&) { post([this, why, error]() { adcStopped(why, error); }); };
    };
    cm->serverStoppedNofiy.connect(stopped("", false));
    cm->serverStoppedMemErrorNofiy.connect(stopped("not enough DMA memory for the ADC (Memory tab)", true));
    cm->serverStoppedMemModifyNofiy.connect(stopped("the memory settings changed", true));
    cm->serverStoppedNoActiveChannelsNofiy.connect(stopped("no input is on (ADC tab, Channels)", true));
    cm->serverStoppedConfigErrorNofiy.connect(stopped("the board cannot stream with these settings (see the log of the board)", true));
    cm->serverStoppedSDFullNofiy.connect(stopped("the SD card is full", true));
    cm->serverStoppedSDDoneNofiy.connect(stopped("the sample limit is written to the SD card", false));

    cm->getTriggerStatusNofiy.connect([this](std::string&, TriggerStatus status) {
        post([this, status]() {
            if (!m_trigKnown || m_adcArmed != status.adcArmed || m_adcTriggerCount != status.adcTriggerCount) {
                m_trigKnown = true;
                m_adcTriggerCount = status.adcTriggerCount;
                m_adcArmed = status.adcArmed;
                emit triggerChanged();
            }
            m_adcArmed = status.adcArmed;
            if (m_state == "streaming" && m_adcArmed)
                setState("armed");
            else if (m_state == "armed" && !m_adcArmed)
                setState("streaming");
        });
    });
    cm->softwareTriggerDoneNofiy.connect([this](std::string&, CNetConfigManager::ETriggerTarget) { addLog("Software trigger sent"); });
    cm->softwareTriggerFailNofiy.connect([this](std::string&, CNetConfigManager::ETriggerTarget) {
        addLog("Software trigger: the ADC does not wait for a trigger", Log::Warning);
    });
}

auto CBoard::applyBoardInfo(const BoardInfoMessage& info) -> void {
    m_info = info;
    m_infoFromBoard = true;
    m_model = info.model;
    if (!info.mac.empty() && info.mac != "NONE")
        m_mac = QString::fromStdString(info.mac);
    m_gpio = info.gpio;
    setMode(info.master ? broadcast_lib::AB_SERVER_MASTER : broadcast_lib::AB_SERVER_SLAVE);
    Q_EMIT modelChanged();
    addLog(getModel() + ", MAC " + m_mac + ": ADC " + QString::number(info.adcChannels) + " ch " +
               QString::number(info.adcRate / 1e6, 'g', 6) + " MHz " + QString::number(info.adcBits) + " bit, DAC " +
               (info.dac && info.dacChannels ? QString::number(info.dacChannels) + " ch" : QString("none")) + ", GPIO " +
               (info.gpio ? "yes" : "no") + ", DMA memory " + QString::number(info.memorySize / 1048576.0, 'f', 1) + " MB",
           Log::Detail);
}

namespace {

auto bytesText(double b) -> QString {
    if (b >= 1073741824)
        return QString::number(b / 1073741824, 'f', 2) + " GB";
    if (b >= 1048576)
        return QString::number(b / 1048576, 'f', 2) + " MB";
    if (b >= 1024)
        return QString::number(b / 1024, 'f', 1) + " kB";
    return QString::number(b, 'f', 0) + " B";
}

auto rateText(double r) -> QString {
    if (r >= 1e6)
        return QString::number(r / 1e6, 'g', 6) + " MS/s";
    if (r >= 1e3)
        return QString::number(r / 1e3, 'g', 6) + " kS/s";
    return QString::number(r, 'g', 6) + " S/s";
}

}  // namespace

auto CBoard::adcSummary() -> QString {
    auto s = settings();
    if (!s)
        return "-";
    const int n = getCaps()["adcChannels"].toInt();
    QStringList on;
    for (int i = 1; i <= n; i++)
        if (s->getADCChannels(i).value == CStreamSettings::State::ON)
            on << "IN" + QString::number(i);
    const uint32_t dec = std::max<uint32_t>(1, s->getADCDecimation());
    const double rate = getCaps()["adcRate"].toDouble() / dec;
    const bool sd = s->getADCPassMode().value == CStreamSettings::PassMode::FILE;
    const uint64_t limit = s->getADCSamples();
    return (on.isEmpty() ? QString("no input on") : on.join(" ")) + ", decimation " + QString::number(dec) + " (" + rateText(rate) +
           " per channel), " + (s->getADCResolution().value == CStreamSettings::Resolution::BIT_8 ? "8" : "16") + " bit, " +
           QString(s->getADCFormat().name()) + " to " + (sd ? "the SD card of the board" : "this PC") +
           (limit ? ", " + QString::number(limit) + " samples per channel" : QString(", until the stop"));
}

auto CBoard::statSummary() -> QString {
    SStat s;
    {
        std::lock_guard lock(m_statMtx);
        s = m_stat;
    }
    uint64_t samples = 0;
    for (auto v : s.samples)
        samples = std::max(samples, v);
    return bytesText(double(s.bytes)) + " received, " + QString::number(samples) + " samples per channel, lost " +
           QString::number(s.lostBoard) + " on the board and " + QString::number(s.lostFile) + " on this PC";
}

auto CBoard::adcStopped(const QString& why, bool error) -> void {
    const bool wasStreaming = m_state != "stopped";
    const bool recorded = m_file_manager != nullptr;
    const QString file = m_fileName;
    closeStreaming();
    m_adcArmed = false;
    if (m_trigKnown) {
        m_trigKnown = false;
        m_adcTriggerCount = 0;
        emit triggerChanged();
    }
    m_stopRequested = false;
    setState("stopped");
    if (wasStreaming || !why.isEmpty())
        addLog(why.isEmpty() ? "ADC stopped" : "ADC stopped: " + why, error ? Log::Error : Log::Normal);
    if (recorded)
        addLog("Recording done: " + statSummary() + ". File " + file);
    if (error)
        Q_EMIT dataLost();
}

auto CBoard::settings() -> CStreamSettings* {
    return m_configManager->getLocalSettingsOfHost(m_ip.toStdString());
}

auto CBoard::updateOffline() -> void {
    const bool broadcast = QDateTime::currentMSecsSinceEpoch() - m_lastOnline <= 10000;
    if (broadcast != m_broadcastOnline) {
        m_broadcastOnline = broadcast;
        Q_EMIT isOnlineChanged();
        if (!broadcast && !m_manual) {
            addLog("The board stopped answering the broadcast search", Log::Warning);
            // it went away: once it is back the client connects again by itself
            m_autoReconnect = true;
        }
    }
}

auto CBoard::setOnline() -> void {
    m_lastOnline = QDateTime::currentMSecsSinceEpoch();
    if (!m_broadcastOnline) {
        m_broadcastOnline = true;
        Q_EMIT isOnlineChanged();
        addLog("Found by the broadcast search", Log::Detail);
        if (m_autoReconnect && !m_configManager->isConnected(m_ip.toStdString()))
            connectConfig();
    }
}

auto CBoard::isOnline() -> bool {
    return m_broadcastOnline || m_connected;
}

auto CBoard::getIP() -> QString {
    return m_ip;
}

auto CBoard::getName() -> QString {
    const QString host = hostNameFromMac(m_mac);
    return host.isEmpty() ? m_ip : host;
}

auto CBoard::isManual() -> bool {
    return m_manual;
}

auto CBoard::setManual(bool manual) -> void {
    if (m_manual == manual)
        return;
    m_manual = manual;
    Q_EMIT sourceChanged();
}

auto CBoard::setMode(broadcast_lib::EMode mode) -> void {
    if (mode == broadcast_lib::AB_NONE || mode == m_mode)
        return;
    m_mode = mode;
    Q_EMIT isMasterChanged();
}

auto CBoard::setModel(uint8_t model) -> void {
    if (m_infoFromBoard || model == m_model)
        return;
    m_model = model;
    Q_EMIT modelChanged();
}

auto CBoard::setMac(const QString& mac) -> void {
    if (m_infoFromBoard || mac.isEmpty() || mac == m_mac)
        return;
    m_mac = mac;
    Q_EMIT modelChanged();
}

auto CBoard::setGpio(bool gpio) -> void {
    if (m_infoFromBoard || gpio == m_gpio)
        return;
    m_gpio = gpio;
    Q_EMIT modelChanged();
}

auto CBoard::getModel() -> QString {
    if (m_model == 255)
        return m_connected ? "Red Pitaya" : "Unknown board";
    return modelCaps(m_model).name;
}

auto CBoard::getIsMaster() -> bool {
    return m_mode != broadcast_lib::AB_SERVER_SLAVE;
}

auto CBoard::getConfigManagerConnected() -> bool {
    return m_connected;
}

auto CBoard::getLink() -> QString {
    return m_connected ? "connected" : m_link;
}

auto CBoard::getIsADCStarted() -> bool {
    return m_state != "stopped";
}

auto CBoard::getCaps() -> QVariantMap {
    const ModelCaps mc = modelCaps(m_model);
    QVariantMap c;
    if (m_infoFromBoard) {
        c["adcChannels"] = m_info.adcChannels;
        c["adcRate"] = double(m_info.adcRate);
        c["adcBits"] = m_info.adcBits;
        c["dacChannels"] = m_info.dac ? m_info.dacChannels : 0;
        c["dacRate"] = double(m_info.dacRate);
        c["adcRanges"] = m_info.adcRanges;
        c["adcACDC"] = m_info.adcACDC;
        c["dacGain"] = m_info.dacGain;
        c["adc"] = m_info.adc;
        c["dac"] = m_info.dac && m_info.dacChannels > 0;
        c["gpio"] = m_info.gpio;
        c["adcBlockMode"] = m_info.adcBlockMode;
        c["dacTriggerOutputs"] = m_info.dacTriggerOutputs;
        c["extTrigger"] = m_info.extTrigger;
        c["memorySize"] = double(m_info.memorySize);
        c["memoryMaxBlock"] = double(m_info.memoryMaxBlock);
    } else {
        c["adcChannels"] = mc.adcChannels;
        c["adcRate"] = double(mc.adcRate);
        c["adcBits"] = mc.adcBits;
        c["dacChannels"] = mc.dacChannels;
        c["dacRate"] = double(mc.dacRate);
        c["adcRanges"] = mc.adcRanges;
        c["adcACDC"] = mc.adcACDC;
        c["dacGain"] = mc.dacGain;
        c["adc"] = true;
        c["dac"] = mc.dacChannels > 0;
        c["gpio"] = m_gpio;
        c["adcBlockMode"] = false;
        c["dacTriggerOutputs"] = false;
        c["extTrigger"] = false;
        c["memorySize"] = 0.0;
        c["memoryMaxBlock"] = 0.0;
    }
    c["extTriggerLevel"] = mc.extTriggerLevel;
    c["fixedRange"] = mc.fixedRange;
    c["headerSize"] = DataLib::sizeHeader();
    c["fromBoard"] = m_infoFromBoard;
    c["mac"] = m_mac;
    return c;
}

auto CBoard::getTrigger() -> QVariantMap {
    QVariantMap t;
    t["known"] = m_trigKnown;
    t["armed"] = m_adcArmed;
    t["count"] = double(m_adcTriggerCount);
    return t;
}

auto CBoard::getState() -> QString {
    return m_state;
}

auto CBoard::getStats() -> QVariantMap {
    SStat s;
    {
        std::lock_guard lock(m_statMtx);
        s = m_stat;
    }
    QVariantMap m;
    m["received"] = double(s.bytes);
    m["speed"] = m_speed;
    uint64_t samples = 0;
    for (auto v : s.samples)
        samples = std::max(samples, v);
    m["samples"] = double(samples);
    m["lostBoard"] = double(s.lostBoard);
    m["lostFile"] = double(s.lostFile);
    m["file"] = m_fileName;
    m["fileRate"] = s.fileRate;
    m["fileQueue"] = s.fileQueue;
    return m;
}

auto CBoard::updateSpeed() -> void {
    uint64_t bytes = 0, lost = 0, lostBoard = 0, lostFile = 0;
    {
        std::lock_guard lock(m_statMtx);
        bytes = m_stat.bytes;
        lostBoard = m_stat.lostBoard;
        lostFile = m_stat.lostFile;
        lost = lostBoard + lostFile;
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastBytesTime && now > m_lastBytesTime && bytes >= m_lastBytes) {
        const double inst = double(bytes - m_lastBytes) * 1000.0 / double(now - m_lastBytesTime);
        // smoothed over about two seconds
        m_speed = m_speed == 0 ? inst : m_speed * 0.75 + inst * 0.25;
    }
    if (m_state == "stopped")
        m_speed = 0;
    m_lastBytes = bytes;
    m_lastBytesTime = now;
    if (lost > m_reportedLost) {
        m_reportedLost = lost;
        Q_EMIT dataLost();
    }
    // the losses in the console, at most every 5 s
    if (lost > m_loggedLost && now - m_lastLossLog >= 5000) {
        addLog("Data lost: " + QString::number(lostBoard) + " samples on the board (the network or this computer does not keep up with " +
                   bytesText(m_speed) + "/s: a higher decimation, 8 bit or fewer inputs help), " + QString::number(lostFile) +
                   " on this PC (the disk is too slow)",
               Log::Warning);
        m_loggedLost = lost;
        m_lastLossLog = now;
    }
    Q_EMIT statsChanged();
}

auto CBoard::pollTrigger() -> void {
    if ((m_state == "streaming" || m_state == "armed") && value("adc_trigger_mode") == "ON")
        m_configManager->requestTriggerStatus(m_ip.toStdString());
}

auto CBoard::settingsRev() -> int {
    return m_settingsRev;
}

auto CBoard::settingsLoaded() -> bool {
    return m_settingsLoaded;
}

auto CBoard::pcRev() -> int {
    return m_pcRev;
}

QObject* CBoard::getConsoleModel() {
    return &m_consoleModel;
}

QString CBoard::value(const QString& key) {
    auto s = settings();
    return s ? QString::fromStdString(s->getValue(key.toStdString())) : QString();
}

bool CBoard::setValue(const QString& key, const QString& value) {
    auto s = settings();
    if (!s || !m_connected) {
        addLog("Not connected: the setting is not sent", Log::Error);
        return false;
    }
    if (QString::fromStdString(s->getValue(key.toStdString())) == value)
        return true;
    CStreamSettings probe = *s;
    if (!probe.setValue(key.toStdString(), value.toStdString())) {
        addLog("Not a valid value of " + key + ": " + value, Log::Error);
        Q_EMIT settingRefused(key);
        m_settingsRev++;
        Q_EMIT settingsChanged();
        return false;
    }
    s->setValue(key.toStdString(), value.toStdString());
    if (!m_configManager->sendConfigVariable(m_ip.toStdString(), key.toStdString(), value.toStdString())) {
        addLog("The setting is not sent: " + key, Log::Error);
        return false;
    }
    addLog("Set " + key + " = " + value);
    m_settingsRev++;
    Q_EMIT settingsChanged();
    return true;
}

void CBoard::readSettings() {
    if (!m_configManager->requestConfig(m_ip.toStdString()))
        addLog("Not connected: the settings are not read", Log::Error);
}

void CBoard::saveDefaults() {
    auto s = settings();
    if (!s || !m_connected)
        return;
    CStreamSettings defaults;
    defaults.resetDefault();
    // sendConfig sends the settings of the manager itself
    static_cast<CStreamSettings&>(*m_configManager).copy(defaults);
    if (!m_configManager->sendConfig(m_ip.toStdString())) {
        addLog("Not connected: the defaults are not sent", Log::Error);
        return;
    }
    addLog("The default settings are sent");
    m_configManager->requestConfig(m_ip.toStdString());
}

QString CBoard::settingsJson() {
    auto s = settings();
    return s ? QString::fromStdString(s->toJson()) : QString();
}

bool CBoard::loadSettingsJson(const QString& json) {
    CStreamSettings parsed;
    if (!parsed.parseJson(json.toStdString()))
        return false;
    if (!m_connected)
        return false;
    static_cast<CStreamSettings&>(*m_configManager).copy(parsed);
    if (!m_configManager->sendConfig(m_ip.toStdString()))
        return false;
    addLog("Settings sent from a file");
    m_configManager->requestConfig(m_ip.toStdString());
    return true;
}

auto CBoard::pcKey(const QString& key) -> QString {
    return "board/" + m_ip + "/" + key;
}

QVariant CBoard::pcValue(const QString& key) {
    auto s = pcSettings();
    if (key == "nameTemplate")
        return s.value(pcKey(key), DEFAULT_NAME_TEMPLATE);
    if (key == "consoleLevel")
        return s.value(pcKey(key), 1);
    if (key == "preview")
        return s.value(pcKey(key), true);
    if (key == "previewDecimation")
        return s.value(pcKey(key), 1);
    if (key == "openFolderOnStop" || key == "saveConsole" || key == "previewFit")
        return s.value(pcKey(key), false);
    return s.value(pcKey(key), "");
}

void CBoard::setPcValue(const QString& key, const QVariant& value) {
    auto s = pcSettings();
    if (s.value(pcKey(key)) == value)
        return;
    s.setValue(pcKey(key), value);
    m_pcRev++;
    Q_EMIT pcSettingsChanged();
}

auto CBoard::getChartEnable() -> bool {
    return m_chartEnable;
}

auto CBoard::setChartEnable(bool enable) -> void {
    m_chartEnable = enable;
    updatePreviewWanted();
    setPcValue("preview", enable);
}

auto CBoard::updatePreviewWanted() -> void {
    m_previewWanted = m_chartEnable || AppSettings::instance()->view() == "list";
}

auto CBoard::previewFit() -> bool {
    return pcValue("previewFit").toBool();
}

auto CBoard::setPreviewFit(bool fit) -> void {
    setPcValue("previewFit", fit);
}

auto CBoard::previewDecimation() -> int {
    return std::max(0, pcValue("previewDecimation").toInt());
}

auto CBoard::setPreviewDecimation(int samplesPerPoint) -> void {
    ChartDataHolder::instance()->setDecimation(m_ip, std::max(0, samplesPerPoint));
    setPcValue("previewDecimation", std::max(0, samplesPerPoint));
}

QString CBoard::recordDir() {
    const QString own = pcValue("recordDir").toString().trimmed();
    if (own.isEmpty())
        return QDir::toNativeSeparators(AppSettings::instance()->recordDirFor(getName()));
    return QDir::toNativeSeparators(QDir::cleanPath(AppSettings::expandHome(own)));
}

auto CBoard::fileBaseName() -> QString {
    QString name = pcValue("nameTemplate").toString().trimmed();
    if (name.isEmpty())
        name = DEFAULT_NAME_TEMPLATE;
    const QDateTime t = m_streamingDT.isValid() ? m_streamingDT : QDateTime::currentDateTime();
    name.replace("{board}", getName());
    name.replace("{ip}", m_ip);
    name.replace("{date}", t.toString("yyyy-MM-dd"));
    name.replace("{time}", t.toString("HH-mm-ss"));
    // no folders or characters a file system refuses
    static const QRegularExpression bad(R"([\\/:*?"<>|])");
    name.replace(bad, "_");
    return name;
}

QString CBoard::exampleFileName() {
    return fileBaseName() + formatExtension(value("format_sd"));
}

void CBoard::openRecordFolder() {
    const QString dir = recordDir();
    QDir().mkpath(dir);
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

auto CBoard::note(const QString& msg, bool error) -> void {
    addLog(msg, error ? Log::Error : Log::Normal);
}

void CBoard::startStreaming() {
    startStreaming(QDateTime::currentDateTime());
}

auto CBoard::startStreaming(QDateTime time) -> void {
    if (m_state != "stopped")
        return;
    if (!m_connected) {
        addLog("Not connected to the board", Log::Error);
        return;
    }
    m_streamingDT = time;
    m_stopRequested = false;
    setState("starting");
    addLog("Starting the ADC: " + adcSummary());
    if (!m_configManager->sendADCServerStart(m_ip.toStdString())) {
        addLog("The start is not sent", Log::Error);
        setState("stopped");
    }
}

void CBoard::stopStreaming() {
    if (m_state == "stopped")
        return;
    m_stopRequested = true;
    addLog("Stopping the ADC");
    const QString file = m_fileName;
    const bool recorded = m_file_manager != nullptr;
    const QString summary = statSummary();
    closeStreaming();
    if (recorded)
        addLog("Recording done: " + summary + ". File " + file);
    setState("stopping");
    if (!m_configManager->sendADCServerStop(m_ip.toStdString()))
        adcStopped("", false);
}

void CBoard::softwareTrigger() {
    m_configManager->sendSoftwareTrigger(m_ip.toStdString(), CNetConfigManager::TT_ADC);
}

void CBoard::reconnect() {
    m_autoReconnect = true;
    setLink("connecting");
    addLog("Reconnecting to " + m_ip + ": another client connected to the board loses it");
    connectConfig();
}

auto CBoard::closeStreaming() -> void {
    const bool recording = m_file_manager != nullptr;
    m_asionet = nullptr;
    if (m_file_manager) {
        m_file_manager->disableNotify();
        m_file_manager->stopAndFlush();
        m_file_manager = nullptr;
    }
    ChartDataHolder::instance()->removeRP(m_ip);
    m_buffer = nullptr;
    // the preview may still read a pack of these buffers for a moment
    if (m_memory) {
        auto memory = m_memory;
        m_memory = nullptr;
        QTimer::singleShot(3000, [memory]() {});
    }
    if (m_consoleFile.isOpen())
        m_consoleFile.close();
    if (recording && pcValue("openFolderOnStop").toBool())
        openRecordFolder();
}

auto CBoard::createStreaming() -> void {
    auto s = settings();
    if (!s)
        return;
    m_buffer = DataLib::CBuffersCached::create();
    {
        std::lock_guard lock(m_statMtx);
        m_stat = SStat();
    }
    m_reportedLost = 0;
    m_loggedLost = 0;
    m_speed = 0;
    m_lastBytes = 0;

    const bool convertVolts = s->getADCType().value == CStreamSettings::DataType::VOLT;
    const uint64_t limit = s->getADCSamples();
    std::string dir = recordDir().toStdString();
    QDir().mkpath(recordDir());
    const QString file = QDir(recordDir()).filePath(fileBaseName() + formatExtension(value("format_sd")));
    m_file_manager = streaming_lib::CStreamingFile::create(s->getADCFormat(), dir, limit ? limit : static_cast<uint64_t>(-1), convertVolts, false);
    m_file_manager->setTDMSTimeColumn(s->getTDMSTimeColumn().value == CStreamSettings::State::ON);
    m_file_manager->stopNotify.connect([this](streaming_lib::CStreamingFile::EStopReason reason) {
        post([this, reason]() {
            if (reason == streaming_lib::CStreamingFile::REACH_LIMIT)
                addLog("The sample limit is written");
            else if (reason == streaming_lib::CStreamingFile::OUT_SPACE)
                addLog("The disk of this computer is full", Log::Error);
            stopStreaming();
        });
    });
    m_file_manager->run("", file.toStdString());
    m_fileName = QDir::toNativeSeparators(file);
    addLog("Recording to " + m_fileName);
    if (pcValue("saveConsole").toBool()) {
        m_consoleFile.setFileName(file + ".console.txt");
        if (!m_consoleFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            addLog("The console file cannot be written", Log::Error);
    }

    auto fileW = std::weak_ptr<streaming_lib::CStreamingFile>(m_file_manager);
    auto buffersW = std::weak_ptr<DataLib::CBuffersCached>(m_buffer);

    m_asionet = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_ip.toStdString(), NET_ADC_STREAMING_PORT, m_buffer);

    // the data thread: counts, writes and hands a pack now and then to the preview
    m_asionet->reciveNotify.connect([this, fileW, buffersW](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        if (error)
            return;
        auto fileObj = fileW.lock();
        auto buffers = buffersW.lock();
        if (!fileObj || !buffers)
            return;
        pack->getInfoFromHeaderADC();
        uint64_t bytes = 0, lost = 0, packId = 0;
        uint64_t samples[4] = {0, 0, 0, 0};
        const DataLib::EDataBuffersPackChannel chs[4] = {DataLib::CH1, DataLib::CH2, DataLib::CH3, DataLib::CH4};
        for (int i = 0; i < 4; i++) {
            auto ch = pack->getBuffer(chs[i]);
            if (!ch)
                continue;
            samples[i] = ch->getSamplesCount();
            bytes += ch->getDataLenght();
            lost += ch->getLostSamplesAll();
            packId = std::max(packId, ch->getADCPackId());
        }
        const uint64_t fileLost = fileObj->getFileLost();
        const double fileRate = fileObj->getWriteRate();
        const double fileQueue = fileObj->getQueueFill();
        {
            std::lock_guard lock(m_statMtx);
            m_stat.bytes += bytes;
            for (int i = 0; i < 4; i++)
                m_stat.samples[i] += samples[i];
            m_stat.lostBoard += lost;
            m_stat.lostFile = fileLost;
            m_stat.fileRate = fileRate;
            m_stat.fileQueue = fileQueue;
        }
        fileObj->passBuffers(pack);
        ChartDataHolder::instance()->addBuffer(pack, packId, m_ip, !m_previewWanted);
        buffers->unlockBufferRead();
    });

    m_asionet->clientConnectNotify.connect([this](std::string) {
        post([this]() {
            addLog("Data connection open", Log::Detail);
            startADCFPGAStreaming();
        });
    });

    m_asionet->clientErrorNotify.connect([this](std::error_code err) {
        const QString msg = QString::fromStdString(err.message());
        post([this, msg]() {
            if (m_state == "stopped" || m_state == "stopping")
                return;
            addLog("Data connection: " + msg, Log::Error);
            stopStreaming();
        });
    });

    ChartDataHolder::instance()->regRP(m_ip);
    m_asionet->start();
}

auto CBoard::startADCFPGAStreaming() -> void {
    if (!m_buffer)
        return;
    m_memory = std::make_shared<uio_lib::CMemoryManager>();
    m_memory->setMemoryBlockSize(m_blockSize);
    m_memory->reallocateBlocks();
    const auto blocks = m_memory->getFreeBlockCount();
    m_memory->reserveMemory(uio_lib::MM_ADC, blocks, m_activeChannels.count());
    m_buffer->generateBuffersEmptyADC(m_activeChannels, m_memory->getRegions(uio_lib::MM_ADC), DataLib::sizeHeader());
    m_configManager->sendADCFPGAStart(m_ip.toStdString());
}
