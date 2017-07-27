/**
 * @brief Red Pitaya SCPI server, generator commands implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "scpi_gen.h"
#include "redpitaya/rp1.h"

#include "common.h"
#include "scpi/parser.h"
#include "scpi/units.h"
#include "scpi/error.h"

// TODO: move into a common area
const scpi_choice_def_t rpscpi_evn_sources[] = {
    {"GEN0", 0},
    {"GEN1", 1},
    {"OSC0", 2},
    {"OSC1", 3},
    {"LG",   4},
    {"LA",   5},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t rpscpi_gen_waveform_names[] = {
    {"SINusoid", 0},
    {"SQUare",   1},
    {"TRIangle", 2},
    {"USER",     7},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t rpscpi_gen_modes[] = {
    {"PERiodic", 0},
    {"BURSt",    1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t rpscpi_gen_burst_modes[] = {
    {"FINite",   0},
    {"INFinite", 1},
    SCPI_CHOICE_LIST_END
};

/**
 * initialization of context and API
 */
scpi_result_t rpscpi_gen_init(rpscpi_context_t *rp, int unsigned channels) {
    // specify number of channels
    rp->gen_num = channels;
    // allocate memory for channels
    rp->gen = malloc(rp->gen_num * sizeof(rp_gen_t));
    if (!rp->gen) {
        syslog(LOG_ERR, "Failed to allocate memory for rp_gen_t pointer.");
        return(SCPI_RES_ERR);
    }
    // run API initialization
    for (int unsigned i=0; i<rp->gen_num; i++) {
        if (rp_gen_init(&rp->gen[i], i) !=0) {
            syslog(LOG_ERR, "Failed to initialize generator %u.", i);
            return(SCPI_RES_ERR);
        }
    }
    // initialize context
    for (int unsigned i=0; i<rp->gen_num; i++) {
        rp->gen[i].tag = 0;
        rp->gen[i].opt = 0.5;
    }
    return(SCPI_RES_OK);
}

/**
 * release of context and API
 */
scpi_result_t rpscpi_gen_release(rpscpi_context_t *rp) {
    for (int unsigned i=0; i<rp->gen_num; i++) {
        if (rp_gen_release(&rp->gen[i]) !=0) {
            syslog(LOG_ERR, "Failed to release generator %u.", i);
            return (SCPI_RES_OK);
        }
    }
    free(rp->gen);
    return(SCPI_RES_OK);
}

/**
 * helper function for detecting selected channel
 */
static scpi_bool_t rpcspi_gen_channels(scpi_t *context, rp_gen_t *gen) {
    
    int32_t num[1];
    rpscpi_context_t *rp = (rpscpi_context_t *) context->user_context;

    SCPI_CommandNumbers(context, num, 1, RPSCPI_CMD_NUM);
    if (!((num[0] > 0) && (num[0] <= rp->gen_num))) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return FALSE;
    }
    *gen = ((rpscpi_context_t *) context->user_context)->gen[num[0] - 1];
    return TRUE;
}

