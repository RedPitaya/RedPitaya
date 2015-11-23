
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

CIntParameter dummyData("DUMMY_PARAM", CBaseParameter::RW, -1, 0, -1, 10);
CBooleanParameter startMeasure("LCR_RUN", CBaseParameter::RW, false, 0);



const char *rp_app_desc(void){
	return (const char *)"Red Pitaya Lcr meter application.\n";
}

int rp_app_init(void){	
	fprintf(stderr, "Loading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);
	CDataManager::GetInstance()->SetParamInterval(parameterPeriiod.Value());

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

	float amplitudez;

	if(startMeasure.NewValue() == false){
		dummyData.Value() = 2;
	}else if(startMeasure.NewValue() == true){
		lcrApp_LcrRun(&amplitudez);
		dummyData.Value() = amplitudez;
	}
}

void OnNewParams(void){

	parameterPeriiod.Update();

	UpdateParams();
	//Returned changed data to the client.
	CDataManager::GetInstance()->SendAllParams();
}

void OnNewSignals(void){}
