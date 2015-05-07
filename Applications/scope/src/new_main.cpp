#include "new_main.h"

#include <DataManager.h>
#include <CustomParameters.h>
#include "../../rp_sdk/include/rpApp.h"
#include "../../rp_sdk/include/CustomParameters.h"


/* --------------------------------  OUT SIGNALS  ------------------------------ *//**/
CFloatSignal ch1("ch1", CH_SIGNAL_SIZE, 0.0f);
CFloatSignal ch2("ch2", CH_SIGNAL_SIZE, 0.0f);

/* --------------------------------  OUT PARAMETERS  ------------------------------ */
CBooleanParameter in1Show("CH1_SHOW", CBaseParameter::RW, false, 0);
CBooleanParameter in2Show("CH2_SHOW", CBaseParameter::RW, false, 0);
CBooleanParameter in1InvShow("CH1_SHOW_INVERTED", CBaseParameter::RW, true, 0);
CBooleanParameter in2InvShow("CH2_SHOW_INVERTED", CBaseParameter::RW, true, 0);

CBooleanParameter inReset("OSC_RST", CBaseParameter::RW, false, 0);
CBooleanParameter inRun("OSC_RUN", CBaseParameter::RW, false, 0);
CBooleanParameter inAutoscale("OSC_AUTOSCALE", CBaseParameter::RW, false, 0);
CBooleanParameter inSingle("OSC_SINGLE", CBaseParameter::RW, false, 0);

CFloatParameter in1Offset("OSC_CH1_OFFSET", CBaseParameter::RW, 0, 0, -40, 40);
CFloatParameter in2Offset("OSC_CH2_OFFSET", CBaseParameter::RW, 0, 0, -40, 40);
CFloatParameter in1Scale("OSC_CH1_SCALE", CBaseParameter::RW, 0.1, 0, 0, 1000);
CFloatParameter in2Scale("OSC_CH2_SCALE", CBaseParameter::RW, 0.1, 0, 0, 1000);
CFloatParameter in1Probe("OSC_CH1_PROBE", CBaseParameter::RW, 10, 0, 0, 1000);
CFloatParameter in2Probe("OSC_CH2_PROBE", CBaseParameter::RW, 10, 0, 0, 1000);
CFloatParameter inTimeOffset("OSC_TIME_OFFSET", CBaseParameter::RW, 0, 0, -100000, 100000);
CFloatParameter inTimeScale("OSC_TIME_SCALE", CBaseParameter::RW, 1, 0, 0, 50000);

CCustomParameter<rpApp_osc_in_gain_t> in1Gain("OSC_CH1_IN_GAIN", CBaseParameter::RW, RPAPP_OSC_IN_GAIN_LV, 0, RPAPP_OSC_IN_GAIN_LV, RPAPP_OSC_IN_GAIN_HV);
CCustomParameter<rpApp_osc_in_gain_t> in2Gain("OSC_CH2_IN_GAIN", CBaseParameter::RW, RPAPP_OSC_IN_GAIN_LV, 0, RPAPP_OSC_IN_GAIN_LV, RPAPP_OSC_IN_GAIN_HV);

CFloatParameter inTriggLevel("OSC_TRIG_LEVEL", CBaseParameter::RW, 0, 0, -20, 20);
CCustomParameter<rpApp_osc_trig_sweep_t> inTrigSweep("OSC_TRIG_SWEEP", CBaseParameter::RW, RPAPP_OSC_TRIG_AUTO, 0, RPAPP_OSC_TRIG_AUTO, RPAPP_OSC_TRIG_SINGLE);
CCustomParameter<rpApp_osc_trig_source_t> inTrigSource("OSC_TRIG_SOURCE", CBaseParameter::RW, RPAPP_OSC_TRIG_SRC_CH1, 0, RPAPP_OSC_TRIG_SRC_CH1, RPAPP_OSC_TRIG_SRC_EXTERNAL);
CCustomParameter<rpApp_osc_trig_slope_t> inTrigSlope("OSC_TRIG_SLOPE", CBaseParameter::RW, RPAPP_OSC_TRIG_SLOPE_PE, 0, RPAPP_OSC_TRIG_SLOPE_NE, RPAPP_OSC_TRIG_SLOPE_PE);

