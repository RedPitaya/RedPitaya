CC=$(CROSS_COMPILE)gcc
RM=rm

OBJECTS=main.o

INCLUDE = -I../include
LIB = -L../lib

CFLAGS+= -Wall -g -fPIC -Os -s $(INCLUDE)
LDFLAGS= -Wl,--whole-archive,--no-as-needed
LDFLAGS+=-shared $(LIB) -lrp -lrpapp -lrp_sdk
LDFLAGS+= -Wl,--no-whole-archive

CONTROLLER = ../controllerhf.so

all: $(CONTROLLER)

$(CONTROLLER): $(OBJECTS)
	$(CC) -o $(CONTROLLER) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

clean:
	-$(RM) -f $(OBJECTS)




# CC=$(CROSS_COMPILE)g++
# RM=rm

# OBJECTS=main.o

# INCLUDE+= -I../../../api/include

# CFLAGS+=-Wall -Werror -g -fPIC -Os -s -std=c++11 $(INCLUDE)

# LDFLAGS =-shared
# LDFLAGS+=-L/opt/redpitaya/lib -lrpapp -lrp -lcryptopp -lstdc++

# CONTROLLER = ../controllerhf.so




# all: $(CONTROLLER)

# $(CONTROLLER): $(OBJECTS)
# 	$(CC) -o $(CONTROLLER) $(OBJECTS) $(CFLAGS) $(LDFLAGS)

# clean:
# 	-$(RM) -f $(OBJECTS)
