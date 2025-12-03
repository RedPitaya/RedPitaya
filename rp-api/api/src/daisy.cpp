/**
 * $Id: $
 *
 * @brief Red Pitaya library daisy module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include "daisy.h"
#include "common.h"

#define CHECK_REGSET(X)                                          \
    if (g_daisy_regset == (void*)-1 || g_daisy_regset == NULL) { \
        fprintf(stderr, "%s %s", X, "regset not init\n");        \
        return RP_EMMD;                                          \
    }

static volatile daisy_regset_u_t* g_daisy_regset = NULL;

int daisy_Init() {
    return cmn_Map(DAISY_BASE_SIZE, DAISY_BASE_ADDR, (void**)&g_daisy_regset);
}

int daisy_Release() {
    if (g_daisy_regset)
        cmn_Unmap(DAISY_BASE_SIZE, (void**)&g_daisy_regset);
    g_daisy_regset = NULL;
    return RP_OK;
}

int daisy_printRegset() {

    volatile daisy_regset_u_t* daisy = NULL;

    int fd1 = -1;
    int ret = cmn_InitMap(DAISY_BASE_SIZE, DAISY_BASE_ADDR, (void**)&daisy, &fd1);
    if (ret != RP_OK) {
        return ret;
    }

    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Control", DAISY_BASE_ADDR + offsetof(daisy_regset_uint_t, cotrol), daisy->reg_uint.cotrol);
    daisy->reg.cotrol.print();
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Transmit", DAISY_BASE_ADDR + offsetof(daisy_regset_uint_t, transmit), daisy->reg_uint.transmit);
    daisy->reg.transmit.print();
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Receiver training", DAISY_BASE_ADDR + offsetof(daisy_regset_uint_t, r_training), daisy->reg_uint.r_training);
    daisy->reg.r_training.print();
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Receiver data", DAISY_BASE_ADDR + offsetof(daisy_regset_uint_t, r_data), daisy->reg_uint.r_data);
    daisy->reg.r_data.print();
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Testing control", DAISY_BASE_ADDR + offsetof(daisy_regset_uint_t, t_control), daisy->reg_uint.t_control);
    daisy->reg.t_control.print();
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Testing error counter", DAISY_BASE_ADDR + offsetof(daisy_regset_uint_t, t_error_counter),
             daisy->reg_uint.t_error_counter);
    printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Testing data counter", DAISY_BASE_ADDR + offsetof(daisy_regset_uint_t, t_data_counter),
             daisy->reg_uint.t_data_counter);

    return cmn_ReleaseClose(fd1, DAISY_BASE_SIZE, (void**)&daisy);
}

int daisy_SetTXEnable(bool enable) {
    CHECK_REGSET("[daisy_SetTXEnable]")
    cmn_Debug("g_daisy_regset->cotrol.tx_enable <- 0x%X", enable);
    g_daisy_regset->reg.cotrol.tx_enable = enable;
    return RP_OK;
}

int daisy_GetTXEnable(bool* state) {
    CHECK_REGSET("[daisy_GetTXEnable]")
    *state = g_daisy_regset->reg.cotrol.tx_enable;
    cmn_Debug("g_daisy_regset->cotrol.tx_enable -> 0x%X", *state);
    return RP_OK;
}

int daisy_SetRXEnable(bool enable) {
    CHECK_REGSET("[daisy_SetRXEnable]")
    cmn_Debug("g_daisy_regset->cotrol.rx_enable <- 0x%X", enable);
    g_daisy_regset->reg.cotrol.rx_enable = enable;
    return RP_OK;
}

int daisy_GetRXEnable(bool* state) {
    CHECK_REGSET("[daisy_GetRXEnable]")
    *state = g_daisy_regset->reg.cotrol.rx_enable;
    cmn_Debug("g_daisy_regset->cotrol.rx_enable -> 0x%X", *state);
    return RP_OK;
}

int daisy_SetDataMode(daisy_data_source_t mode) {
    CHECK_REGSET("[daisy_SetDataMode]")
    cmn_Debug("g_daisy_regset->cotrol.rx_enable <- 0x%X", mode);
    g_daisy_regset->reg.transmit.data_source = (uint8_t)mode;
    return RP_OK;
}

int daisy_GetDataMode(daisy_data_source_t* mode) {
    CHECK_REGSET("[daisy_GetDataMode]")
    *mode = (daisy_data_source_t)g_daisy_regset->reg.transmit.data_source;
    cmn_Debug("g_daisy_regset->cotrol.rx_enable -> 0x%X", *mode);
    return RP_OK;
}

int daisy_SetDataCustom(uint16_t data) {
    CHECK_REGSET("[daisy_SetDataCustom]")
    cmn_Debug("g_daisy_regset->transmit.custom <- 0x%X", data);
    g_daisy_regset->reg.transmit.custom = data;
    return RP_OK;
}

int daisy_GetDataCustom(uint16_t* data) {
    CHECK_REGSET("[daisy_GetDataCustom]")
    *data = g_daisy_regset->reg.transmit.custom;
    cmn_Debug("g_daisy_regset->transmit.custom -> 0x%X", *data);
    return RP_OK;
}

int daisy_SetReceiverTrainingEnable(bool enable) {
    CHECK_REGSET("[daisy_SetReceiverTrainingEnable]")
    cmn_Debug("g_daisy_regset->r_training.enable <- 0x%X", enable);
    g_daisy_regset->reg.r_training.enable = enable;
    return RP_OK;
}

int daisy_GetReceiverTrainingEnable(bool* state) {
    CHECK_REGSET("[daisy_GetReceiverTrainingEnable]")
    *state = g_daisy_regset->reg.r_training.enable;
    cmn_Debug("g_daisy_regset->r_training.enable -> 0x%X", *state);
    return RP_OK;
}

int daisy_GetReceiverTrainingState(bool* state) {
    CHECK_REGSET("[daisy_GetReceiverTrainingState]")
    *state = g_daisy_regset->reg.r_training.state;
    cmn_Debug("g_daisy_regset->r_training.state -> 0x%X", *state);
    return RP_OK;
}

int daisy_GetReceiverDataRaw(uint16_t* data) {
    CHECK_REGSET("[daisy_GetReceiverDataRaw]")
    *data = g_daisy_regset->reg.r_data.raw;
    cmn_Debug("g_daisy_regset->r_data.raw -> 0x%X", *data);
    return RP_OK;
}

int daisy_GetReceiverNonZeroData(uint16_t* data) {
    CHECK_REGSET("[daisy_GetReceiverNonZeroData]")
    *data = g_daisy_regset->reg.r_data.non_zero_data;
    cmn_Debug("g_daisy_regset->r_data.non_zero_data -> 0x%X", *data);
    return RP_OK;
}

int daisy_SetResetTesting(bool reset) {
    CHECK_REGSET("[daisy_SetResetTesting]")
    cmn_Debug("g_daisy_regset->t_control.reset <- 0x%X", reset);
    g_daisy_regset->reg.t_control.reset = reset;
    return RP_OK;
}

int daisy_GetResetTesting(bool* state) {
    CHECK_REGSET("[daisy_GetResetTesting]")
    *state = g_daisy_regset->reg.t_control.reset;
    cmn_Debug("g_daisy_regset->t_control.reset -> 0x%X", *state);
    return RP_OK;
}

int daisy_GetTestingErrorCounter(uint32_t* counter) {
    CHECK_REGSET("[daisy_GetTestingErrorCounter]")
    *counter = g_daisy_regset->reg.t_error_counter;
    cmn_Debug("g_daisy_regset->t_error_counter -> 0x%X", *counter);
    return RP_OK;
}

int daisy_GetTestingDataCounter(uint32_t* counter) {
    CHECK_REGSET("[daisy_GetTestingDataCounter]")
    *counter = g_daisy_regset->reg.t_data_counter;
    cmn_Debug("g_daisy_regset->t_data_counter -> 0x%X", *counter);
    return RP_OK;
}
