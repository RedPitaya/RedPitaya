/**
 * The detection module (LAB_M) half of the API: the parameters and readings
 * of the right hand panel. The emulator answers the LAB_M containers only
 * when it presents a LAB_M module, which is also how the "not supported"
 * paths are covered.
 */

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "rp_ptcc.h"
#include "test_support.h"

namespace {

class PtccLabM : public ::testing::Test {
   protected:
    void SetUp() override {
        m_device = std::make_unique<EmulatedDevice>("--module LAB_M");
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

TEST_F(PtccLabM, MonitorCarriesEveryField) {
    rp_ptcc_labm_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));

    ASSERT_EQ(rp_PtccReadLabMMonitor(&monitor), RP_PTCC_OK);
    EXPECT_TRUE(monitor.valid);
    EXPECT_NEAR(monitor.u_sup_plus, 5.02F, 1e-3F);
    EXPECT_NEAR(monitor.u_sup_minus, -5.01F, 1e-3F);
    EXPECT_NEAR(monitor.u_fan, 11.98F, 1e-3F);
    EXPECT_NEAR(monitor.i_tec_plus, 0.842F, 1e-4F);
    EXPECT_NEAR(monitor.i_tec_minus, -0.840F, 1e-4F);
    EXPECT_NEAR(monitor.u_th1, 1.234F, 1e-3F);
    EXPECT_NEAR(monitor.u_th2, 0.987F, 1e-3F);
    EXPECT_NEAR(monitor.u_det, 0.650F, 1e-3F);
    EXPECT_NEAR(monitor.u_1st, 0.112F, 1e-3F);
    EXPECT_NEAR(monitor.u_out, 1.503F, 1e-3F);
    EXPECT_NEAR(monitor.temperature, 29.5F, 0.1F);
    EXPECT_GT(monitor.timestamp, 0);
}

TEST_F(PtccLabM, ParametersDecodeToSiValuesAndIndices) {
    rp_ptcc_labm_params_t params;
    std::memset(&params, 0, sizeof(params));

    ASSERT_EQ(rp_PtccGetLabMParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_OK);
    EXPECT_TRUE(params.valid);
    EXPECT_NEAR(params.det_bias_u, 0.5F, 1e-3F);
    // DET_I is linear mapped from a 0..256 code, so 4 mA lands on the nearest step.
    EXPECT_NEAR(params.det_bias_i, 0.004F, 5e-5F);
    EXPECT_EQ(params.gain_code, 85U);
    EXPECT_TRUE(params.gain_known);
    EXPECT_NEAR(params.gain, 10.0F, 1e-3F);
    EXPECT_EQ(params.varactor, 2048U);
    EXPECT_EQ(params.transimpedance, RP_PTCC_TRANS_HIGH);
    EXPECT_EQ(params.coupling, RP_PTCC_COUPLING_DC);
    EXPECT_EQ(params.bandwidth, RP_PTCC_BW_HIGH);
}

TEST_F(PtccLabM, LimitsComeFromTheModuleRegisters) {
    rp_ptcc_labm_limits_t limits;
    std::memset(&limits, 0, sizeof(limits));

    ASSERT_EQ(rp_PtccGetLabMLimits(&limits), RP_PTCC_OK);
    EXPECT_TRUE(limits.valid);
    EXPECT_NEAR(limits.det_bias_u_min, 0.0F, 1e-3F);
    EXPECT_NEAR(limits.det_bias_u_max, 1.0F, 1e-3F);
    EXPECT_NEAR(limits.det_bias_i_max, 0.01F, 1e-4F);
    // OFFSET maps raw 0 to +1 V, so the register order is reversed; the API
    // still has to report min below max.
    EXPECT_LT(limits.offset_min, limits.offset_max);
    EXPECT_EQ(limits.varactor_min, 0U);
    EXPECT_EQ(limits.varactor_max, 4095U);
}

TEST_F(PtccLabM, GainValuesAreTheOnesTheDeviceDefines) {
    uint32_t count = 0;
    ASSERT_EQ(rp_PtccGetLabMGainValues(nullptr, 0, &count), RP_PTCC_OK);
    ASSERT_EQ(count, 11U);

    std::vector<float> values(count, 0.0F);
    ASSERT_EQ(rp_PtccGetLabMGainValues(values.data(), values.size(), &count), RP_PTCC_OK);
    EXPECT_NEAR(values.front(), 0.5F, 1e-6F);
    EXPECT_NEAR(values.back(), 30.0F, 1e-6F);
    for (size_t index = 1; index < values.size(); ++index) {
        EXPECT_GT(values[index], values[index - 1]);
    }
}

// Every write is read back from the emulator, which stores what the driver
// actually put on the wire, so an encoding slip cannot cancel itself out.
TEST_F(PtccLabM, SettingsSurviveTheRoundTrip) {
    rp_ptcc_labm_params_t params;

    ASSERT_EQ(rp_PtccSetLabMDetectorBiasVoltage(0.25F), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccSetLabMDetectorBiasCurrent(0.002F), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccSetLabMOffset(-0.5F), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccSetLabMGain(5.0F), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccSetLabMVaractor(1000), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccSetLabMTransimpedance(RP_PTCC_TRANS_LOW), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccSetLabMCoupling(RP_PTCC_COUPLING_AC), RP_PTCC_OK);
    ASSERT_EQ(rp_PtccSetLabMBandwidth(RP_PTCC_BW_MID), RP_PTCC_OK);

    std::memset(&params, 0, sizeof(params));
    ASSERT_EQ(rp_PtccGetLabMParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_OK);
    EXPECT_NEAR(params.det_bias_u, 0.25F, 5e-3F);
    EXPECT_NEAR(params.det_bias_i, 0.002F, 5e-5F);
    EXPECT_NEAR(params.offset, -0.5F, 1e-2F);
    EXPECT_NEAR(params.gain, 5.0F, 1e-3F);
    EXPECT_EQ(params.gain_code, 75U);
    EXPECT_EQ(params.varactor, 1000U);
    EXPECT_EQ(params.transimpedance, RP_PTCC_TRANS_LOW);
    EXPECT_EQ(params.coupling, RP_PTCC_COUPLING_AC);
    EXPECT_EQ(params.bandwidth, RP_PTCC_BW_MID);
}

// Same clamping rule as the controller side: the module stores its own limit
// instead of the value asked for, so the write is refused first.
TEST_F(PtccLabM, SettingsOutsideTheModuleRangeAreRejected) {
    EXPECT_EQ(rp_PtccSetLabMDetectorBiasVoltage(1.5F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetLabMDetectorBiasCurrent(0.05F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetLabMOffset(-2.0F), RP_PTCC_ERANGE);

    EXPECT_EQ(rp_PtccSetLabMDetectorBiasVoltage(1.0F), RP_PTCC_OK);
    EXPECT_EQ(rp_PtccSetLabMOffset(1.0F), RP_PTCC_OK);
}

// The amplifier runs at one of the documented gains, so a value in between is
// refused rather than rounded to a neighbour the panel would then misreport.
TEST_F(PtccLabM, UndefinedGainIsRejected) {
    EXPECT_EQ(rp_PtccSetLabMGain(4.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetLabMGain(0.0F), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetLabMGain(10.0F), RP_PTCC_OK);
}

TEST_F(PtccLabM, DiscreteSettingsRejectAnIndexOutOfTheList) {
    EXPECT_EQ(rp_PtccSetLabMBandwidth(static_cast<rp_ptcc_bw_t>(3)), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetLabMCoupling(static_cast<rp_ptcc_coupling_t>(2)), RP_PTCC_ERANGE);
    EXPECT_EQ(rp_PtccSetLabMTransimpedance(static_cast<rp_ptcc_trans_t>(-1)), RP_PTCC_ERANGE);
}

TEST_F(PtccLabM, GainCodeCanBeWrittenDirectly) {
    ASSERT_EQ(rp_PtccSetLabMGainCode(99), RP_PTCC_OK);

    rp_ptcc_labm_params_t params;
    std::memset(&params, 0, sizeof(params));
    ASSERT_EQ(rp_PtccGetLabMParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_OK);
    EXPECT_EQ(params.gain_code, 99U);
    EXPECT_NEAR(params.gain, 20.0F, 1e-3F);
}

TEST_F(PtccLabM, PollerRefreshesTheCachedMonitor) {
    rp_ptcc_labm_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));

    ASSERT_EQ(rp_PtccStartMonitoring(50), RP_PTCC_OK);
    for (int attempt = 0; attempt < 40 && !monitor.valid; ++attempt) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ASSERT_EQ(rp_PtccGetLabMMonitor(&monitor), RP_PTCC_OK);
    }
    rp_PtccStopMonitoring();

    EXPECT_TRUE(monitor.valid);
    EXPECT_NEAR(monitor.u_out, 1.503F, 1e-3F);
}

TEST_F(PtccLabM, NullPointersAreRejected) {
    uint32_t count = 0;
    EXPECT_EQ(rp_PtccReadLabMMonitor(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetLabMMonitor(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetLabMParams(RP_PTCC_REG_USER_SET, nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetLabMLimits(nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetLabMGainValues(nullptr, 0, nullptr), RP_PTCC_EIP);
    EXPECT_EQ(rp_PtccGetLabMGainValues(nullptr, 4, &count), RP_PTCC_EIP);
}

// A MEM module has no LAB_M containers at all. Reported as "not supported"
// rather than left to time out.
TEST(PtccLabMUnsupported, MemModuleRejectsEveryLabMCall) {
    EmulatedDevice device("--module MEM");
    ASSERT_FALSE(device.port().empty());
    ASSERT_EQ(rp_PtccInitDevice(device.port().c_str()), RP_PTCC_OK);
    rp_PtccSetThrottle(0);

    rp_ptcc_labm_monitor_t monitor;
    rp_ptcc_labm_params_t params;
    std::memset(&monitor, 0, sizeof(monitor));
    std::memset(&params, 0, sizeof(params));

    EXPECT_EQ(rp_PtccReadLabMMonitor(&monitor), RP_PTCC_ENOTSUP);
    EXPECT_EQ(rp_PtccGetLabMParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_ENOTSUP);
    EXPECT_EQ(rp_PtccSetLabMGain(10.0F), RP_PTCC_ENOTSUP);
    EXPECT_EQ(rp_PtccSetLabMBandwidth(RP_PTCC_BW_LOW), RP_PTCC_ENOTSUP);

    // The limits reader answers with an empty, invalid range instead of an
    // error, the same way the controller limits do when the read fails.
    rp_ptcc_labm_limits_t limits;
    std::memset(&limits, 0, sizeof(limits));
    EXPECT_EQ(rp_PtccGetLabMLimits(&limits), RP_PTCC_OK);
    EXPECT_FALSE(limits.valid);

    rp_PtccRelease();
}

TEST(PtccLabMUnsupported, CallsBeforeInitAreRejected) {
    rp_ptcc_labm_params_t params;
    std::memset(&params, 0, sizeof(params));

    EXPECT_EQ(rp_PtccGetLabMParams(RP_PTCC_REG_USER_SET, &params), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetLabMOffset(0.0F), RP_PTCC_ENOINIT);
    EXPECT_EQ(rp_PtccSetLabMVaractor(0), RP_PTCC_ENOINIT);
}
