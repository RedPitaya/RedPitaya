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
#include "scpi/parser.h"

/* Parse channel */
//TODO: RP_ERR message should be in form COMMAND: MESSAGE
int RP_ParseChArgv(scpi_t *context, rp_channel_t *channel){

	int32_t ch_usr[1];

	SCPI_CommandNumbers(context, ch_usr, 1, SCPI_CMD_NUM);
    if(ch_usr[0] < MIN_CH && ch_usr[0] > MAX_CH){
        return RP_EOOR;
    }
    *channel = ch_usr[0];
    
    return RP_OK;
}

int getRpWaveform(const char *waveformString, rp_waveform_t *waveform) {
	if      (strcmp(waveformString, "SINE"     ) == 0)  *waveform = RP_WAVEFORM_SINE     ;
	else if (strcmp(waveformString, "SQUARE"   ) == 0)  *waveform = RP_WAVEFORM_SQUARE   ;
	else if (strcmp(waveformString, "TRIANGLE" ) == 0)  *waveform = RP_WAVEFORM_TRIANGLE ;
	else if (strcmp(waveformString, "PWM"      ) == 0)  *waveform = RP_WAVEFORM_PWM      ;
	else if (strcmp(waveformString, "SAWD"     ) == 0)  *waveform = RP_WAVEFORM_RAMP_DOWN;
	else if (strcmp(waveformString, "SAWU"     ) == 0)  *waveform = RP_WAVEFORM_RAMP_UP  ;
	else if (strcmp(waveformString, "ARBITRARY") == 0)  *waveform = RP_WAVEFORM_ARBITRARY;
	else                                                return RP_EOOR;
	return RP_OK;
}


int getRpWaveformString(rp_waveform_t waveform, char *waveformString) {
    switch (waveform) {
        case RP_WAVEFORM_SINE     :  strcpy(waveformString, "SINE"     );  break;
        case RP_WAVEFORM_SQUARE   :  strcpy(waveformString, "SQUARE"   );  break;
        case RP_WAVEFORM_TRIANGLE :  strcpy(waveformString, "TRIANGLE" );  break;
        case RP_WAVEFORM_PWM      :  strcpy(waveformString, "PWM"      );  break;
        case RP_WAVEFORM_RAMP_DOWN:  strcpy(waveformString, "SAWD"     );  break;
        case RP_WAVEFORM_RAMP_UP  :  strcpy(waveformString, "SAWU"     );  break;
        case RP_WAVEFORM_ARBITRARY:  strcpy(waveformString, "ARBITRARY");  break;
        default                   :  return RP_EOOR;
    }
    return RP_OK;
}

int getRpGenTriggerSource(const char *triggerSourceString, rp_trig_src_t *triggerSource) {
	if      (strcmp(triggerSourceString, "INT"   ) == 0)  *triggerSource = RP_GEN_TRIG_SRC_INTERNAL;
	else if (strcmp(triggerSourceString, "EXT_PE") == 0)  *triggerSource = RP_GEN_TRIG_SRC_EXT_PE;
	else if (strcmp(triggerSourceString, "EXT_NE") == 0)  *triggerSource = RP_GEN_TRIG_SRC_EXT_NE;
	else if (strcmp(triggerSourceString, "GATED" ) == 0)  *triggerSource = RP_GEN_TRIG_GATED_BURST;
	else                                                  return RP_EOOR;
	return RP_OK;
}

int getRpGenTriggerSourceString(rp_trig_src_t triggerSource, char *string) {
    switch (triggerSource) {
        case RP_GEN_TRIG_SRC_INTERNAL:  strcpy(string, "INT"   );  break;
        case RP_GEN_TRIG_SRC_EXT_PE  :  strcpy(string, "EXT_PE");  break;
        case RP_GEN_TRIG_SRC_EXT_NE  :  strcpy(string, "EXT_NE");  break;
        case RP_GEN_TRIG_GATED_BURST :  strcpy(string, "GATED" );  break;
        default                      :  return RP_EOOR;
    }
    return RP_OK;
}

int getRpAppTrigSource(const char *string, rpApp_osc_trig_source_t *source) {
	if      (strcmp(string, "CH1") == 0)  *source = RPAPP_OSC_TRIG_SRC_CH1;
	else if (strcmp(string, "CH2") == 0)  *source = RPAPP_OSC_TRIG_SRC_CH2;
	else if (strcmp(string, "EXT") == 0)  *source = RPAPP_OSC_TRIG_SRC_EXTERNAL;
	else                                  return RP_EOOR;
	return RP_OK;
}

