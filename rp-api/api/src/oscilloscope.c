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

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#include "common.h"
#include "oscilloscope.h"
#include "rp.h"

// The FPGA register structure for oscilloscope
static volatile osc_control_t *osc_reg = NULL;

// The FPGA input signal buffer pointer for channel A
static volatile uint32_t *osc_cha = NULL;

// The FPGA input signal buffer pointer for channel B
static volatile uint32_t *osc_chb = NULL;

// The FPGA input signal buffer pointer for AXI channel A
static volatile uint16_t *osc_axi_cha = NULL;

static uint32_t osc_axi_cha_size = 0;

// The FPGA input signal buffer pointer for AXI channel B
static volatile uint16_t *osc_axi_chb = NULL;

static uint32_t osc_axi_chb_size = 0;

// The /dev/mem file descriptor
static int mem_fd = -1;

static volatile osc_control_t *osc_reg_4ch = NULL;

// The FPGA input signal buffer pointer for channel C
static volatile uint32_t *osc_chc = NULL;

// The FPGA input signal buffer pointer for channel D
static volatile uint32_t *osc_chd = NULL;

// The FPGA input signal buffer pointer for AXI channel C
static volatile uint16_t *osc_axi_chc = NULL;

static uint32_t osc_axi_chc_size = 0;

// The FPGA input signal buffer pointer for AXI channel D
static volatile uint16_t *osc_axi_chd = NULL;

static uint32_t osc_axi_chd_size = 0;

static uint32_t g_adc_axi_start = 0;

static uint32_t g_adc_axi_size = 0;

#define RESERV_DMA_BYTES 8

/**
 * general
 */

int osc_Init(int channels)
{
    ECHECK(cmn_Map(OSC_BASE_SIZE, OSC_BASE_ADDR, (void**)&osc_reg))
    osc_cha = (uint32_t*)((char*)osc_reg + OSC_CHA_OFFSET);
    osc_chb = (uint32_t*)((char*)osc_reg + OSC_CHB_OFFSET);

    if (channels == 4){
        size_t base_addr = OSC_BASE_ADDR_4CH;
        ECHECK(cmn_Map(OSC_BASE_SIZE, base_addr, (void**)&osc_reg_4ch))
        osc_chc = (uint32_t*)((char*)osc_reg_4ch + OSC_CHA_OFFSET);
        osc_chd = (uint32_t*)((char*)osc_reg_4ch + OSC_CHB_OFFSET);
    }

    ECHECK(cmn_GetReservedMemory(&g_adc_axi_start,&g_adc_axi_size))
    if (g_adc_axi_start == 0 || g_adc_axi_size == 0){
        ERROR("Error initializing memory region for axi mode.")
        return RP_EOOR;
    }

    return RP_OK;
}

int osc_Release()
{
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
    if (mem_fd != -1) {
        if(close(mem_fd) < 0) {
            return RP_ECMD;
        }
    }
    mem_fd = -1;
    return RP_OK;
}

int osc_axi_GetMemoryRegion(uint32_t *_start,uint32_t *_size){
    *_start = g_adc_axi_start;
    *_size = g_adc_axi_size;
    return RP_OK;
}

/**
 * decimation
 */

