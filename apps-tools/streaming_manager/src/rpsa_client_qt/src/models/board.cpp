#include <QQmlEngine>
#include <QDateTime>
#include <math.h>
#include "board.h"

#include "src/logic/chartdataholder.h"

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
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
        s =  to_string_with_precision(d,3) + " Mb";
    } else if (value >= 1024){
        d = round(((double)value * 1000.0) / (1024)) / 1000;
        s =  to_string_with_precision(d,3) + " kb";
    }else  {
        s =  std::to_string(value) + " b";
    }
    return s;
}

auto convertBtoST(uint64_t value) -> std::string {
    std::string s = "";
    auto h = value / (60 * 60 * 1000);
    auto m = (value - h * 60 * 60 * 1000) / (60 * 1000);
    auto sec = (value - h * 60 * 60 * 1000 - m * 60 * 1000) / (1000);
    auto ms = value % 1000;
    s = std::to_string(h) +":" + std::to_string(m) +":" + std::to_string(sec) + "." + std::to_string(ms);
    return s;
}

auto convertBtoSpeed(uint64_t value,uint64_t time) -> std::string {
    double d = value;
    double t = time;
    t = t / 1000;
    d = d / t;
    std::string s = "";
    if (value >= 1024 * 1024) {
        d = round(((double)d * 1000.0) / (1024 * 1024)) / 1000;
        s =  to_string_with_precision(d,3) + " MB/s";
    } else if (value >= 1024){
        d = round(((double)d * 1000.0) / (1024)) / 1000;
        s =  to_string_with_precision(d,3) + " kB/s";
    }else  {
        s =  std::to_string(d) + " B/s";
    }
    return s;
}

