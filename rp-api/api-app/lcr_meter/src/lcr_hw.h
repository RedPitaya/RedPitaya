/**
* $Id: $
*
* @brief Red Pitaya application library Impedance Analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*/

#ifndef __LCRHARDWARE_H
#define __LCRHARDWARE_H
#include <stdbool.h>
#include <stdint.h>
#include <mutex>
#include <string>

#include "lcrApp.h"
#include "rp.h"

class CLCRHardware {

   public:
    CLCRHardware();
    ~CLCRHardware();

    CLCRHardware(CLCRHardware&) = delete;
    CLCRHardware(CLCRHardware&&) = delete;

    auto setI2CShunt(lcr_shunt_t _shunt) -> lcr_error_t;
    auto getShunt() -> lcr_shunt_t;
    auto checkExtensionModuleConnection(bool _muteWarnings) -> lcr_error_t;
    auto isExtensionConnected(bool _muteWarnings = false) -> bool;
    auto getShuntValue(lcr_shunt_t _shunt) -> double;
    auto calibShunt(lcr_shunt_t _shunt, float freq) -> double;

   private:
    std::mutex m_mutex;
    lcr_shunt_t m_shunt;
    bool is_connected;
};

#endif  //__LCRHARDWARE_H
