
#include "main.h"

#include <limits.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <cmath>
#include <map>

#include "common/version.h"
#include "rp_hw_calib.h"
#include "settings.h"
#include "web/rp_client.h"

#define CHECK_NAN_INF(X)                \
    if (std::isnan(X) || std::isinf(X)) \
        X = 0;
#define MIN(X, Y) ((X < Y) ? X : Y)
#define MAX(X, Y) ((X > Y) ? X : Y)

enum controlSettings { NONE = 0, REQUEST_RESET = 1, RESET_DONE = 2, REQUEST_LIST = 3, SAVE = 4, DELETE = 5, LOAD = 6, LOAD_DONE = 7 };

/***************************************************************************************
*                                     LCR METER                                        *
****************************************************************************************/

//Out params
CDoubleParameter lcr_amplitude("LCR_Z", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_Inductance("LCR_L", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_Capacitance("LCR_C", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_Resitance("LCR_R", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_phase("LCR_P", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_D("LCR_D", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_Q("LCR_Q", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_ESR("LCR_ESR", CBaseParameter::RW, 0, 0, -1e15, 1e15);

//Measurement parameters for primary display
CDoubleParameter lcr_AmpMin("LCR_Z_MIN", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_AmpMax("LCR_Z_MAX", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_AmpAvg("LCR_Z_AVG", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_IndMin("LCR_L_MIN", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_IndMax("LCR_L_MAX", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_IndAvg("LCR_L_AVG", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_CapMin("LCR_C_MIN", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_CapMax("LCR_C_MAX", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_CapAvg("LCR_C_AVG", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_ResMin("LCR_R_MIN", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_ResMax("LCR_R_MAX", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter lcr_ResAvg("LCR_R_AVG", CBaseParameter::RW, 0, 0, -1e15, 1e15);

//In params
CDoubleParameter frequency("LCR_FREQ", CBaseParameter::RW, 10, 0, 10, 1e7, CONFIG_VAR);
CIntParameter shuntRMode("LCR_SHUNT_MODE", CBaseParameter::RW, -1, 0, -1, 5, CONFIG_VAR);
CIntParameter shuntR("LCR_SHUNT", CBaseParameter::RW, 0, 0, 0, 5);
CIntParameter calibMode("LCR_CALIB_MODE", CBaseParameter::RW, 0, 0, 0, 3);
CBooleanParameter resetMeasData("LCR_M_RESET", CBaseParameter::RW, false, 0);

CBooleanParameter startMeasure("LCR_RUN", CBaseParameter::RW, true, 0);
CBooleanParameter startCalibration("LCR_CALIBRATION", CBaseParameter::RW, false, 0);

CBooleanParameter toleranceMode("LCR_TOLERANCE", CBaseParameter::RW, false, 0);
CBooleanParameter relativeMode("LCR_RELATIVE", CBaseParameter::RW, false, 0);
CDoubleParameter toleranceValue("LCR_TOLERANCE_VALUE", CBaseParameter::RW, 0, 0, -1e15, 1e15);
CDoubleParameter relativeValue("LCR_RELATIVE_VALUE", CBaseParameter::RW, 0, 0, -1e15, 1e15);

CIntParameter seriesMode("LCR_SERIES", CBaseParameter::RW, 1, 0, 0, 1, CONFIG_VAR);

CIntParameter rangeMode("LCR_RANGE", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);
CIntParameter rangeFormat("LCR_RANGE_F", CBaseParameter::RW, 0, 0, 0, 3, CONFIG_VAR);
CIntParameter rangeUnits("LCR_RANGE_U", CBaseParameter::RW, 0, 0, 0, 6, CONFIG_VAR);

CIntParameter primDisplay("LCR_PRIM_DISP", CBaseParameter::RW, 0, 0, 0, 3, CONFIG_VAR);
CIntParameter secDisplay("LCR_SEC_DISP", CBaseParameter::RW, 0, 0, 0, 3, CONFIG_VAR);

CIntParameter logInterval("LOG_INTERVAL", CBaseParameter::RW, 100, 0, 0, 30000000, CONFIG_VAR);

CBooleanParameter moduleStatus("LCR_EXTMODULE_STATUS", CBaseParameter::RW, true, 0);

CIntParameter controlSettings("CONTROL_CONFIG_SETTINGS", CBaseParameter::RW, 0, 0, 0, 10);

// Cyclic Buffers for calculate averenge values
const static int buffSize = 32;
CStatistics lcr_AmpBuff(buffSize, -1e15, 1e15);
CStatistics lcr_IndBuff(buffSize, -1e15, 1e15);
CStatistics lcr_ResBuff(buffSize, -1e15, 1e15);
CStatistics lcr_CapBuff(buffSize, -1e15, 1e15);

double toleranceSave[4] = {0, 0, 0, 0};
double relativeSave[4] = {0, 0, 0, 0};
bool requestSaveTolerance = false;
bool requestSaveRelative = false;

const std::vector<std::string> g_savedParams;

bool g_config_changed = false;
uint16_t g_save_counter = 0;  // By default, a save check every 40 ticks. One tick 50 ms.

void updateFromFront(bool force);

const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya Lcr meter application.\n";
}

void updateParametersByConfig() {
    configGet();
    updateFromFront(true);
}

int rp_app_init(void) {
    fprintf(stderr, "Loading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);

    setHomeSettingsPath("/.config/redpitaya/apps/lcr_meter/");
    CDataManager::GetInstance()->SetParamInterval(20);

    rp_WC_Init();

    lcrApp_lcrInit();

    if (rp_HPGetFastADCIsAC_DCOrDefault()) {
        rp_AcqSetAC_DC(RP_CH_1, RP_DC);
        rp_AcqSetAC_DC(RP_CH_2, RP_DC);
    }

    lcrApp_LcrRun();
    updateParametersByConfig();
    return 0;
}

int rp_app_exit(void) {
    lcrApp_LcrRelease();
    fprintf(stderr, "Unloading lcr meter version %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

int rp_set_params(rp_app_params_t* p, int len) {
    return 0;
}

int rp_get_params(rp_app_params_t** p) {
    return 0;
}

int rp_get_signals(float*** s, int* sig_num, int* sig_len) {
    return 0;
}

void UpdateSignals(void) {}

// Send values to frontend
void UpdateParams(void) {
    bool moduleStatusFlag = lcrApp_LcrCheckExtensionModuleConnection(false) == RP_OK;
    if (moduleStatus.Value() != moduleStatusFlag) {
        moduleStatus.SendValue(moduleStatusFlag);
        TRACE_SHORT("------------> Module Status sended %d", moduleStatusFlag);
    }

    lcr_shunt_t shuntValue;
    if (lcrApp_LcrGetShunt(&shuntValue) == RP_LCR_OK) {
        if (shuntR.Value() != shuntValue) {
            shuntR.SendValue(shuntValue);
        }
    }

    if (startMeasure.Value() == true) {
        lcr_main_data_t data;
        //Acquire calculated parameters frm RP
        lcrApp_LcrCopyParams(&data);

        CHECK_NAN_INF(data.lcr_amplitude)
        CHECK_NAN_INF(data.lcr_L)
        CHECK_NAN_INF(data.lcr_R)
        CHECK_NAN_INF(data.lcr_C)
        CHECK_NAN_INF(data.lcr_D)
        CHECK_NAN_INF(data.lcr_Q)
        CHECK_NAN_INF(data.lcr_ESR)

        data.lcr_amplitude *= 100000.0;
        data.lcr_L *= 100000.0;
        data.lcr_R *= 100000.0;
        data.lcr_C *= 100000.0;
        data.lcr_phase *= 100000.0;
        data.lcr_D *= 100000.0;
        data.lcr_Q *= 100000.0;
        data.lcr_ESR *= 100000.0;

        lcr_Capacitance.SendValue(data.lcr_C);
        lcr_amplitude.SendValue(data.lcr_amplitude);
        lcr_phase.SendValue(data.lcr_phase);
        lcr_Inductance.SendValue(data.lcr_L);
        lcr_Resitance.SendValue(data.lcr_R);
        lcr_D.SendValue(data.lcr_D);
        lcr_Q.SendValue(data.lcr_Q);
        lcr_ESR.SendValue(data.lcr_ESR);

        // Cyclic Buffers for calculate averenge values
        lcr_AmpBuff.add(data.lcr_amplitude);
        lcr_IndBuff.add(data.lcr_L);
        lcr_ResBuff.add(data.lcr_R);
        lcr_CapBuff.add(data.lcr_C);

        // min, max, avg
        lcr_AmpMin.SendValue(lcr_AmpBuff.min());
        lcr_AmpMax.SendValue(lcr_AmpBuff.max());
        lcr_AmpAvg.SendValue(lcr_AmpBuff.avg());

        lcr_CapMin.SendValue(lcr_CapBuff.min());
        lcr_CapMax.SendValue(lcr_CapBuff.max());
        lcr_CapAvg.SendValue(lcr_CapBuff.avg());

        lcr_IndMin.SendValue(lcr_IndBuff.min());
        lcr_IndMax.SendValue(lcr_IndBuff.max());
        lcr_IndAvg.SendValue(lcr_IndBuff.avg());

        lcr_ResMin.SendValue(lcr_ResBuff.min());
        lcr_ResMax.SendValue(lcr_ResBuff.max());
        lcr_ResAvg.SendValue(lcr_ResBuff.avg());

        if (requestSaveTolerance) {
            toleranceSave[0] = MAX(0, lcr_AmpBuff.avg());
            toleranceSave[1] = MAX(0, lcr_IndBuff.avg());
            toleranceSave[2] = MAX(0, lcr_CapBuff.avg());
            toleranceSave[3] = MAX(0, lcr_ResBuff.avg());
            requestSaveTolerance = false;
        }

        if (requestSaveRelative) {
            relativeSave[0] = lcr_AmpBuff.avg();
            relativeSave[1] = lcr_IndBuff.avg();
            relativeSave[2] = lcr_CapBuff.avg();
            relativeSave[3] = lcr_ResBuff.avg();
            requestSaveRelative = false;
        }

        if (toleranceMode.Value()) {
            auto index = primDisplay.Value();
            auto diff = 0.0;
            switch (index) {
                case 0:
                    diff = toleranceSave[index] == 0 ? 0 : ((MAX(0, lcr_AmpBuff.avg()) - toleranceSave[index]) / toleranceSave[index]) * 100.0;
                    break;
                case 1:
                    diff = toleranceSave[index] == 0 ? 0 : ((MAX(0, lcr_IndBuff.avg()) - toleranceSave[index]) / toleranceSave[index]) * 100.0;
                    break;
                case 2:
                    diff = toleranceSave[index] == 0 ? 0 : ((MAX(0, lcr_CapBuff.avg()) - toleranceSave[index]) / toleranceSave[index]) * 100.0;
                    break;
                case 3:
                    diff = toleranceSave[index] == 0 ? 0 : ((MAX(0, lcr_ResBuff.avg()) - toleranceSave[index]) / toleranceSave[index]) * 100.0;
                    break;
                default:
                    break;
            }
            toleranceValue.SendValue(diff);
        }

        if (relativeMode.Value()) {
            auto index = primDisplay.Value();
            auto diff = 0.0;
            switch (index) {
                case 0:
                    diff = (lcr_AmpBuff.avg() - relativeSave[index]);
                    break;
                case 1:
                    diff = (lcr_IndBuff.avg() - relativeSave[index]);
                    break;
                case 2:
                    diff = (lcr_CapBuff.avg() - relativeSave[index]);
                    break;
                case 3:
                    diff = (lcr_ResBuff.avg() - relativeSave[index]);
                    break;
                default:
                    break;
            }
            relativeValue.SendValue(diff);
        }
    }

    if (g_config_changed && (g_save_counter++ % 40 == 0)) {
        g_config_changed = false;
        // Save the configuration file
        configSet();
    }
}

void updateFromFront(bool force) {

    /*Change frequency*/
    if (IS_NEW(frequency) || force) {
        lcrApp_LcrSetFrequency(frequency.NewValue());
        lcrApp_GenRun();
        frequency.Update();
    }

    /*Change shunt*/
    if (IS_NEW(shuntRMode) || force) {
        lcr_shunt_t shunt = (lcr_shunt_t)shuntRMode.NewValue();
        if (shunt == RP_LCR_S_NOT_INIT) {  // Auto mode
            lcrApp_LcrSetShuntIsAuto(true);
        } else {
            lcrApp_LcrSetShuntIsAuto(false);
            lcrApp_LcrSetShunt(shunt);
        }
        shuntRMode.Update();
    }

    /* Change calibration mode */
    if (IS_NEW(calibMode)) {
        calib_t calibration = (calib_t)calibMode.NewValue();
        lcrApp_LcrSetCalibMode(calibration);
        calibMode.Update();
    }

    if (IS_NEW(startMeasure)) {
        startMeasure.Update();
    }

    if (IS_NEW(resetMeasData)) {
        if (resetMeasData.NewValue()) {
            //Rest min saved val
            lcr_AmpMin.SendValue(1e9);
            lcr_IndMin.SendValue(1e9);
            lcr_ResMin.SendValue(1e9);
            lcr_CapMin.SendValue(1e9);

            lcr_AmpMax.SendValue(0);
            lcr_IndMax.SendValue(0);
            lcr_ResMax.SendValue(0);
            lcr_CapMax.SendValue(0);

            //Reset avg saved val
            lcr_AmpAvg.SendValue(0);
            lcr_IndAvg.SendValue(0);
            lcr_ResAvg.SendValue(0);
            lcr_CapAvg.SendValue(0);

            lcr_AmpBuff.clear();
            lcr_IndBuff.clear();
            lcr_ResBuff.clear();
            lcr_CapBuff.clear();
        }
        resetMeasData.Update();
        resetMeasData.Value() = false;
    }

    //Set calibration
    if (IS_NEW(startCalibration) && calibMode.Value() != 0) {
        // TODO
        // lcrApp_LcrStartCorrection();
        startCalibration.Update();
        startCalibration.Value() = false;
    }

    if (IS_NEW(relativeMode)) {
        if (relativeMode.NewValue()) {
            requestSaveRelative = true;
        }
        relativeMode.Update();
    }

    if (IS_NEW(toleranceMode)) {
        if (toleranceMode.NewValue()) {
            requestSaveTolerance = true;
        }
        toleranceMode.Update();
    }

    if (IS_NEW(seriesMode) || force) {
        lcrApp_LcrSetMeasSeries(seriesMode.NewValue());
        seriesMode.Update();
    }

    if (IS_NEW(rangeFormat) || force) {
        rangeFormat.Update();
    }

    if (IS_NEW(rangeUnits) || force) {
        rangeUnits.Update();
    }

    if (IS_NEW(rangeMode) || force) {
        rangeMode.Update();
    }

    if (IS_NEW(primDisplay) || force) {
        primDisplay.Update();
        primDisplay.NeedSend(true);
    }

    if (IS_NEW(secDisplay) || force) {
        secDisplay.Update();
        secDisplay.NeedSend(true);
    }

    if (IS_NEW(logInterval) || force) {
        logInterval.Update();
    }
}

void OnNewParams(void) {
    if (controlSettings.IsNewValue()) {
        if (controlSettings.NewValue() == controlSettings::REQUEST_RESET) {
            deleteConfig();
            configSetWithList(g_savedParams);
            controlSettings.Update();
            controlSettings.SendValue(controlSettings::RESET_DONE);
            return;
        }
    }

    if (!g_config_changed)
        g_config_changed = isChanged();
    updateFromFront(false);
}

void OnNewSignals(void) {}

void PostUpdateSignals(void) {}
