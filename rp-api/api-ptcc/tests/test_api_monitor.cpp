/**
 * Reads against the PTY emulator. The fixture encodes its answers with the
 * upstream Python library, so these tests cover the whole path: C API ->
 * bridge -> embedded interpreter -> ptcc_library decoder -> C structs,
 * including the SI scaling that turns raw counts into Kelvin and Amperes.
 */

#include <gtest/gtest.h>

#include <cstring>
#include <thread>

#include "rp_ptcc.h"
#include "test_support.h"

namespace {

class PtccDevice : public ::testing::Test {
   protected:
    void SetUp() override {
        m_device = std::make_unique<EmulatedDevice>();
        ASSERT_FALSE(m_device->port().empty());
        ASSERT_EQ(rp_PtccInitDevice(m_device->port().c_str()), RP_PTCC_OK);
        // The firmware throttle would make every test wait 0.55 s per command.
        rp_PtccSetThrottle(0);
    }

    void TearDown() override {
        rp_PtccRelease();
        m_device.reset();
    }

    std::unique_ptr<EmulatedDevice> m_device;
};

}  // namespace

TEST_F(PtccDevice, InitReportsTheOpenPortAndDetectedModule) {
    bool connected = false;
    ASSERT_EQ(rp_PtccIsConnected(&connected), RP_PTCC_OK);
    EXPECT_TRUE(connected);

    char path[256] = {0};
    ASSERT_EQ(rp_PtccGetDevicePath(path, sizeof(path)), RP_PTCC_OK);
    EXPECT_EQ(std::string(path), m_device->port());

    rp_ptcc_module_t module = RP_PTCC_MODULE_NONE;
    ASSERT_EQ(rp_PtccGetModuleType(&module), RP_PTCC_OK);
    EXPECT_EQ(module, RP_PTCC_MODULE_MEM);
}

TEST_F(PtccDevice, MonitorValuesArriveInSiUnits) {
    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));

    ASSERT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_OK);

    EXPECT_TRUE(monitor.valid);
    EXPECT_NEAR(monitor.t_det, 253.150F, 1e-3F);
    EXPECT_NEAR(monitor.t_int, 31.5F, 1e-3F);
    EXPECT_NEAR(monitor.i_tec, 0.842F, 1e-4F);
    EXPECT_NEAR(monitor.u_tec, 1.973F, 1e-3F);
    EXPECT_NEAR(monitor.i_fan, 0.11F, 1e-4F);
    EXPECT_NEAR(monitor.u_sup_plus, 5.02F, 1e-3F);
    EXPECT_NEAR(monitor.u_sup_minus, -5.01F, 1e-3F);
    EXPECT_EQ(monitor.pwm, 12800u);
    EXPECT_EQ(monitor.status, 2u);
    EXPECT_TRUE(monitor.supply_on);
    EXPECT_TRUE(monitor.fan_on);
    EXPECT_GT(monitor.timestamp, 0);
}

TEST_F(PtccDevice, GetTemperatureMatchesTheMonitorField) {
    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));
    ASSERT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_OK);

    float temperature = 0.0F;
    ASSERT_EQ(rp_PtccGetTemperature(&temperature), RP_PTCC_OK);
    EXPECT_FLOAT_EQ(temperature, monitor.t_det);
}

TEST_F(PtccDevice, ParametersAreDecodedForEveryRegister) {
    const rp_ptcc_register_t registers[] = {RP_PTCC_REG_USER_SET, RP_PTCC_REG_DEFAULT,
                                            RP_PTCC_REG_USER_MIN, RP_PTCC_REG_USER_MAX};

    for (const rp_ptcc_register_t target : registers) {
        rp_ptcc_params_t params;
        std::memset(&params, 0, sizeof(params));

        ASSERT_EQ(rp_PtccGetParams(target, &params), RP_PTCC_OK) << "register " << target;
        EXPECT_TRUE(params.valid);
        EXPECT_NEAR(params.setpoint, 250.0F, 1e-3F);
        EXPECT_NEAR(params.i_tec_max, 1.5F, 1e-4F);
        EXPECT_EQ(params.tec_ctrl, RP_PTCC_CTRL_ON);
        EXPECT_EQ(params.fan_ctrl, RP_PTCC_CTRL_AUTO);
    }
}

