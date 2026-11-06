#include <filesystem>
#include <type_traits>

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
#include "session_lib/stream_session.h"
#include "streaming_lib/streaming_file.h"
#include "board_lib/board.h"
#include "uio_lib/hardware.h"
#include "uio_lib/oscilloscope.h"
#include "uio_lib/uio_parser.h"
#include "uio_lib/memory_manager.h"

// ============================================================================
// Forward declarations
// ============================================================================

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
CIntParameter mem_gpio_in_size("MM_GPIO_IN_SIZE", CBaseParameter::RW, 0, 0, 0, memmanager->getRAMSize());
CIntParameter mem_gpio_out_size("MM_GPIO_OUT_SIZE", CBaseParameter::RW, 0, 0, 0, memmanager->getRAMSize());

CBooleanParameter mem_adc_size_valid("MM_ADC_VALID", CBaseParameter::RW, false, 0);
CBooleanParameter mem_dac_size_valid("MM_DAC_VALID", CBaseParameter::RW, false, 0);
CBooleanParameter mem_gpio_in_size_valid("MM_GPIO_IN_VALID", CBaseParameter::RW, false, 0);
CBooleanParameter mem_gpio_out_size_valid("MM_GPIO_OUT_VALID", CBaseParameter::RW, false, 0);

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

// the streaming of the board: the settings, the config server and the streams
session_lib::StreamSession::Ptr g_session = nullptr;
std::atomic_bool g_adcFile(false);  // the ADC writes a file: its statistics go to the page
std::atomic_bool g_pageChanges(false);  // the page changes the settings: no update of the page back

auto makeWebCallback() -> std::shared_ptr<session_lib::SessionCallback>;

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
            WARNING("Detected %s mode", g_isMaster == uio_lib::BoardMode::MASTER ? "Master" : "Slave");

            TRACE("ss_ip_addr %s", ss_ip_addr.Value().c_str());
            session_lib::SessionOptions options;
            options.configFile = config_file;
            // the config port on every interface: the address of the page is 127.0.0.1 until the
            // page sets it, and it is the announced one, not a filter of the clients
            options.adcHost = ss_ip_addr.Value();
            options.broadcastModel = getModel();
            options.broadcastMac = getMACAddress();
            options.broadcastHost = ss_ip_addr.Value();
            g_session = session_lib::StreamSession::create(g_hw, options);
            g_session->setCallback(makeWebCallback());
        } catch (std::exception& e) {
            WARNING("Init the streaming session %s\n", e.what());
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

auto rp_app_exit(void) -> int {
    if (g_session) {
        g_session->removeCallback();
        g_session->stop();
    }
    // the module stays loaded, the next start may find another bitstream
    g_session = nullptr;
    g_hw = nullptr;
    aprintf(stderr, "Unloading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

// ============================================================================
// Configuration management
// ============================================================================

auto saveConfigInFile() -> void {
    if (!g_session || !g_session->saveSettings()) {
        ERROR_LOG("Error save to file (%s)", config_file);
    }
}

auto sendMemoryValid() -> void {
    mem_adc_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_ADC));
    mem_dac_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_DAC));
    mem_gpio_in_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_GPIO_IN));
    mem_gpio_out_size_valid.SendValue(memmanager->isValidSize(uio_lib::MM_GPIO_OUT));
}

