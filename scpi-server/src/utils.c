	/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"


/**
 * Converts from String into rp_dpin_t...
 * @param pinStr
 * @param rpPin
 * @return
 */
int getRpDpin(const char *pinStr, rp_dpin_t *rpPin) {
	if(strcasecmp(pinStr, "LED0") == 0) {
		*rpPin = RP_LED0;
	}
	else if(strcasecmp(pinStr, "LED1") == 0) {
		*rpPin = RP_LED1;
	}
	else if(strcasecmp(pinStr, "LED2") == 0) {
		*rpPin = RP_LED2;
	}
	else if(strcasecmp(pinStr, "LED3") == 0) {
		*rpPin = RP_LED3;
	}
	else if(strcasecmp(pinStr, "LED4") == 0) {
		*rpPin = RP_LED4;
	}
	else if(strcasecmp(pinStr, "LED5") == 0) {
		*rpPin = RP_LED5;
	}
	else if(strcasecmp(pinStr, "LED6") == 0) {
		*rpPin = RP_LED6;
	}
	else if(strcasecmp(pinStr, "LED7") == 0) {
		*rpPin = RP_LED7;
	}

	else if(strcasecmp(pinStr, "DIO0_P") == 0) {
		*rpPin = RP_DIO0_P;
	}
	else if(strcasecmp(pinStr, "DIO1_P") == 0) {
		*rpPin = RP_DIO1_P;
	}
	else if(strcasecmp(pinStr, "DIO2_P") == 0) {
		*rpPin = RP_DIO2_P;
	}
	else if(strcasecmp(pinStr, "DIO3_P") == 0) {
		*rpPin = RP_DIO3_P;
	}
	else if(strcasecmp(pinStr, "DIO4_P") == 0) {
		*rpPin = RP_DIO4_P;
	}
	else if(strcasecmp(pinStr, "DIO5_P") == 0) {
		*rpPin = RP_DIO5_P;
	}
	else if(strcasecmp(pinStr, "DIO6_P") == 0) {
		*rpPin = RP_DIO6_P;
	}
	else if(strcasecmp(pinStr, "DIO7_P") == 0) {
		*rpPin = RP_DIO7_P;
	}

	else if(strcasecmp(pinStr, "DIO0_N") == 0) {
		*rpPin = RP_DIO0_N;
	}
	else if(strcasecmp(pinStr, "DIO1_N") == 0) {
		*rpPin = RP_DIO1_N;
	}
	else if(strcasecmp(pinStr, "DIO2_N") == 0) {
		*rpPin = RP_DIO2_N;
	}
	else if(strcasecmp(pinStr, "DIO3_N") == 0) {
		*rpPin = RP_DIO3_N;
	}
	else if(strcasecmp(pinStr, "DIO4_N") == 0) {
		*rpPin = RP_DIO4_N;
	}
	else if(strcasecmp(pinStr, "DIO5_N") == 0) {
		*rpPin = RP_DIO5_N;
	}
	else if(strcasecmp(pinStr, "DIO6_N") == 0) {
		*rpPin = RP_DIO6_N;
	}
	else if(strcasecmp(pinStr, "DIO7_N") == 0) {
		*rpPin = RP_DIO7_N;
	}
	else {
		return 1; // ERROR
	}

	return 0; // OK
}

int getRpDirection(const char *dirStr, rp_pinDirection_t *direction) {
	if (strcasecmp(dirStr, "OUTP") == 0) {
		*direction = RP_OUT;
	}
	else if(strcasecmp(dirStr, "INP") == 0) {
		*direction = RP_IN;
	}
	else {
		return 1; // ERROR
	}

	return 0; // OK
}

int getRpApin(const char *pinStr, rp_apin_t *rpPin) {
	if(strcasecmp(pinStr, "AOUT0") == 0) {
		*rpPin = RP_AOUT0;
	}
	else if(strcasecmp(pinStr, "AOUT1") == 0) {
		*rpPin = RP_AOUT1;
	}
	else if(strcasecmp(pinStr, "AOUT2") == 0) {
		*rpPin = RP_AOUT2;
	}
	else if(strcasecmp(pinStr, "AOUT3") == 0) {
		*rpPin = RP_AOUT3;
	}

	else if(strcasecmp(pinStr, "AIN0") == 0) {
		*rpPin = RP_AIN0;
	}
	else if(strcasecmp(pinStr, "AIN1") == 0) {
		*rpPin = RP_AIN1;
	}
	else if(strcasecmp(pinStr, "AIN2") == 0) {
		*rpPin = RP_AIN2;
	}
	else if(strcasecmp(pinStr, "AIN3") == 0) {
		*rpPin = RP_AIN3;
	}
	else {
		return 1; // ERROR
	}

	return 0; // OK
}

