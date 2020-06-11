#!/bin/bash

    monitor 0x40000014 w 0x0100 # -> Set N to outputs
    sleep 0.2
    monitor 0x4000001C w 0x0100 # ->  Set DIO7_N = 1
