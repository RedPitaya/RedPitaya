/**
 * $Id: $
 *
 * @brief Red Pitaya library - LA api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <map>
#include <memory>
#include "decoders/decoder.h"
#include "decoders/can_decoder.h"
#include "decoders/uart_decoder.h"
#include "decoders/spi_decoder.h"
#include "decoders/i2c_decoder.h"
#include "rp_la.h"
#include "rp_la_api.h"
#include "rp.h"

namespace rp_la{

#define MAX_LINES 8

struct CLAController::Impl {

    auto open() -> void;
    auto close() -> void;

    bool m_isOpen = false;
    RP_DIGITAL_CHANNEL_DIRECTIONS m_triggers[RP_MAX_DIGITAL_CHANNELS];
    std::map<std::string, std::shared_ptr<Decoder>> m_decoders;
};

CLAController::CLAController()
{
    m_pimpl = new Impl();
    m_pimpl->open();
    for(int i = 0; i < RP_MAX_DIGITAL_CHANNELS; i++){
        m_pimpl->m_triggers[i].channel = (RP_DIGITAL_CHANNEL)i;
        m_pimpl->m_triggers[i].direction = RP_DIGITAL_DONT_CARE;
    }
}

CLAController::~CLAController(){
    m_pimpl->close();
    delete m_pimpl;
}

auto CLAController::setMode(la_Mode_t mode) -> void{
    if (!m_pimpl->m_isOpen){
        ERROR_LOG("The LA device is not open")
        return;
    }

    switch (mode)
    {
        case LA_BASIC:
            rp_SetPolarity(0);
            break;

        case LA_PRO:
            rp_SetPolarity(0xffff);
            break;

        default:
            ERROR_LOG("Unknown mode")
            break;
    }
}

auto CLAController::setTrigger(uint8_t channel, la_Trigger_Mode_t mode) -> void{
    if (channel > MAX_LINES) {
        ERROR_LOG("The line is larger than acceptable")
        return;
    }
    switch (mode)
    {
        case LA_NONE:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DONT_CARE;
            break;

        case LA_LOW:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_LOW;
            break;

        case LA_HIGH:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_HIGH;
            break;

        case LA_RISING:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_RISING;
            break;

        case LA_FALLING:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_FALLING;
            break;

        case LA_RISING_OR_FALLING:
            m_pimpl->m_triggers[channel].direction = RP_DIGITAL_DIRECTION_RISING_OR_FALLING;
            break;

        default:
            ERROR_LOG("Undefined trigger mode")
            break;
    }
}

auto CLAController::getTrigger(uint8_t channel) -> la_Trigger_Mode_t{
    if (channel > MAX_LINES) {
        ERROR_LOG("The line is larger than acceptable")
        return LA_ERROR;
    }

    switch (m_pimpl->m_triggers[channel].direction)
    {
        case RP_DIGITAL_DONT_CARE: return LA_NONE;
        case RP_DIGITAL_DIRECTION_LOW: return LA_LOW;
        case RP_DIGITAL_DIRECTION_HIGH: return LA_HIGH;
        case RP_DIGITAL_DIRECTION_RISING: return LA_RISING;
        case RP_DIGITAL_DIRECTION_FALLING: return LA_FALLING;
        case RP_DIGITAL_DIRECTION_RISING_OR_FALLING: return LA_RISING_OR_FALLING;
        default:
            ERROR_LOG("Undefined trigger")
            break;
    }
    return LA_ERROR;
}

auto CLAController::resetTriggers() -> void{
    for(int i = 0; i < RP_MAX_DIGITAL_CHANNELS; i++){
        m_pimpl->m_triggers[i].direction = RP_DIGITAL_DONT_CARE;
    }
}

auto CLAController::softwareTrigger() -> bool{
    if (!m_pimpl->m_isOpen){
        ERROR_LOG("The LA device is not open")
        return false;
    }
    return rp_SoftwareTrigger();
}

auto CLAController::setEnableRLE(bool enable) -> bool{
    if (!m_pimpl->m_isOpen){
        ERROR_LOG("The LA device is not open")
        return false;
    }

    return rp_EnableDigitalPortDataRLE(enable);
}

auto CLAController::isRLEEnable() -> bool{
    bool enable;
    rp_IsRLEEnable(&enable);
    return enable;
}

auto CLAController::addDecoder(std::string name, la_Decoder_t decoder) -> bool{

}

auto CLAController::removeDecoder(std::string name) -> bool{

}

auto CLAController::removeAllDecoders() -> void{

}

auto CLAController::Impl::open() -> void{
    if (rp_OpenUnit() == RP_OK){
        m_isOpen = true;
    }
}

auto CLAController::Impl::close() -> void{
    if (rp_CloseUnit() == RP_OK){
        m_isOpen = false;
    }
}



}