#include <vector>

// Platform-specific includes
#ifdef RP_PLATFORM
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"
#include "api250-12/rp-spi.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"
#endif

// System includes
#ifndef _WIN32
#include <syslog.h>
#endif

// Project includes
#include "dac_streaming.h"
#include "dac_streaming_lib/dac_streaming_application.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "options.h"
#include "uio_lib/generator.h"

using namespace dac_streaming_lib;
using namespace uio_lib;

// ============================================================================
// Global variables
// ============================================================================
CDACStreamingApplication::Ptr g_dac_app = nullptr;
CDACStreamingManager::Ptr g_dac_manger = nullptr;
CGenerator::Ptr g_gen = nullptr;
ServerNetConfigManager::Ptr g_serverDACNetConfig = nullptr;

bool g_dac_verbMode = false;
std::atomic_bool g_dac_serverRun(false);

// ============================================================================
// Helper functions
// ============================================================================

auto setDACServer(ServerNetConfigManager::Ptr serverNetConfig) -> void
{
	g_serverDACNetConfig = serverNetConfig;
}

auto stopDACServer(CDACStreamingManager::NotifyResult x) -> void
{
	try {
		// Stop and cleanup application
		if (g_dac_app) {
			g_dac_app->stop();
			g_dac_app = nullptr;
		}

		// Cleanup manager and memory
		g_dac_manger = nullptr;
		auto memmanager = uio_lib::CMemoryManager::instance();
		memmanager->releaseMemory(MM_DAC);

		// Send status notifications
		if (g_serverDACNetConfig) {
			switch (x) {
				case CDACStreamingManager::NR_STOP:
					g_serverDACNetConfig->sendDACServerStopped();
					break;
				case CDACStreamingManager::NR_ENDED:
					g_serverDACNetConfig->sendDACServerStoppedSDDone();
					break;
				case CDACStreamingManager::NR_EMPTY:
					g_serverDACNetConfig->sendDACServerStoppedSDEmpty();
					break;
				case CDACStreamingManager::NR_BROKEN:
					g_serverDACNetConfig->sendDACServerStoppedSDBroken();
					break;
				case CDACStreamingManager::NR_MISSING_FILE:
					g_serverDACNetConfig->sendDACServerStoppedSDMissingFile();
					break;
				case CDACStreamingManager::NR_MEM_ERROR:
					g_serverDACNetConfig->sendDACServerMemoryErrorStopped();
					break;
				case CDACStreamingManager::NR_MEM_MODIFY:
					g_serverDACNetConfig->sendDACServerMemoryModifyStopped();
					break;
				case CDACStreamingManager::NR_SETTINGS_ERROR:
					g_serverDACNetConfig->sendDACServerConfigErrorStopped();
					break;
				default:
					throw runtime_error("Unknown state");
					break;
			}
		}

		// Log stop
		if (g_dac_verbMode) {
			printWithLog(LOG_NOTICE, stdout, "[Streaming] Stop dac server\n");
		}
	} catch (std::exception &e) {
		printWithLog(LOG_NOTICE, stdout, "Error: stopDACServer() %s\n", e.what());
	}
}

auto stopDACNonBlocking(CDACStreamingManager::NotifyResult x) -> void
{
	try {
		std::thread th(stopDACServer, x);
		th.detach();
	} catch (std::exception &e) {
		printWithLog(LOG_ERR, stderr, "Error: StopNonBlocking() %s\n", e.what());
	}
}

// ============================================================================
// Platform-specific initialization
// ============================================================================

#ifdef RP_PLATFORM

/**
 * Initialize calibration and configure hardware gain settings
 */
