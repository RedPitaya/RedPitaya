#include <atomic>
#include <filesystem>
#include "streaming.h"
#include "records.h"
#include "config.h"
#include "converter_lib/converter.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "remote.h"
#include "settings_lib/stream_settings.h"
#include "json_events.h"
#include "streaming_lib/streaming_file.h"
#include "test_helper.h"
#include "panel.h"

#include <array>
#include <cmath>
#include <set>
#include "writer_lib/file_helper.h"

std::map<std::string, converter_lib::CConverter::Ptr> g_converter;

std::mutex g_smutex;
std::mutex g_s_csv_mutex;
ClientOpt::Options g_soption;
std::string g_filenameDate;
std::atomic<bool> g_streamFailed(false);  // a board refused or a stream ended with an error
std::atomic<bool> sig_exit_flag(false);
std::atomic<int> g_runClientCounter;
std::atomic<int> g_clientsRunning(0);
std::map<std::string, std::string> g_files;  // the file of each board
std::map<std::string, bool> g_endedByBoard;  // the board or the connection ended it, not this client
// --json (streaming_improvements 4.13), under g_smutex: the file object of each board while it
// streams, the files of its recording at its end, the parts told so far
std::map<std::string, std::weak_ptr<streaming_lib::CStreamingFile>> g_fileObj;
std::map<std::string, std::vector<std::string>> g_fileList;
std::map<std::string, size_t> g_partsTold;
std::map<std::string, std::string> g_stopReason;  // why each board ended (the done event)

// an error of a board as an event too (--json)
static auto jsonError(const std::string& host, const std::string& text) -> void {
    if (!jsonev::on())
        return;
    Json::Value e;
    if (!host.empty())
        e["host"] = host;
    e["message"] = text;
    jsonev::emit("error", e);
}
// the levels of the channels since the panel took them: min and max in % of the full scale
struct Levels {
    std::array<int, 4> min{{101, 101, 101, 101}};
    std::array<int, 4> max{{-101, -101, -101, -101}};
    std::array<bool, 4> seen{};
};
std::map<std::string, Levels> g_levels;
std::map<std::string, double> g_queue;  // the fill of the file queue of each board
std::map<std::string, double> g_diskRate;  // the measured write speed of its file, bytes per second
std::mutex g_levelsMutex;

// The ADC trigger of each board (streaming_improvements 3.5): from its settings before the start,
// its state polled every second while it streams
struct TrigInfo {
    bool on = false;
    std::string text;  // the sources and the level
    bool known = false;
    bool armed = false;
    uint32_t count = 0;
};
std::map<std::string, TrigInfo> g_trig;
std::mutex g_trigMutex;
// about how long a buffer of each board lasts (the time limit on the clock), under g_trigMutex
std::map<std::string, double> g_bufferSeconds;

// "IN1 level 0.300 V rising, EXT": the sources of the mask as the stream_app takes them;
// adcChannels 0: unknown (a server without BOARD_INFO)
static auto triggerText(const CStreamSettings& s, int adcChannels) -> std::string {
    const uint32_t m = s.getADCTriggerSources();
    std::string t;
    auto add = [&](const std::string& x) { t += (t.empty() ? "" : ", ") + x; };
    std::string ins;
    if (m & 4)
        ins += "IN1 ";
    if (m & 8)
        ins += "IN2 ";
    if (adcChannels == 4 && (m & 1))
        ins += "IN3 ";
    if (adcChannels == 4 && (m & 2))
        ins += "IN4 ";
    if (!ins.empty()) {
        char buf[96];
        const bool rising = s.getADCTriggerEdge().value == CStreamSettings::TriggerEdge::RISING;
        if (s.getADCTriggerLevelUnit().value == CStreamSettings::LevelUnit::VOLT)
            snprintf(buf, sizeof(buf), "level %.3f V %s", s.getADCTriggerLevelVolt(), rising ? "rising" : "falling");
        else
            snprintf(buf, sizeof(buf), "level codes %d..%d %s", s.getADCTriggerLevelLow(), s.getADCTriggerLevelHigh(), rising ? "rising" : "falling");
        add(ins + buf);
    }
    if (adcChannels != 4 && (m & 1))
        add(adcChannels ? "DAC A" : "DAC A (IN3 on a 4-input board)");
    if (adcChannels != 4 && (m & 2))
        add(adcChannels ? "DAC B" : "DAC B (IN4 on a 4-input board)");
    if (m & 16)
        add("GPIO pattern");
    if (m & 32)
        add("EXT");
    return t.empty() ? "no source" : t;
}

