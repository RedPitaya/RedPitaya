// GPIO example 1: the output plays a Gray coded counter on DIO0..7_P and DIO0..6_N while the
// input reads the lines back (an output line reads its own level, so no wires are needed).
//
// The output words are {P lines, N lines, unused, RLE count}: with RLE a word holds its level for
// count + 1 samples. The input words are {N count, N lines, P count, P lines}.
//
// Usage: gpio_example_1 [IP]    (without IP the first board found on the network)

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "callbacks.h"
#include "config_streaming.h"
#include "gpio_streaming.h"

static std::atomic<bool> g_stop(false);

static uint32_t gray(uint32_t k) {
    return k ^ (k >> 1);
}

class GpioHandler : public GPIOCallback {
   public:
    std::mutex m;
    uint64_t packs = 0, samples = 0, words = 0, lost = 0, changes = 0;
    int last = -1;  // the P lines of the last word

    void receivePack(GPIOStreamClient*, GPIOPack& pack) override {
        std::lock_guard<std::mutex> lock(m);
        packs++;
        lost += pack.fpgaLost;
        for (uint32_t w : pack.words) {
            const int p = w & 0xFF;                                   // the P lines
            samples += pack.rle ? ((w >> 8) & 0xFF) + 1 : 1;          // the samples of the P lane
            if (p != last)
                changes++;
            last = p;
            words++;
        }
    }
    void connected(GPIOStreamClient*, std::string host) override { std::cout << "GPIO connected to " << host << std::endl; }
    void error(GPIOStreamClient*, std::string host, int code) override { std::cout << "GPIO error " << host << " code " << code << std::endl; }
    void outputStopped(GPIOStreamClient*, std::string host, uint64_t waitClocks) override {
        std::cout << "GPIO output of " << host << " stopped, it waited " << waitClocks << " clocks for data" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::signal(SIGINT, [](int) { g_stop = true; });

    auto conf = std::make_shared<ConfigStreamClient>();
    auto gpio = std::make_shared<GPIOStreamClient>(conf);
    auto handler = std::make_shared<GpioHandler>();
    gpio->setCallback(handler);

    if (!(argc > 1 ? conf->connect({argv[1]}) : conf->connect())) {
        std::cerr << "No board found" << std::endl;
        return 1;
    }
    const std::string host = conf->getHosts().front();

    // the lines, RLE and the network mode of both directions
    conf->sendConfig(host, "gpio_outputs_p", "255");
    conf->sendConfig(host, "gpio_outputs_n", "127");
    conf->sendConfig(host, "gpio_decimation", "1");
    conf->sendConfig(host, "gpio_rle", "ON");
    conf->sendConfig(host, "gpio_in_pass_mode", "NET");
    conf->sendConfig(host, "gpio_out_pass_mode", "NET");

    // 1000 values of the counter, each for 256 samples (RLE count 255), repeated until the stop
    std::vector<uint32_t> words;
    for (uint32_t k = 0; k < 1000; k++) {
        const uint32_t g = gray(k);
        words.push_back((g & 0xFF) << 24 | ((g >> 8) & 0x7F) << 16 | 0xFF);
    }
    gpio->setOutputData(words, 0);

    if (!gpio->startInput(host) || !gpio->startOutput(host)) {
        std::cerr << "GPIO streaming did not start (is there a GPIO block in the FPGA?)" << std::endl;
        gpio->stopStreaming();
        return 1;
    }
    std::cout << "GPIO streaming for 5 s, Ctrl+C stops it" << std::endl;
    for (int i = 0; i < 50 && !g_stop; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    gpio->stopStreaming();
    std::lock_guard<std::mutex> lock(handler->m);
    std::cout << "Input: " << handler->packs << " buffers, " << handler->words << " words, " << handler->samples << " samples, " << handler->changes
              << " changes of the P lines, " << handler->lost << " samples lost" << std::endl;
    std::cout << "Output: waited " << gpio->outputWaitClocks() << " clocks for data" << std::endl;
    return 0;
}
