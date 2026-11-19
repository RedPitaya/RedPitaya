#include "gpio_client.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>

#include "bin_lib/gpio_words.h"
#include "config.h"
#include "data_lib/buffers_cached.h"
#include "data_lib/network_header.h"
#include "logger_lib/file_logger.h"
#include "net_lib/asio_net.h"
#include "panel.h"
#include "reader_lib/gpio_file_reader.h"
#include "remote.h"
#include "test_helper.h"
#include "uio_lib/memory_manager.h"
#include "writer_lib/gpio_file_writer.h"

namespace {

std::atomic<bool> g_gpioStop(false);  // Ctrl+C, q or the time

// A ring of the board's block size in memory of its own, one channel
struct Ring {
    uio_lib::CMemoryManager memory;
    DataLib::CBuffersCached::Ptr buffers = DataLib::CBuffersCached::create();

    Ring(uint32_t blockSize, uio_lib::MemoryTAG tag) {
        memory.setMemoryBlockSize(blockSize);
        memory.reallocateBlocks();
        memory.reserveMemory(tag, memory.getFreeBlockCount(), 1);
        adc_channels_t one;
        one[ADCChannels::ADC_CH1] = true;
        buffers->generateBuffersEmptyADC(one, memory.getRegions(tag), DataLib::sizeHeader());
    }
};

struct InState {
    std::mutex m;
    bool connected = false;
    bool opened = false;
    bool limit = false;      // the sample limit is in the file
    bool fileError = false;  // the file could not be written
    bool lost = false;       // the data connection broke
    bool closing = false;    // this client closes it
    bool done = false;       // the thread ended
    uint64_t samples = 0, words = 0, bytes = 0, lostSamples = 0, waitSamples = 0, packs = 0;
    uint32_t clock = 0, decimation = 0;
    bool rle = false;
};

struct OutState {
    std::mutex m;
    bool connected = false;
    bool fileEnded = false;  // every word was sent
    bool lost = false;       // the data connection broke
    bool closing = false;    // this client closes it
    bool done = false;
    long long endedAt = 0;   // when the last word went out
    uint64_t words = 0, bytes = 0, packs = 0;
};

InState g_in;
OutState g_out;

// the replies of the board after the start: the ends of the directions
std::atomic<bool> g_inStopped(false), g_outStopped(false), g_inFailed(false), g_outFailed(false);
std::atomic<uint64_t> g_outWait(0);

auto nowMs() -> long long {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

// The GPIO settings of the board: RLE (the output words) and the pass modes
auto boardSettings(std::shared_ptr<ClientNetConfigManager> cl, const std::string& host, bool& rle, bool& inFile, bool& outFile) -> bool {
    std::atomic<int> count{1};
    std::atomic<bool> got{false};
    cl->getNewSettingsNofiy.connect([&](std::string h) {
        if (h != host)
            return;
        if (auto s = cl->getLocalSettingsOfHost(h)) {
            rle = s->getGPIORLE().value == CStreamSettings::State::ON;
            inFile = s->getGPIOInPassMode().value == CStreamSettings::PassMode::FILE;
            outFile = s->getGPIOOutPassMode().value == CStreamSettings::PassMode::FILE;
            got = true;
        }
        count--;
    });
    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors e, std::string, std::error_code) {
        if (e == ClientNetConfigManager::Errors::SERVER_INTERNAL || e == ClientNetConfigManager::Errors::CANNT_SET_DATA_TO_CONFIG)
            count--;
    });
    if (!cl->requestConfig(host))
        count--;
    const bool ok = waitAnswers(count, [] { return g_gpioStop.load(); }, "the request of the settings");
    cl->removeHadlers();
    return ok && got;
}

