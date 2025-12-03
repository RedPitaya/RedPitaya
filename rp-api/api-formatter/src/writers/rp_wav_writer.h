/**
 * $Id$
 *
 * @brief Red Pitaya data formatter.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_WAV_H__
#define __RP_WAV_H__

#include <stdint.h>
#include <memory>
#include "rp_formatter.h"
#include "common.h"

namespace rp_formatter_api{

class CWaveWriter{

public:

    CWaveWriter(uint32_t _oscRate);
    ~CWaveWriter();

    auto setEndiannes(rp_endianness_t _endiannes) -> void;
    auto resetHeaderInit() -> void;
   
    auto writeToStream(SBufferPack *_pack, std::iostream *_memory) -> bool;
    

private:

    CWaveWriter(const CWaveWriter &) = delete;
    CWaveWriter(CWaveWriter &&) = delete;
    CWaveWriter& operator=(const CWaveWriter&) =delete;
    CWaveWriter& operator=(CWaveWriter&&) =delete;
    
    struct Impl;
    // Pointer to the internal implementation
    Impl *m_pimpl;
};

}

#endif
