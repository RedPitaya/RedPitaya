#ifndef RPSA_CLIENT_GPIO_CLIENT_H
#define RPSA_CLIENT_GPIO_CLIENT_H

#include "config_net_lib/client_net_config_manager.h"
#include "options.h"

// The GPIO streaming of one board through this client: the input into a file here (BIN, CSV or
// VCD), a BIN file here on the output, or both at once. False: the board refused, a file could
// not be read or written, the board or another client ended it.
auto startGPIO(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool;
auto gpioSIGHandler() -> void;

#endif