/* --------------------------------  MEASURE  ------------------------------ */
CIntParameter measureSelect1("OSC_MEAS_SEL1", CBaseParameter::RW, -1, 0, -1, 15);
CIntParameter measureSelect2("OSC_MEAS_SEL2", CBaseParameter::RW, -1, 0, -1, 15);
CIntParameter measureSelect3("OSC_MEAS_SEL3", CBaseParameter::RW, -1, 0, -1, 15);
CIntParameter measureSelect4("OSC_MEAS_SEL4", CBaseParameter::RW, -1, 0, -1, 15);

CFloatParameter measureValue1("OSC_MEAS_VAL1", CBaseParameter::RW, 0, 0, -1000000, 1000000);
CFloatParameter measureValue2("OSC_MEAS_VAL2", CBaseParameter::RW, 0, 0, -1000000, 1000000);
CFloatParameter measureValue3("OSC_MEAS_VAL3", CBaseParameter::RW, 0, 0, -1000000, 1000000);
CFloatParameter measureValue4("OSC_MEAS_VAL4", CBaseParameter::RW, 0, 0, -1000000, 1000000);

/* --------------------------------  CURSORS  ------------------------------ */
CIntParameter cursor1("OSC_CURSOR1", CBaseParameter::RW, -1, 0, 0, 1024);
CIntParameter cursor2("OSC_CURSOR2", CBaseParameter::RW, -1, 0, 0, 1024);
CCustomParameter<rp_channel_t> cursor1CH("OSC_CURSOR1_CH", CBaseParameter::RW, RP_CH_1, 0, RP_CH_1, RP_CH_2);
CCustomParameter<rp_channel_t> cursor2CH("OSC_CURSOR2_CH", CBaseParameter::RW, RP_CH_1, 0, RP_CH_1, RP_CH_2);

