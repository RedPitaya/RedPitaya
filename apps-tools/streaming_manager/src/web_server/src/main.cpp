#include <cmath>
#include <filesystem>
#include <functional>
#include <memory>
#include <vector>
#include <type_traits>

// Platform/Hardware includes

// Common includes
#include "common/version.h"
#include "main.h"
#include "web/rp_client.h"

// Streaming/Config includes
#include "session_lib/stream_session.h"
#include "session_lib/trigger_commands.h"
#include "streaming_lib/streaming_file.h"
#include "board_lib/board.h"
#include "rp_hw-profiles.h"
#include "uio_lib/hardware.h"
#include "uio_lib/oscilloscope.h"
#include "uio_lib/uio_parser.h"
#include "uio_lib/memory_manager.h"

// ============================================================================
// Forward declarations
// ============================================================================


void setConfig(bool _force);
void updateUI();
auto sendTriggerWindow() -> void;
auto gpioFileList() -> std::string;

// ============================================================================
// Global variables
// ============================================================================

static constexpr char config_file[] = "/root/.config/redpitaya/apps/streaming/streaming_config.json";
// set by streaming-server, which runs this module as the service of the page
std::string g_configFile = config_file;
std::string g_listenHost;
bool g_verbose = false;

auto memmanager = uio_lib::CMemoryManager::instance();
// The board (profile, calibration, relays) before the parameters of the page, which take its data;
// the module stays loaded, the board is made once
const std::shared_ptr<uio_lib::IBoard> g_board = board_lib::createBoard();
const uio_lib::BoardInfo g_info = g_board->info();
const uint32_t g_dacRate = g_info.dacRate ? g_info.dacRate : 1;

// ============================================================================
// ADC Parameters
// ============================================================================

