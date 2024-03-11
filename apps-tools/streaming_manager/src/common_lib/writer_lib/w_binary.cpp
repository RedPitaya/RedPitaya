#include <cstring>
#include "w_binary.h"

CBinInfo::CBinInfo(){
    memset(dataFormatSize,0,sizeof(uint8_t) * 4);
    memset(size_ch,0,sizeof(uint64_t) * 4);
    memset(lostCount,0,sizeof(uint64_t) * 4);
    memset(samples_ch,0,sizeof(uint64_t) * 4);
    segSamplesCount = 0;
    segCount = 0;
    lastSegState = false;
    segLastSamplesCount = 0;
}


CBinInfo::BinHeader::BinHeader(){
    memset(dataFormatSize,0,sizeof(uint8_t) * 4);
    memset(sizeCh,0,sizeof(uint32_t) * 4);
    memset(sampleCh,0,sizeof(uint32_t) * 4);
    memset(lostCount,0,sizeof(uint64_t) * 4);
    sigmentLength = 0;
}
