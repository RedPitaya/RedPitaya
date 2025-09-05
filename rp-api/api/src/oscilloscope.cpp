/**
 * $Id: $
 *
 * @brief Red Pitaya library oscilloscope module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <unistd.h>

#include <string>
#include "axi_manager.h"
#include "common.h"
#include "oscilloscope.h"
#include "rp.h"

// The FPGA register structure for oscilloscope
static volatile osc_control_t* osc_reg = NULL;

// The FPGA input signal buffer pointer for channel A
static volatile uint32_t* osc_cha = NULL;

// The FPGA input signal buffer pointer for channel B
static volatile uint32_t* osc_chb = NULL;

// // The FPGA input signal buffer pointer for AXI channel A
static volatile uint64_t osc_axi_mem_reserved_index[4] = {0, 0, 0, 0};

// static uint32_t osc_axi_cha_size = 0;

// // The FPGA input signal buffer pointer for AXI channel B
// static volatile uint16_t *osc_axi_chb = NULL;

// static uint32_t osc_axi_chb_size = 0;

static volatile osc_control_t* osc_reg_4ch = NULL;

// The FPGA input signal buffer pointer for channel C
static volatile uint32_t* osc_chc = NULL;

// The FPGA input signal buffer pointer for channel D
static volatile uint32_t* osc_chd = NULL;

// // The FPGA input signal buffer pointer for AXI channel C
// static volatile uint16_t *osc_axi_chc = NULL;

// static uint32_t osc_axi_chc_size = 0;

// // The FPGA input signal buffer pointer for AXI channel D
// static volatile uint16_t *osc_axi_chd = NULL;

// static uint32_t osc_axi_chd_size = 0;

#define RESERV_DMA_BYTES 8

/**
 * general
 */

int osc_Init(int channels) {
    ECHECK(cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg))
    osc_cha = (uint32_t*)((char*)osc_reg + OSC_CHA_OFFSET);
    osc_chb = (uint32_t*)((char*)osc_reg + OSC_CHB_OFFSET);

    if (channels == 4) {
        size_t base_addr = OSC_BASE_ADDR_4CH;
        ECHECK(cmn_Map(OSC_BASE_SIZE, base_addr, (void**)&osc_reg_4ch))
        osc_chc = (uint32_t*)((char*)osc_reg_4ch + OSC_CHA_OFFSET);
        osc_chd = (uint32_t*)((char*)osc_reg_4ch + OSC_CHB_OFFSET);
    }

    return RP_OK;
}

int osc_Release() {
    if (osc_reg)
        cmn_Unmap(OSC_BASE_SIZE, (void**)&osc_reg);
    if (osc_reg_4ch)
        cmn_Unmap(OSC_BASE_SIZE, (void**)&osc_reg_4ch);
    osc_reg = NULL;
    osc_reg_4ch = NULL;
    osc_cha = NULL;
    osc_chb = NULL;
    osc_chc = NULL;
    osc_chd = NULL;
    return RP_OK;
}

