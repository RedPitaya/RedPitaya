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

#include "rp_gain_compensation.h"


const rb_gain_params_t g_rb_gain_params_hw_1v1[RB_GAIN_PARAMS_HW_1V1_NUM] = {

	{  0,    0,     0 },
	{  1,    1,     1 },
    {  2,    2,     2 },
    {  3,    3,     3 },
    {  4,    4,     4 },
    {  5,    5,     5 }
#if 0
    {    0,    0.85,     1.12 },
    {1e-12,    0.85,     1.12 },
    {  1e0,    0.85,     1.12 },
    {  1e6,    1.08,     1.12 },
    {  2e6,    1.06,     1.14 },
    {  3e6,    1.06,     1.15 },
    {  4e6,    1.06,     1.22 },
    {  5e6,    1.08,     1.28 },
    {  6e6,    1.08,     1.36 },
    {  7e6,    1.08,     1.44 },
    {  8e6,    1.08,     1.52 },
    {  9e6,    1.08,     1.64 },
    { 10e6,    1.08,     1.82 },
    { 11e6,    1.08,     2.04 },
    { 12e6,    1.08,     2.42 },
    { 13e6,    1.08,     2.92 },
    { 14e6,    1.08,     3.76 },
    { 15e6,    1.08,     5.36 },
    { 16e6,    1.08,     9.00 },
    { 17e6,    1.06,    14.00 },
    { 18e6,    1.06,     9.40 },
    { 19e6,    1.05,     6.00 },
    { 20e6,    1.04,     3.76 },
    { 21e6,    1.04,     2.91 },
    { 22e6,    1.03,     2.40 },
    { 23e6,    1.03,     2.03 },
    { 24e6,    1.03,     1.83 },
    { 25e6,    1.02,     1.62 },
    { 26e6,    1.02,     1.52 },
    { 27e6,    1.02,     1.42 },
    { 28e6,    1.02,     1.32 },
    { 29e6,    1.02,     1.26 },
    { 30e6,    1.02,     1.22 },
    { 31e6,    1.04,     1.19 },
    { 32e6,    1.05,     1.18 },
    { 33e6,    1.05,     1.18 },
    { 34e6,    1.05,     1.19 },
    { 35e6,    1.05,     1.21 },
    { 36e6,    1.06,     1.25 },
    { 37e6,    1.06,     1.32 },
    { 38e6,    1.06,     1.39 },
    { 39e6,    1.06,     1.54 },
    { 40e6,    1.06,     1.70 },
    { 41e6,    1.04,     1.92 },
    { 42e6,    1.02,     2.20 },
    { 43e6,    1.01,     2.72 },
    { 44e6,    0.99,     3.52 },
    { 45e6,    0.96,     4.56 },
    { 46e6,    0.92,     5.04 },
    { 47e6,    0.88,     4.24 },
    { 48e6,    0.86,     3.08 },
    { 49e6,    0.83,     2.35 },
    { 50e6,    0.79,     1.80 },
    { 51e6,    0.76,     1.54 },
    { 52e6,    0.73,     1.26 },
    { 53e6,    0.70,     1.09 },
    { 54e6,    0.69,     0.96 },
    { 55e6,    0.68,     0.90 },
    { 56e6,    0.66,     0.85 },
    { 57e6,    0.66,     0.82 },
    { 58e6,    0.65,     0.84 },
    { 59e6,    0.66,     0.91 },
    { 60e6,    0.65,     1.06 },
    { 61e6,    0.66,     1.40 },
    { 62e6,    0.66,     1.80 }
#endif
};


#if 0
/*----------------------------------------------------------------------------------*/
int get_table_pos_int(float frequency_hz)
{
    int   idx = RB_GAIN_PARAMS_HW_1V1_NUM;
    float frequency_leftSide  = 0.0;
    float frequency_rightSide = g_rb_gain_params_hw_1v1[idx - 1][RB_GAIN_PARAMS_FREQUENCY];

    for (;;) {
        if (--idx < 0)
            return -1;

        frequency_leftSide = g_rb_gain_params_hw_1v1[idx][RB_GAIN_PARAMS_FREQUENCY];
        if (frequency_leftSide <= frequency_hz) {
            // *frac = (frequency_hz - frequency_leftSide) / (frequency_rightSide - frequency_leftSide);
            return idx;
        }

        frequency_rightSide = frequency_leftSide;
    }
    return -2;
}
#endif

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

    float t_i       = g_rb_gain_params_hw_1v1[j      ].frequency_hz;
    float t_i_p1    = g_rb_gain_params_hw_1v1[j_p1   ].frequency_hz;
    float t_i_pk_m1 = g_rb_gain_params_hw_1v1[j_pk_m1].frequency_hz;
    float t_i_pk    = g_rb_gain_params_hw_1v1[j_pk   ].frequency_hz;

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
    fprintf(stderr, "DEBUG bspline_n_i_k: k=%d --> return %+3.1f\n",
            k, r);
    return r;
}

/*----------------------------------------------------------------------------------*/
float get_compensation_factor(float frequency_hz, char isTerminated)
{
    fprintf(stderr, "\nDEBUG get_compensation_factor: in(frequency_hz=%f, isTerminated=%d) with spline_k=%d\n", frequency_hz, isTerminated, RB_GAIN_PARAMS_BSPLINE_K);

    if (frequency_hz < 1e-12f) {
        frequency_hz = 1e-12f;
    } else if (frequency_hz > 62.5e6f) {
        frequency_hz = 62.5e6f;
    }

    // B-Spline calculation follows as explained there: @see http://www-lehre.informatik.uni-osnabrueck.de/~cg/2000/skript/7_4_B_Splines.html
    float bspline_p = 0.0f;
    int bspline_n = RB_GAIN_PARAMS_HW_1V1_NUM - 1;

    int bspline_i;
    for (bspline_i = 0; bspline_i <= bspline_n; bspline_i++) {  // Sigma over 0 to n
        int bspline_j = bspline_j_k_n(bspline_i, RB_GAIN_PARAMS_BSPLINE_K, RB_GAIN_PARAMS_HW_1V1_NUM - 1);
        float bspline_nik = bspline_n_i_k(bspline_i, RB_GAIN_PARAMS_BSPLINE_K, frequency_hz);
        float bspline_p_i = isTerminated ?  g_rb_gain_params_hw_1v1[bspline_i].gain_terminated50R :
                                            g_rb_gain_params_hw_1v1[bspline_i].gain_openEnd       ;

        float bspline_p_i_f = bspline_nik * bspline_p_i;

        bspline_p += bspline_p_i_f;
        fprintf(stderr, "DEBUG get_compensation_factor: bspline_i=%2d, bspline_j=%2d - frequency=%+12.3f, weight=%+6.3f, gain=%+6.3f --> part=%+6.3f\n",
                bspline_i, bspline_j, g_rb_gain_params_hw_1v1[bspline_i].frequency_hz, bspline_nik, bspline_p_i, bspline_p_i_f);
    }

    if (bspline_p < 1e-3f) {
        bspline_p = 1e-3f;
    }
    fprintf(stderr, "DEBUG get_compensation_factor: --> out(gain=%f, correction=%f)\n", bspline_p, 1.0/ bspline_p);

    return 1.0f / bspline_p;
}
