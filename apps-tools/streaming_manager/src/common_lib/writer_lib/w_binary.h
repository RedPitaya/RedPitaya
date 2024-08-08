#ifndef WRITER_LIB_WBINARY_H
#define WRITER_LIB_WBINARY_H

#include <stdint.h>

class CBinInfo{
public:
    struct BinHeader{
        uint8_t  dataFormatSize[4] = {0,0,0,0};
        uint32_t sizeCh[4] = {0,0,0,0};
        uint32_t sampleCh[4] = {0,0,0,0};
        uint64_t lostCount[4] = {0,0,0,0}; // In samples
        uint64_t oscRate[4] = {0,0,0,0};
        uint32_t sigmentLength = 0;
        BinHeader();

    };

    CBinInfo();

    uint8_t  dataFormatSize[4] = {0,0,0,0};
    uint64_t size_ch[4] = {0,0,0,0};
    uint64_t samples_ch[4] = {0,0,0,0};
    uint64_t segSamplesCount = 0;
    uint64_t segLastSamplesCount = 0;
    uint64_t segCount = 0;
    bool     lastSegState = false;
    uint64_t lostCount[4] = {0,0,0,0};
};

#endif
