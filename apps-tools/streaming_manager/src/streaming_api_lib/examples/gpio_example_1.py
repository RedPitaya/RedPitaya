#!/usr/bin/python3
# GPIO example 1: the output plays a Gray coded counter on DIO0..7_P and DIO0..6_N while the
# input reads the lines back (an output line reads its own level, so no wires are needed).
#
# The output words are {P lines, N lines, unused, RLE count}: with RLE a word holds its level for
# count + 1 samples. The input words are {N count, N lines, P count, P lines}.
#
# Usage: gpio_example_1.py [IP]    (without IP the first board found on the network)

import sys
import threading
import time

import numpy as np
import streaming


def gray(k):
    return k ^ (k >> 1)


class GpioHandler(streaming.GPIOCallback):
    def __init__(self):
        super().__init__()
        self.lock = threading.Lock()
        self.packs = self.samples = self.words = self.lost = self.changes = 0
        self.last = -1  # the P lines of the last word

    def receivePack(self, client, pack):
        with self.lock:
            self.packs += 1
            self.lost += pack.fpgaLost
            # pack.words: a numpy array (uint32) of the words of the buffer, the whole buffer at once
            w = pack.words
            if w.size == 0:
                return
            p = (w & 0xFF).astype(np.int64)  # the P lines
            self.samples += int((((w >> 8) & 0xFF) + 1).sum()) if pack.rle else int(w.size)
            before = np.concatenate(([self.last], p[:-1]))
            self.changes += int(np.count_nonzero(p != before))
            self.last = int(p[-1])
            self.words += int(w.size)

    def connected(self, client, host):
        print("GPIO connected to", host)

    def error(self, client, host, code):
        print("GPIO error", host, "code", code)

    def outputStopped(self, client, host, waitClocks):
        print(f"GPIO output of {host} stopped, it waited {waitClocks} clocks for data")


def main():
    conf = streaming.ConfigStreamClient()
    gpio = streaming.GPIOStreamClient(conf)
    handler = GpioHandler()
    gpio.setCallback(handler)

    if not (conf.connect([sys.argv[1]]) if len(sys.argv) > 1 else conf.connect()):
        print("No board found")
        return 1
    host = conf.getHosts()[0]

    # the lines, RLE and the network mode of both directions
    for key, value in (("gpio_outputs_p", "255"), ("gpio_outputs_n", "127"), ("gpio_decimation", "1"), ("gpio_rle", "ON"),
                       ("gpio_in_pass_mode", "NET"), ("gpio_out_pass_mode", "NET")):
        conf.sendConfig(host, key, value)

    # 1000 values of the counter, each for 256 samples (RLE count 255), repeated until the stop
    words = [(gray(k) & 0xFF) << 24 | ((gray(k) >> 8) & 0x7F) << 16 | 0xFF for k in range(1000)]
    gpio.setOutputData(words, 0)

    if not gpio.startInput(host) or not gpio.startOutput(host):
        print("GPIO streaming did not start (is there a GPIO block in the FPGA?)")
        gpio.stopStreaming()
        return 1
    print("GPIO streaming for 5 s, Ctrl+C stops it")
    try:
        time.sleep(5)
    except KeyboardInterrupt:
        pass

    gpio.stopStreaming()
    with handler.lock:
        print(f"Input: {handler.packs} buffers, {handler.words} words, {handler.samples} samples, "
              f"{handler.changes} changes of the P lines, {handler.lost} samples lost")
    print(f"Output: waited {gpio.outputWaitClocks()} clocks for data")
    gpio.removeCallback()
    return 0


if __name__ == "__main__":
    sys.exit(main())
