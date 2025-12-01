/**
 * $Id: $
 *
 * @brief Red Pitaya library arb api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __ARB_COE_API_H
#define __ARB_COE_API_H

#include <string>
#include <vector>
#include "rp.h"

using namespace std;

enum Radix { HEX, DEC, BIN, UNKNOWN };

struct CoeData {
    Radix radix;
    vector<double> data;
};

int rp_ARBParseCoeFile(const string& filename, CoeData* data);

#endif
