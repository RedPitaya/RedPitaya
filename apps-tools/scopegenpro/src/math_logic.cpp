#include <DataManager.h>
#include <CustomParameters.h>
#include <math.h>

#include "math_logic.h"
#include "rp_hw-profiles.h"
#include "common.h"
#include "main.h"


constexpr float DEF_MIN_SCALE = 1.f/1000.f;
constexpr float DEF_MAX_SCALE = 5.f;

CFloatBase64Signal  math("math", CH_SIGNAL_SIZE_DEFAULT, 0.0f);

CBooleanParameter   mathShow("MATH_SHOW", CBaseParameter::RW, false, 0,CONFIG_VAR);
CBooleanParameter   mathInvShow("MATH_SHOW_INVERTED", CBaseParameter::RW, false, 0,CONFIG_VAR);

CFloatParameter     inMathOffset("OSC_MATH_OFFSET", CBaseParameter::RW, 0, 0, -50000000, 50000000,CONFIG_VAR);
CFloatParameter     inMathScale("OSC_MATH_SCALE", CBaseParameter::RW, 1, 0, 0.00005, 1000000000000.0,CONFIG_VAR);


/* --------------------------------  MEASURE  ------------------------------ */
CStringParameter    measureSelectN("OSC_MEAS_SELN", CBaseParameter::RW, "[]", 0,CONFIG_VAR);
CIntParameter       measureSelect[4]    = INIT("OSC_MEAS_SEL","", CBaseParameter::RW, -1, 0, -1, 100000,CONFIG_VAR);
CFloatParameter     measureValue[4]     = INIT("OSC_MEAS_VAL","", CBaseParameter::RWSA, 0, 0, -1000000000, 1000000000);

/* --------------------------------  CURSORS  ------------------------------ */
CBooleanParameter   cursorx[2]          = INIT2("OSC_CURSOR_X","", CBaseParameter::RW, false, 0,CONFIG_VAR);
CBooleanParameter   cursory[2]          = INIT2("OSC_CURSOR_Y","", CBaseParameter::RW, false, 0,CONFIG_VAR);

CFloatParameter     cursorV[2]          = INIT2("OSC_CUR","_V", CBaseParameter::RW, -1, 0, -1000000000000.0, 1000000000000.0,CONFIG_VAR);
CFloatParameter     cursorT[2]          = INIT2("OSC_CUR","_T", CBaseParameter::RW, -1, 0, -1000000000000.0, 1000000000000.0,CONFIG_VAR);

/* ----------------------------------  MATH  -------------------------------- */
CIntParameter mathOperation("OSC_MATH_OP", CBaseParameter::RW, RPAPP_OSC_MATH_ADD, RPAPP_OSC_MATH_ADD, RPAPP_OSC_MATH_ADD, RPAPP_OSC_MATH_INT,CONFIG_VAR);
CIntParameter mathSource[2]             = INIT2("OSC_MATH_SRC","", CBaseParameter::RW, RP_CH_1, 0, RP_CH_1, RP_CH_4,CONFIG_VAR);

CStringParameter    mathName("MATH_CHANNEL_NAME_INPUT", CBaseParameter::RW, "MATH", 0,CONFIG_VAR);


auto initMathAfterLoad() -> void{

}

auto updateMathParametersToWEB(bool is_auto_scale) -> void{
    for(int i = 0; i < 4; i++){
        if (measureSelect[i].Value() != -1) {
            auto val = getMeasureValue(measureSelect[i].Value());
            if (measureValue[i].Value() != val){
                measureValue[i].SendValue(val);
            }
        }
    }

    double dvalue = 0;
    rpApp_OscGetAmplitudeScale(RPAPP_OSC_SOUR_MATH, &dvalue);
    WARNING("\t\ttinMathScale %f dvalue %f min %f max %f",inMathScale.Value(), dvalue ,inMathScale.GetMin() , inMathScale.GetMax())

    if (dvalue < inMathScale.GetMin() || dvalue > inMathScale.GetMax()){
        WARNING("inMathScale Rescale");
        rpApp_OscScaleMath();
    }else{
        if(fabs(inMathScale.Value()) - fabs(dvalue) > 0.0005) {
//            fprintf(stderr,"\tSend %f\n",inMathScale.Value());
            inMathScale.SendValue(dvalue);
        }
    }

    rpApp_OscGetAmplitudeOffset(RPAPP_OSC_SOUR_MATH, &dvalue);
    if (inMathOffset.Value() != dvalue){
        inMathOffset.SendValue(dvalue);
    }

    checkMathScale();
}