int osc_SetDecimation(rp_channel_t channel, uint32_t decimation)
{
    uint32_t currentValue = 0;
    switch (channel)
    {
        case RP_CH_1:
            cmn_Debug("cmn_SetValue(&osc_reg->data_dec) mask 0x1FFFF <- 0x%X", decimation);
            return cmn_SetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK,&currentValue);
        case RP_CH_2:
            cmn_Debug("cmn_SetValue(&osc_reg->data_dec_ch2) mask 0x1FFFF <- 0x%X", decimation);
            return cmn_SetValue(&osc_reg->data_dec_ch2, decimation, DATA_DEC_MASK,&currentValue);
        case RP_CH_3:
            if (osc_reg_4ch){
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->data_dec) mask 0x1FFFF <- 0x%X", decimation);
                return cmn_SetValue(&osc_reg_4ch->data_dec, decimation, DATA_DEC_MASK,&currentValue);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch){
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->data_dec_ch2) mask 0x1FFFF <- 0x%X", decimation);
                return cmn_SetValue(&osc_reg_4ch->data_dec_ch2, decimation, DATA_DEC_MASK,&currentValue);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetDecimation(rp_channel_t channel, uint32_t* decimation)
{
    switch (channel)
    {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->data_dec, decimation, DATA_DEC_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->data_dec_ch2, decimation, DATA_DEC_MASK);
        case RP_CH_3:
            if (osc_reg_4ch){
               return cmn_GetValue(&osc_reg_4ch->data_dec, decimation, DATA_DEC_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch){
                return cmn_GetValue(&osc_reg_4ch->data_dec_ch2, decimation, DATA_DEC_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetAveraging(rp_channel_t channel, bool enable)
{
    int value  = enable ? 0x1 : 0x0;
    trig_average_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->average;
            config.reg[channel].average = value;
            osc_reg->average = config.reg_full;
            cmn_Debug("[Write] osc_reg->average <- 0x%X",  config.reg_full);
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetAveraging(rp_channel_t channel, bool* enable)
{
    trig_average_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->average;
            *enable = config.reg[channel].average;
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

/**
 * trigger source
 */

int osc_SetTriggerSource(rp_channel_t channel, uint32_t source)
{
    trig_source_u_t control;
    switch (channel)
    {
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
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetTriggerSource(rp_channel_t channel, uint32_t* source)
{
    trig_source_u_t control;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            control.reg_full = osc_reg->trig_source;
            *source = control.reg[channel].trig_source;
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetSplitTriggerMode(bool enable)
{
    config_u_t config;
    config.reg_full = osc_reg->config;
    config.reg.config_ch[0].enable_split_trigger = enable ? 0x1 : 0x0;
    config.reg.config_ch[1].enable_split_trigger = enable ? 0x1 : 0x0;
    config.reg.config_ch[2].enable_split_trigger = enable ? 0x1 : 0x0;
    config.reg.config_ch[3].enable_split_trigger = enable ? 0x1 : 0x0;
    osc_reg->config = config.reg_full;
    cmn_Debug("[Write] osc_reg->config <- 0x%X",config.reg_full);
    return RP_OK;
}

int osc_GetSplitTriggerMode(bool* enable)
{
    config_u_t config;
    config.reg_full = osc_reg->config;
    *enable = config.reg.config_ch[0].enable_split_trigger;
    cmn_Debug("[Read] osc_reg->config -> 0x%X",config.reg_full);
    return RP_OK;
}



int osc_SetUnlockTrigger(rp_channel_t channel)
{
    trig_lock_control_u_t config;
    switch (channel)
    {
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
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetUnlockTrigger(rp_channel_t channel, bool *state){
    trig_source_u_t control;
        switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            control.reg_full = osc_reg->trig_source;
            *state = control.reg[channel].trig_lock;
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}


int osc_WriteDataIntoMemory(rp_channel_t channel, bool enable)
{
    config_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            config.reg.config_ch[channel].start_write = enable ? 0x1 : 0;
            osc_reg->config = config.reg_full;
            cmn_Debug("[Write] osc_reg->config <- 0x%X",config.reg_full);
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_ResetWriteStateMachine(rp_channel_t channel)
{
    config_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            config.reg.config_ch[channel].reset_state_machine = 0x1;
            osc_reg->config = config.reg_full;
            cmn_Debug("[Write] osc_reg->config <- 0x%X",config.reg_full);
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetArmKeep(rp_channel_t channel, bool enable)
{
    config_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            config.reg.config_ch[channel].arm_keep = enable ? 0x1 : 0;
            osc_reg->config = config.reg_full;
            cmn_Debug("[Write] osc_reg->config <- 0x%X",config.reg_full);
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetArmKeep(rp_channel_t channel, bool *state){
    config_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            *state = config.reg.config_ch[channel].arm_keep;
            cmn_Debug("[Read] osc_reg->config -> 0x%X",config.reg_full);
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_axi_GetBufferFillStateChA(bool *state)
{
    return cmn_AreBitsSet(osc_reg->axi_state, AXI_CHA_FILL_STATE , AXI_CHA_FILL_STATE, state);
}

int osc_axi_GetBufferFillStateChB(bool *state)
{
    return cmn_AreBitsSet(osc_reg->axi_state, AXI_CHB_FILL_STATE , AXI_CHB_FILL_STATE, state);
}

int osc_axi_GetBufferFillStateChC(bool *state)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_AreBitsSet(osc_reg_4ch->axi_state, AXI_CHA_FILL_STATE , AXI_CHA_FILL_STATE, state);
}

int osc_axi_GetBufferFillStateChD(bool *state)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_AreBitsSet(osc_reg_4ch->axi_state, AXI_CHB_FILL_STATE , AXI_CHB_FILL_STATE, state);
}

int osc_GetBufferFillState(rp_channel_t channel, bool *state)
{
    config_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            *state = config.reg.config_ch[channel].all_data_written;
            cmn_Debug("[Read] osc_reg->config -> 0x%X",config.reg_full);
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetTriggerState(rp_channel_t channel, bool *received)
{
    config_u_t config;
    switch (channel)
    {
        case RP_CH_1:
        case RP_CH_2:
        case RP_CH_3:
        case RP_CH_4:
            config.reg_full = osc_reg->config;
            *received = config.reg.config_ch[channel].trigger_status;
            cmn_Debug("[Read] osc_reg->config -> 0x%X",config.reg_full);
            return RP_OK;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetPreTriggerCounter(rp_channel_t channel, uint32_t *value)
{
    switch (channel)
    {
        case RP_CH_1:
                return cmn_GetValue(&osc_reg->pre_trigger_counter, value, PRE_TRIGGER_COUNTER);
        case RP_CH_2:
                return cmn_GetValue(&osc_reg->pre_trigger_counter_ch2, value, PRE_TRIGGER_COUNTER);
        case RP_CH_3:
            if (osc_reg_4ch){
                    return cmn_GetValue(&osc_reg_4ch->pre_trigger_counter, value, PRE_TRIGGER_COUNTER);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch){
                    return cmn_GetValue(&osc_reg_4ch->pre_trigger_counter_ch2, value, PRE_TRIGGER_COUNTER);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

/**
 * trigger delay
 */

int osc_SetTriggerDelay(rp_channel_t channel, uint32_t decimated_data_num)
{
    uint32_t currentValue = 0;
    switch (channel)
    {
        case RP_CH_1:
            cmn_Debug("cmn_SetValue(&osc_reg->trigger_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
            return cmn_SetValue(&osc_reg->trigger_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
        case RP_CH_2:
            cmn_Debug("cmn_SetValue(&osc_reg->trigger_delay_ch2) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
            return cmn_SetValue(&osc_reg->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
        case RP_CH_3:
            if (osc_reg_4ch){
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->trigger_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
                return cmn_SetValue(&osc_reg_4ch->trigger_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch){
                cmn_Debug("cmn_SetValue(&osc_reg_4ch->trigger_delay_ch2) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
                return cmn_SetValue(&osc_reg_4ch->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetTriggerDelay(rp_channel_t channel, uint32_t* decimated_data_num)
{
    switch (channel)
    {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->trigger_delay, decimated_data_num, TRIG_DELAY_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK);
        case RP_CH_3:
            if (osc_reg_4ch){
                return cmn_GetValue(&osc_reg_4ch->trigger_delay, decimated_data_num, TRIG_DELAY_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch){
                return cmn_GetValue(&osc_reg_4ch->trigger_delay_ch2, decimated_data_num, TRIG_DELAY_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
                return RP_NOTS;
            }
            break;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

/**
 * Threshold
 */

int osc_SetThresholdChA(uint32_t threshold)
{
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChA(uint32_t* threshold)
{
    return cmn_GetValue(&osc_reg->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChB(uint32_t threshold)
{
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChB(uint32_t* threshold)
{
    return cmn_GetValue(&osc_reg->chb_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChC(uint32_t threshold)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg_4ch->cha_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChC(uint32_t* threshold)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_thr, threshold, THRESHOLD_MASK);
}

int osc_SetThresholdChD(uint32_t threshold)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_thr) mask 0x3FFF <- 0x%X", threshold);
    return cmn_SetValue(&osc_reg_4ch->chb_thr, threshold, THRESHOLD_MASK, &currentValue);
}

int osc_GetThresholdChD(uint32_t* threshold)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_thr, threshold, THRESHOLD_MASK);
}

/**
 * Hysteresis
 */
int osc_SetHysteresisChA(uint32_t hysteresis)
{
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_hystersis) mask 0x3FFF <- 0x%X", hysteresis);
    return cmn_SetValue(&osc_reg->cha_hystersis, hysteresis, HYSTERESIS_MASK, &currentValue);
}

int osc_GetHysteresisChA(uint32_t* hysteresis)
{
    return cmn_GetValue(&osc_reg->cha_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetHysteresisChB(uint32_t hysteresis)
{
    uint32_t currentValue = 0;
    return cmn_SetValue(&osc_reg->chb_hystersis, hysteresis, HYSTERESIS_MASK,&currentValue);
}

int osc_GetHysteresisChB(uint32_t* hysteresis)
{
    return cmn_GetValue(&osc_reg->chb_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetHysteresisChC(uint32_t hysteresis)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_hystersis) mask 0x3FFF <- 0x%X", hysteresis);
    return cmn_SetValue(&osc_reg_4ch->cha_hystersis, hysteresis, HYSTERESIS_MASK, &currentValue);
}

int osc_GetHysteresisChC(uint32_t* hysteresis)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_hystersis, hysteresis, HYSTERESIS_MASK);
}

int osc_SetHysteresisChD(uint32_t hysteresis)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_hystersis) mask 0x3FFF <- 0x%X", hysteresis);
    return cmn_SetValue(&osc_reg_4ch->chb_hystersis, hysteresis, HYSTERESIS_MASK, &currentValue);
}

int osc_GetHysteresisChD(uint32_t* hysteresis)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_hystersis, hysteresis, HYSTERESIS_MASK);
}

/**
 * Equalization filters
 */
int osc_SetEqFiltersChA(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp)
{
    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;

    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA,&currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER,&currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER,&currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg->cha_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER,&currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChA(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
    cmn_GetValue(&osc_reg->cha_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg->cha_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg->cha_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg->cha_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_SetEqFiltersChB(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp)
{
    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA,&currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER,&currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER,&currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg->chb_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER,&currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChB(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
    cmn_GetValue(&osc_reg->chb_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg->chb_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg->chb_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg->chb_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}


int osc_SetEqFiltersChC(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp)
{
    if (!osc_reg_4ch)
        return RP_NOTS;

    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;

    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg_4ch->cha_filt_aa, coef_aa, EQ_FILTER_AA,&currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg_4ch->cha_filt_bb, coef_bb, EQ_FILTER,&currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg_4ch->cha_filt_kk, coef_kk, EQ_FILTER,&currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg_4ch->cha_filt_pp, coef_pp, EQ_FILTER,&currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChC(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
    if (!osc_reg_4ch)
        return RP_NOTS;

    cmn_GetValue(&osc_reg_4ch->cha_filt_aa, coef_aa, EQ_FILTER_AA);
    cmn_GetValue(&osc_reg_4ch->cha_filt_bb, coef_bb, EQ_FILTER);
    cmn_GetValue(&osc_reg_4ch->cha_filt_kk, coef_kk, EQ_FILTER);
    cmn_GetValue(&osc_reg_4ch->cha_filt_pp, coef_pp, EQ_FILTER);
    return RP_OK;
}

int osc_SetEqFiltersChD(uint32_t coef_aa, uint32_t coef_bb, uint32_t coef_kk, uint32_t coef_pp)
{
    if (!osc_reg_4ch)
        return RP_NOTS;

    uint32_t currentValueAA = 0;
    uint32_t currentValueBB = 0;
    uint32_t currentValueKK = 0;
    uint32_t currentValuePP = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_aa) mask 0x3FFF <- 0x%X", coef_aa);
    cmn_SetValue(&osc_reg_4ch->chb_filt_aa, coef_aa, EQ_FILTER_AA,&currentValueAA);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_bb) mask 0x1FFFFFF <- 0x%X", coef_bb);
    cmn_SetValue(&osc_reg_4ch->chb_filt_bb, coef_bb, EQ_FILTER,&currentValueBB);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_kk) mask 0x1FFFFFF <- 0x%X", coef_kk);
    cmn_SetValue(&osc_reg_4ch->chb_filt_kk, coef_kk, EQ_FILTER,&currentValueKK);
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_filt_pp) mask 0x1FFFFFF <- 0x%X", coef_pp);
    cmn_SetValue(&osc_reg_4ch->chb_filt_pp, coef_pp, EQ_FILTER,&currentValuePP);
    return RP_OK;
}

int osc_GetEqFiltersChD(uint32_t* coef_aa, uint32_t* coef_bb, uint32_t* coef_kk, uint32_t* coef_pp)
{
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
int osc_GetWritePointer(rp_channel_t channel, uint32_t* pos)
{
    switch (channel)
    {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->wr_ptr_cur, pos, WRITE_POINTER_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->wr_ptr_cur_ch2, pos, WRITE_POINTER_MASK);
        case RP_CH_3:
            if (osc_reg_4ch){
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_cur, pos, WRITE_POINTER_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch){
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_cur_ch2, pos, WRITE_POINTER_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
            }
            break;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_GetWritePointerAtTrig(rp_channel_t channel, uint32_t* pos)
{
    switch (channel)
    {
        case RP_CH_1:
            return cmn_GetValue(&osc_reg->wr_ptr_trigger, pos, WRITE_POINTER_MASK);
        case RP_CH_2:
            return cmn_GetValue(&osc_reg->wr_ptr_trigger_ch2, pos, WRITE_POINTER_MASK);
        case RP_CH_3:
            if (osc_reg_4ch){
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_trigger, pos, WRITE_POINTER_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
            }
            break;
        case RP_CH_4:
            if (osc_reg_4ch){
                return cmn_GetValue(&osc_reg_4ch->wr_ptr_trigger_ch2, pos, WRITE_POINTER_MASK);
            }else{
                ERROR("Registers for channels 3 and 4 are not initialized")
            }
            break;
        default:
            ERROR("Wrong channel %d",channel)
            break;
    }
    return RP_EOOR;
}

int osc_SetExtTriggerDebouncer(uint32_t value){
    if (DEBAUNCER_MASK < value) {
        ERROR("Error value 0x%X very big",value)
        return RP_EIPV;
    }
    cmn_Debug("[osc_SetExtTriggerDebouncer] osc_reg.ext_trig_dbc_t <- 0x%X",value);
    osc_reg->ext_trig_dbc_t = value;

    if (osc_reg_4ch){
        cmn_Debug("[osc_SetExtTriggerDebouncer] osc_reg_4ch.ext_trig_dbc_t <- 0x%X",value);
        osc_reg_4ch->ext_trig_dbc_t = value;
    }
    return RP_OK;
}

int osc_GetExtTriggerDebouncer(uint32_t *value){
    *value = osc_reg->ext_trig_dbc_t;
    return RP_OK;
}

/**
 * Raw buffers
 */
const volatile uint32_t* osc_GetDataBufferChA()
{
    return osc_cha;
}

const volatile uint32_t* osc_GetDataBufferChB()
{
    return osc_chb;
}

const volatile uint32_t* osc_GetDataBufferChC()
{
    return osc_chc;
}

const volatile uint32_t* osc_GetDataBufferChD()
{
    return osc_chd;
}

/**
 * AXI mode
 */

int osc_axi_map(size_t size, size_t offset, void** mapped, uint32_t *mapped_size)
{
    if (offset < g_adc_axi_start) {
        return RP_EOOR;
    }

    if (offset + size > (g_adc_axi_start + g_adc_axi_size)) {
        return RP_EOOR;
    }

    if (offset % sysconf(_SC_PAGESIZE) != 0) {
        ERROR("Error size. offset %% sysconf(_SC_PAGESIZE) = %ld  must be zero. sysconf(_SC_PAGESIZE) = %ld\n",offset % sysconf(_SC_PAGESIZE),sysconf(_SC_PAGESIZE));
        return RP_EMMD;
    }

    if (mem_fd == -1) {
        mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (mem_fd < 0) {
            return RP_EOMD;
        }
    }

    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, offset);

    if (*mapped == MAP_FAILED) {
        ERROR("Error osc_axi_map: %d\n",errno);
        return RP_EMMD;
    }

    *mapped_size = size;

    return RP_OK;

}

int osc_axi_unmap(size_t size, void** mapped)
{
    if(mem_fd == -1) {
        return RP_EUMD;
    }

    if((mapped == (void *) -1) || (mapped == NULL)) {
        return RP_EUMD;
    }

    if((*mapped == (void *) -1) || (*mapped == NULL)) {
        return RP_EUMD;
    }

    if(munmap(*mapped, size) < 0){
        return RP_EUMD;
    }
    *mapped = NULL;
    return RP_OK;
}

bool checkBufferOverlap(rp_channel_t _channel,uint32_t _start,uint32_t _end) {
    int maxSize = 0;
    uint32_t start[RP_CH_4 + 1];
    uint32_t end[RP_CH_4 + 1];

    uint32_t ch_en = 0;
    int ret = 0;

    ret = cmn_GetValue(&osc_reg->cha_axi_enable, &ch_en, AXI_ENABLE_MASK);
    if (ret != RP_OK){
        return true;
    }

    if (ch_en && _channel != RP_CH_1) {
        ret = cmn_GetValue(&osc_reg->cha_axi_addr_high, &end[maxSize], FULL_MASK);
        if (ret != RP_OK) {
            return true;
        }
        ret = cmn_GetValue(&osc_reg->cha_axi_addr_low, &start[maxSize], FULL_MASK);
        if (ret != RP_OK) {
            return true;
        }
        maxSize++;
    }

    ret = cmn_GetValue(&osc_reg->chb_axi_enable, &ch_en, AXI_ENABLE_MASK);
    if (ret != RP_OK){
        return true;
    }

    if (ch_en && _channel != RP_CH_2) {
        ret = cmn_GetValue(&osc_reg->chb_axi_addr_high, &end[maxSize], FULL_MASK);
        if (ret != RP_OK) {
            return true;
        }
        ret = cmn_GetValue(&osc_reg->chb_axi_addr_low, &start[maxSize], FULL_MASK);
        if (ret != RP_OK) {
            return true;
        }
        maxSize++;
    }

    if (osc_reg_4ch){
        ret = cmn_GetValue(&osc_reg_4ch->cha_axi_enable, &ch_en, AXI_ENABLE_MASK);
        if (ret != RP_OK){
            return true;
        }

        if (ch_en && _channel != RP_CH_3) {
            ret = cmn_GetValue(&osc_reg_4ch->cha_axi_addr_high, &end[maxSize], FULL_MASK);
            if (ret != RP_OK) {
                return true;
            }
            ret = cmn_GetValue(&osc_reg_4ch->cha_axi_addr_low, &start[maxSize], FULL_MASK);
            if (ret != RP_OK) {
                return true;
            }
            maxSize++;
        }

        ret = cmn_GetValue(&osc_reg_4ch->chb_axi_enable, &ch_en, AXI_ENABLE_MASK);
        if (ret != RP_OK){
            return true;
        }

        if (ch_en && _channel != RP_CH_4) {
            ret = cmn_GetValue(&osc_reg_4ch->chb_axi_addr_high, &end[maxSize], FULL_MASK);
            if (ret != RP_OK) {
                return true;
            }
            ret = cmn_GetValue(&osc_reg_4ch->chb_axi_addr_low, &start[maxSize], FULL_MASK);
            if (ret != RP_OK) {
                return true;
            }
            maxSize++;
        }
    }
    bool overlaped = false;
    for(int i = 0 ; i < maxSize; i++) {
        if (start[i] <= _start && _start <= end[i]){
            overlaped |= true;
        }
        if (start[i] <= _end && _end <= end[i]){
            overlaped |= true;
        }
        if (_start <= start[i] && end[i] <= _end){
            overlaped |= true;
        }
    }
    return overlaped;
}

int osc_axi_EnableChA(bool enable)
{
    int ret = 0;
    uint32_t tmp;
    uint32_t ch_addr_high, ch_addr_low;
    ret = cmn_GetValue(&osc_reg->cha_axi_addr_high, &ch_addr_high, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }
    ret = cmn_GetValue(&osc_reg->cha_axi_addr_low, &ch_addr_low, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }

    if (enable)
    {
        if (ch_addr_low > ch_addr_high)
        {
            return RP_EOOR;
        }
        if (ch_addr_low < g_adc_axi_start)
        {
            return RP_EOOR;
        }
        if (ch_addr_high > (g_adc_axi_start + g_adc_axi_size))
        {
            return RP_EOOR;
        }

        if (checkBufferOverlap(RP_CH_1,ch_addr_low,ch_addr_high)){
            return RP_EOOR;
        }

        if (osc_axi_cha != NULL){
            ret = osc_axi_unmap(osc_axi_cha_size, (void**)&osc_axi_cha);
            if (ret != RP_OK)
            {
                return ret;
            }
        }

        ret = osc_axi_map(ch_addr_high - ch_addr_low, ch_addr_low, (void**)&osc_axi_cha, &osc_axi_cha_size);
        if (ret != RP_OK)
        {
            return ret;
        }
        cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg->cha_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    }

    ret = osc_axi_unmap(osc_axi_cha_size, (void**)&osc_axi_cha);
    if (ret != RP_OK)
    {
        cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_enable) mask 0x1 <- 0x%X", 0);
        cmn_SetValue(&osc_reg->cha_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
        return ret;
    }
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg->cha_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_EnableChB(bool enable)
{
    int ret = 0;
    uint32_t tmp;
    uint32_t ch_addr_high, ch_addr_low;
    ret = cmn_GetValue(&osc_reg->chb_axi_addr_high, &ch_addr_high, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }
    ret = cmn_GetValue(&osc_reg->chb_axi_addr_low, &ch_addr_low, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }

    if (enable)
    {
        if (ch_addr_low > ch_addr_high)
        {
            return RP_EOOR;
        }
        if (ch_addr_low < g_adc_axi_start)
        {
            return RP_EOOR;
        }
        if (ch_addr_high > (g_adc_axi_start + g_adc_axi_size))
        {
            return RP_EOOR;
        }

        if (checkBufferOverlap(RP_CH_2,ch_addr_low,ch_addr_high)){
            return RP_EOOR;
        }

        if (osc_axi_chb != NULL){
            ret = osc_axi_unmap(osc_axi_chb_size, (void**)&osc_axi_chb);
            if (ret != RP_OK)
            {
                return ret;
            }
        }

        ret = osc_axi_map(ch_addr_high - ch_addr_low, ch_addr_low, (void**)&osc_axi_chb,&osc_axi_chb_size);
        if (ret != RP_OK)
        {
            return ret;
        }
        cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg->chb_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    }
    ret = osc_axi_unmap(osc_axi_chb_size, (void**)&osc_axi_chb);
    if (ret != RP_OK)
    {
        cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_enable) mask 0x1 <- 0x%X", 0);
        cmn_SetValue(&osc_reg->chb_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
        return ret;
    }
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg->chb_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_EnableChC(bool enable)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    int ret = 0;
    uint32_t tmp;
    uint32_t ch_addr_high, ch_addr_low;
    ret = cmn_GetValue(&osc_reg_4ch->cha_axi_addr_high, &ch_addr_high, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }
    ret = cmn_GetValue(&osc_reg_4ch->cha_axi_addr_low, &ch_addr_low, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }

    if (enable)
    {
        if (ch_addr_low > ch_addr_high)
        {
            return RP_EOOR;
        }
        if (ch_addr_low < g_adc_axi_start)
        {
            return RP_EOOR;
        }
        if (ch_addr_high > (g_adc_axi_start + g_adc_axi_size))
        {
            return RP_EOOR;
        }

        if (checkBufferOverlap(RP_CH_3,ch_addr_low,ch_addr_high)){
            return RP_EOOR;
        }

        if (osc_axi_chc != NULL){
            ret = osc_axi_unmap(osc_axi_chc_size, (void**)&osc_axi_chc);
            if (ret != RP_OK)
            {
                return ret;
            }
        }

        ret = osc_axi_map(ch_addr_high - ch_addr_low, ch_addr_low, (void**)&osc_axi_chc,&osc_axi_chc_size);
        if (ret != RP_OK)
        {
            return ret;
        }
        cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg_4ch->cha_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    }
    ret = osc_axi_unmap(osc_axi_chc_size, (void**)&osc_axi_chc);
    if (ret != RP_OK)
    {
        cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_enable) mask 0x1 <- 0x%X", 0);
        cmn_SetValue(&osc_reg_4ch->cha_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
        return ret;
    }
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_EnableChD(bool enable)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    int ret = 0;
    uint32_t tmp;
    uint32_t ch_addr_high, ch_addr_low;
    ret = cmn_GetValue(&osc_reg_4ch->chb_axi_addr_high, &ch_addr_high, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }
    ret = cmn_GetValue(&osc_reg_4ch->chb_axi_addr_low, &ch_addr_low, FULL_MASK);
    if (ret != RP_OK) {
        return ret;
    }

    if (enable)
    {
        if (ch_addr_low > ch_addr_high)
        {
            return RP_EOOR;
        }
        if (ch_addr_low < g_adc_axi_start)
        {
            return RP_EOOR;
        }
        if (ch_addr_high > (g_adc_axi_start + g_adc_axi_size))
        {
            return RP_EOOR;
        }

        if (checkBufferOverlap(RP_CH_4,ch_addr_low,ch_addr_high)){
            return RP_EOOR;
        }

        if (osc_axi_chd != NULL){
            ret = osc_axi_unmap(osc_axi_chd_size, (void**)&osc_axi_chd);
            if (ret != RP_OK)
            {
                return ret;
            }
        }

        ret = osc_axi_map(ch_addr_high - ch_addr_low, ch_addr_low, (void**)&osc_axi_chd,&osc_axi_chd_size);
        if (ret != RP_OK)
        {
            return ret;
        }
        cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_enable) mask 0x1 <- 0x%X", 1);
        return cmn_SetValue(&osc_reg_4ch->chb_axi_enable, 1, AXI_ENABLE_MASK, &tmp);
    }
    ret = osc_axi_unmap(osc_axi_chd_size, (void**)&osc_axi_chd);
    if (ret != RP_OK)
    {
        cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_enable) mask 0x1 <- 0x%X", 0);
        cmn_SetValue(&osc_reg_4ch->chb_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
        return ret;
    }
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_enable) mask 0x1 <- 0x%X", 0);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_enable, 0, AXI_ENABLE_MASK, &tmp);
}

int osc_axi_SetAddressStartChA(uint32_t address)
{
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->cha_axi_addr_low, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressStartChB(uint32_t address)
{
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->chb_axi_addr_low, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressStartChC(uint32_t address)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_addr_low, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressStartChD(uint32_t address)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_addr_low) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_addr_low, address, FULL_MASK, &tmp);
}


int osc_axi_SetAddressEndChA(uint32_t address)
{
    address -= RESERV_DMA_BYTES;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->cha_axi_addr_high, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressEndChB(uint32_t address)
{
    address -= RESERV_DMA_BYTES;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg->chb_axi_addr_high, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressEndChC(uint32_t address)
{
    address -= RESERV_DMA_BYTES;
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_addr_high, address, FULL_MASK, &tmp);
}

int osc_axi_SetAddressEndChD(uint32_t address)
{
    address -= RESERV_DMA_BYTES;
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t tmp;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_addr_high) mask 0xFFFFFFFF <- 0x%X", address);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_addr_high, address, FULL_MASK, &tmp);
}


int osc_axi_GetAddressStartChA(uint32_t *address){
    return cmn_GetValue(&osc_reg->cha_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressStartChB(uint32_t *address){
    return cmn_GetValue(&osc_reg->chb_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressStartChC(uint32_t *address){
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressStartChD(uint32_t *address){
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_axi_addr_low, address, FULL_MASK);
}

int osc_axi_GetAddressEndChA(uint32_t *address){
    int ret = cmn_GetValue(&osc_reg->cha_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetAddressEndChB(uint32_t *address){
    int ret = cmn_GetValue(&osc_reg->chb_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetAddressEndChC(uint32_t *address){
    if (!osc_reg_4ch)
        return RP_NOTS;
    int ret = cmn_GetValue(&osc_reg_4ch->cha_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetAddressEndChD(uint32_t *address){
    if (!osc_reg_4ch)
        return RP_NOTS;
    int ret = cmn_GetValue(&osc_reg_4ch->chb_axi_addr_high, address, FULL_MASK);
    *address += RESERV_DMA_BYTES;
    return ret;
}

int osc_axi_GetWritePointerChA(uint32_t* pos)
{
    uint32_t addr;
    osc_axi_GetAddressStartChA(&addr);
    cmn_GetValue(&osc_reg->cha_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerChB(uint32_t* pos)
{
    uint32_t addr;
    osc_axi_GetAddressStartChB(&addr);
    cmn_GetValue(&osc_reg->chb_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos  - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerChC(uint32_t* pos)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChC(&addr);
    cmn_GetValue(&osc_reg_4ch->cha_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerChD(uint32_t* pos)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChD(&addr);
    cmn_GetValue(&osc_reg_4ch->chb_axi_wr_ptr_cur, pos, FULL_MASK);
    *pos = (*pos  - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChA(uint32_t* pos)
{
    uint32_t addr;
    osc_axi_GetAddressStartChA(&addr);
    cmn_GetValue(&osc_reg->cha_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChB(uint32_t* pos)
{
    uint32_t addr;
    osc_axi_GetAddressStartChB(&addr);
    cmn_GetValue(&osc_reg->chb_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChC(uint32_t* pos)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChC(&addr);
    cmn_GetValue(&osc_reg_4ch->cha_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_GetWritePointerAtTrigChD(uint32_t* pos)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t addr;
    osc_axi_GetAddressStartChD(&addr);
    cmn_GetValue(&osc_reg_4ch->chb_axi_wr_ptr_trigger, pos, FULL_MASK);
    *pos = (*pos - addr) / 2;
    return RP_OK;
}

int osc_axi_SetTriggerDelayChA(uint32_t decimated_data_num)
{
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->cha_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_SetTriggerDelayChB(uint32_t decimated_data_num)
{
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg->chb_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_SetTriggerDelayChC(uint32_t decimated_data_num)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->cha_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg_4ch->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_SetTriggerDelayChD(uint32_t decimated_data_num)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    uint32_t currentValue = 0;
    cmn_Debug("cmn_SetValue(&osc_reg_4ch->chb_axi_delay) mask 0xFFFFFFFF <- 0x%X", decimated_data_num);
    return cmn_SetValue(&osc_reg_4ch->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK, &currentValue);
}

int osc_axi_GetTriggerDelayChA(uint32_t* decimated_data_num)
{
    return cmn_GetValue(&osc_reg->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

int osc_axi_GetTriggerDelayChB(uint32_t* decimated_data_num)
{
    return cmn_GetValue(&osc_reg->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

int osc_axi_GetTriggerDelayChC(uint32_t* decimated_data_num)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->cha_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

int osc_axi_GetTriggerDelayChD(uint32_t* decimated_data_num)
{
    if (!osc_reg_4ch)
        return RP_NOTS;
    return cmn_GetValue(&osc_reg_4ch->chb_axi_delay, decimated_data_num, TRIG_DELAY_MASK);
}

const volatile uint16_t* osc_axi_GetDataBufferChA()
{
    return osc_axi_cha;
}

const volatile uint16_t* osc_axi_GetDataBufferChB()
{
    return osc_axi_chb;
}

const volatile uint16_t* osc_axi_GetDataBufferChC()
{
    return osc_axi_chc;
}

const volatile uint16_t* osc_axi_GetDataBufferChD()
{
    return osc_axi_chd;
}
