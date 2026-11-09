// GPIO example 2: the GPIO input into a CSV file on this computer, a row per run of a level:
//   lane,start,length,level      lane P or N, start and length in samples, level in hex
// With RLE the P and N lanes count their runs on their own: each lane has its own time line.
//
// Usage: gpio_example_2 [IP] [FILE] [SECONDS]   (defaults: the first board found, gpio.csv, 5)

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

#include "callbacks.h"
#include "config_streaming.h"
#include "gpio_streaming.h"

static std::atomic<bool> g_stop(false);

// One lane: the runs of its level, a row each when the level changes
struct Lane {
    const char* name;
    uint64_t position = 0;  // the next sample
    uint64_t start = 0;     // of the current run
    int level = -1;

    void take(FILE* f, int value, uint64_t samples) {
        if (value != level) {
            if (level >= 0)
                fprintf(f, "%s,%llu,%llu,0x%02X\n", name, (unsigned long long)start, (unsigned long long)(position - start), level);
            level = value;
            start = position;
        }
        position += samples;
    }
    void gap(uint64_t samples) { position += samples; }  // lost or waited for a trigger
    // the run still open at the end
    void finish(FILE* f) {
        if (level >= 0 && position > start)
            fprintf(f, "%s,%llu,%llu,0x%02X\n", name, (unsigned long long)start, (unsigned long long)(position - start), level);
        level = -1;
    }
};

class CsvWriter : public GPIOCallback {
   public:
    FILE* f;
    std::mutex m;
    Lane p{"P"}, n{"N"};
    uint64_t lost = 0;

    explicit CsvWriter(FILE* file) : f(file) { fprintf(f, "lane,start,length,level\n"); }

    void receivePack(GPIOStreamClient*, GPIOPack& pack) override {
        std::lock_guard<std::mutex> lock(m);
        p.gap(pack.fpgaLost + pack.fpgaWait);
        n.gap(pack.fpgaLost + pack.fpgaWait);
        lost += pack.fpgaLost;
        for (uint32_t w : pack.words) {
            // {N count, N lines, P count, P lines}; without RLE a word is one sample
            p.take(f, w & 0xFF, pack.rle ? ((w >> 8) & 0xFF) + 1 : 1);
            n.take(f, (w >> 16) & 0xFF, pack.rle ? ((w >> 24) & 0xFF) + 1 : 1);
        }
    }
    void error(GPIOStreamClient*, std::string host, int code) override { std::cout << "GPIO error " << host << " code " << code << std::endl; }
};

int main(int argc, char* argv[]) {
    std::signal(SIGINT, [](int) { g_stop = true; });
    const std::string file = argc > 2 ? argv[2] : "gpio.csv";
    const int seconds = argc > 3 ? std::stoi(argv[3]) : 5;

    auto conf = std::make_shared<ConfigStreamClient>();
    if (!(argc > 1 ? conf->connect({argv[1]}) : conf->connect())) {
        std::cerr << "No board found" << std::endl;
        return 1;
    }
    const std::string host = conf->getHosts().front();
    conf->sendConfig(host, "gpio_decimation", "1");
    conf->sendConfig(host, "gpio_rle", "ON");  // runs of a level in one word: far less data
    conf->sendConfig(host, "gpio_in_pass_mode", "NET");

    FILE* f = fopen(file.c_str(), "w");
    if (!f) {
        std::cerr << "Can't write " << file << std::endl;
        return 1;
    }
    auto gpio = std::make_shared<GPIOStreamClient>(conf);
    auto csv = std::make_shared<CsvWriter>(f);
    gpio->setCallback(csv);
    if (!gpio->startInput(host)) {
        std::cerr << "The GPIO input did not start" << std::endl;
        fclose(f);
        return 1;
    }
    std::cout << "Recording the GPIO input of " << host << " into " << file << " for " << seconds << " s" << std::endl;
    for (int i = 0; i < seconds * 10 && !g_stop; i++)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    gpio->stopInput();

    std::lock_guard<std::mutex> lock(csv->m);
    csv->p.finish(f);
    csv->n.finish(f);
    fclose(f);
    std::cout << "Done: " << csv->p.position << " samples, " << csv->lost << " lost" << std::endl;
    return 0;
}
