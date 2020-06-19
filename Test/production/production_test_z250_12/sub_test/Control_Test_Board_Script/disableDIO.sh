#!/bin/bash
    monitor 0x40000010 w 0x0FFF
    sleep 0.2
    # SET N pins in IN mode
    monitor 0x40000014 w 0x0FFF
    sleep 0.2

    # SET P pins in 0 values
    monitor 0x40000018 0x0000
    sleep 0.2
    # SET N pins in 0 values
    monitor 0x4000001C 0x0000
