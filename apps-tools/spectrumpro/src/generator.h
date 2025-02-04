#pragma once

#include <CustomParameters.h>
#include <DataManager.h>
#include <math.h>
#include <fstream>
#include <limits>
#include <mutex>

void setSweepRun(bool run);
void updateGen(void);
void UpdateGeneratorParameters(bool force);
void appGenInit();
void appGenExit();
