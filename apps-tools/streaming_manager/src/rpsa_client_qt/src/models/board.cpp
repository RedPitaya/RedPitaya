// the GPIO headers before Qt: gpio_words.h has a method named emit
#include "bin_lib/bin_file.h"
#include "bin_lib/gpio_words.h"
#include "reader_lib/gpio_file_reader.h"
#include "writer_lib/gpio_file_writer.h"

#include "board.h"
#include <QTime>
#include <algorithm>
#include <cmath>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSettings>
#include <QStorageInfo>
#include <QUrl>
#include "asio.hpp"

#include "dac_streaming_lib/dac_streaming_manager.h"
#include "data_lib/network_header.h"
#include "reader_lib/reader_controller.h"
#include "src/app_settings.h"
#include "src/logic/chartdataholder.h"
#include "uio_lib/memory_manager.h"
#include "wav_lib/wav_reader.h"

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
    ChartDataHolder::instance()->setMeasure(m_ip, measure());
    applySpectrum();
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
    // the bytes the DAC sent
    connect(m_statTimer, &QTimer::timeout, this, [this]() {
        if (isDacRunning())
            Q_EMIT dacChanged();
        if (isGpioInRunning() || isGpioOutRunning())
            Q_EMIT gpioChanged();
    });
    m_statTimer->start(500);

    m_trigTimer = new QTimer(this);
    connect(m_trigTimer, &QTimer::timeout, this, &CBoard::pollTrigger);
    m_trigTimer->start(1000);
    connect(m_trigTimer, &QTimer::timeout, this, &CBoard::pollRecord);
    // the destination may change: the record of the other one at once
    connect(this, &CBoard::settingsChanged, this, &CBoard::recordChanged);

    m_configManager = new ClientNetConfigManager("", false);
    connectHandlers();
    connectDacHandlers();
    connectGpioHandlers();
    connectConfig();
    readDacFile();
    readGpioFile();
}

