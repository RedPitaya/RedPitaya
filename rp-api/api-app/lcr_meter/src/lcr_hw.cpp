/**
* $Id: $
*
* @brief Red Pitaya application Impedance analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <atomic>
#include <vector>

#include "lcr_hw.h"
#include "common.h"
#include "lcrApp.h"
#include "rp-i2c-max7311.h"
#include "rp_hw-profiles.h"

#define I2C_SLAVE_FORCE 		   		0x0706
#define EXPANDER_ADDR            	   	0x20

double G_HW_CALIB[6/*shunt*/][6/*freq*/] =
//100    1k     10k    100k     1M
{{1E-12,1E-12, 1E-12, 1E-12, 1E-12 , 1E-12}, // 10
 {1E-12,1E-12, 1E-12, 1E-12, 1E-12 , 1E-12}, // 100
 {1E-12,1E-12, 1E-12, 1E-12, 1E-12 , 1E-12}, // 1k
 {-1160E-12,-1160E-12, -113E-12, -13E-12, 176E-12, 176E-12}, // 10k
 {-146E-12,-146E-12, -17E-12, 175E-12, 288E-12, 288E-12},     // 100k
 {-40E-12,-40E-12, 230E-12, 290E-12, 170E-12, 170E-12} // 1M
};

/* R shunt values definition */
const double G_SHUNT_TABLE[] =
{10, 1e2, 1e3, 1e4, 1e5, 1.3e6};


 auto getModel() -> rp_HPeModels_t{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        WARNING("Can't get board model\n");
    }
    return c;
}

CLCRHardware::CLCRHardware(){
    m_shunt = RP_LCR_S_NOT_INIT; // Not init
}

CLCRHardware::~CLCRHardware(){

}

auto CLCRHardware::isExtensionConnected() -> bool{
    checkExtensionModuleConnection(false);
    return is_connected;
}

auto CLCRHardware::setI2CShunt(lcr_shunt_t _shunt) -> lcr_error_t {
    if (_shunt == RP_LCR_S_NOT_INIT) return RP_LCR_HW_ERROR;
    if (!is_connected) {
        if (!isExtensionConnected()){
            return RP_LCR_HW_MISSING_DEVICE;
        }
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_shunt == _shunt) return RP_LCR_OK;
    int  dat;
    int  fd;
    int  status;
    char str [1+2*11];

    // parse input arguments
    dat = 1 << (int)_shunt;

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        WARNING("Cannot open the I2C device: %d", fd);
        is_connected = false;
        return RP_LCR_HW_CANT_OPEN;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
        WARNING("Unable to set the I2C address");
        is_connected = false;
        return RP_LCR_HW_MISSING_DEVICE;
    }

    // Write to expander
    str [0] = 0; // set address to 0
    str [1+0x00] = 0x00; // IODIRA - set all to output
    str [1+0x01] = 0x00; // IODIRB - set all to output
    str [1+0x02] = 0x00; // IPOLA
    str [1+0x03] = 0x00; // IPOLB
    str [1+0x04] = 0x00; // GPINTENA
    str [1+0x05] = 0x00; // GPINTENB
    str [1+0x06] = 0x00; // DEFVALA
    str [1+0x07] = 0x00; // DEFVALB
    str [1+0x08] = 0x00; // INTCONA
    str [1+0x09] = 0x00; // INTCONB
    str [1+0x0A] = 0x00; // IOCON
    str [1+0x0B] = 0x00; // IOCON
    str [1+0x0C] = 0x00; // GPPUA
    str [1+0x0D] = 0x00; // GPPUB
    str [1+0x0E] = 0x00; // INTFA
    str [1+0x0F] = 0x00; // INTFB
    str [1+0x10] = 0x00; // INTCAPA
    str [1+0x11] = 0x00; // INTCAPB
    str [1+0x12] = (dat >> 0) & 0xff; // GPIOA
    str [1+0x13] = (dat >> 8) & 0xff; // GPIOB
    str [1+0x14] = (dat >> 0) & 0xff; // OLATA
    str [1+0x15] = (dat >> 8) & 0xff; // OLATB
    status = write(fd, str, 1+2*11);

    if (!status) {
        close(fd);
        WARNING("Error I2C write")
        is_connected = false;
        return RP_LCR_HW_ERROR;
    }

    close(fd);
    m_shunt = _shunt;
    return RP_LCR_OK;
}

auto CLCRHardware::checkExtensionModuleConnection(bool _muteWarnings) -> lcr_error_t{
    std::lock_guard<std::mutex> lock(m_mutex);
    int status;
    int fd;
    char buf[2];

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        if (!_muteWarnings)
            WARNING("Cannot open the I2C device: %d", fd);
        is_connected = false;
        return RP_LCR_HW_CANT_OPEN;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
        if (!_muteWarnings)
            WARNING("Unable to set the I2C address: %d", status);
        is_connected = false;
        return RP_LCR_HW_MISSING_DEVICE;
    }

    int retVal = -90;
    retVal = read(fd, buf, 1);

    if(retVal < 0) {
        close(fd);
        if (!_muteWarnings)
            WARNING("I2C address resolving failed: %d", retVal);
        is_connected = false;
        return RP_LCR_HW_ERROR_DETECT;
    }
    close(fd);
    is_connected = true;

    // This trick for check LCR module in 250-12. Because two chips are on the same address
    static auto model = getModel();
    switch(model){
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:{
            auto maxCheck = rp_max7311::rp_check();
            if (maxCheck == 1) is_connected = false;
            break;
        }
        default:
            break;
    }
    return RP_LCR_OK;
}

auto CLCRHardware::getShunt() -> lcr_shunt_t{
    return m_shunt;
}

auto CLCRHardware::getShuntValue(lcr_shunt_t _shunt) -> double{
    if (_shunt >= 0 && _shunt <= 5){
        return G_SHUNT_TABLE[_shunt];
    }
    WARNING("Value for shunt %d not defined",_shunt)
    return 1;
}

auto CLCRHardware::calibShunt(lcr_shunt_t _shunt,float freq) -> double{
    // auto getIndex = [](int x){
    //     int index = -1;
    //     do {
    //         index++;
    //         x /= 10;
    //     } while(x > 5);
    //     return index;
    // };
    auto s = getShuntValue(_shunt);
    if (_shunt == RP_LCR_S_NOT_INIT) return 1;
    return s;
    // Incorrect logic
    // double c_calib = G_HW_CALIB[_shunt][getIndex(freq)];
    // double w_out = 2 * M_PI * freq;
    // double r_RC = (s * (1.0 / (w_out * c_calib))) / (s + (1.0 / (w_out * c_calib)));
    // return r_RC;
}


