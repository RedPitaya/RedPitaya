#ifndef RPSA_CLIENT_HELP_H
#define RPSA_CLIENT_HELP_H

#include <cstdio>
#include <string>

namespace ClientHelp {

// The exit codes of the client
enum Exit { EXIT_OK = 0, EXIT_USAGE = 1, EXIT_NO_BOARD = 2, EXIT_FAILED = 3 };

// The sections of the help: "" (the overview), detect, config, remote, stream, dac, keys, formats,
// examples. The topic of a mode key (-s, --streaming, ...) is its section. False: no such section.
auto print(FILE* out, const std::string& prog, const std::string& topic, const std::string& filter = "") -> bool;
// The usage lines of a section and where its help is: after an error of the command line
auto synopsis(FILE* out, const std::string& prog, const std::string& topic) -> void;
// The section of a mode key, "" for none
auto topicOfKey(const std::string& key) -> std::string;
auto version() -> std::string;
// The program name without its directory
auto programName(const char* argv0) -> std::string;

}  // namespace ClientHelp

#endif
