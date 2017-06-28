#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>

#include "hwid.h"
#include "gen.h"

#define M_PI 3.14159265358979323846

int main () {
    printf ("DEBUG: start\n");

    rp_hwid_t hwid;
    rp_hwid_init (&hwid);
    printf ("HWID = 0x%0x\n", rp_hwid_get_hwid(&hwid));
    printf ("DNA  = 0x%016" PRIx64 "\n", rp_hwid_get_dna(&hwid));
    rp_hwid_release (&hwid);

    rp_gen_t gen0;
    rp_gen_init (&gen0, 0);

    rp_gen_default(&gen0);
    // generate and program sinusoidal waveform
    float waveform [gen0.buffer_size];
    const float step = 2 * M_PI / gen0.buffer_size;
    for (int unsigned i=0; i<gen0.buffer_size; i++) {
        waveform[i] = sinf(step * (float) i);
    }
    rp_gen_set_waveform(&gen0, waveform, gen0.buffer_size);
    // set continuous/periodic mode
    rp_gen_set_mode(&gen0, CONTINUOUS);
    rp_asg_per_set_table_size(&gen0.per, gen0.buffer_size);
    rp_asg_gen_set_frequency (&gen0.per, 1000);
    rp_asg_gen_set_phase     (&gen0.per, 0);
    // enable generator output
    rp_gen_out_set_enable    (&gen0.out, true);

    rp_gen_release (&gen0);

    printf ("DEBUG: end\n");
    return(0);
}

