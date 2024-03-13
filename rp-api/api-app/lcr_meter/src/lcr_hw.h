/**
* $Id: $
*
* @brief Red Pitaya application library Impedance Analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __LCRHARDWARE_H
#define __LCRHARDWARE_H
#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <mutex>

#include "rp.h"
#include "lcrApp.h"

class CLCRHardware{

public:

    CLCRHardware();
    ~CLCRHardware();

    CLCRHardware(CLCRHardware &) = delete;
    CLCRHardware(CLCRHardware &&) = delete;

    auto setI2CShunt(lcr_shunt_t _shunt) -> lcr_error_t;
    auto getShunt() -> lcr_shunt_t;
    auto checkExtensionModuleConnection(bool _muteWarnings) -> lcr_error_t;
    auto isExtensionConnected() -> bool;
    auto getShuntValue(lcr_shunt_t _shunt) -> double;
    auto calibShunt(lcr_shunt_t _shunt,float freq) -> double;

private:
    std::mutex m_mutex;
    lcr_shunt_t m_shunt;
    bool is_connected;
};

#endif //__LCRHARDWARE_H
