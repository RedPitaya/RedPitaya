#include <math.h>

#define M_PI 3.14159265358979323846

// sinusoidal
int rp_wave_sin (float waveform[], int unsigned len) {
    const float step = 2 * M_PI / len;
    for (int unsigned i=0; i<len; i++) {
        waveform[i] = sinf(step * (float) i);
    }
    return (0);
}

// square
int rp_wave_squ (float waveform[], int unsigned len, float dcyc) {
    if ((dcyc < 0) || (dcyc > 1)) {
        return (-1);
    }
    int unsigned idcyc = (int unsigned) (dcyc * (float) len);
    for (int unsigned i=0; i<len; i++) {
        waveform[i] = (i < idcyc) ? +1.0 : -1.0;
    }
    return (0);
}

// triangle
int rp_wave_tri (float waveform[], int unsigned len, float dcyc) {
    if ((dcyc < 0) || (dcyc > 1)) {
        return (-1);
    }
//    int unsigned idcyc = (int unsigned) (dcyc * (float) len);
    // TODO: add duty cycle handler
    for (int unsigned i=0; i<len; i++) {
        waveform[i] = (float) i * 2.0 - 1.0;
    }
    return (0);
}

