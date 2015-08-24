#pragma once

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