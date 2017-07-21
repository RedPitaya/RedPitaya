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

//// These structure is a direct API mirror and should not be altered!
//const scpi_choice_def_t scpi_RpGenTrig[] = {
//    {"INT",     1},
//    {"EXT_PE",  2},
//    {"EXT_NE",  3},
//    {"GATED",   4},
//    SCPI_CHOICE_LIST_END
//};

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
        rp->gen_waveform_tag[i] = 0;
        rp->gen_waveform_opt[i] = 0.5;
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
static scpi_bool_t rpcspi_gen_channels(scpi_t *context, int unsigned *channel) {
    int32_t num[1];
    rpscpi_context_t *rp = (rpscpi_context_t *) context->user_context;

    SCPI_CommandNumbers(context, num, 1, RPSCPI_CMD_NUM);
    if (!((num[0] > 0) && (num[0] <= rp->gen_num))) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return FALSE;
    }
    *channel = num[0] - 1;
    return TRUE;
}

scpi_result_t rpscpi_gen_reset(scpi_t *context) {
    int unsigned channel;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    rp_evn_reset(&gen[channel].evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_start(scpi_t *context) {
    int unsigned channel;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    rp_evn_start(&gen[channel].evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_stop(scpi_t *context) {
    int unsigned channel;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    rp_evn_stop(&gen[channel].evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_trigger(scpi_t *context) {
    int unsigned channel;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    rp_evn_trigger(&gen[channel].evn);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_mode(scpi_t *context) {
    int unsigned channel;
    int32_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if(!SCPI_ParamChoice(context, rpscpi_gen_modes, &value, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    rp_gen_set_mode(&gen[channel], value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_mode(scpi_t *context) {
    int unsigned channel;
    int32_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = (rp_gen_get_mode(&gen[channel]) >> 0) & 0x1;
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_gen_modes, value, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_burst_mode(scpi_t *context) {
    int unsigned channel;
    int32_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if(!SCPI_ParamChoice(context, rpscpi_gen_burst_modes, &value, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    value = (value << 1) | (rp_gen_get_mode(&gen[channel]) & 0x1);
    rp_gen_set_mode(&gen[channel], value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_burst_mode(scpi_t *context) {
    int unsigned channel;
    int32_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = (rp_gen_get_mode(&gen[channel]) >> 1) & 0x1;
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_gen_burst_modes, value, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_enable(scpi_t *context) {
    int unsigned channel;
    bool value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if(!SCPI_ParamBool(context, &value, true)){
        return SCPI_RES_ERR;
    }
    rp_gen_out_set_enable(&gen[channel].out, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_enable(scpi_t *context) {
    int unsigned channel;
    bool value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_gen_out_get_enable(&gen[channel].out);
    SCPI_ResultBool(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_frequency(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_gen_set_frequency(&gen[channel].per, value.content.value) != 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_frequency(scpi_t *context) {
    int unsigned channel;
    double value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_asg_gen_get_frequency(&gen[channel].per);
    SCPI_ResultDouble(context, value);
    return SCPI_RES_OK;
}    

scpi_result_t rpscpi_gen_set_phase(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;
    
    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_gen_set_phase(&gen[channel].per, value.content.value) != 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_phase(scpi_t *context) {
    int unsigned channel;
    double value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;
    
    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_asg_gen_get_phase(&gen[channel].per);
    SCPI_ResultDouble(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_amplitude(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_gen_out_set_amplitude(&gen[channel].out, (float) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_amplitude(scpi_t *context) {
    int unsigned channel;
    float value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_gen_out_get_amplitude(&gen[channel].out);
    SCPI_ResultFloat(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_offset(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_gen_out_set_offset(&gen[channel].out, (float) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_offset(scpi_t *context) {
    int unsigned channel;
    float value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_gen_out_get_offset(&gen[channel].out);
    SCPI_ResultFloat(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_waveform_shape(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    int32_t *waveform_tag;
    float   *waveform_opt;
    rpscpi_context_t *user_context = (rpscpi_context_t *) context->user_context;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    waveform_tag = &user_context->gen_waveform_tag[channel];
    waveform_opt = &user_context->gen_waveform_opt[channel];
    // read waveform tag
    if(!SCPI_ParamChoice(context, rpscpi_gen_waveform_names, waveform_tag, true)){
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
    // read optional waveform parameter
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, false)) {
        // default in case parameter is not given
        *waveform_opt = 0.5;
    } else {
        if (value.special) {
            // special values are not allowed
            // TODO add support for MAX, MIN, MID?
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return SCPI_RES_ERR;
        } else {
            // TODO add support for percentile values
            if ( (0 <= value.content.value) && (value.content.value <= 1) ) {
                *waveform_opt = value.content.value;
            } else {
                SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
                return SCPI_RES_ERR;
            }
        }
    }
    // construct the waveform
    float waveform[gen[channel].buffer_size];
    switch(*waveform_tag) {
        case 0:
            rp_wave_sin (waveform, gen[channel].buffer_size);
            break;
        case 1:
            rp_wave_squ (waveform, gen[channel].buffer_size, *waveform_opt); // TODO 
            break;
        case 2:
            rp_wave_tri (waveform, gen[channel].buffer_size, *waveform_opt); // TODO
            break;
        default:
            break;
    }
    // write waveform into buffer
    if(rp_gen_set_waveform(&gen[channel], waveform, gen[channel].buffer_size)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    // set waveform size
    rp_asg_per_set_table_size(&gen[channel].per, gen[channel].buffer_size);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_waveform_shape(scpi_t *context) {
    int unsigned channel;
    rpscpi_context_t *user_context = (rpscpi_context_t *) context->user_context;
//    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;
//    float   *waveform     =  user_context->gen_waveform;
    int32_t *waveform_tag;
    float   *waveform_opt;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    // send waveform tag
    waveform_tag = &user_context->gen_waveform_tag[channel];
    const char * text;
    if(!SCPI_ChoiceToName(rpscpi_gen_waveform_names, *waveform_tag, &text)){
        return SCPI_RES_ERR;
    }
    SCPI_ResultMnemonic(context, text);
    // send waveform tag
    waveform_opt = &user_context->gen_waveform_opt[channel];
    SCPI_ResultFloat(context, *waveform_opt);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_waveform_data(scpi_t *context) {
    int unsigned channel;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    // store float values into a temporary waveform
    float waveform[gen[channel].buffer_size];
    size_t len=0;
    scpi_result_t status;
    do {
        float sample;
        status = SCPI_ParamFloat(context, &sample, false);
        if (status == SCPI_RES_OK) {
           // check if too many parameters were received
           if (len == gen[channel].buffer_size) {
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
    if(rp_gen_set_waveform(&gen[channel], waveform, len)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    // set waveform size
    rp_asg_per_set_table_size(&gen[channel].per, len);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_waveform_data(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    // read data length
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    int unsigned len = (int unsigned) value.content.value;
    // load float values into a temporary waveform
    float waveform[gen[channel].buffer_size];
    if (rp_gen_get_waveform(&gen[channel], waveform, &len)) {
        // length was out of range
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    SCPI_ResultArrayFloat(context, waveform, len, SCPI_FORMAT_ASCII);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_data_repetitions(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_data_repetitions(&gen[channel].bst, (int) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_data_repetitions(scpi_t *context) {
    int unsigned channel;
    float value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_asg_bst_get_data_repetitions(&gen[channel].bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_data_length(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_data_length(&gen[channel].bst, (int) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_data_length(scpi_t *context) {
    int unsigned channel;
    float value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_asg_bst_get_data_length(&gen[channel].bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_period_length(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_period_length(&gen[channel].bst, (int) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_period_length(scpi_t *context) {
    int unsigned channel;
    float value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_asg_bst_get_period_length(&gen[channel].bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_set_period_number(scpi_t *context) {
    int unsigned channel;
    scpi_number_t value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (value.special) {
        // special values are not allowed
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    } else if (rp_asg_bst_set_period_number(&gen[channel].bst, (int) value.content.value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_gen_get_period_number(scpi_t *context) {
    int unsigned channel;
    float value;
    rp_gen_t *gen = ((rpscpi_context_t *) context->user_context)->gen;

    if (!rpcspi_gen_channels(context, &channel)) {
        return SCPI_RES_ERR;
    }
    value = rp_asg_bst_get_period_number(&gen[channel].bst);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

//scpi_result_t RP_GenTriggerSource(scpi_t *context) {
//        
//    rp_channel_t channel;
//    int32_t trig_choice;
//    int result;
//
//    if (!rpcspi_gen_channels(context, &channel)) {
//        return SCPI_RES_ERR;
//    }
//
//    if(!SCPI_ParamChoice(context, scpi_RpGenTrig, &trig_choice, true)){
//        RP_LOG(LOG_ERR, "*SOUR#:TRIG:SOUR Failed to parse first parameter.\n");
//        return SCPI_RES_ERR;
//    }
//
//    rp_trig_src_t trig_src = trig_choice;
//    result = rp_GenTriggerSource(channel, trig_src);
//    if(result != RP_OK){
//        RP_LOG(LOG_ERR, "*SOUR#:TRIG:SOUR Failed to set generate"
//        " trigger source: %s\n", rp_GetError(result));
//
//        return SCPI_RES_ERR;
//    }
//
//    RP_LOG(LOG_INFO, "*SOUR#:TRIG:SOUR Successfully set generate trigger source.\n");
//    return SCPI_RES_OK;
//}
//
//scpi_result_t RP_GenTriggerSourceQ(scpi_t *context) {
//    
//    rp_channel_t channel;
//    const char *trig_name;
//
//    if (!rpcspi_gen_channels(context, &channel)) {
//        return SCPI_RES_ERR;
//    }
//
//    rp_trig_src_t trig_src;
//    if (rp_GenGetTriggerSource(channel, &trig_src) != RP_OK){
//        return SCPI_RES_ERR;
//    }
//
//    int32_t trig_n = trig_src;
//
//    if(!SCPI_ChoiceToName(scpi_RpGenTrig, trig_n, &trig_name)){
//        RP_LOG(LOG_ERR, "*SOUR#:TRIG:SOUR? Failed to parse trigger name.\n");
//        return SCPI_RES_ERR;
//    }
//
//    SCPI_ResultMnemonic(context, trig_name);
//
//    RP_LOG(LOG_INFO, "*SOUR#:TRIG:SOUR? Successfully returend"
//    " generate trigger status to client.\n");
//
//    return SCPI_RES_OK;
//}
//
//scpi_result_t RP_GenTrigger(scpi_t *context) {
//    
//    rp_channel_t channel;
//    int result;
//
//    if (!rpcspi_gen_channels(context, &channel)) {
//        return SCPI_RES_ERR;
//    }
//
//    result = rp_GenTrigger(channel);
//    if(result != RP_OK){
//        RP_LOG(LOG_ERR, "*SOUR#:TRIG:IMM Failed to set immediate "
//            "trigger: %s\n", rp_GetError(result));
//        return SCPI_RES_ERR;
//    }
//
//    RP_LOG(LOG_INFO, "*SOUR#:TRIG:IMM Successfully set immediate trigger.\n");
//    return SCPI_RES_OK;
//}
