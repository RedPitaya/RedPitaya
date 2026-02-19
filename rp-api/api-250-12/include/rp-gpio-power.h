#pragma once

namespace rp_gpio_power {

// line   2: "MIO2 (ENAPWRIN)" unused input active-high
#define ADC_POWER (908 - 906)
// line   3: "MIO3 (ENAPWROUT)" unused input active-high
#define DAC_POWER (909 - 906)

#define POWER_ON 1
#define POWER_OFF 0

int rp_set_power_mode(int module, int state);
int rp_get_power_mode(int module);

}  // namespace rp_gpio_power
