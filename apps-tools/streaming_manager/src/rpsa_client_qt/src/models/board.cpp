#include "board.h"
#include <math.h>
#include <QDateTime>
#include <QQmlEngine>
#include <QSettings>
#include <sstream>

#include "src/logic/chartdataholder.h"

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

auto convertBtoS(uint64_t value) -> std::string {
    double d = value;
    std::string s = "";
    if (value >= 1024 * 1024) {
        d = round(((double)value * 1000.0) / (1024 * 1024)) / 1000;
        s = to_string_with_precision(d, 3) + " MB";
    } else if (value >= 1024) {
        d = round(((double)value * 1000.0) / (1024)) / 1000;
        s = to_string_with_precision(d, 3) + " kB";
    } else {
        s = std::to_string(value) + " B";
    }
    return s;
}

auto convertBtoST(uint64_t value) -> std::string {
    std::string s = "";
    auto h = value / (60 * 60 * 1000);
    auto m = (value - h * 60 * 60 * 1000) / (60 * 1000);
    auto sec = (value - h * 60 * 60 * 1000 - m * 60 * 1000) / (1000);
    auto ms = value % 1000;
    s = std::to_string(h) + ":" + std::to_string(m) + ":" + std::to_string(sec) + "." + std::to_string(ms);
    return s;
}

auto convertBtoSpeed(uint64_t value, uint64_t time) -> std::string {
    double d = value;
    double t = time;
    t = t / 1000;
    d = d / t;
    std::string s = "";
    if (value >= 1024 * 1024) {
        d = round(((double)d * 1000.0) / (1024 * 1024)) / 1000;
        s = to_string_with_precision(d, 3) + " MB/s";
    } else if (value >= 1024) {
        d = round(((double)d * 1000.0) / (1024)) / 1000;
        s = to_string_with_precision(d, 3) + " kB/s";
    } else {
        s = std::to_string(d) + " B/s";
    }
    return s;
}

