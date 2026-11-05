#ifndef GPIO_STREAMING_LIB_GPIO_STOP_REASON_H
#define GPIO_STREAMING_LIB_GPIO_STOP_REASON_H

namespace gpio_streaming_lib {

enum class GPIOStopReason { NORMAL, MEM_ERROR, MEM_MODIFY, CONFIG_ERROR };

}  // namespace gpio_streaming_lib

#endif
