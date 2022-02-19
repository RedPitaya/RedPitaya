#include "rp-i2c-max7311.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include "rp_hw.h"

unsigned long g_sleep_time = 50 * 1000;


// I2C Address of device
#define MAX7311_DEFAULT_ADDRESS_1_1	0x20
#define MAX7311_DEFAULT_ADDRESS_1_2	0x21
#define MAX7311_DEFAULT_DEV     "/dev/i2c-0"

char g_I2C_address = 0;

pthread_mutex_t g_max_i2c_mutex = PTHREAD_MUTEX_INITIALIZER;

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int max7311::initController(const char *i2c_dev_path,  char address){
	pthread_mutex_lock(&g_max_i2c_mutex);
    if (rp_I2C_InitDevice(i2c_dev_path,address) != RP_HW_OK){
		pthread_mutex_unlock(&g_max_i2c_mutex);
        return false;
    }
    rp_I2C_setForceMode(true);
    bool state = true;
    uint8_t value = 0xAA;
    // setup reset relay 
    state = (rp_I2C_SMBUS_Write(0x02, value) == RP_HW_OK) & state;
    state = (rp_I2C_SMBUS_Write(0x03, value) == RP_HW_OK) & state;
    usleep(g_sleep_time);
    value = 0x00;
    // setup null level on all out ports
    state = (rp_I2C_SMBUS_Write(0x02, value) == RP_HW_OK) & state;
    state = (rp_I2C_SMBUS_Write(0x03, value) == RP_HW_OK) & state;
    // setup all port in outgoing mode
    value = 0x00;
    state = (rp_I2C_SMBUS_Write(0x06, value) == RP_HW_OK) & state;
    state = (rp_I2C_SMBUS_Write(0x07, value) == RP_HW_OK) & state;

    // check all regs
    state = (rp_I2C_SMBUS_Read(0x02, &value) == RP_HW_OK) & state;
    state = (value == 0x00) & state;
    state = (rp_I2C_SMBUS_Read(0x03, &value) == RP_HW_OK) & state;
    state = (value == 0x00) & state;
    state = (rp_I2C_SMBUS_Read(0x06, &value) == RP_HW_OK) & state;
    state = (value == 0x00) & state;
    state = (rp_I2C_SMBUS_Read(0x07, &value) == RP_HW_OK) & state;
    state = (value == 0x00) & state;
	pthread_mutex_unlock(&g_max_i2c_mutex);  
    return state;
}

int max7311::initControllerDefault(){
    return initController(MAX7311_DEFAULT_DEV, getDefaultAddress());
}

char max7311::getDefaultAddress(){
    if (g_I2C_address) return g_I2C_address;
    g_I2C_address = MAX7311_DEFAULT_ADDRESS_1_1;
    auto model = exec("fw_printenv -n hw_rev");   
    std::size_t found = model.find("STEM_250-12_v1.2");
    if (found != std::string::npos) g_I2C_address = MAX7311_DEFAULT_ADDRESS_1_2;
    //printf("initController: Model = %s I2C address: 0x%x\n",model.c_str(),g_I2C_address);
    return g_I2C_address;
}

int max7311::setPIN(unsigned short pin,bool state){
    return setPIN_EX(MAX7311_DEFAULT_DEV, getDefaultAddress(), pin, state);
}

int max7311::getPIN(unsigned short pin){
    return getPIN_EX(MAX7311_DEFAULT_DEV, getDefaultAddress(), pin);
}

int max7311::setPIN_EX(const char *i2c_dev_path,  char address, unsigned short pin,bool state){
  	pthread_mutex_lock(&g_max_i2c_mutex);

    if (rp_I2C_InitDevice(i2c_dev_path,address) != RP_HW_OK){
    	pthread_mutex_unlock(&g_max_i2c_mutex);
        return -1;
    }
    rp_I2C_setForceMode(true);
    uint8_t value = 0;
    char reg_addr = 0x02;
    if (pin > 0x80) {
        reg_addr = 0x03;
        pin = pin >> 8;
    }

    if (rp_I2C_SMBUS_Read( reg_addr, &value ) != RP_HW_OK){
	    pthread_mutex_unlock(&g_max_i2c_mutex);   
        return -1;
    }
    
    value = (value & ~pin) | ((state ? 0xFF : 0) & pin);


    if (rp_I2C_SMBUS_Write(reg_addr, value) != RP_HW_OK){
    	pthread_mutex_unlock(&g_max_i2c_mutex);
        return -1;
    }
  	pthread_mutex_unlock(&g_max_i2c_mutex);
    return 0;
}
   