// Starts the input or the output of the board for the network: true when it answers so
auto startDirection(std::shared_ptr<ClientNetConfigManager> cl, const std::string& host, bool output) -> bool {
    std::atomic<int> count{1};
    std::atomic<bool> started{false};
    std::string why;
    std::mutex m;
    const char* dir = output ? "output" : "input";
    auto fail = [&](const std::string& text) {
        const std::lock_guard lock(m);
        if (count <= 0)
            return;
        why = text;
        count--;
    };
    auto ok = [&]() {
        const std::lock_guard lock(m);
        if (count <= 0)
            return;
        started = true;
        count--;
    };
    if (output) {
        cl->serverGpioOutStartedNofiy.connect([&](std::string h) { if (h == host) ok(); });
        cl->serverGpioOutStartedSDNofiy.connect([&](std::string h) {
            if (h == host)
                fail("it plays a file of its own (gpio_out_pass_mode=FILE). To play from here: rpsa_client -c -h " + host + " -i gpio_out_pass_mode=NET");
        });
        cl->serverGpioOutStoppedMemErrorNofiy.connect([&](std::string h) { if (h == host) fail("not enough memory for the GPIO output (gpio_out_size)"); });
        cl->serverGpioOutStoppedConfigErrorNofiy.connect([&](std::string h) { if (h == host) fail("the GPIO output did not start: the settings, no GPIO block in the FPGA or the data port taken (journalctl -u redpitaya_streaming on the board)"); });
        cl->serverGpioOutStoppedNofiy.connect([&](std::string h, uint64_t) { if (h == host) fail("it stopped the output at once"); });
    } else {
        cl->serverGpioInStartedNofiy.connect([&](std::string h) { if (h == host) ok(); });
        cl->serverGpioInStartedSDNofiy.connect([&](std::string h) {
            if (h == host)
                fail("it writes the input to a file of its own (gpio_in_pass_mode=FILE). To record here: rpsa_client -c -h " + host + " -i gpio_in_pass_mode=NET");
        });
        cl->serverGpioInStoppedMemErrorNofiy.connect([&](std::string h) { if (h == host) fail("not enough memory for the GPIO input (gpio_in_size)"); });
        cl->serverGpioInStoppedConfigErrorNofiy.connect([&](std::string h) { if (h == host) fail("the GPIO input did not start: the settings, no GPIO block in the FPGA or the data port taken (journalctl -u redpitaya_streaming on the board)"); });
        cl->serverGpioInStoppedNofiy.connect([&](std::string h) { if (h == host) fail("it stopped the input at once"); });
    }
    cl->errorNofiy.connect([&](ClientNetConfigManager::Errors e, std::string h, std::error_code err) {
        if (e == ClientNetConfigManager::Errors::SERVER_INTERNAL && h == host)
            fail("the connection broke: " + err.message());
    });
    if (!(output ? cl->sendGPIOOutServerStart(host) : cl->sendGPIOInServerStart(host)))
        fail("the command could not be sent");
    const std::string what = std::string("the start of the GPIO ") + dir;
    const bool answered = waitAnswers(count, [] { return g_gpioStop.load(); }, what.c_str());
    cl->removeHadlers();
    if (answered && !started && !why.empty())
        aprintf(stderr, "%s [ERROR] %s refused the GPIO %s: %s\n", getTS(": ").c_str(), host.c_str(), dir, why.c_str());
    return started;
}

