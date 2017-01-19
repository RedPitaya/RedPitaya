#!/usr/bin/python3

from redpitaya import *
import sys
from time import sleep

led = RP_LED0;
if len(sys.argv) > 1:
    led += int(sys.argv[1])

print("Blinking LED[%d]" % (led))

base.Init()

for _ in range(1000):
    base.DpinSetState(led, RP_HIGH)
    sleep(1)
    base.DpinSetState(led, RP_LOW)
    sleep(1)

base.Release()