CBooleanParameter ss_start("SS_START", CBaseParameter::RW, false, 0);
CBooleanParameter ss_use_localfile("SS_USE_FILE", CBaseParameter::RW, false, 0);
CIntParameter ss_is_master("SS_IS_MASTER", CBaseParameter::RO, 0, 0, 0, 2);
CStringParameter ss_ip_addr("SS_IP_ADDR", CBaseParameter::RW, "127.0.0.1", 0);
// sample limits are 64 bit in the settings: a double holds every integer up to 2^53 exactly
constexpr double kMaxSamples = 9007199254740992.0;
CDoubleParameter ss_samples("SS_SAMPLES", CBaseParameter::RW, 20000000, 0, 0, kMaxSamples);
CIntParameter ss_channels("SS_CHANNEL", CBaseParameter::RW, 0, 0, 0, 256);
CIntParameter ss_resolution("SS_RESOLUTION", CBaseParameter::RW, 0, 0, 0, 256);
CIntParameter ss_calib("SS_USE_CALIB", CBaseParameter::RW, 1, 0, 0, 1);
CIntParameter ss_save_mode("SS_SAVE_MODE", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter ss_save_capture_time("SS_CAPTURE_TIME", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter ss_rate("SS_RATE", CBaseParameter::RW, 4, 0, 1, 65536);
CIntParameter ss_format("SS_FORMAT", CBaseParameter::RW, 0, 0, 0, 2);
CIntParameter ss_status("SS_STATUS", CBaseParameter::RW, 1, 0, 0, 100);
CBooleanParameter ss_adc_data_pass("SS_ADC_DATA_PASS", CBaseParameter::RW, false, 0);
CIntParameter ss_acd_max("SS_ACD_MAX", CBaseParameter::RO, g_info.adcRate, 0, g_info.adcRate, g_info.adcRate);
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
CIntParameter ss_dac_file_type("SS_DAC_FILE_TYPE", CBaseParameter::RW, 0, 0, 0, 2);
CIntParameter ss_dac_gain("SS_DAC_GAIN", CBaseParameter::RW, 0, 0, 0, 256);
CIntParameter ss_dac_mode("SS_DAC_MODE", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter ss_dac_speed("SS_DAC_HZ", CBaseParameter::RW, g_dacRate, 0, 1.0 / (65536.0 / g_dacRate) + 1.0, g_dacRate);
CIntParameter ss_dac_repeat("SS_DAC_REPEAT", CBaseParameter::RW, -1, 0, -2, 0);
CIntParameter ss_dac_rep_count("SS_DAC_REPEAT_COUNT", CBaseParameter::RW, 1, 0, 1, 2000000000);

// ============================================================================
// Read-only capability parameters
// ============================================================================

CIntParameter ss_adc_channels("SS_ADC_CHANNELS", CBaseParameter::RO, g_info.adcChannels, 0, 0, 10);
CIntParameter ss_dac_channels("SS_DAC_CHANNELS", CBaseParameter::RO, g_info.dacChannels, 0, 0, 10);
CBooleanParameter ss_ac_dc_enable("SS_IS_AC_DC", CBaseParameter::RO, g_info.adcACDC, 0);
CBooleanParameter ss_dac_gain_enable("SS_IS_DAC_GAIN", CBaseParameter::RO, g_info.dacGain, 0);
// the inputs have two ranges (1:1 and 1:20): the page shows the range switch only then
CBooleanParameter ss_attenuator_enable("SS_IS_ATTENUATOR", CBaseParameter::RO, g_info.adcRanges, 0);

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
// the output: start/stop and the state (codes as SS_GPIO_STATUS)
CBooleanParameter ss_gpio_out_start("SS_GPIO_OUT_START", CBaseParameter::RW, false, 0);
CIntParameter ss_gpio_out_status("SS_GPIO_OUT_STATUS", CBaseParameter::RW, 0, 0, 0, 100);
// "<current file>*<file 1>\n<file 2>..." of the GPIO directory, like SS_DAC_FILE
CStringParameter ss_gpio_out_file("SS_GPIO_OUT_FILE", CBaseParameter::RW, "", 0);

// ============================================================================
// Triggers: the settings of the ADC, DAC and GPIO triggers and of the external input. Every
// one is a plain integer of the settings (enums as their values); a refused value is answered
// with the one the settings keep.
// ============================================================================

// command: 1 software trigger of the ADC, 2 of the DAC, 3 of the GPIO
CIntParameter ss_soft_trigger("SS_SOFT_TRIGGER", CBaseParameter::RW, 0, 0, 0, 3);
// the trigger status of the FPGA blocks as JSON (TriggerStatus::toJson), sent when it changes
CStringParameter ss_trigger_status("SS_TRIGGER_STATUS", CBaseParameter::RO, "", 0);
// the ADC level window in volts (settings adc_trigger_level_v / adc_trigger_hysteresis_v): the
// codes SS_TRIG_LEVEL_LOW/HIGH follow from them, the edge and the range of the first chosen input
CDoubleParameter ss_trig_level_v("SS_TRIG_LEVEL_V", CBaseParameter::RW, 0, 0, -20, 20);
CDoubleParameter ss_trig_hyst_v("SS_TRIG_HYST_V", CBaseParameter::RW, 0, 0, 0, 40);

// the settings as a whole. Command: 1 - back to the defaults, 2 - send them as JSON in
// SS_SETTINGS_JSON. A JSON written to SS_SETTINGS_JSON is applied like a client's; the answer of
// both is SS_SETTINGS_RESULT: 1 applied, 2 refused (the settings stay as they were)
CIntParameter ss_settings_cmd("SS_SETTINGS_CMD", CBaseParameter::RW, 0, 0, 0, 2);
CStringParameter ss_settings_json("SS_SETTINGS_JSON", CBaseParameter::RW, "", 0);
CIntParameter ss_settings_result("SS_SETTINGS_RESULT", CBaseParameter::RW, 0, 0, 0, 2);

// the sample limit of the GPIO input file (gpio_in_samples_limit_sd, 64 bit), 0 - until the stop
CDoubleParameter ss_gpio_in_samples("SS_GPIO_IN_SAMPLES", CBaseParameter::RW, 0, 0, 0, kMaxSamples);

// the board for the strip of the page: the profile's model name and the MAC of eth0
CStringParameter ss_board_model("SS_BOARD_MODEL", CBaseParameter::RO, "", 0);
CStringParameter ss_board_mac("SS_BOARD_MAC", CBaseParameter::RO, "", 0);

// capabilities of the bitstream
CBooleanParameter ss_gpio_enable("SS_GPIO_IS_ENABLE", CBaseParameter::RO, false, 0);
CBooleanParameter ss_dac_trig_outputs("SS_DAC_TRIG_OUTPUTS", CBaseParameter::RO, false, 0);
CBooleanParameter ss_ext_trig_enable("SS_EXT_TRIG_IS_ENABLE", CBaseParameter::RO, false, 0);

// rp_sdk registers a parameter by its address: every one is built once in its final place
struct IntSetting {
    CIntParameter param;
    std::function<bool(CStreamSettings&, int)> set;
    std::function<int(const CStreamSettings&)> get;
};

template <typename E>
auto en(int v) -> E {
    return static_cast<typename E::_enumerated>(v);
}

#define SETTING(NAME, LO, HI, SET, GET)                                                                                          \
    std::unique_ptr<IntSetting>(new IntSetting{CIntParameter(NAME, CBaseParameter::RW, LO, 0, LO, HI),                                \
                                               [](CStreamSettings& s, int v) -> bool { SET; }, [](const CStreamSettings& s) -> int { return GET; }})

using SS = CStreamSettings;
std::vector<std::unique_ptr<IntSetting>> g_settings = [] {
    std::vector<std::unique_ptr<IntSetting>> t;
    t.reserve(48);
    // ADC trigger
    t.push_back(SETTING("SS_TRIG_MODE", 0, 1, s.setADCTriggerMode(en<SS::State>(v)); return true, (int)s.getADCTriggerMode()));
    t.push_back(SETTING("SS_TRIG_SOURCES", 0, 63, return s.setADCTriggerSources(v), (int)s.getADCTriggerSources()));
    t.push_back(SETTING("SS_TRIG_LEVEL_LOW", -32768, 32767, return s.setADCTriggerLevelLow(v), s.getADCTriggerLevelLow()));
    t.push_back(SETTING("SS_TRIG_LEVEL_HIGH", -32768, 32767, return s.setADCTriggerLevelHigh(v), s.getADCTriggerLevelHigh()));
    t.push_back(SETTING("SS_TRIG_LEVEL_UNIT", 0, 1, s.setADCTriggerLevelUnit(en<SS::LevelUnit>(v)); return true, (int)s.getADCTriggerLevelUnit()));
    t.push_back(SETTING("SS_TRIG_EDGE", 0, 1, s.setADCTriggerEdge(en<SS::TriggerEdge>(v)); return true, (int)s.getADCTriggerEdge()));
    t.push_back(SETTING("SS_TRIG_EVENT", -1, 4, s.setADCEventSource(en<SS::EventSource>(v)); return true, (int)s.getADCEventSource()));
    t.push_back(SETTING("SS_TRIG_BLOCKS", 0, 65535, return s.setADCTriggerBlocks(v), (int)s.getADCTriggerBlocks()));
    t.push_back(SETTING("SS_TDMS_TIME", 0, 1, s.setTDMSTimeColumn(en<SS::State>(v)); return true, (int)s.getTDMSTimeColumn()));
    // DAC trigger: bit 0 - OUT1 waits for a trigger, bit 1 - OUT2
    t.push_back(SETTING("SS_DAC_TRIG_MODE", 0, 3, s.setDACTriggerMode(1, en<SS::State>(v & 1)); s.setDACTriggerMode(2, en<SS::State>((v >> 1) & 1)); return true,
                        ((int)s.getDACTriggerMode(1) ? 1 : 0) | ((int)s.getDACTriggerMode(2) ? 2 : 0)));
    t.push_back(SETTING("SS_DAC_TRIG_SOURCES", 0, 63, return s.setDACTriggerSources(v), (int)s.getDACTriggerSources()));
    t.push_back(SETTING("SS_DAC_EVENT", -1, 4, s.setDACEventSource(en<SS::EventSource>(v)); return true, (int)s.getDACEventSource()));
    t.push_back(SETTING("SS_DAC_PLAY_GAPS", 0, 1, s.setDACPlayGaps(en<SS::State>(v)); return true, (int)s.getDACPlayGaps()));
    // external trigger input
    t.push_back(SETTING("SS_EXT_COND", 0, 1, s.setExtTrigConditioner(en<SS::State>(v)); return true, (int)s.getExtTrigConditioner()));
    t.push_back(SETTING("SS_EXT_EDGE", 0, 1, s.setExtTrigEdge(en<SS::TriggerEdge>(v)); return true, (int)s.getExtTrigEdge()));
    t.push_back(SETTING("SS_EXT_DEBOUNCE", 0, 65535, return s.setExtTrigDebounce(v), (int)s.getExtTrigDebounce()));
    t.push_back(SETTING("SS_EXT_DIO0", 0, 1, s.setExtTrigDio0(en<SS::State>(v)); return true, (int)s.getExtTrigDio0()));
    // GPIO lines and sampling
    t.push_back(SETTING("SS_GPIO_OUTPUTS_P", 0, 255, return s.setGPIOOutputsP(v), (int)s.getGPIOOutputsP()));
    t.push_back(SETTING("SS_GPIO_OUTPUTS_N", 0, 255, return s.setGPIOOutputsN(v), (int)s.getGPIOOutputsN()));
    t.push_back(SETTING("SS_GPIO_POLARITY", 0, 65535, return s.setGPIOPolarity(v), (int)s.getGPIOPolarity()));
    t.push_back(SETTING("SS_GPIO_DECIMATION", 1, 131072, return s.setGPIODecimation(v), (int)s.getGPIODecimation()));
    t.push_back(SETTING("SS_GPIO_RLE", 0, 1, s.setGPIORLE(en<SS::State>(v)); return true, (int)s.getGPIORLE()));
    t.push_back(SETTING("SS_GPIO_OUT_STEP", 1, 131072, return s.setGPIOOutStep(v), (int)s.getGPIOOutStep()));
    // GPIO input trigger: the pattern (bit i - P i, bit 8 + i - N i) and the other sources
    t.push_back(SETTING("SS_GPIO_PAT_MASK", 0, 65535, return s.setGPIOPatternMask(v), (int)s.getGPIOPatternMask()));
    t.push_back(SETTING("SS_GPIO_PAT_VALUE", 0, 65535, return s.setGPIOPatternValue(v), (int)s.getGPIOPatternValue()));
    t.push_back(SETTING("SS_GPIO_PAT_POS", 0, 65535, return s.setGPIOPatternPosEdge(v), (int)s.getGPIOPatternPosEdge()));
    t.push_back(SETTING("SS_GPIO_PAT_NEG", 0, 65535, return s.setGPIOPatternNegEdge(v), (int)s.getGPIOPatternNegEdge()));
    t.push_back(SETTING("SS_GPIO_TRIG_SOURCES", 0, 63, return s.setGPIOTriggerSources(v), (int)s.getGPIOTriggerSources()));
    t.push_back(SETTING("SS_GPIO_EVENT", -1, 4, s.setGPIOEventSource(en<SS::EventSource>(v)); return true, (int)s.getGPIOEventSource()));
    t.push_back(SETTING("SS_GPIO_IN_TRIG_MODE", 0, 1, s.setGPIOInTriggerMode(en<SS::State>(v)); return true, (int)s.getGPIOInTriggerMode()));
    t.push_back(SETTING("SS_GPIO_OUT_TRIG_MODE", 0, 1, s.setGPIOOutTriggerMode(en<SS::State>(v)); return true, (int)s.getGPIOOutTriggerMode()));
    t.push_back(SETTING("SS_GPIO_TRIG_BLOCKS", 0, 65535, return s.setGPIOTriggerBlocks(v), (int)s.getGPIOTriggerBlocks()));
    // GPIO file mode
    t.push_back(SETTING("SS_GPIO_IN_MODE", 0, 1, s.setGPIOInPassMode(en<SS::PassMode>(v)); return true, (int)s.getGPIOInPassMode()));
    t.push_back(SETTING("SS_GPIO_IN_FORMAT", 0, 2, s.setGPIOInFormat(en<SS::GPIOFormat>(v)); return true, (int)s.getGPIOInFormat()));
    t.push_back(SETTING("SS_GPIO_OUT_MODE", 0, 1, s.setGPIOOutPassMode(en<SS::PassMode>(v)); return true, (int)s.getGPIOOutPassMode()));
    t.push_back(SETTING("SS_GPIO_OUT_REPEAT", -2, 0, s.setGPIOOutRepeat(en<SS::GPIORepeat>(v)); return true, (int)s.getGPIOOutRepeat()));
    t.push_back(SETTING("SS_GPIO_OUT_REPEAT_COUNT", 1, 2000000000, s.setGPIOOutRepeatCount(v); return true, (int)s.getGPIOOutRepeatCount()));
    t.push_back(SETTING("SS_GPIO_OUT_PLAY_GAPS", 0, 1, s.setGPIOOutPlayGaps(en<SS::State>(v)); return true, (int)s.getGPIOOutPlayGaps()));
    return t;
}();
#undef SETTING

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

// ============================================================================
// Application description & signal handling
// ============================================================================

const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya Stream server application.\n";
}

// ============================================================================
// Application initialization
// ============================================================================

void rp_app_set_session_options(const char* configFile, const char* listenHost, bool verbose) {
    g_configFile = configFile && *configFile ? configFile : config_file;
    g_listenHost = listenHost ? listenHost : "";
    g_verbose = verbose;
}

auto rp_app_init(void) -> int {
    fprintf(stderr, "Loading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);

#ifdef ZIP_DISABLED
    CDataManager::GetInstance()->SetEnableParamsGZip(false);
    CDataManager::GetInstance()->SetEnableSignalsGZip(false);
    CDataManager::GetInstance()->SetEnableBinarySignalsGZip(false);
#endif

    CDataManager::GetInstance()->SetParamInterval(100);

    try {
        // Detect master/slave mode
        try {
            // The FPGA blocks while the application runs; the daisy chain role is read from the scope
            g_hw = uio_lib::CHardware::create(g_board);
            if (!g_hw)
                throw std::runtime_error("can't open the FPGA blocks");
            g_isMaster = g_hw->features().slave ? uio_lib::BoardMode::SLAVE : uio_lib::BoardMode::MASTER;
            WARNING("Detected %s mode", g_isMaster == uio_lib::BoardMode::MASTER ? "Master" : "Slave");

            TRACE("ss_ip_addr %s", ss_ip_addr.Value().c_str());
            session_lib::SessionOptions options;
            options.configFile = g_configFile;
            options.host = g_listenHost;
            options.verbose = g_verbose;
            // the config and data ports on every interface: the address of the page is 127.0.0.1
            // until the page sets it, and it is the announced one, not a filter of the clients
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
        ss_gpio_out_status.SendValue(0);
        {
            char* model = nullptr;
            ss_board_model.SendValue(rp_HPGetModelName(&model) == RP_HP_OK && model ? model : "Red Pitaya");
            ss_board_mac.SendValue(g_info.mac);
        }
        if (g_hw) {
            auto f = g_hw->features();
            ss_gpio_enable.SendValue(f.gpio);
            ss_dac_trig_outputs.SendValue(f.dacTriggerOutputs);
            ss_ext_trig_enable.SendValue(f.extTrigger);
        }
        updateUI();


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
        dac_gain |= s.getDACGain(i) ? 1 << (i - 1) : 0;
    }

    ss_dac_file_type.SendValue(s.getDACFileType());
    ss_dac_gain.SendValue(dac_gain);
    ss_dac_mode.SendValue(s.getDACPassMode());
    ss_dac_repeat.SendValue(s.getDACRepeat());
    ss_dac_rep_count.SendValue(s.getDACRepeatCount());
    ss_dac_speed.SendValue(s.getDACSpeed());

    // Update common settings
    ss_rate.SendValue(s.getADCDecimation());
    ss_samples.SendValue((double)s.getADCSamples());
    ss_calib.SendValue(s.getADCCalibration());

    // Update memory settings
    mem_block_size.SendValue(s.getMemoryBlockSize());
    mem_adc_size.SendValue(s.getADCSize());
    mem_dac_size.SendValue(s.getDACSize());
    mem_gpio_in_size.SendValue(s.getGPIOInSize());
    mem_gpio_out_size.SendValue(s.getGPIOOutSize());
    sendMemoryValid();

    for (auto& it : g_settings)
        it->param.SendValue(it->get(s));
    ss_gpio_out_file.SendValue(gpioFileList());
    ss_trig_level_v.SendValue(s.getADCTriggerLevelVolt());
    ss_trig_hyst_v.SendValue(s.getADCTriggerHysteresisVolt());
    ss_gpio_in_samples.SendValue((double)s.getGPIOInSamples());
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

auto sendTriggerWindow() -> void {
    if (!g_session)
        return;
    const auto s = g_session->settings();
    ss_trig_level_v.SendValue(s.getADCTriggerLevelVolt());
    ss_trig_hyst_v.SendValue(s.getADCTriggerHysteresisVolt());
    for (auto& it : g_settings)
        if (std::string(it->param.GetName()) == "SS_TRIG_LEVEL_LOW" || std::string(it->param.GetName()) == "SS_TRIG_LEVEL_HIGH" ||
            std::string(it->param.GetName()) == "SS_TRIG_LEVEL_UNIT")
            it->param.SendValue(it->get(s));
}

auto gpioFileList() -> std::string {
    return (g_session ? g_session->settings().getGPIOOutFile() : std::string()) + "*" + CStreamSettings::getGPIOFiles();
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
        const double v = ss_samples.Value();
        if (v >= 0 && v == std::floor(v))  // a whole number of samples, 64 bit
            change([&](CStreamSettings& s) { s.setADCSamples((uint64_t)v); });
        ss_samples.SendValue((double)g_session->settings().getADCSamples());
        needUpdate = true;
    }

    if (ss_calib.IsNewValue() || _force) {
        ss_calib.Update();
        change([&](CStreamSettings& s) { s.setADCCalibration((CStreamSettings::State::_enumerated)ss_calib.Value()); });
        needUpdate = true;
    }

    // Hardware-specific ADC settings
    if (g_info.adcRanges) {
        if (ss_attenuator.IsNewValue() || _force) {
            ss_attenuator.Update();
            int value = ss_attenuator.Value();
            for (int i = 1; i <= 4; i++) {
                change([&](CStreamSettings& s) { s.setADCAttenuator(i, (value & (1 << (i - 1))) ? CStreamSettings::Attenuator::A_1_20 : CStreamSettings::Attenuator::A_1_1); });
            }
            needUpdate = true;
        }
    }

    if (g_info.adcACDC) {
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

    if (g_info.dacGain) {
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

    // triggers, the external input, GPIO: a refused value goes back to the page as the settings keep it
    for (auto& it : g_settings) {
        if (it->param.IsNewValue() || _force) {
            it->param.Update();
            const int value = it->param.Value();
            change([&](CStreamSettings& s) { return it->set(s, value); });
            it->param.SendValue(it->get(g_session->settings()));
            needUpdate = true;
        }
    }

    // the volts of the level window: both in one change, then the codes they make go back
    if (ss_trig_level_v.IsNewValue() || ss_trig_hyst_v.IsNewValue() || _force) {
        ss_trig_level_v.Update();
        ss_trig_hyst_v.Update();
        const double level = ss_trig_level_v.Value(), hyst = ss_trig_hyst_v.Value();
        change([&](CStreamSettings& s) {
            if (!s.setADCTriggerLevelVolt(level) || !s.setADCTriggerHysteresisVolt(hyst))
                return false;
            s.setADCTriggerLevelUnit(CStreamSettings::LevelUnit::VOLT);
            return true;
        });
        needUpdate = true;
    }

    // the edge, the sources and the ranges move the codes of a window in volts: send it after any change
    if (needUpdate)
        sendTriggerWindow();

    if (ss_gpio_in_samples.IsNewValue() || _force) {
        ss_gpio_in_samples.Update();
        const double v = ss_gpio_in_samples.Value();
        if (v >= 0 && v == std::floor(v))
            change([&](CStreamSettings& s) { s.setGPIOInSamples((uint64_t)v); });
        ss_gpio_in_samples.SendValue((double)g_session->settings().getGPIOInSamples());
        needUpdate = true;
    }

    if (ss_gpio_out_file.IsNewValue() || _force) {
        ss_gpio_out_file.Update();
        change([&](CStreamSettings& s) { return s.setGPIOOutFile(ss_gpio_out_file.Value()); });
        ss_gpio_out_file.SendValue(gpioFileList());
        needUpdate = true;
    }

    g_pageChanges = false;
    if (needUpdate) {
        saveConfigInFile();
    }
}

void UpdateParams(void) {
    try {
        if (g_hw && g_session) {
            static std::string last;
            auto json = session_lib::triggerStatusOf(*g_hw).toJson();
            if (json != last) {
                last = json;
                ss_trigger_status.SendValue(json);
            }
        }
        if (g_session && g_adcFile) {
            ss_pass_samples.SendValue(g_session->adcPassSamples());
            ss_writed_size.SendValue(g_session->adcWrittenSize());
        }
    } catch (std::exception& e) {
        ERROR_LOG("%s", e.what());
    }
}

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

        if (ss_settings_cmd.IsNewValue()) {
            ss_settings_cmd.Update();
            const int cmd = ss_settings_cmd.Value();
            if (cmd == 1 && g_session) {
                const bool ok = change([](CStreamSettings& s) { s.resetDefault(); });
                if (ok)
                    saveConfigInFile();
                updateUI();
                ss_settings_result.SendValue(ok ? 1 : 2);
            }
            if (cmd == 2 && g_session)
                ss_settings_json.SendValue(g_session->settings().toJson());
            ss_settings_cmd.SendValue(0);
        }

        if (ss_settings_json.IsNewValue()) {
            ss_settings_json.Update();
            const std::string json = ss_settings_json.Value();
            bool ok = false;
            if (g_session && !json.empty()) {
                ok = change([&](CStreamSettings& s) { return s.parseJson(json); });
                if (ok)
                    saveConfigInFile();
                updateUI();
            }
            ss_settings_result.SendValue(ok ? 1 : 2);
        }

        if (ss_gpio_out_start.IsNewValue()) {
            ss_gpio_out_start.Update();
            if (ss_gpio_out_start.Value() == 1) {
                if (g_session)
                    g_session->startGPIOOut();
            } else if (g_session) {
                g_session->stopGPIOOut(gpio_streaming_lib::GPIOStopReason::NORMAL);
            }
        }

        // a command: answered by setting it back to 0, so the same trigger can come again
        if (ss_soft_trigger.IsNewValue()) {
            ss_soft_trigger.Update();
            const int target = ss_soft_trigger.Value();
            if (g_hw) {
                auto f = g_hw->features();
                if (target == 1 && f.adc)
                    g_hw->adcSoftwareTrigger();
                if (target == 2 && f.dac)
                    g_hw->dacSoftwareTrigger();
                if (target == 3 && f.gpio)
                    g_hw->gpioSoftwareTrigger();
            }
            ss_soft_trigger.SendValue(0);
        }

        if (ss_gpio_start.IsNewValue()) {
            ss_gpio_start.Update();
            if (ss_gpio_start.Value() == 1) {
                if (g_session)
                    g_session->startGPIOIn();
            } else if (g_session) {
                g_session->stopGPIOIn(gpio_streaming_lib::GPIOStopReason::NORMAL);
            }
        }

    } catch (std::exception& e) {
        // an error of one request, not a reason to end the nginx worker
        ERROR_LOG("%s", e.what());
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

    void gpioOutStarted() override { ss_gpio_out_status.SendValue(1); }

    void gpioOutStopped(gpio_streaming_lib::GPIOStopReason reason, uint64_t) override {
        using R = gpio_streaming_lib::GPIOStopReason;
        switch (reason) {
            case R::NORMAL:
            case R::DISCONNECTED:
                ss_gpio_out_status.SendValue(0);
                break;
            case R::DONE:
                ss_gpio_out_status.SendValue(3);
                break;
            case R::MEM_ERROR:
                ss_gpio_out_status.SendValue(6);
                break;
            case R::MEM_MODIFY:
                ss_gpio_out_status.SendValue(7);
                break;
            case R::BROKEN:
                ss_gpio_out_status.SendValue(4);
                break;
            case R::SD_FULL:
                ss_gpio_out_status.SendValue(2);
                break;
            case R::CONFIG_ERROR:
            case R::MISSING_FILE:
            case R::EMPTY:
                ss_gpio_out_status.SendValue(8);
                break;
        }
    }
};

}  // namespace

auto makeWebCallback() -> std::shared_ptr<session_lib::SessionCallback> {
    return std::make_shared<WebCallback>();
}
