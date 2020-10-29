#!/usr/bin/env python3

import argparse
import csv
import sys
from math import log


def _main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ch0-freq-min", type=float, required=True)
    parser.add_argument("--ch0-freq-max", type=float, required=True)
    parser.add_argument("--ch0-level", type=float, required=True)
    parser.add_argument("--ch1-freq-min", type=float, required=True)
    parser.add_argument("--ch1-freq-max", type=float, required=True)
    parser.add_argument("--ch1-level", type=float, required=True)
    parser.add_argument("--ch-mode", type=str, required=True)
    args = parser.parse_args()
    print("SFDR arguments: ")
    for arg in sorted(vars(args).keys()):
       print ("-",arg," = ", getattr(args, arg))
    reader = csv.reader(iter(sys.stdin.readline, ""))
    header = next(reader)
    del header

    ch0_floor_noise = 0
    ch0_max_noise = -1000
    ch0_min_noise = 0
    ch0_avg_noise = 0
    ch0_carrier_value = -1000

    ch1_floor_noise = 0
    ch1_max_noise = -1000
    ch1_min_noise = 0
    ch1_avg_noise = 0
    ch1_carrier_value = -1000
    
    row_count = 0
    row_ch0_count = 0
    row_ch1_count = 0
    success = True

    for row in reader:
        row_count += 1
        freq = float(row[0].strip())
        ch0_value = float(row[1].strip())
        ch1_value = float(row[2].strip())
	
        if (args.ch_mode == "CH0"  or args.ch_mode == "CH0+1"):
            if ((freq < args.ch0_freq_min) or (freq > args.ch0_freq_max)) and (ch0_value > args.ch0_level):
                success = False
                print("Error SFDR_CH0, freq: {}, meas: {}, max: {}".format(freq, ch0_value, args.ch0_level))
        
        
            if ((freq < args.ch0_freq_min) or (freq > args.ch0_freq_max)):
                if (ch0_max_noise < ch0_value):
                    ch0_max_noise = ch0_value
                if (ch0_min_noise > ch0_value):
                   ch0_min_noise = ch0_value
                ch0_floor_noise += 10**((-ch0_value)/10)
                row_ch0_count += 1
            else:
               if (ch0_carrier_value < ch0_value):
                   ch0_carrier_value = ch0_value
        
            ch0_avg_noise += 10**((-ch0_value)/10)

        if (args.ch_mode == "CH1" or args.ch_mode == "CH0+1"): 
            if ((freq < args.ch1_freq_min) or (freq > args.ch1_freq_max)) and (ch1_value > args.ch1_level):
                success = False
                print("Error SFDR_CH1, freq: {}, meas: {}, max: {}".format(freq, ch1_value, args.ch1_level))

            
            if ((freq < args.ch1_freq_min) or (freq > args.ch1_freq_max)):
                if (ch1_max_noise < ch1_value):
                    ch1_max_noise = ch1_value
                if (ch1_min_noise > ch1_value):
                    ch1_min_noise = ch1_value
                ch1_floor_noise += 10**((-ch1_value)/10)
                row_ch1_count += 1
            else:
                if (ch1_carrier_value < ch1_value):
                    ch1_carrier_value = ch1_value

            ch1_avg_noise += 10**((-ch1_value)/10)

    

    if (args.ch_mode == "CH0"  or args.ch_mode == "CH0+1"):
        ch0_floor_noise = log(ch0_floor_noise/row_ch0_count,10)*-10
        ch0_avg_noise = log(ch0_avg_noise/row_count,10)*-10    
        print ("CH0: carrier value: {}, floor noise: {:.2f}, avg noise: {:.2f}, min noise: {}, max noise: {} ".format(ch0_carrier_value,ch0_floor_noise,ch0_avg_noise, ch0_min_noise,ch0_max_noise))

    if (args.ch_mode == "CH1"  or args.ch_mode == "CH0+1"):    
        ch1_floor_noise = log(ch1_floor_noise/row_ch1_count,10)*-10
        ch1_avg_noise = log(ch1_avg_noise/row_count,10)*-10
        print ("CH1: carrier value: {}, floor noise: {:.2f}, avg noise: {:.2f}, min noise: {}, max noise: {} ".format(ch1_carrier_value,ch1_floor_noise,ch1_avg_noise, ch1_min_noise,ch1_max_noise))
    return success


if __name__ == "__main__":
    if not _main():
        sys.exit(1)
