/**
 * StreamSession on the dummy hardware of the host build, driven by a config client over TCP the
 * way rpsa_client and the client API drive streaming-server: what a change of the settings
 * stops, what it leaves running, what the memory manager is given, what the client hears.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "asio.hpp"
#include "board_lib/board.h"
#include "data_lib/network_header.h"
#include "session_lib/stream_session.h"
#include "session_lib/trigger_commands.h"
#include "uio_lib/gpio.h"
#include "uio_lib/gpio_dummy.h"
#include "uio_lib/memory_manager.h"
#include "uio_lib/oscilloscope_dummy.h"

using namespace session_lib;
using Cmd = CNetConfigManager::ECommands;
using streaming_lib::ADCStopReason;
using dac_streaming_lib::DACStopReason;
using gpio_streaming_lib::GPIOStopReason;

namespace {

auto WaitFor(const std::function<bool()>& done, std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)) -> bool {
    const auto end = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < end) {
        if (done())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return done();
}

auto NextPort() -> uint16_t {
    static std::atomic<uint16_t> port{29600};
    return port++;
}

auto Words(std::initializer_list<uint32_t> words) -> std::string {
    std::string s;
    for (uint32_t w : words)
        s.append(reinterpret_cast<const char*>(&w), sizeof(w));
    return s;
}

auto KeyValue(const std::string& key, const std::string& value) -> std::string {
    return Words({0xABCDEF, uint32_t(16 + key.size() + value.size()), uint32_t(key.size()), uint32_t(value.size())}) + key + value;
}

auto Command(uint32_t command, const std::string& tag = "") -> std::string {
    return Words({0xABCDED, uint32_t(16 + tag.size()), command, uint32_t(tag.size())}) + tag;
}

// A config client that writes the frames of the protocol and collects the commands it gets.
class Peer {
   public:
    ~Peer() { close(); }

    auto connect(uint16_t port) -> bool {
        asio::error_code ec;
        m_socket.connect(asio::ip::tcp::endpoint(asio::ip::make_address("127.0.0.1"), port), ec);
        if (ec)
            return false;
        m_reader = std::thread([this]() { read(); });
        return true;
    }

    auto send(const std::string& bytes) -> void { asio::write(m_socket, asio::buffer(bytes)); }

    auto count(uint32_t command) -> int {
        std::lock_guard lock(m_mtx);
        int n = 0;
        for (auto c : m_commands)
            n += c == command;
        return n;
    }

    auto waitFor(uint32_t command, int times = 1) -> bool {
        return WaitFor([&] { return count(command) >= times; });
    }

    auto close() -> void {
        asio::error_code ec;
        m_socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
        if (m_reader.joinable())
            m_reader.join();
    }

   private:
    auto read() -> void {
        std::string buf;
        char chunk[4096];
        for (;;) {
            asio::error_code ec;
            size_t n = m_socket.read_some(asio::buffer(chunk), ec);
            if (ec)
                return;
            buf.append(chunk, n);
            while (buf.size() >= 8) {
                uint32_t header, size;
                memcpy(&header, buf.data(), 4);
                memcpy(&size, buf.data() + 4, 4);
                if (size < 8 || buf.size() < size)
                    break;
                if (header == 0xABCDED) {
                    uint32_t command;
                    memcpy(&command, buf.data() + 8, 4);
                    std::lock_guard lock(m_mtx);
                    m_commands.push_back(command);
                }
                buf.erase(0, size);
            }
        }
    }

    asio::io_context m_io;
    asio::ip::tcp::socket m_socket{m_io};
    std::thread m_reader;
    std::mutex m_mtx;
    std::vector<uint32_t> m_commands;
};

class Events : public SessionCallback {
   public:
    void settingsChanged(const CStreamSettings&, const CStreamSettings&) override { changes++; }
    void adcStarted(bool) override { adcStarts++; }
    void adcStopped(ADCStopReason r) override { add(adcStops, r); }
    void dacStarted(bool) override { dacStarts++; }
    void dacStopped(DACStopReason r) override { add(dacStops, r); }
    void gpioInStarted() override { gpioInStarts++; }
    void gpioInStopped(GPIOStopReason r) override { add(gpioInStops, r); }

    template <typename T>
    auto stops(const std::vector<T>& v) -> std::vector<T> {
        std::lock_guard lock(mtx);
        return v;
    }

    std::atomic<int> changes{0}, adcStarts{0}, dacStarts{0}, gpioInStarts{0};
    std::mutex mtx;
    std::vector<ADCStopReason> adcStops;
    std::vector<DACStopReason> dacStops;
    std::vector<GPIOStopReason> gpioInStops;

   private:
    template <typename T>
    void add(std::vector<T>& v, T r) {
        std::lock_guard lock(mtx);
        v.push_back(r);
    }
};

class Session : public ::testing::Test {
   protected:
    void SetUp() override {
        uio_lib::dummy::resetOscState();
        uio_lib::dummy::resetGpioState();
        uio_lib::dummy::gpioState().present = true;
        uio_lib::dummy::gpioState().features = uio_lib::GPIO_FEATURE_OUT_FREE | uio_lib::GPIO_FEATURE_OUT_WAIT;
        m_file = (std::filesystem::temp_directory_path() / ("stream_session_" + std::to_string(m_port) + ".json")).string();
        std::filesystem::remove(m_file);
        hw = uio_lib::CHardware::create(std::make_shared<board_lib::CDummyBoard>());
        SessionOptions o;
        o.configFile = m_file;
        o.configPort = m_port;
        o.broadcast = false;
        session = StreamSession::create(hw, o);
        events = std::make_shared<Events>();
        session->setCallback(events);
        ASSERT_TRUE(peer.connect(m_port));
        ASSERT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_MASTER_CONNECTED));
    }

    void TearDown() override {
        peer.close();
        session = nullptr;
        std::filesystem::remove(m_file);
    }

    // a key the server must take
    void set(const std::string& key, const std::string& value) {
        const int before = peer.count(Cmd::CS_RESPONSE_SETTINGS_GET_SUCCESS);
        peer.send(KeyValue(key, value));
        ASSERT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_SETTINGS_GET_SUCCESS, before + 1)) << key << "=" << value;
    }

    // the ADC over the network on channel 1, the DAC over the network on channel 1
    void startADC() {
        set("adc_pass_mode", "NET");
        set("channel_state_1", "ON");
        peer.send(Command(Cmd::CS_REQUEST_ADC_SERVER_START));
        ASSERT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_SERVER_STARTED_TCP));
    }

    void startDAC() {
        set("dac_pass_mode", "DAC_NET");
        peer.send(Command(Cmd::CS_REQUEST_DAC_SERVER_START, "1"));
        ASSERT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_DAC_SERVER_STARTED));
    }

    void startGPIOIn() {
        peer.send(Command(Cmd::CS_REQUEST_GPIO_IN_SERVER_START));
        ASSERT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_GPIO_IN_SERVER_STARTED));
    }

    const uint16_t m_port = NextPort();
    std::string m_file;
    uio_lib::CHardware::Ptr hw;
    StreamSession::Ptr session;
    std::shared_ptr<Events> events;
    Peer peer;
};

}  // namespace

// adc_dac_example_1: one config connection, the ADC runs, then the DAC is set up - with the same
// block size again - and started. The ADC goes on.
TEST_F(Session, TheDACStartsBesideTheRunningADCWithTheSameBlockSizeSentAgain) {
    set("block_size", "65536");
    set("adc_size", "1638400");
    startADC();
    set("block_size", "65536");
    set("dac_size", "1638400");
    set("dac_rate", "1953125");
    startDAC();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(events->stops(events->adcStops).empty()) << "the ADC is not stopped";
    EXPECT_EQ(peer.count(Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY), 0);
    EXPECT_EQ(uio_lib::CMemoryManager::instance()->getReserverdMemory(uio_lib::MM_DAC), 1638400u);
}

TEST_F(Session, AWholeConfigThatKeepsTheBlockSizeStopsNothing) {
    startADC();
    auto s = session->settings();
    s.setADCDecimation(64);
    s.setDACSize(s.getDACSize() * 2);
    peer.send(Words({0xABCDEB, uint32_t(12 + s.toJson().size()), uint32_t(s.toJson().size())}) + s.toJson());
    ASSERT_TRUE(WaitFor([&] { return session->settings().getADCDecimation() == 64u; }));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(events->stops(events->adcStops).empty());
}

TEST_F(Session, AStreamMemorySizeTakesEffectWithoutAStop) {
    startADC();
    set("adc_size", "3276800");
    set("gpio_in_size", "2000000");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(events->stops(events->adcStops).empty());
    auto mm = uio_lib::CMemoryManager::instance();
    EXPECT_EQ(mm->getReserverdMemory(uio_lib::MM_ADC), 3276800u);
    EXPECT_EQ(mm->getReserverdMemory(uio_lib::MM_GPIO_IN), 2000000u);
}

TEST_F(Session, ANewBlockSizeStopsEveryStreamAndSplitsTheMemoryAgain) {
    startADC();
    startDAC();
    startGPIOIn();
    set("block_size", "32768");
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_DAC_SERVER_STOPPED_MEM_MODIFY));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_GPIO_IN_SERVER_STOPPED));
    EXPECT_EQ(events->stops(events->adcStops), std::vector<ADCStopReason>{ADCStopReason::MEM_MODIFY});
    EXPECT_EQ(events->stops(events->dacStops), std::vector<DACStopReason>{DACStopReason::MEM_MODIFY});
    EXPECT_EQ(events->stops(events->gpioInStops), std::vector<GPIOStopReason>{GPIOStopReason::MEM_MODIFY});
    EXPECT_EQ(uio_lib::CMemoryManager::instance()->getMemoryBlockSize(), 32768 + DataLib::sizeHeader());

    // and the streams start again on the new blocks
    peer.send(Command(Cmd::CS_REQUEST_ADC_SERVER_START));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_SERVER_STARTED_TCP, 2));
}

TEST_F(Session, ABlockSizeTheRAMDoesNotHoldIsRefusedAndStopsNothing) {
    startADC();
    const auto before = session->settings().getMemoryBlockSize();
    auto mm = uio_lib::CMemoryManager::instance();
    // a multiple of 128 the settings take, 12 of which do not fit the RAM
    const uint32_t tooBig = (mm->getMaxBlockSize() / 128 + 1) * 128;
    ASSERT_LE(tooBig, CStreamSettings::MEMORY_BLOCK_MAX) << "the host RAM of the test is too large for this check";
    peer.send(KeyValue("block_size", std::to_string(tooBig)));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_SETTINGS_GET_FAIL));
    EXPECT_EQ(session->settings().getMemoryBlockSize(), before);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(events->stops(events->adcStops).empty());
}

TEST_F(Session, TheApplicationsChangeOfTheBlockSizeStopsTheStreamsToo) {
    startADC();
    ASSERT_TRUE(session->updateSettings([](CStreamSettings& s) { return s.setMemoryBlockSize(16384); }));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY));
    EXPECT_EQ(uio_lib::CMemoryManager::instance()->getMemoryBlockSize(), 16384 + DataLib::sizeHeader());
    EXPECT_FALSE(session->updateSettings([](CStreamSettings& s) { return s.setMemoryBlockSize(100); }));
    EXPECT_EQ(session->settings().getMemoryBlockSize(), 16384u);
}

TEST_F(Session, LoadingTheFileAppliesItsBlockSize) {
    CStreamSettings onDisk;
    onDisk.setMemoryBlockSize(8192);
    ASSERT_TRUE(onDisk.writeToFile(m_file));
    startADC();
    peer.send(Command(Cmd::CS_REQUEST_LOAD_SETTING_FROM_FILE));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_LOAD_SETTING_FROM_FILE_SUCCES));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_SERVER_STOPPED_MEM_MODIFY));
    EXPECT_EQ(uio_lib::CMemoryManager::instance()->getMemoryBlockSize(), 8192 + DataLib::sizeHeader());
}

TEST_F(Session, TheStreamsStopOnTheirOwn) {
    startADC();
    startDAC();
    peer.send(Command(Cmd::CS_REQUEST_DAC_SERVER_STOP));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_DAC_SERVER_STOPPED));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(events->stops(events->adcStops).empty()) << "the ADC goes on";
    peer.send(Command(Cmd::CS_REQUEST_ADC_SERVER_STOP));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_SERVER_STOPPED));
}

TEST_F(Session, TheSecondStartCommandOfTheADCIsAnswered) {
    startADC();
    peer.send(Command(Cmd::CS_REQUEST_ADC_FPGA_START));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_FPGA_START_ADC_DONE));
}

TEST_F(Session, StoppingTheSessionStopsTheStreamsAndTellsTheClient) {
    startADC();
    startGPIOIn();
    session->stop();
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_ADC_SERVER_STOPPED));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_GPIO_IN_SERVER_STOPPED));
    EXPECT_EQ(events->stops(events->adcStops), std::vector<ADCStopReason>{ADCStopReason::NORMAL});
    session->stop();  // once
}

TEST_F(Session, TheTriggerStatusComesFromTheHardware) {
    auto& st = uio_lib::dummy::oscState();
    st.armed = true;
    st.triggerCount = []() -> uint32_t { return 5; };
    auto status = triggerStatusOf(*hw);
    EXPECT_TRUE(status.adcArmed);
    EXPECT_EQ(status.adcTriggerCount, 5u);
}

TEST_F(Session, TheTriggerStatusIsAnswered) {
    peer.send(Command(Cmd::CS_REQUEST_TRIGGER_STATUS));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_TRIGGER_STATUS));
}

// The GPIO file mode over the protocol: the input writes its file up to the sample limit
TEST_F(Session, TheGpioInputWritesAFileAndSaysSo) {
    const auto dir = std::filesystem::temp_directory_path() / ("session_gpio_" + std::to_string(m_port));
    std::filesystem::create_directories(dir);
    CStreamSettings::setGPIODirPath(dir.string());
    uio_lib::dummy::gpioState().inFill = [](uint8_t, uint32_t) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return true;
    };
    set("gpio_in_pass_mode", "FILE");
    set("gpio_in_format_sd", "VCD");
    set("gpio_in_samples_limit_sd", "1");
    peer.send(Command(Cmd::CS_REQUEST_GPIO_IN_SERVER_START));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_GPIO_IN_SERVER_STARTED_SD));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_GPIO_IN_SERVER_STOPPED_SD_DONE));
    bool vcd = false;
    for (const auto& e : std::filesystem::directory_iterator(dir))
        vcd |= e.path().extension() == ".vcd";
    EXPECT_TRUE(vcd);
    std::filesystem::remove_all(dir);
}

TEST_F(Session, TheGpioOutputWithoutItsFileSaysWhy) {
    set("gpio_out_pass_mode", "FILE");
    set("gpio_out_file_sd", "none.bin");
    peer.send(Command(Cmd::CS_REQUEST_GPIO_OUT_SERVER_START));
    EXPECT_TRUE(peer.waitFor(Cmd::CS_RESPONSE_GPIO_OUT_SERVER_STOPPED_SD_MISSING));
}