int getRpAppTrigSourceString(rpApp_osc_trig_source_t source, char *string) {
	switch (source) {
		case RPAPP_OSC_TRIG_SRC_CH1     :  strcpy(string, "CH1");  break;
		case RPAPP_OSC_TRIG_SRC_CH2     :  strcpy(string, "CH2");  break;
		case RPAPP_OSC_TRIG_SRC_EXTERNAL:  strcpy(string, "EXT");  break;
		default                         :  return RP_EOOR;
	}
	return RP_OK;
}

int getRpAppTrigSlope(const char *string, rpApp_osc_trig_slope_t *slope) {
	if      (strcmp(string, "POS") == 0)  *slope= RPAPP_OSC_TRIG_SLOPE_PE;
	else if (strcmp(string, "NEG") == 0)  *slope= RPAPP_OSC_TRIG_SLOPE_NE;
	else                                  return RP_EOOR;
	return RP_OK;
}

int getRpAppTrigSlopeString(rpApp_osc_trig_slope_t slope, char *string) {
	switch (slope) {
		case RPAPP_OSC_TRIG_SLOPE_NE:  strcpy(string, "POS");  break;
		case RPAPP_OSC_TRIG_SLOPE_PE:  strcpy(string, "NEG");  break;
		default                     :  return RP_EOOR;
	}
	return RP_OK;
}

int getRpAppTrigSweep(const char *string, rpApp_osc_trig_sweep_t *sweep) {
	if      (strcmp(string, "AUTO"  ) == 0)  *sweep = RPAPP_OSC_TRIG_AUTO;
	else if (strcmp(string, "NORMAL") == 0)  *sweep = RPAPP_OSC_TRIG_NORMAL;
	else if (strcmp(string, "SINGLE") == 0)  *sweep = RPAPP_OSC_TRIG_SINGLE;
	else                                     return RP_EOOR;
	return RP_OK;
}

int getRpAppTrigSweepString(rpApp_osc_trig_sweep_t sweep, char *string) {
	switch (sweep) {
		case RPAPP_OSC_TRIG_AUTO  :  strcpy(string, "AUTO"  );  break;
		case RPAPP_OSC_TRIG_NORMAL:  strcpy(string, "NORMAL");  break;
		case RPAPP_OSC_TRIG_SINGLE:  strcpy(string, "SINGLE");  break;
		default                   :  return RP_EOOR;
	}
	return RP_OK;
}

int getRpAppMathOperation(const char *string, rpApp_osc_math_oper_t *op) {
	if      (strcmp(string, "NONE") == 0)  *op = RPAPP_OSC_MATH_NONE;
	else if (strcmp(string, "ADD" ) == 0)  *op = RPAPP_OSC_MATH_ADD ;
	else if (strcmp(string, "SUB" ) == 0)  *op = RPAPP_OSC_MATH_SUB ;
	else if (strcmp(string, "MUL" ) == 0)  *op = RPAPP_OSC_MATH_MUL ;
	else if (strcmp(string, "DIV" ) == 0)  *op = RPAPP_OSC_MATH_DIV ;
	else if (strcmp(string, "ABS" ) == 0)  *op = RPAPP_OSC_MATH_ABS ;
	else if (strcmp(string, "DER" ) == 0)  *op = RPAPP_OSC_MATH_DER ;
	else if (strcmp(string, "INT" ) == 0)  *op = RPAPP_OSC_MATH_INT ;
	else                                   return RP_EOOR;
	return RP_OK;
}

int getRpAppMathOperationString(rpApp_osc_math_oper_t op, char *string) {
	switch (op) {
		case RPAPP_OSC_MATH_NONE:  strcpy(string, "NONE");  break;
		case RPAPP_OSC_MATH_ADD :  strcpy(string, "ADD" );  break;
		case RPAPP_OSC_MATH_SUB :  strcpy(string, "SUB" );  break;
		case RPAPP_OSC_MATH_MUL :  strcpy(string, "MUL" );  break;
		case RPAPP_OSC_MATH_DIV :  strcpy(string, "DIV" );  break;
		case RPAPP_OSC_MATH_ABS :  strcpy(string, "ABS" );  break;
		case RPAPP_OSC_MATH_DER :  strcpy(string, "DER" );  break;
		case RPAPP_OSC_MATH_INT :  strcpy(string, "INT" );  break;
		default                 :  return RP_EOOR;
	}
	return RP_OK;
}

int getRpInfinityInteger(const char *string, int32_t *value) {
    if (strcmp(string, "INF") == 0)  *value = 0;
    else                             *value = atoi(string);
    return RP_OK;
}

int getRpInfinityIntegerString(int32_t value, char *string) {
    if (value == 0)   strcpy(string, "INF");
    else              sprintf(string, "%d", value);
    return RP_OK;
}

int getRpStateIntegerString(int32_t value, char *string){
	if(value == 1) strcpy(string, "ON");
	else		   strcpy(string, "OFF");
	return RP_OK;
}