auto resetMathParams() -> void {
    inMathScale.SendValue(1.f);
    rpApp_OscSetAmplitudeScale(RPAPP_OSC_SOUR_MATH, 1.f);

    inMathOffset.SendValue(0.f);
    rpApp_OscSetAmplitudeOffset(RPAPP_OSC_SOUR_MATH, 0);
}

auto setMathParams() -> void {
    if (IS_NEW(inMathScale)){
        auto val = inMathScale.CheckMinMax(inMathScale.NewValue());
        if (rpApp_OscSetAmplitudeScale(RPAPP_OSC_SOUR_MATH, val ) == RP_OK){
            inMathScale.Update();
            inMathScale.Value() = val;
        }
    }

    if (IS_NEW(inMathOffset)){
        if (rpApp_OscSetAmplitudeOffset(RPAPP_OSC_SOUR_MATH, inMathOffset.NewValue()) == RP_OK){
            inMathOffset.Update();
        }
    }
}

auto checkMathScale() -> void {
    if(mathShow.Value()) {
        float vpp = 0;
        rpApp_OscMeasureVpp(RPAPP_OSC_SOUR_MATH, &vpp);
        double mul = 10;
        for(int i = 0 ; i < 10; i++ ){
            if (mul > vpp)
                break;
            mul *= 10.0;

        }

        if (inMathScale.GetMax() != mul){
            inMathScale.SetMax(mul);
        }

        if (inMathScale.GetMin() != (mul / 1000.0)){
            inMathScale.SetMin(mul / 1000.0);
        }
    }
}

auto isMathShow() -> bool{
    return mathShow.Value();
}

auto updateMathSignal() -> void{
    if (mathShow.Value()) {
        if (math.GetSize() != CH_SIGNAL_SIZE_DEFAULT)
            math.Resize(CH_SIGNAL_SIZE_DEFAULT);
        rpApp_OscGetViewData(RPAPP_OSC_SOUR_MATH, &math[0], (uint32_t) CH_SIGNAL_SIZE_DEFAULT);

    } else {
        math.Resize(0);
    }
}

auto updateMathParams(bool force) -> void{


    if(IS_NEW(mathShow) || force) {
        mathShow.Update();
    }

    for(int i = 0; i < 4 ; i++){
        if (IS_NEW(measureSelect[i]) || force)
            measureSelect[i].Update();
    }

    if (IS_NEW(measureSelectN) || force)
        measureSelectN.Update();

    if (IS_NEW(mathName) || force){
        if (mathName.NewValue() == ""){
            auto str = mathName.Value();
            mathName.Update();
            mathName.SendValue(str);
        }else{
            mathName.Update();
        }
    }

    for(int i = 0; i < 2 ; i++){
        if (IS_NEW(cursorx[i]) || force)
            cursorx[i].Update();
        if (IS_NEW(cursory[i]) || force)
            cursory[i].Update();
        if (IS_NEW(cursorV[i]) || force)
            cursorV[i].Update();
        if (IS_NEW(cursorT[i]) || force)
            cursorT[i].Update();
    }

    setMathParams();

    if (IS_NEW(mathOperation) || IS_NEW(mathSource[0]) || IS_NEW(mathSource[1]) || force) {
        if (rpApp_OscSetMathOperation((rpApp_osc_math_oper_t) mathOperation.NewValue()) == RP_OK)
		    mathOperation.Update();
        if (rpApp_OscSetMathSources((rp_channel_t) mathSource[0].NewValue(), (rp_channel_t) mathSource[1].NewValue()) == RP_OK) {
            mathSource[0].Update();
            mathSource[1].Update();
        }
        if (!force)
            resetMathParams();
    }

    IF_VALUE_CHANGED_FORCE(inMathOffset, rpApp_OscSetAmplitudeOffset(RPAPP_OSC_SOUR_MATH, inMathOffset.NewValue()),force)

    IF_VALUE_CHANGED_FORCE(mathInvShow, rpApp_OscSetInverted(RPAPP_OSC_SOUR_MATH, mathInvShow.NewValue()),force)
    checkMathScale();
}
