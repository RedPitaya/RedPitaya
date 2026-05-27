#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <array>
#include <stdint.h>
#include <string>
#include <vector>

struct ADCChannel {
    uint32_t samples = 0;
    uint8_t bitsPerSample = 0;
    uint64_t fpgaLost = 0;
    bool attenuator_1_20 = false;
    uint32_t baseRate = 0;
    uint8_t adcBaseBits = 0;
    uint64_t packId = 0;
    int64_t timeCapture = 0;
    std::vector<int16_t> raw;
};

struct ADCPack {
    std::string host;
    ADCChannel channel1;
    ADCChannel channel2;
    ADCChannel channel3;
    ADCChannel channel4;
};

class ADCStreamClient;
class DACStreamClient;
class ConfigStreamClient;

class ConfigCallback {
   public:
    virtual ~ConfigCallback() {}
    virtual void configConnected(ConfigStreamClient*, std::string) {}
    virtual void configError(ConfigStreamClient*, std::string, int) {}
    virtual void configErrorTimeout(ConfigStreamClient*, std::string) {}
    virtual void configErrorFileMissed(ConfigStreamClient*, std::string) {}
    virtual void configMemoryBlockSize(ConfigStreamClient*, std::string, size_t) {}
	virtual void configActiveChannels(ConfigStreamClient *, std::string, std::array<bool, 4>) {}

	virtual void configSuccessSend(ConfigStreamClient *, std::string) {}
	virtual void configFailSend(ConfigStreamClient*, std::string) {}
    virtual void configSuccessSave(ConfigStreamClient*, std::string) {}
    virtual void configFailSave(ConfigStreamClient*, std::string) {}

    virtual void configGetNewSettings(ConfigStreamClient*, std::string) {}
    virtual void configGetNewSettingsItem(ConfigStreamClient*, std::string) {}

    virtual void adcServerStopped(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedNoActiveChannels(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedMemError(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedMemModify(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedSDFull(ConfigStreamClient*, std::string) {}
    virtual void adcServerStoppedSDDone(ConfigStreamClient*, std::string) {}
    virtual void adcServerStartedTCP(ConfigStreamClient*, std::string) {}
    virtual void adcServerStartedSD(ConfigStreamClient*, std::string) {}
    virtual void adcServerStartedFPGA(ConfigStreamClient*, std::string) {}

    virtual void dacServerStartedTCP(ConfigStreamClient*, std::string) {}
    virtual void dacServerStartedSD(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedMemError(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedMemModify(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedConfigError(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedFileMissed(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDDone(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDEmpty(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDBroken(ConfigStreamClient*, std::string) {}
    virtual void dacServerStoppedSDMissing(ConfigStreamClient*, std::string) {}

    virtual void sigInt() {};
};

class ADCCallback {
   public:
    virtual ~ADCCallback() {}
    virtual void receivePack(ADCStreamClient*, ADCPack& pack) {}
    virtual void connected(ADCStreamClient*, std::string) {}
    virtual void disconnected(ADCStreamClient*, std::string) {}
    virtual void error(ADCStreamClient*, std::string, int) {}
};

class DACCallback {
   public:
    virtual ~DACCallback() {}
    virtual void sentPack(DACStreamClient*, uint32_t ch1_size, uint32_t ch2_size) {}
    virtual void connected(DACStreamClient*, std::string) {}
    virtual void disconnected(DACStreamClient*, std::string) {}
    virtual void error(DACStreamClient*, std::string, int) {}
    virtual void stopped(DACStreamClient*, std::string) {}
    virtual void stoppedFileEnd(DACStreamClient*, std::string) {}
    virtual void stoppedFileBroken(DACStreamClient*, std::string) {}
    virtual void stoppedEmpty(DACStreamClient*, std::string) {}
    virtual void stoppedMissingFile(DACStreamClient*, std::string) {}
    virtual void stoppedMemError(DACStreamClient*, std::string) {}
    virtual void stoppedMemModify(DACStreamClient*, std::string) {}

    virtual bool streamData8Bit(DACStreamClient*, int8_t* ch1_8Bit, int8_t* ch2_8Bit, size_t size) { return true; }
    virtual bool streamData16Bit(DACStreamClient*, int16_t* ch1_16Bit, int16_t* ch2_16Bit, size_t size) { return true; }
};
#endif
