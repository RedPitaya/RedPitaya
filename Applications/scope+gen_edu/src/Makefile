CC=$(CROSS_COMPILE)gcc
RM=rm

OBJECTS=main.o fpga.o worker.o calib.o fpga_awg.o generate.o fpga_pid.o pid.o websocket.o

INCLUDE= -I../../../OS/buildroot/buildroot-2014.02/output/build/libwebsockets-v1.22-chrome26-firefox18/lib/

#CFLAGS=$(CFLAGS) -Wall -Werror -g
CFLAGS+= -Wall -Werror -g -fPIC $(INCLUDE)
LDFLAGS= -shared -L../../../OS/buildroot/buildroot-2014.02/output/target/usr/lib/ -lwebsockets


OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controller.so

all: make_c_app
clean: clean_c_app

make_c_app: $(OBJECTS)
	$(CC) -o $(C_OUT_NAME) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)