int osc_printRegset() {

    auto is_calib_fpga = rp_HPGetIsCalibInFPGAOrDefault();

    auto print = [is_calib_fpga](volatile osc_control_t* reg, int baseOffset, int baseCh, int channels) {
        auto strWithCh = [](const char* fmt, int ch) -> std::string {
            char buff[255];
            sprintf(buff, fmt, ch);
            return buff;
        };

        if (reg == NULL)
            return;

        if (baseCh == 1) {
            config_u_t conf;
            conf.reg_full = reg->config;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Configuration", baseOffset + offsetof(osc_control_t, config), conf.reg_full);
            for (auto i = 0; i < channels; i++) {
                printf("Channel %d\n", i + 1);
                conf.reg.config_ch[i].print();
            }

            trig_source_u_t trig;
            trig.reg_full = reg->trig_source;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Trigger source", baseOffset + offsetof(osc_control_t, trig_source), trig.reg_full);
            for (auto i = 0; i < channels; i++) {
                printf("Channel %d\n", i + 1);
                trig.reg[i].print();
            }
        }

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d threshold", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_thr), reg->cha_thr);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d threshold", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_thr), reg->chb_thr);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d trigger delay", baseCh).c_str(), baseOffset + offsetof(osc_control_t, trigger_delay), reg->trigger_delay);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d decimation", baseCh).c_str(), baseOffset + offsetof(osc_control_t, data_dec), reg->data_dec);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d current write pointer", baseCh).c_str(), baseOffset + offsetof(osc_control_t, wr_ptr_cur), reg->wr_ptr_cur);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d trigger write pointer", baseCh).c_str(), baseOffset + offsetof(osc_control_t, wr_ptr_trigger),
                 reg->wr_ptr_trigger);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d hysteresis", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_hystersis), reg->cha_hystersis);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d hysteresis", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_hystersis), reg->chb_hystersis);

        if (baseCh == 1) {
            trig_average_u_t conf;
            conf.reg_full = reg->average;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Average", baseOffset + offsetof(osc_control_t, average), conf.reg_full);
            for (auto i = 0; i < channels; i++) {
                printf("Channel %d\n", i + 1);
                conf.reg[i].print();
            }
        }

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d Pre Trigger counter", baseCh).c_str(), baseOffset + offsetof(osc_control_t, pre_trigger_counter),
                 reg->pre_trigger_counter);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AA filter", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_filt_aa), reg->cha_filt_aa);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d BB filter", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_filt_bb), reg->cha_filt_bb);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d KK filter", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_filt_kk), reg->cha_filt_kk);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d PP filter", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_filt_pp), reg->cha_filt_pp);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AA filter", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_filt_aa), reg->chb_filt_aa);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d BB filter", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_filt_bb), reg->chb_filt_bb);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d KK filter", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_filt_kk), reg->chb_filt_kk);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d PP filter", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_filt_pp), reg->chb_filt_pp);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI lower address", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_axi_addr_low),
                 reg->cha_axi_addr_low);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI upper address", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_axi_addr_high),
                 reg->cha_axi_addr_high);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI trigger delay", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_axi_delay), reg->cha_axi_delay);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI enable", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_axi_enable), reg->cha_axi_enable);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI WP trigger", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_axi_wr_ptr_trigger),
                 reg->cha_axi_wr_ptr_trigger);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI WP current", baseCh).c_str(), baseOffset + offsetof(osc_control_t, cha_axi_wr_ptr_cur),
                 reg->cha_axi_wr_ptr_cur);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI lower address", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_axi_addr_low),
                 reg->chb_axi_addr_low);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI upper address", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_axi_addr_high),
                 reg->chb_axi_addr_high);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI trigger delay", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_axi_delay),
                 reg->chb_axi_delay);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI enable", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_axi_enable), reg->chb_axi_enable);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI WP trigger", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_axi_wr_ptr_trigger),
                 reg->chb_axi_wr_ptr_trigger);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d AXI WP current", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, chb_axi_wr_ptr_cur),
                 reg->chb_axi_wr_ptr_cur);

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", "AXI state", baseOffset + offsetof(osc_control_t, axi_state), reg->axi_state);
        axi_state_u_t axi_state;
        axi_state.reg_full = reg->axi_state;
        axi_state.reg.print();

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", "External trigger debuncer", baseOffset + offsetof(osc_control_t, ext_trig_dbc), reg->ext_trig_dbc);
        ext_trig_dbc_u_t ext_trig_dbc;
        ext_trig_dbc.reg_full = reg->ext_trig_dbc;
        ext_trig_dbc.reg.print();

        if (baseCh == 1) {
            trig_lock_control_u_t conf;
            conf.reg_full = reg->trigger_lock_ctr;
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Trigger lock control", baseOffset + offsetof(osc_control_t, trigger_lock_ctr), conf.reg_full);
            for (auto i = 0; i < channels; i++) {
                printf("Channel %d\n", i + 1);
                conf.reg[i].print();
            }
        }

        rec_filter_bypass_u_t rec_filter;
        rec_filter.reg_full = reg->filter_bypass;
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", "Filter bypass", baseOffset + offsetof(osc_control_t, filter_bypass), reg->filter_bypass);
        rec_filter.reg.print();

        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d trigger delay", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, trigger_delay_ch2),
                 reg->trigger_delay_ch2);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d decimation", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, data_dec_ch2), reg->data_dec_ch2);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d current write pointer", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, wr_ptr_cur_ch2),
                 reg->wr_ptr_cur_ch2);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d trigger write pointer", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, wr_ptr_trigger_ch2),
                 reg->wr_ptr_trigger_ch2);
        printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d Pre Trigger counter", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, pre_trigger_counter_ch2),
                 reg->pre_trigger_counter_ch2);

        if (is_calib_fpga) {
            printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d Calibration offset", baseCh).c_str(), baseOffset + offsetof(osc_control_t, calib_offset_ch1),
                     reg->calib_offset_ch1);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d Calibration gain", baseCh).c_str(), baseOffset + offsetof(osc_control_t, calib_gain_ch1),
                     reg->calib_gain_ch1);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d Calibration offset", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, calib_offset_ch2),
                     reg->calib_offset_ch2);

            printReg("%-25s\t0x%X = 0x%08X (%d)\n", strWithCh("Channel %d Calibration gain", baseCh + 1).c_str(), baseOffset + offsetof(osc_control_t, calib_gain_ch2),
                     reg->calib_gain_ch2);
        }
    };

    auto channels = rp_HPGetFastADCChannelsCountOrDefault();
    volatile osc_control_t* osc_reg = NULL;
    volatile osc_control_t* osc_reg_4ch = NULL;

    int fd1 = -1;
    int fd2 = -1;
    int ret = cmn_InitMap(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg, &fd1);
    if (ret != RP_OK) {
        return ret;
    }
    if (channels == 4) {
        ret = cmn_InitMap(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg_4ch, &fd2);
        if (ret != RP_OK) {
            cmn_ReleaseClose(fd1, OSC_BASE_SIZE, (void**)&osc_reg);
            return ret;
        }
    }

    print(osc_reg, OSC_BASE_ADDR, 1, channels);
    print(osc_reg_4ch, OSC_BASE_ADDR_4CH, 3, channels);

    cmn_ReleaseClose(fd1, OSC_BASE_SIZE, (void**)&osc_reg);
    if (channels == 4) {
        cmn_ReleaseClose(fd2, OSC_BASE_SIZE, (void**)&osc_reg_4ch);
    }
    return RP_OK;
}

