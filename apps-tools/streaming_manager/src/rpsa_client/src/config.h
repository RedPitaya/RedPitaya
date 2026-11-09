#ifndef CONFIG_H
#define CONFIG_H

#include "config_net_lib/client_net_config_manager.h"
#include "options.h"

#include <atomic>
#include <functional>

auto sleepMs(int ms) -> void;
// How long an answer of a board is waited for (--reply-timeout), milliseconds
auto setReplyTimeout(int ms) -> void;
auto replyTimeout() -> int;
// Waits until counter is down to 0 (every board answered). False on a stop (stopped() true) or when
// the answers do not come within the reply timeout: then it is told what was asked.
auto waitAnswers(std::atomic<int>& counter, const std::function<bool()>& stopped, const char* what) -> bool;
auto connectConfigServer(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool;
auto startConfig(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool;
auto configSIGHandler() -> void;

#endif