// The page from the settings; the session has applied them to the memory already
void updateUI() {
    if (!g_session)
        return;
    auto s = g_session->settings();
    // Update DAC file list
    auto fileName = s.getDACFile();
    auto files = CStreamSettings::getDACFiles();
    ss_dac_file.SendValue(fileName + "*" + files);

    // Update ADC settings
    ss_use_localfile.SendValue(s.getADCPassMode());

    int ch = 0, att = 0, ac_dc = 0;
    for (int i = 1; i <= 4; i++) {
        ch |= s.getADCChannels(i) ? 1 << (i - 1) : 0;
        att |= s.getADCAttenuator(i) ? 1 << (i - 1) : 0;
        ac_dc |= s.getADCAC_DC(i) ? 1 << (i - 1) : 0;
    }

    ss_channels.SendValue(ch);
    ss_resolution.SendValue(s.getADCResolution());
    ss_save_mode.SendValue(s.getADCType());
    ss_save_capture_time.SendValue(s.getADCCaptureTime());
    ss_format.SendValue(s.getADCFormat());
    ss_attenuator.SendValue(att);
    ss_ac_dc.SendValue(ac_dc);

    // Update DAC settings
    int dac_gain = 0;
    for (int i = 1; i <= 2; i++) {
        dac_gain |= s.getDACGain(i) ? 1 << (1 - i) : 0;
    }

    ss_dac_file_type.SendValue(s.getDACFileType());
    ss_dac_gain.SendValue(dac_gain);
    ss_dac_mode.SendValue(s.getDACPassMode());
    ss_dac_repeat.SendValue(s.getDACRepeat());
    ss_dac_rep_count.SendValue(s.getDACRepeatCount());
    ss_dac_speed.SendValue(s.getDACSpeed());

    // Update common settings
    ss_rate.SendValue(s.getADCDecimation());
    ss_samples.SendValue(s.getADCSamples());
    ss_calib.SendValue(s.getADCCalibration());

    // Update memory settings
    mem_block_size.SendValue(s.getMemoryBlockSize());
    mem_adc_size.SendValue(s.getADCSize());
    mem_dac_size.SendValue(s.getDACSize());
    mem_gpio_in_size.SendValue(s.getGPIOInSize());
    mem_gpio_out_size.SendValue(s.getGPIOOutSize());
    sendMemoryValid();
}

// A change of the page goes to the settings like a client's: checked, applied to the memory
template <typename F>
auto change(F&& f) -> bool {
    return g_session && g_session->updateSettings([&](CStreamSettings& s) {
        if constexpr (std::is_same_v<decltype(f(s)), bool>)
            return f(s);
        else {
            f(s);
            return true;
        }
    });
}

auto dacFileList() -> std::string {
    return (g_session ? g_session->settings().getDACFile() : std::string()) + "*" + CStreamSettings::getDACFiles();
}

