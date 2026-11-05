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
#include "dac_streaming_lib/dac_stream_server.h"
#include "data_lib/buffers_cached.h"
#include "streaming_lib/adc_stream_server.h"
#include "streaming_lib/streaming_file.h"
#include "streaming_lib/trigger_commands.h"
#include "streaming_lib/streaming_fpga.h"
#include "streaming_lib/streaming_net.h"
#include "board_lib/board.h"
#include "uio_lib/hardware.h"
#include "uio_lib/oscilloscope.h"
#include "uio_lib/uio_parser.h"
#include "uio_lib/memory_manager.h"

// ============================================================================
// Forward declarations
// ============================================================================

bool startServer(bool testMode);
void stopServer(ServerNetConfigManager::EStopReason x);
void stopNonBlocking(ServerNetConfigManager::EStopReason x);

auto startDACServer(dac_channels_t channels) -> bool;
auto stopDACNonBlocking(dac_streaming_lib::DACStopReason x) -> void;
auto stopDACServer(dac_streaming_lib::DACStopReason x) -> void;
void startADC();
auto makeADCCallback() -> std::shared_ptr<streaming_lib::ADCServerCallback>;
auto makeDACCallback() -> std::shared_ptr<dac_streaming_lib::DACServerCallback>;

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

uio_lib::CHardware::Ptr g_hw = nullptr;

streaming_lib::ADCStreamServer::Ptr g_adc = nullptr;
std::atomic_bool g_adcPrepared(false);  // started, waits for the start of the ADC or runs
std::atomic_bool g_adcFile(false);      // writes a file: its statistics go to the page

dac_streaming_lib::DACStreamServer::Ptr g_dac = nullptr;
ServerNetConfigManager::Ptr g_serverNetConfig = nullptr;

uio_lib::BoardMode g_isMaster = uio_lib::BoardMode::UNKNOWN;

