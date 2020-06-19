#pragma once


#ifdef  __cplusplus
extern "C" {
#endif


namespace rp_gpio_power{

#define ADC_POWER 908
#define DAC_POWER 909

#define POWER_ON  1
#define POWER_OFF 0

int rp_set_power_mode(int module,int state);
int rp_get_power_mode(int module);

}

#ifdef  __cplusplus
}
#endif