void setConfig(bool _force) {
    if (!g_session)
        return;
    g_pageChanges = true;
    bool needUpdate = false;

    // DAC file management
    if (ss_dac_file.IsNewValue() || _force) {
        ss_dac_file.Update();
        change([&](CStreamSettings& s) { return s.setDACFile(ss_dac_file.Value()); });
        ss_dac_file.SendValue(dacFileList());
        needUpdate = true;
    }

    if (ss_dac_file_ctr.IsNewValue()) {
        ss_dac_file_ctr.Update();
        if (ss_dac_file_ctr.Value() == 1) {
            // Refresh file list
            ss_dac_file.SendValue(dacFileList());
        }
        if (ss_dac_file_ctr.Value() == 2) {
            // Delete file
            auto path = CStreamSettings::getDACDirPath();
            auto file = g_session->settings().getDACFile();
            // the setting holds a checked name, never a path: an empty one would be the directory
            if (!file.empty() && CStreamSettings::isValidDACFileName(file)) {
                try {
                    std::filesystem::remove(path + "/" + file);
                } catch (const std::exception& e) {}
            }
            change([](CStreamSettings& s) { return s.setDACFile(""); });
            ss_dac_file.SendValue(dacFileList());
        }
        ss_dac_file_ctr.Value() = 0;
    }

    // Network settings
    if (ss_ip_addr.IsNewValue() || _force) {
        ss_ip_addr.Update();
        g_session->setBroadcastAddress(ss_ip_addr.Value());
    }

    // ADC settings
    if (ss_use_localfile.IsNewValue() || _force) {
        ss_use_localfile.Update();
        change([&](CStreamSettings& s) { s.setADCPassMode(ss_use_localfile.Value() ? CStreamSettings::PassMode::FILE : CStreamSettings::PassMode::NET); });
        needUpdate = true;
    }

    if (ss_channels.IsNewValue() || _force) {
        ss_channels.Update();
        int value = ss_channels.Value();
        for (int i = 1; i <= 4; i++) {
            change([&](CStreamSettings& s) { s.setADCChannels(i, (value & (1 << (i - 1))) ? CStreamSettings::State::ON : CStreamSettings::State::OFF); });
        }
        needUpdate = true;
    }

    if (ss_resolution.IsNewValue() || _force) {
        ss_resolution.Update();
        change([&](CStreamSettings& s) { s.setADCResolution((CStreamSettings::Resolution::_enumerated)ss_resolution.Value()); });
        needUpdate = true;
    }

    if (ss_save_mode.IsNewValue() || _force) {
        ss_save_mode.Update();
        change([&](CStreamSettings& s) { s.setADCType((CStreamSettings::DataType::_enumerated)ss_save_mode.Value()); });
        needUpdate = true;
    }

    if (ss_save_capture_time.IsNewValue() || _force) {
        ss_save_capture_time.Update();
        change([&](CStreamSettings& s) { s.setADCCaptureTime((CStreamSettings::ADCCaptureTime::_enumerated)ss_save_capture_time.Value()); });
        needUpdate = true;
    }

    if (ss_rate.IsNewValue() || _force) {
        ss_rate.Update();
        change([&](CStreamSettings& s) { s.setADCDecimation(ss_rate.Value()); });
        needUpdate = true;
    }

    if (ss_format.IsNewValue() || _force) {
        ss_format.Update();
        change([&](CStreamSettings& s) { s.setADCFormat((CStreamSettings::DataFormat::_enumerated)ss_format.Value()); });
        needUpdate = true;
    }

    if (ss_samples.IsNewValue() || _force) {
        ss_samples.Update();
        change([&](CStreamSettings& s) { s.setADCSamples(ss_samples.Value()); });
        needUpdate = true;
    }

    if (ss_calib.IsNewValue() || _force) {
        ss_calib.Update();
        change([&](CStreamSettings& s) { s.setADCCalibration((CStreamSettings::State::_enumerated)ss_calib.Value()); });
        needUpdate = true;
    }

    // Hardware-specific ADC settings
    if (rp_HPGetFastADCIsLV_HVOrDefault()) {
        if (ss_attenuator.IsNewValue() || _force) {
            ss_attenuator.Update();
            int value = ss_attenuator.Value();
            for (int i = 1; i <= 4; i++) {
                change([&](CStreamSettings& s) { s.setADCAttenuator(i, (value & (1 << (i - 1))) ? CStreamSettings::Attenuator::A_1_20 : CStreamSettings::Attenuator::A_1_1); });
            }
            needUpdate = true;
        }
    }

    if (rp_HPGetFastADCIsAC_DCOrDefault()) {
        if (ss_ac_dc.IsNewValue() || _force) {
            ss_ac_dc.Update();
            int value = ss_ac_dc.Value();
            for (int i = 1; i <= 4; i++) {
                change([&](CStreamSettings& s) { s.setADCAC_DC(i, (value & (1 << (i - 1))) ? CStreamSettings::AC_DC::AC : CStreamSettings::AC_DC::DC); });
            }
            needUpdate = true;
        }
    }

    // DAC settings
    if (ss_dac_file_type.IsNewValue() || _force) {
        ss_dac_file_type.Update();
        change([&](CStreamSettings& s) { s.setDACFileType((CStreamSettings::DataFormat::_enumerated)ss_dac_file_type.Value()); });
        needUpdate = true;
    }

    if (rp_HPGetIsGainDACx5OrDefault()) {
        if (ss_dac_gain.IsNewValue() || _force) {
            ss_dac_gain.Update();
            int value = ss_dac_gain.Value();
            for (int i = 1; i <= 2; i++) {
                change([&](CStreamSettings& s) { s.setDACGain(i, (value & (1 << (i - 1))) ? CStreamSettings::DACGain::X5 : CStreamSettings::DACGain::X1); });
            }
            needUpdate = true;
        }
    }

    if (ss_dac_mode.IsNewValue() || _force) {
        ss_dac_mode.Update();
        change([&](CStreamSettings& s) { s.setDACPassMode((CStreamSettings::DACPassMode::_enumerated)ss_dac_mode.Value()); });
        needUpdate = true;
    }

    if (ss_dac_repeat.IsNewValue() || _force) {
        ss_dac_repeat.Update();
        change([&](CStreamSettings& s) { s.setDACRepeat((CStreamSettings::DACRepeat::_enumerated)ss_dac_repeat.Value()); });
        needUpdate = true;
    }

    if (ss_dac_rep_count.IsNewValue() || _force) {
        ss_dac_rep_count.Update();
        change([&](CStreamSettings& s) { s.setDACRepeatCount(ss_dac_rep_count.Value()); });
        needUpdate = true;
    }

    if (ss_dac_speed.IsNewValue() || _force) {
        ss_dac_speed.Update();
        change([&](CStreamSettings& s) { s.setDACSpeed(ss_dac_speed.Value()); });
        needUpdate = true;
    }

    // Memory settings: a new block size stops the streams (the session splits the memory again)
    if (mem_block_size.IsNewValue() || _force) {
        mem_block_size.Update();
        const uint32_t block = mem_block_size.Value();
        if (!change([&](CStreamSettings& s) { return s.setMemoryBlockSize(block); }))
            mem_block_size.SendValue(g_session->settings().getMemoryBlockSize());
        sendMemoryValid();
        needUpdate = true;
    }

    if (mem_adc_size.IsNewValue() || _force) {
        mem_adc_size.Update();
        change([&](CStreamSettings& s) { s.setADCSize(mem_adc_size.Value()); });
        sendMemoryValid();
        needUpdate = true;
    }

    if (mem_dac_size.IsNewValue() || _force) {
        mem_dac_size.Update();
        change([&](CStreamSettings& s) { s.setDACSize(mem_dac_size.Value()); });
        sendMemoryValid();
        needUpdate = true;
    }

    if (mem_gpio_in_size.IsNewValue() || _force) {
        mem_gpio_in_size.Update();
        change([&](CStreamSettings& s) { s.setGPIOInSize(mem_gpio_in_size.Value()); });
        sendMemoryValid();
        needUpdate = true;
    }

    if (mem_gpio_out_size.IsNewValue() || _force) {
        mem_gpio_out_size.Update();
        change([&](CStreamSettings& s) { s.setGPIOOutSize(mem_gpio_out_size.Value()); });
        sendMemoryValid();
        needUpdate = true;
    }

    g_pageChanges = false;
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
                if (g_session && g_session->startADCStreaming()) {
                    g_session->startADC();
                }
            } else if (g_session) {
                g_session->stopADCStreaming(streaming_lib::ADCStopReason::NORMAL);
            }
        }

        if (ss_dac_start.IsNewValue()) {
            ss_dac_start.Update();
            if (ss_dac_start.Value() == 1) {
                if (g_session)
                    g_session->startDAC();
            } else if (g_session) {
                WARNING("Stop command");
                g_session->stopDAC(dac_streaming_lib::DACStopReason::NORMAL);
            }
        }

        // the page starts the input only: the output plays what a client sends over the network
        if (ss_gpio_start.IsNewValue()) {
            ss_gpio_start.Update();
            if (ss_gpio_start.Value() == 1) {
                if (g_session)
                    g_session->startGPIOIn();
            } else if (g_session) {
                g_session->stopGPIOIn(gpio_streaming_lib::GPIOStopReason::NORMAL);
            }
        }

        // Update statistics
        if (g_session && g_adcFile) {
            ss_pass_samples.SendValue(g_session->adcPassSamples());
            ss_writed_size.SendValue(g_session->adcWrittenSize());
        }
    } catch (std::exception& e) {
        FATAL("%s", e.what());
    }
}

