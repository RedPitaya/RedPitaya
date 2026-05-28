#include <filesystem>

// Platform/Hardware includes
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"
#include "api250-12/rp-spi.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"

// Common includes
#include "common/version.h"
#include "main.h"
#include "web/rp_client.h"

// Streaming/Config includes
#include "config_net_lib/server_net_config_manager.h"
#include "dac_streaming_lib/dac_streaming_application.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "data_lib/buffers_cached.h"
#include "streaming_lib/streaming_file.h"
#include "streaming_lib/streaming_fpga.h"
#include "streaming_lib/streaming_net.h"
#include "uio_lib/generator.h"
#include "uio_lib/memory_manager.h"
#include "uio_lib/oscilloscope.h"

// ============================================================================
// Forward declarations
// ============================================================================

bool startServer(bool testMode);
void stopServer(ServerNetConfigManager::EStopReason x);
void stopNonBlocking(ServerNetConfigManager::EStopReason x);

auto startDACServer(bool testMode, dac_channels_t) -> bool;
auto stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void;
auto stopDACServer(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void;
void startADC();

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getModel() -> uint8_t;
auto getModelS() -> std::string;
auto getAC_DC() -> bool;
auto getIsDACGain() -> bool;

void setConfig(bool _force);
void updateUI();

// ============================================================================
// Global variables
// ============================================================================

static std::mutex g_adc_mutex;
static std::mutex g_dac_mutex;
static constexpr char config_file[] = "/root/.config/redpitaya/apps/streaming/streaming_config.json";

auto memmanager = uio_lib::CMemoryManager::instance();

// ============================================================================
// ADC Parameters
// ============================================================================

CBooleanParameter ss_start("SS_START", CBaseParameter::RW, false, 0);
CBooleanParameter ss_use_localfile("SS_USE_FILE", CBaseParameter::RW, false, 0);
CIntParameter ss_is_master("SS_IS_MASTER", CBaseParameter::RO, 0, 0, 0, 2);
CStringParameter ss_ip_addr("SS_IP_ADDR", CBaseParameter::RW, "127.0.0.1", 0);
CIntParameter ss_samples("SS_SAMPLES", CBaseParameter::RW, 20000000, 0, 0, 2000000000);
CIntParameter ss_channels("SS_CHANNEL", CBaseParameter::RW, 0, 0, 0, 256);
CIntParameter ss_resolution("SS_RESOLUTION", CBaseParameter::RW, 0, 0, 0, 256);
CIntParameter ss_calib("SS_USE_CALIB", CBaseParameter::RW, 1, 0, 0, 1);
CIntParameter ss_save_mode("SS_SAVE_MODE", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter ss_save_capture_time("SS_CAPTURE_TIME", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter ss_rate("SS_RATE", CBaseParameter::RW, 4, 0, 1, 65536);
CIntParameter ss_format("SS_FORMAT", CBaseParameter::RW, 0, 0, 0, 2);
CIntParameter ss_status("SS_STATUS", CBaseParameter::RW, 1, 0, 0, 100);
CBooleanParameter ss_adc_data_pass("SS_ADC_DATA_PASS", CBaseParameter::RW, false, 0);
CIntParameter ss_acd_max("SS_ACD_MAX", CBaseParameter::RO, getADCRate(), 0, getADCRate(), getADCRate());
CIntParameter ss_attenuator("SS_ATTENUATOR", CBaseParameter::RW, 0, 0, 0, 256);
CIntParameter ss_ac_dc("SS_AC_DC", CBaseParameter::RW, 0, 0, 0, 256);

// ============================================================================
// DAC Parameters
// ============================================================================

CBooleanParameter ss_dac_start("SS_DAC_START", CBaseParameter::RW, false, 0);
CBooleanParameter ss_dac_is_enable("SS_DAC_IS_ENABLE", CBaseParameter::RO, isDACAviable(), 0);
CStringParameter ss_dac_file("SS_DAC_FILE", CBaseParameter::RW, "", 0);
CIntParameter ss_dac_file_ctr("SS_DAC_FILE_CTR", CBaseParameter::RW, 0, 0, 0, 100);
CIntParameter ss_dac_status("SS_DAC_STATUS", CBaseParameter::RW, 1, 0, 0, 100);
CBooleanParameter ss_dac_data_pass("SS_DAC_DATA_PASS", CBaseParameter::RW, false, 0);
CIntParameter ss_dac_file_type("SS_DAC_FILE_TYPE", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter ss_dac_gain("SS_DAC_GAIN", CBaseParameter::RW, 0, 0, 0, 256);
CIntParameter ss_dac_mode("SS_DAC_MODE", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter ss_dac_speed("SS_DAC_HZ", CBaseParameter::RW, getDACRate(), 0, 1.0 / (65536.0 / getDACRate()) + 1.0, getDACRate());
CIntParameter ss_dac_repeat("SS_DAC_REPEAT", CBaseParameter::RW, -1, 0, -2, 0);
CIntParameter ss_dac_rep_count("SS_DAC_REPEAT_COUNT", CBaseParameter::RW, 1, 0, 1, 2000000000);

// ============================================================================
// Read-only capability parameters
// ============================================================================

CIntParameter ss_adc_channels("SS_ADC_CHANNELS", CBaseParameter::RO, getADCChannels(), 0, 0, 10);
CIntParameter ss_dac_channels("SS_DAC_CHANNELS", CBaseParameter::RO, getDACChannels(), 0, 0, 10);
CBooleanParameter ss_ac_dc_enable("SS_IS_AC_DC", CBaseParameter::RO, getAC_DC(), 0);
CBooleanParameter ss_dac_gain_enable("SS_IS_DAC_GAIN", CBaseParameter::RO, getIsDACGain(), 0);

// ============================================================================
// Statistics parameters
// ============================================================================

CDoubleParameter ss_pass_samples("SS_PASS_SAMPLES", CBaseParameter::RW, 0, 0, 0, 1e20);
CDoubleParameter ss_writed_size("SS_WRITED_SIZE", CBaseParameter::RW, 0, 0, 0, 1e20);

// ============================================================================
// Memory parameters
// ============================================================================

CIntParameter mem_block_size("MM_BLOCK_SIZE", CBaseParameter::RW, 0, 0, 0, memmanager->getMaxBlockSize());
CIntParameter mem_adc_size("MM_ADC_SIZE", CBaseParameter::RW, 0, 0, 0, memmanager->getRAMSize());
CIntParameter mem_dac_size("MM_DAC_SIZE", CBaseParameter::RW, 0, 0, 0, memmanager->getRAMSize());
CIntParameter mem_gpio_size("MM_GPIO_SIZE", CBaseParameter::RW, 0, 0, 0, memmanager->getRAMSize());

CBooleanParameter mem_adc_size_valid("MM_ADC_VALID", CBaseParameter::RW, false, 0);
CBooleanParameter mem_dac_size_valid("MM_DAC_VALID", CBaseParameter::RW, false, 0);
CBooleanParameter mem_gpio_size_valid("MM_GPIO_VALID", CBaseParameter::RW, false, 0);

// ============================================================================
// GPIO Parameters
// ============================================================================

CBooleanParameter ss_gpio_start("SS_GPIO_START", CBaseParameter::RW, false, 0);
CIntParameter ss_gpio_status("SS_GPIO_STATUS", CBaseParameter::RW, 1, 0, 0, 100);
CBooleanParameter ss_gpio_data_pass("SS_GPIO_DATA_PASS", CBaseParameter::RW, false, 0);

// ============================================================================
// Global component pointers
// ============================================================================

uio_lib::COscilloscope::Ptr g_osc = nullptr;
uio_lib::CGenerator::Ptr g_gen = nullptr;

streaming_lib::CStreamingFPGA::Ptr g_s_fpga = nullptr;
streaming_lib::CStreamingNet::Ptr g_s_net = nullptr;
streaming_lib::CStreamingFile::Ptr g_s_file = nullptr;

DataLib::CBuffersCached::Ptr g_s_buffer = nullptr;

dac_streaming_lib::CDACStreamingApplication::Ptr g_dac_app = nullptr;
dac_streaming_lib::CDACStreamingManager::Ptr g_dac_manger = nullptr;
ServerNetConfigManager::Ptr g_serverNetConfig = nullptr;

uio_lib::BoardMode g_isMaster = uio_lib::BoardMode::UNKNOWN;

std::atomic_bool g_serverRun(false);
std::atomic_bool g_dac_serverRun(false);

// ============================================================================
// Hardware detection functions
// ============================================================================

auto isDACAviable() -> bool {
    auto uioList = uio_lib::GetUioList();
    for (auto uio : uioList) {
        if (uio.nodeName == "rp_dac@40100000") {
            return true;
        }
    }
    return false;
}

auto getADCChannels() -> uint8_t {
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC channels count");
    }
    return c;
}

auto getDACChannels() -> uint8_t {
    uint8_t c = 0;
    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC channels count");
    }
    return c;
}

auto getAC_DC() -> bool {
    bool c = false;
    if (rp_HPGetFastADCIsAC_DC(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast AC/DC mode");
    }
    return c;
}

auto getIsDACGain() -> bool {
    bool c = false;
    if (rp_HPGetIsGainDACx5(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC gain mode");
    }
    return c;
}

auto getDACRate() -> uint32_t {
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast DAC speed");
        return 1;
    }
    return c;
}

auto getADCRate() -> uint32_t {
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC speed");
    }
    return c;
}

auto getADCBits() -> uint8_t {
    uint8_t c = 0;
    if (rp_HPGetFastADCBits(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get fast ADC bits");
    }
    return c;
}

auto getIsXStreaming() -> bool {
    bool c = false;
    if (rp_HPGetIsXStreamingAvailable(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get xstreaming mode");
    }
    return c;
}

auto getModel() -> uint8_t {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }
    return (uint8_t)c;
}

auto getMACAddress() -> std::string {
    char* address = nullptr;
    if (rp_HPGetModelETH_MAC_Address(&address) != RP_HP_OK) {
        FATAL("Can't get MAC address")
    }
    return std::string(address);
}

// ============================================================================
// Application description & signal handling
// ============================================================================

const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya Stream server application.\n";
}

auto termSignalHandler(int) -> void {}

auto installTermSignalHandler() -> void {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = termSignalHandler;
    sigaction(SIGINT, &action, NULL);
}

// ============================================================================
// Application initialization
// ============================================================================

auto rp_app_init(void) -> int {
    fprintf(stderr, "Loading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
    installTermSignalHandler();

#ifdef ZIP_DISABLED
    CDataManager::GetInstance()->SetEnableParamsGZip(false);
    CDataManager::GetInstance()->SetEnableSignalsGZip(false);
    CDataManager::GetInstance()->SetEnableBinarySignalsGZip(false);
#endif

    CDataManager::GetInstance()->SetParamInterval(100);
    g_serverRun = false;

    try {
        // Detect master/slave mode
        try {
            auto uioList = uio_lib::GetUioList();
            for (auto& uio : uioList) {
                if (uio.nodeName == "rp_oscilloscope@40000000") {
                    WARNING("Check master/slave");
                    auto osc = uio_lib::COscilloscope::create(uio, 1, true, getADCRate(), false, getADCBits(), getADCChannels());
                    g_isMaster = osc->isMaster();

                    if (getIsXStreaming() == false) {
                        g_isMaster = uio_lib::BoardMode::MASTER;
                    }

                    WARNING("Detected %s mode", g_isMaster == uio_lib::BoardMode::MASTER ? "Master" : (g_isMaster == uio_lib::BoardMode::SLAVE ? "Slave" : "Unknown"));
                    break;
                }
            }

            // Initialize network config manager
            TRACE("ss_ip_addr %s", ss_ip_addr.Value().c_str());
            g_serverNetConfig =
                std::make_shared<ServerNetConfigManager>(config_file,
                                                         g_isMaster != uio_lib::BoardMode::SLAVE ? broadcast_lib::EMode::AB_SERVER_MASTER : broadcast_lib::EMode::AB_SERVER_SLAVE,
                                                         ss_ip_addr.Value(),
                                                         NET_CONFIG_PORT,
                                                         getADCChannels());

            // Connect signals
            g_serverNetConfig->getNewSettingsNofiy.connect([]() { updateUI(); });
            g_serverNetConfig->startStreamingNofiy.connect([]() { startServer(false); });
            g_serverNetConfig->stopStreamingNofiy.connect([]() { stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL); });
            g_serverNetConfig->startDacStreamingNofiy.connect([](auto channels) { startDACServer(false, channels); });
            g_serverNetConfig->stopDacStreamingNofiy.connect([]() { stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_STOP); });
            g_serverNetConfig->startADCNofiy.connect([]() { startADC(); });
            g_serverNetConfig->memoryBlockSizeChangeNofiy.connect([]() {
                stopServer(ServerNetConfigManager::EStopReason::MEM_MODIFY);
                stopDACServer(dac_streaming_lib::CDACStreamingManager::NR_MEM_MODIFY);
                uio_lib::CMemoryManager::instance()->setMemoryBlockSize(g_serverNetConfig->getSettings().getMemoryBlockSize());
                uio_lib::CMemoryManager::instance()->reallocateBlocks();
            });

            g_serverNetConfig->startBroadcast(getModel(), getMACAddress(), ss_ip_addr.Value(), NET_BROADCAST_PORT);
        } catch (std::exception& e) {
            WARNING("Init ServerNetConfigManager() %s\n", e.what());
        }

        // Set initial parameter values
        ss_is_master.SendValue(g_isMaster);
        ss_status.SendValue(0);
        ss_adc_data_pass.SendValue(0);
        ss_dac_status.SendValue(0);
        ss_dac_data_pass.SendValue(0);
        ss_gpio_status.SendValue(0);
        ss_gpio_data_pass.SendValue(0);
        updateUI();

        // Initialize file directory and hardware
        streaming_lib::CStreamingFile::makeEmptyDir(FILE_PATH);
        if (rp_HPGetIsAttenuatorControllerPresentOrDefault() && rp_HPGetFastADCIsAC_DCOrDefault() && rp_HPGetIsGainDACx5OrDefault()) {
            rp_max7311::rp_initController();
        }

        rp_WC_Init();
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what());
    }

    return 0;
}

// ============================================================================
// Application exit & cleanup
// ============================================================================

void stopAll() {
    stopServer(ServerNetConfigManager::EStopReason::MEM_MODIFY);
    stopDACServer(dac_streaming_lib::CDACStreamingManager::NotifyResult::NR_MEM_MODIFY);
}

auto rp_app_exit(void) -> int {
    g_serverNetConfig->stop();
    stopServer(ServerNetConfigManager::EStopReason::NORMAL);
    stopDACServer(dac_streaming_lib::CDACStreamingManager::NR_STOP);
    aprintf(stderr, "Unloading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

// ============================================================================
// Configuration management
// ============================================================================

auto saveConfigInFile() -> void {
    if (!g_serverNetConfig->getSettingsRef().writeToFile(config_file)) {
        ERROR_LOG("Error save to file (%s)", config_file);
    }
}

void updateUI() {
    // Update DAC file list
    auto fileName = g_serverNetConfig->getSettingsRef().getDACFile();
    auto files = CStreamSettings::getDACFiles();
    ss_dac_file.SendValue(fileName + "*" + files);

    // Update ADC settings
    ss_use_localfile.SendValue(g_serverNetConfig->getSettingsRef().getADCPassMode());

    int ch = 0, att = 0, ac_dc = 0;
    for (int i = 1; i <= 4; i++) {
        ch |= g_serverNetConfig->getSettingsRef().getADCChannels(i) ? 1 << (i - 1) : 0;
        att |= g_serverNetConfig->getSettingsRef().getADCAttenuator(i) ? 1 << (i - 1) : 0;
        ac_dc |= g_serverNetConfig->getSettingsRef().getADCAC_DC(i) ? 1 << (i - 1) : 0;
    }

    ss_channels.SendValue(ch);
    ss_resolution.SendValue(g_serverNetConfig->getSettingsRef().getADCResolution());
    ss_save_mode.SendValue(g_serverNetConfig->getSettingsRef().getADCType());
    ss_save_capture_time.SendValue(g_serverNetConfig->getSettingsRef().getADCCaptureTime());
    ss_format.SendValue(g_serverNetConfig->getSettingsRef().getADCFormat());
    ss_attenuator.SendValue(att);
    ss_ac_dc.SendValue(ac_dc);

    // Update DAC settings
    int dac_gain = 0;
    for (int i = 1; i <= 2; i++) {
        dac_gain |= g_serverNetConfig->getSettingsRef().getDACGain(i) ? 1 << (1 - i) : 0;
    }

    ss_dac_file_type.SendValue(g_serverNetConfig->getSettingsRef().getDACFileType());
    ss_dac_gain.SendValue(dac_gain);
    ss_dac_mode.SendValue(g_serverNetConfig->getSettingsRef().getDACPassMode());
    ss_dac_repeat.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeat());
    ss_dac_rep_count.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeatCount());
    ss_dac_speed.SendValue(g_serverNetConfig->getSettingsRef().getDACSpeed());

    // Update common settings
    ss_rate.SendValue(g_serverNetConfig->getSettingsRef().getADCDecimation());
    ss_samples.SendValue(g_serverNetConfig->getSettingsRef().getADCSamples());
    ss_calib.SendValue(g_serverNetConfig->getSettingsRef().getADCCalibration());

    // Update memory settings
    int new_block_size = g_serverNetConfig->getSettingsRef().getMemoryBlockSize();
    if (mem_block_size.Value() != new_block_size) {
        stopAll();
        memmanager->setMemoryBlockSize(new_block_size);
        memmanager->reallocateBlocks();
    }

    mem_block_size.SendValue(new_block_size);
    mem_adc_size.SendValue(g_serverNetConfig->getSettingsRef().getADCSize());
    memmanager->setReserverdMemory(uio_lib::MM_ADC, g_serverNetConfig->getSettingsRef().getADCSize());
    mem_dac_size.SendValue(g_serverNetConfig->getSettingsRef().getDACSize());
    memmanager->setReserverdMemory(uio_lib::MM_DAC, g_serverNetConfig->getSettingsRef().getDACSize());
    mem_gpio_size.SendValue(g_serverNetConfig->getSettingsRef().getGPIOSize());
    memmanager->setReserverdMemory(uio_lib::MM_GPIO, g_serverNetConfig->getSettingsRef().getGPIOSize());

    mem_adc_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_ADC));
    mem_dac_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_DAC));
    mem_gpio_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_GPIO));
}

