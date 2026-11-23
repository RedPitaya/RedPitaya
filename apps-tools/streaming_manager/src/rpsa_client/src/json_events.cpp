#include "json_events.h"

#include <chrono>
#include <csignal>
#include <cstdio>
#include <ctime>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
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
int g_errors = 0;

#ifdef _WIN32
// the strings of the client (file names, paths) are in the ANSI code page on Windows; JSON is UTF-8
auto utf8(const std::string& a) -> std::string {
    if (a.empty())
        return a;
    const int wn = MultiByteToWideChar(CP_ACP, 0, a.data(), static_cast<int>(a.size()), nullptr, 0);
    if (wn <= 0)
        return a;
    std::wstring w(wn, L'\0');
    MultiByteToWideChar(CP_ACP, 0, a.data(), static_cast<int>(a.size()), w.data(), wn);
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), wn, nullptr, 0, nullptr, nullptr);
    if (n <= 0)
        return a;
    std::string u(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), wn, u.data(), n, nullptr, nullptr);
    return u;
}
auto toUtf8(Json::Value& v) -> void {
    if (v.isString())
        v = utf8(v.asString());
    else if (v.isArray())
        for (auto& x : v)
            toUtf8(x);
    else if (v.isObject())
        for (const auto& k : v.getMemberNames())
            toUtf8(v[k]);
}
#else
auto toUtf8(Json::Value&) -> void {}
#endif

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
#ifndef _WIN32
    // a reader that went away (| head, | jq ended) must not end the recording: the events are lost
    signal(SIGPIPE, SIG_IGN);
#endif
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
    toUtf8(o);
    if (event == "error")
        g_errors++;
    Json::StreamWriterBuilder b;
    b["indentation"] = "";
    b["emitUTF8"] = true;
    const std::string line = Json::writeString(b, o);
    fprintf(g_out, "%s\n", line.c_str());
    fflush(g_out);
}

auto jsonev::errors() -> int {
    const std::lock_guard lock(g_mtx);
    return g_errors;
}

auto jsonev::exitCode(int code) -> int {
    if (code != 0 && errors() == 0) {
        Json::Value e;
        e["message"] = "the run failed: the reason is on stderr";
        emit("error", e);
    }
    Json::Value f;
    f["code"] = code;
    emit("exit", f);
    return code;
}