// The input into the file
auto runInput(std::string host, uint32_t blockSize, std::string path, CStreamSettings::GPIOFormat format, uint64_t limit, int timeoutMs, bool withOutput) -> void {
    Ring ring(blockSize, uio_lib::MM_GPIO_IN);
    CGpioFileWriter writer;
    auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, host, NET_GPIO_IN_STREAMING_PORT, ring.buffers);
    net->setPackID(DataLib::ID_PACK_GPIO);
    net->clientConnectNotify.connect([](std::string) {
        const std::lock_guard lock(g_in.m);
        g_in.connected = true;
    });
    net->clientErrorNotify.connect([](std::error_code) {
        const std::lock_guard lock(g_in.m);
        if (!g_gpioStop && !g_in.limit && !g_in.closing)
            g_in.lost = true;
    });
    net->reciveNotify.connect([&](std::error_code error, DataLib::CDataBuffersPackDMA::Ptr pack) {
        if (!error) {
            auto buffer = pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1);
            DataLib::GPIOHeader h;
            const std::lock_guard lock(g_in.m);
            // a buffer after the close of the file is not taken
            if (!g_in.closing && DataLib::getHeaderGPIO(buffer, h) && !g_in.limit && !g_in.fileError) {
                const bool rle = h.flags & DataLib::GPIO_FLAG_RLE;
                if (!g_in.opened) {
                    // the file takes the clock, the decimation and the modes of the first buffer
                    GpioRecording rec;
                    rec.format = format;
                    rec.rle = rle;
                    rec.clock = h.gpioClock;
                    rec.decimation = h.decimation ? h.decimation : 1;
                    rec.triggerMode = h.flags & DataLib::GPIO_FLAG_TRIGGER_MODE;
                    rec.waitCounter = h.flags & DataLib::GPIO_FLAG_WAIT_COUNTER;
                    rec.startTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
                    rec.samplesLimit = limit;
                    g_in.opened = writer.open(path, rec);
                    g_in.fileError = !g_in.opened;
                    g_in.clock = h.gpioClock;
                    g_in.decimation = rec.decimation;
                    g_in.rle = rle;
                }
                if (g_in.opened) {
                    const size_t count = std::min<size_t>(h.dataSize, buffer->getDataLenght()) / 4;
                    auto words = static_cast<const uint32_t*>(buffer->getMappedDataMemory());
                    g_in.samples += bin_lib::gpioInSamples(words, count, rle);
                    g_in.words += count;
                    g_in.bytes += count * 4;
                    g_in.lostSamples += h.lostFPGA;
                    g_in.waitSamples += h.waitFPGA;
                    g_in.packs++;
                    switch (writer.write(words, count, h.lostFPGA, h.waitFPGA, h.triggerCount)) {
                        case CGpioFileWriter::Result::OK:
                            break;
                        case CGpioFileWriter::Result::LIMIT:
                            g_in.limit = true;
                            break;
                        default:
                            g_in.fileError = true;
                    }
                }
            }
        }
        ring.buffers->unlockBufferRead();
    });
    net->start();
    const auto begin = nowMs();
    while (!g_gpioStop) {
        {
            const std::lock_guard lock(g_in.m);
            if (g_in.limit || g_in.fileError || g_in.lost)
                break;
        }
        if (timeoutMs > 0 && nowMs() - begin >= timeoutMs)
            break;
        if (g_inStopped || g_inFailed)
            break;
        // recording what the output plays: done a moment after the board played the file out
        if (withOutput && limit == 0 && timeoutMs <= 0 && g_outStopped) {
            sleepMs(200);
            break;
        }
        sleepMs(20);
    }
    {
        const std::lock_guard lock(g_in.m);
        g_in.closing = true;
    }
    net->stop();
    writer.close();
    const std::lock_guard lock(g_in.m);
    g_in.done = true;
}