CFloatParameter cursor1V("OSC_CUR1_V", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursor2V("OSC_CUR2_V", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursor1T("OSC_CUR1_T", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursor2T("OSC_CUR2_T", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursorDT("OSC_CUR_DT", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursorDV("OSC_CUR_DV", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursorDF("OSC_CUR_DF", CBaseParameter::RW, -1, 0, -1000, 1000);


/* --------------------------------  OUTPUT PARAMETERS  ------------------------------ */
CBooleanParameter out1Show("OUTPUT1_SHOW", CBaseParameter::RW, true, 0);
CBooleanParameter out2Show("OUTPUT2_SHOW", CBaseParameter::RW, true, 0);

CBooleanParameter out1State("OUTPUT1_STATE", CBaseParameter::RW, false, 0);
CBooleanParameter out2State("OUTPUT2_STATE", CBaseParameter::RW, false, 0);
CFloatParameter out1Amplitude("SOUR1_VOLT", CBaseParameter::RW, 1, 0, -1, 1);
CFloatParameter out2Amplitude("SOUR2_VOLT", CBaseParameter::RW, 1, 0, -1, 1);
CFloatParameter out1Offset("SOUR1_VOLT_OFFS", CBaseParameter::RW, 0, 0, -1, 1);
CFloatParameter out2Offset("SOUR2_VOLT_OFFS", CBaseParameter::RW, 0, 0, -1, 1);
CFloatParameter out1Frequancy("SOUR1_FREQ_FIX", CBaseParameter::RW, 1000, 0, 0, 62.5e6);
CFloatParameter out2Frequancy("SOUR2_FREQ_FIX", CBaseParameter::RW, 1000, 0, 0, 62.5e6);

CFloatParameter out1Phase("SOUR1_PHAS", CBaseParameter::RW, 0, 0, -360, 360);
CFloatParameter out2Phase("SOUR2_PHAS", CBaseParameter::RW, 0, 0, -360, 360);
CFloatParameter out1DCYC("SOUR1_DCYC", CBaseParameter::RW, 0.5, 0, 0, 100);
CFloatParameter out2DCYC("SOUR2_DCYC", CBaseParameter::RW, 0.5, 0, 0, 100);

CCustomParameter<rp_waveform_t> out1WAveform("SOUR1_FUNC", CBaseParameter::RW, RP_WAVEFORM_SINE, 0, RP_WAVEFORM_SINE, RP_WAVEFORM_ARBITRARY);
CCustomParameter<rp_waveform_t> out2WAveform("SOUR2_FUNC", CBaseParameter::RW, RP_WAVEFORM_SINE, 0, RP_WAVEFORM_SINE, RP_WAVEFORM_ARBITRARY);

CBooleanParameter out1Burst("SOUR1_BURS_STAT", CBaseParameter::RW, false, 0);
CBooleanParameter out2Burst("SOUR2_BURS_STAT", CBaseParameter::RW, false, 0);

CCustomParameter<rp_trig_src_t> out1TriggerSource("SOUR1_TRIG_SOUR", CBaseParameter::RW, RP_GEN_TRIG_SRC_INTERNAL, 0, RP_GEN_TRIG_SRC_INTERNAL, RP_GEN_TRIG_GATED_BURST);
CCustomParameter<rp_trig_src_t> out2TriggerSource("SOUR2_TRIG_SOUR", CBaseParameter::RW, RP_GEN_TRIG_SRC_INTERNAL, 0, RP_GEN_TRIG_SRC_INTERNAL, RP_GEN_TRIG_GATED_BURST);


void UpdateParams(void) {
	if (measureSelect1.Value() != -1)
		measureValue1.Value() = getMeasureValue(measureSelect1.Value());
	if (measureSelect2.Value() != -1)
		measureValue2.Value() = getMeasureValue(measureSelect2.Value());
	if (measureSelect3.Value() != -1)
		measureValue3.Value() = getMeasureValue(measureSelect3.Value());
	if (measureSelect4.Value() != -1)
		measureValue4.Value() = getMeasureValue(measureSelect4.Value());

	if (cursor1.Value() != -1) {
		rpApp_OscGetCursorVoltage(cursor1CH.Value(), (uint32_t) cursor1.Value(), &cursor1V.Value());
		rpApp_OscGetCursorTime((uint32_t) cursor1.Value(), &cursor1T.Value());
	}
	if (cursor2.Value() != -1) {
		rpApp_OscGetCursorVoltage(cursor2CH.Value(), (uint32_t) cursor2.Value(), &cursor2V.Value());
		rpApp_OscGetCursorTime((uint32_t) cursor2.Value(), &cursor2T.Value());
	}
	if (cursor1.Value() != -1 && cursor1.Value() != -1) {
		rpApp_OscGetCursorDeltaAmplitude(cursor1CH.Value(), (uint32_t) cursor1.Value(), (uint32_t) cursor2.Value(), &cursorDV.Value());
		rpApp_OscGetCursorDeltaTime((uint32_t) cursor1.Value(), (uint32_t) cursor2.Value(), &cursorDT.Value());
		rpApp_OscGetCursorDeltaFrequency((uint32_t) cursor1.Value(), (uint32_t) cursor2.Value(), &cursorDF.Value());
	}
}

float getMeasureValue(int measure) {
	float value;
	switch (measure) {
		case 0:
		case 1:
			rpApp_OscMeasureVpp((rp_channel_t) (measure % 2), &value);
			break;
		case 2:
		case 3:
			rpApp_OscMeasureAmplitudeMin((rp_channel_t) (measure % 2), &value);
			break;
		case 4:
		case 5:
			rpApp_OscMeasureAmplitudeMax((rp_channel_t) (measure % 2), &value);
			break;
		case 6:
		case 7:
			rpApp_OscMeasureMeanVoltage((rp_channel_t) (measure % 2), &value);
			break;
		case 8:
		case 9:
			rpApp_OscMeasurePeriod((rp_channel_t) (measure % 2), &value);
			break;
		case 10:
		case 11:
			rpApp_OscMeasureFrequency((rp_channel_t) (measure % 2), &value);
			break;
		case 12:
		case 13:
			rpApp_OscMeasureDutyCycle((rp_channel_t) (measure % 2), &value);
			break;
		case 14:
		case 15:
			rpApp_OscMeasureRootMeanSquare((rp_channel_t) (measure % 2), &value);
			break;
		default:
			value = 0;
	}
	return value;
}

void UpdateSignals(void) {
	float data[1024];

	if (in1Show.Value()) {
		if(in1InvShow.Value()){
			rpApp_OscGetInvViewData(RP_CH_1, data, 1024);
		}else{
			rpApp_OscGetViewData(RP_CH_1, data, 1024);
		}

		if (ch1.GetSize() != CH_SIGNAL_SIZE)
			ch1.Resize(CH_SIGNAL_SIZE);
		for (int i = 0; i < 1024; i++)
			ch1[i] = data[i];
	} else {
		ch1.Resize(0);
	}

	if (in2Show.Value()) {
		
		if(in2InvShow.Value()){
			rpApp_OscGetInvViewData(RP_CH_2, data, 1024);
		}else{
			rpApp_OscGetViewData(RP_CH_2, data, 1024);
		}
		
		if (ch2.GetSize() != CH_SIGNAL_SIZE)
			ch2.Resize(CH_SIGNAL_SIZE);
		for (int i = 0; i < 1024; i++)
			ch2[i] = data[i];
	} else {
		ch2.Resize(0);
	}
}

void OnNewParams(void) {
/* ------------------------------ IN PARAMETERS ---------------------------------------- */
	in1Show.Update();
	in2Show.Update();
	if (inReset.NewValue()) {
		rpApp_OscReset();
	}
	if (inRun.NewValue()) {
		rpApp_OscRun();
	} else {
		rpApp_OscStop();
	}
	inRun.Update();

	if (inAutoscale.NewValue()) {
		rpApp_OscAutoScale();
		float value;
		rpApp_OscGetAmplitudeScale(RP_CH_1, &value);
		in1Scale.Value() = value;
		rpApp_OscGetAmplitudeScale(RP_CH_2, &value);
		in2Scale.Value() = value;
		rpApp_OscGetAmplitudeOffset(RP_CH_1, &value);
		in1Offset.Value() = value;
		rpApp_OscGetAmplitudeOffset(RP_CH_2, &value);
		in1Offset.Value() = value;
		rpApp_OscGetTimeOffset(&value);
		inTimeOffset.Value() = value;
		rpApp_OscGetTimeScale(&value);
		inTimeScale.Value() = value;
	}
	if (inSingle.NewValue()) {
		rpApp_OscSingle();
	}

	if (rpApp_OscSetAmplitudeOffset(RP_CH_1, in1Offset.NewValue())) {
		in1Offset.Update();
	}
	if (rpApp_OscSetAmplitudeOffset(RP_CH_2, in2Offset.NewValue())) {
		in2Offset.Update();
	}
	if (rpApp_OscSetAmplitudeOffset(RP_CH_1, in1Scale.NewValue())) {
		in1Scale.Update();
	}
	if (rpApp_OscSetAmplitudeOffset(RP_CH_2, in2Scale.NewValue())) {
		in2Scale.Update();
	}
	if (rpApp_OscSetProbeAtt(RP_CH_1, in1Probe.NewValue())) {
		in1Probe.Update();
	}
	if (rpApp_OscSetProbeAtt(RP_CH_2, in2Probe.NewValue())) {
		in2Probe.Update();
	}
	if (rpApp_OscSetInputGain(RP_CH_1, in1Gain.NewValue())) {
		in1Gain.Update();
	}
	if (rpApp_OscSetInputGain(RP_CH_2, in2Gain.NewValue())) {
		in2Gain.Update();
	}
	if (rpApp_OscSetTimeOffset(inTimeOffset.NewValue())) {
		inTimeOffset.Update();
	}
	if (rpApp_OscSetTimeScale(inTimeScale.NewValue())) {
		inTimeScale.Update();
	}
	if (rpApp_OscSetTriggerSweep(inTrigSweep.NewValue())) {
		inTrigSweep.Update();
	}
	if (rpApp_OscSetTriggerSource(inTrigSource.NewValue())) {
		inTrigSource.Update();
	}
	if (rpApp_OscSetTriggerSlope(inTrigSlope.NewValue())) {
		inTrigSlope.Update();
	}
	if (rpApp_OscSetTriggerLevel(inTriggLevel.NewValue())) {
		inTriggLevel.Update();
	}
	cursor1.Update();
	cursor2.Update();
	cursor1CH.Update();
	cursor2CH.Update();

/* ------------------------------ OUT PARAMETERS ---------------------------------------- */
	out1Show.Update();
	out2Show.Update();
	if (rp_GenAmp(RP_CH_1, out1State.NewValue())) {
		out1State.Update();
	}
	if (rp_GenAmp(RP_CH_2, out2State.NewValue())) {
		out2State.Update();
	}
	if (rp_GenOffset(RP_CH_1, out1Offset.NewValue())) {
		out1Offset.Update();
	}
	if (rp_GenOffset(RP_CH_2, out2Offset.NewValue())) {
		out2Offset.Update();
	}
	if (rp_GenAmp(RP_CH_1, out1Amplitude.NewValue())) {
		out1Amplitude.Update();
	}
	if (rp_GenAmp(RP_CH_2, out2Amplitude.NewValue())) {
		out2Amplitude.Update();
	}
	if (rp_GenFreq(RP_CH_1, out1Frequancy.NewValue())) {
		out1Frequancy.Update();
	}
	if (rp_GenFreq(RP_CH_2, out2Frequancy.NewValue())) {
		out2Frequancy.Update();
	}
	if (rp_GenPhase(RP_CH_1, out1Phase.NewValue())) {
		out1Phase.Update();
	}
	if (rp_GenPhase(RP_CH_2, out2Phase.NewValue())) {
		out2Phase.Update();
	}
	if (rp_GenDutyCycle(RP_CH_1, out1DCYC.NewValue())) {
		out1DCYC.Update();
	}
	if (rp_GenDutyCycle(RP_CH_2, out2DCYC.NewValue())) {
		out2DCYC.Update();
	}
	if (rp_GenWaveform(RP_CH_1, out1WAveform.NewValue())) {
		out1WAveform.Update();
	}
	if (rp_GenWaveform(RP_CH_2, out2WAveform.NewValue())) {
		out2WAveform.Update();
	}
	if (rp_GenMode(RP_CH_1, out1Burst.NewValue() ? RP_GEN_MODE_BURST : RP_GEN_MODE_CONTINUOUS)) {
		out1Burst.Update();
	}
	if (rp_GenMode(RP_CH_2, out2Burst.NewValue() ? RP_GEN_MODE_BURST : RP_GEN_MODE_CONTINUOUS)) {
		out2Burst.Update();
	}
	if (rp_GenTriggerSource(RP_CH_1, out1TriggerSource.NewValue())) {
		out1TriggerSource.Update();
	}
	if (rp_GenTriggerSource(RP_CH_2, out2TriggerSource.NewValue())) {
		out2TriggerSource.Update();
	}
}