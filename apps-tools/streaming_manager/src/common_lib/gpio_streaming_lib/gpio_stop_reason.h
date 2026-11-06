#ifndef GPIO_STREAMING_LIB_GPIO_STOP_REASON_H
#define GPIO_STREAMING_LIB_GPIO_STOP_REASON_H

namespace gpio_streaming_lib {

// DISCONNECTED: the network client of the data has gone. The file mode: DONE - the input wrote its
// sample limit, the output played its file to the end; SD_FULL - no room for the input file;
// BROKEN - the file cannot be written (input) or read (output); MISSING_FILE, EMPTY - the file of
// the output is not there or holds no words.
enum class GPIOStopReason { NORMAL, MEM_ERROR, MEM_MODIFY, CONFIG_ERROR, DISCONNECTED, DONE, SD_FULL, BROKEN, MISSING_FILE, EMPTY };

}  // namespace gpio_streaming_lib

#endif