// The file on the output; the connection closes after the last word: the board plays what it got
auto runOutput(std::string host, uint32_t blockSize, std::shared_ptr<CGpioFileReader> reader, int timeoutMs) -> void {
    Ring ring(blockSize, uio_lib::MM_GPIO_OUT);
    auto net = net_lib::CAsioNet::create(net_lib::M_CLIENT, host, NET_GPIO_OUT_STREAMING_PORT, nullptr);
    std::atomic<bool> connected{false};
    net->clientConnectNotify.connect([&](std::string) { connected = true; });
    net->clientErrorNotify.connect([](std::error_code) {
        const std::lock_guard lock(g_out.m);
        if (!g_gpioStop && !g_out.fileEnded && !g_out.closing)
            g_out.lost = true;
    });
    net->start();
    const auto connectEnd = nowMs() + replyTimeout();
    while (!connected && !g_gpioStop && nowMs() < connectEnd)
        sleepMs(10);
    {
        const std::lock_guard lock(g_out.m);
        g_out.connected = connected;
        if (!connected && !g_gpioStop)
            g_out.lost = true;
    }
    auto pack = connected ? ring.buffers->writeBuffer(true) : nullptr;
    auto buffer = pack ? pack->getBuffer(DataLib::EDataBuffersPackChannel::CH1) : nullptr;
    const size_t maxWords = buffer ? buffer->getDataLenght() / 4 : 0;
    auto data = buffer ? static_cast<uint32_t*>(buffer->getMappedDataMemory()) : nullptr;
    uint64_t id = 0;
    const auto begin = nowMs();
    while (buffer && !g_gpioStop && !g_outStopped && !g_outFailed) {
        if (timeoutMs > 0 && nowMs() - begin >= timeoutMs)
            break;
        size_t got = 0;
        const auto result = reader->read(data, maxWords, got);
        if (got > 0) {
            DataLib::GPIOHeader h;
            h.dataSize = static_cast<uint32_t>(got * 4);
            DataLib::setHeaderGPIO(buffer, id++, h);
            if (!net->sendSyncData(pack)) {
                const std::lock_guard lock(g_out.m);
                if (!g_gpioStop)
                    g_out.lost = true;
                break;
            }
            const std::lock_guard lock(g_out.m);
            g_out.words += got;
            g_out.bytes += got * 4;
            g_out.packs++;
        }
        if (result == CGpioFileReader::Result::ENDED) {
            const std::lock_guard lock(g_out.m);
            g_out.fileEnded = true;
            g_out.endedAt = nowMs();
            break;
        }
    }
    {
        const std::lock_guard lock(g_out.m);
        g_out.closing = true;
    }
    net->stop();
    const std::lock_guard lock(g_out.m);
    g_out.done = true;
}

auto formatOf(const ClientOpt::Options& option, CStreamSettings::GPIOFormat& f) -> bool {
    std::string name = option.gpio_format;
    if (name.empty()) {
        auto dot = option.gpio_in_file.find_last_of('.');
        name = dot == std::string::npos ? "" : option.gpio_in_file.substr(dot + 1);
    }
    for (auto& c : name)
        c = static_cast<char>(tolower(c));
    if (name == "bin")
        f = CStreamSettings::GPIOFormat::BIN;
    else if (name == "csv")
        f = CStreamSettings::GPIOFormat::CSV;
    else if (name == "vcd")
        f = CStreamSettings::GPIOFormat::VCD;
    else
        return false;
    return true;
}

}  // namespace

auto gpioSIGHandler() -> void {
    g_gpioStop = true;
}

