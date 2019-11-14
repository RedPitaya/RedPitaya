#include "rp-i2c-mcp47x6.h"
#include "i2c/i2c.h"
#include <iostream>

using namespace RP_MCP47X6;

/******************************************
 * Default constructor, uses default I2C address.
 * @see mcp47x6_DEFAULT_ADDRESS
 */
mcp47x6::mcp47x6(mcp47x6_model model, const char* i2c_dev_path) {
    m_devAddr = MCP47X6_DEFAULT_ADDRESS;
    m_i2c_dev_path = i2c_dev_path;
    m_config = 0;
    m_config_eeprom = 0;
    m_level = 0;
    m_level_eeprom = 0;
    m_model = model;
}

/******************************************
 * Specific address constructor.
 * @param address I2C address
 * @see mcp47x6_DEFAULT_ADDRESS
 */
mcp47x6::mcp47x6(mcp47x6_model model, const char* i2c_dev_path, uint8_t address) {
    m_devAddr = address;
    m_i2c_dev_path = i2c_dev_path;
    m_config = 0;
    m_config_eeprom = 0;
    m_level = 0;
    m_level_eeprom = 0;
    m_model = model;
}

bool mcp47x6::readConfig() {
    unsigned char buff[8];
    int size = read_to_i2c_buffer(m_i2c_dev_path,m_devAddr,0,buff,true);
    //printf("size %d 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x \n",size,buff[0],buff[1],buff[2],buff[3],buff[4],buff[5]);    
    if (m_model == MCP4706){
        m_config = buff[0] & 0x1F;
        m_level  = buff[1];
        m_config_eeprom = buff[2] & 0x1F; 
        m_level_eeprom  = buff[3];
    }
    else{
        m_config = buff[0] & 0x1F;
        m_config_eeprom = buff[3] & 0x1F; 

        if (m_model == MCP4716){  
            m_level = (buff[1] << 8 | buff[2]) >> 6;
            m_level_eeprom = (buff[4] << 8 | buff[5]) >> 6;
        }

        if (m_model == MCP4726){
            m_level = (buff[1] << 8 | buff[2]) >> 4;
            m_level_eeprom = (buff[4] << 8 | buff[5]) >> 4;
        }
    }
    return (size >  0);
}

bool mcp47x6::writeConfig() {
    // WRITE all memory don't work 
    // char  command = (m_config | MCP47X6_CMD_VOLALL); 
    // unsigned short data = 0;
    // if (m_model == MCP4706){
    //     data = m_level << 8;
    // }

    // if (m_model == MCP4716){
    //     data = m_level << 6; 
    // }

    // if (m_model == MCP4726){
    //     data = m_level << 4;  
    // }

    // printf("level value 0x%.2x level 0x%.2x \n",command, data);
    // return (write_to_i2c_word(m_i2c_dev_path , m_devAddr , command, data) == 0);
    
    bool state = true;

    char command = (m_config & ~MCP47X6_PWRDN_MASK) << 3; 
    char data = 0;
    if (m_model == MCP4706){
        data = 0xFF & m_level;
    }

    if (m_model == MCP4716){
        command = command | ((m_level >> 6) & 0x0F);
        data    = 0xFF & (m_level << 2);
    }

    if (m_model == MCP4726){
        command = command | ((m_level >> 8) & 0x0F);
        data    = 0xFF & m_level;
    }

    state = (write_to_i2c(m_i2c_dev_path , m_devAddr , command, data,false) == 0) & state;
    char value = (m_config & MCP47X6_CMD_MASK) | MCP47X6_CMD_VOLCONFIG;
    state = (write_to_i2c_command(m_i2c_dev_path , m_devAddr , value,false) == 0) & state;
    return state;
}

bool mcp47x6::writeConfigAll() {
    char  command = (m_config | MCP47X6_CMD_ALL); 
    unsigned short data = 0;
    if (m_model == MCP4706){
        data = m_level << 8;
    }

    if (m_model == MCP4716){
        data = m_level << 6; 
    }

    if (m_model == MCP4726){
        data = m_level << 4;  
    }

    //printf("level value 0x%.2x level 0x%.2x \n",command, data);
    return (write_to_i2c_word(m_i2c_dev_path , m_devAddr , command, data,false) == 0);
}

/******************************************
 * Set the configuration bits for the DAC
 */
void mcp47x6::setGain(uint8_t gain) {
    m_config = (m_config & MCP47X6_GAIN_MASK) | (gain & ~MCP47X6_GAIN_MASK);
}

char mcp47x6::getGain(){
    return m_config & ~MCP47X6_GAIN_MASK; 
}

char mcp47x6::getGainEeprom(){
    return m_config_eeprom & ~MCP47X6_GAIN_MASK; 
}


void mcp47x6::setVReference(uint8_t vref) {
    m_config = (m_config & MCP47X6_VREF_MASK) | (vref & ~MCP47X6_VREF_MASK);
}

void mcp47x6::setOutputLevel(unsigned short level) {
    m_level = level;
    // char command = (m_config & ~MCP47X6_PWRDN_MASK) << 3; 
    // char data = 0;
    // if (m_model == MCP4706){
    //     data = 0xFF & level;
    // }

    // if (m_model == MCP4716){
    //     command = command | ((level >> 6) & 0x0F);
    //     data    = 0xFF & (level << 2);
    // }

    // if (m_model == MCP4726){
    //     command = command | ((level >> 8) & 0x0F);
    //     data    = 0xFF & level;
    // }

    // printf("level value 0x%.2x level 0x%.2x\n",command, data);
    // return (write_to_i2c(m_i2c_dev_path , m_devAddr , command, data) == 0);
}

short   mcp47x6::getOutputLevel(){
    return m_level;
}

short   mcp47x6::getOutputLevelEeprom(){
    return m_level_eeprom;
}

short  mcp47x6::getMaxLevel(){
    switch(m_model){
        case MCP4706: return 0x00FF;
        case MCP4716: return 0x03FF;
        case MCP4726: return 0x0FFF;
    }
    return 0;
}

void mcp47x6::setPowerDown(uint8_t pdOutR){
    m_config = (m_config & MCP47X6_PWRDN_MASK) | (pdOutR & ~MCP47X6_PWRDN_MASK);
}

char mcp47x6::getPowerDown(){
    return m_config & ~MCP47X6_PWRDN_MASK; 
}

char mcp47x6::getPowerDownEeprom(){
    return m_config_eeprom & ~MCP47X6_PWRDN_MASK; 
}

char mcp47x6::getVReferenc(){
    return m_config & ~MCP47X6_VREF_MASK; 
}

char mcp47x6::getVReferencEeprom(){
    return m_config_eeprom & ~MCP47X6_VREF_MASK;
}