// ============================================================================
// Parameter update handlers
// ============================================================================

void setConfig(bool _force) {
    bool needUpdate = false;

    // DAC file management
    if (ss_dac_file.IsNewValue() || _force) {
        ss_dac_file.Update();
        g_serverNetConfig->getSettingsRef().setDACFile(ss_dac_file.Value());
        auto fileName = g_serverNetConfig->getSettingsRef().getDACFile();
        auto files = CStreamSettings::getDACFiles();
        ss_dac_file.SendValue(fileName + "*" + files);
        needUpdate = true;
    }

    if (ss_dac_file_ctr.IsNewValue()) {
        ss_dac_file_ctr.Update();
        if (ss_dac_file_ctr.Value() == 1) {
            // Refresh file list
            auto fileName = g_serverNetConfig->getSettingsRef().getDACFile();
            auto files = CStreamSettings::getDACFiles();
            ss_dac_file.SendValue(fileName + "*" + files);
        }
        if (ss_dac_file_ctr.Value() == 2) {
            // Delete file
            auto path = CStreamSettings::getDACDirPath();
            auto file = g_serverNetConfig->getSettingsRef().getDACFile();
            try {
                std::filesystem::remove(path + "/" + file);
            } catch (const std::exception& e) {}
            g_serverNetConfig->getSettingsRef().setDACFile("");
            auto fileName = g_serverNetConfig->getSettingsRef().getDACFile();
            auto files = CStreamSettings::getDACFiles();
            ss_dac_file.SendValue(fileName + "*" + files);
        }
        ss_dac_file_ctr.Value() = 0;
    }

    // Network settings
    if (ss_ip_addr.IsNewValue() || _force) {
        ss_ip_addr.Update();
        g_serverNetConfig->setBroadcastAddress(ss_ip_addr.Value());
    }

    // ADC settings
    if (ss_use_localfile.IsNewValue() || _force) {
        ss_use_localfile.Update();
        g_serverNetConfig->getSettingsRef().setADCPassMode(ss_use_localfile.Value() ? CStreamSettings::PassMode::FILE : CStreamSettings::PassMode::NET);
        needUpdate = true;
    }

    if (ss_channels.IsNewValue() || _force) {
        ss_channels.Update();
        int value = ss_channels.Value();
        for (int i = 1; i <= 4; i++) {
            g_serverNetConfig->getSettingsRef().setADCChannels(i, (value & (1 << (i - 1))) ? CStreamSettings::State::ON : CStreamSettings::State::OFF);
        }
        needUpdate = true;
    }

    if (ss_resolution.IsNewValue() || _force) {
        ss_resolution.Update();
        g_serverNetConfig->getSettingsRef().setADCResolution((CStreamSettings::Resolution::_enumerated)ss_resolution.Value());
        needUpdate = true;
    }

    if (ss_save_mode.IsNewValue() || _force) {
        ss_save_mode.Update();
        g_serverNetConfig->getSettingsRef().setADCType((CStreamSettings::DataType::_enumerated)ss_save_mode.Value());
        needUpdate = true;
    }

    if (ss_save_capture_time.IsNewValue() || _force) {
        ss_save_capture_time.Update();
        g_serverNetConfig->getSettingsRef().setADCCaptureTime((CStreamSettings::ADCCaptureTime::_enumerated)ss_save_capture_time.Value());
        needUpdate = true;
    }

    if (ss_rate.IsNewValue() || _force) {
        ss_rate.Update();
        g_serverNetConfig->getSettingsRef().setADCDecimation(ss_rate.Value());
        needUpdate = true;
    }

    if (ss_format.IsNewValue() || _force) {
        ss_format.Update();
        g_serverNetConfig->getSettingsRef().setADCFormat((CStreamSettings::DataFormat::_enumerated)ss_format.Value());
        needUpdate = true;
    }

    if (ss_samples.IsNewValue() || _force) {
        ss_samples.Update();
        g_serverNetConfig->getSettingsRef().setADCSamples(ss_samples.Value());
        needUpdate = true;
    }

    if (ss_calib.IsNewValue() || _force) {
        ss_calib.Update();
        g_serverNetConfig->getSettingsRef().setADCCalibration((CStreamSettings::State::_enumerated)ss_calib.Value());
        needUpdate = true;
    }

    // Hardware-specific ADC settings
    if (rp_HPGetFastADCIsLV_HVOrDefault()) {
        if (ss_attenuator.IsNewValue() || _force) {
            ss_attenuator.Update();
            int value = ss_attenuator.Value();
            for (int i = 1; i <= 4; i++) {
                g_serverNetConfig->getSettingsRef().setADCAttenuator(i, (value & (1 << (i - 1))) ? CStreamSettings::Attenuator::A_1_20 : CStreamSettings::Attenuator::A_1_1);
            }
            needUpdate = true;
        }
    }

    if (rp_HPGetFastADCIsAC_DCOrDefault()) {
        if (ss_ac_dc.IsNewValue() || _force) {
            ss_ac_dc.Update();
            int value = ss_ac_dc.Value();
            for (int i = 1; i <= 4; i++) {
                g_serverNetConfig->getSettingsRef().setADCAC_DC(i, (value & (1 << (i - 1))) ? CStreamSettings::AC_DC::AC : CStreamSettings::AC_DC::DC);
            }
            needUpdate = true;
        }
    }

    // DAC settings
    if (ss_dac_file_type.IsNewValue() || _force) {
        ss_dac_file_type.Update();
        g_serverNetConfig->getSettingsRef().setDACFileType((CStreamSettings::DataFormat::_enumerated)ss_dac_file_type.Value());
        needUpdate = true;
    }

    if (rp_HPGetIsGainDACx5OrDefault()) {
        if (ss_dac_gain.IsNewValue() || _force) {
            ss_dac_gain.Update();
            int value = ss_dac_gain.Value();
            for (int i = 1; i <= 2; i++) {
                g_serverNetConfig->getSettingsRef().setDACGain(i, (value & (1 << (i - 1))) ? CStreamSettings::DACGain::X5 : CStreamSettings::DACGain::X1);
            }
            needUpdate = true;
        }
    }

    if (ss_dac_mode.IsNewValue() || _force) {
        ss_dac_mode.Update();
        g_serverNetConfig->getSettingsRef().setDACPassMode((CStreamSettings::DACPassMode::_enumerated)ss_dac_mode.Value());
        needUpdate = true;
    }

    if (ss_dac_repeat.IsNewValue() || _force) {
        ss_dac_repeat.Update();
        g_serverNetConfig->getSettingsRef().setDACRepeat((CStreamSettings::DACRepeat::_enumerated)ss_dac_repeat.Value());
        needUpdate = true;
    }

    if (ss_dac_rep_count.IsNewValue() || _force) {
        ss_dac_rep_count.Update();
        g_serverNetConfig->getSettingsRef().setDACRepeatCount(ss_dac_rep_count.Value());
        needUpdate = true;
    }

    if (ss_dac_speed.IsNewValue() || _force) {
        ss_dac_speed.Update();
        g_serverNetConfig->getSettingsRef().setDACSpeed(ss_dac_speed.Value());
        needUpdate = true;
    }

    // Memory settings
    if (mem_block_size.IsNewValue() || _force) {
        mem_block_size.Update();
        int block = g_serverNetConfig->getSettingsRef().getMemoryBlockSize();
        g_serverNetConfig->getSettingsRef().setMemoryBlockSize(mem_block_size.Value());
        if (mem_block_size.Value() != block) {
            stopAll();
            memmanager->setMemoryBlockSize(mem_block_size.Value());
            memmanager->reallocateBlocks();
            mem_adc_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_ADC));
            mem_dac_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_DAC));
            mem_gpio_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_GPIO));
        }
        needUpdate = true;
    }

    if (mem_adc_size.IsNewValue() || _force) {
        mem_adc_size.Update();
        g_serverNetConfig->getSettingsRef().setADCSize(mem_adc_size.Value());
        memmanager->setReserverdMemory(uio_lib::MM_ADC, mem_adc_size.Value());
        mem_adc_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_ADC));
        needUpdate = true;
    }

    if (mem_dac_size.IsNewValue() || _force) {
        mem_dac_size.Update();
        g_serverNetConfig->getSettingsRef().setDACSize(mem_dac_size.Value());
        memmanager->setReserverdMemory(uio_lib::MM_DAC, mem_dac_size.Value());
        mem_dac_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_DAC));
        needUpdate = true;
    }

    if (mem_gpio_size.IsNewValue() || _force) {
        mem_gpio_size.Update();
        g_serverNetConfig->getSettingsRef().setGPIOSize(mem_gpio_size.Value());
        memmanager->setReserverdMemory(uio_lib::MM_GPIO, mem_gpio_size.Value());
        mem_gpio_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_GPIO));
        needUpdate = true;
    }

    if (needUpdate) {
        saveConfigInFile();
    }
}

