Red Pitaya Bode analyzer

MANPAGE:
Bode analyzer version 0.25, compiled at Mon Sep 29 12:02:42 2014

Usage:  bode [channel] [amplitude] [dc bias] [averaging] [count/steps] [start freq] [stop freq] [scale type]

        channel            Channel to generate signal on [1 / 2].
        amplitude          Signal amplitude in V [0 - 1, which means max 2Vpp].
        dc bias            DC bias/offset/component in V [0 - 1].
                           Max sum of amplitude and DC bias is 1V.
        averaging          Number of samples per one measurement [>1].
        count/steps        Number of measurements [>2].
        start freq         Lower frequency limit in Hz [3 - 62.5e6].
        stop freq          Upper frequency limit in Hz [3 - 62.5e6].
        scale type         0 - linear, 1 - logarithmic.

Output: frequency [Hz], phase [deg], amplitude [dB]
