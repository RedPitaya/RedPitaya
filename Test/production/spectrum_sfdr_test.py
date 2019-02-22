#!/usr/bin/env python3

import argparse
import csv
import sys


def _main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ch0-freq-min", type=float, required=True)
    parser.add_argument("--ch0-freq-max", type=float, required=True)
    parser.add_argument("--ch0-level", type=float, required=True)
    parser.add_argument("--ch1-freq-min", type=float, required=True)
    parser.add_argument("--ch1-freq-max", type=float, required=True)
    parser.add_argument("--ch1-level", type=float, required=True)
    args = parser.parse_args()

    reader = csv.reader(iter(sys.stdin.readline, ""))
    header = next(reader)
    del header

    success = True

    for row in reader:
        freq = float(row[0].strip())
        ch0_value = float(row[1].strip())
        ch1_value = float(row[2].strip())

        if ((freq < args.ch0_freq_min) or (freq > args.ch0_freq_max)) and (ch0_value > args.ch0_level):
            success = False
            print("Error SFDR_CH0, freq: {}, meas: {}, max: {}".format(freq, ch0_value, args.ch0_level))

        if ((freq < args.ch1_freq_min) or (freq > args.ch1_freq_max)) and (ch1_value > args.ch1_level):
            success = False
            print("Error SFDR_CH1, freq: {}, meas: {}, max: {}".format(freq, ch1_value, args.ch1_level))

    return success


if __name__ == "__main__":
    if not _main():
        sys.exit(1)
