#include "rp-i2c-max7311.h"
#include "i2c/i2c.h"
#include <iostream>


int max7311::initController(const char *i2c_dev_path,  char address){
    bool state = true;
    char value = 0x00;
    // setup null level on all out ports
    state = (write_to_i2c(i2c_dev_path , address , 0x02, value) == 0) & state;
    state = (write_to_i2c(i2c_dev_path , address , 0x03, value) == 0) & state;
    // setup all port in outgoing mode
    state = (write_to_i2c(i2c_dev_path , address , 0x06, value) == 0) & state;
    state = (write_to_i2c(i2c_dev_path , address , 0x07, value) == 0) & state;

    // check all regs
    state = (read_from_i2c(i2c_dev_path , address , 0x02, value) == 0) & state;
    state = (value == 0x00) & state;
    state = (read_from_i2c(i2c_dev_path , address , 0x03, value) == 0) & state;
    state = (value == 0x00) & state;
    state = (read_from_i2c(i2c_dev_path , address , 0x06, value) == 0) & state;
    state = (value == 0x00) & state;
    state = (read_from_i2c(i2c_dev_path , address , 0x07, value) == 0) & state;
    state = (value == 0x00) & state;
    return state;
}

int max7311::initControllerDefault(){
    return initController(MAX7311_DEFAULT_DEV, MAX7311_DEFAULT_ADDRESS);
}

int max7311::setPIN(unsigned short pin,bool state){
    return setPIN_EX(MAX7311_DEFAULT_DEV, MAX7311_DEFAULT_ADDRESS, pin, state);
}

int max7311::getPIN(unsigned short pin){
    return getPIN_EX(MAX7311_DEFAULT_DEV, MAX7311_DEFAULT_ADDRESS, pin);
}

int max7311::setPIN_EX(const char *i2c_dev_path,  char address, unsigned short pin,bool state){
    char value = 0;
    char reg_addr = 0x02;
    if (pin > 0x80) {
        reg_addr = 0x03;
        pin = pin >> 8;
    }

    if (read_from_i2c(i2c_dev_path , address , reg_addr, value) == -1)
        return -1;
    
    value = (value & ~pin) | ((state ? 0xFF : 0) & pin);


    if (write_to_i2c(i2c_dev_path , address , reg_addr, value) == -1) 
        return -1;
    
    if (read_from_i2c(i2c_dev_path , address , reg_addr, value) == -1)
        return -1;
    return 0;
}
   

int max7311::getPIN_EX(const char *i2c_dev_path,  char address, unsigned short pin){
    char value = 0;
    char reg_addr = 0x02;
    if (pin > 0x80) {
        reg_addr = 0x03;
        pin = pin >> 8;
    }

    if (read_from_i2c(i2c_dev_path , address , reg_addr, value) == -1)
        return -1;

    return  (pin & value) > 0;
}


int  rp_max7311::rp_initController(){
    return max7311::initControllerDefault();
}


int  rp_max7311::rp_setAC_DC(char port,char mode){
    switch(port){
        case RP_MAX7311_IN1:
            if (max7311::setPIN(PIN_0,false) == -1) return -1;
            if (max7311::setPIN(PIN_1,mode)  == -1) return -1;
            break;
        case RP_MAX7311_IN2:
            if (max7311::setPIN(PIN_2,false) == -1) return -1;
            if (max7311::setPIN(PIN_3,mode)  == -1) return -1;
            break;
        default:
            return -1;
    }
    return 0;
}


int  rp_max7311::rp_getAC_DC(char port){
    switch(port){
        case RP_MAX7311_IN1:
            return max7311::getPIN(PIN_1);
        case RP_MAX7311_IN2:
            return max7311::getPIN(PIN_3);
        default:
            return -1;
    }
}



int  rp_max7311::rp_setAttenuator(char port,char mode){
    switch(port){
        case RP_MAX7311_IN1:
            if (max7311::setPIN(PIN_4,false) == -1) return -1;
            if (max7311::setPIN(PIN_5,mode)  == -1) return -1;
            break;
        case RP_MAX7311_IN2:
            if (max7311::setPIN(PIN_6,false) == -1) return -1;
            if (max7311::setPIN(PIN_7,mode)  == -1) return -1;
            break;
        default:
            return -1;
    }
    return 0;
}


int  rp_max7311::rp_getAttenuator(char port){
    switch(port){
        case RP_MAX7311_IN1:
            return max7311::getPIN(PIN_5);
        case RP_MAX7311_IN2:
            return max7311::getPIN(PIN_7);
        default:
            return -1;
    }
}


int  rp_max7311::rp_setGainOut(char port,char mode){
    switch(port){
        case RP_MAX7311_OUT1:
            if (max7311::setPIN(PIN_8,false) == -1) return -1;
            if (max7311::setPIN(PIN_9,mode)  == -1) return -1;
            break;
        case RP_MAX7311_OUT2:
            if (max7311::setPIN(PIN_10,false) == -1) return -1;
            if (max7311::setPIN(PIN_11,mode)  == -1) return -1;
            break;
        default:
            return -1;
    }
    return 0;
}

int  rp_max7311::rp_getGainOut(char port){
    switch(port){
        case RP_MAX7311_OUT1:
            return max7311::getPIN(PIN_9);
        case RP_MAX7311_OUT2:
            return max7311::getPIN(PIN_11);
        default:
            return -1;
    }
}