void UpdateParams(void) {}

void UpdateParamsFromWeb(void) {
    try {
        setConfig(false);

        // Handle start/stop commands
        if (ss_start.IsNewValue()) {
            ss_start.Update();
            if (ss_start.Value() == 1) {
                if (startServer(false)) {
                    startADC();
                }
            } else {
                stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
            }
        }

        if (ss_dac_start.IsNewValue()) {
            ss_dac_start.Update();
            if (ss_dac_start.Value() == 1) {
                startDACServer(false, dac_channels_t());
            } else {
                WARNING("Stop command");
                stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_STOP);
            }
        }

        if (ss_gpio_start.IsNewValue()) {
            ss_gpio_start.Update();
            // GPIO functionality not implemented yet
        }

        // Update statistics
        if (g_s_file) {
            ss_pass_samples.SendValue(g_s_file->getPassSamples());
            ss_writed_size.SendValue(g_s_file->getWritedSize());
        }
    } catch (std::exception& e) {
        FATAL("%s", e.what());
    }
}

void OnNewParams(void) {
    UpdateParamsFromWeb();
}

// ============================================================================
// ADC Server functions
// ============================================================================

bool startServer(bool testMode) {
    try {
        std::lock_guard guard(g_adc_mutex);

        // Reset state
        g_s_fpga = nullptr;
        g_s_buffer = nullptr;
        g_osc = nullptr;
        g_s_file = nullptr;
        g_s_net = nullptr;

        CStreamSettings settings = g_serverNetConfig->getSettings();

        // Extract settings
        auto resolution = settings.getADCResolution();
        auto format = settings.getADCFormat();
        auto sock_port = NET_ADC_STREAMING_PORT;
        auto use_file = settings.getADCPassMode();
        auto rate = settings.getADCDecimation();
        auto ip_addr_host = ss_ip_addr.Value();
        auto samples = settings.getADCSamples();
        auto save_mode = settings.getADCType();
        auto save_capture_time = settings.getADCCaptureTime();
        auto use_calib = settings.getADCCalibration();
        auto max_channels = getADCChannels();

        // Initialize calibration
        if (rp_CalibInit() != RP_HW_CALIB_OK) {
            FATAL("Error init calibration");
        }

        auto uioList = uio_lib::GetUioList();
        int32_t ch_off[4] = {0, 0, 0, 0};
        double ch_gain[4] = {1, 1, 1, 1};
        bool isNewCalib = false;
        bool filterBypass = !rp_HPGetFastADCIsFilterPresentOrDefault();
        uint32_t aa_ch[4] = {0, 0, 0, 0};
        uint32_t bb_ch[4] = {0, 0, 0, 0};
        uint32_t kk_ch[4] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};
        uint32_t pp_ch[4] = {0, 0, 0, 0};

        // Load calibration data
        if (use_calib) {
            uint8_t cver = 0;
            if (rp_GetCalibrationVersion(&cver) == RP_HW_CALIB_OK) {
                isNewCalib = cver >= RP_HW_PACK_ID_V6;
            } else {
                ERROR_LOG("Error get calibration version");
            }

            for (uint8_t ch = 0; ch < max_channels; ++ch) {
                rp_acq_ac_dc_mode_calib_t mode = (settings.getADCAC_DC(ch + 1).value == CStreamSettings::AC_DC::DC) ? RP_DC_CALIB : RP_AC_CALIB;
                bool isAttenuator1_1 = (settings.getADCAttenuator(ch + 1).value == CStreamSettings::Attenuator::A_1_1);

                if (isAttenuator1_1) {
                    if (rp_CalibGetFastADCCalibValue((rp_channel_calib_t)ch, mode, &ch_gain[ch], &ch_off[ch]) != RP_HW_CALIB_OK) {
                        ERROR_LOG("Error get calibration channel: %d", ch);
                    }
                    if (!filterBypass) {
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter((rp_channel_calib_t)ch, &f) == RP_HW_CALIB_OK) {
                            aa_ch[ch] = f.aa;
                            bb_ch[ch] = f.bb;
                            kk_ch[ch] = f.kk;
                            pp_ch[ch] = f.pp;
                        } else {
                            ERROR_LOG("Error get filter value: %d", ch);
                        }
                    }
                } else {
                    if (rp_CalibGetFastADCCalibValue_1_20((rp_channel_calib_t)ch, mode, &ch_gain[ch], &ch_off[ch]) != RP_HW_CALIB_OK) {
                        ERROR_LOG("Error get calibration channel: %d", ch);
                    }
                    if (!filterBypass) {
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter_1_20((rp_channel_calib_t)ch, &f) == RP_HW_CALIB_OK) {
                            aa_ch[ch] = f.aa;
                            bb_ch[ch] = f.bb;
                            kk_ch[ch] = f.kk;
                            pp_ch[ch] = f.pp;
                        } else {
                            ERROR_LOG("Error get filter value: %d", ch);
                        }
                    }
                }
            }
        }

        // Configure hardware
        if (rp_HPGetIsAttenuatorControllerPresentOrDefault()) {
            rp_max7311::rp_setAttenuator(RP_MAX7311_IN1, settings.getADCAttenuator(1).value == CStreamSettings::Attenuator::A_1_20 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
            rp_max7311::rp_setAttenuator(RP_MAX7311_IN2, settings.getADCAttenuator(2).value == CStreamSettings::Attenuator::A_1_20 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
        }

        if (rp_HPGetFastADCIsAC_DCOrDefault()) {
            rp_max7311::rp_setAC_DC(RP_MAX7311_IN1, settings.getADCAC_DC(1).value == CStreamSettings::AC_DC::DC ? RP_DC_MODE : RP_AC_MODE);
            rp_max7311::rp_setAC_DC(RP_MAX7311_IN2, settings.getADCAC_DC(2).value == CStreamSettings::AC_DC::DC ? RP_DC_MODE : RP_AC_MODE);
        }

        // Create oscilloscope
        for (auto& uio : uioList) {
            if (uio.nodeName == "rp_oscilloscope@40000000") {
                memmanager->releaseMemory(uio_lib::MM_ADC_RESERVE_SKIP);
                auto reserved = memmanager->reserveMemory(uio_lib::MM_ADC_RESERVE_SKIP, 1, 1);
                if (reserved != 1) {
                    TRACE_SHORT("Reserved: %d", reserved);
                    ERROR_LOG("Can't reserve memory via memory manager");
                    stopNonBlocking(ServerNetConfigManager::EStopReason::MEM_ERROR);
                    return false;
                }
                auto blocks = memmanager->getRegions(uio_lib::MM_ADC_RESERVE_SKIP);

                TRACE("COscilloscope::Create rate %d", rate);
                g_osc = uio_lib::COscilloscope::create(uio, rate, g_isMaster != uio_lib::BoardMode::SLAVE, getADCRate(), !filterBypass, getADCBits(), max_channels);
                for (uint8_t ch = 0; ch < max_channels; ++ch) {
                    g_osc->setCalibration(ch, ch_off[ch], ch_gain[ch], isNewCalib);
                    g_osc->setFilterCalibration(ch, aa_ch[ch], bb_ch[ch], kk_ch[ch], pp_ch[ch]);
                }
                g_osc->setFilterBypass(filterBypass);
                g_osc->set8BitMode(resolution.value == CStreamSettings::Resolution::BIT_8);
                g_osc->setSkipDataAddress(blocks[0]);
                break;
            }
        }

        // Create buffer cache
        g_s_buffer = DataLib::CBuffersCached::create();
        auto g_s_buffer_w = std::weak_ptr<DataLib::CBuffersCached>(g_s_buffer);

        // Create streaming handlers
        if (use_file.value == CStreamSettings::PassMode::NET) {
            g_s_net = streaming_lib::CStreamingNet::create(ip_addr_host, sock_port);
            g_s_net->getBuffer = [g_s_buffer_w]() -> DataLib::CDataBuffersPackDMA::Ptr {
                auto obj = g_s_buffer_w.lock();
                return obj ? obj->readBuffer() : nullptr;
            };
            g_s_net->unlockBufferF = [g_s_buffer_w]() {
                auto obj = g_s_buffer_w.lock();
                if (obj)
                    obj->unlockBufferRead();
            };
        }

        if (use_file.value == CStreamSettings::PassMode::FILE) {
            auto path = std::string(FILE_PATH);
            g_s_file = streaming_lib::CStreamingFile::create(format, path, samples, save_mode.value == CStreamSettings::DataType::VOLT, testMode, true);
            g_s_file->stopNotify.connect([](streaming_lib::CStreamingFile::EStopReason r) {
                switch (r) {
                    case streaming_lib::CStreamingFile::EStopReason::NORMAL:
                        stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
                        break;
                    case streaming_lib::CStreamingFile::EStopReason::OUT_SPACE:
                        stopNonBlocking(ServerNetConfigManager::EStopReason::SD_FULL);
                        break;
                    case streaming_lib::CStreamingFile::EStopReason::REACH_LIMIT:
                        stopNonBlocking(ServerNetConfigManager::EStopReason::DONE);
                        break;
                }
            });
        }

        // Create FPGA streaming
        g_s_fpga = std::make_shared<streaming_lib::CStreamingFPGA>(g_osc, save_capture_time);
        uint8_t bits = (resolution.value == CStreamSettings::Resolution::BIT_8 ? 8 : 16);
        TRACE_SHORT("Set channels resolution %d", bits);

        // Configure channels
        uint8_t channelsActive = 0;
        for (int i = 0; i < max_channels; i++) {
            if (settings.getADCChannels(i + 1).value == CStreamSettings::State::ON) {
                g_s_buffer->addChannel(
                    (DataLib::EDataBuffersPackChannel)i,
                    bits,
                    settings.getADCAttenuator(i + 1).value == CStreamSettings::Attenuator::A_1_20 ? DataLib::CDataBufferDMA::ATT_1_20 : DataLib::CDataBufferDMA::ATT_1_1);
                channelsActive++;
            }
        }

        // Allocate memory
        memmanager->releaseMemory(uio_lib::MM_ADC);
        auto mbSize = memmanager->getMemoryBlockSize();
        auto ramSize = memmanager->getReserverdMemory(uio_lib::MM_ADC);

        if (ramSize < memmanager->getMinRAMSize(uio_lib::MM_ADC)) {
            ERROR_LOG("Not enough memory for ADC mode");
            stopNonBlocking(ServerNetConfigManager::EStopReason::MEM_ERROR);
            return false;
        }

        auto freeblocks = ramSize / mbSize;
        if (testMode) {
            freeblocks = std::min(freeblocks, 100u);
        }
        auto reservedBlocks = memmanager->reserveMemory(uio_lib::MM_ADC, freeblocks, channelsActive);

        if (channelsActive == 0) {
            WARNING("No active channels");
            stopNonBlocking(ServerNetConfigManager::EStopReason::NO_CHANNELS);
            return false;
        }

        if (reservedBlocks == 0) {
            WARNING("Can't reserve memory via memory manager");
            stopNonBlocking(ServerNetConfigManager::EStopReason::MEM_ERROR);
            return false;
        }

        WARNING("Reserved memory blocks: %d size: %d", reservedBlocks, reservedBlocks * memmanager->getMemoryBlockSize());

        // Generate buffers
        g_s_buffer->generateBuffers(memmanager->getRegions(uio_lib::MM_ADC), DataLib::sizeHeader(), testMode);
        g_s_buffer->setADCBits(getADCBits());
        g_s_buffer->setOSCRate(getADCRate() / rate);
        if (use_file.value == CStreamSettings::PassMode::NET) {
            g_s_buffer->initHeadersADC();
        }
        g_osc->setDataSize(g_s_buffer->getDataSize());
        g_s_fpga->setTestMode(testMode);

        // Setup FPGA callbacks
        auto weak_obj = std::weak_ptr<DataLib::CBuffersCached>(g_s_buffer);
        g_s_fpga->getBuffF = [weak_obj]() -> DataLib::CDataBuffersPackDMA::Ptr {
            auto obj = weak_obj.lock();
            return obj ? obj->writeBuffer() : nullptr;
        };
        g_s_fpga->unlockBuffF = [weak_obj]() {
            auto obj = weak_obj.lock();
            if (obj)
                obj->unlockBufferWrite();
        };

        auto g_s_file_w = std::weak_ptr<streaming_lib::CStreamingFile>(g_s_file);
        g_s_fpga->oscNotify.connect([g_s_file_w, g_s_buffer_w](__attribute__((unused)) DataLib::CDataBuffersPackDMA::Ptr) {
            auto f_obj = g_s_file_w.lock();
            auto b_obj = g_s_buffer_w.lock();
            if (f_obj && b_obj) {
                auto p = b_obj->readBuffer();
                if (p) {
                    f_obj->passBuffers(p);
                    b_obj->unlockBufferRead();
                }
            }
        });

        // Generate timestamp filename
        char time_str[40];
        time_t now = time(nullptr);
        struct tm* timenow = gmtime(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
        std::string filenameDate = time_str;

        // Start streaming services
        if (g_s_net) {
            g_s_net->run();
            usleep(1000);
            g_serverNetConfig->sendADCServerStartedTCP();
        }

        if (g_s_file) {
            g_s_file->run(filenameDate);
            g_serverNetConfig->sendADCServerStartedSD();
        }

        ss_status.SendValue(1);
        TRACE("Start server");
        return true;
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what());
    }
    return false;
}

