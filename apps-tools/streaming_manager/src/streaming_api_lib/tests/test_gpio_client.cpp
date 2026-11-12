/**
 * GPIOStreamClient in the file mode against a fake board: a ServerNetConfigManager on 127.0.0.1
 * that answers the GPIO starts the way streaming-server does (started SD) and ends the file
 * itself when the test says so. wait() must return once the board ended both directions, also
 * when the stop comes right behind the start answer (a short file, a small sample limit).
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "callbacks.h"
#include "config_net_lib/server_net_config_manager.h"
#include "config_streaming.h"
#include "gpio_streaming.h"
#include "net_lib/asio_common.h"

namespace {

auto WaitFor(const std::function<bool()>& done, std::chrono::milliseconds timeout = std::chrono::milliseconds(3000)) -> bool {
    const auto end = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < end) {
        if (done())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return done();
}

// What the board does after the file mode start answer
enum class End {
    LATER,      // the test ends the file (endInput / endOutput)
    AT_START,   // the stop right behind the start answer: a file that ends at once
};

struct FakeBoard {
    std::unique_ptr<ServerNetConfigManager> config;
    std::atomic<End> inEnd{End::LATER};
    std::atomic<End> outEnd{End::LATER};

    explicit FakeBoard(const std::string& file) {
        config = std::make_unique<ServerNetConfigManager>(file, broadcast_lib::AB_SERVER_MASTER, "127.0.0.1", NET_CONFIG_PORT, 2);
        auto* c = config.get();
        c->startGpioInStreamingNofiy.connect([this, c]() {
            c->sendGPIOInServerStartedSD();
            if (inEnd == End::AT_START)
                c->sendGPIOInServerStoppedSDDone();
        });
        c->startGpioOutStreamingNofiy.connect([this, c]() {
            c->sendGPIOOutServerStartedSD();
            if (outEnd == End::AT_START)
                c->sendGPIOOutServerStoppedSDDone(42);
        });
        c->stopGpioInStreamingNofiy.connect([c]() { c->sendGPIOInServerStopped(); });
        c->stopGpioOutStreamingNofiy.connect([c]() { c->sendGPIOOutServerStopped(0); });
    }

    auto endInput() -> void { config->sendGPIOInServerStoppedSDDone(); }
    auto endOutput() -> void { config->sendGPIOOutServerStoppedSDDone(7); }
};

struct FileCb : public GPIOCallback {
    std::atomic<int> inDone{0};
    std::atomic<int> outDone{0};
    std::atomic<uint64_t> outWait{0};

    void inputFileStopped(GPIOStreamClient*, std::string, GPIOFileStop reason) override {
        if (reason == GPIO_FILE_DONE)
            inDone++;
    }
    void outputFileStopped(GPIOStreamClient*, std::string, GPIOFileStop reason, uint64_t waitClocks) override {
        if (reason == GPIO_FILE_DONE)
            outDone++;
        outWait = waitClocks;
    }
};

class GpioFile : public ::testing::Test {
   protected:
    void SetUp() override {
        m_dir = std::filesystem::temp_directory_path() / "streaming_api_gpio_file";
        std::filesystem::create_directories(m_dir);
        m_board = std::make_unique<FakeBoard>((m_dir / "streaming_config.json").string());
        m_conf = std::make_shared<ConfigStreamClient>();
        ASSERT_TRUE(m_conf->connect({m_host}));
        m_gpio = std::make_unique<GPIOStreamClient>(m_conf);
        m_cb = std::make_shared<FileCb>();
        m_gpio->setCallback(m_cb);
    }

    void TearDown() override {
        m_gpio.reset();
        m_conf.reset();
        m_board.reset();
        std::error_code ec;
        std::filesystem::remove_all(m_dir, ec);
    }

    // wait() in a thread of its own: true when it returned within `ms`; else it is released
    auto waitReturns(int ms) -> bool {
        std::atomic<bool> back{false};
        std::thread t([&] {
            m_gpio->wait();
            back = true;
        });
        const bool ok = WaitFor([&] { return back.load(); }, std::chrono::milliseconds(ms));
        if (!ok)
            m_gpio->notifyStop();
        t.join();
        return ok;
    }

    const std::string m_host = "127.0.0.1";
    std::filesystem::path m_dir;
    std::unique_ptr<FakeBoard> m_board;
    std::shared_ptr<ConfigStreamClient> m_conf;
    std::unique_ptr<GPIOStreamClient> m_gpio;
    std::shared_ptr<FileCb> m_cb;
};

}  // namespace

// The file mode lasts until the board ends it: wait() does not return before
TEST_F(GpioFile, WaitLastsUntilTheBoardEndsBoth) {
    ASSERT_TRUE(m_gpio->startInput(m_host));
    ASSERT_TRUE(m_gpio->startOutput(m_host));
    EXPECT_TRUE(m_gpio->inputToFile());
    EXPECT_TRUE(m_gpio->outputFromFile());
    std::atomic<bool> back{false};
    std::thread t([&] {
        m_gpio->wait();
        back = true;
    });
    EXPECT_FALSE(WaitFor([&] { return back.load(); }, std::chrono::milliseconds(300))) << "nothing ended yet";
    m_board->endOutput();
    EXPECT_TRUE(WaitFor([&] { return m_cb->outDone == 1; }));
    EXPECT_FALSE(WaitFor([&] { return back.load(); }, std::chrono::milliseconds(300))) << "the input still runs";
    EXPECT_EQ(m_gpio->outputWaitClocks(), 7u);
    m_board->endInput();
    EXPECT_TRUE(WaitFor([&] { return back.load(); }));
    EXPECT_EQ(m_cb->inDone, 1);
    if (!back)
        m_gpio->notifyStop();
    t.join();
}

// The stop right behind the start answer: it comes while startOutput() still waits for the
// answer and must not be undone by the end of the start (the lost wake-up of wait()).
TEST_F(GpioFile, AnOutputFileThatEndsAtTheStartEndsWait) {
    m_board->outEnd = End::AT_START;
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(m_gpio->startOutput(m_host));
        ASSERT_TRUE(WaitFor([&] { return m_cb->outDone == i + 1; }));
        EXPECT_EQ(m_gpio->outputWaitClocks(), 42u) << "the reported wait is kept";
        ASSERT_TRUE(waitReturns(3000)) << "run " << i;
    }
}

TEST_F(GpioFile, AnInputFileThatEndsAtTheStartEndsWait) {
    m_board->inEnd = End::AT_START;
    for (int i = 0; i < 10; i++) {
        ASSERT_TRUE(m_gpio->startInput(m_host));
        ASSERT_TRUE(WaitFor([&] { return m_cb->inDone == i + 1; }));
        ASSERT_TRUE(waitReturns(3000)) << "run " << i;
    }
}

// streamapi.cpp part 2: the input ends at its limit, the output with its file
TEST_F(GpioFile, BothEndAtTheStartWaitReturns) {
    m_board->inEnd = End::AT_START;
    m_board->outEnd = End::AT_START;
    ASSERT_TRUE(m_gpio->startInput(m_host));
    ASSERT_TRUE(m_gpio->startOutput(m_host));
    ASSERT_TRUE(WaitFor([&] { return m_cb->inDone == 1 && m_cb->outDone == 1; }));
    EXPECT_TRUE(waitReturns(3000));
}

// A stop of the client after a file that ended by itself: the next start runs again
TEST_F(GpioFile, AStartAfterAnEndedFileRunsAgain) {
    m_board->outEnd = End::AT_START;
    ASSERT_TRUE(m_gpio->startOutput(m_host));
    ASSERT_TRUE(waitReturns(3000));
    m_board->outEnd = End::LATER;
    ASSERT_TRUE(m_gpio->startOutput(m_host));
    EXPECT_FALSE(waitReturns(300)) << "the new file plays";
    m_board->endOutput();
    EXPECT_TRUE(waitReturns(3000));
}
