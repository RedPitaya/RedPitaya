
#include "main.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <sys/syslog.h>

#include "version.h"

/***************************************************************************************
*                                     LCR METER                                        *
****************************************************************************************/

//TODO make a more detailed parameters specification. More parameters soon to be added.
CIntParameter parameterPeriiod("DEBUG_PARAM_PERIOD", CBaseParameter::RW, 200, 0, 0, 100);

//Out params
CFloatParameter lcr_amplitude("LCR_Z", CBaseParameter::RW, 0, 0, 0, 1000000);
CFloatParameter lcr_Inductance("LCR_L", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_Capacitance("LCR_C", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_Resitance("LCR_R", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_phase("LCR_P", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_D("LCR_D", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_Q("LCR_Q", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_ESR("LCR_ESR", CBaseParameter::RW, 0, 0, -1e6, 1e6);

//In params
CFloatParameter frequency("LCR_FREQ", CBaseParameter::RW, 1000, 0, 10, 100000);
CIntParameter   calibMode("LCR_CALIB_MODE", CBaseParameter::RW, 0, 0, 0, 3);

CBooleanParameter startMeasure("LCR_RUN", CBaseParameter::RW, false, 0);
CBooleanParameter startCalibration("LCR_CALIBRATION", CBaseParameter::RW, false, 0);
CBooleanParameter toleranceMode("LCR_TOLERANCE", CBaseParameter::RW, false, 0);
CBooleanParameter seriesMode("LCR_SERIES", CBaseParameter::RW, false, 0);
CBooleanParameter rangeMode("LCR_RANGE", CBaseParameter::RW, false, 0);

const char *rp_app_desc(void){
	return (const char *)"Red Pitaya Lcr meter application.\n";
}

int rp_app_init(void){	
	fprintf(stderr, "Loading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);
	CDataManager::GetInstance()->SetParamInterval(parameterPeriiod.Value());

	lcrApp_lcrInit();

	//impApp_ImpInit();
	return 0;
}

int rp_app_exit(void){
	fprintf(stderr, "Unloading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);
	//impApp_ImpRelease();
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

		lcr_amplitude.Value() 		= data->lcr_amplitude;
		lcr_phase.Value()     		= data->lcr_phase;
		lcr_Inductance.Value()		= data->lcr_L;
		lcr_Capacitance.Value() 	= data->lcr_C;
		lcr_Resitance.Value()		= data->lcr_R;
		lcr_D.Value()				= data->lcr_D;
		lcr_Q.Value()				= data->lcr_Q;
		lcr_ESR.Value()				= data->lcr_ESR;

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

	if(IS_NEW(toleranceMode)){
		lcrApp_LcrSetMeasTolerance(toleranceMode.NewValue());
		toleranceMode.Update();
	}

	if(IS_NEW(seriesMode)){
		lcrApp_LcrSetMeasSeries(seriesMode.NewValue());
		seriesMode.Update();
	}

}

void OnNewParams(void){

	parameterPeriiod.Update();

	UpdateParams();
	//Returned changed data to the client.
	CDataManager::GetInstance()->SendAllParams();
}

void OnNewSignals(void){}