CBoard::CBoard(QString ip)
    : QObject{nullptr},
      m_offlineTimer(nullptr),
      m_IsOnline(false),
      m_ip(ip),
      m_lastOnline(0),
      m_mode(broadcast_lib::AB_NONE),
      m_chartEnable(false),
      m_isADCStarted(false),
      m_testMode(false),
      m_adcChannels(2),
      m_IsACDC(false),
      m_IsAttenuator(false) {
    QSettings setting("RedPitaya", "rpsa_cient_qt_" + m_ip);
    m_chartEnable = setting.value("chartEnable", false).toBool();
    m_testMode = setting.value("testMode", false).toBool();

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    m_offlineTimer = new QTimer(this);
    connect(m_offlineTimer, &QTimer::timeout, this, QOverload<>::of(&CBoard::updateOffline));
    m_offlineTimer->start(10000);
    m_configManager = new ClientNetConfigManager("", false);

    m_configManager->serverConnectedNofiy.connect(&CBoard::configManagerConnected, this);
    m_configManager->errorNofiy.connect(&CBoard::configMangerError, this);
    m_configManager->getNewSettingsNofiy.connect(&CBoard::getNewSettings, this);
    m_configManager->successSendConfigNofiy.connect(&CBoard::sendSettings, this);

    m_configManager->serverStartedTCPNofiy.connect([=](auto host) {
        addLog("Streaming started");
        m_configManager->requestMemoryBlockSize(host);
    });

    m_configManager->serverStartedSDNofiy.connect([=](auto host) {
        addLog("Streaming started in SD mode");
        m_configManager->sendADCFPGAStart(m_ip.toStdString());
    });

    m_configManager->serverStoppedNofiy.connect([=](auto host) {
        addLog("Streaming stopped");
        m_isADCStarted = false;
        ChartDataHolder::instance()->removeRP(ip);
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->serverStoppedMemErrorNofiy.connect([=](auto host) {
        addLog("Streaming stopped. Not enough DMA memory.");
        m_isADCStarted = false;
        ChartDataHolder::instance()->removeRP(ip);
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->serverStoppedMemModifyNofiy.connect([=](auto host) {
        addLog("Streaming stopped. The memory manager has changed.");
        m_isADCStarted = false;
        ChartDataHolder::instance()->removeRP(ip);
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->serverStoppedNoActiveChannelsNofiy.connect([=](auto host) {
        addLog("Streaming stopped. No active channels.");
        m_isADCStarted = false;
        ChartDataHolder::instance()->removeRP(ip);
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->getMemBlockSizeNofiy.connect([=](auto host, auto size) {
        addLog(" Memory block size of " + QString::fromStdString(size));
        m_blockSize = std::stoi(size);
        m_configManager->requestActiveChannels(host);
    });

    m_configManager->getActiveChannelsNofiy.connect([=](auto host, auto channels) {
        addLog(" Active channels " + QString::fromStdString(channels));
        m_activeChannels = std::stoi(channels);
        createStreaming();
        ChartDataHolder::instance()->regRP(ip);
        Q_EMIT updateSaveFileName();
    });

    m_configManager->serverStoppedSDFullNofiy.connect([=](auto host) {
        addLog("Streaming stopped SD FULL");
        m_isADCStarted = false;
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->serverStoppedSDDoneNofiy.connect([=](auto host) {
        addLog("Streaming stopped SD mode done");
        m_isADCStarted = false;
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->startADCDoneNofiy.connect([=](auto host) {
        addLog("ADC started");
        m_isADCStarted = true;
        Q_EMIT isADCStartedChanged();
    });

    std::vector ipList{ip.toStdString()};
    m_configManager->connectToServers(ipList, NET_CONFIG_PORT);
}

auto CBoard::addLog(QString msg) -> void {
    QMetaObject::invokeMethod(&m_consoleModel,
                              "addNewLine",
                              Qt::AutoConnection  // Can also use any other except DirectConnection
                              ,
                              Q_ARG(QString, msg));  // And some more args if needed
}

CBoard::~CBoard() {
    delete m_configManager;
}

auto CBoard::createStreaming() -> void {
    m_buffer = DataLib::CBuffersCached::create();
    m_stat = SStat();
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    auto df = m_configManager->getADCFormat();
    //    auto samples = m_configManager->getSamples();

    bool convert_v = m_configManager->getADCType().value == CStreamSettings::DataType::VOLT;

    std::string dir = "output";
    m_file_manager = streaming_lib::CStreamingFile::create(df, dir, -1, convert_v, m_testMode);
    QString str = m_ip + "_" + m_streamingDT.toString("yyyy-MM-dd_HH-mm-ss");
    m_file_manager->run(str.toStdString());

    auto g_s_file_w = std::weak_ptr<streaming_lib::CStreamingFile>(m_file_manager);
    auto g_s_buffers_w = std::weak_ptr<DataLib::CBuffersCached>(m_buffer);

    m_asionet = net_lib::CAsioNet::create(net_lib::M_CLIENT, m_ip.toStdString(), NET_ADC_STREAMING_PORT, m_buffer);

    m_asionet->reciveNotify.connect([=](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        if (!error) {
            auto obj = g_s_file_w.lock();
            auto obj2 = g_s_buffers_w.lock();
            if (obj && obj2) {
                pack->getInfoFromHeaderADC();
                uint64_t sempCh1 = 0;
                uint64_t sempCh2 = 0;
                uint64_t sempCh3 = 0;
                uint64_t sempCh4 = 0;
                uint64_t sizeCh1 = 0;
                uint64_t sizeCh2 = 0;
                uint64_t sizeCh3 = 0;
                uint64_t sizeCh4 = 0;
                uint64_t lostRate = 0;
                uint64_t packId = 0;
                auto ch1 = pack->getBuffer(DataLib::CH1);
                auto ch2 = pack->getBuffer(DataLib::CH2);
                auto ch3 = pack->getBuffer(DataLib::CH3);
                auto ch4 = pack->getBuffer(DataLib::CH4);

                if (ch1) {
                    sempCh1 = ch1->getSamplesCount();
                    sizeCh1 = ch1->getDataLenght();
                    lostRate += ch1->getLostSamplesAll();
                    packId = std::max(packId, ch1->getADCPackId());
                }

                if (ch2) {
                    sempCh2 = ch2->getSamplesCount();
                    sizeCh2 = ch2->getDataLenght();
                    lostRate += ch2->getLostSamplesAll();
                    packId = std::max(packId, ch2->getADCPackId());
                }

                if (ch3) {
                    sempCh3 = ch3->getSamplesCount();
                    sizeCh3 = ch3->getDataLenght();
                    lostRate += ch3->getLostSamplesAll();
                    packId = std::max(packId, ch3->getADCPackId());
                }

                if (ch4) {
                    sempCh4 = ch4->getSamplesCount();
                    sizeCh4 = ch4->getDataLenght();
                    lostRate += ch4->getLostSamplesAll();
                    packId = std::max(packId, ch4->getADCPackId());
                }

                auto flost = obj->getFileLost();

                m_stat.bytes += sizeCh1 + sizeCh2 + sizeCh3 + sizeCh4;
                m_stat.bw = convertBtoSpeed(m_stat.bytes, QDateTime::currentMSecsSinceEpoch() - startTime);
                m_stat.samples1 += sempCh1;
                m_stat.samples2 += sempCh2;
                m_stat.samples3 += sempCh3;
                m_stat.samples4 += sempCh4;

                m_stat.lost += lostRate;
                m_stat.flost += flost;
                m_stat.broken_b = 0;
                obj->passBuffers(pack);
                Q_EMIT updateStatistic();
                ChartDataHolder::instance()->addBuffer(pack, packId, m_ip, !m_chartEnable);
                obj2->unlockBufferRead();
            }
        }
    });

    m_asionet->clientConnectNotify.connect([=](std::string host) {
        QString msg = "Connect for streaming";
        addLog(msg);
        startADCFPGAStreaming();
    });

    m_asionet->clientErrorNotify.connect([=](std::error_code err) {
#if defined(Q_OS_WIN)
        std::wstringstream ws;
        ws << err.message().c_str();
        QString msg = "Error " + QString::fromWCharArray(ws.str().c_str());
#else
        QString msg = "Error " + QString::fromStdString(err.message());
#endif
        addLog(msg);
        stopStreaming();
    });

    m_asionet->start();
}

auto CBoard::startADCFPGAStreaming() -> void {
    auto memoryManager = new uio_lib::CMemoryManager();
    memoryManager->setMemoryBlockSize(m_blockSize);
    memoryManager->reallocateBlocks();
    auto blocks = memoryManager->getFreeBlockCount();
    auto reserved __attribute__((unused)) = memoryManager->reserveMemory(uio_lib::MM_ADC, blocks, m_activeChannels);
    m_buffer->generateBuffersEmpty(m_activeChannels, memoryManager->getRegions(uio_lib::MM_ADC), DataLib::sizeHeader());
    TRACE_SHORT("Reserved blocks %d", reserved)
    m_configManager->sendADCFPGAStart(m_ip.toStdString());
}

auto CBoard::updateOffline() -> void {
    if (QDateTime::currentMSecsSinceEpoch() - m_lastOnline > 10000) {
        m_IsOnline = false;
        Q_EMIT isOnlineChanged();
    }
}

QObject* CBoard::getConsoleModel() {
    return &m_consoleModel;
}

auto CBoard::setOnline() -> void {
    m_IsOnline = true;
    m_lastOnline = QDateTime::currentMSecsSinceEpoch();
    Q_EMIT isOnlineChanged();
}

auto CBoard::getIP() -> QString {
    return m_ip;
}

auto CBoard::setMode(broadcast_lib::EMode mode) -> void {
    m_mode = mode;
    Q_EMIT isMasterChanged();
}

auto CBoard::setModel(uint8_t model) -> void {

    // typedef enum {
    //     STEM_125_10_v1_0 = 0,
    //     STEM_125_14_v1_0 = 1,
    //     STEM_125_14_v1_1 = 2,
    //     STEM_122_16SDR_v1_0 = 3,
    //     STEM_122_16SDR_v1_1 = 4,
    //     STEM_125_14_LN_v1_1 = 5,
    //     STEM_125_14_Z7020_v1_0 = 6,
    //     STEM_125_14_Z7020_LN_v1_1 = 7,
    //     STEM_125_14_Z7020_4IN_v1_0 = 8,
    //     STEM_125_14_Z7020_4IN_v1_2 = 9,
    //     STEM_125_14_Z7020_4IN_v1_3 = 10,
    //     STEM_250_12_v1_0 = 11,
    //     STEM_250_12_v1_1 = 12,
    //     STEM_250_12_v1_2 = 13,
    //     STEM_250_12_120 = 14,
    //     STEM_250_12_v1_2a = 15,
    //     STEM_250_12_v1_2b = 16,
    //     STEM_125_14_LN_BO_v1_1 = 17,
    //     STEM_125_14_LN_CE1_v1_1 = 18,
    //     STEM_125_14_LN_CE2_v1_1 = 19,

    //     STEM_125_14_v2_0 = 20,
    //     STEM_125_14_Pro_v2_0 = 21,
    //     STEM_125_14_Z7020_Pro_v2_0 = 22,
    //     STEM_125_14_Z7020_Ind_v2_0 = 23,
    //     STEM_125_14_Z7020_Pro_v1_0 = 24,  // Prototype

    //     STEM_125_14_Z7020_LL_v1_1 = 25,
    //     STEM_65_16_Z7020_LL_v1_1 = 26,
    //     STEM_125_14_Z7020_LL_v1_2 = 27,
    //     STEM_125_14_Z7020_TI_v1_3 = 28,
    //     STEM_65_16_Z7020_TI_v1_3 = 29,

    //     STEM_125_14_Z7020_4IN_BO_v1_3 = 30
    // } rp_HPeModels_t;

    switch (model) {
        case 0:
        case 1:
        case 2:
        case 5:
        case 6:
        case 7:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29: {
            m_adcChannels = 2;
            m_IsACDC = false;
            m_IsAttenuator = true;
            break;
        }
        case 3:
        case 4: {
            m_adcChannels = 2;
            m_IsACDC = false;
            m_IsAttenuator = false;
            break;
        }

        case 8:
        case 9:
        case 10:
        case 30: {
            m_adcChannels = 4;
            m_IsACDC = false;
            m_IsAttenuator = true;
            break;
        }

        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16: {
            m_adcChannels = 2;
            m_IsACDC = true;
            m_IsAttenuator = true;
            break;
        }

        default:
            m_IsAttenuator = false;
            m_adcChannels = 0;
            m_IsACDC = false;
    }
    m_model = model;
    Q_EMIT modelChanged();
}

auto CBoard::getIsMaster() -> bool {
    return m_mode == broadcast_lib::AB_SERVER_MASTER;
}

auto CBoard::getModel() -> QString {

    // typedef enum {
    //     STEM_125_10_v1_0 = 0,
    //     STEM_125_14_v1_0 = 1,
    //     STEM_125_14_v1_1 = 2,
    //     STEM_122_16SDR_v1_0 = 3,
    //     STEM_122_16SDR_v1_1 = 4,
    //     STEM_125_14_LN_v1_1 = 5,
    //     STEM_125_14_Z7020_v1_0 = 6,
    //     STEM_125_14_Z7020_LN_v1_1 = 7,
    //     STEM_125_14_Z7020_4IN_v1_0 = 8,
    //     STEM_125_14_Z7020_4IN_v1_2 = 9,
    //     STEM_125_14_Z7020_4IN_v1_3 = 10,
    //     STEM_250_12_v1_0 = 11,
    //     STEM_250_12_v1_1 = 12,
    //     STEM_250_12_v1_2 = 13,
    //     STEM_250_12_120 = 14,
    //     STEM_250_12_v1_2a = 15,
    //     STEM_250_12_v1_2b = 16,
    //     STEM_125_14_LN_BO_v1_1 = 17,
    //     STEM_125_14_LN_CE1_v1_1 = 18,
    //     STEM_125_14_LN_CE2_v1_1 = 19,

    //     STEM_125_14_v2_0 = 20,
    //     STEM_125_14_Pro_v2_0 = 21,
    //     STEM_125_14_Z7020_Pro_v2_0 = 22,
    //     STEM_125_14_Z7020_Ind_v2_0 = 23,
    //     STEM_125_14_Z7020_Pro_v1_0 = 24,  // Prototype

    //     STEM_125_14_Z7020_LL_v1_1 = 25,
    //     STEM_65_16_Z7020_LL_v1_1 = 26,
    //     STEM_125_14_Z7020_LL_v1_2 = 27,
    //     STEM_125_14_Z7020_TI_v1_3 = 28,
    //     STEM_65_16_Z7020_TI_v1_3 = 29,

    //     STEM_125_14_Z7020_4IN_BO_v1_3 = 30
    // } rp_HPeModels_t;

    switch (m_model) {
        case 0:
            return "STEAMlab 125-10 v1.0";
        case 1:
            return "STEAMlab 125-14 v1.0";
        case 2:
            return "STEAMlab 125-14 v1.1";
        case 3:
            return "SDRlab 122-16 v1.0";
        case 4:
            return "SDRlab 122-16 v1.1";
        case 5:
            return "STEMlab 125-14 LN v1.1";
        case 6:
            return "STEMlab 125-14 Z7020 v1.0";
        case 7:
            return "STEMlab 125-14 LN-Z7020 v1.1";
        case 8:
            return "STEMlab 125-14 4-Input v1.0";
        case 9:
            return "STEMlab 125-14 4-Input v1.2";
        case 10:
            return "STEMlab 125-14 4-Input v1.3";

        case 11:
            return "SIGNALlab 250-12 v1.0";
        case 12:
            return "SIGNALlab 250-12 v1.1";
        case 13:
            return "SIGNALlab 250-12 v1.2";
        case 14:
            return "SIGNALlab 250-12/120";
        case 15:
            return "SIGNALlab 250-12 v1.2a";
        case 16:
            return "SIGNALlab 250-12 v1.2b";

        case 17:
            return "STEMlab 125-14 LN BO v1.1";
        case 18:
            return "STEMlab 125-14 LN CE1 v1.1";
        case 19:
            return "STEMlab 125-14 LN CE2 v1.1";

        case 20:
            return "STEMlab 125-14 v2.0";
        case 21:
            return "STEMlab 125-14 Pro v2.0";
        case 22:
            return "STEMlab 125-14-Z7020 Pro v2.0";
        case 23:
            return "STEMlab 125-14 Ind v2.0";
        case 24:
            return "STEMlab 125-14-Z7020 Pro v1.0";

        case 25:
            return "STEMlab 125-14 LL v1.1";
        case 26:
            return "STEMlab 65-16 LL v1.1";
        case 27:
            return "STEMlab 125-14 LL v1.2";
        case 28:
            return "STEMlab 125-14 TI v1.3";
        case 29:
            return "STEMlab 65-16 TI v1.3";
        case 30:
            return "STEMlab 125-14 4-Input BO v1.3";
        default:
            break;
    }
    return "Unknown";
}

auto CBoard::configManagerConnected(std::string host) -> void {
    addLog("Connected to the configuration server");
    Q_EMIT configManagerConnectedChanged();
    getConfig();
}

auto CBoard::configMangerError(ClientNetConfigManager::Errors errors, std::string host, error_code err) -> void {
    QString msg = "Error from configuratoin server: " + QString::fromStdString(err.message());
    addLog(msg);
    Q_EMIT configManagerConnectedChanged();
}

auto CBoard::getConfigManagerConnected() -> bool {
    return m_configManager->isServersConnected();
}

void CBoard::setChartEnable(bool enable) {
    m_chartEnable = enable;
    QSettings setting("RedPitaya", "rpsa_cient_qt_" + m_ip);
    setting.setValue("chartEnable", enable);
}

bool CBoard::getChartEnable() {
    return m_chartEnable;
}

void CBoard::sendConfig() {
    if (!m_configManager->isServersConnected()) {
        QString msg = "Error: Configuration server not connected";
        addLog(msg);
        return;
    }
    m_configManager->sendConfig(m_ip.toStdString());
}

void CBoard::getConfig() {
    if (!m_configManager->isServersConnected()) {
        QString msg = "Error: Configuration server not connected";
        addLog(msg);
        return;
    }
    m_configManager->requestConfig(m_ip.toStdString());
}

auto CBoard::getNewSettings(std::string host) -> void {
    QString msg = "Get settings from server";
    addLog(msg);
    Q_EMIT getNewSettingSignal();
}

auto CBoard::sendSettings(std::string host) -> void {
    QString msg = "Settings sent to server";
    addLog(msg);
    m_configManager->sendSaveToFile(m_ip.toStdString());
}

auto CBoard::getSaveType() -> int {
    auto mode = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCPassMode();
    m_configManager->setADCPassMode(mode);
    return mode;
}

void CBoard::setSaveType(int mode) {
    m_configManager->setADCPassMode((CStreamSettings::PassMode::_enumerated)mode);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCPassMode((CStreamSettings::PassMode::_enumerated)mode);
}

auto CBoard::getDecimation() -> int {
    auto dec = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCDecimation();
    m_configManager->setADCDecimation(dec);
    return dec;
}

void CBoard::setDecimation(int dec) {
    if ((dec >= 16 && dec <= 65535) || (dec == 1) || (dec == 2) || (dec == 4) || (dec == 8)) {
        m_configManager->setADCDecimation(dec);
        m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCDecimation(dec);
    }
}

auto CBoard::getSampleLimit() -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCSamples();
    m_configManager->setADCSamples(x);
    return x;
}

void CBoard::setSampleLimit(int s) {
    m_configManager->setADCSamples(s);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCSamples(s);
}

auto CBoard::getResolution() -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCResolution();
    m_configManager->setADCResolution(x);
    return x;
}

void CBoard::setResolution(int r) {
    m_configManager->setADCResolution((CStreamSettings::Resolution::_enumerated)r);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCResolution((CStreamSettings::Resolution::_enumerated)r);
}

auto CBoard::getAttenuator(int _channel) -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCAttenuator(_channel);
    m_configManager->setADCAttenuator(_channel, x);
    return x;
}

void CBoard::setAttenuator(int _channel, int a) {
    m_configManager->setADCAttenuator(_channel, (CStreamSettings::Attenuator::_enumerated)a);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCAttenuator(_channel, (CStreamSettings::Attenuator::_enumerated)a);
}

auto CBoard::getCalibration() -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCCalibration();
    m_configManager->setADCCalibration(x);
    return x;
}

void CBoard::setCalibration(int a) {
    m_configManager->setADCCalibration((CStreamSettings::State::_enumerated)a);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCCalibration((CStreamSettings::State::_enumerated)a);
}

auto CBoard::getChannels(int channel) -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCChannels(channel);
    m_configManager->setADCChannels(channel, x);
    return x;
}

void CBoard::setChannels(int channel, int x) {
    m_configManager->setADCChannels(channel, (CStreamSettings::State::_enumerated)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCChannels(channel, (CStreamSettings::State::_enumerated)x);
}

auto CBoard::getDataFormat() -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCFormat();
    m_configManager->setADCFormat(x);
    return x;
}

void CBoard::setDataFormat(int x) {
    m_configManager->setADCFormat((CStreamSettings::DataFormat::_enumerated)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCFormat((CStreamSettings::DataFormat::_enumerated)x);
}

auto CBoard::getType() -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCType();
    m_configManager->setADCType(x);
    return x;
}

void CBoard::setType(int x) {
    m_configManager->setADCType((CStreamSettings::DataType::_enumerated)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCType((CStreamSettings::DataType::_enumerated)x);
}

auto CBoard::getCoupling(int channel) -> int {
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getADCAC_DC(channel);
    m_configManager->setADCAC_DC(channel, x);
    return x;
}

void CBoard::setCoupling(int channel, int x) {
    m_configManager->setADCAC_DC(channel, (CStreamSettings::AC_DC::_enumerated)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setADCAC_DC(channel, (CStreamSettings::AC_DC::_enumerated)x);
}

void CBoard::startStreaming(QDateTime time) {
    m_streamingDT = time;
    m_configManager->sendADCServerStart(m_ip.toStdString());
}

void CBoard::stopStreaming() {
    m_asionet = nullptr;
    if (m_file_manager)
        m_file_manager->stopAndFlush();
    m_file_manager = nullptr;
    m_configManager->sendADCServerStop(m_ip.toStdString());
}

auto CBoard::getIsADCStarted() -> bool {
    return m_isADCStarted;
}

QString CBoard::getRecivedBytes() {
    return QString::fromStdString(convertBtoS(m_stat.bytes));
}

QString CBoard::getBandwidth() {
    return QString::fromStdString(m_stat.bw);
}

QString CBoard::getSamplesCH1() {
    return QString::number(m_stat.samples1);
}

QString CBoard::getSamplesCH2() {
    return QString::number(m_stat.samples2);
}

QString CBoard::getSamplesCH3() {
    return QString::number(m_stat.samples3);
}

QString CBoard::getSamplesCH4() {
    return QString::number(m_stat.samples4);
}

QString CBoard::getLostCount() {
    return QString::number(m_stat.lost);
}

QString CBoard::getFLostCount() {
    return QString::number(m_stat.flost);
}

QString CBoard::getSaveFileName() {
    if (m_file_manager) {
        return QString::fromStdString(m_file_manager->getCSVFileName());
    }
    return "";
}

bool CBoard::getCouplingVisible() {

    // typedef enum {
    //     STEM_125_10_v1_0 = 0,
    //     STEM_125_14_v1_0 = 1,
    //     STEM_125_14_v1_1 = 2,
    //     STEM_122_16SDR_v1_0 = 3,
    //     STEM_122_16SDR_v1_1 = 4,
    //     STEM_125_14_LN_v1_1 = 5,
    //     STEM_125_14_Z7020_v1_0 = 6,
    //     STEM_125_14_Z7020_LN_v1_1 = 7,
    //     STEM_125_14_Z7020_4IN_v1_0 = 8,
    //     STEM_125_14_Z7020_4IN_v1_2 = 9,
    //     STEM_125_14_Z7020_4IN_v1_3 = 10,
    //     STEM_250_12_v1_0 = 11,
    //     STEM_250_12_v1_1 = 12,
    //     STEM_250_12_v1_2 = 13,
    //     STEM_250_12_120 = 14,
    //     STEM_250_12_v1_2a = 15,
    //     STEM_250_12_v1_2b = 16,
    //     STEM_125_14_LN_BO_v1_1 = 17,
    //     STEM_125_14_LN_CE1_v1_1 = 18,
    //     STEM_125_14_LN_CE2_v1_1 = 19,

    //     STEM_125_14_v2_0 = 20,
    //     STEM_125_14_Pro_v2_0 = 21,
    //     STEM_125_14_Z7020_Pro_v2_0 = 22,
    //     STEM_125_14_Z7020_Ind_v2_0 = 23,
    //     STEM_125_14_Z7020_Pro_v1_0 = 24,  // Prototype

    //     STEM_125_14_Z7020_LL_v1_1 = 25,
    //     STEM_65_16_Z7020_LL_v1_1 = 26,
    //     STEM_125_14_Z7020_LL_v1_2 = 27,
    //     STEM_125_14_Z7020_TI_v1_3 = 28,
    //     STEM_65_16_Z7020_TI_v1_3 = 29,

    //     STEM_125_14_Z7020_4IN_BO_v1_3 = 30
    // } rp_HPeModels_t;

    switch (m_model) {
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            return true;
        default:
            return false;
    }
}

bool CBoard::getTestMode() {
    return m_testMode;
}

void CBoard::setTestMode(bool mode) {
    QSettings setting("RedPitaya", "rpsa_cient_qt_" + m_ip);
    setting.setValue("testMode", mode);
    m_testMode = mode;
}
