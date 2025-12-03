/**
 * $Id$
 *
 * @brief Red Pitaya data formatter.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_TDMS_H__
#define __RP_TDMS_H__

#include <stdint.h>
#include <memory>
#include "rp_formatter.h"
#include "common.h"


namespace rp_formatter_api{

class CTDMSWriter{

public:

    CTDMSWriter(uint32_t _oscRate);
    ~CTDMSWriter();

    auto writeToStream(SBufferPack *_pack, std::iostream *_memory) -> bool;
 

private:

    CTDMSWriter(const CTDMSWriter &) = delete;
    CTDMSWriter(CTDMSWriter &&) = delete;
    CTDMSWriter& operator=(const CTDMSWriter&) =delete;
    CTDMSWriter& operator=(CTDMSWriter&&) =delete;
    
    struct Impl;
    // Pointer to the internal implementation
    Impl *m_pimpl;
};

}

#endif
