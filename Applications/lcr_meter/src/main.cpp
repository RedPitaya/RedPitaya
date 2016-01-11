
#include "main.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "version.h"

/***************************************************************************************
*                                     LCR METER                                        *
****************************************************************************************/
//TODO make a more detailed parameters specification.
CIntParameter parameterPeriiod("DEBUG_PARAM_PERIOD", CBaseParameter::RW, 200, 0, 0, 100);

//Out params
CFloatParameter lcr_amplitude("LCR_Z", CBaseParameter::RW, 0, 0, 0, 1e6);
CDoubleParameter lcr_Inductance("LCR_L", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CDoubleParameter lcr_Capacitance("LCR_C", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CDoubleParameter lcr_Resitance("LCR_R", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_phase("LCR_P", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_D("LCR_D", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_Q("LCR_Q", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_ESR("LCR_ESR", CBaseParameter::RW, 0, 0, -1e6, 1e6);

CFloatParameter lcr_C_precision("LCR_C_PREC", CBaseParameter::RW, 0, 0, 0, 10);

//Measurement parameters for primary display
CFloatParameter lcr_AmpMin("LCR_Z_MIN", CBaseParameter::RW, 1e9, 0, -1e6, 1e6);
CFloatParameter lcr_AmpMax("LCR_Z_MAX", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_AmpAvg("LCR_Z_AVG", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_IndMin("LCR_L_MIN", CBaseParameter::RW, 1e9, 0, -1e6, 1e6);
CFloatParameter lcr_IndMax("LCR_L_MAX", CBaseParameter::RW, -1e9, 0, -1e6, 1e6);
CFloatParameter lcr_IndAvg("LCR_L_AVG", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_CapMin("LCR_C_MIN", CBaseParameter::RW, 1e9, 0, -1e6, 1e6);
CFloatParameter lcr_CapMax("LCR_C_MAX", CBaseParameter::RW, -1e9, 0, -1e6, 1e6);
CFloatParameter lcr_CapAvg("LCR_C_AVG", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_ResMin("LCR_R_MIN", CBaseParameter::RW, 1e7, 0, -1e6, 1e6);
CFloatParameter lcr_ResMax("LCR_R_MAX", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_ResAvg("LCR_R_AVG", CBaseParameter::RW, 0, 0, -1e6, 1e6);

//In params
CFloatParameter frequency("LCR_FREQ", CBaseParameter::RW, 1000, 0, 10, 1e6);
CIntParameter   calibMode("LCR_CALIB_MODE", CBaseParameter::RW, 0, 0, 0, 3);

CBooleanParameter startMeasure("LCR_RUN", CBaseParameter::RW, false, 0);
CBooleanParameter startCalibration("LCR_CALIBRATION", CBaseParameter::RW, false, 0);
CIntParameter   toleranceMode("LCR_TOLERANCE", CBaseParameter::RW, 0, 0, 0, 4);
CIntParameter   relativeMode("LCR_RELATIVE", CBaseParameter::RW, 0, 0, 0, 4);
CFloatParameter relSavedZ("LCR_REL_SAVED", CBaseParameter::RW, 0, 0, 0, 1e6);
CFloatParameter tolSavedZ("LCR_TOL_SAVED", CBaseParameter::RW, 0, 0, 0, 1e6);
CBooleanParameter seriesMode("LCR_SERIES", CBaseParameter::RW, true, 0);

CIntParameter   rangeMode("LCR_RANGE", CBaseParameter::RW, 0, 0, 0, 4);
CIntParameter   rangeFormat("LCR_RANGE_F", CBaseParameter::RW, 0, 0, 0, 3);
CIntParameter   rangeUnits("LCR_RANGE_U", CBaseParameter::RW, 0, 0, 0, 6);




const char *rp_app_desc(void){
	return (const char *)"Red Pitaya Lcr meter application.\n";
}

int rp_app_init(void){	
	fprintf(stderr, "Loading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);
	CDataManager::GetInstance()->SetParamInterval(parameterPeriiod.Value());
	lcrApp_lcrInit();
	return 0;
}

int rp_app_exit(void){
	//lcrApp_LcrRelease();
	fprintf(stderr, "Unloading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);
	return 0;
}

int rp_set_params(rp_app_params_t *p, int len) {
    return 0;
}

int rp_get_params(rp_app_params_t **p) {
    return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len) {
    return 0;
}

void UpdateSignals(void){}

void UpdateParams(void){
	CDataManager::GetInstance()->SetParamInterval(parameterPeriiod.Value());
	CDataManager::GetInstance()->SendAllParams();

	lcr_main_data_t *data = 
		(lcr_main_data_t *)malloc(sizeof(lcr_main_data_t));

	/*Change frequency*/
	if(IS_NEW(frequency)){
		lcrApp_LcrSetFrequency(frequency.NewValue());
		frequency.Update();
	}

	/* Change calibration mode */
	if(IS_NEW(calibMode)){
		calib_t calibration = (calib_t)calibMode.NewValue();
		lcrApp_LcrSetCalibMode(calibration);
		calibMode.Update();
	}

	if(startMeasure.NewValue() == true){
		
		lcrApp_LcrRun();
		startMeasure.Update();

		//Acquire calculated parameters frm RP
		lcrApp_LcrCopyParams(data);
			
		//Precision casting
		float c_out_prec;
		if(data->lcr_C < 0.0001){
			lcr_Capacitance.Value() = data->lcr_C * pow(10, 9);
			lcr_C_precision.Value() = 9;
			lcr_Capacitance.Update();
			lcr_C_precision.Update();
		}else{
			lcr_Capacitance.Value() 	= data->lcr_C;
			lcr_C_precision.Value() = 0;
			lcr_Capacitance.Update();
		}

		lcr_amplitude.Value()       = data->lcr_amplitude;	
		lcr_phase.Value()     		= data->lcr_phase;
		lcr_Inductance.Value()		= data->lcr_L;
		lcr_Resitance.Value()		= data->lcr_R;
		lcr_D.Value()				= data->lcr_D;
		lcr_Q.Value()				= data->lcr_Q;
		lcr_ESR.Value()				= data->lcr_ESR;

		if(lcr_AmpMin.Value() > lcr_amplitude.Value() && !(lcr_amplitude.Value() < -1e6)){
			lcr_AmpMin.Value() = lcr_amplitude.Value();
			lcr_AmpMin.Update();
		} 

		if(lcr_AmpMax.Value() < lcr_amplitude.Value() && !(lcr_amplitude.Value() > 1e6)){
			lcr_AmpMax.Value() = lcr_amplitude.Value();
			lcr_AmpMax.Update();
		} 

		lcr_AmpAvg.Value() = 
			(lcr_AmpMax.Value() + lcr_AmpMin.Value()) / 2;

		lcr_AmpAvg.Update();

		if(lcr_CapMin.Value() > lcr_Capacitance.Value()){
			lcr_CapMin.Value() = lcr_Capacitance.Value();
			lcr_CapMin.Update();
		} 

		if(lcr_CapMax.Value() < lcr_Capacitance.Value()){
			lcr_CapMax.Value() = lcr_Capacitance.Value();
			lcr_CapMax.Update();
		} 

		lcr_CapAvg.Value() = 
			(lcr_CapMax.Value() + lcr_CapMin.Value()) / 2;

		lcr_CapAvg.Update();

		if(lcr_IndMin.Value() > lcr_Inductance.Value()){
			lcr_IndMin.Value() = lcr_Inductance.Value();
			lcr_IndMin.Update();
		} 

		if(lcr_IndMax.Value() < lcr_Inductance.Value()){
			lcr_IndMax.Value() = lcr_Inductance.Value();
			lcr_IndMax.Update();
		} 

		lcr_IndAvg.Value() = 
			(lcr_IndMax.Value() + lcr_IndMin.Value()) / 2;

		lcr_IndAvg.Update();

		if(lcr_ResMin.Value() > lcr_Resitance.Value()){
			lcr_ResMin.Value() = lcr_Resitance.Value();
			lcr_ResMin.Update();
		} 

		if(lcr_ResMax.Value() < lcr_Resitance.Value()){
			lcr_ResMax.Value() = lcr_Resitance.Value();
			lcr_ResMax.Update();
		} 

		lcr_ResAvg.Value() = 
			(lcr_ResMax.Value() + lcr_ResMin.Value()) / 2;

		lcr_ResAvg.Update();

		//Relative mode
		switch(relativeMode.Value()){
			case 0:
				break;
			case 1:
				lcr_amplitude.Value() = 
					relSavedZ.Value() - data->lcr_amplitude;
				break;
			case 2:
				lcr_Inductance.Value() = 
					relSavedZ.Value() - data->lcr_L;
				break;
			case 3:
				lcr_Capacitance.Value() = 
					relSavedZ.Value() - data->lcr_C;
				break;
			case 4:
				lcr_Resitance.Value() = 
					relSavedZ.Value() - data->lcr_R;
				break;
		}

	}else if(startMeasure.NewValue() == false){
		startMeasure.Update();
		lcr_amplitude.Value()   = 0;
		lcr_phase.Value()       = 0;
		lcr_Inductance.Value()  = 0;
		lcr_Capacitance.Value() = 0;
		lcr_Resitance.Value()   = 0;
		lcr_D.Value()           = 0;
		lcr_Q.Value()           = 0;
		lcr_ESR.Value()         = 0;
	}


	//Set calibration
	if(IS_NEW(startCalibration) && calibMode.Value() != 0){
		//Set correction mode;
		lcrApp_LcrStartCorrection();
		startCalibration.Update();
		startCalibration.Value() = false;
	}

	if(IS_NEW(relativeMode)){
		
		//lcrApp_LcrSetMeasRelative(relativeMode.NewValue());
		switch(relativeMode.NewValue()){
			case 0:
				break;
			case 1:
				relSavedZ.Value() = lcr_amplitude.Value();
				break;
			case 2:
				relSavedZ.Value() = lcr_Inductance.Value();
				break;
			case 3:
				relSavedZ.Value() = lcr_Capacitance.Value();
				break;
			case 4:
				relSavedZ.Value() = lcr_Resitance.Value();
				break;
		}

		relSavedZ.Update();
		relativeMode.Update();
	}

	if(IS_NEW(toleranceMode)){

		lcrApp_LcrSetMeasTolerance(toleranceMode.NewValue());
		switch(toleranceMode.NewValue()){
			case 0:
				break;
			case 1:
				tolSavedZ.Value() = lcr_amplitude.Value();
				break;
			case 2:
				tolSavedZ.Value() = lcr_Inductance.Value();
				break;
			case 3:
				tolSavedZ.Value() = lcr_Capacitance.Value();
				break;
			case 4:
				tolSavedZ.Value() = lcr_Resitance.Value();
				break;
		}

		tolSavedZ.Update();
		toleranceMode.Update();
	}

	if(IS_NEW(seriesMode)){
		lcrApp_LcrSetMeasSeries(seriesMode.NewValue());
		seriesMode.Update();
	}

	if(IS_NEW(rangeFormat)){
		lcrApp_LcrSetMeasRangeFormat(rangeFormat.NewValue());
		rangeFormat.Update();
	}

	if(IS_NEW(rangeUnits)){
		lcrApp_LcrSetMeasRangeUnits(rangeUnits.NewValue());
		rangeUnits.Update();
	}

	if(IS_NEW(rangeMode)){
		lcrApp_LcrSetMeasRangeMode(rangeMode.NewValue());
		rangeMode.Update();
	}

	free(data);

}

void OnNewParams(void){

	parameterPeriiod.Update();

	UpdateParams();
	//Returned changed data to the client.
	CDataManager::GetInstance()->SendAllParams();
}

void OnNewSignals(void){}
