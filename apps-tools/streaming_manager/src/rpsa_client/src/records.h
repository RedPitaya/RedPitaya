#ifndef RPSA_CLIENT_RECORDS_H
#define RPSA_CLIENT_RECORDS_H

#include <functional>
#include <list>
#include <memory>
#include <string>

#include "config_net_lib/client_net_config_manager.h"
#include "options.h"

// The recordings of the boards on their SD cards (CS_REQUEST_RECORD_STATUS):
//  watch - a panel (a line per board every 5 s without a terminal) until no board writes any more,
//          or until stop() says so; the recordings go on;
//  once  - key=value lines for scripts, once.
// False when a board did not answer (an older streaming server) or none writes (watch).
// reportMissing: a board without an answer is an error (not for start_stop: the panel is an extra).
auto watchRecords(std::shared_ptr<ClientNetConfigManager> cl, const std::list<std::string>& hosts, const ClientOpt::Options& option, bool once,
                  const std::function<bool()>& stop, bool reportMissing = true) -> bool;

// "45 s", "12 min", "2 h 05 min"; "-" for none
auto durationText(double seconds) -> std::string;

#endif
