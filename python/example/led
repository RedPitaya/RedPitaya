#!/usr/bin/python
from time import sleep

from PyRedPitaya.board import RedPitaya

red_pitaya = RedPitaya()

def disp_bar(i):
    red_pitaya.hk.led = 2**(i+1)-1

if __name__=="__main__":
    i = 0
    while True:
        i = (i+1)%8
        disp_bar(i)
        sleep(0.03)


