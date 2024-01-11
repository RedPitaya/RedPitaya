/**
 * $Id$
 *
 * @brief Red Pitaya data formatter.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __RP_CSV_H__
#define __RP_CSV_H__

#include <stdint.h>
#include <memory>
#include "rp_formatter.h"
#include "common.h"


namespace rp_formatter_api{

class CCSVWriter{

public:

    CCSVWriter(uint32_t _oscRate);
    ~CCSVWriter();

    auto writeToStream(SBufferPack *_pack, std::iostream *_memory) -> bool;
    auto resetHeaderInit() -> void;


private:

    CCSVWriter(const CCSVWriter &) = delete;
    CCSVWriter(CCSVWriter &&) = delete;
    CCSVWriter& operator=(const CCSVWriter&) =delete;
    CCSVWriter& operator=(CCSVWriter&&) =delete;
    
    struct Impl;
    // Pointer to the internal implementation
    Impl *m_pimpl;
};

}

#endif