auto startGPIO(std::shared_ptr<ClientNetConfigManager> cl, ClientOpt::Options& option) -> bool {
    g_gpioStop = false;
    g_inStopped = g_outStopped = g_inFailed = g_outFailed = false;
    g_outWait = 0;
    const std::string host = option.hosts.front();
    const bool wantIn = !option.gpio_in_file.empty();
    const bool wantOut = !option.gpio_out_file.empty();

    CStreamSettings::GPIOFormat format = CStreamSettings::GPIOFormat::BIN;
    if (wantIn && !formatOf(option, format)) {
        aprintf(stderr, "[ERROR] The format of %s: give it with -f bin|csv|vcd or as the extension\n", option.gpio_in_file.c_str());
        return false;
    }

    std::map<std::string, uint32_t> sizes;
    std::list<std::string> hosts{host};
    if (!requestMemoryBlockSize(cl, hosts, &sizes, option.verbous) || !sizes.count(host)) {
        aprintf(stderr, "%s [ERROR] Can't get the memory block size of %s\n", getTS(": ").c_str(), host.c_str());
        return false;
    }
    bool rle = false, inFile = false, outFile = false;
    if (!boardSettings(cl, host, rle, inFile, outFile)) {
        aprintf(stderr, "%s [ERROR] Can't get the settings of %s\n", getTS(": ").c_str(), host.c_str());
        return false;
    }
    if (wantIn && inFile) {
        aprintf(stderr, "%s [ERROR] %s writes the GPIO input to a file of its own (gpio_in_pass_mode=FILE).\n        To record here: rpsa_client -c -h %s -i gpio_in_pass_mode=NET\n",
                getTS(": ").c_str(), host.c_str(), host.c_str());
        return false;
    }
    if (wantOut && outFile) {
        aprintf(stderr, "%s [ERROR] %s plays a GPIO file of its own (gpio_out_pass_mode=FILE).\n        To play from here: rpsa_client -c -h %s -i gpio_out_pass_mode=NET\n",
                getTS(": ").c_str(), host.c_str(), host.c_str());
        return false;
    }

    // the output file first: a file that cannot be played starts nothing
    std::shared_ptr<CGpioFileReader> reader;
    if (wantOut) {
        const bool inf = option.dac_repeat == (int)ClientOpt::RepeatDAC::INF;
        const auto repeat = inf ? CStreamSettings::GPIORepeat::GPIO_REP_INF
                            : option.dac_repeat > 1 ? CStreamSettings::GPIORepeat::GPIO_REP_ON
                                                    : CStreamSettings::GPIORepeat::GPIO_REP_OFF;
        reader = std::make_shared<CGpioFileReader>(option.gpio_out_file, repeat, inf ? 0 : (uint32_t)option.dac_repeat, rle, option.gpio_gaps);
        if (reader->isOpen() != CGpioFileReader::Open::OK) {
            aprintf(stderr, "[ERROR] Can't play %s: %s\n", option.gpio_out_file.c_str(),
                    reader->isOpen() == CGpioFileReader::Open::EMPTY        ? "it holds no GPIO words"
                    : reader->isOpen() == CGpioFileReader::Open::WRONG_TYPE ? "not a GPIO recording or GPIO output words (a BIN stream file of the GPIO)"
                                                                            : reader->error().c_str());
            return false;
        }
    }

    {
        const std::lock_guard a(g_in.m);
        g_in.connected = g_in.opened = g_in.limit = g_in.fileError = g_in.lost = g_in.closing = g_in.done = false;
        g_in.samples = g_in.words = g_in.bytes = g_in.lostSamples = g_in.waitSamples = g_in.packs = 0;
    }
    {
        const std::lock_guard b(g_out.m);
        g_out.connected = g_out.fileEnded = g_out.lost = g_out.closing = g_out.done = false;
        g_out.endedAt = 0;
        g_out.words = g_out.bytes = g_out.packs = 0;
    }

    // the input first: it records the output from its start
    bool inStarted = false, outStarted = false;
    if (wantIn) {
        inStarted = startDirection(cl, host, false);
        if (!inStarted)
            return false;
    }
    if (wantOut) {
        outStarted = startDirection(cl, host, true);
        if (!outStarted) {
            if (inStarted)
                cl->sendGPIOInServerStop(host);
            return false;
        }
    }

    // the board's ends of the directions while they run
    cl->serverGpioInStoppedNofiy.connect([&](std::string h) { if (h == host) g_inStopped = true; });
    cl->serverGpioInStoppedMemErrorNofiy.connect([&](std::string h) { if (h == host) g_inFailed = true; });
    cl->serverGpioInStoppedConfigErrorNofiy.connect([&](std::string h) { if (h == host) g_inFailed = true; });
    cl->serverGpioOutStoppedNofiy.connect([&](std::string h, uint64_t wait) {
        if (h == host) {
            g_outWait = wait;
            g_outStopped = true;
        }
    });
    cl->serverGpioOutStoppedMemErrorNofiy.connect([&](std::string h) { if (h == host) g_outFailed = true; });
    cl->serverGpioOutStoppedConfigErrorNofiy.connect([&](std::string h) { if (h == host) g_outFailed = true; });

    std::thread inThread, outThread;
    if (inStarted)
        inThread = std::thread(runInput, host, sizes[host], option.gpio_in_file, format, (uint64_t)option.samples, option.timeout, outStarted);
    if (outStarted)
        outThread = std::thread(runOutput, host, sizes[host], reader, option.timeout);

    // the panel while it runs; the output ends with its file, the input with the time, the
    // limit or the stop
    const auto begin = nowMs();
    {
        Panel panel(option);
        auto running = [&] {
            const std::lock_guard a(g_in.m);
            const std::lock_guard b(g_out.m);
            // after the last word the board plays what it got and answers: waited for up to the reply timeout
            const bool playingOut = outStarted && g_out.fileEnded && !g_outStopped && !g_gpioStop && nowMs() - g_out.endedAt < replyTimeout();
            return (inStarted && !g_in.done) || (outStarted && !g_out.done) || playingOut;
        };
        // without a terminal: a line every few seconds (streaming_improvements 3.4)
        const bool plain = !panel.enabled() && plainProgressWanted(option);
        auto lastPlain = nowMs();
        while (running()) {
            if (plain && nowMs() - lastPlain >= PLAIN_PROGRESS_MS) {
                lastPlain = nowMs();
                const double secs = (nowMs() - begin) / 1000.0;
                if (inStarted) {
                    const std::lock_guard a(g_in.m);
                    aprintf(stdout, "%s %s in: %s, %llu samples, %llu words, %s, lost %llu, waited %llu\n", clockText().c_str(), host.c_str(),
                            g_in.lost || g_in.fileError ? "stopped" : g_in.done ? "done" : g_in.packs ? "recording" : "waiting", (unsigned long long)g_in.samples,
                            (unsigned long long)g_in.words, sizeText(g_in.bytes).c_str(), (unsigned long long)g_in.lostSamples, (unsigned long long)g_in.waitSamples);
                }
                if (outStarted) {
                    const std::lock_guard b(g_out.m);
                    aprintf(stdout, "%s %s out: %s, %llu words, %s, %s/s\n", clockText().c_str(), host.c_str(),
                            g_out.lost ? "stopped" : g_out.fileEnded ? "playing out" : g_out.connected ? "playing" : "connecting", (unsigned long long)g_out.words,
                            sizeText(g_out.bytes).c_str(), sizeText(secs > 0 ? (uint64_t)(g_out.bytes / secs) : 0).c_str());
                }
                fflush(stdout);
            }
            if (panel.enabled()) {
                const double secs = (nowMs() - begin) / 1000.0;
                char buf[256];
                std::vector<std::string> lines;
                snprintf(buf, sizeof(buf), "   %s   %.1f s", host.c_str(), secs);
                std::string head = Panel::bold("rpsa_client -G") + buf;
                if (option.timeout > 0)
                    head += " " + Panel::bar(secs * 1000.0 / option.timeout, 16);
                lines.push_back(head);
                lines.push_back("");
                if (inStarted) {
                    const std::lock_guard a(g_in.m);
                    const std::string state = g_in.lost || g_in.fileError ? Panel::red("stopped")
                                              : g_in.done                ? std::string("done")
                                              : g_in.packs ? Panel::green("recording")
                                                           : std::string("waiting");
                    snprintf(buf, sizeof(buf), "  in   %s  %llu samples, %llu words, %s", Panel::left(state, 10).c_str(), (unsigned long long)g_in.samples,
                             (unsigned long long)g_in.words, sizeText(g_in.bytes).c_str());
                    std::string l = buf;
                    snprintf(buf, sizeof(buf), "  lost %llu  waited %llu", (unsigned long long)g_in.lostSamples, (unsigned long long)g_in.waitSamples);
                    l += g_in.lostSamples ? Panel::red(buf) : std::string(buf);
                    if (option.samples > 0)
                        l += "  " + Panel::bar((double)g_in.samples / option.samples, 10);
                    lines.push_back(l);
                    lines.push_back(Panel::dim("       -> " + option.gpio_in_file));
                }
                if (outStarted) {
                    const std::lock_guard b(g_out.m);
                    const std::string state = g_out.lost              ? Panel::red("stopped")
                                              : g_out.fileEnded        ? std::string("playing out")
                                              : g_out.connected        ? Panel::green("playing")
                                                                       : std::string("connecting");
                    snprintf(buf, sizeof(buf), "  out  %s  %llu words, %s, %s/s", Panel::left(state, 10).c_str(), (unsigned long long)g_out.words,
                             sizeText(g_out.bytes).c_str(), sizeText(secs > 0 ? (uint64_t)(g_out.bytes / secs) : 0).c_str());
                    lines.push_back(buf);
                    lines.push_back(Panel::dim("       <- " + option.gpio_out_file));
                }
                lines.push_back("");
                lines.push_back(Panel::dim(std::string("  ") + (panel.keys() ? "[q] stop   " : "") + "[Ctrl+C] stop"));
                panel.draw(lines);
                const int k = panel.key();
                if (k == 'q' || k == 'Q')
                    g_gpioStop = true;
            }
            sleepMs(panel.enabled() ? 250 : 50);
        }
    }
    if (inThread.joinable())
        inThread.join();
    if (outThread.joinable())
        outThread.join();

    // the stops: the input always, the output when the client ended it before the board did
    // the board stops the input in the background (a file is closed first) and answers then: the
    // answer is waited for, a late one would reach the next client as the answer to its start
    if (inStarted && !g_inStopped && !g_inFailed) {
        cl->sendGPIOInServerStop(host);
        const auto end = nowMs() + replyTimeout();
        while (!g_inStopped && nowMs() < end)
            sleepMs(10);
    }
    if (outStarted && !g_outStopped && !g_outFailed) {
        cl->sendGPIOOutServerStop(host);
        const auto end = nowMs() + replyTimeout();
        while (!g_outStopped && nowMs() < end)
            sleepMs(10);
    }
    sleepMs(100);
    const bool connected = cl->isConnected(host);
    cl->removeHadlers();

    bool ok = true;
    if (inStarted) {
        const std::lock_guard a(g_in.m);
        const char* why = g_in.fileError ? "the file could not be written" : g_inFailed ? "the board stopped it with an error" : g_in.lost ? endedText(connected) : nullptr;
        if (why) {
            aprintf(stderr, "%s [ERROR] %s ended the GPIO input early: %s\n", getTS(": ").c_str(), host.c_str(), why);
            ok = false;
        }
        aprintf(stdout, "%s %s: GPIO input %llu samples (%llu words%s), lost %llu, waited %llu; %s\n", why ? "Stopped:" : "Done:", host.c_str(),
                (unsigned long long)g_in.samples, (unsigned long long)g_in.words, g_in.rle ? ", RLE" : "", (unsigned long long)g_in.lostSamples,
                (unsigned long long)g_in.waitSamples, g_in.opened ? option.gpio_in_file.c_str() : "no file");
    }
    if (outStarted) {
        const std::lock_guard b(g_out.m);
        const char* why = g_outFailed ? "the board stopped it with an error" : g_out.lost ? endedText(connected) : nullptr;
        if (why) {
            aprintf(stderr, "%s [ERROR] %s ended the GPIO output early: %s\n", getTS(": ").c_str(), host.c_str(), why);
            ok = false;
        }
        aprintf(stdout, "%s %s: GPIO output %llu words of %s%s; the output waited %llu clocks for data\n", why ? "Stopped:" : "Done:", host.c_str(),
                (unsigned long long)g_out.words, option.gpio_out_file.c_str(), g_out.fileEnded ? " (the whole file)" : "", (unsigned long long)g_outWait.load());
    }
    return ok;
}
