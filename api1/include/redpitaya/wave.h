#ifndef WAVE_H
#define WAVE_H

int rp_wave_sin (float waveform[], int unsigned len);              // sinusoidal
int rp_wave_squ (float waveform[], int unsigned len, float dcyc);  // square
int rp_wave_tri (float waveform[], int unsigned len, float dcyc);  // triangle

#endif
