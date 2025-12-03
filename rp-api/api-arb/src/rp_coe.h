/**
 * $Id: $
 *
 * @brief Red Pitaya library arb api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
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
