#ifndef GPIO_STREAMING_LIB_GPIO_STOP_REASON_H
#define GPIO_STREAMING_LIB_GPIO_STOP_REASON_H

namespace gpio_streaming_lib {

// DISCONNECTED: the network client of the data has gone
enum class GPIOStopReason { NORMAL, MEM_ERROR, MEM_MODIFY, CONFIG_ERROR, DISCONNECTED };

}  // namespace gpio_streaming_lib

#endif
