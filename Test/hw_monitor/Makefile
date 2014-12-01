#Object files to either reference or create
OBJECTS = hw_monitor.o main.o

#Executable file
EXEC = hw_monitor
#FLAGS = -std=c++11
CXX = $(CROSS_COMPILE)g++
CC =  $(CROSS_COMPILE)g++

#Perform action on all objects (May or may not exists)
$(EXEC): $(OBJECTS)
	$(CC) -o $(EXEC) $(OBJECTS)

clean: 
	$(RM) *.o $(EXEC)