void startADC() {
    try {
        if (g_s_fpga) {
            TRACE("start");
            g_s_fpga->runNonBlock();
            ss_adc_data_pass.SendValue(1);
            g_serverNetConfig->sendADCStarted();
        }
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what());
    }
}

// ============================================================================
// ADC Server stop functions
// ============================================================================

void stopNonBlocking(ServerNetConfigManager::EStopReason x) {
    try {
        std::thread th(stopServer, x);
        th.detach();
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what());
    }
}

void stopServer(ServerNetConfigManager::EStopReason x) {
    try {
        std::lock_guard guard(g_adc_mutex);

        if (g_s_buffer)
            g_s_buffer->notifyToDestory();
        if (g_s_file)
            g_s_file->disableNotify();

        switch (x) {
            case ServerNetConfigManager::EStopReason::NORMAL:
                g_serverNetConfig->sendServerStopped();
                ss_status.SendValue(0);
                break;
            case ServerNetConfigManager::EStopReason::SD_FULL:
                g_serverNetConfig->sendServerStoppedSDFull();
                ss_status.SendValue(2);
                break;
            case ServerNetConfigManager::EStopReason::DONE:
                g_serverNetConfig->sendServerStoppedDone();
                ss_status.SendValue(3);
                break;
            case ServerNetConfigManager::EStopReason::MEM_ERROR:
                g_serverNetConfig->sendServerMemoryErrorStopped();
                ss_status.SendValue(4);
                break;
            case ServerNetConfigManager::EStopReason::MEM_MODIFY:
                g_serverNetConfig->sendServerMemoryModifyStopped();
                ss_status.SendValue(5);
                break;
            case ServerNetConfigManager::EStopReason::NO_CHANNELS:
                g_serverNetConfig->sendServerNoChannelsStopped();
                ss_status.SendValue(6);
                break;
            default:
                throw std::runtime_error("Unknown state");
        }

        if (g_s_fpga) {
            g_s_fpga->stop();
            g_s_fpga = nullptr;
        }

        g_s_buffer = nullptr;
        g_s_net = nullptr;
        g_s_file = nullptr;
        ss_adc_data_pass.SendValue(0);

        memmanager->releaseMemory(uio_lib::MM_ADC);
        memmanager->releaseMemory(uio_lib::MM_ADC_RESERVE_SKIP);
        TRACE("Stop server");
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what());
    }
}

