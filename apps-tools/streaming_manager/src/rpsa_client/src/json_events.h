#ifndef RPSA_CLIENT_JSON_EVENTS_H
#define RPSA_CLIENT_JSON_EVENTS_H

#include <json/json.h>
#include <string>

// --json (streaming_improvements 4.13): the events of a run as JSON Lines on stdout, an object a
// line, written at once. Every object has "event" and "time" (UTC, ISO 8601 with ms), most a
// "host". Everything else the client prints (the messages, the Done lines) goes to stderr then.
//   start     a board starts: the file, the format, the limits
//   progress  every second a board: samples, bytes, rate, losses, the part and its file
//   part      a new part of a recording in parts (--split): its number, its file
//   error     a board ended with an error: the message
//   done      a board ended: ok, samples, bytes, seconds, losses, the files
//   board     -d: a board found
//   exit      the last line: the exit code of the client
namespace jsonev {

// from now on: the JSON keeps the stdout of the process, what is printed to stdout goes to stderr
auto enable() -> bool;
auto on() -> bool;
// one line: {"event": event, "time": ..., fields...}
auto emit(const std::string& event, Json::Value fields = Json::Value(Json::objectValue)) -> void;
// the exit code as the last line; returns it. A failed run without an error event gets one
// first (its reason is on stderr)
auto exitCode(int code) -> int;
// error events written so far
auto errors() -> int;

}  // namespace jsonev

#endif
