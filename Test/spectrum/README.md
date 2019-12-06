# Run application
```
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
LD_LIBRARY_PATH=/opt/redpitaya/lib spectrum
```

# Usage
* -h, --help: help
* -m, --min: minimum frequency (default: 0)
* -M, --max: maximum frequency (default: 62500000)
* -c, --count: iteration count (default: 1, negative: infinity)
* -a, --average: average the measurement from 10 times (default: enabled)
* -n, --no-average: disable average the measurement from 10 times
* -C, --csv: print values by columns Frequency (Hz), ch0 (dB), ch1 (dB)
* -L, --csv-limit: print values by columns Frequency (Hz), ch0 min (dB), ch0 max (dB), ch1 min (dB), ch1 max (dB)
* -t, --test: test mode avoids the initiating/resetting/releasing FPGA

# Examples
```
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
LD_LIBRARY_PATH=/opt/redpitaya/lib spectrum -m 5000 -M 20000 -c 10

ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak: 9994.14 Hz, -5.66 dB
ch1 peak: 14991.21 Hz, -5.98 dB
ch0 peak summary: 9994.14 Hz, -5.66 dB
ch1 peak summary: 14991.21 Hz, -5.98 dB
```

```
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
LD_LIBRARY_PATH=/opt/redpitaya/lib spectrum -m 5000 -M 20000 -C

Frequency (Hz), ch0 (dB), ch1 (dB)
4997.07, -82.60, -82.13
5004.52, -85.25, -85.39
5011.96, -96.31, -92.84
5019.41, -98.83, -93.82
5026.86, -95.01, -88.88
5034.31, -92.14, -86.11
5041.75, -92.64, -89.86
5049.20, -97.11, -97.61
5056.65, -101.76, -96.45
5064.09, -90.20, -99.99
...
19936.15, -91.16, -103.93
19943.60, -94.67, -103.84
19951.04, -110.36, -90.97
19958.49, -98.84, -90.27
19965.94, -94.09, -96.89
19973.38, -93.11, -102.65
19980.83, -87.56, -96.00
19988.28, -80.81, -89.86
19995.73, -82.90, -91.80
20003.17, -90.31, -95.15
```

```
cat /opt/redpitaya/fpga/fpga_0.94.bit > /dev/xdevcfg
LD_LIBRARY_PATH=/opt/redpitaya/lib spectrum -m 5000 -M 20000 -L -c 10

Frequency (Hz), ch0 min (dB), ch0 max (dB), ch1 min (dB), ch1 max (dB)
4997.07, -86.13, -79.79, -84.41, -80.12
5004.52, -88.61, -85.56, -91.46, -82.50
5011.96, -101.85, -87.46, -109.80, -87.67
5019.41, -99.89, -87.81, -100.97, -85.96
5026.86, -102.96, -85.72, -105.65, -85.56
5034.31, -97.97, -86.37, -106.42, -89.44
5041.75, -98.22, -86.68, -113.84, -87.04
5049.20, -112.02, -87.81, -109.86, -85.26
5056.65, -102.35, -87.70, -100.77, -87.54
5064.09, -103.42, -87.38, -104.93, -89.51
...
19936.15, -114.66, -84.41, -99.82, -83.39
19943.60, -106.68, -88.86, -100.68, -84.00
19951.04, -103.79, -89.17, -112.73, -88.91
19958.49, -100.00, -85.89, -107.91, -88.63
19965.94, -110.12, -90.15, -113.49, -87.20
19973.38, -103.56, -87.70, -100.68, -86.50
19980.83, -103.51, -82.43, -102.41, -86.18
19988.28, -82.96, -77.75, -95.58, -84.13
19995.73, -87.12, -79.13, -98.90, -82.72
20003.17, -101.35, -87.47, -106.83, -88.50
```