// The trigger settings of the boards; the number of inputs from BOARD_INFO (an older server does
// not answer it: 2 s at most, the sources are then named for both kinds of board)
static auto readTriggers(std::shared_ptr<ClientNetConfigManager> cl, const std::list<std::string>& hosts) -> void {
    std::atomic<int> settings(static_cast<int>(hosts.size()));
    std::atomic<int> infos(static_cast<int>(hosts.size()));
    std::map<std::string, int> channels;
    std::map<std::string, double> rates;
    std::mutex m;
    std::set<std::string> answered;  // the boards whose settings came (else the local ones are defaults)
    cl->getNewSettingsNofiy.connect([&](std::string& h) {
        {
            const std::lock_guard lock(m);
            answered.insert(h);
        }
        settings--;
    });
    cl->getBoardInfoNofiy.connect([&](std::string& h, BoardInfoMessage i) {
        const std::lock_guard lock(m);
        channels[h] = i.adcChannels;
        rates[h] = i.adcRate;
        infos--;
    });
    cl->failBoardInfoNofiy.connect([&](std::string&) { infos--; });
    for (auto& h : hosts) {
        if (!cl->requestConfig(h))
            settings--;
        if (!cl->requestBoardInfo(h))
            infos--;
    }
    for (int waited = 0; (settings > 0 || infos > 0) && waited < 2000 && !sig_exit_flag; waited += 20)
        sleepMs(20);
    cl->removeHadlers();
    const std::lock_guard lock(g_trigMutex);
    g_trig.clear();
    g_bufferSeconds.clear();
    for (auto& h : hosts) {
        auto* s = cl->getLocalSettingsOfHost(h);
        {
            const std::lock_guard l(m);
            // its own settings only (none: learnt from its buffers); an older server without
            // BOARD_INFO: 125 MHz
            if (s && answered.count(h))
                g_bufferSeconds[h] = s->adcBufferSeconds(rates.count(h) ? rates[h] : 125e6);
        }
        if (!s || s->getADCTriggerMode().value != CStreamSettings::State::ON)
            continue;
        int n = 0;
        {
            const std::lock_guard l(m);
            n = channels.count(h) ? channels[h] : 0;
        }
        TrigInfo t;
        t.on = true;
        t.text = triggerText(*s, n);
        g_trig[h] = t;
        if (s->getADCTriggerSources() == 0)
            aprintf(stderr,
                    "%s [WARN] %s waits for a trigger with no source chosen (adc_trigger_sources=0):\n"
                    "       only a software trigger starts it (the web page or the desktop client)\n",
                    getTS(": ").c_str(), h.c_str());
    }
}

// min and max of a buffer of raw ADC codes, in % of the full scale
static auto levelOf(DataLib::CDataBufferDMA::Ptr b, int& lo, int& hi) -> bool {
    if (!b || b->getSamplesCount() == 0)
        return false;
    const size_t n = b->getSamplesCount();
    int mn = INT32_MAX, mx = INT32_MIN;
    if (b->getBitBySample() == 8) {
        auto d = static_cast<const int8_t*>(b->getMappedDataMemory());
        for (size_t i = 0; i < n; i++) {
            mn = std::min<int>(mn, d[i]);
            mx = std::max<int>(mx, d[i]);
        }
        lo = mn * 100 / 128;
        hi = mx * 100 / 127;
    } else {
        auto d = static_cast<const int16_t*>(b->getMappedDataMemory());
        for (size_t i = 0; i < n; i++) {
            mn = std::min<int>(mn, d[i]);
            mx = std::max<int>(mx, d[i]);
        }
        lo = mn * 100 / 32768;
        hi = mx * 100 / 32767;
    }
    return true;
}

std::map<std::string, bool> g_terminate;

std::vector<std::thread> clients;

auto stopCSV() -> void;
auto stopStreaming() -> void;
auto stopStreaming(std::string host) -> void;

