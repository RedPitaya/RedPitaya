#!/bin/bash

    monitor 0x40000010 w 0x0200 # -> Set N to outputs
    sleep 0.2
    monitor 0x40000018 w 0x0200 # ->  Set DIO7_N = 1