auto initPlatformCalibration(const CStreamSettings &settings, int32_t *ch_off, double *ch_gain, uint8_t channels_max) -> bool
{
	// Initialize calibration
	if (rp_CalibInit() != RP_HW_CALIB_OK) {
		aprintf(stderr, "Error init calibration\n");
		return false;
	}

	// Get gain settings for both channels
	CStreamSettings::DACGain dac_gain[2] = {settings.getDACGain(1), settings.getDACGain(2)};

	// Load calibration values for each channel
	for (uint8_t ch = 0; ch < channels_max; ++ch) {
		rp_gen_gain_calib_t mode = (dac_gain[ch].value == CStreamSettings::DACGain::X1) ? RP_GAIN_CALIB_1X : RP_GAIN_CALIB_5X;

		if (rp_CalibGetFastDACCalibValue((rp_channel_calib_t) ch, mode, &ch_gain[ch], &ch_off[ch]) != RP_HW_CALIB_OK) {
			aprintf(stderr, "Error get calibration channel: %d\n", ch);
		}
	}

	// Configure hardware gain outputs if supported
	if (rp_HPGetIsGainDACx5OrDefault()) {
		rp_max7311::rp_setGainOut(RP_MAX7311_OUT1, dac_gain[0].value == CStreamSettings::DACGain::X5 ? RP_GAIN_10V : RP_GAIN_2V);
		rp_max7311::rp_setGainOut(RP_MAX7311_OUT2, dac_gain[1].value == CStreamSettings::DACGain::X5 ? RP_GAIN_10V : RP_GAIN_2V);
	}

	return true;
}

/**
 * Create platform-specific generator with calibration
 */
auto createPlatformGenerator(const std::vector<UioT> &uioList, uint32_t dac_speed, const int32_t *ch_off, const double *ch_gain)
	-> CGenerator::Ptr
{
	for (const UioT &uio : uioList) {
		if (uio.nodeName == "rp_dac@40100000") {
			auto gen = CGenerator::create(uio, true, true, dac_speed, ClientOpt::getDACRate());
			gen->setCalibration(ch_off[0], ch_gain[0], ch_off[1], ch_gain[1]);

			if (!gen->setDacHz(dac_speed)) {
				printWithLog(LOG_ERR, stdout, "DAC rate cannot be set.\n");
				return nullptr;
			}
			return gen;
		}
	}
	return nullptr;
}

#else

/**
 * Create non-platform generator with default calibration
 */
auto createPlatformGenerator(const std::vector<UioT> &, uint32_t dac_speed, const int32_t *ch_off, const double *ch_gain) -> CGenerator::Ptr
{
	uio_lib::UioT uio_t;
	auto gen = CGenerator::create(uio_t, true, true, dac_speed, ClientOpt::getDACRate());
	gen->setCalibration(ch_off[0], ch_gain[0], ch_off[1], ch_gain[1]);
	gen->setDacHz(dac_speed);
	return gen;
}

#endif

// ============================================================================
// Streaming mode initialization
// ============================================================================

/**
 * Create network-based streaming manager
 */
auto createNetworkManager(const std::string &ip_addr_host, bool verbMode) -> CDACStreamingManager::Ptr
{
	return CDACStreamingManager::Create(ip_addr_host, verbMode);
}

/**
 * Create file-based streaming manager
 */
auto createFileManager(const CStreamSettings &settings, bool verbMode, dac_channels_t activeChannels) -> CDACStreamingManager::Ptr
{
	auto format = settings.getDACFileType();
	auto filePath = settings.getDACFile();
	auto dacRepeatMode = settings.getDACRepeat();
	auto dacRepeatCount = settings.getDACRepeatCount();
	std::string fullPath = CStreamSettings::getDACDirPath() + "/" + filePath;

	CDACStreamingManager::Ptr manager = nullptr;

	if (format.value == CStreamSettings::DataFormat::WAV) {
		manager = CDACStreamingManager::Create(CDACStreamingManager::WAV_TYPE,
											   fullPath,
											   dacRepeatMode,
											   dacRepeatCount,
											   settings.getMemoryBlockSize(),
											   verbMode);
	} else if (format.value == CStreamSettings::DataFormat::TDMS) {
		manager = CDACStreamingManager::Create(CDACStreamingManager::TDMS_TYPE,
											   fullPath,
											   dacRepeatMode,
											   dacRepeatCount,
											   settings.getMemoryBlockSize(),
											   verbMode);
	}

	if (manager) {
		// Register stop callback
		manager->notifyStop.connect([](CDACStreamingManager::NotifyResult status) { stopDACNonBlocking(status); });

		// Validate channels
		if (!manager->getChannels(activeChannels)) {
			printWithLog(LOG_ERR, stdout, "There are no channels in the file.\n");
			return nullptr;
		}
	}

	return manager;
}

// ============================================================================
// Memory management
// ============================================================================

/**
 * Initialize and allocate memory for DAC streaming
 */