CBoard::~CBoard() {
    closeStreaming();
    m_dacTerminate = true;
    {
        std::lock_guard lock(m_dacNetMtx);
        if (m_dacNet)
            m_dacNet->disconnect();
    }
    if (m_dacThread.joinable())
        m_dacThread.join();
    if (m_dacFileThread.joinable())
        m_dacFileThread.join();
    for (auto* d : {&m_gin, &m_gout}) {
        d->terminate = true;
        if (d->thread.joinable())
            d->thread.join();
    }
    if (m_gpioFileThread.joinable())
        m_gpioFileThread.join();
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
            post([this]() {
                setError("The settings of the board could not be taken: one of its values is not accepted by this program "
                         "(an older or newer version?). The settings shown are not the ones of the board.");
            });
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
                adcStopped("the connection to the board is lost", true, true);
            if (was && !m_connected && isDacRunning())
                dacStopped("the connection to the board is lost", true);
            if (was && !m_connected) {
                gpioBoardStopped(false, "the connection to the board is lost", true);
                gpioBoardStopped(true, "the connection to the board is lost", true);
            }
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
            dismissError();  // a start that goes
        });
    });

    auto stopped = [this](const char* why, bool error) {
        return [this, why, error](std::string&) { post([this, why, error]() { adcStopped(why, error); }); };
    };
    cm->serverStoppedNofiy.connect(stopped("", false));
    cm->serverStoppedMemErrorNofiy.connect(stopped("not enough DMA memory for the ADC (Memory tab)", true));
    cm->serverStoppedMemModifyNofiy.connect(stopped("the memory settings changed", true));
    cm->serverStoppedNoActiveChannelsNofiy.connect(stopped("no input is on (ADC tab, Channels)", true));
    cm->serverStoppedConfigErrorNofiy.connect(stopped("the board did not start the stream: the settings or the board (see its log: journalctl -u redpitaya_streaming)", true));
    cm->serverStoppedSDFullNofiy.connect(stopped("the SD card is full", true));
    cm->serverStoppedSDDoneNofiy.connect(stopped("the sample limit is written to the SD card", false));

    cm->getRecordStatusNofiy.connect([this](std::string&, RecordStatus st) {
        post([this, st]() {
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            const auto& a = st.adc;
            // the samples a second between two answers; a new run starts again
            if (a.samples < m_recSamples || !m_recTime || !a.running) {
                m_recRate = 0;
            } else if (now > m_recTime + 200 && a.samples > m_recSamples) {
                const double inst = double(a.samples - m_recSamples) * 1000.0 / double(now - m_recTime);
                m_recRate = m_recRate > 0 ? 0.7 * m_recRate + 0.3 * inst : inst;
            }
            m_recSamples = a.samples;
            m_recTime = now;
            m_sdRecord = a;
            if (m_gin.source == "sd" && !(m_sdGpioIn == st.gpioIn)) {
                m_sdGpioIn = st.gpioIn;
                Q_EMIT gpioChanged();
            }
            m_sdFree = st.sdFree;
            m_sdTotal = st.sdTotal;
            m_recordKnown = true;
            Q_EMIT recordChanged();
        });
    });

    cm->getTriggerStatusNofiy.connect([this](std::string&, TriggerStatus status) {
        post([this, status]() {
            if (!m_trigKnown || m_adcArmed != status.adcArmed || m_adcTriggerCount != status.adcTriggerCount) {
                m_trigKnown = true;
                m_adcTriggerCount = status.adcTriggerCount;
                m_adcArmed = status.adcArmed;
                emit triggerChanged();
            }
            m_adcArmed = status.adcArmed;
            if (m_gpioInArmed != status.gpioInArmed || m_gpioOutArmed != status.gpioOutArmed || m_gpioInTriggers != status.gpioInTriggerCount ||
                m_gpioOutWait != status.gpioOutWait) {
                m_gpioInArmed = status.gpioInArmed && isGpioInRunning();
                m_gpioOutArmed = status.gpioOutArmed && isGpioOutRunning();
                m_gpioInTriggers = status.gpioInTriggerCount;
                m_gpioOutWait = status.gpioOutWait;
                Q_EMIT gpioChanged();
            }
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

// the file of the recording; in parts the first, the last and how many
auto CBoard::recordedFiles() -> QString {
    if (!m_file_manager)
        return m_fileName;
    const auto names = m_file_manager->getFileNames();
    if (names.size() < 2)
        return m_fileName;
    return QDir::toNativeSeparators(QString::fromStdString(names.front())) + " ... " + QDir::toNativeSeparators(QString::fromStdString(names.back())) + " (" +
           QString::number(names.size()) + " parts)";
}

auto CBoard::adcStopped(const QString& why, bool error, bool broken) -> void {
    const bool wasStreaming = m_state != "stopped";
    const bool recorded = m_file_manager != nullptr;
    const QString file = recordedFiles();
    closeStreaming();
    m_adcArmed = false;
    if (m_trigKnown) {
        m_trigKnown = false;
        m_adcTriggerCount = 0;
        emit triggerChanged();
    }
    m_stopRequested = false;
    setState("stopped");
    if (error)
        setError(why.isEmpty() ? "ADC stopped" : "ADC stopped: " + why);
    else if (wasStreaming || !why.isEmpty())
        addLog(why.isEmpty() ? "ADC stopped" : "ADC stopped: " + why);
    if (recorded)
        addLog("Recording done: " + statSummary() + ". File " + file);
    // Stop all on loss: a stream that broke off, not a refusal of the start (no input on, ...)
    if (broken)
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
    m["file"] = s.fileNow.isEmpty() ? m_fileName : s.fileNow;
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

// every second: the board writing its SD card is asked for its record (an older server does
// not answer, the card then shows no progress); this computer: the space on the disk of the files
auto CBoard::pollRecord() -> void {
    const bool adcSd = value("adc_pass_mode") == "FILE";
    if ((adcSd || (m_gin.source == "sd" && isGpioInRunning())) && getConfigManagerConnected())
        m_configManager->requestRecordStatus(m_ip.toStdString());
    if (adcSd)
        return;
    const QStorageInfo disk = storageOf(recordDir());
    const uint64_t free = disk.isValid() ? static_cast<uint64_t>(disk.bytesAvailable()) : 0;
    const uint64_t total = disk.isValid() ? static_cast<uint64_t>(disk.bytesTotal()) : 0;
    if (free != m_pcFree || total != m_pcTotal || m_state != "stopped") {
        m_pcFree = free;
        m_pcTotal = total;
        Q_EMIT recordChanged();
    }
}

auto CBoard::getRecord() -> QVariantMap {
    QVariantMap r;
    if (value("adc_pass_mode") == "FILE") {
        r["dest"] = "sd";
        r["known"] = m_recordKnown;
        r["running"] = m_sdRecord.running && m_sdRecord.mode == StreamRecordStatus::SD;
        r["limit"] = double(m_sdRecord.limit);
        r["samples"] = double(m_sdRecord.samples);
        r["rate"] = m_recRate;
        r["written"] = double(m_sdRecord.written);
        r["write"] = m_sdRecord.writeRate;
        r["queue"] = m_sdRecord.queue;
        r["lostBoard"] = double(m_sdRecord.lostBoard);
        r["lostQueue"] = double(m_sdRecord.lostQueue);
        r["free"] = double(m_sdFree);
        r["total"] = double(m_sdTotal);
        r["file"] = QString::fromStdString(m_sdRecord.file);
        r["mode"] = int(m_sdRecord.mode);
        r["dataRate"] = dataRate();
        // the limits the run started with (the board says them)
        fillLimit(r, m_sdRecord.samples, m_sdRecord.written, double(m_sdRecord.limit), m_sdRecord.limitTime, double(m_sdRecord.limitSize),
                  double(m_sdRecord.splitSize), m_sdRecord.splitTime, int(m_sdRecord.part), m_sdRecord.partWritten);
        return r;
    }
    SStat s;
    {
        std::lock_guard lock(m_statMtx);
        s = m_stat;
    }
    uint64_t samples = 0;
    for (auto v : s.samples)
        samples = std::max(samples, v);
    r["dest"] = "pc";
    r["known"] = true;
    r["need"] = expectedBytes();
    r["dataRate"] = dataRate();
    r["running"] = m_state != "stopped" && m_state != "stopping";
    const QString limit = value("samples_limit_sd");
    r["limit"] = limit.toDouble();
    r["samples"] = double(samples);
    r["rate"] = 0.0;  // the card of the board takes the sample rate of the settings
    r["written"] = double(s.bytes);
    r["write"] = s.fileRate;
    r["queue"] = s.fileQueue;
    r["lostBoard"] = double(s.lostBoard);
    r["lostQueue"] = double(s.lostFile);
    r["free"] = double(m_pcFree);
    r["total"] = double(m_pcTotal);
    r["file"] = s.fileNow.isEmpty() ? m_fileName : s.fileNow;
    fillLimit(r, samples, s.bytes, value("samples_limit_sd").toDouble(), value("limit_time_sd").toDouble(), value("limit_size_sd").toDouble(),
              value("split_size_sd").toDouble(), value("split_time_sd").toDouble(), s.part, s.partBytes);
    // before the start: what the limit takes on this disk
    if (!r["running"].toBool())
        r["need"] = expectedBytes();
    return r;
}

auto CBoard::pollTrigger() -> void {
    if (((m_state == "streaming" || m_state == "armed") && value("adc_trigger_mode") == "ON") ||
        (isGpioInRunning() && value("gpio_in_trigger_mode") == "ON") || isGpioOutRunning())
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
        Q_EMIT settingRefused(key, value);
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

bool CBoard::loadSettingsJson(const QString& json, const QString& from) {
    CStreamSettings parsed;
    if (!parsed.parseJson(json.toStdString()))
        return false;
    if (!m_connected)
        return false;
    static_cast<CStreamSettings&>(*m_configManager).copy(parsed);
    if (!m_configManager->sendConfig(m_ip.toStdString()))
        return false;
    addLog("Settings sent from " + (from.isEmpty() ? QString("a file") : from));
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
    if (key == "spectrumWindow")
        return s.value(pcKey(key), 1);
    if (key == "spectrumAverage")
        return s.value(pcKey(key), 4);
    if (key == "dacRepeat")
        return s.value(pcKey(key), "once");
    if (key == "dacTimes")
        return s.value(pcKey(key), 10);
    if (key == "gpioOutRepeat")
        return s.value(pcKey(key), "once");
    if (key == "gpioOutTimes")
        return s.value(pcKey(key), 10);
    if (key == "gpioInFormat")
        return s.value(pcKey(key), "BIN");
    if (key == "gpioInLimitKind")
        return s.value(pcKey(key), "none");
    if (key == "gpioInLimit")
        return s.value(pcKey(key), 0);
    if (key == "openFolderOnStop" || key == "saveConsole" || key == "previewFit" || key == "measure" || key == "spectrum" || key == "gpioOutGaps" ||
        key == "gpioInOff" || key == "gpioOutOff")
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
    if (key == "dacFile")
        readDacFile();
    if (key == "gpioOutFile")
        readGpioFile();
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

auto CBoard::measure() -> bool {
    return pcValue("measure").toBool();
}

auto CBoard::setMeasure(bool on) -> void {
    ChartDataHolder::instance()->setMeasure(m_ip, on);
    setPcValue("measure", on);
}

auto CBoard::spectrum() -> bool {
    return pcValue("spectrum").toBool();
}

auto CBoard::setSpectrum(bool on) -> void {
    setPcValue("spectrum", on);
    applySpectrum();
}

auto CBoard::spectrumWindow() -> int {
    const int w = pcValue("spectrumWindow").toInt();
    return w == 0 || w == 1 || w == 4 ? w : 1;
}

auto CBoard::setSpectrumWindow(int window) -> void {
    setPcValue("spectrumWindow", window == 0 || window == 4 ? window : 1);
    applySpectrum();
}

auto CBoard::spectrumAverage() -> int {
    const int a = pcValue("spectrumAverage").toInt();
    return a == 1 || a == 16 ? a : 4;
}

auto CBoard::setSpectrumAverage(int blocks) -> void {
    setPcValue("spectrumAverage", blocks == 1 || blocks == 16 ? blocks : 4);
    applySpectrum();
}

auto CBoard::applySpectrum() -> void {
    ChartDataHolder::instance()->setSpectrum(m_ip, spectrum(), spectrumWindow(), spectrumAverage());
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

auto CBoard::fileBaseName(const QDateTime& when) -> QString {
    QString name = pcValue("nameTemplate").toString().trimmed();
    if (name.isEmpty())
        name = DEFAULT_NAME_TEMPLATE;
    const QDateTime t = when.isValid() ? when : m_streamingDT.isValid() ? m_streamingDT : QDateTime::currentDateTime();
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
    if (error && m_errorText.isEmpty())
        setError(msg);
    else
        addLog(msg, error ? Log::Error : Log::Normal);
}

auto CBoard::storageOf(const QString& dir) -> QStorageInfo {
    QDir d(dir);
    while (!d.exists() && !d.isRoot() && d.cdUp()) {
    }
    return QStorageInfo(d.exists() ? d.absolutePath() : dir);
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
        setError("The start is not sent: no connection to the board");
        setState("stopped");
    }
}

void CBoard::stopStreaming() {
    if (m_state == "stopped")
        return;
    m_stopRequested = true;
    addLog("Stopping the ADC");
    const bool recorded = m_file_manager != nullptr;
    const QString file = recordedFiles();
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
    const auto limits = s->getADCLimits();
    m_file_manager->setLimits(limits.time, limits.size, s->adcBufferSeconds(getCaps()["adcRate"].toDouble()));
    m_file_manager->setSplit(limits.splitSize, limits.splitTime);
    m_file_manager->stopNotify.connect([this](streaming_lib::CStreamingFile::EStopReason reason) {
        post([this, reason]() {
            if (reason == streaming_lib::CStreamingFile::REACH_LIMIT)
                addLog("The limit of the recording is written");
            else if (reason == streaming_lib::CStreamingFile::OUT_SPACE)
                setError("The disk of this computer is full: the recording stopped, its file is closed");
            stopStreaming();
        });
    });
    m_file_manager->run("", file.toStdString());
    // the file written: with parts the first of them (<name>_0001.<ext>)
    m_fileName = QDir::toNativeSeparators(QString::fromStdString(m_file_manager->getCSVFileName()));
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
        {
            // the part written now (the file goes on in the next one at its size or time)
            const int part = fileObj->getPart();
            const uint64_t partBytes = fileObj->getPartSize();
            std::lock_guard lock(m_statMtx);
            if (part != m_stat.part)
                m_stat.fileNow = QDir::toNativeSeparators(QString::fromStdString(fileObj->getCSVFileName()));
            m_stat.part = part;
            m_stat.partBytes = partBytes;
        }
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
            setError("The data connection broke: " + msg);
            stopStreaming();
            Q_EMIT dataLost();  // the stream broke off
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

auto CBoard::setError(const QString& msg) -> void {
    addLog(msg, Log::Error);
    m_errorText = msg;
    m_errorTime = QTime::currentTime().toString("hh:mm:ss");
    m_errorCount++;
    Q_EMIT errorChanged();
}

void CBoard::dismissError() {
    if (m_errorText.isEmpty() && !m_errorCount)
        return;
    m_errorText.clear();
    m_errorTime.clear();
    m_errorCount = 0;
    Q_EMIT errorChanged();
}

auto CBoard::getError() -> QVariantMap {
    QVariantMap m;
    if (m_errorText.isEmpty())
        return m;
    m["text"] = m_errorText;
    m["time"] = m_errorTime;
    m["count"] = m_errorCount;
    return m;
}

auto CBoard::dataRate() -> double {
    auto s = settings();
    if (!s)
        return 0;
    int on = 0;
    const int n = getCaps()["adcChannels"].toInt();
    for (int i = 1; i <= n; i++)
        if (s->getADCChannels(i).value == CStreamSettings::State::ON)
            on++;
    const uint32_t dec = std::max<uint32_t>(1, s->getADCDecimation());
    // volts are floats in WAV and TDMS; BIN keeps the codes
    const bool floats = s->getADCType().value == CStreamSettings::DataType::VOLT && value("format_sd") != "BIN";
    const int bytes = floats ? 4 : s->getADCResolution().value == CStreamSettings::Resolution::BIT_8 ? 1 : 2;
    return getCaps()["adcRate"].toDouble() / dec * on * bytes;
}

auto CBoard::expectedBytes() -> double {
    auto s = settings();
    if (!s || value("adc_pass_mode") == "FILE")
        return 0;
    const uint32_t dec = std::max<uint32_t>(1, s->getADCDecimation());
    const double perChannel = getCaps()["adcRate"].toDouble() / dec;
    const auto l = s->getADCLimits();
    // a size is what it says; a time or samples: their data, a few per cent for the headers of
    // the blocks and the format
    if (l.size)
        return double(l.size);
    if (l.time > 0)
        return l.time * dataRate() * 1.02;
    const uint64_t limit = s->getADCSamples();
    if (!limit)
        return 0;
    return perChannel > 0 ? double(limit) * dataRate() / perChannel * 1.02 : 0;
}

namespace {

auto durText(double s) -> QString {
    if (!(s >= 0) || !std::isfinite(s))
        return "-";
    if (s < 90)
        return QString::number(std::lround(s)) + " s";
    if (s < 5400)
        return QString::number(std::lround(s / 60)) + " min";
    return QString::number(std::floor(s / 3600)) + " h " + QString::number(std::lround(std::fmod(s, 3600) / 60)) + " min";
}

}  // namespace

auto CBoard::fillLimit(QVariantMap& r, uint64_t samples, uint64_t written, double limitSamples, double limitTime, double limitSize, double splitSize, double splitTime,
                       int part, uint64_t partBytes) -> void {
    auto s = settings();
    const uint32_t dec = s ? std::max<uint32_t>(1, s->getADCDecimation()) : 1;
    const double perCh = r.value("rate").toDouble() > 0 ? r.value("rate").toDouble() : getCaps()["adcRate"].toDouble() / dec;
    const double data = dataRate();
    QString kind = "none";
    double frac = 0, left = 0, need = 0;
    QString done;
    if (limitTime > 0) {
        kind = "time";
        frac = perCh > 0 ? double(samples) / perCh / limitTime : 0;
        left = limitTime - (perCh > 0 ? double(samples) / perCh : 0);
        need = limitTime * data;
        done = durText(perCh > 0 ? double(samples) / perCh : 0) + " of " + durText(limitTime);
    } else if (limitSize > 0) {
        kind = "size";
        frac = double(written) / limitSize;
        left = data > 0 ? (limitSize - double(written)) / data : 0;
        need = limitSize;
        done = bytesText(double(written)) + " of " + bytesText(limitSize);
    } else if (limitSamples > 0 && limitSamples < 1.8e19) {
        kind = "samples";
        frac = double(samples) / limitSamples;
        left = perCh > 0 ? (limitSamples - double(samples)) / perCh : 0;
        need = perCh > 0 ? limitSamples * data / perCh : 0;
        done = QString::number(samples) + " of " + QString::number(uint64_t(limitSamples)) + " samples";
    }
    r["limitKind"] = kind;
    r["frac"] = std::clamp(frac, 0.0, 1.0);
    r["left"] = std::max(0.0, left);
    r["doneText"] = done;
    r["need"] = need;
    r["part"] = part;
    r["partBytes"] = double(partBytes);
    r["splitSize"] = splitSize;
    r["splitTime"] = splitTime;
    r["parts"] = kind == "none" || (splitSize <= 0 && splitTime <= 0) ? 0 : std::max(1.0, std::ceil(splitSize > 0 ? need / splitSize : (data > 0 ? need / data / splitTime : 0)));
}

// ---- the DAC (streaming_improvements 4.17): as rpsa_client -o (dac_streaming.cpp) does it

namespace {

// CStreamSettings::DataFormat of a file by its extension, -1 for none the DAC plays
auto dacFormatOf(const QString& path) -> int {
    const QString e = QFileInfo(path).suffix().toLower();
    return e == "wav" ? int(CStreamSettings::DataFormat::WAV) : e == "tdms" ? int(CStreamSettings::DataFormat::TDMS) : e == "bin" ? int(CStreamSettings::DataFormat::BIN) : -1;
}

auto dataFormat(int format) -> CStreamSettings::DataFormat {
    return CStreamSettings::DataFormat(static_cast<CStreamSettings::DataFormat::_enumerated>(format));
}

auto openText(CReaderController::OpenResult r) -> QString {
    switch (r) {
        case CReaderController::OR_MISSING_CHANNELS:
            return "no channel the DAC plays (TDMS: ch1, ch2 of a group; BIN: channels 1, 2)";
        case CReaderController::OR_WRONG_DATA_TYPE:
            return "samples of a type the DAC does not take (8 or 16 bit integers)";
        case CReaderController::OR_DATA_NOT_EQUAL:
            return "the channels are not of the same length";
        case CReaderController::OR_CLOSE:
            return "it cannot be opened";
        default:
            return "";
    }
}

// What the DAC gets of a file: runs on a thread of its own (a big BIN or TDMS file takes a while)
auto dacFileInfo(const QString& path) -> QVariantMap {
    QVariantMap m;
    m["path"] = path;
    m["ok"] = false;
    if (path.isEmpty())
        return m;
    const QFileInfo fi(path);
    if (!fi.isFile()) {
        m["error"] = "No such file";
        return m;
    }
    m["size"] = double(fi.size());
    const int format = dacFormatOf(path);
    if (format < 0) {
        m["error"] = "Not a WAV, TDMS or BIN file";
        return m;
    }
    m["format"] = fi.suffix().toUpper();
    try {
        CReaderController reader(dataFormat(format), path.toStdString(), CStreamSettings::DACRepeat::DAC_REP_OFF, 0, 4096, false);
        const auto open = reader.isOpen();
        if (open != CReaderController::OR_OK) {
            m["error"] = "The DAC cannot play it: " + openText(open);
            return m;
        }
        dac_channels_t ch;
        reader.getChannels(ch);
        size_t size1 = 0, size2 = 0;
        reader.getChannelsSize(&size1, &size2);
        // the bits of a sample come with the first block
        std::vector<uint8_t> b1(4096), b2(4096);
        CReaderController::Data data;
        reader.readBlock({b1.data(), b2.data()}, data);
        const int bits = std::max(data.bits[0], data.bits[1]);
        m["channels"] = (ch[size_t(0)] ? 1 : 0) | (ch[size_t(1)] ? 2 : 0);
        m["bits"] = bits;
        m["bytes"] = double(size1 + size2);
        m["samples"] = bits > 0 ? double(std::max(size1, size2) / (bits / 8)) : 0.0;
        if (format == int(CStreamSettings::DataFormat::WAV)) {
            CWaveReader wav;
            if (wav.openFile(path.toStdString()))
                m["fileRate"] = double(wav.sampleRate());
        }
        m["ok"] = ch.count() > 0 && size1 + size2 > 0;
        if (!m["ok"].toBool())
            m["error"] = "The file has no samples";
    } catch (std::exception& e) {
        m["error"] = QString("The file cannot be read: ") + e.what();
    } catch (...) {
        m["error"] = "The file cannot be read";
    }
    return m;
}

}  // namespace

auto CBoard::readDacFile() -> void {
    if (m_dacFileBusy) {
        m_dacFileAgain = true;
        return;
    }
    if (m_dacFileThread.joinable())
        m_dacFileThread.join();
    const QString path = pcValue("dacFile").toString().trimmed();
    m_dacFileBusy = true;
    QVariantMap busy;
    busy["path"] = path;
    busy["reading"] = !path.isEmpty();
    m_dacFile = busy;
    Q_EMIT dacFileChanged();
    m_dacFileThread = std::thread([this, path]() {
        const QVariantMap info = dacFileInfo(path);
        post([this, info]() {
            m_dacFileBusy = false;
            m_dacFile = info;
            Q_EMIT dacFileChanged();
            if (m_dacFileAgain) {
                m_dacFileAgain = false;
                readDacFile();
            }
        });
    });
}

auto CBoard::getDacFile() -> QVariantMap {
    return m_dacFile;
}

auto CBoard::isDacRunning() -> bool {
    return m_dacState == "connecting" || m_dacState == "playing" || m_dacState == "stopping";
}

auto CBoard::getDacState() -> QVariantMap {
    QVariantMap m;
    m["state"] = m_dacState;
    m["source"] = m_dacSource;
    m["file"] = m_dacSource == "sd" ? m_dacPath : QFileInfo(m_dacPath).fileName();
    m["path"] = m_dacPath;
    m["text"] = m_dacText;
    m["done"] = m_dacDone;
    const double sent = double(m_dacSent.load());
    const double secs = m_dacStart ? double((m_dacEnd ? m_dacEnd : QDateTime::currentMSecsSinceEpoch()) - m_dacStart) / 1000.0 : 0.0;
    m["sent"] = sent;
    m["seconds"] = secs;
    m["rate"] = secs > 0 && m_dacSource == "pc" ? sent / secs : 0.0;
    const int passes = m_dacRun.repeat == 2 ? 0 : m_dacRun.repeat == 1 ? m_dacRun.times : 1;
    m["passes"] = passes;
    // the data are whole blocks: the pass is close, not exact
    const double fileBytes = double(m_dacRun.fileBytes);
    double pass = fileBytes > 0 ? std::floor(sent / fileBytes) + 1 : 1;
    if (passes > 0)
        pass = std::min(pass, double(passes));
    m["pass"] = pass;
    m["onePack"] = m_dacRun.fileBytes > 0 && m_dacRun.blockSize >= m_dacRun.fileBytes;
    return m;
}

auto CBoard::setDacState(const QString& state, const QString& text) -> void {
    m_dacState = state;
    if (!text.isNull())
        m_dacText = text;
    Q_EMIT dacChanged();
}

auto CBoard::dacSummary() -> QString {
    if (m_dacSource == "sd") {
        const QString rep = value("repeat");
        return m_dacPath + " (" + value("file_type_sd") + ") of the SD card of the board, " +
               (rep == "DAC_REP_INF" ? QString("until the stop") : rep == "DAC_REP_ON" ? value("repeatCount") + " times" : QString("once")) + ", " +
               rateText(value("dac_rate").toDouble());
    }
    return QDir::toNativeSeparators(m_dacPath) + ", " +
           (m_dacRun.repeat == 2 ? QString("until the stop") : m_dacRun.repeat == 1 ? QString::number(m_dacRun.times) + " times" : QString("once")) + ", " +
           rateText(value("dac_rate").toDouble()) + (m_dacRun.gaps ? ", the gaps as 0" : "");
}

auto CBoard::connectDacHandlers() -> void {
    auto* cm = m_configManager;
    // the board waits for the data of this computer
    cm->serverDacStartedNofiy.connect([this](std::string&) {
        post([this]() {
            if (m_dacState != "connecting")
                return;
            if (m_dacSource == "sd") {
                m_configManager->sendDACServerStop(m_ip.toStdString());
                dacStopped("the board waits for the data of a client (dac_pass_mode is DAC_NET now): start it again", true);
                return;
            }
            startDacSender();
        });
    });
    // the board plays a file of its own SD card
    cm->serverDacStartedSDNofiy.connect([this](std::string&) {
        post([this]() {
            if (m_dacState != "connecting")
                return;
            if (m_dacSource == "pc") {
                m_configManager->sendDACServerStop(m_ip.toStdString());
                dacStopped("the board plays a file of its own SD card (dac_pass_mode is DAC_FILE now), not the file of this computer: start it again", true);
                return;
            }
            setDacState("playing");
            addLog("DAC plays " + m_dacPath + " of the SD card of the board");
            dismissError();  // a start that goes
        });
    });
    auto stopped = [this](const char* why, bool error) {
        return [this, why, error](std::string&) { post([this, why, error]() { dacStopped(why, error); }); };
    };
    cm->serverDacStoppedNofiy.connect(stopped("", false));
    cm->serverDacStoppedSDDoneNofiy.connect(stopped("the file is played", false));
    cm->serverDacStoppedSDEmptyNofiy.connect(stopped("the file on the SD card is empty", true));
    cm->serverDacStoppedSDBrokenNofiy.connect(stopped("the file on the SD card is broken or not of its type (file_type_sd)", true));
    cm->serverDacStoppedSDMissingNofiy.connect(stopped("the file is not on the SD card (/home/redpitaya/streaming_files/dac)", true));
    cm->serverDacStoppedMemErrorNofiy.connect(stopped("not enough DMA memory for the DAC (Memory tab) or no channel to play", true));
    cm->serverDacStoppedMemModifyNofiy.connect(stopped("the memory settings changed", true));
    cm->serverDacStoppedConfigErrorNofiy.connect(
        stopped("the board did not start the DAC: the settings or the board (see its log: journalctl -u redpitaya_streaming)", true));
}

void CBoard::startDac() {
    if (isDacRunning())
        return;
    joinDacSender();
    if (!m_connected) {
        addLog("Not connected to the board", Log::Error);
        return;
    }
    const bool sd = value("dac_pass_mode") == "DAC_FILE";
    m_dacSource = sd ? "sd" : "pc";
    m_dacStopRequested = false;
    m_dacDone = false;
    m_dacSent = 0;
    m_dacConnected = false;
    m_dacTerminate = false;
    m_dacStart = 0;
    m_dacEnd = 0;
    m_dacRun = SDacRun();
    m_dacGen++;
    auto fail = [this](const QString& why) {
        setDacState("error", why);
        setError("DAC not started: " + why);
    };
    dac_channels_t channels;
    if (sd) {
        m_dacPath = value("file_sd");
        if (m_dacPath.isEmpty())
            return fail("no file of the SD card is chosen (DAC tab)");
    } else {
        SDacRun run;
        run.path = pcValue("dacFile").toString().trimmed();
        m_dacPath = run.path;
        if (run.path.isEmpty())
            return fail("no file of this computer is chosen (DAC tab)");
        run.format = dacFormatOf(run.path);
        if (run.format < 0)
            return fail(QDir::toNativeSeparators(run.path) + " is not a WAV, TDMS or BIN file");
        if (!QFileInfo(run.path).isFile())
            return fail("no file " + QDir::toNativeSeparators(run.path));
        // the channels of the file: from its info when it is the one read, else read now
        QVariantMap info = m_dacFile;
        if (info.value("path").toString() != run.path || !info.contains("ok"))
            info = dacFileInfo(run.path);
        if (!info.value("ok").toBool())
            return fail(QDir::toNativeSeparators(run.path) + ": " + info.value("error").toString());
        const int mask = info.value("channels").toInt();
        if (mask & 1)
            channels.enable(size_t(0));
        if (mask & 2)
            channels.enable(size_t(1));
        const QString rep = pcValue("dacRepeat").toString();
        run.repeat = rep == "inf" ? 2 : rep == "times" ? 1 : 0;
        run.times = std::max(1, pcValue("dacTimes").toInt());
        run.gaps = value("dac_play_gaps") == "ON";
        run.blockSize = value("block_size").toUInt();
        run.fileBytes = static_cast<uint64_t>(info.value("bytes").toDouble());
        if (!run.blockSize)
            return fail("the block size of the board is not known: read the settings again");
        m_dacRun = run;
    }
    m_dacStart = QDateTime::currentMSecsSinceEpoch();
    setDacState("connecting", "");
    addLog("Starting the DAC: " + dacSummary());
    if (!m_configManager->sendDACServerStart(m_ip.toStdString(), channels)) {
        setDacState("error", "the start is not sent: no connection to the board");
        return;
    }
    // an older server or a board that does not answer
    const int gen = m_dacGen;
    QTimer::singleShot(10000, this, [this, gen]() {
        if (gen == m_dacGen && m_dacState == "connecting" && !m_dacThread.joinable()) {
            m_configManager->sendDACServerStop(m_ip.toStdString());
            dacStopped("the board did not answer the start in 10 s", true);
        }
    });
}

void CBoard::stopDac() {
    if (!isDacRunning() || m_dacState == "stopping")
        return;
    m_dacStopRequested = true;
    m_dacTerminate = true;
    setDacState("stopping");
    addLog("Stopping the DAC");
    if (!m_configManager->sendDACServerStop(m_ip.toStdString())) {
        dacStopped("", false);
        return;
    }
    const int gen = m_dacGen;
    QTimer::singleShot(5000, this, [this, gen]() {
        if (gen == m_dacGen && m_dacState == "stopping")
            dacStopped("the board did not confirm the stop", false);
    });
}

// the board stopped the DAC (or the client gave up on it)
auto CBoard::dacStopped(const QString& why, bool error) -> void {
    if (!isDacRunning())
        return;
    const bool requested = m_dacStopRequested;
    const bool sending = m_dacThread.joinable() && !m_dacDone;
    m_dacTerminate = true;
    m_dacEnd = QDateTime::currentMSecsSinceEpoch();
    m_dacGen++;
    QString state = "stopped";
    QString text = why;
    if (error) {
        state = "error";
    } else if (!requested && sending && m_dacState == "playing") {
        // the board ended it while the file was still being sent
        state = "board";
        if (text.isEmpty())
            text = "the board stopped the DAC";
    } else if (text.isEmpty() && m_dacDone) {
        text = "the file is played";
    }
    setDacState(state, text);
    const QString sent = m_dacSource == "pc" ? " (" + bytesText(double(m_dacSent.load())) + " sent)" : QString();
    if (error)
        setError("DAC stopped: " + text + sent);
    else
        addLog("DAC stopped" + (text.isEmpty() ? QString() : ": " + text) + sent, state == "board" ? Log::Warning : Log::Normal);
}

auto CBoard::startDacSender() -> void {
    joinDacSender();
    m_dacTerminate = false;
    m_dacConnected = false;
    m_dacSent = 0;
    m_dacStart = QDateTime::currentMSecsSinceEpoch();
    addLog("The board waits for the data on port " + QString::number(NET_DAC_STREAMING_PORT), Log::Detail);
    m_dacThread = std::thread(&CBoard::dacSender, this, m_dacRun);
}

auto CBoard::joinDacSender() -> void {
    if (!m_dacThread.joinable())
        return;
    m_dacTerminate = true;
    m_dacThread.join();
}

// The sender thread: reads the file block by block and sends it on the data connection of the
// DAC; the board takes as fast as it plays (TCP holds the rest back)
auto CBoard::dacSender(SDacRun run) -> void {
    using dac_streaming_lib::CDACStreamingManager;
    DacEnd end = DacEnd::Done;
    QString text;
    std::atomic<bool> fileEnded{false};
    std::atomic<bool> lost{false};
    std::atomic<int> fileError{-1};
    try {
        auto memory = std::make_unique<uio_lib::CMemoryManager>();
        memory->setMemoryBlockSize(run.blockSize);
        memory->reallocateBlocks();
        const auto blocks = memory->getFreeBlockCount();
        const auto type = run.format == int(CStreamSettings::DataFormat::TDMS)  ? CDACStreamingManager::TDMS_TYPE
                          : run.format == int(CStreamSettings::DataFormat::BIN) ? CDACStreamingManager::BIN_TYPE
                                                                                 : CDACStreamingManager::WAV_TYPE;
        const auto repeat = run.repeat == 2   ? CStreamSettings::DACRepeat::DAC_REP_INF
                            : run.repeat == 1 ? CStreamSettings::DACRepeat::DAC_REP_ON
                                              : CStreamSettings::DACRepeat::DAC_REP_OFF;
        auto manager = CDACStreamingManager::Create(type, run.path.toStdString(), repeat, run.repeat == 1 ? run.times : 0, run.blockSize, false, run.gaps);
        manager->setRemoteClientMode(true);
        dac_channels_t channels;
        manager->getChannels(channels);
        memory->reserveMemory(uio_lib::MM_DAC, blocks, channels.count());
        manager->getBufferManager()->generateBuffersEmptyDAC(channels, memory->getRegions(uio_lib::MM_DAC), DataLib::sizeHeader());
        manager->getBufferManager()->initHeadersDAC(channels);
        manager->notifyStop.connect([&](CDACStreamingManager::NotifyResult r) {
            if (r == CDACStreamingManager::NR_ENDED)
                fileEnded = true;
            else if (r == CDACStreamingManager::NR_BROKEN || r == CDACStreamingManager::NR_EMPTY || r == CDACStreamingManager::NR_MISSING_FILE)
                fileError = int(r);
        });

        auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_ip.toStdString(), NET_DAC_STREAMING_PORT, nullptr);
        net->clientConnectNotify.connect([this](std::string&) { m_dacConnected = true; });
        net->clientDisconnectNotify.connect([this](std::string&) { m_dacConnected = false; });
        net->sendNotify.connect([&](std::error_code err, size_t) {
            if (err)
                lost = true;
        });
        {
            std::lock_guard lock(m_dacNetMtx);
            m_dacNet = net;
        }
        net->start();
        const qint64 until = QDateTime::currentMSecsSinceEpoch() + 5000;
        while (!m_dacConnected && !m_dacTerminate && QDateTime::currentMSecsSinceEpoch() < until)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (!m_dacConnected) {
            end = m_dacTerminate ? DacEnd::Stopped : DacEnd::NoConnection;
        } else {
            post([this]() {
                if (m_dacState != "connecting")
                    return;
                setDacState("playing");
                addLog("DAC playing: the data connection is open");
                dismissError();  // a start that goes
            });
            manager->run();
            while (true) {
                auto pack = manager->getBuffer();
                if (pack) {
                    uint64_t bytes = 0;
                    for (auto c : {DataLib::CH1, DataLib::CH2})
                        if (auto ch = pack->getBuffer(c))
                            bytes += ch->getDataLenght();
                    if (!net->sendSyncData(pack))
                        lost = true;
                    else
                        m_dacSent += bytes;
                    manager->unlockBuffer();
                }
                if (m_dacTerminate) {
                    end = DacEnd::Stopped;
                    break;
                }
                if (lost) {
                    end = DacEnd::Lost;
                    break;
                }
                if (fileError >= 0) {
                    end = DacEnd::FileError;
                    text = fileError == CDACStreamingManager::NR_EMPTY ? "the file is empty" : fileError == CDACStreamingManager::NR_MISSING_FILE ? "the file is gone" : "the file is broken";
                    break;
                }
                if (fileEnded && manager->isEmptyBuffer())
                    break;
            }
        }
        manager->stop();
        {
            std::lock_guard lock(m_dacNetMtx);
            m_dacNet = nullptr;
        }
        // closes the data connection: the board plays what it got and then stops
        net->stop();
        net = nullptr;
        manager = nullptr;
    } catch (std::exception& e) {
        end = DacEnd::FileError;
        text = e.what();
    }
    post([this, end, text]() { dacSenderEnded(end, text); });
}

auto CBoard::dacSenderEnded(DacEnd end, const QString& text) -> void {
    joinDacSender();
    if (!isDacRunning())
        return;
    const QString sent = bytesText(double(m_dacSent.load()));
    switch (end) {
        case DacEnd::Done:
            m_dacDone = true;
            addLog("DAC: the whole file is sent (" + sent + "), the board plays the rest");
            Q_EMIT dacChanged();
            break;
        case DacEnd::Stopped:
            break;
        case DacEnd::NoConnection:
            m_configManager->sendDACServerStop(m_ip.toStdString());
            dacStopped("no data connection to the board on port " + QString::number(NET_DAC_STREAMING_PORT) + " in 5 s", true);
            break;
        case DacEnd::FileError:
            m_configManager->sendDACServerStop(m_ip.toStdString());
            dacStopped("the file cannot be read: " + text, true);
            break;
        case DacEnd::Lost: {
            // the board closed it: its stop says why, else the connection broke
            const int gen = m_dacGen;
            QTimer::singleShot(2000, this, [this, gen]() {
                if (gen == m_dacGen && isDacRunning())
                    dacStopped("the data connection broke", true);
            });
            break;
        }
    }
}

// ---- the GPIO (streaming_improvements 4.17): as rpsa_client -G (gpio_client.cpp) does it

namespace {

// The words of the GPIO output a file gives: what rpsa_client -G --out takes. Runs on a thread
auto gpioFileInfo(const QString& path) -> QVariantMap {
    QVariantMap m;
    m["path"] = path;
    m["ok"] = false;
    if (path.isEmpty())
        return m;
    const QFileInfo fi(path);
    if (!fi.isFile()) {
        m["error"] = "No such file";
        return m;
    }
    m["size"] = double(fi.size());
    try {
        bin_lib::FileReader reader;
        if (!reader.open(path.toStdString())) {
            m["error"] = "Not a BIN stream file: " + QString::fromStdString(reader.error());
            return m;
        }
        const auto& info = reader.info();
        if (info.channels.size() != 1 ||
            (info.channels[0].type != bin_lib::SampleType::GPIO_IN_WORD && info.channels[0].type != bin_lib::SampleType::GPIO_OUT_WORD)) {
            m["error"] = "Not a recording of the GPIO input or GPIO output words";
            return m;
        }
        const bool in = info.channels[0].type == bin_lib::SampleType::GPIO_IN_WORD;
        const bool rle = (info.channels[0].flags & bin_lib::CHANNEL_FLAG_RLE) != 0;
        uint64_t words = 0, samples = 0;
        bin_lib::BlockInfo b;
        std::vector<uint8_t> data;
        while (reader.nextBlock(b, data)) {
            const size_t n = data.size() / 4;
            words += n;
            samples += in ? b.samples : bin_lib::gpioOutSamples(reinterpret_cast<const uint32_t*>(data.data()), n, rle);
        }
        m["kind"] = in ? "in" : "out";
        m["rle"] = rle;
        m["words"] = double(words);
        m["samples"] = double(samples);
        m["clock"] = double(info.clock);
        m["decimation"] = double(info.decimation);
        m["ok"] = words > 0;
        if (!words)
            m["error"] = "The file holds no GPIO words";
    } catch (std::exception& e) {
        m["error"] = QString("The file cannot be read: ") + e.what();
    }
    return m;
}

auto endIsError(int end) -> bool {
    return end >= 5;  // FileError, DiskFull, NoConnection, Lost
}

}  // namespace

auto CBoard::gpioClock() -> double {
    // the GPIO block runs at the DAC rate on the 65-16 (twice its ADC rate), else at the ADC rate
    const double adc = getCaps()["adcRate"].toDouble();
    return adc == 62500000 ? 125000000 : adc;
}

auto CBoard::readGpioFile() -> void {
    if (m_gpioFileBusy) {
        m_gpioFileAgain = true;
        return;
    }
    if (m_gpioFileThread.joinable())
        m_gpioFileThread.join();
    const QString path = pcValue("gpioOutFile").toString().trimmed();
    m_gpioFileBusy = true;
    QVariantMap busy;
    busy["path"] = path;
    busy["reading"] = !path.isEmpty();
    m_gpioFile = busy;
    Q_EMIT gpioFileChanged();
    m_gpioFileThread = std::thread([this, path]() {
        const QVariantMap info = gpioFileInfo(path);
        post([this, info]() {
            m_gpioFileBusy = false;
            m_gpioFile = info;
            Q_EMIT gpioFileChanged();
            if (m_gpioFileAgain) {
                m_gpioFileAgain = false;
                readGpioFile();
            }
        });
    });
}

auto CBoard::getGpioFile() -> QVariantMap {
    return m_gpioFile;
}

auto CBoard::gpioRunning(const SGpioDir& d) -> bool {
    return d.state == "connecting" || d.state == "recording" || d.state == "playing" || d.state == "sd" || d.state == "stopping";
}

auto CBoard::isGpioInRunning() -> bool {
    return gpioRunning(m_gin);
}

auto CBoard::isGpioOutRunning() -> bool {
    return gpioRunning(m_gout);
}

auto CBoard::setGpioState(SGpioDir& d, const QString& state, const QString& text) -> void {
    d.state = state;
    if (!text.isNull())
        d.text = text;
    Q_EMIT gpioChanged();
}

auto CBoard::getGpioState() -> QVariantMap {
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    auto common = [now](const SGpioDir& d, QVariantMap& m) {
        m["state"] = d.state;
        m["source"] = d.source;
        m["path"] = d.path;
        m["file"] = d.source == "sd" ? d.path : QFileInfo(d.path).fileName();
        m["text"] = d.text;
        m["seconds"] = d.start ? double((d.stop ? d.stop : now) - d.start) / 1000.0 : 0.0;
    };
    SGpioIn in;
    SGpioOut out;
    {
        std::lock_guard lock(m_gpioMtx);
        in = m_ginStat;
        out = m_goutStat;
    }
    QVariantMap i;
    common(m_gin, i);
    i["armed"] = m_gpioInArmed;
    i["triggers"] = double(m_gpioInTriggers);
    i["rate"] = m_ginRate;
    if (m_gin.source == "sd") {
        const auto& r = m_sdGpioIn;
        i["known"] = r.mode == StreamRecordStatus::SD;
        i["samples"] = double(r.samples);
        i["written"] = double(r.written);
        i["bytes"] = double(r.written);
        i["lost"] = double(r.lostBoard + r.lostQueue);
        i["part"] = int(r.part);
        if (!r.file.empty())
            i["file"] = QString::fromStdString(r.file);
        const double limitSamples = r.limit ? double(r.limit) : r.limitTime > 0 ? r.limitTime * m_ginRate : 0;
        i["limitKind"] = r.limit ? "samples" : r.limitTime > 0 ? "time" : r.limitSize ? "size" : "none";
        i["frac"] = limitSamples > 0 ? std::min(1.0, double(r.samples) / limitSamples) : r.limitSize ? std::min(1.0, double(r.written) / double(r.limitSize)) : 0.0;
    } else {
        i["packs"] = double(in.packs);
        i["samples"] = double(in.samples);
        i["words"] = double(in.words);
        i["bytes"] = double(in.bytes);
        i["lost"] = double(in.lost);
        i["waited"] = double(in.waited);
        i["rle"] = in.rle;
        i["part"] = in.part;
        if (!in.file.isEmpty())
            i["file"] = QFileInfo(in.file).fileName();
        const double limitSamples = m_ginLimitSamples ? double(m_ginLimitSamples) : m_ginLimitTime * m_ginRate;
        i["limitKind"] = m_ginLimitSamples ? "samples" : m_ginLimitTime > 0 ? "time" : "none";
        i["limit"] = m_ginLimitSamples ? double(m_ginLimitSamples) : m_ginLimitTime;
        i["frac"] = limitSamples > 0 ? std::min(1.0, double(in.samples + in.lost + in.waited) / limitSamples) : 0.0;
    }
    QVariantMap o;
    common(m_gout, o);
    o["armed"] = m_gpioOutArmed;
    o["words"] = double(out.words);
    o["bytes"] = double(out.bytes);
    const double secs = o["seconds"].toDouble();
    o["rate"] = secs > 0 && m_gout.source == "pc" ? double(out.bytes) / secs : 0.0;
    o["whole"] = m_goutWhole;
    o["waited"] = double(gpioRunning(m_gout) ? m_gpioOutWait : m_goutWaited);
    o["passes"] = m_goutPasses;
    QVariantMap m;
    m["in"] = i;
    m["out"] = o;
    return m;
}

auto CBoard::gpioInSummary() -> QString {
    const QString rate = rateText(m_ginRate) + (value("gpio_rle") == "ON" ? ", RLE" : "");
    if (m_gin.source == "sd") {
        const double t = value("gpio_in_limit_time_sd").toDouble();
        const QString n = value("gpio_in_samples_limit_sd");
        return value("gpio_in_format_sd") + " to the SD card of the board, " + rate +
               (t > 0 ? ", " + durText(t) : n.toDouble() > 0 ? ", " + n + " samples" : QString(", until the stop"));
    }
    return QDir::toNativeSeparators(m_gin.path) + ", " + rate +
           (m_ginLimitSamples ? ", " + QString::number(m_ginLimitSamples) + " samples" : m_ginLimitTime > 0 ? ", " + durText(m_ginLimitTime) : QString(", until the stop"));
}

auto CBoard::gpioOutSummary() -> QString {
    const QString step = ", step " + value("gpio_out_step") + (value("gpio_rle") == "ON" ? ", RLE" : "");
    if (m_gout.source == "sd") {
        const QString rep = value("gpio_out_repeat");
        return m_gout.path + " of the SD card of the board, " +
               (rep == "GPIO_REP_INF" ? QString("until the stop") : rep == "GPIO_REP_ON" ? value("gpio_out_repeat_count") + " times" : QString("once")) + step;
    }
    return QDir::toNativeSeparators(m_gout.path) + ", " +
           (m_goutPasses == 0 ? QString("until the stop") : m_goutPasses > 1 ? QString::number(m_goutPasses) + " times" : QString("once")) + step;
}

auto CBoard::connectGpioHandlers() -> void {
    auto* cm = m_configManager;
    // the board is ready for the data connection of this computer
    cm->serverGpioInStartedNofiy.connect([this](std::string&) {
        post([this]() {
            auto& d = m_gin;
            if (d.state != "connecting")
                return;
            if (d.source == "sd") {
                gpioStopSent(false);
                gpioBoardStopped(false, "the board waits for a client (gpio_in_pass_mode is NET now): start it again", true);
                return;
            }
            d.running = true;
            d.thread = std::thread(&CBoard::gpioInRun, this, d.path, m_ginFormat, m_gpioBlock, m_ginLimitSamples, m_ginLimitTime);
            setGpioState(d, "recording");
            addLog("GPIO input: the board sends, recording to " + QDir::toNativeSeparators(d.path));
            dismissError();  // a start that goes
        });
    });
    cm->serverGpioInStartedSDNofiy.connect([this](std::string&) {
        post([this]() {
            auto& d = m_gin;
            if (d.state != "connecting")
                return;
            if (d.source == "pc") {
                gpioStopSent(false);
                gpioBoardStopped(false, "the board writes the input to its own SD card (gpio_in_pass_mode is FILE now): start it again", true);
                return;
            }
            setGpioState(d, "sd");
            addLog("GPIO input: the board writes its SD card (/home/redpitaya/streaming_files/gpio)");
            dismissError();
        });
    });
    cm->serverGpioOutStartedNofiy.connect([this](std::string&) {
        post([this]() {
            auto& d = m_gout;
            if (d.state != "connecting")
                return;
            if (d.source == "sd") {
                gpioStopSent(true);
                gpioBoardStopped(true, "the board waits for the data of a client (gpio_out_pass_mode is NET now): start it again", true);
                return;
            }
            d.running = true;
            d.thread = std::thread(&CBoard::gpioOutRun, this, m_goutReader, m_gpioBlock);
            m_goutReader = nullptr;
            addLog("GPIO output: the board waits for the data on port " + QString::number(NET_GPIO_OUT_STREAMING_PORT), Log::Detail);
        });
    });
    cm->serverGpioOutStartedSDNofiy.connect([this](std::string&) {
        post([this]() {
            auto& d = m_gout;
            if (d.state != "connecting")
                return;
            if (d.source == "pc") {
                gpioStopSent(true);
                gpioBoardStopped(true, "the board plays a file of its own SD card (gpio_out_pass_mode is FILE now), not the file of this computer: start it again", true);
                return;
            }
            setGpioState(d, "playing");
            addLog("GPIO output plays " + d.path + " of the SD card of the board");
            dismissError();
        });
    });
    auto in = [this](const char* why, bool error) {
        return [this, why, error](std::string&) { post([this, why, error]() { gpioBoardStopped(false, why, error); }); };
    };
    auto out = [this](const char* why, bool error) {
        return [this, why, error](std::string&) { post([this, why, error]() { gpioBoardStopped(true, why, error); }); };
    };
    const char* const config = "the board did not start it: the settings, no GPIO block in the FPGA or the data port taken (see its log: journalctl -u redpitaya_streaming)";
    cm->serverGpioInStoppedNofiy.connect(in("", false));
    cm->serverGpioInStoppedMemErrorNofiy.connect(in("not enough DMA memory for the GPIO input (Memory tab)", true));
    cm->serverGpioInStoppedConfigErrorNofiy.connect(in(config, true));
    cm->serverGpioInStoppedSDFullNofiy.connect(in("the SD card is full", true));
    cm->serverGpioInStoppedSDDoneNofiy.connect(in("the limit is written to the SD card", false));
    cm->serverGpioInStoppedSDBrokenNofiy.connect(in("the file on the SD card could not be written", true));
    cm->serverGpioOutStoppedNofiy.connect([this](std::string&, uint64_t wait) {
        post([this, wait]() {
            m_goutWaited = wait;
            gpioBoardStopped(true, "", false);
        });
    });
    cm->serverGpioOutStoppedSDDoneNofiy.connect([this](std::string&, uint64_t wait) {
        post([this, wait]() {
            m_goutWaited = wait;
            gpioBoardStopped(true, "the file is played", false);
        });
    });
    cm->serverGpioOutStoppedMemErrorNofiy.connect(out("not enough DMA memory for the GPIO output (Memory tab)", true));
    cm->serverGpioOutStoppedConfigErrorNofiy.connect(out(config, true));
    cm->serverGpioOutStoppedSDEmptyNofiy.connect(out("the file on the SD card holds no GPIO words", true));
    cm->serverGpioOutStoppedSDBrokenNofiy.connect(out("the file on the SD card is broken or not a GPIO file", true));
    cm->serverGpioOutStoppedSDMissingNofiy.connect(out("the file is not on the SD card (/home/redpitaya/streaming_files/gpio)", true));
}

void CBoard::startGpioIn() {
    auto& d = m_gin;
    if (gpioRunning(d))
        return;
    if (d.thread.joinable())
        d.thread.join();
    if (!m_connected) {
        addLog("Not connected to the board", Log::Error);
        return;
    }
    const bool sd = value("gpio_in_pass_mode") == "FILE";
    d.source = sd ? "sd" : "pc";
    d.stopRequested = d.stopSent = d.boardDone = d.boardError = false;
    d.boardText.clear();
    d.end = GpioEnd::None;
    d.endText.clear();
    d.terminate = false;
    d.start = d.stop = 0;
    d.gen++;
    {
        std::lock_guard lock(m_gpioMtx);
        m_ginStat = SGpioIn();
    }
    m_sdGpioIn = StreamRecordStatus();
    m_gpioInArmed = false;
    m_gpioInTriggers = 0;
    m_ginLimitSamples = 0;
    m_ginLimitTime = 0;
    m_ginRate = gpioClock() / std::max(1.0, value("gpio_decimation").toDouble());
    auto fail = [this, &d](const QString& why) {
        setGpioState(d, "error", why);
        setError("GPIO input not started: " + why);
    };
    if (sd) {
        d.path = "/home/redpitaya/streaming_files/gpio";
    } else {
        const QString format = pcValue("gpioInFormat").toString();
        m_ginFormat = format == "CSV" ? int(CStreamSettings::GPIOFormat::CSV) : format == "VCD" ? int(CStreamSettings::GPIOFormat::VCD) : int(CStreamSettings::GPIOFormat::BIN);
        const QString ext = format == "CSV" ? ".csv" : format == "VCD" ? ".vcd" : ".bin";
        const QString dir = recordDir();
        if (!QDir().mkpath(dir))
            return fail("the folder " + dir + " cannot be made (This PC tab)");
        // an existing file is never written over
        const QString base = QDir(dir).filePath(fileBaseName(QDateTime::currentDateTime()) + "_gpio");
        QString path = base + ext;
        for (int n = 2; QFileInfo::exists(path); n++)
            path = base + "_" + QString::number(n) + ext;
        d.path = path;
        const QString kind = pcValue("gpioInLimitKind").toString();
        const double limit = pcValue("gpioInLimit").toDouble();
        if (kind == "samples" && limit > 0)
            m_ginLimitSamples = static_cast<uint64_t>(limit);
        else if (kind == "time" && limit > 0)
            m_ginLimitTime = limit;
        m_gpioBlock = value("block_size").toUInt();
        if (!m_gpioBlock)
            return fail("the block size of the board is not known: read the settings again");
    }
    d.start = QDateTime::currentMSecsSinceEpoch();
    setGpioState(d, "connecting", "");
    addLog("Starting the GPIO input: " + gpioInSummary());
    if (!m_configManager->sendGPIOInServerStart(m_ip.toStdString())) {
        setGpioState(d, "error", "the start is not sent: no connection to the board");
        return;
    }
    const int gen = d.gen;
    QTimer::singleShot(10000, this, [this, gen]() {
        if (gen == m_gin.gen && m_gin.state == "connecting") {
            gpioStopSent(false);
            gpioBoardStopped(false, "the board did not answer the start in 10 s", true);
        }
    });
}

void CBoard::startGpioOut() {
    auto& d = m_gout;
    if (gpioRunning(d))
        return;
    if (d.thread.joinable())
        d.thread.join();
    if (!m_connected) {
        addLog("Not connected to the board", Log::Error);
        return;
    }
    const bool sd = value("gpio_out_pass_mode") == "FILE";
    d.source = sd ? "sd" : "pc";
    d.stopRequested = d.stopSent = d.boardDone = d.boardError = false;
    d.boardText.clear();
    d.end = GpioEnd::None;
    d.endText.clear();
    d.terminate = false;
    d.start = d.stop = 0;
    d.gen++;
    {
        std::lock_guard lock(m_gpioMtx);
        m_goutStat = SGpioOut();
    }
    m_goutWhole = false;
    m_goutWaited = 0;
    m_gpioOutWait = 0;
    m_gpioOutArmed = false;
    m_goutReader = nullptr;
    auto fail = [this, &d](const QString& why) {
        setGpioState(d, "error", why);
        setError("GPIO output not started: " + why);
    };
    if (sd) {
        d.path = value("gpio_out_file_sd");
        const QString rep = value("gpio_out_repeat");
        m_goutPasses = rep == "GPIO_REP_INF" ? 0 : rep == "GPIO_REP_ON" ? std::max(1, value("gpio_out_repeat_count").toInt()) : 1;
        if (d.path.isEmpty())
            return fail("no file of the SD card is chosen (GPIO tab)");
    } else {
        d.path = pcValue("gpioOutFile").toString().trimmed();
        if (d.path.isEmpty())
            return fail("no file of this computer is chosen (GPIO tab)");
        if (!QFileInfo(d.path).isFile())
            return fail("no file " + QDir::toNativeSeparators(d.path));
        const QString rep = pcValue("gpioOutRepeat").toString();
        const int times = std::max(1, pcValue("gpioOutTimes").toInt());
        m_goutPasses = rep == "inf" ? 0 : rep == "times" ? times : 1;
        const auto repeat = rep == "inf"     ? CStreamSettings::GPIORepeat::GPIO_REP_INF
                            : rep == "times" ? CStreamSettings::GPIORepeat::GPIO_REP_ON
                                             : CStreamSettings::GPIORepeat::GPIO_REP_OFF;
        // the words go out as the board takes them: with RLE or not, as gpio_rle
        auto reader = std::make_shared<CGpioFileReader>(d.path.toStdString(), repeat, rep == "times" ? uint32_t(times) : 0u, value("gpio_rle") == "ON",
                                                        pcValue("gpioOutGaps").toBool());
        const auto open = reader->isOpen();
        if (open != CGpioFileReader::Open::OK)
            return fail(QDir::toNativeSeparators(d.path) + ": " +
                        (open == CGpioFileReader::Open::EMPTY        ? QString("it holds no GPIO words")
                         : open == CGpioFileReader::Open::WRONG_TYPE ? QString("not a recording of the GPIO input or GPIO output words (a BIN stream file of the GPIO)")
                                                                     : QString::fromStdString(reader->error())));
        m_gpioBlock = value("block_size").toUInt();
        if (!m_gpioBlock)
            return fail("the block size of the board is not known: read the settings again");
        m_goutReader = reader;
    }
    d.start = QDateTime::currentMSecsSinceEpoch();
    setGpioState(d, "connecting", "");
    addLog("Starting the GPIO output: " + gpioOutSummary());
    if (!m_configManager->sendGPIOOutServerStart(m_ip.toStdString())) {
        m_goutReader = nullptr;
        setGpioState(d, "error", "the start is not sent: no connection to the board");
        return;
    }
    const int gen = d.gen;
    QTimer::singleShot(10000, this, [this, gen]() {
        if (gen == m_gout.gen && m_gout.state == "connecting" && !m_gout.running) {
            gpioStopSent(true);
            gpioBoardStopped(true, "the board did not answer the start in 10 s", true);
        }
    });
}

void CBoard::stopGpioIn() {
    if (!gpioRunning(m_gin) || m_gin.state == "stopping")
        return;
    m_gin.stopRequested = true;
    m_gin.terminate = true;
    addLog("Stopping the GPIO input");
    gpioStopSent(false);
}

void CBoard::stopGpioOut() {
    if (!gpioRunning(m_gout) || m_gout.state == "stopping")
        return;
    m_gout.stopRequested = true;
    m_gout.terminate = true;
    addLog("Stopping the GPIO output");
    gpioStopSent(true);
}

// the stop to the board, once; the board answers when it has stopped (a file closed first)
auto CBoard::gpioStopSent(bool out) -> void {
    auto& d = out ? m_gout : m_gin;
    if (!gpioRunning(d))
        return;
    if (d.state != "stopping")
        setGpioState(d, "stopping");
    if (d.stopSent)
        return;
    d.stopSent = true;
    const bool sent = out ? m_configManager->sendGPIOOutServerStop(m_ip.toStdString()) : m_configManager->sendGPIOInServerStop(m_ip.toStdString());
    if (!sent) {
        d.boardDone = true;
        d.terminate = true;
        if (!d.running)
            gpioFinish(out);
        return;
    }
    const int gen = d.gen;
    QTimer::singleShot(5000, this, [this, out, gen]() {
        auto& d = out ? m_gout : m_gin;
        if (gen == d.gen && d.state == "stopping") {
            if (d.boardText.isEmpty() && !d.boardDone)
                d.boardText = "the board did not confirm the stop";
            gpioFinish(out);
        }
    });
}

// the board ended a direction: its answer to the stop, a stop of its own or a refusal
auto CBoard::gpioBoardStopped(bool out, const QString& why, bool error) -> void {
    auto& d = out ? m_gout : m_gin;
    if (!gpioRunning(d))
        return;
    if (!d.boardDone || error) {
        d.boardText = why;
        d.boardError = error;
    }
    d.boardDone = true;
    if (d.running) {
        // the thread ends and then finishes it
        d.terminate = true;
        return;
    }
    gpioFinish(out);
}

// the thread of a direction ended: the limit, the file, the connection or the stop
auto CBoard::gpioThreadEnded(bool out) -> void {
    auto& d = out ? m_gout : m_gin;
    if (d.thread.joinable())
        d.thread.join();
    if (!gpioRunning(d))
        return;
    if (out && d.end == GpioEnd::Done)
        m_goutWhole = true;
    if (d.boardDone) {
        gpioFinish(out);
        return;
    }
    switch (d.end) {
        case GpioEnd::Done: {
            // the board plays what it got and then stops by itself
            uint64_t words = 0;
            {
                std::lock_guard lock(m_gpioMtx);
                words = m_goutStat.words;
            }
            addLog("GPIO output: the whole file is sent (" + QString::number(words) + " words), the board plays the rest");
            Q_EMIT gpioChanged();
            return;
        }
        case GpioEnd::Limit:
            addLog("GPIO input: the limit is written");
            break;
        case GpioEnd::TimeUp:
            addLog("GPIO input: the time limit passed with no data to end it (a trigger not come)");
            break;
        default:
            break;
    }
    gpioStopSent(out);
}

auto CBoard::gpioFinish(bool out) -> void {
    auto& d = out ? m_gout : m_gin;
    if (!gpioRunning(d))
        return;
    d.terminate = true;
    if (d.thread.joinable())
        d.thread.join();
    d.running = false;
    d.stop = QDateTime::currentMSecsSinceEpoch();
    d.gen++;
    if (out)
        m_goutReader = nullptr;
    const bool threadError = endIsError(int(d.end));
    const bool error = d.boardError || threadError;
    QString text = d.boardError ? d.boardText : threadError ? d.endText : !d.boardText.isEmpty() ? d.boardText : d.endText;
    QString state = error ? "error" : "stopped";
    if (!error && !d.stopRequested && d.end != GpioEnd::Limit && d.end != GpioEnd::TimeUp && d.end != GpioEnd::Done && text.isEmpty()) {
        // the board ended it while it ran
        state = "board";
        text = out ? "the board stopped the output" : "the board stopped the input";
    }
    if (text.isEmpty() && d.end == GpioEnd::Limit)
        text = "the limit is written";
    if (text.isEmpty() && d.end == GpioEnd::TimeUp)
        text = "the time is up";
    if (text.isEmpty() && out && m_goutWhole)
        text = "the file is played";
    (out ? m_gpioOutArmed : m_gpioInArmed) = false;
    setGpioState(d, state, text);
    QString done;
    if (out) {
        SGpioOut s;
        {
            std::lock_guard lock(m_gpioMtx);
            s = m_goutStat;
        }
        done = d.source == "sd" ? "the output waited " + QString::number(m_goutWaited) + " clocks for data"
                                : QString::number(s.words) + " words of " + QDir::toNativeSeparators(d.path) + (m_goutWhole ? " (the whole file)" : "") +
                                      "; the output waited " + QString::number(m_goutWaited) + " clocks for data";
    } else if (d.source == "pc") {
        SGpioIn s;
        {
            std::lock_guard lock(m_gpioMtx);
            s = m_ginStat;
        }
        done = QString::number(s.samples) + " samples (" + QString::number(s.words) + " words" + (s.rle ? ", RLE" : "") + "), lost " + QString::number(s.lost) +
               ", waited " + QString::number(s.waited) + "; " + (s.opened ? QDir::toNativeSeparators(s.file.isEmpty() ? d.path : s.file) : QString("no file"));
    } else if (!m_sdGpioIn.file.empty()) {
        done = QString::number(m_sdGpioIn.samples) + " samples, " + bytesText(double(m_sdGpioIn.written)) + "; " + QString::fromStdString(m_sdGpioIn.file) + " on the board";
    }
    const QString what = out ? "GPIO output" : "GPIO input";
    if (error)
        setError(what + " stopped: " + text);
    else
        addLog(what + " stopped" + (text.isEmpty() ? QString() : ": " + text), state == "board" ? Log::Warning : Log::Normal);
    if (!done.isEmpty())
        addLog(what + ": " + done);
}

// The input of the board into the file, on a thread of its own (runInput of rpsa_client -G)
auto CBoard::gpioInRun(QString path, int format, uint32_t blockSize, uint64_t samplesLimit, double timeLimit) -> void {
    GpioEnd end = GpioEnd::Stopped;
    QString text;
    try {
        uio_lib::CMemoryManager memory;
        memory.setMemoryBlockSize(blockSize);
        memory.reallocateBlocks();
        memory.reserveMemory(uio_lib::MM_GPIO_IN, memory.getFreeBlockCount(), 1);
        auto buffers = DataLib::CBuffersCached::create();
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        buffers->generateBuffersEmptyADC(one, memory.getRegions(uio_lib::MM_GPIO_IN), DataLib::sizeHeader());

        const std::string file = path.toStdString();
        CGpioFileWriter writer;
        std::mutex writerMtx;
        std::atomic<bool> connected{false}, closing{false}, limit{false}, fileError{false}, full{false}, lost{false}, opened{false};
        auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_ip.toStdString(), NET_GPIO_IN_STREAMING_PORT, buffers);
        net->setPackID(DataLib::ID_PACK_GPIO);
        net->clientConnectNotify.connect([&](std::string&) { connected = true; });
        net->clientErrorNotify.connect([&](std::error_code) {
            if (!closing && !limit)
                lost = true;
        });
        net->reciveNotify.connect([&](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
            if (!error) {
                auto buffer = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
                DataLib::GPIOHeader h;
                std::lock_guard wl(writerMtx);
                // a buffer after the close of the file is not taken
                if (!closing && DataLib::getHeaderGPIO(buffer, h) && !limit && !fileError && !full) {
                    const bool rle = h.flags & DataLib::GPIO_FLAG_RLE;
                    if (!opened) {
                        // the file takes the clock, the decimation and the modes of the first buffer
                        GpioRecording rec;
                        rec.format = CStreamSettings::GPIOFormat(static_cast<CStreamSettings::GPIOFormat::_enumerated>(format));
                        rec.rle = rle;
                        rec.clock = h.gpioClock;
                        rec.decimation = h.decimation ? h.decimation : 1;
                        rec.triggerMode = h.flags & DataLib::GPIO_FLAG_TRIGGER_MODE;
                        rec.waitCounter = h.flags & DataLib::GPIO_FLAG_WAIT_COUNTER;
                        rec.startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                        rec.samplesLimit = samplesLimit;
                        rec.limits.time = timeLimit;
                        // a buffer lasts at most its words of runs of 256 samples (RLE), else a sample each
                        if (rec.clock)
                            rec.bufferSeconds = blockSize / 4.0 * (rle ? 256 : 1) * rec.decimation / double(rec.clock);
                        opened = writer.open(file, rec);
                        fileError = !opened;
                    }
                    if (opened) {
                        const size_t count = std::min<size_t>(h.dataSize, buffer->getDataLenght()) / 4;
                        auto words = static_cast<const uint32_t*>(buffer->getMappedDataMemory());
                        const uint64_t samples = bin_lib::gpioInSamples(words, count, rle);
                        switch (writer.write(words, count, h.lostFPGA, h.waitFPGA, h.triggerCount)) {
                            case CGpioFileWriter::Result::OK:
                                break;
                            case CGpioFileWriter::Result::LIMIT:
                                limit = true;
                                break;
                            case CGpioFileWriter::Result::FULL:
                                full = true;
                                break;
                            default:
                                fileError = true;
                        }
                        std::lock_guard lock(m_gpioMtx);
                        auto& s = m_ginStat;
                        s.opened = true;
                        s.rle = rle;
                        s.samples += samples;
                        s.words += count;
                        s.bytes += count * 4;
                        s.lost += h.lostFPGA;
                        s.waited += h.waitFPGA;
                        s.packs++;
                        s.part = writer.part();
                        s.file = QString::fromStdString(writer.path());
                    }
                }
            }
            buffers->unlockBufferRead();
        });
        net->start();
        const qint64 connectEnd = QDateTime::currentMSecsSinceEpoch() + 5000;
        while (!connected && !m_gin.terminate && QDateTime::currentMSecsSinceEpoch() < connectEnd)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (!connected && !m_gin.terminate) {
            end = GpioEnd::NoConnection;
            text = "no data connection to the board on port " + QString::number(NET_GPIO_IN_STREAMING_PORT) + " in 5 s";
        }
        while (connected) {
            if (m_gin.terminate)
                break;
            if (limit) {
                end = GpioEnd::Limit;
                break;
            }
            if (fileError) {
                end = GpioEnd::FileError;
                text = "the file " + QDir::toNativeSeparators(path) + " could not be written";
                break;
            }
            if (full) {
                end = GpioEnd::DiskFull;
                text = "the disk of this computer is full: the file is closed";
                break;
            }
            if (lost) {
                end = GpioEnd::Lost;
                text = "the data connection broke";
                break;
            }
            if (timeLimit > 0 && opened) {
                std::lock_guard wl(writerMtx);
                if (writer.timeUp()) {
                    end = GpioEnd::TimeUp;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        closing = true;
        net->stop();
        std::lock_guard wl(writerMtx);
        writer.close();
    } catch (std::exception& e) {
        end = GpioEnd::FileError;
        text = e.what();
    }
    m_gin.running = false;
    post([this, end, text]() {
        m_gin.end = end;
        m_gin.endText = text;
        gpioThreadEnded(false);
    });
}

// The file on the output; the connection closes after the last word: the board plays what it
// got and stops by itself (runOutput of rpsa_client -G)
auto CBoard::gpioOutRun(std::shared_ptr<CGpioFileReader> reader, uint32_t blockSize) -> void {
    GpioEnd end = GpioEnd::Stopped;
    QString text;
    try {
        uio_lib::CMemoryManager memory;
        memory.setMemoryBlockSize(blockSize);
        memory.reallocateBlocks();
        memory.reserveMemory(uio_lib::MM_GPIO_OUT, memory.getFreeBlockCount(), 1);
        auto buffers = DataLib::CBuffersCached::create();
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        buffers->generateBuffersEmptyADC(one, memory.getRegions(uio_lib::MM_GPIO_OUT), DataLib::sizeHeader());

        std::atomic<bool> connected{false}, closing{false}, lost{false};
        auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_ip.toStdString(), NET_GPIO_OUT_STREAMING_PORT, nullptr);
        net->clientConnectNotify.connect([&](std::string&) { connected = true; });
        net->clientErrorNotify.connect([&](std::error_code) {
            if (!closing)
                lost = true;
        });
        net->start();
        const qint64 connectEnd = QDateTime::currentMSecsSinceEpoch() + 5000;
        while (!connected && !m_gout.terminate && QDateTime::currentMSecsSinceEpoch() < connectEnd)
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (!connected && !m_gout.terminate) {
            end = GpioEnd::NoConnection;
            text = "no data connection to the board on port " + QString::number(NET_GPIO_OUT_STREAMING_PORT) + " in 5 s";
        }
        auto pack = connected ? buffers->writeBuffer(true) : nullptr;
        auto buffer = pack ? pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1) : nullptr;
        if (buffer) {
            post([this]() {
                if (m_gout.state != "connecting")
                    return;
                setGpioState(m_gout, "playing");
                addLog("GPIO output playing: the data connection is open");
                dismissError();  // a start that goes
            });
        }
        const size_t maxWords = buffer ? buffer->getDataLenght() / 4 : 0;
        auto data = buffer ? static_cast<uint32_t*>(buffer->getMappedDataMemory()) : nullptr;
        uint64_t id = 0;
        while (buffer && !m_gout.terminate) {
            size_t got = 0;
            const auto result = reader->read(data, maxWords, got);
            if (got > 0) {
                DataLib::GPIOHeader h;
                h.dataSize = static_cast<uint32_t>(got * 4);
                DataLib::setHeaderGPIO(buffer, id++, h);
                if (!net->sendSyncData(pack)) {
                    if (!m_gout.terminate) {
                        end = GpioEnd::Lost;
                        text = "the data connection broke";
                    }
                    break;
                }
                std::lock_guard lock(m_gpioMtx);
                m_goutStat.words += got;
                m_goutStat.bytes += got * 4;
            }
            if (lost && !m_gout.terminate) {
                end = GpioEnd::Lost;
                text = "the data connection broke";
                break;
            }
            if (result == CGpioFileReader::Result::ENDED) {
                end = GpioEnd::Done;
                break;
            }
        }
        closing = true;
        net->stop();
    } catch (std::exception& e) {
        end = GpioEnd::FileError;
        text = QString("the file cannot be read: ") + e.what();
    }
    m_gout.running = false;
    post([this, end, text]() {
        m_gout.end = end;
        m_gout.endText = text;
        gpioThreadEnded(true);
    });
}