CBoard::CBoard(QString ip)
    : QObject{nullptr}
    ,m_offlineTimer(nullptr)
    ,m_IsOnline(false)
    ,m_ip(ip)
    ,m_lastOnline(0)
    ,m_mode(broadcast_lib::AB_NONE)
    ,m_chartEnable(false)
    ,m_isADCStarted(false)    
{


    QQmlEngine::setObjectOwnership(this,QQmlEngine::CppOwnership);
    m_offlineTimer = new QTimer(this);
    connect(m_offlineTimer, &QTimer::timeout, this, QOverload<>::of(&CBoard::updateOffline));
    m_offlineTimer->start(10000);
    m_configManager = new ClientNetConfigManager("",false);

    m_configManager->serverConnectedNofiy.connect(&CBoard::configManagerConnected,this);
    m_configManager->errorNofiy.connect(&CBoard::configMangerError,this);
    m_configManager->getNewSettingsNofiy.connect(&CBoard::getNewSettings,this);
    m_configManager->successSendConfigNofiy.connect(&CBoard::sendSettings,this);

    m_configManager->serverStartedTCPNofiy.connect([=](auto host){
        addLog("Streaming started in TCP");
        createStreaming(net_lib::P_TCP);        
        ChartDataHolder::instance()->regRP(ip);
        Q_EMIT updateSaveFileName();
    });

    m_configManager->serverStartedUDPNofiy.connect([=](auto host){
        addLog("Streaming started in UDP");
        createStreaming(net_lib::P_UDP);
        ChartDataHolder::instance()->regRP(ip);
        Q_EMIT updateSaveFileName();
    });

    m_configManager->serverStartedSDNofiy.connect([=](auto host){
        addLog("Streaming started in SD mode");
    });

    m_configManager->serverStoppedNofiy.connect([=](auto host){
        addLog("Streaming stopped");
        m_isADCStarted = false;
        ChartDataHolder::instance()->removeRP(ip);
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->serverStoppedSDFullNofiy.connect([=](auto host){
        addLog("Streaming stopped SD FULL");
        m_isADCStarted = false;
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->serverStoppedSDDoneNofiy.connect([=](auto host){
        addLog("Streaming stopped SD mode done");
        m_isADCStarted = false;
        Q_EMIT isADCStartedChanged();
    });

    m_configManager->startADCDoneNofiy.connect([=](auto host){
        addLog("ADC started");
        m_isADCStarted = true;
        Q_EMIT isADCStartedChanged();
    });

    std::vector ipList {ip.toStdString()};
    m_configManager->connectToServers(ipList,"8901");


}

auto CBoard::addLog(QString msg) -> void{
     QMetaObject::invokeMethod(&m_consoleModel
                                , "addNewLine"
                                , Qt::AutoConnection // Can also use any other except DirectConnection
                                , Q_ARG(QString, msg)); // And some more args if needed
}

CBoard::~CBoard(){
    delete m_configManager;
}

auto CBoard::createStreaming(net_lib::EProtocol protocol) -> void{
    m_stat = SStat();
    auto startTime = QDateTime::currentMSecsSinceEpoch();
    auto df = m_configManager->getFormat();
    auto samples = m_configManager->getSamples();

    bool convert_v = m_configManager->getType() == CStreamSettings::VOLT;


    std::string dir = "output";
    m_file_manager = streaming_lib::CStreamingFile::create(df, dir, samples, convert_v,false);
    QString str = m_ip + "_" + m_streamingDT.toString("yyyy-MM-dd_HH-mm-ss");
    m_file_manager->run(str.toStdString());

    m_net_buffer = streaming_lib::CStreamingNetBuffer::create();
    m_net_buffer->outMemoryNotify.connect([=](uint64_t ram){
        QString msg = "Out of memory (" + QString::number(ram) + ")";
        addLog(msg);
    });

    auto g_s_file_w = std::weak_ptr<streaming_lib::CStreamingFile>(m_file_manager);
    m_net_buffer->brokenPacksNotify.connect([=](uint64_t count){
        auto obj = g_s_file_w.lock();
        if(obj){
            obj->addNetWorkLost(count);
        }
    });


    m_net_buffer->receivedPackNotify.connect([=](DataLib::CDataBuffersPack::Ptr pack,uint64_t id){
        auto obj = g_s_file_w.lock();
        if (obj){

                uint64_t sempCh1 = 0;
                uint64_t sempCh2 = 0;
                uint64_t sizeCh1 = 0;
                uint64_t sizeCh2 = 0;
                uint64_t lostRate = 0;
                auto ch1 = pack->getBuffer(DataLib::CH1);
                auto ch2 = pack->getBuffer(DataLib::CH2);
                if (ch1){
                    sempCh1 = ch1->getSamplesCount();
                    sizeCh1 = ch1->getBufferLenght();
                    lostRate += ch1->getLostSamplesAll();
                }

                if (ch2){
                    sempCh2 = ch2->getSamplesCount();
                    sizeCh2 = ch2->getBufferLenght();
                    lostRate += ch2->getLostSamplesAll();
                }

                auto net   = obj->getNetworkLost();
                auto flost = obj->getFileLost();
//            int  brokenBuffer = -1;
//                if (g_soption.testStreamingMode == ClientOpt::TestSteamingMode::WITH_TEST_DATA){
//                    brokenBuffer = testBuffer(ch1 ? ch1->getBuffer().get() : nullptr,ch2 ? ch2->getBuffer().get() : nullptr,sizeCh1,sizeCh2) ? 0 : 1;
//                }
                m_stat.bytes += sizeCh1 + sizeCh2;
                m_stat.bw = convertBtoSpeed(m_stat.bytes,QDateTime::currentMSecsSinceEpoch() - startTime);
                m_stat.samples1 += sempCh1;
                m_stat.samples2 += sempCh2;
                m_stat.lost += lostRate;
                m_stat.flost += flost;
                m_stat.broken_b = 0;

                obj->passBuffers(pack);
                Q_EMIT updateStatistic();
        }

        ChartDataHolder::instance()->addBuffer(pack,id,m_ip,!m_chartEnable);

    });


    m_asionet = net_lib::CAsioNet::create(net_lib::M_CLIENT, protocol ,m_ip.toStdString() , "8900");

    m_asionet->clientConnectNotify.connect([=](std::string host) {
        QString msg = "Connect for streaming";
        addLog(msg);
        m_configManager->sendStartADC(m_ip.toStdString());
    });

    m_asionet->clientErrorNotify.connect([=](std::error_code err)
    {
        QString msg = "Error " + QString::fromStdString(err.message());
        addLog(msg);
        stopStreaming();
    });

    auto g_net_buffer_w = std::weak_ptr<streaming_lib::CStreamingNetBuffer>(m_net_buffer);
    m_asionet->reciveNotify.connect([g_net_buffer_w](std::error_code error,uint8_t *buff,size_t _size){
        auto obj = g_net_buffer_w.lock();
        if (obj){
            if (!error){
                obj->addNewBuffer(buff,_size);
            }
        }
    });
    m_asionet->start();
}

auto CBoard::updateOffline() -> void{
    if (QDateTime::currentMSecsSinceEpoch() - m_lastOnline > 10000){
        m_IsOnline = false;
        Q_EMIT isOnlineChanged();
    }
}

QObject* CBoard::getConsoleModel(){
    return &m_consoleModel;
}

auto CBoard::setOnline() -> void{
    m_IsOnline = true;
    m_lastOnline = QDateTime::currentMSecsSinceEpoch();
    Q_EMIT isOnlineChanged();
}

auto CBoard::getIP() -> QString{
    return m_ip;
}

auto CBoard::setMode(broadcast_lib::EMode mode) -> void{
    m_mode = mode;
    Q_EMIT isMasterChanged();
}

auto CBoard::setModel(broadcast_lib::EModel model) -> void{
    m_model = model;
    Q_EMIT modelChanged();
}

auto CBoard::getIsMaster() -> bool{
    return m_mode == broadcast_lib::AB_SERVER_MASTER;
}

auto CBoard::getModel() -> QString{
    switch(m_model){
        case broadcast_lib::EModel::RP_125_14: return "STEMLab 125-14";
        case broadcast_lib::EModel::RP_122_16: return "SDRLab 122-16";
        case broadcast_lib::EModel::RP_125_14_Z20: return "STEMLab 125-14 Z7020";
        case broadcast_lib::EModel::RP_125_4CH: return "STEMLab 125-14 4-channels";
        case broadcast_lib::EModel::RP_250_12: return "SIGNALab 250-12";
        default:
            return "Unknown";
    }
}

auto CBoard::configManagerConnected(std::string host) -> void{        
    addLog("Connected to configuration server");
    Q_EMIT configManagerConnectedChanged();
    getConfig();
}

auto CBoard::configMangerError(ClientNetConfigManager::Errors errors,std::string host,error_code err) -> void{
    QString msg = "Error from configuratoin server: " + QString::fromStdString(err.message());
    addLog(msg);
    Q_EMIT configManagerConnectedChanged();
}

auto CBoard::getConfigManagerConnected() -> bool{
    return m_configManager->isServersConnected();
}

void CBoard::setChartEnable(bool enable){
    m_chartEnable = enable;
}

void CBoard::sendConfig(){
    if (!m_configManager->isServersConnected()){
        QString msg = "Error: Configuration server not connected";
        addLog(msg);
        return;
    }
    m_configManager->sendConfig(m_ip.toStdString());
}

void CBoard::getConfig(){
    if (!m_configManager->isServersConnected()){
        QString msg = "Error: Configuration server not connected";
        addLog(msg);
        return;
    }
    m_configManager->requestConfig(m_ip.toStdString());
}

auto CBoard::getNewSettings(std::string host) -> void{    
    QString msg = "Get settings from server";
    addLog(msg);
    auto p = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getPort();
    m_configManager->setPort(p);
    auto dacp = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getDACPort();
    m_configManager->setDACPort(dacp);    
    Q_EMIT getNewSettingSignal();
}

auto CBoard::sendSettings(std::string host) -> void{
    QString msg = "Setted settings to server";
    addLog(msg);
    m_configManager->sendSaveToFile(m_ip.toStdString());
}

auto CBoard::getSaveType() -> int{
    auto mode = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getSaveType();
    m_configManager->setSaveType(mode);
    return mode;
}

void CBoard::setSaveType(int mode){
    m_configManager->setSaveType((CStreamSettings::SaveType)mode);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setSaveType((CStreamSettings::SaveType)mode);
}

auto CBoard::getProtocol() -> int{
    auto mode = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getProtocol();
    m_configManager->setProtocol(mode);
    return mode;
}

void CBoard::setProtocol(int protocol){
    m_configManager->setProtocol((CStreamSettings::Protocol)protocol);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setProtocol((CStreamSettings::Protocol)protocol);
}

auto CBoard::getDecimation() -> int{
    auto dec = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getDecimation();
    m_configManager->setDecimation(dec);
    return dec;
}

void CBoard::setDecimation(int dec){
    m_configManager->setDecimation(dec);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setDecimation(dec);
}

auto CBoard::getSampleLimit() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getSamples();
    m_configManager->setSamples(x);
    return x;
}

void CBoard::setSampleLimit(int s){
    m_configManager->setSamples(s);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setSamples(s);
}

auto CBoard::getResolution() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getResolution();
    m_configManager->setResolution(x);
    return x;
}

void CBoard::setResolution(int r){
    m_configManager->setResolution((CStreamSettings::Resolution)r);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setResolution((CStreamSettings::Resolution)r);
}

auto CBoard::getAttenuator() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getAttenuator();
    m_configManager->setAttenuator(x);
    return x;
}

void CBoard::setAttenuator(int a){
    m_configManager->setAttenuator((CStreamSettings::Attenuator)a);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setAttenuator((CStreamSettings::Attenuator)a);
}

auto CBoard::getCalibration() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getCalibration();
    m_configManager->setCalibration(x);
    return x;
}