TEST_F(PtccDevice, GetSetpointReadsTheUserRegister) {
    float setpoint = 0.0F;
    ASSERT_EQ(rp_PtccGetSetpoint(&setpoint), RP_PTCC_OK);
    EXPECT_NEAR(setpoint, 250.0F, 1e-3F);
}

TEST_F(PtccDevice, DeviceIdentificationIsDecoded) {
    rp_ptcc_device_iden_t iden;
    std::memset(&iden, 0, sizeof(iden));

    ASSERT_EQ(rp_PtccGetDeviceIden(&iden), RP_PTCC_OK);
    EXPECT_TRUE(iden.valid);
    EXPECT_EQ(iden.firmware_version, 1234u);
    EXPECT_EQ(iden.hardware_version, 11u);
}

TEST_F(PtccDevice, ModuleIdentificationIsDecoded) {
    rp_ptcc_module_iden_t iden;
    std::memset(&iden, 0, sizeof(iden));

    ASSERT_EQ(rp_PtccGetModuleIden(&iden), RP_PTCC_OK);
    EXPECT_TRUE(iden.valid);
    EXPECT_EQ(iden.cool_time, 300u);
}

TEST_F(PtccDevice, ErrorCounterStaysAtZeroOnCleanTraffic) {
    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));
    ASSERT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_OK);

    uint64_t errors = 0;
    ASSERT_EQ(rp_PtccGetErrorCount(&errors), RP_PTCC_OK);
    EXPECT_EQ(errors, 0u);
}

TEST_F(PtccDevice, CachedMonitorIsEmptyUntilTheFirstRead) {
    // A fresh handle has never polled, so the cache must not pretend otherwise.
    rp_ptcc_monitor_t cached;
    std::memset(&cached, 0, sizeof(cached));
    ASSERT_EQ(rp_PtccGetMonitor(&cached), RP_PTCC_OK);
    EXPECT_FALSE(cached.valid);
}

TEST_F(PtccDevice, ReadMonitorPopulatesTheCache) {
    rp_ptcc_monitor_t direct;
    std::memset(&direct, 0, sizeof(direct));
    ASSERT_EQ(rp_PtccReadMonitor(&direct), RP_PTCC_OK);

    rp_ptcc_monitor_t cached;
    std::memset(&cached, 0, sizeof(cached));
    ASSERT_EQ(rp_PtccGetMonitor(&cached), RP_PTCC_OK);

    EXPECT_TRUE(cached.valid);
    EXPECT_FLOAT_EQ(cached.t_det, direct.t_det);
    EXPECT_EQ(cached.timestamp, direct.timestamp);
}

TEST_F(PtccDevice, BackgroundPollerRefreshesTheCache) {
    ASSERT_EQ(rp_PtccStartMonitoring(100), RP_PTCC_OK);

    rp_ptcc_monitor_t first;
    std::memset(&first, 0, sizeof(first));
    for (int attempt = 0; attempt < 50 && !first.valid; ++attempt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ASSERT_EQ(rp_PtccGetMonitor(&first), RP_PTCC_OK);
    }
    ASSERT_TRUE(first.valid) << "the poller never produced a sample";

    rp_ptcc_monitor_t second = first;
    for (int attempt = 0; attempt < 50 && second.timestamp == first.timestamp; ++attempt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ASSERT_EQ(rp_PtccGetMonitor(&second), RP_PTCC_OK);
    }

    EXPECT_GT(second.timestamp, first.timestamp);
    EXPECT_EQ(rp_PtccStopMonitoring(), RP_PTCC_OK);
}

TEST_F(PtccDevice, MonitoringCanBeRestarted) {
    ASSERT_EQ(rp_PtccStartMonitoring(200), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccStartMonitoring(200), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccStopMonitoring(), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccStopMonitoring(), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccStartMonitoring(200), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccStopMonitoring(), RP_PTCC_OK);
}

TEST_F(PtccDevice, ReleaseStopsTheBackgroundPoller) {
    ASSERT_EQ(rp_PtccStartMonitoring(100), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccRelease(), RP_PTCC_OK);

    bool connected = true;
    EXPECT_EQ(rp_PtccIsConnected(&connected), RP_PTCC_OK);
    EXPECT_FALSE(connected);
}