int getRpDecimation(int decimationInt, rp_acq_decimation_t *decimation) {
	switch (decimationInt) {
		case 1:
			*decimation = RP_DEC_1;
			return RP_OK;
		case 8:
			*decimation = RP_DEC_8;
			return RP_OK;
		case 64:
			*decimation = RP_DEC_64;
			return RP_OK;
		case 1024:
			*decimation = RP_DEC_1024;
			return RP_OK;
		case 8192:
			*decimation = RP_DEC_8192;
			return RP_OK;
		case 65536:
			*decimation = RP_DEC_65536;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}


int getRpDecimationInt(rp_acq_decimation_t decimation, int *decimationInt) {
	switch (decimation) {
		case RP_DEC_1:
			*decimationInt = 1;
			return RP_OK;
		case RP_DEC_8:
			*decimationInt = 8;
			return RP_OK;
		case RP_DEC_64:
			*decimationInt = 64;
			return RP_OK;
		case RP_DEC_1024:
			*decimationInt = 1024;
			return RP_OK;
		case RP_DEC_8192:
			*decimationInt = 8192;
			return RP_OK;
		case RP_DEC_65536:
			*decimationInt = 65536;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}

int getRpSamplingRateString(rp_acq_sampling_rate_t decimation, char *decimationString) {
	switch (decimation) {
		case RP_SMP_125M:
			strcpy(decimationString, "125MHz");
			return RP_OK;
		case RP_SMP_15_625M:
			strcpy(decimationString, "15_6MHz");
			return RP_OK;
		case RP_SMP_1_953M:
			strcpy(decimationString, "1_9MHz");
			return RP_OK;
		case RP_SMP_122_070K:
			strcpy(decimationString, "103_8kHz");
			return RP_OK;
		case RP_SMP_15_258K:
			strcpy(decimationString, "15_2kHz");
			return RP_OK;
		case RP_SMP_1_907K:
			strcpy(decimationString, "1_9kHz");
			return RP_OK;
		default:
			return RP_EOOR;
	}
}

int getRpSamplingRate(const char *decimationString, rp_acq_sampling_rate_t *decimation) {
	if (strcmp(decimationString, "125MHz") == 0) {
		*decimation = RP_SMP_125M;
	}
	else if (strcmp(decimationString, "15_6MHz") == 0) {
		*decimation = RP_SMP_15_625M;
	}
	else if (strcmp(decimationString, "1_9MHz") == 0) {
		*decimation = RP_SMP_1_953M;
	}
	else if (strcmp(decimationString, "103_8kHz") == 0) {
		*decimation = RP_SMP_122_070K;
	}
	else if (strcmp(decimationString, "15_2kHz") == 0) {
		*decimation = RP_SMP_15_258K;
	}
	else if (strcmp(decimationString, "1_9kHz") == 0) {
		*decimation = RP_SMP_1_907K;
	}
	else {
		return RP_EOOR;
	}
	return RP_OK;
}


	int getRpGain(const char *gainStr, rp_pinState_t *state) {
	if (strcmp(gainStr, "LV") == 0) {
		*state = RP_LOW;
	}
	else if (strcmp(gainStr, "HV") == 0) {
		*state = RP_HIGH;
	}
	else {
		return RP_EOOR;
	}
	return RP_OK;
}

int getRpTriggerSource(const char *sourceStr, rp_acq_trig_src_t *source) {
	if (strcmp(sourceStr, "DISABLED") == 0) {
		*source = RP_TRIG_SRC_DISABLED;
	}
	else if (strcmp(sourceStr, "NOW") == 0) {
		*source = RP_TRIG_SRC_NOW;
	}
	else if (strcmp(sourceStr, "CH1_PE") == 0) {
		*source = RP_TRIG_SRC_CHA_PE;
	}
	else if (strcmp(sourceStr, "CH1_NE") == 0) {
		*source = RP_TRIG_SRC_CHA_NE;
	}
	else if (strcmp(sourceStr, "CH2_PE") == 0) {
		*source = RP_TRIG_SRC_CHB_PE;
	}
	else if (strcmp(sourceStr, "CH2_NE") == 0) {
		*source = RP_TRIG_SRC_CHB_NE;
	}
	else if (strcmp(sourceStr, "EXT_PE") == 0) {
		*source = RP_TRIG_SRC_EXT_PE;
	}
	else if (strcmp(sourceStr, "EXT_NE") == 0) {
		*source = RP_TRIG_SRC_EXT_NE;
	}
	else if (strcmp(sourceStr, "AWG_PE") == 0) {
		*source = RP_TRIG_SRC_AWG_PE;
	}
	else if (strcmp(sourceStr, "AWG_NE") == 0) {
		*source = RP_TRIG_SRC_AWG_NE;
	}
	else {
		return RP_EOOR;
	}
	return RP_OK;
}

int getRpTriggerSourceString(rp_acq_trig_src_t source, char *triggSourceString) {
	switch (source) {
		case RP_TRIG_SRC_DISABLED:
			strcpy(triggSourceString, "TD");
			break;
		default:
			strcpy(triggSourceString, "WAIT");
	}
	return  RP_OK;
}

int getRpUnit(const char *unitString, rp_scpi_acq_unit_t *unit) {
	if (strcmp(unitString, "VOLTS") == 0) {
		*unit = RP_SCPI_VOLTS;
	}
	else if (strcmp(unitString, "RAW") == 0) {
		*unit = RP_SCPI_RAW;
	}
	else {
		return RP_EOOR;
	}
	return RP_OK;
}

int getRpFormat(const char *formatString, rp_scpi_acq_format_t *format) {
	if (strcmp(formatString, "FLOAT") == 0) {
		*format = RP_SCPI_FLAOT;
	}
	else if (strcmp(formatString, "ASCII") == 0) {
		*format = RP_SCPI_ADCII;
	}
	else {
		return RP_EOOR;
	}
	return RP_OK;
}

int getRpWaveform(const char *waveformString, rp_waveform_t *waveform) {
	if (strcmp(waveformString, "SINE") == 0) {
		*waveform = RP_WAVEFORM_SINE;
	}
	else if (strcmp(waveformString, "SQUARE") == 0) {
		*waveform = RP_WAVEFORM_SQUARE;
	}
	else if (strcmp(waveformString, "TRIANGLE") == 0) {
		*waveform = RP_WAVEFORM_TRIANGLE;
	}
	else if (strcmp(waveformString, "PWM") == 0) {
		*waveform = RP_WAVEFORM_PWM;
	}
	else if (strcmp(waveformString, "SAWD") == 0) {
		*waveform = RP_WAVEFORM_RAMP_DOWN;
	}
	else if (strcmp(waveformString, "SAWU") == 0) {
		*waveform = RP_WAVEFORM_RAMP_UP;
	}
	else if (strcmp(waveformString, "ARBITRARY") == 0) {
		*waveform = RP_WAVEFORM_ARBITRARY;
	}
	else {
		return RP_EOOR;
	}
	return RP_OK;
}


int getRpWaveformString(rp_waveform_t waveform, char *waveformString) {
    switch (waveform) {
        case RP_WAVEFORM_SINE:
            strcpy(waveformString, "SINE");
            break;
        case RP_WAVEFORM_SQUARE:
            strcpy(waveformString, "SQUARE");
            break;
        case RP_WAVEFORM_TRIANGLE:
            strcpy(waveformString, "TRIANGLE");
            break;
        case RP_WAVEFORM_PWM:
            strcpy(waveformString, "PWM");
            break;
        case RP_WAVEFORM_RAMP_DOWN:
            strcpy(waveformString, "SAWD");
            break;
        case RP_WAVEFORM_RAMP_UP:
            strcpy(waveformString, "SAWU");
            break;
        case RP_WAVEFORM_ARBITRARY:
            strcpy(waveformString, "ARBITRARY");
            break;
        default:
            return RP_EOOR;
    }
    return RP_OK;
}

int getRpGenTriggerSource(const char *triggerSourceString, rp_trig_src_t *triggerSource) {
	if (strcmp(triggerSourceString, "INT") == 0) {
		*triggerSource = RP_GEN_TRIG_SRC_INTERNAL;
	}
	else if (strcmp(triggerSourceString, "EXT_PE") == 0) {
		*triggerSource = RP_GEN_TRIG_SRC_EXT_PE;
	}
	else if (strcmp(triggerSourceString, "EXT_NE") == 0) {
		*triggerSource = RP_GEN_TRIG_SRC_EXT_NE;
	}
    else if (strcmp(triggerSourceString, "GATED") == 0) {
        *triggerSource = RP_GEN_TRIG_GATED_BURST;
    }
	else {
		return RP_EOOR;
	}
	return RP_OK;
}

int getRpGenTriggerSourceString(rp_trig_src_t triggerSource, char *string) {
    switch (triggerSource) {
        case RP_GEN_TRIG_SRC_INTERNAL:
            strcpy(string, "INT");
            break;
        case RP_GEN_TRIG_SRC_EXT_PE:
            strcpy(string, "EXT_PE");
            break;
        case RP_GEN_TRIG_SRC_EXT_NE:
            strcpy(string, "EXT_NE");
            break;
        case RP_GEN_TRIG_GATED_BURST:
            strcpy(string, "GATED");
            break;
        default:
            return RP_EOOR;
    }
    return RP_OK;
}

int getRpInfinityInteger(const char *string, int32_t *value) {
    if (strcmp(string, "INF") == 0) {
        *value = -1;
    }
    else {
        *value = atoi(string);
    }
    return RP_OK;
}

int getRpInfinityIntegerString(int32_t value, char *string) {
    if (value == -1) {
        strcpy(string, "INF");
    }
    else {
        sprintf(string, "%d", value);
    }
    return RP_OK;
}