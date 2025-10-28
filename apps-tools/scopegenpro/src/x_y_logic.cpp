#include <CustomParameters.h>
#include <DataManager.h>
#include <math.h>

#include "common.h"
#include "main.h"
#include "rp_hw-profiles.h"
#include "x_y_logic.h"

CFloatBinarySignal x_axis("X_AXIS_VALUES", CH_SIGNAL_SIZE_DEFAULT, 0.0f);
CFloatBinarySignal y_axis("Y_AXIS_VALUES", CH_SIGNAL_SIZE_DEFAULT, 0.0f);

CBooleanParameter xyShow("X_Y_SHOW", CBaseParameter::RW, false, 0, CONFIG_VAR);

CIntParameter xSrc("X_AXIS_SOURCE", CBaseParameter::RW, 0, 0, RPAPP_OSC_SOUR_CH1, RPAPP_OSC_SOUR_MATH, CONFIG_VAR);
CIntParameter ySrc("Y_AXIS_SOURCE", CBaseParameter::RW, 0, 0, RPAPP_OSC_SOUR_CH1, RPAPP_OSC_SOUR_MATH, CONFIG_VAR);

// /* --------------------------------  CURSORS  ------------------------------ */
CBooleanParameter xyCursorXEnable[2] = INIT2("OSC_XY_CURSOR_X", "", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter xyCursorYEnable[2] = INIT2("OSC_XY_CURSOR_Y", "", CBaseParameter::RW, false, 0, CONFIG_VAR);

CFloatParameter xyCursorX[2] = INIT2("OSC_XY_CUR", "_X", CBaseParameter::RW, -1, 0, -1000000000000.0, 1000000000000.0, CONFIG_VAR);
CFloatParameter xyCursorY[2] = INIT2("OSC_XY_CUR", "_Y", CBaseParameter::RW, -1, 0, -1000000000000.0, 1000000000000.0, CONFIG_VAR);

// /* ----------------------------------  MATH  -------------------------------- */
// CIntParameter mathOperation("OSC_MATH_OP", CBaseParameter::RW, RPAPP_OSC_MATH_ADD, RPAPP_OSC_MATH_ADD, RPAPP_OSC_MATH_ADD, RPAPP_OSC_MATH_INT,CONFIG_VAR);
// CIntParameter mathSource[2]             = INIT2("OSC_MATH_SRC","", CBaseParameter::RW, RP_CH_1, 0, RP_CH_1, RP_CH_4,CONFIG_VAR);

auto initXYAfterLoad() -> void {}

auto updateXYParametersToWEB() -> void {}

auto resetXYParams() -> void {}

auto setXYParams() -> void {}

auto isXYShow() -> bool {
    return xyShow.Value();
}

auto updateXYSignal() -> void {
    if (isXYShow()) {
        if (x_axis.GetSize() != CH_SIGNAL_SIZE_DEFAULT)
            x_axis.Resize(CH_SIGNAL_SIZE_DEFAULT);
        if (y_axis.GetSize() != CH_SIGNAL_SIZE_DEFAULT)
            y_axis.Resize(CH_SIGNAL_SIZE_DEFAULT);
        rpApp_OscGetViewDataXY(&x_axis[0], &y_axis[0], (uint32_t)CH_SIGNAL_SIZE_DEFAULT);
    } else {
        x_axis.Resize(0);
        y_axis.Resize(0);
    }
}

auto updateXYParams(bool force) -> void {

    if (IS_NEW(xyShow) || force) {
        if (rpApp_OscSetEnableXY(xyShow.NewValue()) == RP_OK) {
            xyShow.Update();
        }
    }

    if (IS_NEW(xSrc) || force) {
        if (rpApp_OscSetSrcXAxis((rpApp_osc_source)xSrc.NewValue()) == RP_OK) {
            xSrc.Update();
        }
    }

    if (IS_NEW(ySrc) || force) {
        if (rpApp_OscSetSrcYAxis((rpApp_osc_source)ySrc.NewValue()) == RP_OK) {
            ySrc.Update();
        }
    }

    for (int i = 0; i < 2; i++) {
        if (IS_NEW(xyCursorXEnable[i]) || force)
            xyCursorXEnable[i].Update();
        if (IS_NEW(xyCursorYEnable[i]) || force)
            xyCursorYEnable[i].Update();
        if (IS_NEW(xyCursorX[i]) || force)
            xyCursorX[i].Update();
        if (IS_NEW(xyCursorY[i]) || force)
            xyCursorY[i].Update();
    }

    // if (IS_NEW(cursorSrc) || force)
    //     cursorSrc.Update();

    setXYParams();
}