void OnNewParams(void) {
    UpdateParamsFromWeb();
}

// ============================================================================
// The events of the streams on the page (the session answers the config protocol)
// ============================================================================

namespace {

// SS_STATUS (ADC): 0 stopped, 1 running, 2 SD full, 3 done, 4 memory error, 5 memory changed,
// 6 no channels. SS_DAC_STATUS: 0 stopped, 1 running, 2 done, 3 empty, 4 broken, 5 missing file,
// 6 memory error, 7 memory changed, 8 no channels. SS_GPIO_STATUS (the input): 0 stopped,
// 1 running, 2 SD full, 3 the file has its samples, 4 the file cannot be written, 6 memory error,
// 7 memory changed, 8 configuration error.
class WebCallback : public session_lib::SessionCallback {
    void settingsChanged(const CStreamSettings&, const CStreamSettings&) override {
        if (!g_pageChanges)
            updateUI();
    }

    void adcStarted(bool file) override {
        g_adcFile = file;
        ss_status.SendValue(1);
        TRACE("Start server");
    }

    void adcRunning() override { ss_adc_data_pass.SendValue(1); }

    void adcStopped(streaming_lib::ADCStopReason reason) override {
        using R = streaming_lib::ADCStopReason;
        g_adcFile = false;
        switch (reason) {
            case R::NORMAL:
            case R::CONFIG_ERROR:
            case R::DISCONNECTED:
                ss_status.SendValue(0);
                break;
            case R::SD_FULL:
                ss_status.SendValue(2);
                break;
            case R::DONE:
                ss_status.SendValue(3);
                break;
            case R::MEM_ERROR:
                ss_status.SendValue(4);
                break;
            case R::MEM_MODIFY:
                ss_status.SendValue(5);
                break;
            case R::NO_CHANNELS:
                ss_status.SendValue(6);
                break;
        }
        ss_adc_data_pass.SendValue(0);
        TRACE("Stop server");
    }

