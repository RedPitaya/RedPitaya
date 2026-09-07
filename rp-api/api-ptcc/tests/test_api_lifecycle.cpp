/**
 * Lifecycle of the public API: initialization, release, argument validation
 * and the behaviour of every call made before a device is open. These paths
 * must not reach the Python bridge at all, so they are the first thing to
 * break if the C layer stops guarding its inputs.
 */

#include <gtest/gtest.h>

#include <cstring>

#include "rp_ptcc.h"
#include "test_support.h"

namespace {

class PtccApi : public ::testing::Test {
   protected:
    void TearDown() override { rp_PtccRelease(); }
};

}  // namespace

TEST_F(PtccApi, ReleaseWithoutInitIsHarmlessAndIdempotent) {
    EXPECT_EQ(rp_PtccRelease(), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccRelease(), RP_PTCC_OK);
}

TEST_F(PtccApi, IsConnectedIsFalseBeforeInit) {
    bool connected = true;
    EXPECT_EQ(rp_PtccIsConnected(&connected), RP_PTCC_OK);
    EXPECT_FALSE(connected);
}

TEST_F(PtccApi, EveryOutParameterRejectsNull) {
    EXPECT_EQ(rp_PtccIsConnected(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetModuleType(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetDevicePath(nullptr, 16), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccReadMonitor(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetMonitor(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetTemperature(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetSetpoint(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetParams(RP_PTCC_REG_USER_SET, nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetDeviceIden(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetModuleIden(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetErrorCount(nullptr), RP_PTCC_EIP);
}

TEST_F(PtccApi, ZeroSizedBuffersAreRejected) {
    char buffer[8] = {0};
    uint32_t count = 0;
    EXPECT_EQ(rp_PtccGetDevicePath(buffer, 0), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccListPorts(buffer, 0, &count), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccListPorts(buffer, sizeof(buffer), nullptr), RP_PTCC_EIP);
}

TEST_F(PtccApi, CallsBeforeInitReportNotInitialized) {
    rp_ptcc_monitor_t monitor;
    rp_ptcc_params_t params;
    rp_ptcc_module_t module = RP_PTCC_MODULE_NONE;
    char path[64] = {0};
    uint64_t errors = 0;
    float value = 0.0F;

    std::memset(&monitor, 0, sizeof(monitor));
    std::memset(&params, 0, sizeof(params));

    EXPECT_EQ(rp_PtccGetDevicePath(path, sizeof(path)), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccGetModuleType(&module), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccGetMonitor(&monitor), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccGetTemperature(&value), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccGetSetpoint(&value), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccGetDeviceIden(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetErrorCount(&errors), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetCooler(RP_PTCC_CTRL_ON), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetFan(RP_PTCC_CTRL_AUTO), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetThrottle(1000), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetTimeout(1000), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccStartMonitoring(1000), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccStopMonitoring(), RP_PTCC_ENOINIT);
}

// The range check has to happen in the C layer: reaching the device with an
// out of range setpoint would burn an EEPROM write cycle for nothing.
TEST_F(PtccApi, SetpointRangeIsRejectedBeforeTouchingTheDevice) {
    EXPECT_EQ(rp_PtccSetSetpoint(RP_PTCC_MIN_TEMPERATURE - 0.1F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSetpoint(RP_PTCC_MAX_TEMPERATURE + 0.1F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSetpoint(0.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetSetpoint(-273.0F), RP_PTCC_ERANGE);

    // In range, but no device: the range check passed and the call moved on.
    EXPECT_EQ(rp_PtccSetSetpoint(230.0F), RP_PTCC_ENOINIT);
}

TEST_F(PtccApi, NegativeCurrentLimitIsRejected) {
    EXPECT_EQ(rp_PtccSetMaxCurrent(-0.5F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetMaxCurrent(1.5F), RP_PTCC_ENOINIT);
}

TEST_F(PtccApi, ListPortsWorksWithoutAnOpenDevice) {
    char buffer[1024] = {0};
    uint32_t count = 0;

    ASSERT_EQ(rp_PtccListPorts(buffer, sizeof(buffer), &count), RP_PTCC_OK);

    // The emulator's pty is not a ttyACM/ttyUSB node, so the count only has to
    // agree with the buffer contents.
    const size_t length = std::strlen(buffer);
    EXPECT_EQ(count == 0, length == 0);
}

TEST_F(PtccApi, InitOnAMissingDeviceNodeFails) {
    EXPECT_EQ(rp_PtccInitDevice("/dev/definitely-not-a-ptcc"), RP_PTCC_EOPEN);

    bool connected = true;
    EXPECT_EQ(rp_PtccIsConnected(&connected), RP_PTCC_OK);
    EXPECT_FALSE(connected);
}

TEST_F(PtccApi, InitOnANonSerialFileFails) {
    EXPECT_NE(rp_PtccInitDevice("/dev/null"), RP_PTCC_OK);
}

// Regression: the status and port lookups create the bridge on demand, which
// used to make the "no device" checks pass on a non-null handle. Device calls
// must keep reporting ENOINIT after a lookup and after a release.
TEST_F(PtccApi, LookupsDoNotMakeTheLibraryLookInitialized) {
    char buffer[1024] = {0};
    uint32_t count = 0;
    ASSERT_EQ(rp_PtccListPorts(buffer, sizeof(buffer), &count), RP_PTCC_OK);
    ASSERT_NE(rp_PtccGetStatusText(0), nullptr);
    ASSERT_NE(rp_PtccGetProtocolRevision(), nullptr);

    bool connected = true;
    EXPECT_EQ(rp_PtccIsConnected(&connected), RP_PTCC_OK);
    EXPECT_FALSE(connected);

    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));
    EXPECT_EQ(rp_PtccReadMonitor(&monitor), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetSetpoint(230.0F), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetCooler(RP_PTCC_CTRL_ON), RP_PTCC_ENOINIT);

    EXPECT_EQ(rp_PtccRelease(), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetSetpoint(230.0F), RP_PTCC_ENOINIT);
}
