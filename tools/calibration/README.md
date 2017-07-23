# Calibration

We wish to store the same calibration values as they were measured. This requires following a certain procedure order and in hardware the values should also be processed in a specific order.

## Hardware design



## Signal generator

### Full range

The generator full range should be from -1V to +1V. The DAC digital input should be set to the maximum positive and negative value and the analog output should be measured:
```
V_gen_max
V_gen_min
```

The gain is an unitless value:
```
gen_gain = (V_gen_max - V_gen_min) / (1V - -1V) = (V_gen_max - V_gen_min) / 2V
```

### Zero offset

The generator zero offset should be 0V. The DAC digital input should be set to zero and the alanlog output should be measured:
```
V_gen_off
```

The offset is a voltage:
```
gen_offset = -Vgen
```

## Osciloscope

### 
