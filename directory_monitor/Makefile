# Makefile for the monitor 

C = ../common

PROGS = monitor
LIBS = -pthread

CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$C -D_BSD_SOURCE
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

monitor: monitor.o $(C)/file.o 
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
##### source dependencies
monitor.o: monitor.h $(C)/constants.h $(C)/file.h

.PHONY: valgrind clean

########### valgrind ##################
valgrind: monitor
	$(VALGRIND) ./monitor

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)

