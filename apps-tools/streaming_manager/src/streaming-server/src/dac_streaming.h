#ifndef DAC_STREAMING_H
#define DAC_STREAMING_H

#include "config_net_lib/server_net_config_manager.h"
#include "dac_streaming_lib/dac_stream_server.h"
#include "settings_lib/channels.hpp"
#include "uio_lib/hardware.h"

auto startDACServer(bool verbMode, dac_channels_t activeChannels) -> void;
auto stopDACNonBlocking(dac_streaming_lib::DACStopReason reason) -> void;
auto stopDACServer(dac_streaming_lib::DACStopReason reason) -> void;
auto setDACServer(ServerNetConfigManager::Ptr serverNetConfig) -> void;
auto setDACHardware(uio_lib::CHardware::Ptr hw) -> void;

#endif
