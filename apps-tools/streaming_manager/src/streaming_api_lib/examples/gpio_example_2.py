#!/usr/bin/python3
# GPIO example 2: the GPIO input into a CSV file on this computer, a row per run of a level:
#   lane,start,length,level      lane P or N, start and length in samples, level in hex
# With RLE the P and N lanes count their runs on their own: each lane has its own time line.
#
# Usage: gpio_example_2.py [IP] [FILE] [SECONDS]   (defaults: the first board found, gpio.csv, 5)

import sys
import threading
import time

import streaming


class Lane:
    """One lane: the runs of its level, a row each when the level changes."""

    def __init__(self, name, out):
        self.name, self.out = name, out
        self.position = 0  # the next sample
        self.start = 0     # of the current run
        self.level = -1

    def take(self, value, samples):
        if value != self.level:
            if self.level >= 0:
                self.out.write(f"{self.name},{self.start},{self.position - self.start},0x{self.level:02X}\n")
            self.level = value
            self.start = self.position
        self.position += samples

    def finish(self):
        """The run still open at the end."""
        if self.level >= 0 and self.position > self.start:
            self.out.write(f"{self.name},{self.start},{self.position - self.start},0x{self.level:02X}\n")
        self.level = -1


class CsvWriter(streaming.GPIOCallback):
    def __init__(self, out):
        super().__init__()
        self.lock = threading.Lock()
        out.write("lane,start,length,level\n")
        self.p, self.n = Lane("P", out), Lane("N", out)
        self.lost = 0

    def receivePack(self, client, pack):
        with self.lock:
            gap = pack.fpgaLost + pack.fpgaWait  # lost or waited for a trigger
            self.p.position += gap
            self.n.position += gap
            self.lost += pack.fpgaLost
            for w in pack.words:
                # {N count, N lines, P count, P lines}; without RLE a word is one sample
                self.p.take(w & 0xFF, ((w >> 8) & 0xFF) + 1 if pack.rle else 1)
                self.n.take((w >> 16) & 0xFF, ((w >> 24) & 0xFF) + 1 if pack.rle else 1)

    def error(self, client, host, code):
        print("GPIO error", host, "code", code)


def main():
    path = sys.argv[2] if len(sys.argv) > 2 else "gpio.csv"
    seconds = int(sys.argv[3]) if len(sys.argv) > 3 else 5

    conf = streaming.ConfigStreamClient()
    if not (conf.connect([sys.argv[1]]) if len(sys.argv) > 1 else conf.connect()):
        print("No board found")
        return 1
    host = conf.getHosts()[0]
    conf.sendConfig(host, "gpio_decimation", "1")
    conf.sendConfig(host, "gpio_rle", "ON")  # runs of a level in one word: far less data
    conf.sendConfig(host, "gpio_in_pass_mode", "NET")

    with open(path, "w") as out:
        gpio = streaming.GPIOStreamClient(conf)
        csv = CsvWriter(out)
        gpio.setCallback(csv)
        if not gpio.startInput(host):
            print("The GPIO input did not start")
            return 1
        print(f"Recording the GPIO input of {host} into {path} for {seconds} s")
        try:
            time.sleep(seconds)
        except KeyboardInterrupt:
            pass
        gpio.stopInput()
        gpio.removeCallback()
        with csv.lock:
            csv.p.finish()
            csv.n.finish()
            print(f"Done: {csv.p.position} samples, {csv.lost} lost")
    return 0


if __name__ == "__main__":
    sys.exit(main())
