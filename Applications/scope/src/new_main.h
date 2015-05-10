#pragma once

#define CH_SIGNAL_SIZE_DEFAULT		1024

#define IF_VALUE_CHANGED(X, ACTION) \
if (X.Value() != X.NewValue()) { \
    if (ACTION) { \
        X.Update(); } }


float getMeasureValue(int measure);