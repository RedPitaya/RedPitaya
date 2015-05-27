
#include "new_main.h"

#include <DataManager.h>
#include <CustomParameters.h>
//#include "../../rp_sdk/include/lcr_app.h"
#include "../../rp_sdk/include/CustomParameters.h"

/***************************************************************************************
*                                     LCR METER                                        *
****************************************************************************************/

//TODO make a more detailed parameters specification. More parameters soon to be added.

CIntParameter startMeasure("START_MEASURE", CBaseParameter::RW, -1, 0, -1, 3);
CFloatParameter genAmplitude("GEN_AMPL", CBaseParameter::RW, 0, 0, 0, 1.0);
CFloatParameter genAveraging("GEN_AVG", CBaseParameter::RW, 0, 0, 0, 10);
CFloatParameter genDcBias("DC_BIAS", CBaseParameter::RW, 0, 0, -2, 2);

CIntParameter measSteps("MEAS_STEPS", CBaseParameter::RW, 100, 0, 1, 1000);
CFloatParameter genStartFreq("GEN_START_FREQ", CBaseParameter::RW, 1000, 0, 0, 1000000);
CFloatParameter genEndFreq("GEN_END_FREQ", CBaseParameter::RW, 10000, 0, 0, 1000000);
CIntParameter plotData("PLOT_DATA", CBaseParameter::RW, 0, 0, 0, 15);
CIntParameter scaleType("SCALE_TYPE", CBaseParameter::RW, 0, 0, 0, 1);
CFloatParameter genLoadRe("LOAD_RE", CBaseParameter::RW, 0, 0, 0, 1);
CFloatParameter genLoadImg("LOAD_IMG", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter genCalibration("GEN_CALIB", CBaseParameter::RW, 0, 0, 0, 3);
CIntParameter genSaveData("SAVE_DATA", CBaseParameter::RW, 0, 0, 0, 1);


