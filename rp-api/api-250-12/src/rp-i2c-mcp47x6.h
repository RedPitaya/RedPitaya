#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

// I2C Address of device
// MCP4706, MCP4716 & MCP4726 are factory programed for any of 0x60 thru 0x67
//  commonly 0x60
#define MCP47X6_DEFAULT_ADDRESS	0x60

// Programmable Gain definitions
#define MCP47X6_GAIN_MASK	0xFE

#define MCP47X6_GAIN_1X  	0x00
#define MCP47X6_GAIN_2X 	0x01

// Power Down Mode definitions
#define MCP47X6_PWRDN_MASK     0xF9

#define MCP47X6_AWAKE          0x00
#define MCP47X6_PWRDN_1K       0x02
#define MCP47X6_PWRDN_100K     0x04
#define MCP47X6_PWRDN_500K     0x06

// Reference Voltage definitions
#define MCP47X6_VREF_MASK             0xE7

#define MCP47X6_VREF_VDD              0x00
#define MCP47X6_VREF_VREFPIN	      0x10
#define MCP47X6_VREF_VREFPIN_BUFFERED 0x18

// Command definitioins
#define MCP47X6_CMD_MASK       0x1F

#define MCP47X6_CMD_VOLDAC     0x00
#define MCP47X6_CMD_VOLALL     0x40
#define MCP47X6_CMD_VOLCONFIG  0x80
#define MCP47X6_CMD_ALL        0x60

namespace RP_MCP47X6{
    enum mcp47x6_model{
        MCP4706,
        MCP4716,
        MCP4726
    };

    class mcp47x6 {
        typedef unsigned char  uint8_t; 
        typedef unsigned short uint16_t; 
        public:
            mcp47x6(mcp47x6_model model, const char* i2c_dev_path);
            mcp47x6(mcp47x6_model model, const char* i2c_dev_path, uint8_t address);
            

            bool    readConfig();
            bool    writeConfig();
            bool    writeConfigAll();
            void    setGain(uint8_t gain);
            char    getGain();
            char    getGainEeprom();
               
            void    setPowerDown(uint8_t pdOutR);
            char    getPowerDown();
            char    getPowerDownEeprom();

            void    setVReference(uint8_t ref);
            char    getVReferenc();
            char    getVReferencEeprom();

            void    setOutputLevel(unsigned short level);
            short   getOutputLevel();
            short   getOutputLevelEeprom();
            short   getMaxLevel();

        private:
            mcp47x6_model m_model;
            char          m_devAddr;
            const char*   m_i2c_dev_path;

            char          m_config;
            char          m_config_eeprom;
            short         m_level;
            short         m_level_eeprom;
    };
}

#ifdef  __cplusplus
}
#endif