// ============================================================================
// DAC Server functions
// ============================================================================

auto startDACServer(__attribute__((unused)) bool testMode, dac_channels_t activeChannels) -> bool {
    if (!g_serverNetConfig)
        return false;

    g_gen = nullptr;
    g_dac_app = nullptr;
    g_dac_manger = nullptr;

    try {
        std::lock_guard guard(g_dac_mutex);
        CStreamSettings settings = g_serverNetConfig->getSettings();

        auto use_file = settings.getDACPassMode();
        auto dac_speed = settings.getDACSpeed();
        auto ip_addr_host = "127.0.0.1";

        if (rp_CalibInit() != RP_HW_CALIB_OK) {
            ERROR_LOG("Error init calibration");
        }

        auto max_channels = getDACChannels();
        auto uioList = uio_lib::GetUioList();
        int32_t ch_off[2] = {0, 0};
        double ch_gain[2] = {1, 1};

        // Load DAC calibration
        for (uint8_t ch = 0; ch < max_channels && ch < 2; ++ch) {
            rp_gen_gain_calib_t mode = settings.getDACGain(ch + 1).value == CStreamSettings::DACGain::X1 ? RP_GAIN_CALIB_1X : RP_GAIN_CALIB_5X;
            if (rp_CalibGetFastDACCalibValue((rp_channel_calib_t)ch, mode, &ch_gain[ch], &ch_off[ch]) != RP_HW_CALIB_OK) {
                ERROR_LOG("Error get calibration channel: %d", ch);
            }
        }

        // Configure hardware gain
        if (rp_HPGetIsGainDACx5OrDefault()) {
            rp_max7311::rp_setGainOut(RP_MAX7311_OUT1, settings.getDACGain(1).value == CStreamSettings::DACGain::X1 ? RP_GAIN_2V : RP_GAIN_10V);
            rp_max7311::rp_setGainOut(RP_MAX7311_OUT2, settings.getDACGain(2).value == CStreamSettings::DACGain::X1 ? RP_GAIN_2V : RP_GAIN_10V);
        }

        // Create generator
        for (auto uio : uioList) {
            if (uio.nodeName == "rp_dac@40100000") {
                g_gen = uio_lib::CGenerator::create(uio, true, true, dac_speed, getDACRate());
                g_gen->setCalibration(ch_off[0], ch_gain[0], ch_off[1], ch_gain[1]);
                if (!g_gen->setDacHz(dac_speed)) {
                    ERROR_LOG("DAC rate cannot be set.");
                    stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_SETTINGS_ERROR);
                    return false;
                }
                break;
            }
        }

        if (!g_gen) {
            ERROR_LOG("Error init generator module");
            stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_MEM_ERROR);
            return false;
        }

        // Create streaming manager
        if (use_file.value == CStreamSettings::DACPassMode::DAC_NET) {
            g_dac_manger = dac_streaming_lib::CDACStreamingManager::Create(ip_addr_host, false);
        } else if (use_file.value == CStreamSettings::DACPassMode::DAC_FILE) {
            auto format = settings.getDACFileType();
            auto filePath = settings.getDACFile();
            auto dacRepeatMode = settings.getDACRepeat();
            auto dacRepeatCount = settings.getDACRepeatCount();

            if (format.value == CStreamSettings::DataFormat::WAV) {
                g_dac_manger = dac_streaming_lib::CDACStreamingManager::Create(dac_streaming_lib::CDACStreamingManager::WAV_TYPE,
                                                                               CStreamSettings::getDACDirPath() + "/" + filePath,
                                                                               dacRepeatMode,
                                                                               dacRepeatCount,
                                                                               settings.getMemoryBlockSize(),
                                                                               false);
            } else if (format.value == CStreamSettings::DataFormat::TDMS) {
                g_dac_manger = dac_streaming_lib::CDACStreamingManager::Create(dac_streaming_lib::CDACStreamingManager::TDMS_TYPE,
                                                                               CStreamSettings::getDACDirPath() + "/" + filePath,
                                                                               dacRepeatMode,
                                                                               dacRepeatCount,
                                                                               settings.getMemoryBlockSize(),
                                                                               false);
            } else {
                stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_BROKEN);
                return false;
            }

            g_dac_manger->notifyStop.connect([](dac_streaming_lib::CDACStreamingManager::NotifyResult status) { stopDACNonBlocking(status); });

            if (!g_dac_manger->getChannels(activeChannels)) {
                ERROR_LOG("There are no channels in the file.\n");
                stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_EMPTY);
                return false;
            }
        }

        // Allocate memory
        memmanager->releaseMemory(uio_lib::MM_DAC);
        auto mbSize = memmanager->getMemoryBlockSize();
        auto ramSize = memmanager->getReserverdMemory(uio_lib::MM_DAC);

        if (ramSize < memmanager->getMinRAMSize(uio_lib::MM_DAC)) {
            ERROR_LOG("Not enough memory for DAC mode");
            stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_MEM_ERROR);
            return false;
        }

        if (activeChannels.count() == 0) {
            WARNING("No active channels");
            stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_NO_ACTIVE_CNAHHELS);
            return false;
        }

        auto freeblocks = ramSize / mbSize;
        auto reservedBlocks = memmanager->reserveMemory(uio_lib::MM_DAC, freeblocks, activeChannels.count());

        if (reservedBlocks == 0) {
            WARNING("Can't reserve memory via memory manager");
            stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_MEM_ERROR);
            return false;
        }

        WARNING("Reserved memory blocks: %d size: %d", reservedBlocks, reservedBlocks * memmanager->getMemoryBlockSize());

        // Create and start DAC application
        g_dac_app = std::make_shared<dac_streaming_lib::CDACStreamingApplication>(g_dac_manger, g_gen);
        g_dac_manger->getBufferManager()->generateBuffersEmptyDAC(activeChannels, memmanager->getRegions(uio_lib::MM_DAC), DataLib::sizeHeader());
        g_dac_app->runNonBlock();

        if (g_dac_manger->isLocalMode()) {
            g_serverNetConfig->sendDACServerStartedSD();
        } else {
            g_serverNetConfig->sendDACServerStarted();
        }

        ss_dac_status.SendValue(1);
        ss_dac_data_pass.SendValue(1);
        TRACE("Start dac server");
        return true;
    } catch (std::exception& e) {
        ERROR_LOG("Error: startDACServer() %s", e.what());
    }
    return false;
}

