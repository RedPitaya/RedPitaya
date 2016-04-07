/**
 * @brief Red Pitaya RadioBox gain compensation module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <math.h>

#include "rp_gain_compensation.h"


const rb_gain_params_t g_rb_gain_params_hw_1v1[RB_GAIN_PARAMS_HW_1V1_NUM] = {
    //         R50       open
    {1e-12,   1e-16,    1e-16 },
    { 1e-6,   1e-10,    1e-10 },
    { 1e-1,    1e-5,     1e-5 },
    {1.2e-1,   0.11,     0.11 },
    {1.5e-1,   0.14,     0.14 },
    { 2e-1,    0.21,     0.21 },
    { 5e-1,    0.35,     0.38 },
    {  1e0,    0.70,     0.71 },
    {  2e0,    0.94,     1.01 },
    {  5e0,    1.02,     1.11 },
    {  1e1,    1.12,     1.07 },
    {  1e2,    1.04,     1.07 },
    {  1e3,    1.04,     1.08 },
    {  1e4,    1.03,     1.08 },
    {  1e5,    1.02,     1.06 },
    {  1e6,    1.01,     1.06 },
    {  2e6,    1.01,     1.05 },
    {  3e6,    1.01,     1.05 },
    {  4e6,    1.01,     1.08 },
    {  5e6,    1.01,     1.12 },
    {  6e6,    1.01,     1.20 },
    {  7e6,    1.02,     1.33 },
    {  8e6,    1.02,     1.43 },
    {  9e6,    1.02,     1.57 },
    { 10e6,    1.02,     1.81 },
    { 11e6,    1.03,     2.00 },
    { 12e6,    1.03,     2.38 },
    { 13e6,    1.03,     2.93 },
    { 14e6,    1.03,     3.94 },
    { 14.25e6, 1.03,     4.17 },
    { 14.50e6, 1.03,     4.55 },
    { 14.75e6, 1.03,     5.20 },
    { 15e6,    1.03,     5.68 },
    { 15.25e6, 1.03,     6.41 },
    { 15.50e6, 1.03,     7.32 },
    { 15.75e6, 1.03,     8.45 },
    { 16e6,    1.03,     9.69 },
    { 16.25e6, 1.03,    11.10 },
    { 16.50e6, 1.03,    12.60 },
    { 16.75e6, 1.03,    13.40 },
    { 17e6,    1.03,    13.20 },
    { 17.25e6, 1.02,    11.90 },
    { 17.50e6, 1.02,    10.30 },
    { 17.75e6, 1.02,     8.96 },
    { 18e6,    1.02,     7.60 },
    { 18.25e6, 1.02,     6.64 },
    { 18.50e6, 1.02,     5.87 },
    { 18.75e6, 1.02,     5.24 },
    { 19e6,    1.02,     4.74 },
    { 19.25e6, 1.02,     4.40 },
    { 19.50e6, 1.02,     4.04 },
    { 19.75e6, 1.02,     3.72 },
    { 20e6,    1.01,     3.45 },
    { 21e6,    1.01,     2.72 },
    { 22e6,    1.01,     2.23 },
    { 23e6,    1.00,     1.98 },
    { 24e6,    1.00,     1.72 },
    { 25e6,    1.00,     1.54 },
    { 26e6,    0.99,     1.43 },
    { 27e6,    0.99,     1.33 },
    { 28e6,    0.98,     1.27 },
    { 29e6,    0.98,     1.21 },
    { 30e6,    0.99,     1.17 },
    { 31e6,    0.99,     1.16 },
    { 32e6,    1.00,     1.15 },
    { 33e6,    1.00,     1.15 },
    { 34e6,    1.01,     1.17 },
    { 35e6,    1.02,     1.20 },
    { 36e6,    1.03,     1.26 },
    { 37e6,    1.04,     1.32 },
    { 38e6,    1.04,     1.41 },
    { 39e6,    1.03,     1.55 },
    { 40e6,    1.02,     1.74 },
    { 41e6,    1.01,     1.99 },
    { 42e6,    0.99,     2.39 },
    { 43e6,    0.97,     2.98 },
    { 43.25e6, 0.96,     3.17 },
    { 43.50e6, 0.95,     3.39 },
    { 43.75e6, 0.95,     3.63 },
    { 44e6,    0.95,     3.92 },
    { 44.25e6, 0.94,     4.21 },
    { 44.50e6, 0.93,     4.45 },
    { 44.75e6, 0.92,     4.72 },
    { 45e6,    0.92,     4.94 },
    { 45.25e6, 0.91,     5.11 },
    { 45.50e6, 0.90,     5.13 },
    { 45.75e6, 0.89,     5.06 },
    { 46e6,    0.89,     4.87 },
    { 46.25e6, 0.88,     4.62 },
    { 46.50e6, 0.88,     4.33 },
    { 46.75e6, 0.87,     4.05 },
    { 47e6,    0.85,     3.73 },
    { 47.25e6, 0.85,     3.43 },
    { 47.50e6, 0.84,     3.16 },
    { 47.75e6, 0.83,     2.92 },
    { 48e6,    0.82,     2.72 },
    { 49e6,    0.78,     2.12 },
    { 50e6,    0.76,     1.66 },
    { 51e6,    0.73,     1.38 },
    { 52e6,    0.72,     1.20 },
    { 53e6,    0.68,     1.05 },
    { 54e6,    0.66,     0.96 },
    { 55e6,    0.64,     0.89 },
    { 56e6,    0.63,     0.85 },
    { 57e6,    0.63,     0.83 },
    { 58e6,    0.63,     0.83 },
    { 59e6,    0.62,     0.90 },
    { 60e6,    0.62,     1.04 },
    { 61e6,    0.63,     1.26 },
    { 62e6,    0.63,     1.88 },
    { 63e6,    0.63,     1.88 },  // repeat last entry 3-times
    { 63e6,    0.63,     1.88 },
    { 63e6,    0.63,     1.88 }
};


/*----------------------------------------------------------------------------------*/
int bspline_j_k_n(int j, int k, int n)
{
    if (j < k)
        return 0;
    else if (j > n)
        return n - k + 2;
    else
        return j - k + 1;
}

