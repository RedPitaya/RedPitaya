
#include "main.h"

#include <limits.h>
#include <cmath>
#include <stdio.h>
#include <sys/syslog.h>
#include <map>

#include "common/version.h"
#include "rp_hw-calib.h"

#define CHECK_NAN_INF(X) if (std::isnan(X) || std::isinf(X)) X=0;

/***************************************************************************************
*                                     LCR METER                                        *
****************************************************************************************/
//TODO make a more detailed parameters specification.
CBooleanParameter IsDemoParam("is_demo", CBaseParameter::RO, false, 1);
CIntParameter parameterPeriiod("DEBUG_PARAM_PERIOD", CBaseParameter::RW, 200, 0, 0, 100);

//Out params
CDoubleParameter lcr_amplitude("LCR_Z", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CDoubleParameter lcr_Inductance("LCR_L", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CDoubleParameter lcr_Capacitance("LCR_C", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CDoubleParameter lcr_Resitance("LCR_R", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CDoubleParameter lcr_phase("LCR_P", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CDoubleParameter lcr_D("LCR_D", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CDoubleParameter lcr_Q("LCR_Q", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CDoubleParameter lcr_ESR("LCR_ESR", CBaseParameter::RW, 0, 0, -1e9, 1e9);
CBooleanParameter lcr_is_sine("LCR_IS_SINE", CBaseParameter::RW, false, 1);

CDoubleParameter lcr_C_precision("LCR_C_PREC", CBaseParameter::RW, 0, 0, 0, 10);

//Measurement parameters for primary display
CDoubleParameter lcr_AmpMin("LCR_Z_MIN", CBaseParameter::RW, 1e9, 0, 0, 1e9);
CDoubleParameter lcr_AmpMax("LCR_Z_MAX", CBaseParameter::RW, 0, 0, 0, 1e9);
CDoubleParameter lcr_AmpAvg("LCR_Z_AVG", CBaseParameter::RW, 0, 0, 0, 1e9);
CDoubleParameter lcr_IndMin("LCR_L_MIN", CBaseParameter::RW, 1e9, 0, 0, 1e9);
CDoubleParameter lcr_IndMax("LCR_L_MAX", CBaseParameter::RW, 0, 0, 0, 1e9);
CDoubleParameter lcr_IndAvg("LCR_L_AVG", CBaseParameter::RW, 0, 0, 0, 1e9);
CDoubleParameter lcr_CapMin("LCR_C_MIN", CBaseParameter::RW, 1e9, 0, 0, 1e9);
CDoubleParameter lcr_CapMax("LCR_C_MAX", CBaseParameter::RW, 0, 0, 0, 1e9);
CDoubleParameter lcr_CapAvg("LCR_C_AVG", CBaseParameter::RW, 0, 0, 0, 1e9);
CDoubleParameter lcr_ResMin("LCR_R_MIN", CBaseParameter::RW, 1e9, 0, 0, 1e9);
CDoubleParameter lcr_ResMax("LCR_R_MAX", CBaseParameter::RW, 0, 0, 0, 1e9);
CDoubleParameter lcr_ResAvg("LCR_R_AVG", CBaseParameter::RW, 0, 0, 0, 1e9);

//In params
CDoubleParameter frequency("LCR_FREQ", CBaseParameter::RW, 10, 0, 10, 1e7);
CIntParameter shuntR("LCR_SHUNT", CBaseParameter::RW, -1, 0, -1, 5);
CIntParameter   calibMode("LCR_CALIB_MODE", CBaseParameter::RW, 0, 0, 0, 3);
CBooleanParameter resetMeasData("LCR_M_RESET", CBaseParameter::RW, false, 0);

CBooleanParameter startMeasure("LCR_RUN", CBaseParameter::RW, true, 0);
CBooleanParameter startCalibration("LCR_CALIBRATION", CBaseParameter::RW, false, 0);
CIntParameter   toleranceMode("LCR_TOLERANCE", CBaseParameter::RW, 0, 0, 0, 4);
CIntParameter   relativeMode("LCR_RELATIVE", CBaseParameter::RW, 0, 0, 0, 4);
CDoubleParameter relSaved("LCR_REL_SAVED", CBaseParameter::RW, 0, 0, 0, 1e6);
CDoubleParameter tolSavedZ("LCR_TOL_SAVED", CBaseParameter::RW, 0, 0, 0, 1e6);
CBooleanParameter seriesMode("LCR_SERIES", CBaseParameter::RW, true, 0);

CIntParameter   rangeMode("LCR_RANGE", CBaseParameter::RW, 0, 0, 0, 4);
CIntParameter   rangeFormat("LCR_RANGE_F", CBaseParameter::RW, 0, 0, 0, 3);
CIntParameter   rangeUnits("LCR_RANGE_U", CBaseParameter::RW, 0, 0, 0, 6);

CBooleanParameter moduleStatus("LCR_EXTMODULE_STATUS", CBaseParameter::RW, true, 0);

// Cyclic Buffers for calculate averenge values
const static int buffSize = 31; // NOTE: is it good value?
CStatistics lcr_AmpBuff(buffSize, 1e0, 1e7);
CStatistics lcr_IndBuff(buffSize, 1e-7, 1e3);
CStatistics lcr_ResBuff(buffSize, 1e0, 1e7);
CStatistics lcr_CapBuff(buffSize, 1e-12, 1e-1);

void flog(char *s){
    FILE *out = fopen("/tmp/debug.log", "a+");
    fprintf(out, "%s", s);
    fclose(out);
}

const char *rp_app_desc(void){
    return (const char *)"Red Pitaya Lcr meter application.\n";
}


int rp_app_init(void){
    fprintf(stderr, "Loading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);

    CDataManager::GetInstance()->SetParamInterval(parameterPeriiod.Value());
    lcrApp_lcrInit();

    if (rp_HPGetFastADCIsAC_DCOrDefault()){
        rp_AcqSetAC_DC(RP_CH_1,RP_DC);
        rp_AcqSetAC_DC(RP_CH_2,RP_DC);
    }

    lcrApp_LcrRun();
    lcrApp_LcrSetFrequency(frequency.Value());
    lcrApp_GenRun();
    return 0;
}

int rp_app_exit(void){
    lcrApp_LcrRelease();
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
    bool moduleStatusFlag = false;

    moduleStatusFlag = lcrApp_LcrCheckExtensionModuleConnection(false) == RP_OK;

    if(moduleStatus.Value() != moduleStatusFlag) {
        moduleStatus.Value() = moduleStatusFlag;
        moduleStatus.SendValue(moduleStatusFlag);
        fprintf(stderr, "------------> Module Status sended\n");
    }
    lcr_main_data_t *data = (lcr_main_data_t *)malloc(sizeof(lcr_main_data_t));

    /*Change frequency*/
    if(IS_NEW(frequency)){
        lcrApp_LcrSetFrequency(frequency.NewValue());
        lcrApp_GenRun();
        frequency.Update();
    }
    /*Change shunt*/
    if(IS_NEW(shuntR)){
        lcr_shunt_t shunt = (lcr_shunt_t)shuntR.NewValue();
        if(shunt == RP_LCR_S_NOT_INIT) {
            lcrApp_LcrSetShuntIsAuto(true);
        } else {
            lcrApp_LcrSetShuntIsAuto(false);
            lcrApp_LcrSetShunt(shunt);
        }
        shuntR.Update();
    }

    /*Change is_sine status*/
    lcr_is_sine.Value() = true;

    /*Get current shunt*/
    lcr_shunt_t shuntValue;
    lcrApp_LcrGetShunt(&shuntValue);
    shuntR.Value() = shuntValue;
    /* Change calibration mode */
    if(IS_NEW(calibMode)){
        calib_t calibration = (calib_t)calibMode.NewValue();
        lcrApp_LcrSetCalibMode(calibration);
        calibMode.Update();
    }

    if(startMeasure.NewValue() == true){
        startMeasure.Update();
        //Acquire calculated parameters frm RP
        lcrApp_LcrCopyParams(data);

        CHECK_NAN_INF(data->lcr_amplitude)
        CHECK_NAN_INF(data->lcr_L)
        CHECK_NAN_INF(data->lcr_R)
        CHECK_NAN_INF(data->lcr_C)
        CHECK_NAN_INF(data->lcr_D)
        CHECK_NAN_INF(data->lcr_Q)
        CHECK_NAN_INF(data->lcr_ESR)

        //Precision casting
        if(data->lcr_C < 0.0001) {
            lcr_C_precision.Value() = 9;
        } else {
            lcr_C_precision.Value() = 0;
        }
        lcr_Capacitance.Value() = data->lcr_C * pow(10, lcr_C_precision.Value());
        lcr_Capacitance.Update();
        lcr_C_precision.Update();
        lcr_amplitude.Value()   = data->lcr_amplitude; lcr_amplitude.Update();
        lcr_phase.Value()       = data->lcr_phase; lcr_phase.Update();
        lcr_Inductance.Value()  = data->lcr_L * 100000.0; lcr_Inductance.Update();
        lcr_Resitance.Value()   = data->lcr_R; lcr_Resitance.Update();
        lcr_D.Value()           = data->lcr_D; lcr_D.Update();
        lcr_Q.Value()           = data->lcr_Q; lcr_Q.Update();
        lcr_ESR.Value()         = data->lcr_ESR; lcr_ESR.Update();

        // Cyclic Buffers for calculate averenge values
        lcr_AmpBuff.add(data->lcr_amplitude);
        lcr_IndBuff.add(data->lcr_L);
        lcr_ResBuff.add(data->lcr_R);
        lcr_CapBuff.add(data->lcr_C);

        // min, max, avg
        lcr_AmpMin.Value() = lcr_AmpBuff.min();
        lcr_AmpMin.Update();
        lcr_AmpMax.Value() = lcr_AmpBuff.max();
        lcr_AmpMax.Update();
        lcr_AmpAvg.Value() = lcr_AmpBuff.avg();
        lcr_AmpAvg.Update();

        lcr_CapMin.Value() = lcr_CapBuff.min() * pow(10, lcr_C_precision.Value());
        if(lcr_CapMin.Value() > 1e9) lcr_CapMin.Value() = 1e9;
        lcr_CapMin.Update();
        lcr_CapMax.Value() = lcr_CapBuff.max() * pow(10, lcr_C_precision.Value());
        lcr_CapMax.Update();
        lcr_CapAvg.Value() = lcr_CapBuff.avg() * pow(10, lcr_C_precision.Value());
        lcr_CapAvg.Update();

        lcr_IndMin.Value() = lcr_IndBuff.min() * 100000;
        lcr_IndMin.Update();
        lcr_IndMax.Value() = lcr_IndBuff.max() * 100000;
        lcr_IndMax.Update();
        lcr_IndAvg.Value() = lcr_IndBuff.avg() * 100000;
        lcr_IndAvg.Update();

        lcr_ResMin.Value() = lcr_ResBuff.min();
        lcr_ResMin.Update();
        lcr_ResMax.Value() = lcr_ResBuff.max();
        lcr_ResMax.Update();
        lcr_ResAvg.Value() = lcr_ResBuff.avg();
        lcr_ResAvg.Update();

        //Relative mode
        switch(relativeMode.Value()){
        case 0:
            break;
        case 1:
            lcr_amplitude.Value() = relSaved.Value() - lcr_amplitude.Value();
            break;
        case 2:
            lcr_Inductance.Value() = relSaved.Value() - lcr_Inductance.Value();
            break;
        case 3:
            lcr_Capacitance.Value() = relSaved.Value() - lcr_Capacitance.Value();
            break;
        case 4:
            lcr_Resitance.Value() = relSaved.Value() - lcr_Resitance.Value();
            break;
        }

    } else if (startMeasure.NewValue() == false) {
        startMeasure.Update();
        lcr_amplitude.Value()   = 0;
        lcr_amplitude.Update();
        lcr_phase.Value()       = 0;
        lcr_phase.Update();
        lcr_Inductance.Value()  = 0;
        lcr_Inductance.Update();
        lcr_Capacitance.Value() = 0;
        lcr_Capacitance.Update();
        lcr_Resitance.Value()   = 0;
        lcr_Resitance.Update();
        lcr_D.Value()           = 0;
        lcr_D.Update();
        lcr_Q.Value()           = 0;
        lcr_Q.Update();
        lcr_ESR.Value()         = 0;
        lcr_ESR.Update();
    }

    if(resetMeasData.NewValue() == true){
        //Rest min saved val
        lcr_AmpMin.Value() = 1e9;
        lcr_AmpMin.Update();
        lcr_IndMin.Value() = 1e9;
        lcr_IndMin.Update();
        lcr_ResMin.Value() = 1e9;
        lcr_ResMin.Update();
        lcr_CapMin.Value() = 1e9;
        lcr_CapMin.Update();

        //Reset max saved val
        lcr_AmpMax.Value() = 0;
        lcr_AmpMax.Update();
        lcr_IndMax.Value() = 0;
        lcr_IndMax.Update();
        lcr_ResMax.Value() = 0;
        lcr_ResMax.Update();
        lcr_CapMax.Value() = 0;
        lcr_CapMax.Update();

        //Reset avg saved val
        lcr_AmpAvg.Value() = 0;
        lcr_AmpAvg.Update();
        lcr_IndAvg.Value() = 0;
        lcr_IndAvg.Update();
        lcr_ResAvg.Value() = 0;
        lcr_ResAvg.Update();
        lcr_CapAvg.Value() = 0;
        lcr_CapAvg.Update();

        lcr_AmpBuff.clear();
        lcr_IndBuff.clear();
        lcr_ResBuff.clear();
        lcr_CapBuff.clear();

        resetMeasData.Value() = false;
        resetMeasData.Update();
    }


    //Set calibration
    if(IS_NEW(startCalibration) && calibMode.Value() != 0){
        //Set correction mode;
        // TODO
        // lcrApp_LcrStartCorrection();
        startCalibration.Update();
        startCalibration.Value() = false;
    }

    if(IS_NEW(relativeMode)){

        //lcrApp_LcrSetMeasRelative(relativeMode.NewValue());
        switch(relativeMode.NewValue()){
        case 0:
            break;
        case 1:
            relSaved.Value() = lcr_amplitude.Value();
            break;
        case 2:
            relSaved.Value() = lcr_Inductance.Value();
            break;
        case 3:
            relSaved.Value() = lcr_Capacitance.Value();
            break;
        case 4:
            relSaved.Value() = lcr_Resitance.Value();
            break;
        }

        relSaved.Update();
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

    //Set app to digital loop if in demo mode
    rp_EnableDigitalLoop(IsDemoParam.Value());

    free(data);
}

void OnNewParams(void){

    parameterPeriiod.Update();

    UpdateParams();
    //Returned changed data to the client.
    CDataManager::GetInstance()->SendAllParams();
}

void OnNewSignals(void){}

void PostUpdateSignals(void){}
