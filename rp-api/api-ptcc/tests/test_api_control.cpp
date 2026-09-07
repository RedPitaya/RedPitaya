/**
 * Control commands and the failure paths. The emulator can be told to answer
 * with a broken CRC or to stay silent, which is the only practical way to
 * exercise the timeout and rejected-frame handling without hardware.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>

#include "rp_ptcc.h"
#include "test_support.h"

namespace {

class PtccControl : public ::testing::Test {
   protected:
    void SetUp() override {
        m_device = std::make_unique<EmulatedDevice>();
        ASSERT_FALSE(m_device->port().empty());
        ASSERT_EQ(rp_PtccInitDevice(m_device->port().c_str()), RP_PTCC_OK);
        rp_PtccSetThrottle(0);
    }

    void TearDown() override {
        rp_PtccRelease();
        m_device.reset();
    }

    std::unique_ptr<EmulatedDevice> m_device;
};

}  // namespace

TEST_F(PtccControl, SetpointInsideTheRangeIsAccepted) {
    EXPECT_EQ(rp_PtccSetSetpoint(230.0F), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetSetpoint(RP_PTCC_MIN_TEMPERATURE), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetSetpoint(RP_PTCC_MAX_TEMPERATURE), RP_PTCC_OK);
}

TEST_F(PtccControl, SetpointOutsideTheRangeNeverReachesTheDevice) {
    EXPECT_EQ(rp_PtccSetSetpoint(99.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSetpoint(401.0F), RP_PTCC_ERANGE);

    // The link is still usable afterwards.
    EXPECT_EQ(rp_PtccSetSetpoint(250.0F), RP_PTCC_OK);
}

TEST_F(PtccControl, CurrentLimitIsAccepted) {
    EXPECT_EQ(rp_PtccSetMaxCurrent(1.5F), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetMaxCurrent(0.0F), RP_PTCC_OK);
}

TEST_F(PtccControl, CoolerAcceptsEveryMode) {
    EXPECT_EQ(rp_PtccSetCooler(RP_PTCC_CTRL_ON), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetCooler(RP_PTCC_CTRL_OFF), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetCooler(RP_PTCC_CTRL_AUTO), RP_PTCC_OK);
}

TEST_F(PtccControl, FanAcceptsEveryMode) {
    EXPECT_EQ(rp_PtccSetFan(RP_PTCC_CTRL_ON), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetFan(RP_PTCC_CTRL_OFF), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetFan(RP_PTCC_CTRL_AUTO), RP_PTCC_OK);
}

TEST_F(PtccControl, InvalidControlModeIsRejected) {
    EXPECT_NE(rp_PtccSetCooler(static_cast<rp_ptcc_ctrl_t>(7)), RP_PTCC_OK);
    EXPECT_NE(rp_PtccSetFan(static_cast<rp_ptcc_ctrl_t>(7)), RP_PTCC_OK);
}

TEST_F(PtccControl, ThrottleAndTimeoutAreAccepted) {
    EXPECT_EQ(rp_PtccSetTimeout(1500), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetThrottle(RP_PTCC_THROTTLE_MS), RP_PTCC_OK);
    // Below the firmware limit is allowed but warned about.
    EXPECT_EQ(rp_PtccSetThrottle(0), RP_PTCC_OK);
}

// The throttle is what keeps the firmware from dropping commands, so it must
// actually delay the next write rather than only being stored.
TEST_F(PtccControl, ThrottleDelaysConsecutiveCommands) {
    ASSERT_EQ(rp_PtccSetThrottle(300), RP_PTCC_OK);

    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));
    ASSERT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_OK);

    const auto started = std::chrono::steady_clock::now();
    ASSERT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_OK);
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - started)
                             .count();

    EXPECT_GE(elapsed, 250);
}

TEST(PtccFailure, SilentDeviceIsNotDetected) {
    EmulatedDevice device("--silent");
    ASSERT_FALSE(device.port().empty());

    EXPECT_NE(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);
    rp_PtccRelease();
}

TEST(PtccFailure, CorruptedFramesAreRejectedAndNotDetected) {
    EmulatedDevice device("--corrupt-crc");
    ASSERT_FALSE(device.port().empty());

    // Detection needs one valid frame; a bad CRC must not be accepted as one.
    EXPECT_NE(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);
    rp_PtccRelease();
}

TEST(PtccFailure, TimeoutIsReportedAndTheLinkRecovers) {
    EmulatedDevice device;
    ASSERT_FALSE(device.port().empty());
    ASSERT_EQ(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);
    rp_PtccSetThrottle(0);

    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));
    ASSERT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_OK);

    // A timeout that cannot cover a round trip must surface as ETIMEOUT
    // rather than blocking or returning a partially decoded frame.
    ASSERT_EQ(rp_PtccSetTimeout(0), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_ETIMEOUT);

    uint64_t errors = 0;
    ASSERT_EQ(rp_PtccGetErrorCount(&errors), RP_PTCC_OK);
    EXPECT_GT(errors, 0u);

    // Restoring the timeout brings the link back without reopening the port.
    ASSERT_EQ(rp_PtccSetTimeout(RP_PTCC_TIMEOUT_MS), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_OK);

    rp_PtccRelease();
}

TEST(PtccFailure, NomemModuleIsDetected) {
    EmulatedDevice device("--module NOMEM");
    ASSERT_FALSE(device.port().empty());
    ASSERT_EQ(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);

    rp_ptcc_module_t module = RP_PTCC_MODULE_NONE;
    ASSERT_EQ(rp_PtccGetModuleType(&module), RP_PTCC_OK);
    EXPECT_EQ(module, RP_PTCC_MODULE_NOMEM);

    rp_PtccRelease();
}

// LAB_M has no basic temperature setpoint; upstream raises instead of
// silently writing to the wrong register.
TEST(PtccFailure, LabMModuleRejectsTheTemperatureSetpoint) {
    EmulatedDevice device("--module LAB_M");
    ASSERT_FALSE(device.port().empty());
    ASSERT_EQ(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);
    rp_PtccSetThrottle(0);

    rp_ptcc_module_t module = RP_PTCC_MODULE_NONE;
    ASSERT_EQ(rp_PtccGetModuleType(&module), RP_PTCC_OK);
    ASSERT_EQ(module, RP_PTCC_MODULE_LAB_M);

    EXPECT_EQ(rp_PtccSetSetpoint(230.0F), RP_PTCC_ENOTSUP);

    rp_PtccRelease();
}