int max7311::getPIN_EX(const char *i2c_dev_path,  char address, unsigned short pin){
  	pthread_mutex_lock(&g_max_i2c_mutex);

    if (rp_I2C_InitDevice(i2c_dev_path,address) != RP_HW_OK){
  	    pthread_mutex_unlock(&g_max_i2c_mutex);        
        return -1;
    }
    rp_I2C_setForceMode(true);
    uint8_t value = 0;
    char reg_addr = 0x02;
    if (pin > 0x80) {
        reg_addr = 0x03;
        pin = pin >> 8;
    }

    if (rp_I2C_SMBUS_Read( reg_addr, &value) != RP_HW_OK){
       	pthread_mutex_unlock(&g_max_i2c_mutex);
        return -1;
    }
  	pthread_mutex_unlock(&g_max_i2c_mutex);
    return  (pin & value) > 0;
}

int max7311::setPIN_GROUP(unsigned short pin_group,int state){
    return setPIN_GROUP_EX(MAX7311_DEFAULT_DEV, getDefaultAddress(), pin_group, state);
}

int max7311::setPIN_GROUP_EX(const char *i2c_dev_path, char address, unsigned short pin_group,int state){
  	pthread_mutex_lock(&g_max_i2c_mutex);

    if (rp_I2C_InitDevice(i2c_dev_path,address) != RP_HW_OK){
      	pthread_mutex_unlock(&g_max_i2c_mutex);
        return -1;
    }
    rp_I2C_setForceMode(true);
    uint8_t value = 0;
    char flag = 0;
    char reg_addr = 0x02;
    if (pin_group > 0xFF) {
        reg_addr = 0x03;
        pin_group = pin_group >> 8;
    }

    if (rp_I2C_SMBUS_Read( reg_addr, &value ) != RP_HW_OK){
      	pthread_mutex_unlock(&g_max_i2c_mutex);
        return -1;
    }

    if (state == 0) flag = 0xAA;
    if (state == 1) flag = 0x55;

    value = ((value & ~pin_group) | (pin_group & flag));

    if (rp_I2C_SMBUS_Write( reg_addr, value ) != RP_HW_OK){
      	pthread_mutex_unlock(&g_max_i2c_mutex);
        return -1;
    }
  	pthread_mutex_unlock(&g_max_i2c_mutex);
    return 0;
}

void max7311::setSleepTime(unsigned long time){
    g_sleep_time = time * 1000;
}


int  rp_max7311::rp_initController(){
    return max7311::initControllerDefault();
}


int  rp_max7311::rp_setAC_DC(char port,char mode){
    switch(port){
        case RP_MAX7311_IN2:
            if (max7311::setPIN_GROUP(PIN_K1,mode) == -1) return -1;
            usleep(g_sleep_time);
            if (max7311::setPIN_GROUP(PIN_K1,-1) == -1) return -1; 
            break;
        case RP_MAX7311_IN1:
            if (max7311::setPIN_GROUP(PIN_K2,mode) == -1) return -1;
            usleep(g_sleep_time);
            if (max7311::setPIN_GROUP(PIN_K2,-1) == -1) return -1; 
            break;
        default:
            return -1;
    }
    return 0;
}


int  rp_max7311::rp_setAttenuator(char port,char mode){
    switch(port){
        case RP_MAX7311_IN2:
            if (max7311::setPIN_GROUP(PIN_K3,mode) == -1) return -1;
            usleep(g_sleep_time);
            if (max7311::setPIN_GROUP(PIN_K3,-1) == -1) return -1; 
            break;
        case RP_MAX7311_IN1:
            if (max7311::setPIN_GROUP(PIN_K4,mode) == -1) return -1;
            usleep(g_sleep_time);
            if (max7311::setPIN_GROUP(PIN_K4,-1) == -1) return -1; 
            break;
        default:
            return -1;
    }
    return 0;
}


int  rp_max7311::rp_setGainOut(char port,char mode){
    switch(port){
        case RP_MAX7311_OUT2:
            if (max7311::setPIN_GROUP(PIN_K5,mode) == -1) return -1;
            usleep(g_sleep_time);
            if (max7311::setPIN_GROUP(PIN_K5,-1) == -1) return -1; 
            break;
        case RP_MAX7311_OUT1:
            if (max7311::setPIN_GROUP(PIN_K6,mode) == -1) return -1;
            usleep(g_sleep_time);
            if (max7311::setPIN_GROUP(PIN_K6,-1) == -1) return -1; 
            break;
        default:
            return -1;
    }
    return 0;
}

void rp_max7311::rp_setSleepTime(unsigned long time){
    max7311::setSleepTime(time);
}

char rp_max7311::rp_check(){
    if (max7311::getDefaultAddress() == MAX7311_DEFAULT_ADDRESS_1_2) return 0;
  	pthread_mutex_lock(&g_max_i2c_mutex);    
    if (rp_I2C_InitDevice(MAX7311_DEFAULT_DEV, max7311::getDefaultAddress()) != RP_HW_OK){
        return -1;
    }
    rp_I2C_setForceMode(true);
    uint8_t value = 0;
    if (rp_I2C_SMBUS_Read(0x08, &value) != RP_HW_OK){
        pthread_mutex_unlock(&g_max_i2c_mutex);
        return -1;
    }
    pthread_mutex_unlock(&g_max_i2c_mutex);   
    return value;
}