/*----------------------------------------------------------------------------------*/
float bspline_n_i_k(int i, int k, float t)
{
    int i_p1        = i + 1;
    int i_pk_m1     = i + k - 1;
    int i_pk        = i + k;

    int j           = bspline_j_k_n(i,       RB_GAIN_PARAMS_BSPLINE_K, RB_GAIN_PARAMS_HW_1V1_NUM - 1);
    int j_p1        = bspline_j_k_n(i_p1,    RB_GAIN_PARAMS_BSPLINE_K, RB_GAIN_PARAMS_HW_1V1_NUM - 1);
    int j_pk_m1     = bspline_j_k_n(i_pk_m1, RB_GAIN_PARAMS_BSPLINE_K, RB_GAIN_PARAMS_HW_1V1_NUM - 1);
    int j_pk        = bspline_j_k_n(i_pk,    RB_GAIN_PARAMS_BSPLINE_K, RB_GAIN_PARAMS_HW_1V1_NUM - 1);

    float t_i       = log10(g_rb_gain_params_hw_1v1[j      ].frequency_hz);
    float t_i_p1    = log10(g_rb_gain_params_hw_1v1[j_p1   ].frequency_hz);
    float t_i_pk_m1 = log10(g_rb_gain_params_hw_1v1[j_pk_m1].frequency_hz);
    float t_i_pk    = log10(g_rb_gain_params_hw_1v1[j_pk   ].frequency_hz);

    float q1;
    if (!(t_i_pk_m1 - t_i))
        q1 = 0.0;
    else
        q1 = (t - t_i) / (t_i_pk_m1 - t_i);

    float q2;
    if (!(t_i_pk - t_i_p1))
        q2 = 0.0;
    else
        q2 = (t_i_pk - t) / (t_i_pk - t_i_p1);

    float r = 0.0f;  // k < 1 is not specified
    if (k == 1)
        r = ((t_i <= t) && (t < t_i_p1)) ?  1.0f : 0.0f;
    else if (k > 1)
        r = q1 * bspline_n_i_k(i, k - 1, t) + q2 * bspline_n_i_k(i + 1, k - 1, t);

#if 0
    fprintf(stderr, "DEBUG bspline_n_i_k: k=%d - i=%02d, i_p1=%02d, i_pk_m1=%02d, i_pk=%02d\n",
            k, i, i_p1, i_pk_m1, i_pk);
    fprintf(stderr, "DEBUG bspline_n_i_k: k=%d - j=%02d, j_p1=%02d, j_pk_m1=%02d, j_pk=%02d\n",
            k, j, j_p1, j_pk_m1, j_pk);
    fprintf(stderr, "DEBUG bspline_n_i_k: k=%d - (t      - t_i)=%+12.3f, (t_i_pk_m1 - t_i   )=%+12.3f;\t t     =%12.3f, t_i=%12.3f, t_i_pk_m1=%12.3f\n",
            k, t - t_i   , t_i_pk_m1 - t_i   ,  t     , t_i, t_i_pk_m1);
    fprintf(stderr, "DEBUG bspline_n_i_k: k=%d - (t_i_pk - t  )=%+12.3f, (t_i_pk    - t_i_p1)=%+12.3f;\t t_i_pk=%12.3f, t  =%12.3f, t_i_p1   =%12.3f\n",
            k, t_i_pk - t, t_i_pk    - t_i_p1,  t_i_pk, t  , t_i_p1   );
    fprintf(stderr, "DEBUG bspline_n_i_k: k=%d - q1=%+9.3f, q2=%+9.3f\n",
            k, q1, q2);
    fprintf(stderr, "DEBUG bspline_n_i_k: k=%d --> return %+9.3f\n",
            k, r);
#endif
    return r;
}