/**
 * decimation
 */

int osc_SetDecimation(rp_channel_t channel, uint32_t decimation) {
    uint32_t currentValue = 0;
    switch (channel) {
        case RP_CH_1:
            cmn_Debug("cmn_SetValue(&osc_reg->data_dec) mask 0x1FFFF <- 0x%X", decimation);
            return cmn_SetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK, &currentValue);
        case RP_CH_2:
            cmn_Debug("cmn_SetValue(&osc_reg->data_dec_ch2) mask 0x1FFFF <- 0x%X", decimation);
            return cmn_SetValue(&osc_reg->data_dec_ch2, decimation, DATA_DEC_MASK, &currentValue);
        case RP_CH_3:
            if (osc_reg_4ch) {
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->data_dec) mask 0x1FFFF <- 0x%X", decimation);
                return cmn_SetValue(&osc_reg_4ch->data_dec, decimation, DATA_DEC_MASK, &currentValue);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->data_dec_ch2) mask 0x1FFFF <- 0x%X", decimation);
                return cmn_SetValue(&osc_reg_4ch->data_dec_ch2, decimation, DATA_DEC_MASK, &currentValue);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetDecimation(rp_channel_t channel, uint32_t* decimation) {
    switch (channel) {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->data_dec_ch2, decimation, DATA_DEC_MASK);
        case RP_CH_3:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->data_dec, decimation, DATA_DEC_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->data_dec_ch2, decimation, DATA_DEC_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetAveraging(rp_channel_t channel, bool enable) {
    int value = enable ? 0x1 : 0x0;
    trig_average_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->average;
            config.reg[channel].average = value;
            osc_reg->average = config.reg_full;
            cmn_Debug("[Write] osc_reg->average <- 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetAveraging(rp_channel_t channel, bool* enable) {
    trig_average_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->average;
            *enable = config.reg[channel].average;
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

/**
 * trigger source
 */

int osc_SetTriggerSource(rp_channel_t channel, uint32_t source) {
    trig_source_u_t control;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            control.reg_full = osc_reg->trig_source;
            control.reg[channel].trig_source = source;
            osc_reg->trig_source = control.reg_full;
            cmn_Debug("[Write] osc_reg->trig_source <- 0x%X", control.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetTriggerSource(rp_channel_t channel, uint32_t* source) {
    trig_source_u_t control;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            control.reg_full = osc_reg->trig_source;
            *source = control.reg[channel].trig_source;
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetSplitTriggerMode(bool enable) {
    config_u_t config;
    config.reg_full = osc_reg->config;
    config.reg.config_ch[0].enable_split_trigger = enable ? 0x1 : 0x0;
    config.reg.config_ch[1].enable_split_trigger = enable ? 0x1 : 0x0;
    config.reg.config_ch[2].enable_split_trigger = enable ? 0x1 : 0x0;
    config.reg.config_ch[3].enable_split_trigger = enable ? 0x1 : 0x0;
    osc_reg->config = config.reg_full;
    cmn_Debug("[Write] osc_reg->config <- 0x%X", config.reg_full);
    return RP_OK;
}

int osc_GetSplitTriggerMode(bool* enable) {
    config_u_t config;
    config.reg_full = osc_reg->config;
    *enable = config.reg.config_ch[0].enable_split_trigger;
    cmn_Debug("[Read] osc_reg->config -> 0x%X", config.reg_full);
    return RP_OK;
}

int osc_SetUnlockTrigger(rp_channel_t channel) {
    trig_lock_control_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->trigger_lock_ctr;
            config.reg[channel].trig_lock = 0x1;
            osc_reg->trigger_lock_ctr = config.reg_full;
            cmn_Debug("[Write] osc_reg->trigger_lock_ctr <- 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetUnlockTrigger(rp_channel_t channel, bool* state) {
    trig_source_u_t control;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            control.reg_full = osc_reg->trig_source;
            *state = control.reg[channel].trig_lock;
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_WriteDataIntoMemory(rp_channel_t channel, bool enable) {
    config_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            config.reg.config_ch[channel].start_write = enable ? 0x1 : 0;
            osc_reg->config = config.reg_full;
            cmn_Debug("[Write] osc_reg->config <- 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_ResetWriteStateMachine(rp_channel_t channel) {
    config_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            config.reg.config_ch[channel].reset_state_machine = 0x1;
            osc_reg->config = config.reg_full;
            cmn_Debug("[Write] osc_reg->config <- 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetArmKeep(rp_channel_t channel, bool enable) {
    config_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            config.reg.config_ch[channel].arm_keep = enable ? 0x1 : 0;
            osc_reg->config = config.reg_full;
            cmn_Debug("[Write] osc_reg->config <- 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetArmKeep(rp_channel_t channel, bool* state) {
    config_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            *state = config.reg.config_ch[channel].arm_keep;
            cmn_Debug("[Read] osc_reg->config -> 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_axi_GetBufferFillStateChA(bool* state) {
    return cmn_AreBitsSet(osc_reg->axi_state, AXI_CHA_FILL_STATE, AXI_CHA_FILL_STATE, state);
}

int osc_axi_GetBufferFillStateChB(bool* state) {
    return cmn_AreBitsSet(osc_reg->axi_state, AXI_CHB_FILL_STATE, AXI_CHB_FILL_STATE, state);
}

int osc_axi_GetBufferFillStateChC(bool* state) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_AreBitsSet(osc_reg_4ch->axi_state, AXI_CHA_FILL_STATE, AXI_CHA_FILL_STATE, state);
}

int osc_axi_GetBufferFillStateChD(bool* state) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_AreBitsSet(osc_reg_4ch->axi_state, AXI_CHB_FILL_STATE, AXI_CHB_FILL_STATE, state);
}

int osc_GetBufferFillState(rp_channel_t channel, bool* state) {
    config_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            *state = config.reg.config_ch[channel].all_data_written;
            cmn_Debug("[Read] osc_reg->config -> 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetTriggerState(rp_channel_t channel, bool* received) {
    config_u_t config;
    switch (channel) {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            *received = config.reg.config_ch[channel].trigger_status;
            cmn_Debug("[Read] osc_reg->config -> 0x%X", config.reg_full);
            return RP_OK;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetPreTriggerCounter(rp_channel_t channel, uint32_t* value) {
    switch (channel) {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->pre_trigger_counter, value, PRE_TRIGGER_COUNTER);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->pre_trigger_counter_ch2, value, PRE_TRIGGER_COUNTER);
        case RP_CH_3:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->pre_trigger_counter, value, PRE_TRIGGER_COUNTER);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->pre_trigger_counter_ch2, value, PRE_TRIGGER_COUNTER);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

/**
 * trigger delay
 */

int osc_SetTriggerDelay(rp_channel_t channel, uint32_t decimated_data_num) {
    uint32_t currentValue = 0;
    switch (channel) {
        case RP_CH_1:
            cmn_Debug("cmn_SetValue(&osc_reg->trigger_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
            return cmn_SetValue(&osc_reg->trigger_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
        case RP_CH_2:
            cmn_Debug("cmn_SetValue(&osc_reg->trigger_delay_ch2) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
            return cmn_SetValue(&osc_reg->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
        case RP_CH_3:
            if (osc_reg_4ch) {
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->trigger_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
                return cmn_SetValue(&osc_reg_4ch->trigger_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->trigger_delay_ch2) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
                return cmn_SetValue(&osc_reg_4ch->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetTriggerDelay(rp_channel_t channel, uint32_t* decimated_data_num) {
    switch (channel) {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->trigger_delay, decimated_data_num, TRIG_DELAY_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK);
        case RP_CH_3:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->trigger_delay, decimated_data_num, TRIG_DELAY_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

/**
 * Threshold
 */

int osc_SetThresholdChA(uint32_t threshold) {
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChA(uint32_t* threshold) {
    return cmn_GetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChB(uint32_t threshold) {
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChB(uint32_t* threshold) {
    return cmn_GetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChC(uint32_t threshold) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg_4ch->cha_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChC(uint32_t* threshold) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChD(uint32_t threshold) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg_4ch->chb_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChD(uint32_t* threshold) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_thr, threshold, THRESHOLD_MASK);
}

/**
 * Hysteresis
 */
int osc_SetHysteresisChA(uint32_t hysteresis) {
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_hystersis) mask 0x3FFF <- 0x%X", hysteresis);
    return cmn_SetValue(&osc_reg->cha_hystersis, hysteresis, HYSTERESIS_MASK, &currentValue);
}

int osc_GetHysteresisChA(uint32_t* hysteresis) {
    return cmn_GetValue(&osc_reg->cha_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetHysteresisChB(uint32_t hysteresis) {
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_hystersis) mask 0x3FFF <- 0x%X", hysteresis);
    return cmn_SetValue(&osc_reg->chb_hystersis, hysteresis, HYSTERESIS_MASK, &currentValue);
}

int osc_GetHysteresisChB(uint32_t* hysteresis) {
    return cmn_GetValue(&osc_reg->chb_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetHysteresisChC(uint32_t hysteresis) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_hystersis) mask 0x3FFF <- 0x%X", hysteresis);
    return cmn_SetValue(&osc_reg_4ch->cha_hystersis, hysteresis, HYSTERESIS_MASK, &currentValue);
}

int osc_GetHysteresisChC(uint32_t* hysteresis) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetHysteresisChD(uint32_t hysteresis) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_hystersis) mask 0x3FFF <- 0x%X", hysteresis);
    return cmn_SetValue(&osc_reg_4ch->chb_hystersis, hysteresis, HYSTERESIS_MASK, &currentValue);
}

int osc_GetHysteresisChD(uint32_t* hysteresis) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetEqFilterBypass(rp_channel_t channel, bool enable) {
    rec_filter_bypass_u_t bypass;
    switch (channel) {
        case RP_CH_1:
            bypass.reg_full = osc_reg->filter_bypass;
            bypass.reg.bypass_ch1 = enable;
            cmn_Debug("cmn_SetValue(osc_reg->filter_bypass) <- 0x%X", bypass.reg_full);
            osc_reg->filter_bypass = bypass.reg_full;
            return RP_OK;
        case RP_CH_2:
            bypass.reg_full = osc_reg->filter_bypass;
            bypass.reg.bypass_ch2 = enable;
            cmn_Debug("cmn_SetValue(osc_reg->filter_bypass) <- 0x%X", bypass.reg_full);
            osc_reg->filter_bypass = bypass.reg_full;
            return RP_OK;
        case RP_CH_3:
            if (osc_reg_4ch) {
                bypass.reg_full = osc_reg_4ch->filter_bypass;
                bypass.reg.bypass_ch1 = enable;
                cmn_Debug("cmn_SetValue(osc_reg_4ch->filter_bypass) <- 0x%X", bypass.reg_full);
                osc_reg_4ch->filter_bypass = bypass.reg_full;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                bypass.reg_full = osc_reg_4ch->filter_bypass;
                bypass.reg.bypass_ch2 = enable;
                cmn_Debug("cmn_SetValue(osc_reg_4ch->filter_bypass) <- 0x%X", bypass.reg_full);
                osc_reg_4ch->filter_bypass = bypass.reg_full;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetEqFilterBypass(rp_channel_t channel, bool* enable) {
    rec_filter_bypass_u_t bypass;
    switch (channel) {
        case RP_CH_1:
            bypass.reg_full = osc_reg->filter_bypass;
            *enable = bypass.reg.bypass_ch1;
            return RP_OK;
        case RP_CH_2:
            bypass.reg_full = osc_reg->filter_bypass;
            *enable = bypass.reg.bypass_ch2;
            return RP_OK;
        case RP_CH_3:
            if (osc_reg_4ch) {
                bypass.reg_full = osc_reg_4ch->filter_bypass;
                *enable = bypass.reg.bypass_ch1;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                bypass.reg_full = osc_reg_4ch->filter_bypass;
                *enable = bypass.reg.bypass_ch2;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetCalibOffsetInFPGA(rp_channel_t channel, uint8_t bits, int32_t offset) {

    if (bits > 16) {
        FATAL("The number of bits for ADC is greater than the allowed limit.")
        return RP_EOOR;
    }

    int16_t offsetCalc = offset * -1;  //-(pow(2, 16 - bits));

    switch (channel) {
        case RP_CH_1:
            cmn_Debug("osc_reg->calib_offset_ch1 <- 0x%X", offsetCalc);
            osc_reg->calib_offset_ch1 = offsetCalc;
            return RP_OK;
        case RP_CH_2:
            cmn_Debug("osc_reg->calib_offset_ch2 <- 0x%X", offsetCalc);
            osc_reg->calib_offset_ch2 = offsetCalc;
            return RP_OK;
        case RP_CH_3:
            if (osc_reg_4ch) {
                cmn_Debug("osc_reg_4ch->calib_offset_ch1 <- 0x%X", offsetCalc);
                osc_reg_4ch->calib_offset_ch1 = offsetCalc;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                cmn_Debug("osc_reg_4ch->calib_offset_ch2 <- 0x%X", offsetCalc);
                osc_reg_4ch->calib_offset_ch2 = offsetCalc;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetCalibGainInFPGA(rp_channel_t channel, double gain) {
    if (gain >= 2) {
        ERROR_LOG("The gain for channel %d is very large %f. Will be limited to 1.999999", channel + 1, gain)
        gain = 1.999999;
    }

    if (gain < 0) {
        ERROR_LOG("The gain for channel %d should not be negative.", channel + 1)
        gain = 0;
    }

    uint16_t gainCalc = gain * 32768;

    uint32_t currentValue = 0;
    switch (channel) {
        case RP_CH_1:
            cmn_Debug("cmn_SetValue(&osc_reg->calib_gain_ch1) mask 0xFFFF <- 0x%X", gainCalc);
            return cmn_SetValue(&osc_reg->calib_gain_ch1, gainCalc, CALIB_MASK, &currentValue);
        case RP_CH_2:
            cmn_Debug("cmn_SetValue(&osc_reg->calib_gain_ch2) mask 0xFFFF <- 0x%X", gainCalc);
            return cmn_SetValue(&osc_reg->calib_gain_ch2, gainCalc, CALIB_MASK, &currentValue);
        case RP_CH_3:
            if (osc_reg_4ch) {
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->calib_gain_ch1) mask 0xFFFF <- 0x%X", gainCalc);
                return cmn_SetValue(&osc_reg_4ch->calib_gain_ch1, gainCalc, CALIB_MASK, &currentValue);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->calib_gain_ch2) mask 0xFFFF <- 0x%X", gainCalc);
                return cmn_SetValue(&osc_reg_4ch->calib_gain_ch2, gainCalc, CALIB_MASK, &currentValue);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetCalibOffsetInFPGA(rp_channel_t channel, uint8_t bits, int32_t* offset) {
    uint32_t offset_fpga = 0;
    // int32_t devider = -(pow(2, 16 - bits));
    int32_t devider = -1;
    switch (channel) {
        case RP_CH_1: {
            auto ret = cmn_GetValue(&osc_reg->calib_offset_ch1, &offset_fpga, CALIB_MASK);
            *offset = (int16_t)offset_fpga / devider;
            return ret;
        }
        case RP_CH_2: {
            auto ret = cmn_GetValue(&osc_reg->calib_offset_ch2, &offset_fpga, CALIB_MASK);
            *offset = (int16_t)offset_fpga / devider;
            return ret;
        }
        case RP_CH_3: {
            if (osc_reg_4ch) {
                auto ret = cmn_GetValue(&osc_reg_4ch->calib_offset_ch1, &offset_fpga, CALIB_MASK);
                *offset = (int16_t)offset_fpga / devider;
                return ret;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        }
        case RP_CH_4: {
            if (osc_reg_4ch) {
                auto ret = cmn_GetValue(&osc_reg_4ch->calib_offset_ch2, &offset_fpga, CALIB_MASK);
                *offset = (int16_t)offset_fpga / devider;
                return ret;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        }
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetCalibGainInFPGA(rp_channel_t channel, double* gain) {
    switch (channel) {
        case RP_CH_1:
            *gain = (double)osc_reg->calib_gain_ch1 / 32768.0;
            return RP_OK;
        case RP_CH_2:
            *gain = (double)osc_reg->calib_gain_ch2 / 32768.0;
            return RP_OK;
        case RP_CH_3:
            if (osc_reg_4ch) {
                *gain = (double)osc_reg_4ch->calib_gain_ch1 / 32768.0;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                *gain = (double)osc_reg_4ch->calib_gain_ch2 / 32768.0;
                return RP_OK;
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

/**
 * Equalization filters
 */
int osc_SetEqFiltersChA(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp) {
    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;

    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA, &currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER, &currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER, &currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER, &currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChA(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp) {
    cmn_GetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_SetEqFiltersChB(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp) {
    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA, &currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER, &currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER, &currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER, &currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChB(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp) {
    cmn_GetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_SetEqFiltersChC(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp) {
    if (!osc_reg_4ch)
        return RP_NOTS;

    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;

    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg_4ch->cha_filt_aa, coef_aa, EQ_FILTER_AA, &currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg_4ch->cha_filt_bb, coef_bb, EQ_FILTER, &currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg_4ch->cha_filt_kk, coef_kk, EQ_FILTER, &currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg_4ch->cha_filt_pp, coef_pp, EQ_FILTER, &currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChC(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp) {
    if (!osc_reg_4ch)
        return RP_NOTS;

    cmn_GetValue(&osc_reg_4ch->cha_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg_4ch->cha_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg_4ch->cha_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg_4ch->cha_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_SetEqFiltersChD(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp) {
    if (!osc_reg_4ch)
        return RP_NOTS;

    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg_4ch->chb_filt_aa, coef_aa, EQ_FILTER_AA, &currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg_4ch->chb_filt_bb, coef_bb, EQ_FILTER, &currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg_4ch->chb_filt_kk, coef_kk, EQ_FILTER, &currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg_4ch->chb_filt_pp, coef_pp, EQ_FILTER, &currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChD(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp) {
    if (!osc_reg_4ch)
        return RP_NOTS;

    cmn_GetValue(&osc_reg_4ch->chb_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg_4ch->chb_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg_4ch->chb_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg_4ch->chb_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

/**
 * Write pointer
 */
int osc_GetWritePointer(rp_channel_t channel, uint32_t* pos) {
    switch (channel) {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->wr_ptr_cur, pos, WRITE_POINTER_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->wr_ptr_cur_ch2, pos, WRITE_POINTER_MASK);
        case RP_CH_3:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_cur, pos, WRITE_POINTER_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_cur_ch2, pos, WRITE_POINTER_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos) {

    switch (channel) {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->wr_ptr_trigger, pos, WRITE_POINTER_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->wr_ptr_trigger_ch2, pos, WRITE_POINTER_MASK);
        case RP_CH_3:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_trigger, pos, WRITE_POINTER_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch) {
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_trigger_ch2, pos, WRITE_POINTER_MASK);
            } else {
                ERROR_LOG("Registers for channels 3 and 4 are not initialized")
            }
            break;
        default:
            ERROR_LOG("Wrong channel %d", channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetExtTriggerDebouncer(uint32_t value) {
    if (DEBAUNCER_MASK < value) {
        ERROR_LOG("Error value 0x%X very big", value)
        return RP_EIPV;
    }
    cmn_Debug("[osc_SetExtTriggerDebouncer] osc_reg.ext_trig_dbc <- 0x%X", value);
    osc_reg->ext_trig_dbc = value;

    if (osc_reg_4ch) {
        cmn_Debug("[osc_SetExtTriggerDebouncer] osc_reg_4ch.ext_trig_dbc <- 0x%X", value);
        osc_reg_4ch->ext_trig_dbc = value;
    }
    return RP_OK;
}

int osc_GetExtTriggerDebouncer(uint32_t* value) {
    *value = osc_reg->ext_trig_dbc;
    return RP_OK;
}

/**
 * Raw buffers
 */
const volatile uint32_t* osc_GetDataBufferChA() {
    return osc_cha;
}

const volatile uint32_t* osc_GetDataBufferChB() {
    return osc_chb;
}

const volatile uint32_t* osc_GetDataBufferChC() {
    return osc_chc;
}

const volatile uint32_t* osc_GetDataBufferChD() {
    return osc_chd;
}

/**
 * AXI mode
 */

int osc_axi_EnableChA(bool enable) {
    uint32_t tmp;
    uint32_t ch_addr_end, ch_addr_start;
    ECHECK(cmn_GetValue(&osc_reg->cha_axi_addr_high, &ch_addr_end, FULL_MASK))
    ECHECK(cmn_GetValue(&osc_reg->cha_axi_addr_low, &ch_addr_start, FULL_MASK))
    if (enable) {
        ECHECK(axi_initManager())
        axi_releaseMemory(osc_axi_mem_reserved_index[0]);
        if (ch_addr_start > ch_addr_end) {
            return RP_EOOR;
        }
        uint64_t index;
        ECHECK(axi_reserveMemory(ch_addr_start, ch_addr_end - ch_addr_start, &index))
        osc_axi_mem_reserved_index[0] = index;
        cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg->cha_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    } else {
        axi_releaseMemory(osc_axi_mem_reserved_index[0]);
        osc_axi_mem_reserved_index[0] = 0;
    }
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg->cha_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_EnableChB(bool enable) {
    uint32_t tmp;
    uint32_t ch_addr_end, ch_addr_start;
    ECHECK(cmn_GetValue(&osc_reg->chb_axi_addr_high, &ch_addr_end, FULL_MASK))
    ECHECK(cmn_GetValue(&osc_reg->chb_axi_addr_low, &ch_addr_start, FULL_MASK))
    if (enable) {
        ECHECK(axi_initManager())
        axi_releaseMemory(osc_axi_mem_reserved_index[1]);
        if (ch_addr_start > ch_addr_end) {
            return RP_EOOR;
        }
        uint64_t index;
        ECHECK(axi_reserveMemory(ch_addr_start, ch_addr_end - ch_addr_start, &index))
        osc_axi_mem_reserved_index[1] = index;
        cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg->chb_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    } else {
        axi_releaseMemory(osc_axi_mem_reserved_index[1]);
        osc_axi_mem_reserved_index[1] = 0;
    }
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg->chb_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_EnableChC(bool enable) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    uint32_t ch_addr_end, ch_addr_start;
    ECHECK(cmn_GetValue(&osc_reg_4ch->cha_axi_addr_high, &ch_addr_end, FULL_MASK))
    ECHECK(cmn_GetValue(&osc_reg_4ch->cha_axi_addr_low, &ch_addr_start, FULL_MASK))
    if (enable) {
        ECHECK(axi_initManager())
        axi_releaseMemory(osc_axi_mem_reserved_index[2]);
        if (ch_addr_start > ch_addr_end) {
            return RP_EOOR;
        }
        uint64_t index;
        ECHECK(axi_reserveMemory(ch_addr_start, ch_addr_end - ch_addr_start, &index))
        osc_axi_mem_reserved_index[2] = index;
        cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg_4ch->cha_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    } else {
        axi_releaseMemory(osc_axi_mem_reserved_index[2]);
        osc_axi_mem_reserved_index[2] = 0;
    }
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_EnableChD(bool enable) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    uint32_t ch_addr_end, ch_addr_start;
    ECHECK(cmn_GetValue(&osc_reg_4ch->chb_axi_addr_high, &ch_addr_end, FULL_MASK))
    ECHECK(cmn_GetValue(&osc_reg_4ch->chb_axi_addr_low, &ch_addr_start, FULL_MASK))
    if (enable) {
        ECHECK(axi_initManager())
        axi_releaseMemory(osc_axi_mem_reserved_index[3]);
        if (ch_addr_start > ch_addr_end) {
            return RP_EOOR;
        }
        uint64_t index;
        ECHECK(axi_reserveMemory(ch_addr_start, ch_addr_end - ch_addr_start, &index))
        osc_axi_mem_reserved_index[3] = index;
        cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg_4ch->chb_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    } else {
        axi_releaseMemory(osc_axi_mem_reserved_index[3]);
        osc_axi_mem_reserved_index[3] = 0;
    }
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_SetAddressStartChA(uint32_t address) {
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->cha_axi_addr_low, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressStartChB(uint32_t address) {
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->chb_axi_addr_low, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressStartChC(uint32_t address) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_addr_low, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressStartChD(uint32_t address) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_addr_low, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressEndChA(uint32_t address) {
    address -= RESERV_DMA_BYTES;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->cha_axi_addr_high, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressEndChB(uint32_t address) {
    address -= RESERV_DMA_BYTES;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->chb_axi_addr_high, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressEndChC(uint32_t address) {
    address -= RESERV_DMA_BYTES;
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_addr_high, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressEndChD(uint32_t address) {
    address -= RESERV_DMA_BYTES;
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_addr_high, address, FULL_MASK, &tmp);
}

int osc_axi_GetAddressStartChA(uint32_t* address) {
    return cmn_GetValue(&osc_reg->cha_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressStartChB(uint32_t* address) {
    return cmn_GetValue(&osc_reg->chb_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressStartChC(uint32_t* address) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressStartChD(uint32_t* address) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressEndChA(uint32_t* address) {
    int ret = cmn_GetValue(&osc_reg->cha_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetAddressEndChB(uint32_t* address) {
    int ret = cmn_GetValue(&osc_reg->chb_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetAddressEndChC(uint32_t* address) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    int ret = cmn_GetValue(&osc_reg_4ch->cha_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetAddressEndChD(uint32_t* address) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    int ret = cmn_GetValue(&osc_reg_4ch->chb_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetWritePointerChA(uint32_t* pos) {
    uint32_t addr;
    osc_axi_GetAddressStartChA(&addr);
    cmn_GetValue(&osc_reg->cha_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerChB(uint32_t* pos) {
    uint32_t addr;
    osc_axi_GetAddressStartChB(&addr);
    cmn_GetValue(&osc_reg->chb_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerChC(uint32_t* pos) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChC(&addr);
    cmn_GetValue(&osc_reg_4ch->cha_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerChD(uint32_t* pos) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChD(&addr);
    cmn_GetValue(&osc_reg_4ch->chb_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChA(uint32_t* pos) {
    uint32_t addr;
    osc_axi_GetAddressStartChA(&addr);
    cmn_GetValue(&osc_reg->cha_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChB(uint32_t* pos) {
    uint32_t addr;
    osc_axi_GetAddressStartChB(&addr);
    cmn_GetValue(&osc_reg->chb_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChC(uint32_t* pos) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChC(&addr);
    cmn_GetValue(&osc_reg_4ch->cha_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChD(uint32_t* pos) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChD(&addr);
    cmn_GetValue(&osc_reg_4ch->chb_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_SetTriggerDelayChA(uint32_t decimated_data_num) {
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_SetTriggerDelayChB(uint32_t decimated_data_num) {
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_SetTriggerDelayChC(uint32_t decimated_data_num) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_SetTriggerDelayChD(uint32_t decimated_data_num) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_GetTriggerDelayChA(uint32_t* decimated_data_num) {
    return cmn_GetValue(&osc_reg->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

int osc_axi_GetTriggerDelayChB(uint32_t* decimated_data_num) {
    return cmn_GetValue(&osc_reg->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

int osc_axi_GetTriggerDelayChC(uint32_t* decimated_data_num) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

int osc_axi_GetTriggerDelayChD(uint32_t* decimated_data_num) {
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

const uint16_t* osc_axi_GetDataBufferCh(rp_channel_t channel) {
    auto idx = osc_axi_mem_reserved_index[channel];
    if (idx == 0) {
        ERROR_LOG("Buffer for channel %d not mapped", channel + 1)
    }
    uint16_t* buffer = NULL;
    uint32_t size = 0;
    if (axi_getMapped(idx, &buffer, &size) != RP_OK) {
        ERROR_LOG("Buffer for channel %d not mapped", channel + 1)
        return NULL;
    }
    return buffer;
}
