Red Pitaya LCR meter

TODO:
- waiting functionality
- calibration results have to be saved on pitaya and later used for calculating results
- DC_bias has to be tested
- calibration selection functionality testing

MANPAGE:
LCR meter version 0.25, compiled at Tue Sep 30 23:54:00 2014

Usage:  lcr [channel] [amplitude] [dc bias] [r_shunt] [averaging] [calibration mode] [z_ref real] [z_ref imag] [count/steps] [sweep mode] [start freq] [stop freq] [scale type] [wait]

        channel            Channel to generate signal on [1 / 2].
        amplitude          Signal amplitude in V [0 - 1, which means max 2Vpp].
        dc bias            DC bias/offset/component in V [0 - 1].
                           Max sum of amplitude and DC bias is 1V.
        r_shunt            Shunt resistor value in Ohms [>0].
        averaging          Number of samples per one measurement [>1].
        calibration mode   0 - none, 1 - open and short, 2 - z_ref.
        z_ref real         Reference impedance, real part.
        z_ref imag         Reference impedance, imaginary part.
        count/steps        Number of measurements [>1 / >2, dep. on sweep mode].
        sweep mode         0 - measurement sweep, 1 - frequency sweep.
        start freq         Lower frequency limit in Hz [3 - 62.5e6].
        stop freq          Upper frequency limit in Hz [3 - 62.5e6].
        scale type         0 - linear, 1 - logarithmic.
        wait               Wait for user before performing each step [0 / 1].

Output: frequency [Hz], phase [deg], Z [Ohm], Y, PhaseY, R_s, X_s, G_p, B_p, C_s, C_p, L_s, L_p, R_p, Q, D