    // before the playback: the stop of a short file comes after it
    void dacStarted(bool) override {
        ss_dac_status.SendValue(1);
        ss_dac_data_pass.SendValue(1);
        TRACE("Start dac server");
    }

    void dacStopped(dac_streaming_lib::DACStopReason reason) override {
        using R = dac_streaming_lib::DACStopReason;
        switch (reason) {
            case R::NORMAL:
            case R::DISCONNECTED:
                ss_dac_status.SendValue(0);
                break;
            case R::DONE:
            case R::CONFIG_ERROR:
                ss_dac_status.SendValue(2);
                break;
            case R::EMPTY:
                ss_dac_status.SendValue(3);
                break;
            case R::BROKEN:
                ss_dac_status.SendValue(4);
                break;
            case R::MISSING_FILE:
                ss_dac_status.SendValue(5);
                break;
            case R::MEM_ERROR:
                ss_dac_status.SendValue(6);
                break;
            case R::MEM_MODIFY:
                ss_dac_status.SendValue(7);
                break;
            case R::NO_CHANNELS:
                ss_dac_status.SendValue(8);
                break;
        }
        ss_dac_data_pass.SendValue(0);
        TRACE("Stop dac server");
    }

    void gpioInStarted() override {
        ss_gpio_status.SendValue(1);
        ss_gpio_data_pass.SendValue(1);
        TRACE("Start gpio input server");
    }

    void gpioInStopped(gpio_streaming_lib::GPIOStopReason reason) override {
        using R = gpio_streaming_lib::GPIOStopReason;
        switch (reason) {
            case R::NORMAL:
            case R::DISCONNECTED:
                ss_gpio_status.SendValue(0);
                break;
            case R::MEM_ERROR:
                ss_gpio_status.SendValue(6);
                break;
            case R::MEM_MODIFY:
                ss_gpio_status.SendValue(7);
                break;
            case R::CONFIG_ERROR:
            case R::MISSING_FILE:
            case R::EMPTY:
                ss_gpio_status.SendValue(8);
                break;
            case R::SD_FULL:
                ss_gpio_status.SendValue(2);
                break;
            case R::DONE:
                ss_gpio_status.SendValue(3);
                break;
            case R::BROKEN:
                ss_gpio_status.SendValue(4);
                break;
        }
        ss_gpio_data_pass.SendValue(0);
        TRACE("Stop gpio input server");
    }
};

}  // namespace

auto makeWebCallback() -> std::shared_ptr<session_lib::SessionCallback> {
    return std::make_shared<WebCallback>();
}
