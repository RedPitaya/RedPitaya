#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "rp-i2c.h"
#include "rp-i2c-mcp47x6.h"
#include "rp-i2c-max7311.h"
#include "rp-spi.h"
#include "rp-gpio-power.h"

#define MSG(X,Y) printf("%s status %d\n",X,Y);

int main()
{
    // RP_MCP47X6::mcp47x6 chip(RP_MCP47X6::mcp47x6_model::MCP4716,"/dev/i2c-0");
    // printf("MAX Level %x\n",chip.getMaxLevel());
    // chip.readConfig();
    // printf("GAIN %x\n",chip.getGain());
    // printf("GAIN eeprom %x\n",chip.getGainEeprom());
    // printf("POWER %x\n",chip.getPowerDown());
    // printf("POWER eeprom %x\n",chip.getPowerDownEeprom());
    // printf("Vref %x\n",chip.getVReferenc());
    // printf("Vref eeprom %x\n",chip.getVReferencEeprom());
    // printf("Level %x\n",chip.getOutputLevel());
    // printf("Level eeprom %x\n",chip.getOutputLevelEeprom());

    // chip.setGain(MCP47X6_GAIN_2X);
    // chip.setPowerDown(MCP47X6_AWAKE);
    // chip.setVReference(MCP47X6_VREF_VDD);
    // chip.setOutputLevel(0);
    // chip.writeConfig();

     MSG("Init",rp_max7311::rp_initController());
      MSG("Check",rp_max7311::rp_check());
//     MSG("set AC/DC 1",rp_max7311::rp_setAC_DC(RP_MAX7311_IN1,RP_DC_MODE));
//     MSG("get AC/DC 1",rp_max7311::rp_getAC_DC(RP_MAX7311_IN1));
//max7311::setPIN_GROUP(PIN_K1,RP_AC_MODE);
//     MSG("set AC/DC 2",rp_max7311::rp_setAC_DC(RP_MAX7311_IN2,RP_AC_MODE));
  //   MSG("get AC/DC 2",rp_max7311::rp_getAC_DC(RP_MAX7311_IN2));

  //   MSG("set Attenuator 1",rp_max7311::rp_setAttenuator(RP_MAX7311_IN1,RP_ATTENUATOR_1_20));
 //    MSG("get Attenuator 1",rp_max7311::rp_getAttenuator(RP_MAX7311_IN1));

   ///  MSG("set Attenuator 2",rp_max7311::rp_setAttenuator(RP_MAX7311_IN2,RP_ATTENUATOR_1_20));
 //    MSG("get Attenuator 2",rp_max7311::rp_getAttenuator(RP_MAX7311_IN2));

    // MSG("set Gain 1",rp_max7311::rp_setGainOut(RP_MAX7311_OUT1,RP_GAIN_10V));
    // MSG("get Gain 1",rp_max7311::rp_getGainOut(RP_MAX7311_OUT1));

    // MSG("set Gain 2",rp_max7311::rp_setGainOut(RP_MAX7311_OUT2,RP_GAIN_10V));
    // MSG("get Gain 2",rp_max7311::rp_getGainOut(RP_MAX7311_OUT2));

    // MSG("Init",rp_max7311::rp_initController());

    // rp_i2c_enable_verbous();
    // rp_i2c_load("../configs/SI571.xml");
    // rp_i2c_print("../configs/SI571.xml");
    // std::cout << "Compare status = " << rp_i2c_compare("../configs/SI571.xml") << "\n";


   // rp_gpio_power::rp_set_power_mode(ADC_POWER,POWER_ON);
//    sleep(2);
   // rp_gpio_power::rp_set_power_mode(ADC_POWER,POWER_OFF);
//    rp_spi_fpga::rp_spi_enable_verbous();
//    rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml");
 //   rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250_default.xml");
    //rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250_streaming.xml");
    // char x = 0;
    // rp_spi_fpga::rp_read_from_spi_fpga("/dev/mem/",0x40000000,0x50,0x14,x);
    // printf ("0x14 = %d\n",x);
    // rp_spi_fpga::rp_read_from_spi_fpga("/dev/mem/",0x40000000,0x50,0xFF,x);
    // printf ("0xFF = %d\n",x);
    // rp_spi_fpga::rp_read_from_spi_fpga("/dev/mem/",0x40000000,0x50,0x16,x);
    // printf ("0x16 = %d\n",x);
    // rp_spi_fpga::rp_read_from_spi_fpga("/dev/mem/",0x40000000,0x50,0x18,x);
    // printf ("0x18 = %d\n",x);
//    rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9746BCPZ-250.xml");
    return 0;
}
