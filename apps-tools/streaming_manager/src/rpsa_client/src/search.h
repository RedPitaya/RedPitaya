#ifndef SEARCH_H
#define SEARCH_H

#include "options.h"

auto startSearch(ClientOpt::Options& option) -> bool;  // false: no board answered
auto startSearch() -> std::string;

#endif
