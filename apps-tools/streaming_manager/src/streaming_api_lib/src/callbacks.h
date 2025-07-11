#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <stdint.h>
#include <string>
#include <vector>

struct ADCChannel {
    uint32_t samples = 0;
    uint8_t bitsBySample = 0;
    uint64_t fpgaLost = 0;
    bool attenuator_1_20 = 0;
    uint32_t baseRate = 0;
    uint8_t adcBaseBits = 0;
    uint64_t packId = 0;
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

class ADCCallback {
   public:
    virtual ~ADCCallback() {}
    virtual void recievePack(ADCStreamClient*, ADCPack& pack) {}
    virtual void connected(ADCStreamClient*, std::string) {}
    virtual void disconnected(ADCStreamClient*, std::string) {}
    virtual void error(ADCStreamClient*, std::string, int) {}
    virtual void stopped(ADCStreamClient*, std::string) {}
    virtual void stoppedNoActiveChannels(ADCStreamClient*, std::string) {}
    virtual void stoppedMemError(ADCStreamClient*, std::string) {}
    virtual void stoppedMemModify(ADCStreamClient*, std::string) {}
    virtual void stoppedSDFull(ADCStreamClient*, std::string) {}
    virtual void stoppedSDDone(ADCStreamClient*, std::string) {}

    virtual void configConnected(ADCStreamClient*, std::string) {}
    virtual void configError(ADCStreamClient*, std::string, int) {}
    virtual void configErrorTimeout(ADCStreamClient*, std::string) {}
};

class DACCallback {
   public:
    virtual ~DACCallback() {}
    virtual void sendedPack(DACStreamClient*, uint32_t ch1_size, uint32_t ch2_size) {}
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

    virtual void configConnected(DACStreamClient*, std::string) {}
    virtual void configError(DACStreamClient*, std::string, int) {}
    virtual void configErrorTimeout(DACStreamClient*, std::string) {}
};
#endif
