#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#include "redpitaya/rp1.h"

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
    rp_wave_sin(waveform, gen0.buffer_size);
    rp_gen_set_waveform(&gen0, waveform, gen0.buffer_size);
    // set continuous/periodic mode
    rp_gen_set_mode(&gen0, CONTINUOUS);
    rp_asg_per_set_table_size(&gen0.per, gen0.buffer_size);
    rp_asg_gen_set_frequency (&gen0.per, 1000);
    rp_asg_gen_set_phase     (&gen0.per, 0);
    // enable generator output
    rp_gen_out_set_amplitude (&gen0.out, 0.5);
    rp_gen_out_set_offset    (&gen0.out, 0);
    rp_gen_out_set_enable    (&gen0.out, true);
    // start generator
    rp_evn_reset             (&gen0.evn);
    rp_evn_start             (&gen0.evn);
    rp_evn_trigger           (&gen0.evn);

    // print regset for debug purposes
    rp_gen_print             (&gen0);
    getchar();

    rp_gen_release (&gen0);
    printf ("DEBUG: end\n");
    return(0);
}