void CBoard::setCalibration(int a){
    m_configManager->setCalibration(a);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setCalibration(a);
}

auto CBoard::getChannels() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getChannels();
    m_configManager->setChannels(x);
    return x;
}

void CBoard::setChannels(int x){
    m_configManager->setChannels((CStreamSettings::Channel)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setChannels((CStreamSettings::Channel)x);
}

auto CBoard::getDataFormat() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getFormat();
    m_configManager->setFormat(x);
    return x;
}

void CBoard::setDataFormat(int x){
    m_configManager->setFormat((CStreamSettings::DataFormat)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setFormat((CStreamSettings::DataFormat)x);
}

auto CBoard::getSaveMode() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getType();
    m_configManager->setType(x);
    return x;
}

void CBoard::setSaveMode(int x){
    m_configManager->setType((CStreamSettings::DataType)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setType((CStreamSettings::DataType)x);
}

auto CBoard::getCoupling() -> int{
    auto x = m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->getAC_DC();
    m_configManager->setAC_DC(x);
    return x;
}

void CBoard::setCoupling(int x){
    m_configManager->setAC_DC((CStreamSettings::AC_DC)x);
    m_configManager->getLocalSettingsOfHost(m_ip.toStdString())->setAC_DC((CStreamSettings::AC_DC)x);
}

void CBoard::startStreaming(QDateTime time){
    m_streamingDT = time;
    m_configManager->sendStart(m_ip.toStdString());
}

void CBoard::stopStreaming(){
    m_asionet = nullptr;
    m_net_buffer = nullptr;
    m_file_manager = nullptr;
    m_configManager->sendStop(m_ip.toStdString());
}

auto CBoard::getIsADCStarted() -> bool{
    return m_isADCStarted;
}

QString CBoard::getRecivedBytes(){
    return QString::fromStdString(convertBtoS(m_stat.bytes));
}

QString CBoard::getBandwidth(){
    return QString::fromStdString(m_stat.bw);
}

QString CBoard::getSamplesCH1(){
    return QString::number(m_stat.samples1);
}

QString CBoard::getSamplesCH2(){
    return QString::number(m_stat.samples2);
}

QString CBoard::getLostCount(){
    return QString::number(m_stat.lost);
}

QString CBoard::getFLostCount(){
    return QString::number(m_stat.flost);
}

QString CBoard::getSaveFileName(){
    if (m_file_manager){
        return QString::fromStdString(m_file_manager->getCSVFileName());
    }
    return "";
}