std::atomic_bool g_serverRun(false);

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
            // The FPGA blocks while the application runs; the daisy chain role is read from the scope
            g_hw = uio_lib::CHardware::create(board_lib::createBoard());
            if (!g_hw) {
                FATAL("Can't open the FPGA blocks")
            }
            g_isMaster = g_hw->features().slave ? uio_lib::BoardMode::SLAVE : uio_lib::BoardMode::MASTER;
            g_adc = std::make_shared<streaming_lib::ADCStreamServer>(g_hw);
            g_adc->setCallback(makeADCCallback());
            g_dac = std::make_shared<dac_streaming_lib::DACStreamServer>(g_hw);
            g_dac->setCallback(makeDACCallback());
            WARNING("Detected %s mode", g_isMaster == uio_lib::BoardMode::MASTER ? "Master" : "Slave");

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
            g_serverNetConfig->startDacStreamingNofiy.connect([](auto channels) { startDACServer(channels); });
            g_serverNetConfig->stopDacStreamingNofiy.connect([]() { stopDACNonBlocking(dac_streaming_lib::DACStopReason::NORMAL); });
            g_serverNetConfig->startADCNofiy.connect([]() { startADC(); });
            streaming_lib::connectTriggerCommands(g_serverNetConfig, g_hw);
            g_serverNetConfig->memoryBlockSizeChangeNofiy.connect([]() {
                stopServer(ServerNetConfigManager::EStopReason::MEM_MODIFY);
                stopDACServer(dac_streaming_lib::DACStopReason::MEM_MODIFY);
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
    stopDACServer(dac_streaming_lib::DACStopReason::MEM_MODIFY);
}

auto rp_app_exit(void) -> int {
    g_serverNetConfig->stop();
    stopServer(ServerNetConfigManager::EStopReason::NORMAL);
    stopDACServer(dac_streaming_lib::DACStopReason::NORMAL);
    // the module stays loaded, the next start may find another bitstream
    g_adc = nullptr;
    g_dac = nullptr;
    g_hw = nullptr;
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
                startDACServer(dac_channels_t());
            } else {
                WARNING("Stop command");
                stopDACNonBlocking(dac_streaming_lib::DACStopReason::NORMAL);
            }
        }

        if (ss_gpio_start.IsNewValue()) {
            ss_gpio_start.Update();
            // GPIO functionality not implemented yet
        }

        // Update statistics
        if (g_adc && g_adcFile) {
            ss_pass_samples.SendValue(g_adc->getPassSamples());
            ss_writed_size.SendValue(g_adc->getWritedSize());
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

namespace {

auto toADC(ServerNetConfigManager::EStopReason reason) -> streaming_lib::ADCStopReason {
    using R = streaming_lib::ADCStopReason;
    switch (reason) {
        case ServerNetConfigManager::EStopReason::SD_FULL:
            return R::SD_FULL;
        case ServerNetConfigManager::EStopReason::DONE:
            return R::DONE;
        case ServerNetConfigManager::EStopReason::MEM_ERROR:
            return R::MEM_ERROR;
        case ServerNetConfigManager::EStopReason::MEM_MODIFY:
            return R::MEM_MODIFY;
        case ServerNetConfigManager::EStopReason::NO_CHANNELS:
            return R::NO_CHANNELS;
        case ServerNetConfigManager::EStopReason::CONFIG_ERROR:
            return R::CONFIG_ERROR;
        default:
            return R::NORMAL;
    }
}

// The events of the ADC streaming to the config protocol and to the page
class WebADCCallback : public streaming_lib::ADCServerCallback {
    void startedTCP(streaming_lib::ADCStreamServer*) override { g_serverNetConfig->sendADCServerStartedTCP(); }

    void startedSD(streaming_lib::ADCStreamServer*) override { g_serverNetConfig->sendADCServerStartedSD(); }

    void stopped(streaming_lib::ADCStreamServer*, streaming_lib::ADCStopReason reason) override {
        using R = streaming_lib::ADCStopReason;
        g_adcPrepared = false;
        g_adcFile = false;
        switch (reason) {
            case R::NORMAL:
                g_serverNetConfig->sendServerStopped();
                ss_status.SendValue(0);
                break;
            case R::SD_FULL:
                g_serverNetConfig->sendServerStoppedSDFull();
                ss_status.SendValue(2);
                break;
            case R::DONE:
                g_serverNetConfig->sendServerStoppedDone();
                ss_status.SendValue(3);
                break;
            case R::MEM_ERROR:
                g_serverNetConfig->sendServerMemoryErrorStopped();
                ss_status.SendValue(4);
                break;
            case R::MEM_MODIFY:
                g_serverNetConfig->sendServerMemoryModifyStopped();
                ss_status.SendValue(5);
                break;
            case R::NO_CHANNELS:
                g_serverNetConfig->sendServerNoChannelsStopped();
                ss_status.SendValue(6);
                break;
            case R::CONFIG_ERROR:
                g_serverNetConfig->sendServerConfigErrorStopped();
                ss_status.SendValue(0);
                break;
        }
        ss_adc_data_pass.SendValue(0);
        TRACE("Stop server");
    }
};

}  // namespace

auto makeADCCallback() -> std::shared_ptr<streaming_lib::ADCServerCallback> {
    return std::make_shared<WebADCCallback>();
}

bool startServer(bool testMode) {
    std::lock_guard guard(g_adc_mutex);
    if (!g_adc)
        return false;
    CStreamSettings settings = g_serverNetConfig->getSettings();
    streaming_lib::ADCStreamOptions options;
    options.host = ss_ip_addr.Value();
    options.testMode = testMode;
    g_adcFile = settings.getADCPassMode().value == CStreamSettings::PassMode::FILE;
    if (!g_adc->start(settings, options))
        return false;
    g_adcPrepared = true;
    ss_status.SendValue(1);
    TRACE("Start server");
    return true;
}

void startADC() {
    if (!g_adc || !g_adcPrepared)
        return;
    g_adc->startADC();
    ss_adc_data_pass.SendValue(1);
    g_serverNetConfig->sendADCStarted();
}

void stopNonBlocking(ServerNetConfigManager::EStopReason x) {
    if (g_adc)
        g_adc->stopNonBlocking(toADC(x));
}

void stopServer(ServerNetConfigManager::EStopReason x) {
    std::lock_guard guard(g_adc_mutex);
    if (g_adc)
        g_adc->stop(toADC(x));
}

// ============================================================================
// DAC Server functions
// ============================================================================

namespace {

// The events of the DAC streaming to the config protocol and to the page
class WebDACCallback : public dac_streaming_lib::DACServerCallback {
    // before the playback: the stop of a short file comes after them
    void startedTCP(dac_streaming_lib::DACStreamServer*) override {
        g_serverNetConfig->sendDACServerStarted();
        started();
    }

    void startedSD(dac_streaming_lib::DACStreamServer*) override {
        g_serverNetConfig->sendDACServerStartedSD();
        started();
    }

    void stopped(dac_streaming_lib::DACStreamServer*, dac_streaming_lib::DACStopReason reason) override {
        using R = dac_streaming_lib::DACStopReason;
        switch (reason) {
            case R::NORMAL:
                g_serverNetConfig->sendDACServerStopped();
                ss_dac_status.SendValue(0);
                break;
            case R::DONE:
                g_serverNetConfig->sendDACServerStoppedSDDone();
                ss_dac_status.SendValue(2);
                break;
            case R::EMPTY:
                g_serverNetConfig->sendDACServerStoppedSDEmpty();
                ss_dac_status.SendValue(3);
                break;
            case R::BROKEN:
                g_serverNetConfig->sendDACServerStoppedSDBroken();
                ss_dac_status.SendValue(4);
                break;
            case R::MISSING_FILE:
                g_serverNetConfig->sendDACServerStoppedSDMissingFile();
                ss_dac_status.SendValue(5);
                break;
            case R::MEM_ERROR:
                g_serverNetConfig->sendDACServerMemoryErrorStopped();
                ss_dac_status.SendValue(6);
                break;
            case R::MEM_MODIFY:
                g_serverNetConfig->sendDACServerMemoryModifyStopped();
                ss_dac_status.SendValue(7);
                break;
            case R::NO_CHANNELS:
                g_serverNetConfig->sendDACServerMemoryErrorStopped();
                ss_dac_status.SendValue(8);
                break;
            case R::CONFIG_ERROR:
                g_serverNetConfig->sendDACServerConfigErrorStopped();
                ss_dac_status.SendValue(2);
                break;
        }
        ss_dac_data_pass.SendValue(0);
        TRACE("Stop dac server");
    }

    static void started() {
        ss_dac_status.SendValue(1);
        ss_dac_data_pass.SendValue(1);
        TRACE("Start dac server");
    }
};

}  // namespace

auto makeDACCallback() -> std::shared_ptr<dac_streaming_lib::DACServerCallback> {
    return std::make_shared<WebDACCallback>();
}

auto startDACServer(dac_channels_t channels) -> bool {
    if (!g_serverNetConfig || !g_dac)
        return false;
    CStreamSettings settings = g_serverNetConfig->getSettings();
    memmanager->setReserverdMemory(uio_lib::MM_DAC, settings.getDACSize());
    return g_dac->start(settings, channels);
}

auto stopDACNonBlocking(dac_streaming_lib::DACStopReason x) -> void {
    if (g_dac)
        g_dac->stopNonBlocking(x);
}

auto stopDACServer(dac_streaming_lib::DACStopReason x) -> void {
    if (g_dac)
        g_dac->stop(x);
}