scpi_result_t rpscpi_gen_reset(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    rp_evn_reset(&gen.evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_start(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    rp_evn_start(&gen.evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_stop(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    rp_evn_stop(&gen.evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_trigger(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    rp_evn_trigger(&gen.evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_status_run(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    bool value = rp_evn_status_run(&gen.evn);
    SCPI_ResultBool(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_status_trigger(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    bool value = rp_evn_status_trigger(&gen.evn);
    SCPI_ResultBool(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_sync_src(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value;
    if(!SCPI_ParamChoice(context, rpscpi_evn_sources, &value, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    rp_evn_set_sync_src(&gen.evn, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_sync_src(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value = (rp_evn_get_sync_src(&gen.evn) >> 0) & 0x1;
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_evn_sources, value, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_trig_src(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value;
    if(!SCPI_ParamChoice(context, rpscpi_evn_sources, &value, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    rp_evn_set_trig_src(&gen.evn, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_trig_src(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value = (rp_evn_get_trig_src(&gen.evn) >> 0) & 0x1;
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_evn_sources, value, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_mode(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value;
    if(!SCPI_ParamChoice(context, rpscpi_gen_modes, &value, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    rp_gen_set_mode(&gen, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_mode(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value = (rp_gen_get_mode(&gen) >> 0) & 0x1;
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_gen_modes, value, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_burst_mode(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value;
    if(!SCPI_ParamChoice(context, rpscpi_gen_burst_modes, &value, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    value = (value << 1) | (rp_gen_get_mode(&gen) & 0x1);
    rp_gen_set_mode(&gen, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_burst_mode(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    int32_t value = (rp_gen_get_mode(&gen) >> 1) & 0x1;
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_gen_burst_modes, value, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_enable(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    bool value;
    if(!SCPI_ParamBool(context, &value, true)){
        return SCPI_RES_ERR;
    }
    rp_gen_out_set_enable(&gen.out, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_enable(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    bool value = rp_gen_out_get_enable(&gen.out);
    SCPI_ResultBool(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_frequency(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_gen_set_frequency(&gen.per, value.content.value) != 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_frequency(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    double value = rp_asg_gen_get_frequency(&gen.per);
    SCPI_ResultDouble(context, value);
    return SCPI_RES_OK;
}    

scpi_result_t rpscpi_gen_set_phase(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_gen_set_phase(&gen.per, value.content.value) != 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_phase(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    double value;
    value = rp_asg_gen_get_phase(&gen.per);
    SCPI_ResultDouble(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_amplitude(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_gen_out_set_amplitude(&gen.out, (float) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_amplitude(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    float value = rp_gen_out_get_amplitude(&gen.out);
    SCPI_ResultFloat(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_offset(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_gen_out_set_offset(&gen.out, (float) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_offset(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    float value = rp_gen_out_get_offset(&gen.out);
    SCPI_ResultFloat(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_waveform_shape(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    // read waveform tag
    if(!SCPI_ParamChoice(context, rpscpi_gen_waveform_names, &gen.tag, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    // read optional waveform parameter
    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, false)) {
        // default in case parameter is not given
        gen.opt = 0.5;
    } else {
        if (value.special) {
            // special values are not allowed
            // TODO add support for MAX, MIN, MID?
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return SCPI_RES_ERR;
        } else {
            // TODO add support for percentile values
            if ( (0 <= value.content.value) && (value.content.value <= 1) ) {
                gen.opt = value.content.value;
            } else {
                SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
                return SCPI_RES_ERR;
            }
        }
    }
    // construct the waveform
    float waveform[gen.buffer_size];
    switch(gen.tag) {
        case 0:
            rp_wave_sin (waveform, gen.buffer_size);
            break;
        case 1:
            rp_wave_squ (waveform, gen.buffer_size, gen.opt); // TODO 
            break;
        case 2:
            rp_wave_tri (waveform, gen.buffer_size, gen.opt); // TODO
            break;
        default:
            break;
    }
    // write waveform into buffer
    if(rp_gen_set_waveform(&gen, waveform, gen.buffer_size)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    // set waveform size
    rp_asg_per_set_table_size(&gen.per, gen.buffer_size);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_waveform_shape(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    // send waveform tag
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_gen_waveform_names, gen.tag, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    // send waveform option
    SCPI_ResultFloat(context, gen.opt);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_waveform_data(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    // store float values into a temporary waveform
    float waveform[gen.buffer_size];
    size_t len=0;
    scpi_result_t status;
    do {
        float sample;
        status = SCPI_ParamFloat(context, &sample, false);
        if (status == SCPI_RES_OK) {
           // check if too many parameters were received
           if (len == gen.buffer_size) {
               SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
               return SCPI_RES_ERR;
           }
           waveform[len] = sample;
           len++;
        } else {
           // there are no new samples, so break the loop
           break;
        }
    } while (1);
    // write waveform into buffer
    if(rp_gen_set_waveform(&gen, waveform, len)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    // set waveform size
    rp_asg_per_set_table_size(&gen.per, len);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_waveform_data(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    // read data length
    int unsigned len;
    if (!SCPI_ParamUInt32(context, &len, false)) {
        len = gen.buffer_size;
    }
    // load float values into a temporary waveform
    float waveform[gen.buffer_size];
    if (rp_gen_get_waveform(&gen, waveform, &len)) {
        // length was out of range
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    SCPI_ResultArrayFloat(context, waveform, len, SCPI_FORMAT_ASCII);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_waveform_raw(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    // store float values into a temporary waveform
    int16_t *waveform;
    size_t len=0;
    if (!SCPI_ParamArbitraryBlock(context, (const char **) &waveform, &len, true)) {
        return SCPI_RES_ERR;
    }
    // check block data size proper allignement
    if (len % sizeof(*waveform)) {
        SCPI_ErrorPush(context, SCPI_ERROR_INVALID_BLOCK_DATA);
        return SCPI_RES_ERR;
    }
    // calculate number of samples from block size
    len = len / sizeof(*waveform);
    // check if data fits inside buffer
    if (len > gen.buffer_size) {
        SCPI_ErrorPush(context, SCPI_ERROR_BLOCK_DATA_ERROR);
        return SCPI_RES_ERR;
    }
    // write waveform into buffer
    for (size_t i=0; i<len; i++) {
	 gen.table[i] = waveform[i];
    }
    // set waveform size
    rp_asg_per_set_table_size(&gen.per, len);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_waveform_raw(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    // read data length
    int unsigned len;
    if (!SCPI_ParamUInt32(context, &len, false)) {
        len = gen.buffer_size;
    }
    // store float values into a temporary waveform
    int16_t waveform[len];
    // write waveform into buffer
    for (size_t i=0; i<len; i++) {
	 waveform[i] = gen.table[i];
    }
    // send back the data
    if (!SCPI_ResultArbitraryBlock(context, &waveform, len * sizeof(*waveform))) {
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_data_repetitions(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_data_repetitions(&gen.bst, (uint32_t) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_data_repetitions(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_asg_bst_get_data_repetitions(&gen.bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_data_length(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_data_length(&gen.bst, (uint32_t) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_data_length(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_asg_bst_get_data_length(&gen.bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_period_length(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_period_length(&gen.bst, (uint64_t) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_period_length(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_asg_bst_get_period_length(&gen.bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_period_number(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    scpi_number_t value;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_period_number(&gen.bst, (uint32_t) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_period_number(scpi_t *context) {
    rp_gen_t gen;
    if (!rpcspi_gen_channels(context, &gen)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_asg_bst_get_period_number(&gen.bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

