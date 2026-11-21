#include "json_events.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#define DUP _dup
#define DUP2 _dup2
#define FDOPEN _fdopen
#define FILENO _fileno
#else
#include <unistd.h>
#define DUP dup
#define DUP2 dup2
#define FDOPEN fdopen
#define FILENO fileno
#endif

namespace {
std::mutex g_mtx;
FILE* g_out = nullptr;  // the stdout of the process

auto timeText() -> std::string {
    const auto now = std::chrono::system_clock::now();
    const time_t secs = std::chrono::system_clock::to_time_t(now);
    const int ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000);
    struct tm t;
#ifdef _WIN32
    gmtime_s(&t, &secs);
#else
    gmtime_r(&secs, &t);
#endif
    char buf[40];
    const size_t n = strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &t);
    snprintf(buf + n, sizeof(buf) - n, ".%03dZ", ms);
    return buf;
}
}  // namespace

auto jsonev::enable() -> bool {
    const std::lock_guard lock(g_mtx);
    if (g_out)
        return true;
    fflush(stdout);
    const int fd = DUP(FILENO(stdout));
    if (fd < 0)
        return false;
#ifdef _WIN32
    _setmode(fd, _O_BINARY);  // the lines end with \n, as JSON Lines want
#endif
    g_out = FDOPEN(fd, "wb");
    if (!g_out)
        return false;
    // the rest of the client prints to stdout: to stderr from now on
    DUP2(FILENO(stderr), FILENO(stdout));
    return true;
}

auto jsonev::on() -> bool {
    const std::lock_guard lock(g_mtx);
    return g_out != nullptr;
}

auto jsonev::emit(const std::string& event, Json::Value fields) -> void {
    const std::lock_guard lock(g_mtx);
    if (!g_out)
        return;
    Json::Value o(Json::objectValue);
    o["event"] = event;
    o["time"] = timeText();
    for (const auto& k : fields.getMemberNames())
        o[k] = fields[k];
    Json::StreamWriterBuilder b;
    b["indentation"] = "";
    b["emitUTF8"] = true;
    const std::string line = Json::writeString(b, o);
    fprintf(g_out, "%s\n", line.c_str());
    fflush(g_out);
}

auto jsonev::exitCode(int code) -> int {
    Json::Value f;
    f["code"] = code;
    emit("exit", f);
    return code;
}
