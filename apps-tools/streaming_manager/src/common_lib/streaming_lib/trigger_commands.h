#ifndef STREAMING_LIB_TRIGGER_COMMANDS_H
#define STREAMING_LIB_TRIGGER_COMMANDS_H

#include <memory>

#include "config_net_lib/server_net_config_manager.h"
#include "uio_lib/hardware.h"

namespace streaming_lib {

// The trigger state of the ADC, the DAC and the GPIO blocks the bitstream has
auto triggerStatusOf(uio_lib::CHardware& hw) -> TriggerStatus;

// Answers the software trigger and the trigger status requests of the config protocol from the
// hardware. The handlers hold weak references: they keep neither the hardware nor the manager
// (the web application module stays loaded after its exit, the bitstream may change).
auto connectTriggerCommands(std::shared_ptr<ServerNetConfigManager> config, uio_lib::CHardware::Ptr hw) -> void;

}  // namespace streaming_lib

#endif
