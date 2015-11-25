
#include "main.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "version.h"

/***************************************************************************************
*                                     LCR METER                                        *
****************************************************************************************/

//TODO make a more detailed parameters specification. More parameters soon to be added.
CIntParameter parameterPeriiod("DEBUG_PARAM_PERIOD", CBaseParameter::RW, 200, 0, 0, 100);

//Meas param
CFloatParameter amplitudeZ("AMPLITUDEZ", CBaseParameter::RW, 0, 0, 0, 1000000);
CFloatParameter phaseZ("PHASEZ", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_L("LCR_L", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_C("LCR_C", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_R("LCR_R", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_P("LCR_P", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_D("LCR_D", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_Q("LCR_Q", CBaseParameter::RW, 0, 0, -1e6, 1e6);
CFloatParameter lcr_E("LCR_E", CBaseParameter::RW, 0, 0, -1e6, 1e6);

CBooleanParameter startMeasure("LCR_RUN", CBaseParameter::RW, false, 0);
CFloatParameter frequency("LCR_FREQ", CBaseParameter::RW, 1000, 0, 10, 100000);




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

	float ampl_z;

	/*Change frequency*/
	if(IS_NEW(frequency)){
		lcrApp_LcrSetFrequency(frequency.NewValue());
		frequency.Update();
	}


	if(startMeasure.NewValue() == false){
		
		amplitudeZ.Value() = 0;
		startMeasure.Update();
		
	}else if(startMeasure.NewValue() == true){
		lcrApp_LcrRun(&ampl_z);
		startMeasure.Update();
		amplitudeZ.Value() = ampl_z;
	}

}

void OnNewParams(void){

	parameterPeriiod.Update();

	UpdateParams();
	//Returned changed data to the client.
	CDataManager::GetInstance()->SendAllParams();
}

void OnNewSignals(void){}
