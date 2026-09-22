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

TEST_F(PtccControl, SetpointIsAccepted) {
    EXPECT_EQ(rp_PtccSetSetpoint(230), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetSetpoint(200), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetSetpoint(280), RP_PTCC_OK);
}

// The protocol carries the setpoint with three decimals, so a tenth of a
// Kelvin has to survive the round trip. The emulator answers with whatever the
// driver wrote, which is what makes the read back meaningful.
TEST_F(PtccControl, FractionalSetpointSurvivesTheRoundTrip) {
    float value = 0.0F;

    ASSERT_EQ(rp_PtccSetSetpoint(200.1F), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccGetSetpoint(&value), RP_PTCC_OK);
    EXPECT_NEAR(value, 200.1F, 1e-3F);

    ASSERT_EQ(rp_PtccSetSetpoint(213.75F), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccGetSetpoint(&value), RP_PTCC_OK);
    EXPECT_NEAR(value, 213.75F, 1e-3F);

    ASSERT_EQ(rp_PtccSetSetpoint(230.0F), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccGetSetpoint(&value), RP_PTCC_OK);
    EXPECT_NEAR(value, 230.0F, 1e-3F);
}

// Vigo confirmed the firmware clamps a value outside the module's own
// USER_MIN/USER_MAX without reporting anything, so a write that would be
// clamped is refused before it reaches the EEPROM. 350 K is inside the
// protocol range of 100 to 400 K and outside the emulated module's 180 to 300.
TEST_F(PtccControl, SetpointOutsideTheModuleRangeIsRejected) {
    EXPECT_EQ(rp_PtccSetSetpoint(350.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSetpoint(150.0F), RP_PTCC_ERANGE);

    // The limits themselves stay writable.
    EXPECT_EQ(rp_PtccSetSetpoint(300.0F), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetSetpoint(180.0F), RP_PTCC_OK);
}

// The rejection comes from the upstream value tables, not from a copy of them
// in the C layer, and it arrives as ERANGE rather than a decode failure.
TEST_F(PtccControl, OutOfRangeSetpointIsRejectedByTheUpstreamLibrary) {
    EXPECT_EQ(rp_PtccSetSetpoint(50), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSetpoint(500), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSetpoint(-273), RP_PTCC_ERANGE);

    EXPECT_EQ(rp_PtccSetSetpoint(230), RP_PTCC_OK);
}

TEST_F(PtccControl, OutOfRangeCurrentLimitIsRejectedByTheUpstreamLibrary) {
    EXPECT_EQ(rp_PtccSetMaxCurrent(-1.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetMaxCurrent(100.0F), RP_PTCC_ERANGE);
}

// The supply rails and the control mode travel in one protocol message, so
// they are checked together, reading back what the emulator stored.
TEST_F(PtccControl, SupplyIsWrittenAsOneSetting) {
    ASSERT_EQ(rp_PtccSetSupply(RP_PTCC_CTRL_ON, 12.0F, -12.0F), RP_PTCC_OK);

    rp_ptcc_params_t params;
    std::memset(&params, 0, sizeof(params));
    ASSERT_EQ(rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_OK);
    EXPECT_NEAR(params.u_sup_plus, 12.0F, 1e-3F);
    EXPECT_NEAR(params.u_sup_minus, -12.0F, 1e-3F);
    EXPECT_EQ(params.supply_ctrl, RP_PTCC_CTRL_ON);
}

TEST_F(PtccControl, OutOfRangeSupplyIsRejectedByTheUpstreamLibrary) {
    EXPECT_EQ(rp_PtccSetSupply(RP_PTCC_CTRL_ON, 20.0F, -12.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSupply(RP_PTCC_CTRL_ON, 12.0F, 12.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSupply(RP_PTCC_CTRL_AUTO, 5.0F, -5.0F), RP_PTCC_OK);
}

TEST_F(PtccControl, PwmIsWrittenThroughTheGenericParameterMessage) {
    ASSERT_EQ(rp_PtccSetPwm(12800), RP_PTCC_OK);

    rp_ptcc_params_t params;
    std::memset(&params, 0, sizeof(params));
    ASSERT_EQ(rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_OK);
    EXPECT_EQ(params.pwm, 12800U);

    EXPECT_EQ(rp_PtccSetPwm(100000), RP_PTCC_ERANGE);
}

TEST_F(PtccControl, FanModeIsStoredByTheModule) {
    ASSERT_EQ(rp_PtccSetFan(RP_PTCC_CTRL_ON), RP_PTCC_OK);

    rp_ptcc_params_t params;
    std::memset(&params, 0, sizeof(params));
    ASSERT_EQ(rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_OK);
    EXPECT_EQ(params.fan_ctrl, RP_PTCC_CTRL_ON);
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

// A field the device did not send must be reported, never substituted with a
// zero that would be indistinguishable from a real measurement.
TEST(PtccFailure, MissingMonitorFieldIsReportedNotZeroed) {
    EmulatedDevice device("--drop-field");
    ASSERT_FALSE(device.port().empty());
    ASSERT_EQ(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);
    rp_PtccSetThrottle(0);

    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));
    EXPECT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_ERESP);
    EXPECT_FALSE(monitor.valid);

    const std::string detail = rp_PtccGetLastPythonError();
    EXPECT_NE(detail.find("I_TEC"), std::string::npos) << "detail: " << detail;

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

// PtccLabMDevice derives from PtccMemDevice and inherits the temperature
// setpoint, so the controller panel works with a LAB_M module just like with
// any other. A guard here would break exactly what Smart Manager can do.
TEST(PtccFailure, LabMModuleAcceptsTheTemperatureSetpoint) {
    EmulatedDevice device("--module LAB_M");
    ASSERT_FALSE(device.port().empty());
    ASSERT_EQ(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);
    rp_PtccSetThrottle(0);

    rp_ptcc_module_t module = RP_PTCC_MODULE_NONE;
    ASSERT_EQ(rp_PtccGetModuleType(&module), RP_PTCC_OK);
    ASSERT_EQ(module, RP_PTCC_MODULE_LAB_M);

    EXPECT_EQ(rp_PtccSetSetpoint(230), RP_PTCC_OK);

    rp_PtccRelease();
}