/*----------------------------------------------------------------------------------*/
float get_compensation_factor(float frequency_hz, int isTerminated)
{
    if (!frequency_hz) {
        return 0.0;  // marks the gain correction block to switch off
    }

    if (frequency_hz < 1e-2f) {
        frequency_hz = 1e-2f;
    } else if (frequency_hz > 62.5e6f) {
        frequency_hz = 62.5e6f;
    }

    // B-Spline calculation follows as explained there: @see http://www-lehre.informatik.uni-osnabrueck.de/~cg/2000/skript/7_4_B_Splines.html
    float bspline_p = 0.0f;
    int bspline_n = RB_GAIN_PARAMS_HW_1V1_NUM - 1;

    int bspline_i;
    for (bspline_i = 0; bspline_i <= bspline_n; bspline_i++) {  // Sigma over 0 to n
        int bspline_i_m1 = bspline_i - 1;
        if (bspline_i_m1 < 0)
            bspline_i_m1 = 0;

        float bspline_nik = bspline_n_i_k(bspline_i, RB_GAIN_PARAMS_BSPLINE_K, log10f(frequency_hz));
        float bspline_p_i = isTerminated ?  g_rb_gain_params_hw_1v1[bspline_i_m1].gain_terminated50R :
                                            g_rb_gain_params_hw_1v1[bspline_i_m1].gain_openEnd       ;

        float bspline_p_i_f = bspline_nik * bspline_p_i;

        bspline_p += bspline_p_i_f;
        //fprintf(stderr, "DEBUG get_compensation_factor: bspline_i=%2d, bspline_i_m1=%2d - frequency=%+12.3f, weight=%+6.3f, gain=%+6.3f --> part=%+6.3f\n",
        //        bspline_i, bspline_i_m1, g_rb_gain_params_hw_1v1[bspline_i_m1].frequency_hz, bspline_nik, bspline_p_i, bspline_p_i_f);
    }

    if (bspline_p < 1e-6f) {  // out of table --> no correction
        bspline_p = 1.0f;
    }
    //fprintf(stderr, "DEBUG get_compensation_factor: in(frequency_hz=%f, isTerminated=%d) with spline_k=%d --> out(gain=%f, correction=%f)\n",
    //        frequency_hz, isTerminated, RB_GAIN_PARAMS_BSPLINE_K, bspline_p, 1.0/ bspline_p);

    return 1.0f / bspline_p;
}
