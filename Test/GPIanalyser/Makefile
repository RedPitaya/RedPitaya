domake: GPIanalyse.c
	$(CROSS_COMPILE)gcc -c -g -std=gnu99 -Wall GPIanalyse.c -o GPIanalyse.o
	$(CROSS_COMPILE)gcc -c -g -std=gnu99 -Wall fpga_awg.c -o fpga_awg.o
	$(CROSS_COMPILE)gcc -c -g -std=gnu99 -Wall genCtrl.c -o genCtrl.o
	$(CROSS_COMPILE)gcc -c -g -std=gnu99 -Wall fpga_osc.c -o fpga_osc.o
	$(CROSS_COMPILE)gcc -c -g -std=gnu99 -Wall main_osc.c -o main_osc.o
	$(CROSS_COMPILE)gcc -c -g -std=gnu99 -Wall worker.c -o worker.o
	$(CROSS_COMPILE)gcc -c -g -std=gnu99 -Wall linAlg.c -o linAlg.o
	$(CROSS_COMPILE)gcc -o GPIanalyse GPIanalyse.o fpga_osc.o worker.o linAlg.o fpga_awg.o genCtrl.o main_osc.o -g -std=gnu99 -Wall -Werror -lm -lpthread
