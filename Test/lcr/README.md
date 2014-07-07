
developer: CimeM
All changes made on 1.7.2014 till 20:35

Unformal comments:
lcr.c was created. The code doesent work yet because i had to finish fast. 
There are some backups of lcr.c that i made along the way. 
I have only cross compiled it but didnt test on red pitaya yet. 
Basicaly the acquire.c and generate.c are combined together and some parts of the algorythm of the lock in method is written.

folowed files were copied from Test/acquire/ and from Test/generate:
/RedPitaya/Test/lcr/acquire.c
/RedPitaya/Test/lcr/fpga_awg.c
/RedPitaya/Test/lcr/fpga_awg.h
/RedPitaya/Test/lcr/fpga_awg.o
/RedPitaya/Test/lcr/fpga_osc.c
/RedPitaya/Test/lcr/fpga_osc.h
/RedPitaya/Test/lcr/main_osc.c
/RedPitaya/Test/lcr/main_osc.h
/RedPitaya/Test/lcr/Makefile
/RedPitaya/Test/lcr/version.h
/RedPitaya/Test/lcr/worker.c
/RedPitaya/Test/lcr/worker.h


TO DO:
-the part for selectig sweep frequencies is not included
-only decimation of 1 is avaliable to the user
-finish the lock in method
-only sine signal is put on the output ports. Expanding to square and triangle signal is a must!