auto runClient(std::string host, StateRunnedHosts, uint32_t size, adc_channels_t activeChannels) -> void {
    auto memoryManager = new uio_lib::CMemoryManager();
    auto buffers = DataLib::CBuffersCached::create();
    memoryManager->setMemoryBlockSize(size);
    memoryManager->reallocateBlocks();
    auto blocks = memoryManager->getFreeBlockCount();
    auto reserved __attribute__((unused)) = memoryManager->reserveMemory(uio_lib::MM_ADC, blocks, activeChannels.count());
    buffers->generateBuffersEmptyADC(activeChannels, memoryManager->getRegions(uio_lib::MM_ADC), DataLib::sizeHeader());
    TRACE_SHORT("Reserved blocks %d", reserved)

    g_terminate[host] = false;

    if (g_soption.save_dir == "")
        g_soption.save_dir = ".";

    CStreamSettings::DataFormat file_type = CStreamSettings::DataFormat::BIN;
    switch (g_soption.streamign_type) {
        case ClientOpt::StreamingType::TDMS:
            file_type = CStreamSettings::DataFormat::TDMS;
            break;
        case ClientOpt::StreamingType::WAV:
            file_type = CStreamSettings::DataFormat::WAV;
            break;
        case ClientOpt::StreamingType::CSV:
            file_type = CStreamSettings::DataFormat::BIN;
            break;
        case ClientOpt::StreamingType::BIN:
            file_type = CStreamSettings::DataFormat::BIN;
            break;
        default:
            stopStreaming(host);
            return;
    }

    bool convert_v = false;
    switch (g_soption.save_type) {
        case ClientOpt::SaveType::RAW:
            convert_v = false;
            break;
        case ClientOpt::SaveType::VOL:
            convert_v = true;
            break;
        default:
            convert_v = false;
            break;
    }

    auto g_file_manager = streaming_lib::CStreamingFile::create(file_type, g_soption.save_dir, g_soption.samples, convert_v, false);
    g_file_manager->setTDMSTimeColumn(g_soption.tdms_time);
    double bufferSeconds = 0;
    {
        const std::lock_guard lock(g_trigMutex);
        if (g_bufferSeconds.count(host))
            bufferSeconds = g_bufferSeconds[host];
    }
    g_file_manager->setLimits(g_soption.limit_time, g_soption.limit_size, bufferSeconds);
    g_file_manager->setSplit(g_soption.split_size, g_soption.split_time);
    // why the file ended by itself: a limit, the clock of the time limit, a full disk
    auto fileStop = std::make_shared<std::atomic<int>>(-1);
    g_file_manager->stopNotify.connect([fileStop](streaming_lib::CStreamingFile::EStopReason r) { *fileStop = static_cast<int>(r); });
    g_file_manager->run(host + "_" + g_filenameDate);
    {
        const std::lock_guard lock(g_smutex);
        g_files[host] = g_file_manager->getCSVFileName();
        g_fileObj[host] = g_file_manager;
        g_partsTold[host] = 1;
    }
    if (jsonev::on()) {
        static const char* formats[] = {"", "TDMS", "WAV", "CSV", "BIN"};
        Json::Value e;
        e["host"] = host;
        e["file"] = g_file_manager->getCSVFileName();
        e["format"] = formats[(int)g_soption.streamign_type];
        e["volts"] = convert_v;
        Json::Value lim;
        lim["samples"] = Json::UInt64(g_soption.samples);
        lim["time"] = g_soption.limit_time;
        lim["size"] = Json::UInt64(g_soption.limit_size);
        lim["split_size"] = Json::UInt64(g_soption.split_size);
        lim["split_time"] = g_soption.split_time;
        e["limits"] = lim;
        jsonev::emit("start", e);
    }

    auto g_s_file_w = std::weak_ptr<streaming_lib::CStreamingFile>(g_file_manager);
    auto g_s_buffers_w = std::weak_ptr<DataLib::CBuffersCached>(buffers);

    auto g_asionet = net_lib::CAsioNet::create(net_lib::M_CLIENT, host, NET_ADC_STREAMING_PORT, buffers);

    g_asionet->clientConnectNotify.connect([](std::string host) {
        const std::lock_guard lock(g_smutex);
        if (g_soption.verbous)
            aprintf(stdout, "%s Connect %s\n", getTS(": ").c_str(), host.c_str());
        g_runClientCounter--;
    });

    g_asionet->clientErrorNotify.connect([host](std::error_code err) {
        const std::lock_guard lock(g_smutex);
        if (g_soption.verbous)
            aprintf(stdout, "%s Error %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
        stopStreaming(host);
        g_runClientCounter--;
    });

    // auto g_net_buffer_w = std::weak_ptr<streaming_lib::CStreamingNetBuffer>(g_net_buffer);
    g_asionet->reciveNotify.connect([g_s_file_w, g_s_buffers_w, host](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        if (!error) {
            auto obj = g_s_file_w.lock();
            auto obj2 = g_s_buffers_w.lock();
            if (obj && obj2) {
                pack->getInfoFromHeaderADC();
                {  // the statistics: the table of -v, the progress line and the result
                    uint64_t sempCh1 = 0;
                    uint64_t sempCh2 = 0;
                    uint64_t sempCh3 = 0;
                    uint64_t sempCh4 = 0;
                    uint64_t sizeCh1 = 0;
                    uint64_t sizeCh2 = 0;
                    uint64_t sizeCh3 = 0;
                    uint64_t sizeCh4 = 0;

                    uint64_t lostRate = 0;
                    auto ch1 = pack->getBuffer(DataLib::CH1);
                    auto ch2 = pack->getBuffer(DataLib::CH2);
                    auto ch3 = pack->getBuffer(DataLib::CH3);
                    auto ch4 = pack->getBuffer(DataLib::CH4);

                    if (ch1) {
                        sempCh1 = ch1->getSamplesCount();
                        sizeCh1 = ch1->getDataLenght();
                        lostRate += ch1->getLostSamplesAll();
                    }

                    if (ch2) {
                        sempCh2 = ch2->getSamplesCount();
                        sizeCh2 = ch2->getDataLenght();
                        lostRate += ch2->getLostSamplesAll();
                    }

                    if (ch3) {
                        sempCh3 = ch3->getSamplesCount();
                        sizeCh3 = ch3->getDataLenght();
                        lostRate += ch3->getLostSamplesAll();
                    }

                    if (ch4) {
                        sempCh4 = ch4->getSamplesCount();
                        sizeCh4 = ch4->getDataLenght();
                        lostRate += ch4->getLostSamplesAll();
                    }

                    {
                        const std::lock_guard lock(g_levelsMutex);
                        auto& lv = g_levels[host];
                        DataLib::CDataBufferDMA::Ptr chs[4] = {ch1, ch2, ch3, ch4};
                        for (int k = 0; k < 4; k++) {
                            int lo = 0, hi = 0;
                            if (levelOf(chs[k], lo, hi)) {
                                lv.min[k] = std::min(lv.min[k], lo);
                                lv.max[k] = std::max(lv.max[k], hi);
                                lv.seen[k] = true;
                            }
                        }
                        g_queue[host] = obj->getQueueFill();
                        g_diskRate[host] = obj->getWriteRate();
                    }
                    auto flost = obj->getFileLost();
                    auto h = host;
                    addStatisticStreaming(h, sizeCh1 + sizeCh2 + sizeCh3 + sizeCh4, sempCh1, sempCh2, sempCh3, sempCh4, lostRate, flost, -1);
                }
                obj->passBuffers(pack);
                obj2->unlockBufferRead();
            }
        }
    });
    g_asionet->start();
    auto beginTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    auto curTime = beginTime;
    bool timedOut = false;
    while (g_file_manager->isFileThreadWork() && !g_terminate[host]) {
        sleepMs(1);
        if (g_soption.timeout >= 0) {
            if (curTime - beginTime >= g_soption.timeout) {
                timedOut = true;
                break;
            }
            curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
        }
        if (g_soption.verbous) {
            printStatisitc(false);
        }
    }

    // not the time, the limit or Ctrl+C: the board or the connection ended it
    const bool byBoard = g_terminate[host] && !sig_exit_flag && g_file_manager->isFileThreadWork();
    if (byBoard) {
        const std::lock_guard lock(g_smutex);
        g_endedByBoard[host] = true;
    }
    g_file_manager->stopAndFlush();
    // after the stop: one that began on another thread (a limit) has told its reason by now
    {
        const int r = *fileStop;
        const char* why = r == streaming_lib::CStreamingFile::REACH_LIMIT ? (g_file_manager->endedOnTheClock() ? "clock" : "limit")
                          : r == streaming_lib::CStreamingFile::OUT_SPACE ? "disk_full"
                          : timedOut                                       ? "timeout"
                          : sig_exit_flag                                  ? "stopped"
                          : byBoard                                        ? "board"
                          : g_streamFailed                                 ? "error"
                                                                           : "stopped";
        const std::lock_guard lock(g_smutex);
        g_stopReason[host] = why;
    }
    {
        // a recording in parts: the first, the last and how many
        const auto files = g_file_manager->getFileNames();
        const std::lock_guard lock(g_smutex);
        g_fileList[host] = files;
        if (files.size() > 1)
            g_files[host] = files.front() + " ... " + files.back() + " (" + std::to_string(files.size()) + " parts)";
    }
    g_asionet->stop();
    g_asionet = nullptr;
    if (g_soption.streamign_type == ClientOpt::StreamingType::CSV) {
        const std::lock_guard lock(g_s_csv_mutex);
        // every part of the recording into a CSV of its own
        g_converter[host] = converter_lib::CConverter::create();
        // Ctrl+C stops the part converted now and the ones after it
        for (const auto& fileName : g_file_manager->getFileNames()) {
            if (sig_exit_flag)
                break;
            g_converter[host]->convertToCSV(fileName, host, FH_CSV_ADD_INDEX);
        }
    }
    delete memoryManager;
    g_clientsRunning--;
}

// The lines of the streaming panel
static auto streamingPanel(const std::map<std::string, Levels>& levels, bool keys) -> std::vector<std::string> {
    static const char* formats[] = {"", "TDMS", "WAV", "CSV", "BIN"};
    const auto stats = statSnapshot();
    const double secs = statSeconds();
    std::map<std::string, std::string> files;
    {
        const std::lock_guard lock(g_smutex);
        files = g_files;
    }
    char buf[256];
    std::vector<std::string> lines;

    // the head: what, where, how far
    // the directory last: a narrow terminal cuts it, not the progress
    std::string head = Panel::bold("rpsa_client -s") + "  " + std::to_string(files.size()) + (files.size() == 1 ? " board  " : " boards  ") +
                       formats[(int)g_soption.streamign_type];
    double fraction = -1;
    double left = -1;  // seconds to the end of the run, -1 unknown
    if (g_soption.timeout > 0) {
        fraction = secs * 1000.0 / g_soption.timeout;
        left = std::max(0.0, g_soption.timeout / 1000.0 - secs);
        snprintf(buf, sizeof(buf), "   %.1f s of %.1f s ", secs, g_soption.timeout / 1000.0);
    } else if (g_soption.samples > 0) {
        uint64_t least = UINT64_MAX;
        for (auto& [h, st] : stats)
            least = std::min(least, st.samples);
        fraction = stats.empty() ? 0 : (double)least / g_soption.samples;
        // at the rate so far
        if (!stats.empty() && least > 0 && secs > 1)
            left = (g_soption.samples - std::min<uint64_t>(least, g_soption.samples)) / (least / secs);
        snprintf(buf, sizeof(buf), "   %.1f s ", secs);  // the limit in samples: the bar
    } else {
        snprintf(buf, sizeof(buf), "   %.1f s ", secs);
    }
    head += buf;
    if (fraction >= 0) {
        snprintf(buf, sizeof(buf), " %3.0f%%", std::min(fraction, 1.0) * 100);
        head += Panel::bar(fraction, 16) + buf;
        if (left >= 0)
            head += "  " + durationText(left) + " left";
    }
    head += "  -> " + g_soption.save_dir;
    lines.push_back(head);
    lines.push_back("");
    lines.push_back(Panel::dim(Panel::left("  board", 16) + Panel::left("state", 11) + Panel::right("samples/ch", 11) + Panel::right("kS/s", 9) +
                               Panel::right("data", 10) + Panel::right("lost b/f", 9) + Panel::right("disk/s", 7) + Panel::right("queue", 6)));
    for (auto& [host, file] : files) {
        HostStat st;
        auto it = stats.find(host);
        if (it != stats.end())
            st = it->second;
        TrigInfo trig;
        {
            const std::lock_guard lock(g_trigMutex);
            if (g_trig.count(host))
                trig = g_trig[host];
        }
        std::string state = g_endedByBoard.count(host) && g_endedByBoard.at(host) ? Panel::red("stopped")
                            : g_terminate.count(host) && g_terminate[host]          ? "done"
                            : trig.known && trig.armed                              ? Panel::yellow("armed")
                            : st.bytes == 0                                         ? "connecting"
                                                                                    : Panel::green("streaming");
        const double rate = secs > 0 ? st.samples / secs : 0;
        snprintf(buf, sizeof(buf), "%.1f", rate / 1000.0);
        std::string rateText = buf;
        std::string lost = std::to_string(st.lost) + "/" + std::to_string(st.fileLost);
        if (st.lost || st.fileLost)
            lost = Panel::red(lost);
        double q = 0, disk = 0;
        {
            const std::lock_guard lock(g_levelsMutex);
            if (g_queue.count(host))
                q = g_queue[host];
            if (g_diskRate.count(host))
                disk = g_diskRate[host];
        }
        // the write speed of the file: measured each 32 MB or second of writing, "-" before that
        // short: the line keeps to 80 columns
        if (disk >= 1024.0 * 1024 * 1024)
            snprintf(buf, sizeof(buf), "%.1fG", disk / (1024.0 * 1024 * 1024));
        else if (disk >= 1024.0 * 1024)
            snprintf(buf, sizeof(buf), "%.1fM", disk / (1024.0 * 1024));
        else
            snprintf(buf, sizeof(buf), "%.0fk", disk / 1024.0);
        const std::string diskText = disk > 0 ? std::string(buf) : "-";
        snprintf(buf, sizeof(buf), "%.0f %%", q * 100);
        std::string queue = q > 0.5 ? Panel::red(buf) : std::string(buf);
        lines.push_back(Panel::left("  " + host, 16) + Panel::left(state, 11) + Panel::right(std::to_string(st.samples), 11) + Panel::right(rateText, 9) +
                        Panel::right(sizeText(st.bytes), 10) + Panel::right(lost, 9) + Panel::right(diskText, 7) + Panel::right(queue, 6));
        // the levels: min..max of the last second per channel, % of the full scale
        auto lv = levels.find(host);
        std::string l = Panel::left("", 16) + Panel::dim("levels  ");
        bool any = false;
        if (lv != levels.end()) {
            for (int k = 0; k < 4; k++) {
                if (!lv->second.seen[k])
                    continue;
                const bool clip = lv->second.min[k] <= -99 || lv->second.max[k] >= 99;
                snprintf(buf, sizeof(buf), "IN%d %+d..%+d %%  ", k + 1, lv->second.min[k], lv->second.max[k]);
                l += clip ? Panel::red(buf) : std::string(buf);
                any = true;
            }
        }
        lines.push_back(any ? l : Panel::left("", 16) + Panel::dim("levels  -"));
        if (trig.on) {
            std::string t = Panel::left("", 16) + Panel::dim("trigger") + "  ";
            if (trig.known)
                t += std::string(trig.armed ? "armed" : "fired") + " · " + std::to_string(trig.count) + (trig.count == 1 ? " trigger · " : " triggers · ");
            lines.push_back(t + trig.text);
        }
    }
    lines.push_back("");
    const auto free = getFreeSpaceDisk(g_soption.save_dir);
    // when the disk is full: at what the files take a second together (the data received; the
    // measured speed of the disk overstates one the data do not keep busy); red when that comes first
    double disk = 0;
    if (secs > 1) {
        uint64_t bytes = 0;
        for (auto& [h, st] : stats)
            bytes += st.bytes;
        disk = bytes / secs;
    }
    std::string space = "  disk " + sizeText(free) + " free";
    bool full = false;
    if (disk > 0) {
        const double fullIn = free / disk;
        space += ", full in " + durationText(fullIn) + " at " + sizeText(static_cast<uint64_t>(disk)) + "/s";
        full = left >= 0 ? fullIn < left : fullIn < 600;
    }
    lines.push_back((full ? Panel::red(space) : Panel::dim(space)) + Panel::dim(std::string("   ") + (keys ? "[q] " : "") + "[Ctrl+C] stop"));
    return lines;
}

// --json: a progress event a board, a part event for each new part
static auto jsonProgress() -> void {
    const auto stats = statSnapshot();
    const double secs = statSeconds();
    std::map<std::string, std::weak_ptr<streaming_lib::CStreamingFile>> objs;
    {
        const std::lock_guard lock(g_smutex);
        objs = g_fileObj;
    }
    for (auto& [host, w] : objs) {
        HostStat st;
        if (stats.count(host))
            st = stats.at(host);
        TrigInfo trig;
        {
            const std::lock_guard lock(g_trigMutex);
            if (g_trig.count(host))
                trig = g_trig[host];
        }
        double q = 0, disk = 0;
        {
            const std::lock_guard lock(g_levelsMutex);
            if (g_queue.count(host))
                q = g_queue[host];
            if (g_diskRate.count(host))
                disk = g_diskRate[host];
        }
        const char* state = g_endedByBoard.count(host) && g_endedByBoard.at(host) ? "stopped"
                            : g_terminate.count(host) && g_terminate[host]          ? "done"
                            : trig.known && trig.armed                              ? "armed"
                            : st.bytes == 0                                         ? "connecting"
                                                                                    : "streaming";
        Json::Value e;
        e["host"] = host;
        e["state"] = state;
        e["seconds"] = secs;
        e["samples"] = Json::UInt64(st.samples);
        e["bytes"] = Json::UInt64(st.bytes);
        e["rate"] = secs > 0 ? st.bytes / secs : 0.0;
        e["lost_board"] = Json::UInt64(st.lost);
        e["lost_file"] = Json::UInt64(st.fileLost);
        e["queue"] = q;
        e["disk_rate"] = disk;
        if (trig.on) {
            Json::Value t;
            t["known"] = trig.known;
            t["armed"] = trig.armed;
            t["count"] = Json::UInt64(trig.count);
            e["trigger"] = t;
        }
        if (auto f = w.lock()) {
            e["part"] = f->getPart();
            e["file"] = f->getCSVFileName();
            e["part_bytes"] = Json::UInt64(f->getPartSize());
            // the parts begun since the last event
            const auto names = f->getFileNames();
            size_t told = 1;
            {
                const std::lock_guard lock(g_smutex);
                told = g_partsTold[host];
                g_partsTold[host] = std::max(told, names.size());
            }
            for (size_t k = told; k < names.size(); k++) {
                Json::Value p;
                p["host"] = host;
                p["part"] = Json::UInt64(k + 1);
                p["file"] = names[k];
                jsonev::emit("part", p);
            }
        }
        jsonev::emit("progress", e);
    }
}

// a line a board for a log or a pipe: the state, the samples, the data, the rate, the losses
static auto plainLines() -> std::vector<std::string> {
    const auto stats = statSnapshot();
    const double secs = statSeconds();
    std::map<std::string, std::string> files;
    {
        const std::lock_guard lock(g_smutex);
        files = g_files;
    }
    std::vector<std::string> lines;
    for (auto& [host, file] : files) {
        HostStat st;
        if (stats.count(host))
            st = stats.at(host);
        TrigInfo trig;
        {
            const std::lock_guard lock(g_trigMutex);
            if (g_trig.count(host))
                trig = g_trig[host];
        }
        double q = 0;
        {
            const std::lock_guard lock(g_levelsMutex);
            if (g_queue.count(host))
                q = g_queue[host];
        }
        const char* state = g_endedByBoard.count(host) && g_endedByBoard.at(host) ? "stopped"
                            : g_terminate.count(host) && g_terminate[host]          ? "done"
                            : trig.known && trig.armed                              ? "armed"
                            : st.bytes == 0                                         ? "connecting"
                                                                                    : "streaming";
        char buf[320];
        snprintf(buf, sizeof(buf), "%s %s: %s, %llu samples/ch, %s, %s/s, lost %llu/%llu, queue %.0f %%", clockText().c_str(), host.c_str(), state,
                 (unsigned long long)st.samples, sizeText(st.bytes).c_str(), sizeText(secs > 0 ? (uint64_t)(st.bytes / secs) : 0).c_str(), (unsigned long long)st.lost,
                 (unsigned long long)st.fileLost, q * 100);
        std::string l = buf;
        if (trig.on && trig.known)
            l += ", trigger " + std::string(trig.armed ? "armed" : "fired") + " (" + std::to_string(trig.count) + ")";
        lines.push_back(l);
    }
    return lines;
}

auto startStreaming(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool {
    g_soption = option;
    if (g_soption.save_dir.empty())
        g_soption.save_dir = ".";
    g_streamFailed = false;

    char time_str[40];
    struct tm* timenow;
    time_t now = time(nullptr);
    timenow = gmtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
    g_filenameDate = time_str;

    if (option.save_dir != "")
        streaming_lib::CStreamingFile::makeEmptyDir(option.save_dir);  // before its free space is asked
    auto size = getFreeSpaceDisk(option.save_dir != "" ? option.save_dir : ".");
    if (g_soption.verbous)
        aprintf(stdout, "%s Free disk space:  %lld Mb\n", getTS(": ").c_str(), size / (1024 * 1024));

    resetStreamingCounter();
    g_files.clear();
    g_endedByBoard.clear();

    ClientOpt::Options remote_opt = g_soption;
    remote_opt.mode = ClientOpt::Mode::REMOTE;
    remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
    remote_opt.verbous = g_soption.verbous;
    std::map<std::string, StateRunnedHosts> runned_hosts;
    if (!startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
        aprintf(stdout, "%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
        return false;
    }

    std::map<std::string, uint32_t> blockSizes;
    auto hosts = cl->getHosts();
    if (!requestMemoryBlockSize(cl, hosts, &blockSizes, remote_opt.verbous)) {
        aprintf(stdout, "%s Can't get block sizes\n", getTS(": ").c_str());
        return false;
    }

    std::map<std::string, adc_channels_t> activeChannels;
    if (!requestActiveChannels(cl, hosts, &activeChannels, remote_opt.verbous)) {
        aprintf(stdout, "%s Can't get active channels\n", getTS(": ").c_str());
        return false;
    }

    readTriggers(cl, hosts);

    runned_hosts.clear();
    remote_opt.remote_mode = ClientOpt::RemoteMode::START;
    if (startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
        // A board that writes to its own SD card sends nothing here: the client would wait forever
        bool local = false;
        for (auto& kv : runned_hosts) {
            if (kv.second == StateRunnedHosts::LOCAL) {
                aprintf(stderr,
                        "%s [ERROR] %s writes the ADC to its own SD card (adc_pass_mode=FILE), not to this client.\n"
                        "        To stream to the network: rpsa_client -c -h %s -i adc_pass_mode=NET\n"
                        "        To start the recording on the board: rpsa_client -r -h %s -m start\n",
                        getTS(": ").c_str(),
                        kv.first.c_str(),
                        kv.first.c_str(),
                        kv.first.c_str());
                local = true;
            }
        }
        if (local) {
            remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
            startRemote(cl, remote_opt, nullptr, &runned_hosts);
            g_converter.clear();
            return false;
        }
        if (runned_hosts.empty()) {
            aprintf(stderr, "%s [ERROR] No board started the streaming\n", getTS(": ").c_str());
            g_converter.clear();
            return false;
        }
        int started = 0;
        for (auto kv : runned_hosts) {
            if (kv.second == StateRunnedHosts::TCP && activeChannels[kv.first].count() > 0) {
                g_clientsRunning++;
                clients.push_back(std::thread(runClient, kv.first, kv.second, blockSizes[kv.first], activeChannels[kv.first]));
                started++;
            } else if (kv.second == StateRunnedHosts::TCP) {
                aprintf(stderr, "%s [ERROR] %s has no active channel: rpsa_client -c -h %s -i channel_state_1=ON\n", getTS(": ").c_str(), kv.first.c_str(), kv.first.c_str());
                jsonError(kv.first, "no active channel (channel_state_N=ON)");
                g_streamFailed = true;
            }
        }
        g_runClientCounter = started;
        // the data connections; a board that does not connect ends the streaming
        if (!waitAnswers(g_runClientCounter, [] { return sig_exit_flag.load(); }, "the data connection") && !sig_exit_flag) {
            g_streamFailed = true;
            stopStreaming();
        }

        remote_opt.remote_mode = ClientOpt::RemoteMode::START_FPGA_ADC;
        std::vector<std::string> runnedThreads;
        for (const auto& [key, _] : runned_hosts) {
            runnedThreads.push_back(key);
        }
        if (!startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
            aprintf(stdout, "%s Can't start ADC on remote machines\n", getTS(": ").c_str());
            g_streamFailed = true;
            for (auto& h : runnedThreads) {
                if (runned_hosts.count(h) == 0)
                    stopStreaming(h);
            }
        }

        cl->errorNofiy.connect([](ClientNetConfigManager::Errors errors, std::string host, std::error_code err) {
            if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL) {
                g_streamFailed = true;
                aprintf(stderr, "%s Error: %s %s\n", getTS(": ").c_str(), host.c_str(), err.message().c_str());
                jsonError(host, err.message());
            }
            stopStreaming(host);
        });

        cl->serverStoppedNofiy.connect([](std::string host) {
            if (g_soption.verbous)
                aprintf(stderr, "%s Streaming stopped: %s [OK]\n", getTS(": ").c_str(), host.c_str());
            stopStreaming(host);
        });

        cl->serverStoppedMemErrorNofiy.connect([](std::string host) {
            g_streamFailed = true;
            aprintf(stderr, "%s [ERROR] Streaming stopped: %s: not enough DMA memory on the board\n", getTS(": ").c_str(), host.c_str());
            jsonError(host, "Streaming stopped: not enough DMA memory on the board");
            stopStreaming(host);
        });

        cl->serverStoppedMemModifyNofiy.connect([](std::string host) {
            g_streamFailed = true;
            aprintf(stderr, "%s [ERROR] Streaming stopped: %s: the memory of the board was split again (a new block_size)\n", getTS(": ").c_str(), host.c_str());
            jsonError(host, "Streaming stopped: the memory of the board was split again (a new block_size)");
            stopStreaming(host);
        });

        cl->serverStoppedNoActiveChannelsNofiy.connect([](std::string host) {
            g_streamFailed = true;
            aprintf(stderr, "%s [ERROR] Streaming stopped: %s: no active channel (channel_state_N=ON)\n", getTS(": ").c_str(), host.c_str());
            jsonError(host, "Streaming stopped: no active channel (channel_state_N=ON)");
            stopStreaming(host);
        });

        cl->serverStoppedConfigErrorNofiy.connect([](std::string host) {
            g_streamFailed = true;
            aprintf(stderr, "%s [ERROR] Streaming stopped: %s: the board did not start the ADC: the settings or the board (journalctl -u redpitaya_streaming on the board)\n",
                    getTS(": ").c_str(), host.c_str());
            jsonError(host, "Streaming stopped: the board did not start the ADC: the settings or the board (journalctl -u redpitaya_streaming on the board)");
            stopStreaming(host);
        });

        cl->serverStoppedSDFullNofiy.connect([](std::string host) {
            g_streamFailed = true;
            aprintf(stderr, "%s [ERROR] Streaming stopped: %s: the SD card of the board is full\n", getTS(": ").c_str(), host.c_str());
            jsonError(host, "Streaming stopped: the SD card of the board is full");
            stopStreaming(host);
        });

        cl->serverStoppedSDDoneNofiy.connect([](std::string host) {
            if (g_soption.verbous)
                aprintf(stderr, "%s Streaming stopped: %s Local mode [OK]\n", getTS(": ").c_str(), host.c_str());
            stopStreaming(host);
        });

        // the trigger of the boards that wait for one, asked every second
        cl->getTriggerStatusNofiy.connect([](std::string& host, TriggerStatus st) {
            const std::lock_guard lock(g_trigMutex);
            if (!g_trig.count(host))
                return;
            g_trig[host].known = true;
            g_trig[host].armed = st.adcArmed;
            g_trig[host].count = st.adcTriggerCount;
        });
        std::list<std::string> trigHosts;
        {
            const std::lock_guard lock(g_trigMutex);
            for (auto& [h, t] : g_trig)
                trigHosts.push_back(h);
        }

        // the panel on a terminal while the boards stream; without one a line a board now and then
        {
            Panel panel(g_soption);
            const bool plain = !panel.enabled() && plainProgressWanted(g_soption);
            std::map<std::string, Levels> last;
            int tick = 0;
            while (g_clientsRunning > 0) {
                if (tick % 4 == 0) {
                    for (auto& h : trigHosts)
                        cl->requestTriggerStatus(h);
                }
                if (panel.enabled()) {
                    if (tick % 4 == 0) {  // the levels of the last second
                        const std::lock_guard lock(g_levelsMutex);
                        last = g_levels;
                        g_levels.clear();
                    }
                    panel.draw(streamingPanel(last, panel.keys()));
                    const int k = panel.key();
                    if (k == 'q' || k == 'Q') {
                        sig_exit_flag = true;
                        stopStreaming();
                    }
                } else if (jsonev::on() && tick > 0 && tick % 4 == 0) {
                    jsonProgress();
                } else if (plain && tick > 0 && tick % (PLAIN_PROGRESS_MS / 250) == 0) {
                    for (auto& l : plainLines())
                        aprintf(stdout, "%s\n", l.c_str());
                    fflush(stdout);
                }
                tick++;
                sleepMs(250);
            }
        }
        for (std::thread& t : clients) {
            if (t.joinable()) {
                t.join();
            }
        }
        clients.clear();
        if (!g_endedByBoard.empty())
            sleepMs(300);  // the closed config connection is seen
        for (auto& [host, ended] : g_endedByBoard) {
            if (!ended)
                continue;
            g_streamFailed = true;
            aprintf(stderr, "%s [ERROR] %s ended the streaming early: %s\n", getTS(": ").c_str(), host.c_str(), endedText(cl->isConnected(host)));
            jsonError(host, std::string("ended the streaming early: ") + endedText(cl->isConnected(host)));
        }

        remote_opt.remote_mode = ClientOpt::RemoteMode::STOP;
        if (!startRemote(cl, remote_opt, nullptr, &runned_hosts)) {
            aprintf(stdout, "%s Can't stop streaming on remote machines\n", getTS(": ").c_str());
            g_converter.clear();
            return false;
        }

        if (g_soption.verbous) {
            const std::lock_guard<std::mutex> lock(g_smutex);
            printFinalStatisitc();
        }
        for (auto& [host, file] : g_files)
            aprintf(stdout, "%s\n", resultLine(host, g_soption.streamign_type == ClientOpt::StreamingType::CSV ? file + " (and its .csv)" : file, !g_endedByBoard[host]).c_str());
        if (jsonev::on()) {
            const auto stats = statSnapshot();
            const double secs = statSeconds();
            const std::lock_guard lock(g_smutex);
            for (auto& [host, file] : g_files) {
                HostStat st;
                if (stats.count(host))
                    st = stats.at(host);
                Json::Value e;
                e["host"] = host;
                e["ok"] = !g_endedByBoard[host];
                e["reason"] = g_stopReason.count(host) ? g_stopReason[host] : std::string("error");
                e["seconds"] = secs;
                e["samples"] = Json::UInt64(st.samples);
                e["bytes"] = Json::UInt64(st.bytes);
                e["lost_board"] = Json::UInt64(st.lost);
                e["lost_file"] = Json::UInt64(st.fileLost);
                Json::Value files(Json::arrayValue), csv(Json::arrayValue);
                const auto list = g_fileList.count(host) ? g_fileList[host] : std::vector<std::string>{file};
                // the parts begun after the last progress event
                for (size_t k = g_partsTold.count(host) ? g_partsTold[host] : list.size(); k < list.size(); k++) {
                    Json::Value p;
                    p["host"] = host;
                    p["part"] = Json::UInt64(k + 1);
                    p["file"] = list[k];
                    jsonev::emit("part", p);
                }
                for (auto& f : list) {
                    files.append(f);
                    // CSV: each part converted next to it
                    std::error_code ec;
                    const auto c = std::filesystem::path(f).replace_extension(".csv").string();
                    if (g_soption.streamign_type == ClientOpt::StreamingType::CSV && std::filesystem::exists(c, ec))
                        csv.append(c);
                }
                e["files"] = files;
                if (g_soption.streamign_type == ClientOpt::StreamingType::CSV)
                    e["csv"] = csv;
                jsonev::emit("done", e);
            }
        }
    } else {
        aprintf(stderr, "%s [ERROR] Can't start streaming on the boards\n", getTS(": ").c_str());
        jsonError("", "Can't start streaming on the boards");
        g_streamFailed = true;
    }
    g_converter.clear();
    return !g_streamFailed;
}

auto streamingSIGHandler() -> void {
    stopCSV();
    stopStreaming();
    sig_exit_flag = true;
}

auto stopCSV() -> void {
    for (const auto& kv : g_converter) {
        kv.second->stopWriteToCSV();
    }
}

auto stopStreaming(std::string host) -> void {
    g_terminate[host] = true;
}

auto stopStreaming() -> void {
    for (auto& kv : g_terminate) {
        kv.second = true;
    }
}
