#ifndef WRITER_LIB_WBINARY_H
#define WRITER_LIB_WBINARY_H

#include <stdint.h>

class CBinInfo{
public:
    struct BinHeader{
        uint8_t  dataFormatSize[4];
        uint32_t sizeCh[4];
        uint32_t sampleCh[4];
        uint64_t lostCount[4];
        uint32_t sigmentLength;
        BinHeader();

    };

    CBinInfo();

    uint8_t  dataFormatSize[4];
    uint64_t size_ch[4];
    uint64_t samples_ch[4];
    uint64_t segSamplesCount;
    uint64_t segLastSamplesCount;
    uint64_t segCount;
    bool     lastSegState;
    uint64_t lostCount[4];
};

#endif