auto initializeMemory(const CStreamSettings &settings, dac_channels_t activeChannels) -> bool
{
	auto memmanager = uio_lib::CMemoryManager::instance();

	// Configure memory
	memmanager->releaseMemory(MM_DAC);
	memmanager->setReserverdMemory(uio_lib::MM_DAC, settings.getDACSize());

	// Validate memory size
	auto ramSize = memmanager->getReserverdMemory(MM_DAC);
	if (ramSize < memmanager->getMinRAMSize(MM_DAC)) {
		printWithLog(LOG_ERR, stdout, "Not enough memory for MM_DAC mode\n");
		stopDACNonBlocking(CDACStreamingManager::NR_MEM_ERROR);
		return false;
	}

	// Allocate memory blocks
	auto mbSize = memmanager->getMemoryBlockSize();
	auto freeblocks = ramSize / mbSize;
	auto reservedBlocks = memmanager->reserveMemory(MM_DAC, freeblocks, activeChannels.count());

	if (reservedBlocks == 0) {
		printWithLog(LOG_ERR, stdout, "Can't reserve memory via memory manager\n");
		stopDACNonBlocking(CDACStreamingManager::NR_MEM_ERROR);
		return false;
	}

	// Generate buffer structures
	g_dac_manger->getBufferManager()->generateBuffersEmptyDAC(activeChannels, memmanager->getRegions(MM_DAC), DataLib::sizeHeader());

	return true;
}

// ============================================================================
// Main DAC server control
// ============================================================================

/**
 * Start DAC server with specified configuration
 */
auto startDACServer(bool verbMode, dac_channels_t activeChannels) -> void
{
	if (!g_serverDACNetConfig) {
		return;
	}

	// Reset global state
	g_gen = nullptr;
	g_dac_app = nullptr;
	g_dac_manger = nullptr;
	g_dac_verbMode = verbMode;

	try {
		// Get configuration
		auto memmanager = uio_lib::CMemoryManager::instance();
		CStreamSettings settings = g_serverDACNetConfig->getSettings();
		g_dac_serverRun = true;

		auto use_file = settings.getDACPassMode();
		auto dac_speed = settings.getDACSpeed();
		auto ip_addr_host = "127.0.0.1";

		// Initialize calibration data
		int32_t ch_off[MAX_DAC_CHANNELS] = {0, 0};
		double ch_gain[MAX_DAC_CHANNELS] = {1, 1};
		std::vector<UioT> uioList = GetUioList();

#ifdef RP_PLATFORM
		auto channels_max = ClientOpt::getDACChannels();
		if (!initPlatformCalibration(settings, ch_off, ch_gain, channels_max)) {
			stopDACNonBlocking(CDACStreamingManager::NR_SETTINGS_ERROR);
			return;
		}
#endif

		// Create generator
		g_gen = createPlatformGenerator(uioList, dac_speed, ch_off, ch_gain);
		if (!g_gen) {
			printWithLog(LOG_ERR, stderr, "[Streaming] Error init generator module\n");
			stopDACNonBlocking(CDACStreamingManager::NR_SETTINGS_ERROR);
			return;
		}

		// Create streaming manager based on mode
		if (use_file.value == CStreamSettings::DACPassMode::DAC_NET) {
			g_dac_manger = createNetworkManager(ip_addr_host, verbMode);
		} else if (use_file.value == CStreamSettings::DACPassMode::DAC_FILE) {
			g_dac_manger = createFileManager(settings, verbMode, activeChannels);
			if (!g_dac_manger) {
				g_serverDACNetConfig->sendDACServerStoppedSDBroken();
				return;
			}
		}

		// Initialize memory
		if (!initializeMemory(settings, activeChannels)) {
			return;
		}

		// Create and start application
		g_dac_app = std::make_shared<CDACStreamingApplication>(g_dac_manger, g_gen);
		g_gen->setDataSize(settings.getMemoryBlockSize());
		g_dac_app->runNonBlock();

		// Send appropriate started notification
		if (g_dac_manger->isLocalMode()) {
			g_serverDACNetConfig->sendDACServerStartedSD();
		} else {
			g_serverDACNetConfig->sendDACServerStarted();
		}

		if (g_dac_verbMode) {
			printWithLog(LOG_ERR, stderr, "[Streaming] Start dac server\n");
		}
	} catch (std::exception &e) {
		printWithLog(LOG_ERR, stderr, "Error: startDACServer() %s\n", e.what());
	}
}
