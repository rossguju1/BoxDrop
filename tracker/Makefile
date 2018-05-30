# Makefile for the Tracker
#
# CS 60, March 2018


##NEED TO REMAKE MAKEFILE FROM START

C = ../common

PROGS = tracker
LIBS = -pthread

CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

tracker: tracker.o $(C)/filetable.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

##### source dependencies
tracker.o: tracker.h $(C)/filetable.h $(C)/constants.h
filetable.o: $(C)/filetable.h $(C)/constants.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: tracker
	$(VALGRIND) ./tracker

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)

