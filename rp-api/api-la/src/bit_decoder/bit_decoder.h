/**
 * $Id: $
 *
 * @brief Red Pitaya library logic analizer api
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __BIT_DECODER_API_H
#define __BIT_DECODER_API_H

#include <stdint.h>
#include <string>
#include <vector>

namespace bit_decoder {

struct Bit {
    bool valid = false;
    bool bitValue = false;
    double bitSampleStart = 0;
    double bitSampleEnd = 0;
    static auto print(Bit& bit) -> void {
        fprintf(stderr, "Bit valid %d value %d start %f end %f\n", bit.valid, bit.bitValue, bit.bitSampleStart, bit.bitSampleEnd);
    };
};

}  // namespace bit_decoder

#endif  // __BIT_DECODER_API_H
