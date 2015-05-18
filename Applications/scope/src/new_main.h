#pragma once

#define CH_SIGNAL_SIZE_DEFAULT		1024
#define CALIB_FE_LV_REF_V           1.0f
#define CALIB_FE_HV_REF_V           5.0f

#define IF_VALUE_CHANGED(X, ACTION) \
if (X.Value() != X.NewValue()) { \
    if (ACTION) { \
        X.Update(); } }

#define IF_VALUE_CHANGED_BOOL(X, ACTION1, ACTION2) \
if (X.Value() != X.NewValue()) { \
    if (X.NewValue()) { \
        ACTION1;    X.Update(); \
    } else { \
        ACTION2;    X.Update(); }}

float getMeasureValue(int measure);