// ============================================================================
// DAC Server stop functions
// ============================================================================

auto stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void {
    try {
        std::thread th(stopDACServer, x);
        th.detach();
    } catch (std::exception& e) {
        ERROR_LOG("Error: stopDACNonBlocking() %s", e.what());
    }
}

auto stopDACServer(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void {
    try {
        std::lock_guard guard(g_dac_mutex);

        if (g_serverNetConfig) {
            switch (x) {
                case dac_streaming_lib::CDACStreamingManager::NR_STOP:
                    g_serverNetConfig->sendDACServerStopped();
                    ss_dac_status.SendValue(0);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_ENDED:
                    g_serverNetConfig->sendDACServerStoppedSDDone();
                    ss_dac_status.SendValue(2);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_EMPTY:
                    g_serverNetConfig->sendDACServerStoppedSDEmpty();
                    ss_dac_status.SendValue(3);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_BROKEN:
                    g_serverNetConfig->sendDACServerStoppedSDBroken();
                    ss_dac_status.SendValue(4);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_MISSING_FILE:
                    g_serverNetConfig->sendDACServerStoppedSDMissingFile();
                    ss_dac_status.SendValue(5);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_MEM_ERROR:
                    g_serverNetConfig->sendDACServerMemoryErrorStopped();
                    ss_dac_status.SendValue(6);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_MEM_MODIFY:
                    g_serverNetConfig->sendDACServerMemoryModifyStopped();
                    ss_dac_status.SendValue(7);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_NO_ACTIVE_CNAHHELS:
                    g_serverNetConfig->sendDACServerMemoryErrorStopped();
                    ss_dac_status.SendValue(8);
                    break;
                case dac_streaming_lib::CDACStreamingManager::NR_SETTINGS_ERROR:
                    g_serverNetConfig->sendDACServerConfigErrorStopped();
                    ss_dac_status.SendValue(2);
                    break;
                default:
                    throw std::runtime_error("Unknown state");
            }
        }

        if (g_dac_app) {
            g_dac_app->stop();
            g_dac_app = nullptr;
        }

        memmanager->releaseMemory(uio_lib::MM_DAC);
        ss_dac_data_pass.SendValue(0);
        TRACE("Stop dac server");
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what());
    }
}
